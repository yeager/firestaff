#include "redmcsb_f8162_vidrv_scroll_message_area_up_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum { kRows = 5, kPlaneBytes = kRows * REDMCSB_F8162_SCREEN_STRIDE_BYTES_PC34 };

static int failures;

static void expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %d, expected %d)\n", name, actual, expected);
        ++failures;
    }
}

int main(void)
{
    uint8_t planes[REDMCSB_F8162_MAX_PLANES_PC34][kPlaneBytes];
    RedmcsbF8162VideoPagesPc34Compat pages;
    RedmcsbF8162BoxPc34Compat box = {0, 1, 15, 4};
    size_t plane;
    size_t row;
    size_t byte_index;

    memset(&pages, 0, sizeof(pages));
    for (plane = 0U; plane < REDMCSB_F8162_MAX_PLANES_PC34; ++plane) {
        pages.planes[plane] = planes[plane];
        for (row = 0U; row < kRows; ++row) {
            memset(planes[plane] + row * REDMCSB_F8162_SCREEN_STRIDE_BYTES_PC34,
                   (int)(plane * 16U + row), REDMCSB_F8162_SCREEN_STRIDE_BYTES_PC34);
        }
    }
    pages.plane_byte_count = kPlaneBytes;
    pages.plane_count = REDMCSB_F8162_MAX_PLANES_PC34;

    expect_int("four-plane scroll", redmcsb_f8162_vidrv_scroll_message_area_up_pc34_compat(
                   &pages, &box, 1U), 1);
    for (plane = 0U; plane < pages.plane_count; ++plane) {
        for (row = 1U; row < 4U; ++row) {
            for (byte_index = 0U; byte_index < 4U; ++byte_index) {
                expect_int("shifted source row",
                           planes[plane][row * REDMCSB_F8162_SCREEN_STRIDE_BYTES_PC34 + byte_index],
                           (int)(plane * 16U + row + 1U));
            }
        }
        /* F8162 never clears the newly exposed final row. */
        expect_int("bottom row preserved",
                   planes[plane][4U * REDMCSB_F8162_SCREEN_STRIDE_BYTES_PC34],
                   (int)(plane * 16U + 4U));
        expect_int("outside width preserved",
                   planes[plane][REDMCSB_F8162_SCREEN_STRIDE_BYTES_PC34 + 4U],
                   (int)(plane * 16U + 1U));
    }

    expect_int("zero scroll rejected", redmcsb_f8162_vidrv_scroll_message_area_up_pc34_compat(
                   &pages, &box, 0U), 0);
    box.right = 700;
    expect_int("too-wide box rejected", redmcsb_f8162_vidrv_scroll_message_area_up_pc34_compat(
                   &pages, &box, 1U), 0);

    if (strstr(redmcsb_f8162_vidrv_scroll_message_area_up_source_evidence_pc34(),
               "NEC816.C:2438-2466") == NULL) {
        fprintf(stderr, "FAIL: source evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8162 multi-plane message-area scroll");
    return 0;
}
