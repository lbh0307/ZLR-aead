#include <x86intrin.h>
#include <stdint.h>
#include "deoxysBC-256.h"

inline __m128i MULX(__m128i x)
{
    __m128i a, b, s, cp;

    /*
    cp = _mm_set_epi64x(0x0LL, 0x87LL);

    s = _mm_setzero_si128();
    b = _mm_shuffle_epi32(x, 0b11111111); // braodcast upper most dword
    a = x;

    x = _mm_slli_epi64(x, 1);

    a = _mm_srli_epi64(a, 63);
    s = _mm_cmpgt_epi32(s, b); // bradcast sign bit

    a = _mm_slli_si128(a, 8);
    x = _mm_or_si128(x, a);

    s = _mm_and_si128(s, cp); // magic reduction
    x = _mm_xor_si128(x, s);
    */

    /*
        t0 = D;
        twres = _mm_shuffle_epi32(D, 0b01011111);

        twtmp = _mm_srai_epi32(twres, 31);
        twres = _mm_add_epi32(twres, twres);
        t1 = _mm_add_epi64(t0, t0);
        twtmp = _mm_and_si128(twtmp, twmask);
        t1 = _mm_xor_si128(t1, twtmp);
    */

    cp = _mm_set_epi64x(0x1LL, 0x87LL);
    a = _mm_shuffle_epi32(x, 0b01011111);
    a = _mm_srai_epi32(a, 31);
    a = _mm_and_si128(a, cp);
    x = _mm_add_epi64(x, x);
    x = _mm_xor_si128(x, a);

    return x;
}

