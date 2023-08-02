#include <x86intrin.h>
#include <stdint.h>

#include "deoxysBC-384.h"

void tweakey_schedule_lane3(const uint8_t key[16], __m128i stk[17])
{
    const uint8_t rcon[17] = {0x2f,0x5e,0xbc,0x63,0xc6,0x97,
                              0x35,0x6a,0xd4,0xb3,0x7d,0xfa,
                              0xef,0xc5,0x91,0x39,0x72};
    const __m128i H_PERMUTATION = _mm_set_epi8( 7,0,13,10, 11,4,1,14, 15,8,5,2, 3,12,9,6 );
    const __m128i mask_bottom_7_bits = _mm_set1_epi8(0x7f);
    const __m128i mask_top_1_bit = _mm_set1_epi8(0x80);
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
        // LFSR4
        tmp1 = _mm_srli_epi32(tmp4, 1);
        tmp2 = _mm_slli_epi32(tmp4, 7);
        tmp3 = _mm_slli_epi32(tmp4, 1);

        tmp1 = _mm_and_si128(mask_bottom_7_bits, tmp1);
        tmp2 = _mm_and_si128(mask_top_1_bit, tmp2);
        tmp3 = _mm_and_si128(mask_top_1_bit, tmp3);

        tmp1 = _mm_xor_si128(tmp1, tmp2);
        tmp3 = _mm_xor_si128(tmp3, RCONS[i]);

        tmp4 = _mm_xor_si128(tmp1, tmp3);

        // H-PERM
        stk[i] = tmp4 = _mm_shuffle_epi8(tmp4, H_PERMUTATION);
    }
}

void tweakey_schedule_lane2(const __m128i tweak, __m128i tk2[17])
{
    const __m128i H_PERMUTATION = _mm_set_epi8( 7,0,13,10, 11,4,1,14, 15,8,5,2, 3,12,9,6 );
    const __m128i mask_bottom_1_bit = _mm_set1_epi8(0x1);
    const __m128i mask_top_7_bits = _mm_set1_epi8(0xfe);
    __m128i tmp1, tmp2, tmp3, tmp4;

    tmp4 = tk2[0] = tweak;

    for(int i = 1; i < 17; i++)
    {
        // LFSR2
        tmp1 = _mm_slli_epi32(tmp4, 1);
        tmp2 = _mm_srli_epi32(tmp4, 7);
        tmp3 = _mm_srli_epi32(tmp4, 5);

        tmp1 = _mm_and_si128(mask_top_7_bits, tmp1);
        tmp2 = _mm_and_si128(mask_bottom_1_bit, tmp2);
        tmp3 = _mm_and_si128(mask_bottom_1_bit, tmp3);

        tmp1 = _mm_xor_si128(tmp1, tmp2);
        tmp4 = _mm_xor_si128(tmp1, tmp3);

        // H-PERM
        tk2[i] = tmp4 = _mm_shuffle_epi8(tmp4, H_PERMUTATION);
    }
}

