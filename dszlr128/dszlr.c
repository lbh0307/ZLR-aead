#include <x86intrin.h>
#include <stdint.h>
#include "deoxysBC-384.h"

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
    const __m128i H_PERMUTATION = _mm_set_epi8( 7,0,13,10, 11,4,1,14, 15,8,5,2, 3,12,9,6 );
    const __m128i EIGHT	= _mm_set_epi8( 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,8);
    const __m128i ONE   = _mm_set_epi8( 0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,1);
    const __m128i MSB_AD = _mm_set_epi8( (0x0<<4),0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00 );
    const __m128i MSB_M1 = _mm_set_epi8( (0x1<<4),0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00 );
    const __m128i MSB_M2 = _mm_set_epi8( (0x2<<4),0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00 );
    const __m128i MSB_M3 = _mm_set_epi8( (0x3<<4),0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00 );
    const __m128i MSB_TAG = _mm_set_epi8( (0x4<<4),0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00 );
    __m128i tk1_add_eight[8], tk1_base_ad[8], tk1_base_m1[8], tk1_base_m2[8], tk1_base_m3[8], tk1_base_tag[8];
    __m128i tw_c[17][4], stk[17];
    __m128i block[4], tweak[4];
    __m128i U, V, tmp, t0, t1;
    int i, j;

    tmp = ONE;
    for(i = 0; i < 17; i++)
    {
        tw_c[i][0] = _mm_setzero_si128();
        tw_c[i][1] = _mm_add_epi8(tw_c[i][0], tmp);
        tw_c[i][2] = _mm_add_epi8(tw_c[i][1], tmp);
        tw_c[i][3] = _mm_add_epi8(tw_c[i][2], tmp);
        tmp = _mm_shuffle_epi8(tmp, H_PERMUTATION);
    }

    tmp = EIGHT;
    for (i = 0; i < 8; i++)
    {
        tk1_add_eight[i] = tmp;
        tmp = _mm_shuffle_epi8(tmp, H_PERMUTATION);
    }

    tk1_base_ad[0] = MSB_AD;
    tk1_base_m1[0] = MSB_M1;
    tk1_base_m2[0] = MSB_M2;
    tk1_base_m3[0] = MSB_M3;
    tk1_base_tag[0] = MSB_TAG;
    for (i = 1; i < 8; i++)
    {
        tk1_base_ad[i] = _mm_shuffle_epi8(tk1_base_ad[i-1], H_PERMUTATION);
        tk1_base_m1[i] = _mm_shuffle_epi8(tk1_base_m1[i-1], H_PERMUTATION);
        tk1_base_m2[i] = _mm_shuffle_epi8(tk1_base_m2[i-1], H_PERMUTATION);
        tk1_base_m3[i] = _mm_shuffle_epi8(tk1_base_m3[i-1], H_PERMUTATION);
        tk1_base_tag[i] = _mm_shuffle_epi8(tk1_base_tag[i-1], H_PERMUTATION);
    }

    U = _mm_setzero_si128();
    V = _mm_setzero_si128();

    /* Key schedule */
    tweakey_schedule_lane3(key, stk);

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

        deoxysBC_ctr_x4(block, tweak, stk, tw_c, tk1_base_ad);

        // rho
        U = MULX(_mm_xor_si128(U, _mm_xor_si128(block[0], tweak[0])));
        V = _mm_xor_si128(V, tweak[0]);
        U = MULX(_mm_xor_si128(U, _mm_xor_si128(block[1], tweak[1])));
        V = _mm_xor_si128(V, tweak[1]);
        U = MULX(_mm_xor_si128(U, _mm_xor_si128(block[2], tweak[2])));
        V = _mm_xor_si128(V, tweak[2]);
        U = MULX(_mm_xor_si128(U, _mm_xor_si128(block[3], tweak[3])));
        V = _mm_xor_si128(V, tweak[3]);

        tk1_base_ad[0] = _mm_add_epi8(tk1_base_ad[0], tk1_add_eight[0]);
        tk1_base_ad[1] = _mm_add_epi8(tk1_base_ad[1], tk1_add_eight[1]);
        tk1_base_ad[2] = _mm_add_epi8(tk1_base_ad[2], tk1_add_eight[2]);
        tk1_base_ad[3] = _mm_add_epi8(tk1_base_ad[3], tk1_add_eight[3]);
        tk1_base_ad[4] = _mm_add_epi8(tk1_base_ad[4], tk1_add_eight[4]);
        tk1_base_ad[5] = _mm_add_epi8(tk1_base_ad[5], tk1_add_eight[5]);
        tk1_base_ad[6] = _mm_add_epi8(tk1_base_ad[6], tk1_add_eight[6]);
        tk1_base_ad[7] = _mm_add_epi8(tk1_base_ad[7], tk1_add_eight[7]);
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

        deoxysBC_ctr_x4(block, tweak, stk, tw_c, tk1_base_m1);

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

        deoxysBC_ctr_x4(tweak, block, stk, tw_c, tk1_base_m2);
        deoxysBC_ctr_x4(block, tweak, stk, tw_c, tk1_base_m3);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 0), block[0]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 16), tweak[0]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 32), block[1]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 48), tweak[1]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 64), block[2]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 80), tweak[2]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 96), block[3]);
        _mm_storeu_si128((__m128i *)(ciphertext + 32 * i + 112), tweak[3]);

        for (j = 0; j < 8; j++)
        {
            tk1_base_m1[j] = _mm_add_epi8(tk1_base_m1[j], tk1_add_eight[j]);
            tk1_base_m2[j] = _mm_add_epi8(tk1_base_m2[j], tk1_add_eight[j]);
            tk1_base_m3[j] = _mm_add_epi8(tk1_base_m3[j], tk1_add_eight[j]);
        }
    }

    /* Tag */
    deoxysBC(&U, V, stk, tk1_base_tag);
    _mm_storeu_si128((__m128i *)(ciphertext + m_len), U);
}
