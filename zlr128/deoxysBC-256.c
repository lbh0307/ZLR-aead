#include <x86intrin.h>
#include <stdint.h>

#include "deoxysBC-256.h"

void tweakey_schedule_lane2(const uint8_t key[16], __m128i stk[17])
{
    const uint8_t rcon[17] = {0x2f,0x5e,0xbc,0x63,0xc6,0x97,
                              0x35,0x6a,0xd4,0xb3,0x7d,0xfa,
                              0xef,0xc5,0x91,0x39,0x72};
    const __m128i H_PERMUTATION = _mm_set_epi8( 7,0,13,10, 11,4,1,14, 15,8,5,2, 3,12,9,6 );
    const __m128i mask_bottom_1_bit = _mm_set1_epi8(0x1);
    const __m128i mask_top_7_bits = _mm_set1_epi8(0xfe);
    __m128i tmp1, tmp2, tmp3, tmp4;
    __m128i RCONS[17];
    int i;

    // Set up the round constants
    for(i = 0; i < 17; i++)
    {
        RCONS[i] = _mm_set_epi8(1,2,4,8, rcon[i],rcon[i],rcon[i],rcon[i], 0,0,0,0, 0,0,0,0);
    }

    stk[0] = _mm_loadu_si128((__m128i *)key);
    stk[0] = tmp4 = _mm_xor_si128(stk[0], RCONS[0]);

    for(i = 1; i < 17; i++)
    {
        // LFSR2
        tmp1 = _mm_slli_epi32(tmp4, 1);
        tmp2 = _mm_srli_epi32(tmp4, 7);
        tmp3 = _mm_srli_epi32(tmp4, 5);

        tmp1 = _mm_and_si128(mask_top_7_bits, tmp1);
        tmp2 = _mm_and_si128(mask_bottom_1_bit, tmp2);
        tmp3 = _mm_and_si128(mask_bottom_1_bit, tmp3);

        tmp1 = _mm_xor_si128(tmp1, tmp2);
        tmp3 = _mm_xor_si128(tmp3, RCONS[i]);

        tmp4 = _mm_xor_si128(tmp1, tmp3);

        // H-PERM
        stk[i] = tmp4 = _mm_shuffle_epi8(tmp4, H_PERMUTATION);
    }
}

void deoxysBC(__m128i inout[1], const __m128i tweak,
              const __m128i stk[17])
{
    const __m128i H_PERMUTATION = _mm_set_epi8( 7,0,13,10, 11,4,1,14, 15,8,5,2, 3,12,9,6 );
    __m128i tk = tweak;

    inout[0] = _mm_xor_si128(inout[0], _mm_xor_si128(stk[0], tk));
    for(int r = 1; r < 17; r++)
    {
        tk = _mm_shuffle_epi8(tk, H_PERMUTATION);
        inout[0] = _mm_aesenc_si128(inout[0], _mm_xor_si128(stk[r], tk));
    }
}

void deoxysBC_x4(__m128i inout[4], const __m128i tweaks[4],
                 const __m128i stk[17])
{
    const __m128i H_PERMUTATION = _mm_set_epi8( 7,0,13,10, 11,4,1,14, 15,8,5,2, 3,12,9,6 );
    __m128i tk1[4];
    tk1[0] = tweaks[0];
    tk1[1] = tweaks[1];
    tk1[2] = tweaks[2];
    tk1[3] = tweaks[3];

    inout[0] = _mm_xor_si128(inout[0], _mm_xor_si128(tk1[0], stk[0]));
    inout[1] = _mm_xor_si128(inout[1], _mm_xor_si128(tk1[1], stk[0]));
    inout[2] = _mm_xor_si128(inout[2], _mm_xor_si128(tk1[2], stk[0]));
    inout[3] = _mm_xor_si128(inout[3], _mm_xor_si128(tk1[3], stk[0]));

    for(int r = 1; r < 17; r++)
    {
        tk1[0] = _mm_shuffle_epi8(tk1[0], H_PERMUTATION);
        tk1[1] = _mm_shuffle_epi8(tk1[1], H_PERMUTATION);
        tk1[2] = _mm_shuffle_epi8(tk1[2], H_PERMUTATION);
        tk1[3] = _mm_shuffle_epi8(tk1[3], H_PERMUTATION);

        inout[0] = _mm_aesenc_si128(inout[0], _mm_xor_si128(tk1[0], stk[r]));
        inout[1] = _mm_aesenc_si128(inout[1], _mm_xor_si128(tk1[1], stk[r]));
        inout[2] = _mm_aesenc_si128(inout[2], _mm_xor_si128(tk1[2], stk[r]));
        inout[3] = _mm_aesenc_si128(inout[3], _mm_xor_si128(tk1[3], stk[r]));
    }
}

void deoxysBC_zeroweak_x4(__m128i inout[4], const __m128i stk[17])
{
    inout[0] = _mm_xor_si128(inout[0], stk[0]);
    inout[1] = _mm_xor_si128(inout[1], stk[0]);
    inout[2] = _mm_xor_si128(inout[2], stk[0]);
    inout[3] = _mm_xor_si128(inout[3], stk[0]);

    for(int r = 1; r < 17; r++)
    {
        inout[0] = _mm_aesenc_si128(inout[0], stk[r]);
        inout[1] = _mm_aesenc_si128(inout[1], stk[r]);
        inout[2] = _mm_aesenc_si128(inout[2], stk[r]);
        inout[3] = _mm_aesenc_si128(inout[3], stk[r]);
    }
}