void tweakey_schedule_lane2_x4(const __m128i tweaks[4], __m128i tk2[17][4])
{
    const __m128i H_PERMUTATION = _mm_set_epi8( 7,0,13,10, 11,4,1,14, 15,8,5,2, 3,12,9,6 );
    const __m128i mask_bottom_1_bit = _mm_set1_epi8(0x1);
    const __m128i mask_top_7_bits = _mm_set1_epi8(0xfe);
    __m128i tmp[8];

    tk2[0][0] = tweaks[0];
    tk2[0][1] = tweaks[1];
    tk2[0][2] = tweaks[2];
    tk2[0][3] = tweaks[3];

    // LFSR2
    for(int i = 1; i < 17; i++)
    {
        tmp[0] = _mm_slli_epi32(tk2[i-1][0], 1);
        tmp[1] = _mm_slli_epi32(tk2[i-1][1], 1);
        tmp[2] = _mm_slli_epi32(tk2[i-1][2], 1);
        tmp[3] = _mm_slli_epi32(tk2[i-1][3], 1);

        tmp[4] = _mm_srli_epi32(tk2[i-1][0], 7);
        tmp[5] = _mm_srli_epi32(tk2[i-1][1], 7);
        tmp[6] = _mm_srli_epi32(tk2[i-1][2], 7);
        tmp[7] = _mm_srli_epi32(tk2[i-1][3], 7);

        tmp[0] = _mm_and_si128(mask_top_7_bits, tmp[0]);
        tmp[1] = _mm_and_si128(mask_top_7_bits, tmp[1]);
        tmp[2] = _mm_and_si128(mask_top_7_bits, tmp[2]);
        tmp[3] = _mm_and_si128(mask_top_7_bits, tmp[3]);

        tmp[4] = _mm_and_si128(mask_bottom_1_bit, tmp[4]);
        tmp[5] = _mm_and_si128(mask_bottom_1_bit, tmp[5]);
        tmp[6] = _mm_and_si128(mask_bottom_1_bit, tmp[6]);
        tmp[7] = _mm_and_si128(mask_bottom_1_bit, tmp[7]);

        tmp[4] = _mm_and_si128(tmp[4], tmp[0]);
        tmp[5] = _mm_and_si128(tmp[5], tmp[1]);
        tmp[6] = _mm_and_si128(tmp[6], tmp[2]);
        tmp[7] = _mm_and_si128(tmp[7], tmp[3]);

        tmp[0] = _mm_srli_epi32(tk2[i-1][0], 5);
        tmp[1] = _mm_srli_epi32(tk2[i-1][1], 5);
        tmp[2] = _mm_srli_epi32(tk2[i-1][2], 5);
        tmp[3] = _mm_srli_epi32(tk2[i-1][3], 5);

        tmp[0] = _mm_and_si128(mask_bottom_1_bit, tmp[0]);
        tmp[1] = _mm_and_si128(mask_bottom_1_bit, tmp[1]);
        tmp[2] = _mm_and_si128(mask_bottom_1_bit, tmp[2]);
        tmp[3] = _mm_and_si128(mask_bottom_1_bit, tmp[3]);

        tmp[0] = _mm_xor_si128(tmp[4], tmp[0]);
        tmp[1] = _mm_xor_si128(tmp[5], tmp[1]);
        tmp[2] = _mm_xor_si128(tmp[6], tmp[2]);
        tmp[3] = _mm_xor_si128(tmp[7], tmp[3]);

        // H-Perm
        tk2[i][0] = _mm_shuffle_epi8(tmp[0], H_PERMUTATION);
        tk2[i][1] = _mm_shuffle_epi8(tmp[1], H_PERMUTATION);
        tk2[i][2] = _mm_shuffle_epi8(tmp[2], H_PERMUTATION);
        tk2[i][3] = _mm_shuffle_epi8(tmp[3], H_PERMUTATION);
    }
}

void deoxysBC(__m128i inout[1], const __m128i tweak,
              const __m128i stk[17], const __m128i base_tk1[8])
{
    __m128i tk2[17];
    __m128i tmp;

    tweakey_schedule_lane2(tweak, tk2);

    tmp = _mm_xor_si128(stk[0], base_tk1[0]);
    tk2[0] = _mm_xor_si128(tk2[0], tmp);

    inout[0] = _mm_xor_si128(inout[0], tk2[0]);

    for(int r = 1; r < 17; r++)
    {
        tmp = _mm_xor_si128(stk[r], base_tk1[r % 8]);
        tk2[r] = _mm_xor_si128(tk2[r], tmp);
        inout[0] = _mm_aesenc_si128(inout[0], tk2[r]);
    }
}

void deoxysBC_ctr_x4(__m128i inout[4], const __m128i tweaks[4],
                     const __m128i stk[17], const __m128i tw_c[17][4],
                     const __m128i base_tk1[8])
{
    __m128i tk2[17][4];
    __m128i tmp;

    tweakey_schedule_lane2_x4(tweaks, tk2);

    tmp = _mm_xor_si128(stk[0], base_tk1[0]);
    tk2[0][0] = _mm_xor_si128(tk2[0][0], tmp);
    tk2[0][1] = _mm_xor_si128(tk2[0][1], _mm_xor_si128(tmp, tw_c[0][1]));
    tk2[0][2] = _mm_xor_si128(tk2[0][2], _mm_xor_si128(tmp, tw_c[0][2]));
    tk2[0][3] = _mm_xor_si128(tk2[0][3], _mm_xor_si128(tmp, tw_c[0][3]));

    inout[0] = _mm_xor_si128(inout[0], tk2[0][0]);
    inout[1] = _mm_xor_si128(inout[1], tk2[0][1]);
    inout[2] = _mm_xor_si128(inout[2], tk2[0][2]);
    inout[3] = _mm_xor_si128(inout[3], tk2[0][3]);

    for(int r = 1; r < 17; r++)
    {
        tmp = _mm_xor_si128(stk[r], base_tk1[r % 8]);
        tk2[r][0] = _mm_xor_si128(tk2[r][0], tmp);
        tk2[r][1] = _mm_xor_si128(tk2[r][1], _mm_xor_si128(tmp, tw_c[r][1]));
        tk2[r][2] = _mm_xor_si128(tk2[r][2], _mm_xor_si128(tmp, tw_c[r][2]));
        tk2[r][3] = _mm_xor_si128(tk2[r][3], _mm_xor_si128(tmp, tw_c[r][3]));

        inout[0] = _mm_aesenc_si128(inout[0], tk2[r][0]);
        inout[1] = _mm_aesenc_si128(inout[1], tk2[r][1]);
        inout[2] = _mm_aesenc_si128(inout[2], tk2[r][2]);
        inout[3] = _mm_aesenc_si128(inout[3], tk2[r][3]);
    }
}
