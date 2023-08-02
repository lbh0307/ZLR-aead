#ifndef DEO384_H
#define DEO384_H

#include <x86intrin.h>
#include <stdint.h>

void tweakey_schedule_lane3(const uint8_t key[16], __m128i stk[17]);

void deoxysBC_ctr_x4(__m128i inout[4], const __m128i tweaks[4],
                     const __m128i stk[17], const __m128i tw_c[17][4],
                     const __m128i base_tk1[8]);

void deoxysBC(__m128i inout[1], const __m128i tweak,
              const __m128i stk[17], const __m128i base_tk1[8]);

#endif /* DEO384_H */
