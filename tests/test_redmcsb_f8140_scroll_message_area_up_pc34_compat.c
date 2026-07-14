#include "redmcsb_f8140_scroll_message_area_up_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_bytes(const char *name, const uint8_t *actual,
                         const uint8_t *expected, size_t count)
{
    if (memcmp(actual, expected, count) != 0) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

int main(void)
{
    uint8_t down[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t down_expected[9] = {2, 3, 4, 5, 6, 5, 6, 7, 8};
    uint8_t up[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t up_expected[9] = {0, 1, 0, 1, 2, 3, 4, 7, 8};
    uint8_t same[5] = {9, 8, 7, 6, 5};
    uint8_t same_expected[5] = {9, 8, 7, 6, 5};
    uint8_t untouched[3] = {4, 5, 6};
    uint8_t untouched_expected[3] = {4, 5, 6};

    /* source > destination: NEC816.C's low-to-high branch. */
    redmcsb_f8140_scroll_message_area_up_pc34_compat(down + 2, down, 5);
    expect_bytes("source above destination", down, down_expected, sizeof(down));

    /* source <= destination: NEC816.C decrements both endpoints first. */
    redmcsb_f8140_scroll_message_area_up_pc34_compat(up, up + 2, 5);
    expect_bytes("source below destination", up, up_expected, sizeof(up));

    redmcsb_f8140_scroll_message_area_up_pc34_compat(same, same, sizeof(same));
    expect_bytes("same start", same, same_expected, sizeof(same));

    redmcsb_f8140_scroll_message_area_up_pc34_compat(NULL, NULL, 0U);
    expect_bytes("zero count", untouched, untouched_expected, sizeof(untouched));

    if (strstr(redmcsb_f8140_scroll_message_area_up_source_evidence_pc34(),
               "NEC816.C:463-499") == NULL) {
        fprintf(stderr, "FAIL: source evidence\n");
        ++failures;
    }

    if (failures != 0) {
        return 1;
    }
    puts("PASSED: ReDMCSB F8140 overlap-safe message-area copy");
    return 0;
}
