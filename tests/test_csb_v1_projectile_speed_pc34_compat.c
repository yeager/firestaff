/*
 * test_csb_v1_projectile_speed_pc34_compat.c
 *
 * CSB V1 Projectile Speed Normalization (Combat GAP 1,
 * CHANGE7_20). Source-locked per ReDMCSB PROJEXPL.C
 * F0219 lines 755-760: DM1/older media add +1 on the party map
 * and +3 elsewhere; CSB MEDIA265_S20E/S21E increments by +1 on
 * every map.
 */
#include "csb_v1_projectile_speed_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <stdint.h>
#include <string.h>

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(got, want, msg) do { \
    int got_value = (int)(got); \
    int want_value = (int)(want); \
    if (got_value == want_value) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else { ++g_fail; printf("  FAIL: %s got=%d want=%d\n", msg, got_value, want_value); } \
} while (0)

static void make_projectile(struct ProjectileInstance_Compat *proj,
                            int slotIndex,
                            int mapIndex,
                            int cell)
{
    memset(proj, 0, sizeof(*proj));
    proj->slotIndex = slotIndex;
    proj->projectileSubtype = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    proj->ownerKind = PROJECTILE_OWNER_CREATURE;
    proj->ownerIndex = 3;
    proj->mapIndex = mapIndex;
    proj->mapX = 12;
    proj->mapY = 7;
    proj->cell = cell;
}

static int schedule_delay(const struct ProjectileInstance_Compat *proj,
                          int onPartyMap,
                          uint32_t currentTick,
                          struct TimelineEvent_Compat *ev)
{
    if (!F0825_PROJECTILE_ScheduleNextMove_Compat(
            proj, onPartyMap, currentTick, ev)) {
        return -1;
    }
    return (int)(ev->fireAtTick - currentTick);
}

static void test_invalid_inputs_do_not_schedule(void)
{
    struct TimelineEvent_Compat ev;
    unsigned char sentinel[sizeof(ev)];

    memset(&ev, 0xA5, sizeof(ev));
    memset(sentinel, 0xA5, sizeof(sentinel));
    CHECK_EQ(F0825_PROJECTILE_ScheduleNextMove_Compat(NULL, 1, 100, &ev), 0,
             "NULL projectile is rejected");
    CHECK(memcmp(&ev, sentinel, sizeof(ev)) == 0,
          "NULL projectile rejection leaves caller event untouched");
    CHECK_EQ(F0825_PROJECTILE_ScheduleNextMove_Compat(NULL, 1, 100, NULL), 0,
             "NULL projectile and NULL output event are rejected");
}

static void test_dm1_vs_csb_delay_matrix(void)
{
    struct ProjectileInstance_Compat proj;
    struct ProjectileInstance_Compat deep_map_proj;
    struct TimelineEvent_Compat ev;

    make_projectile(&proj, 9, 0, 0);
    make_projectile(&deep_map_proj, 10, 23, 1);

    /* ReDMCSB PROJEXPL.C F0219:755-760:
     *   DM1 media branch: Map_Time += party-map ? 1 : 3
     *   CSB media branch: Map_Time++
     * Firestaff passes the source party-map comparison in as `onPartyMap`
     * and gates CSB's all-map speed with csb_v1_projectile_speed_normalization. */
    /* Default: mode disabled (DM1 behaviour). */
    csb_v1_projectile_speed_normalization_set(0);
    CHECK_EQ(csb_v1_projectile_speed_normalization_get(), 0,
             "projectile-speed-normalization starts disabled for the DM1 path");

    /* DM1: party map -> delay 1; other map -> delay 3. */
    CHECK_EQ(schedule_delay(&proj, 1, 100, &ev), 1,
             "DM1 branch keeps party-map projectile delay at +1 tick");
    CHECK_EQ(schedule_delay(&proj, 0, 100, &ev), 3,
             "DM1 branch keeps non-party-map projectile delay at +3 ticks");
    CHECK_EQ(schedule_delay(&deep_map_proj, 0, 200, &ev), 3,
             "DM1 branch slowdown applies on any caller-marked non-party map");

    /* CSB: both party and other maps -> delay 1. */
    csb_v1_projectile_speed_normalization_set(1);
    CHECK_EQ(csb_v1_projectile_speed_normalization_get(), 1,
             "projectile-speed-normalization can be enabled for CSB");

    CHECK_EQ(schedule_delay(&proj, 1, 100, &ev), 1,
             "CSB branch keeps party-map projectile delay at +1 tick");
    CHECK_EQ(schedule_delay(&proj, 0, 100, &ev), 1,
             "CSB branch normalizes non-party-map projectile delay to +1 tick");
    CHECK_EQ(schedule_delay(&deep_map_proj, 0, 200, &ev), 1,
             "CSB branch normalizes a deep-map projectile to full speed");

    /* Toggling back restores DM1 behaviour. */
    csb_v1_projectile_speed_normalization_set(0);
    CHECK_EQ(schedule_delay(&proj, 0, 100, &ev), 3,
             "toggle off restores DM1 non-party-map delay");
}

