/*
 * test_csb_v1_projectile_speed_pc34_compat.c
 *
 * CSB V1 Projectile Speed Normalization (Combat GAP 1,
 * CHANGE7_20).  Source-locked per PROJEXPL.C
 * CHANGE7_20_IMPROVEMENT: "Projectiles now move at full
 * speed on ALL maps."
 */
#include "csb_v1_projectile_speed_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <string.h>

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    struct ProjectileInstance_Compat proj;
    struct TimelineEvent_Compat ev;
    int delay_party_dm1;
    int delay_party_csb;
    int delay_other_dm1;
    int delay_other_csb;

    printf("=== CSB V1 projectile speed normalization (CHANGE7_20) ===\n");

    /* Default: mode disabled (DM1 behaviour). */
    csb_v1_projectile_speed_normalization_set(0);
    CHECK(csb_v1_projectile_speed_normalization_get() == 0,
          "default projectile-speed-normalization is disabled");

    /* Build a minimal valid projectile instance. */
    memset(&proj, 0, sizeof(proj));
    proj.mapIndex = 0;
    proj.mapX     = 1;
    proj.mapY     = 1;
    proj.cell     = 0;

    /* DM1: party map -> delay 1; other map -> delay 3. */
    F0825_PROJECTILE_ScheduleNextMove_Compat(&proj, 1, 100, &ev);
    delay_party_dm1 = (int)(ev.fireAtTick - 100);
    F0825_PROJECTILE_ScheduleNextMove_Compat(&proj, 0, 100, &ev);
    delay_other_dm1 = (int)(ev.fireAtTick - 100);
    CHECK(delay_party_dm1 == 1, "DM1: party map delay = 1");
    CHECK(delay_other_dm1 == 3, "DM1: other map delay = 3");

    /* CSB: both party and other maps -> delay 1. */
    csb_v1_projectile_speed_normalization_set(1);
    CHECK(csb_v1_projectile_speed_normalization_get() == 1,
          "projectile-speed-normalization can be enabled");

    F0825_PROJECTILE_ScheduleNextMove_Compat(&proj, 1, 100, &ev);
    delay_party_csb = (int)(ev.fireAtTick - 100);
    F0825_PROJECTILE_ScheduleNextMove_Compat(&proj, 0, 100, &ev);
    delay_other_csb = (int)(ev.fireAtTick - 100);
    CHECK(delay_party_csb == 1, "CSB: party map delay = 1");
    CHECK(delay_other_csb == 1, "CSB: other map delay = 1 (full speed on ALL maps)");

    /* Toggling back restores DM1 behaviour. */
    csb_v1_projectile_speed_normalization_set(0);
    F0825_PROJECTILE_ScheduleNextMove_Compat(&proj, 0, 100, &ev);
    delay_other_dm1 = (int)(ev.fireAtTick - 100);
    CHECK(delay_other_dm1 == 3, "toggle off restores DM1: other map delay = 3");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
