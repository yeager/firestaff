#include "dm1_v2_startup_title_filter_handoff_pc34.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    DM1_V2_StartupTitleFilterHandoffReceiptPc34 receipt;

    memset(&receipt, 0, sizeof(receipt));
    expect(dm1_v2_startup_title_filter_handoff_pc34(
               "dm1", 1, 1, 1, &receipt) == 1 &&
               receipt.handled == 1 && receipt.filters_active == 1 &&
               receipt.source_palette_preserved == 1 &&
               receipt.source_timing_consumed == 1,
           "DM1 V2.0 admits filters only after original title palette and timing facts");

    expect(dm1_v2_startup_title_filter_handoff_pc34(
               "dm1", 0, 1, 1, &receipt) == 0 &&
               receipt.filters_active == 0,
           "V1 title remains unfiltered");
    expect(dm1_v2_startup_title_filter_handoff_pc34(
               "csb", 1, 1, 1, &receipt) == 0 &&
               receipt.filters_active == 0,
           "non-DM1 title cannot borrow DM1 V2.0 filters");
    expect(dm1_v2_startup_title_filter_handoff_pc34(
               "dm1", 1, 0, 1, &receipt) == 0 &&
               receipt.filters_active == 0,
           "missing original timing blocks title filter handoff");
    expect(dm1_v2_startup_title_filter_handoff_pc34(
               "dm1", 1, 1, 0, &receipt) == 0 &&
               receipt.filters_active == 0,
           "missing original palette blocks title filter handoff");
    expect(dm1_v2_startup_title_filter_handoff_pc34(
               "dm1", 1, 1, 1, NULL) == 0,
           "null receipt is rejected");

    if (!failures) {
        puts("dm1_v2_startup_title_filter_handoff_pc34: ok");
    }
    return failures;
}
