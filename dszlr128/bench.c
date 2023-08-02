#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "dszlr.h"
#include "util.h"

int main(int argc, char *argv[])
{
    uint8_t *aad, *pt, *ct;
    size_t aad_len, pt_len;

    uint8_t key[16];

    if (argc == 1)
    {
        aad_len = 16 * 4;
        pt_len = 16 * 4;
    }
    else if (argc == 2)
    {
        aad_len = 16 * 4;
        pt_len = atoi(argv[1]);
    }
    else if (argc == 3)
    {
        aad_len = atoi(argv[1]);
        pt_len = atoi(argv[2]);
    }
    printf("DSZLR-128 Test (aad_len = %ld, pt_len = %ld, repeat = %d)\n",
            aad_len, pt_len, REPEAT);

    aad = (uint8_t *) malloc(aad_len);
    pt = (uint8_t *) malloc(pt_len);
    ct = (uint8_t *) malloc(pt_len+16);

    rand_bytes(key, sizeof(key));
    rand_bytes(pt, sizeof(pt));
    rand_bytes(aad, sizeof(aad));

    MEASURE(dszlr_encrypt(aad, aad_len/16, pt, pt_len/16, key, ct);)

    printf("C/B: %.2f\n", CYCLES_PER_ITER / (aad_len + pt_len));
}
