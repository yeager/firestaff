/* DM2_1c9a_0cf7 production boundary.  No caller-authored dungeon may create
 * a CAII timer chain; source: SKProject c_1c9a.cpp:5695-5728. */

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

    memset(&boot, 0, sizeof(boot));
    memset(&receipt, 0, sizeof(receipt));
    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);

    CHECK(!dm2_v1_runtime_schedule_creature_at(0, 0, 0, &receipt) &&
              !receipt.valid && !receipt.enqueued,
          "DM2_1c9a_0cf7 rejects a creature without a source GAME_LOAD owner");
    CHECK(!dm2_v1_runtime_record_pools_valid(),
          "producer cannot materialize caller-authored record pools");

    printf("DM2 creature scheduling production boundary: %d passed, %d failed\n",
           passed, failed);
    return failed == 0 ? 0 : 1;
}
