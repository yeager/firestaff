/* DM2_1c9a_0db0 production boundary.  Source c_1c9a.cpp:5734-5763 may only
 * replace a timer belonging to an authenticated CAII slot. */

#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void)
{
    DM2_V1_BootProfile boot;
    DM2_V1_CreatureScheduleReceipt receipt;
    DM2_V1_ThinkCreatureReceipt think;

    memset(&boot, 0, sizeof(boot));
    memset(&receipt, 0, sizeof(receipt));
    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);
    CHECK(dm2_v1_runtime_caii_init(4),
          "private CAII storage exists without a live creature");
    CHECK(!dm2_v1_runtime_reschedule_creature_at(0, 0, &receipt) &&
              !receipt.valid && !receipt.enqueued && !receipt.replaced_existing,
          "reschedule cannot manufacture a timer for an unowned creature");
    dm2_v1_runtime_tick();
    CHECK(!dm2_v1_runtime_think_creature_receipt(&think),
          "rejected reschedule publishes no THINK_CREATURE work");

    printf("DM2 CAII reschedule production boundary: %d passed, %d failed\n",
           passed, failed);
    return failed == 0 ? 0 : 1;
}
