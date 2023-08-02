#include "util.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

static unsigned char rxbuf[256];
static size_t rxcnt = 0;

uint64_t RDTSC_START()
{
    unsigned cyc_high, cyc_low;
    __asm volatile("" ::: /* pretend to clobber */ "memory");             \
    __asm volatile(
            "cpuid\n\t"
            "rdtsc\n\t"
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t"
            : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx");
    return ((uint64_t)cyc_high << 32) | cyc_low;
}

uint64_t RDTSC_END()
{
    unsigned cyc_high, cyc_low;
    __asm volatile(
            "rdtscp\n\t"
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t"
            "cpuid\n\t"
            : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx");
    return ((uint64_t)cyc_high << 32) | cyc_low;
}

void rand_bytes(void* buf, size_t sz)
{
    size_t idx = 0;
    size_t cnt = 0;
    while (sz > 0)
    {
        if (rxcnt == 0)
        {
            if (getrandom(rxbuf, 256, 0) != 256)
            {
                puts("Invalid random");
                exit(-1);
            }

            rxcnt = 256;
        }

        cnt = sz < rxcnt ? sz : rxcnt;

        memcpy((uint8_t *)buf + idx, rxbuf + (256 - rxcnt), cnt);
        sz -= cnt;
        rxcnt -= cnt;
        idx += cnt;
    }
}

void p128(__m128i in)
{
    unsigned long long v[2] __attribute__((aligned(16)));
    _mm_store_si128((__m128i*)v, in);
    printf("%llu %llu\n", v[0], v[1]);
}

void p256(__m256i in)
{
    unsigned long long v[4] __attribute__((aligned(32)));
    _mm256_store_si256((__m256i*)v, in);
    printf("%llu %llu %llu %llu\n", v[0], v[1], v[2], v[3]);
}
