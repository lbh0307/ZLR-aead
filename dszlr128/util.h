#ifndef UTIL_H
#define UTIL_H

#include <sys/random.h>
#include <x86intrin.h>
#include <stdint.h>

static uint64_t NCLOCKS_START;
static uint64_t NCLOCKS_END;
static uint64_t NCLOCKS;
static double CYCLES_PER_ITER;


#if 0
#define CLOCK_START() {NCLOCKS = RDTSC_START();}
#define CLOCK_END() {NCLOCKS = RDTSC_END() - NCLOCKS;}
#else
  #ifndef REPEAT
  #define REPEAT 1024
  #endif

  #ifndef OUTER_REPEAT
  #define OUTER_REPEAT 5
  #endif

  #ifndef WARMUP
  #define WARMUP REPEAT/4
  #endif

  static int RDTSC_BENCH_ITER;
  static int RDTSC_OUTER_ITER;
  static uint64_t RDTSC_TEMP_CLK;

  #define MEASURE(x)                                                              \
  for (RDTSC_BENCH_ITER = 0; RDTSC_BENCH_ITER < WARMUP; RDTSC_BENCH_ITER++)       \
  {                                                                               \
      {x};                                                                        \
  }                                                                               \
  NCLOCKS = UINT64_MAX;                                                           \
  for (RDTSC_OUTER_ITER = 0; RDTSC_OUTER_ITER < OUTER_REPEAT; RDTSC_OUTER_ITER++) \
  {                                                                               \
      RDTSC_TEMP_CLK = RDTSC_START();                                             \
      for (RDTSC_BENCH_ITER = 0; RDTSC_BENCH_ITER < REPEAT; RDTSC_BENCH_ITER++)   \
      {                                                                           \
          {x};                                                                    \
      }                                                                           \
      RDTSC_TEMP_CLK = RDTSC_END() - RDTSC_TEMP_CLK;                              \
      if (RDTSC_TEMP_CLK < NCLOCKS)                                               \
          NCLOCKS = RDTSC_TEMP_CLK;                                               \
  }                                                                               \
  CYCLES_PER_ITER = (double) NCLOCKS / REPEAT;
#endif

typedef unsigned __int128 uint128_t;

typedef union m128_st {
    uint8_t c[16];
    uint16_t s[8];
    uint32_t i[4];
    uint64_t d[2];
    uint128_t l;
    __m128i x;
} __attribute__((aligned(16))) M128;

uint64_t RDTSC_START();
uint64_t RDTSC_END();

void rand_bytes(void*, size_t);
void p128(__m128i);
void p256(__m256i);

#endif /* UTIL_H */
