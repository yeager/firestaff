/* CSBWin ITEM16 original-save atomic runtime handoff regression.
 * Source: CSBWin SaveGame.cpp:1168-1176, 1815-1821; CSB.h:2257-2280. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;

    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_item16_summary_count = 2u;
    profile.csbwin_item16_summary_total = 2u;
    profile.csbwin_item16[0].valid = 1;
    profile.csbwin_item16[0].monster_index = 0x1234u;
    profile.csbwin_item16[0].facings = 0x20u;
    profile.csbwin_item16[0].current_x = 12u;
    profile.csbwin_item16[0].current_y = 7u;
    profile.csbwin_item16[1].valid = 1;
    profile.csbwin_item16[1].monster_index = 0xffffu;

    check(csb_v1_runtime_materialize_csbwin_item16_summaries(&profile) == 1 &&
              profile.csbwin_runtime_item16_count == 1u &&
              profile.csbwin_runtime_item16_total == 2u &&
              profile.csbwin_runtime_item16[0].monster_index == 0x1234u,
          "CSBWin restore publishes its complete ITEM16 table atomically");

    profile.csbwin_item16_summary_total = 3u;
    check(csb_v1_runtime_materialize_csbwin_item16_summaries(&profile) < 0 &&
              profile.csbwin_runtime_item16_count == 1u &&
              profile.csbwin_runtime_item16[0].monster_index == 0x1234u,
          "truncated CSBWin ITEM16 summary cannot replace live AI state");

    profile.csbwin_item16_summary_total = 2u;
    profile.csbwin_item16[1].valid = 0;
    check(csb_v1_runtime_materialize_csbwin_item16_summaries(&profile) < 0 &&
              profile.csbwin_runtime_item16_count == 1u &&
              profile.csbwin_runtime_item16[0].current_x == 12u,
          "missing CSBWin ITEM16 record cannot partially publish runtime state");

    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