static void test_event_payload_is_stable(void)
{
    struct ProjectileInstance_Compat proj;
    struct TimelineEvent_Compat ev;

    make_projectile(&proj, 17, 12, 7);
    csb_v1_projectile_speed_normalization_set(1);
    CHECK_EQ(schedule_delay(&proj, 0, 400, &ev), 1,
             "payload fixture schedules through the CSB full-speed branch");

    CHECK_EQ(ev.kind, TIMELINE_EVENT_PROJECTILE_MOVE,
             "scheduled event kind is PROJECTILE_MOVE");
    CHECK_EQ(ev.fireAtTick, 401,
             "scheduled event fire tick advances by the normalized delay");
    CHECK_EQ(ev.mapIndex, 12,
             "scheduled event preserves projectile map index");
    CHECK_EQ(ev.mapX, 12,
             "scheduled event preserves projectile X");
    CHECK_EQ(ev.mapY, 7,
             "scheduled event preserves projectile Y");
    CHECK_EQ(ev.cell, 3,
             "scheduled event masks the projectile cell to a quarter-cell");
    CHECK_EQ(ev.aux0, 17,
             "scheduled event preserves projectile slot in aux0");
    CHECK_EQ(ev.aux1, PROJECTILE_OWNER_CREATURE,
             "scheduled event preserves owner kind in aux1");
    CHECK_EQ(ev.aux2, 3,
             "scheduled event preserves owner index in aux2");
    CHECK_EQ(ev.aux3, PROJECTILE_SUBTYPE_LIGHTNING_BOLT,
             "scheduled event preserves projectile subtype in aux3");
    CHECK_EQ(ev.aux4, 0,
             "scheduled event leaves aux4 clear");
}

static void test_mode_setter_normalizes_inputs(void)
{
    csb_v1_projectile_speed_normalization_set(42);
    CHECK_EQ(csb_v1_projectile_speed_normalization_get(), 1,
             "non-zero mode setter input normalizes to enabled");
    csb_v1_projectile_speed_normalization_set(0);
    CHECK_EQ(csb_v1_projectile_speed_normalization_get(), 0,
             "zero mode setter input normalizes to disabled");
    csb_v1_projectile_speed_normalization_set(-9);
    CHECK_EQ(csb_v1_projectile_speed_normalization_get(), 1,
             "negative non-zero mode setter input also normalizes to enabled");
    csb_v1_projectile_speed_normalization_set(0);
}

int main(void) {
    printf("=== CSB V1 projectile speed normalization (CHANGE7_20) ===\n");

    test_invalid_inputs_do_not_schedule();
    test_dm1_vs_csb_delay_matrix();
    test_event_payload_is_stable();
    test_mode_setter_normalizes_inputs();

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
