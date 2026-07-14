#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0751_get_bitmap_byte_count_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

static int expect_u16(uint16_t actual, uint16_t expected, const char *label)
{
    if (actual == expected) {
        return 0;
    }

    fprintf(stderr, "%s: expected %u, got %u\n", label, (unsigned)expected,
            (unsigned)actual);
    return 1;
}

int main(void)
{
    const redmcsb_f0751_graphic_width_height_pc34 graphics[] = {
        { 320, 200 },
        { 19, 14 },
        { 1, 29 },
        { 18, 18 }
    };

    if (expect_u16(redmcsb_f0751_get_bitmap_byte_count_pc34_compat(
                       graphics, 0), 32000, "320x200") != 0 ||
        expect_u16(redmcsb_f0751_get_bitmap_byte_count_pc34_compat(
                       graphics, 1), 140, "19x14 rounds to 20x14") != 0 ||
        expect_u16(redmcsb_f0751_get_bitmap_byte_count_pc34_compat(
                       graphics, 2), 29, "1x29") != 0 ||
        expect_u16(redmcsb_f0751_get_bitmap_byte_count_pc34_compat(
                       graphics, 3), 162, "18x18") != 0 ||
        strstr(redmcsb_f0751_get_bitmap_byte_count_source_evidence_pc34(),
               "STARTUP2.C:465-472") == NULL) {
        return 1;
    }

    puts("ok: ReDMCSB F0751 PC 3.4 graphic byte count");
    return 0;
}