void dszlr_encrypt(const unsigned char *ass_data, unsigned long long int ass_data_len,
                   const unsigned char *message, unsigned long long int m_len,
                   const unsigned char *key, unsigned char *ciphertext)
{
    const __m128i ZERO  = _mm_setzero_si128();
    const __m128i ONE   = _mm_set_epi8( 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,1);
    const __m128i TWO   = _mm_set_epi8( 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,2);
    const __m128i THREE = _mm_set_epi8( 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,3);
    __m128i block[4], tweak[4], Dels[6], stk[17];
    __m128i twres, twtmp, twmask;
    __m128i U, V, t0, t1;
    int i, j;

    twmask = _mm_set_epi64x(0x1LL, 0x87LL);
    U = _mm_setzero_si128();
    V = _mm_setzero_si128();

    /* Key schedule */
    tweakey_schedule_lane2(key, stk);

    /* Generate Delta */
    Dels[0] = ZERO;
    Dels[1] = ONE;
    Dels[2] = TWO;
    Dels[3] = THREE;

    deoxysBC_zeroweak_x4(Dels, stk);
    Dels[4] = _mm_xor_si128(Dels[0], Dels[2]);
    Dels[5] = _mm_xor_si128(Dels[1], Dels[3]);

    /* Associated Data */
    for (i = 0 ; i < ass_data_len; i += 4)
    {

        block[0] = _mm_loadu_si128((__m128i *)(ass_data + 32 * i));
        tweak[0] = _mm_loadu_si128((__m128i *)(ass_data + 32 * i + 16));
        block[1] = _mm_loadu_si128((__m128i *)(ass_data + 32 * i + 32));
        tweak[1] = _mm_loadu_si128((__m128i *)(ass_data + 32 * i + 48));
        block[2] = _mm_loadu_si128((__m128i *)(ass_data + 32 * i + 64));
        tweak[2] = _mm_loadu_si128((__m128i *)(ass_data + 32 * i + 80));
        block[3] = _mm_loadu_si128((__m128i *)(ass_data + 32 * i + 96));
        tweak[3] = _mm_loadu_si128((__m128i *)(ass_data + 32 * i + 112));

        twres = _mm_shuffle_epi32(Dels[0], 0b01011111);
        for (j = 0; j < 4; j++)
        {
            block[j] = _mm_xor_si128(block[j], Dels[0]);

            twtmp = _mm_srai_epi32(twres, 31);
            twres = _mm_add_epi32(twres, twres);
            Dels[0] = _mm_add_epi64(Dels[0], Dels[0]);
            twtmp = _mm_and_si128(twtmp, twmask);
            Dels[0] = _mm_xor_si128(Dels[0], twtmp);
        }

        twres = _mm_shuffle_epi32(Dels[1], 0b01011111);
        for (j = 0; j < 4; j++)
        {
            tweak[j] = _mm_xor_si128(tweak[j], Dels[1]);

            twtmp = _mm_srai_epi32(twres, 31);
            twres = _mm_add_epi32(twres, twres);
            Dels[1] = _mm_add_epi64(Dels[1], Dels[1]);
            twtmp = _mm_and_si128(twtmp, twmask);
            Dels[1] = _mm_xor_si128(Dels[1], twtmp);
        }

        deoxysBC_x4(block, tweak, stk);

        // rho
        U = MULX(_mm_xor_si128(U, _mm_xor_si128(block[0], tweak[0])));
        V = _mm_xor_si128(V, tweak[0]);
        U = MULX(_mm_xor_si128(U, _mm_xor_si128(block[1], tweak[1])));
        V = _mm_xor_si128(V, tweak[1]);
        U = MULX(_mm_xor_si128(U, _mm_xor_si128(block[2], tweak[2])));
        V = _mm_xor_si128(V, tweak[2]);
        U = MULX(_mm_xor_si128(U, _mm_xor_si128(block[3], tweak[3])));
        V = _mm_xor_si128(V, tweak[3]);
    }

    /* Message */
    for (i = 0 ; i < m_len; i += 4)
    {
        block[0] = _mm_loadu_si128((__m128i *)(message + 32 * i));
        tweak[0] = _mm_loadu_si128((__m128i *)(message + 32 * i + 16));
        block[1] = _mm_loadu_si128((__m128i *)(message + 32 * i + 32));
        tweak[1] = _mm_loadu_si128((__m128i *)(message + 32 * i + 48));
        block[2] = _mm_loadu_si128((__m128i *)(message + 32 * i + 64));
        tweak[2] = _mm_loadu_si128((__m128i *)(message + 32 * i + 80));
        block[3] = _mm_loadu_si128((__m128i *)(message + 32 * i + 96));
        tweak[3] = _mm_loadu_si128((__m128i *)(message + 32 * i + 112));

        twres = _mm_shuffle_epi32(Dels[0], 0b01011111);
        for (j = 0; j < 4; j++)
        {
            block[j] = _mm_xor_si128(block[j], Dels[2]);

            twtmp = _mm_srai_epi32(twres, 31);
            twres = _mm_add_epi32(twres, twres);
            Dels[2] = _mm_add_epi64(Dels[2], Dels[2]);
            twtmp = _mm_and_si128(twtmp, twmask);
            Dels[2] = _mm_xor_si128(Dels[2], twtmp);
        }

        twres = _mm_shuffle_epi32(Dels[3], 0b01011111);
        for (j = 0; j < 4; j++)
        {
            tweak[j] = _mm_xor_si128(tweak[j], Dels[3]);

            twtmp = _mm_srai_epi32(twres, 31);
            twres = _mm_add_epi32(twres, twres);
            Dels[3] = _mm_add_epi64(Dels[3], Dels[3]);
            twtmp = _mm_and_si128(twtmp, twmask);
            Dels[3] = _mm_xor_si128(Dels[3], twtmp);
        }

        deoxysBC_x4(block, tweak, stk);

        // rho
        for (j = 0; j < 4; j++)
        {
            t0 = block[j];
            t1 = tweak[j];
            block[j] = _mm_xor_si128(V, _mm_xor_si128(block[j], tweak[j]));
            tweak[j] = _mm_xor_si128(U, block[j]);
            U = MULX(_mm_xor_si128(U, _mm_xor_si128(t0, t1)));
            V = _mm_xor_si128(V, t1);
        }

        deoxysBC_x4(tweak, block, stk);
        deoxysBC_x4(block, tweak, stk);

        twres = _mm_shuffle_epi32(Dels[0], 0b01011111);
        for (j = 0; j < 4; j++)
        {
            block[j] = _mm_xor_si128(block[j], Dels[4]);

            twtmp = _mm_srai_epi32(twres, 31);
            twres = _mm_add_epi32(twres, twres);
            Dels[4] = _mm_add_epi64(Dels[4], Dels[4]);
            twtmp = _mm_and_si128(twtmp, twmask);
            Dels[4] = _mm_xor_si128(Dels[4], twtmp);
        }

        twres = _mm_shuffle_epi32(Dels[3], 0b01011111);
        for (j = 0; j < 4; j++)
        {
            tweak[j] = _mm_xor_si128(tweak[j], Dels[5]);

            twtmp = _mm_srai_epi32(twres, 31);
            twres = _mm_add_epi32(twres, twres);
            Dels[5] = _mm_add_epi64(Dels[5], Dels[5]);
            twtmp = _mm_and_si128(twtmp, twmask);
            Dels[5] = _mm_xor_si128(Dels[5], twtmp);
        }

        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 0), block[0]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 16), tweak[0]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 32), block[1]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 48), tweak[1]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 64), block[2]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 80), tweak[2]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 96), block[3]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 112), tweak[3]);
    }

    /* Tag */
    deoxysBC(&U, V, stk);
    _mm_storeu_si128((__m128i *)(ciphertext + m_len), U);
}
