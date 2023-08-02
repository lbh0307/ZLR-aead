#ifndef DEO256_H
#define DEO256_H

#include <x86intrin.h>
#include <stdint.h>

void tweakey_schedule_lane2(const uint8_t key[16], __m128i stk[17]);

void deoxysBC_x4(__m128i inout[4], const __m128i tweaks[4],
                 const __m128i stk[17]);

void deoxysBC(__m128i inout[1], const __m128i tweak,
              const __m128i stk[17]);

void deoxysBC_zeroweak_x4(__m128i inout[4], const __m128i stk[17]);

#endif /* DEO256_H */
