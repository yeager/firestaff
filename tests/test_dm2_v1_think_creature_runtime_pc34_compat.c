/*
 * DM2_THINK_CREATURE production boundary.
 *
 * A 0x21/0x22 timer is source-shaped, but it is not sufficient to create a
 * creature runtime.  SKProject c_tim_proc.cpp dispatches it only after
 * GAME_LOAD has retained c_map, c_record, CAII, CCM and c_tim together.
 * This test deliberately supplies no original world and proves the public
 * legacy runtime cannot promote that timer into a fabricated session.
 */

#include "dm2_v1_runtime.h"
#include "dm2_v1_think_creature_pc34_compat.h"

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
    DM2_V1_SourceTimer timer;
    DM2_V1_ThinkCreatureReceipt think;
    DM2_V1_ProceedTimersReceipt proceed;

    memset(&boot, 0, sizeof(boot));
    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = 1u;
    timer.type = DM2_V1_TIMER_THINK_CREATURE_A;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);
    CHECK(dm2_v1_runtime_enqueue_source_timer(&timer, 0) ==
              DM2_V1_SOURCE_TIMER_OK,
          "source-shaped think timer can enter the private queue");
    dm2_v1_runtime_tick();

    CHECK(!dm2_v1_runtime_record_pools_valid() &&
              !dm2_v1_runtime_think_creature_receipt(&think),
          "timer cannot invent record pools or a THINK_CREATURE receipt");
    CHECK(!dm2_v1_runtime_last_proceed_timers_receipt(&proceed),
          "an unowned timer cannot advance the GAME_LOAD tick or dispatch a creature");

    printf("DM2 THINK_CREATURE production boundary: %d passed, %d failed\n",
           passed, failed);
    return failed == 0 ? 0 : 1;
}
