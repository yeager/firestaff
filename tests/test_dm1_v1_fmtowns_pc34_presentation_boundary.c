#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(const char *name, int condition)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

int main(void)
{
    DM1_V1_StartupSelectedLaunchRouteFacts_PC34 facts;
    DM1_V1_StartupSelectedLaunchRouteReceipt_PC34 receipt;

    memset(&facts, 0, sizeof(facts));
    memset(&receipt, 0, sizeof(receipt));
    facts.selected_game_id = "dm1-fmtowns";
    expect_true("FM Towns route builds",
                dm1_v1_startup_selected_launch_route_receipt_pc34(
                    &facts, &receipt));
    expect_true("FM Towns route is generic until native P3/TBIOS playback",
                receipt.handled && !receipt.use_dm1_transaction &&
                receipt.use_generic_launch &&
                !receipt.requires_source_visible_intro);

    facts.selected_game_id = "dm1";
    expect_true("PC34 route keeps the PC34 presentation transaction",
                dm1_v1_startup_selected_launch_route_receipt_pc34(
                    &facts, &receipt) && receipt.use_dm1_transaction &&
                !receipt.use_generic_launch &&
                receipt.requires_source_visible_intro);

    if (failures != 0) {
        fprintf(stderr, "dm1_v1_fmtowns_pc34_presentation_boundary failures=%d\n",
                failures);
        return 1;
    }
    puts("PASS: FM Towns does not borrow PC34 startup presentation");
    return 0;
}
