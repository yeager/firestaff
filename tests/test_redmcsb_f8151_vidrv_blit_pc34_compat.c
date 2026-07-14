#include "redmcsb_f8151_vidrv_blit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_byte(const char *name, uint8_t actual, uint8_t expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %u, expected %u)\n", name,
                (unsigned)actual, (unsigned)expected);
        ++failures;
    }
}

static void expect_true(const char *name, int actual)
{
    if (!actual) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

int main(void)
{
    /* Four source rows of packed pixels: 0..15. */
    const uint8_t source[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    uint8_t target_bytes[24];
    RedmcsbF0680C25VgaAperturePc34Compat target = {target_bytes, sizeof(target_bytes)};
    RedmcsbF8151BoxPc34Compat box = {1, 2, 1, 2};

    memset(target_bytes, 0xEE, sizeof(target_bytes));
    expect_true("opaque source-to-screen", redmcsb_f8151_vidrv_blit_pc34_compat(
                    source, sizeof(source), &target, &box, 1, 0, 4, 6, -1, 0, 0x10));
    expect_byte("opaque row one x1", target_bytes[7], 0x11);
    expect_byte("opaque row one x2", target_bytes[8], 0x12);
    expect_byte("opaque row two x1", target_bytes[13], 0x15);
    expect_byte("opaque row two x2", target_bytes[14], 0x16);
    expect_byte("opaque outside box", target_bytes[6], 0xEE);

    memset(target_bytes, 0xEE, sizeof(target_bytes));
    /* C25 F0681/F0683 bodies are empty, so horizontal paths do not draw. */
    expect_true("both flips", redmcsb_f8151_vidrv_blit_pc34_compat(
                    source, sizeof(source), &target, &box, 1, 0, 4, 6, -1, 3, 0x00));
    expect_byte("both flips leave target", target_bytes[7], 0xEE);
    expect_byte("both flips leave second target", target_bytes[14], 0xEE);

    memset(target_bytes, 0xEE, sizeof(target_bytes));
    box.left = 0;
    box.right = 3;
    box.top = 0;
    box.bottom = 0;
    expect_true("transparent horizontal", redmcsb_f8151_vidrv_blit_pc34_compat(
                    source, sizeof(source), &target, &box, 0, 0, 4, 6, 0, 1, 0x10));
    expect_byte("transparent flipped target untouched", target_bytes[0], 0xEE);
    expect_byte("transparent flipped target remains untouched", target_bytes[3], 0xEE);

    expect_true("odd stride normalized", redmcsb_f8151_vidrv_blit_pc34_compat(
                    source, sizeof(source), &target, &box, 0, 0, 5, 7, -1, 0, 0x00));
    expect_true("bad flip rejected", !redmcsb_f8151_vidrv_blit_pc34_compat(
                    source, sizeof(source), &target, &box, 0, 0, 4, 6, -1, 4, 0x00));

    if (strstr(redmcsb_f8151_vidrv_blit_source_evidence_pc34(), "NEC816.C:1559-1704") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8151 PC 3.4 C25 VGA blit");
    return 0;
}
