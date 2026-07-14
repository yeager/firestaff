#include "redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    const uint8_t source[] = {0xa5u, 0x20u, 0x7eu};
    uint8_t aperture[10] = {0x91u, 0x92u, 0x93u, 0x94u, 0x95u,
                             0x96u, 0x97u, 0x98u, 0x99u, 0x9au};
    uint8_t before[sizeof(aperture)];
    int ok = 1;

    ok &= check(
        redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat(
            source, sizeof(source), 0u, aperture, sizeof(aperture), 2u, 6u,
            0u, 0x30u) == 1 &&
        memcmp(aperture,
               (const uint8_t[]){0x91u, 0x92u, 0x3eu, 0x37u, 0x95u,
                                 0x32u, 0x35u, 0x3au, 0x99u, 0x9au},
               sizeof(aperture)) == 0,
        "reverses high/low nibble pixels into byte aperture and preserves zero transparency");

    memset(aperture, 0x55, sizeof(aperture));
    ok &= check(
        redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat(
            source, sizeof(source), 1u, aperture, sizeof(aperture), 3u, 4u,
            0x0eu, 0xa0u) == 1 &&
        memcmp(aperture,
               (const uint8_t[]){0x55u, 0x55u, 0x55u, 0xa7u, 0xa0u,
                                 0xa2u, 0xa5u, 0x55u, 0x55u, 0x55u},
               sizeof(aperture)) == 0,
        "uses raw offset OR rather than masking or addition");

    memcpy(before, aperture, sizeof(aperture));
    ok &= check(
        redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat(
            source, 2u, 0u, aperture, sizeof(aperture), 0u, 5u, 0u, 0u) == 0 &&
        redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat(
            source, sizeof(source), 0u, aperture, 4u, 0u, 5u, 0u, 0u) == 0 &&
        memcmp(aperture, before, sizeof(aperture)) == 0,
        "rejects incomplete ranges before writing the aperture");

    ok &= check(
        strstr(redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat_source_evidence(),
               "IMAGE3.C:612-829") != NULL &&
        strstr(redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat_source_evidence(),
               "IMAGE5.C:936-1010") != NULL,
        "records exact source branches");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0683_c25_vga_flipped_transparent_copy_pc34_compat");
    return 0;
}
