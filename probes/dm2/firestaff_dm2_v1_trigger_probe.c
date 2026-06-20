/*
 * firestaff_dm2_v1_trigger_probe.c — DM2 V1 Trigger System Probe
 *
 * Exercises the dm2_v1_trigger.c API: 8 built-in triggers, 4 kinds,
 * 6 targets.  Reports fire counts, signal counts, and a sample
 * square-entered + item-used + time-elapsed + combat-ended sequence.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_trigger.cpp        - trigger system core
 *   skproject/SKWIN/DME.h:1700-1780          - trigger_descriptor_t
 *   skproject/SKWIN/SkGlobal.cpp:1212-1280   - dTriggersTable
 *   ReDMCSB TIMELINE.C:43-220                - DM1 timeline events
 */

#include "dm2_v1_trigger.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int errors = 0;
static int passed = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        passed++; \
    } \
} while (0)

static const char *kind_str(int k) {
    switch (k) {
        case DM2_TRIGGER_KIND_SQUARE_ENTERED: return "SQUARE_ENTERED";
        case DM2_TRIGGER_KIND_ITEM_USED:      return "ITEM_USED";
        case DM2_TRIGGER_KIND_TIME_ELAPSED:   return "TIME_ELAPSED";
        case DM2_TRIGGER_KIND_COMBAT_ENDED:   return "COMBAT_ENDED";
        default: return "?";
    }
}

static const char *target_str(int t) {
    switch (t) {
        case DM2_TRIGGER_TARGET_DOOR_TOGGLE:    return "DOOR_TOGGLE";
        case DM2_TRIGGER_TARGET_DOOR_OPEN:      return "DOOR_OPEN";
        case DM2_TRIGGER_TARGET_DOOR_CLOSE:     return "DOOR_CLOSE";
        case DM2_TRIGGER_TARGET_SPAWN_CREATURE: return "SPAWN_CREATURE";
        case DM2_TRIGGER_TARGET_DISPLAY_MSG:    return "DISPLAY_MSG";
        case DM2_TRIGGER_TARGET_TELEPORT_PARTY: return "TELEPORT_PARTY";
        default: return "?";
    }
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    fprintf(stderr, "=== DM2 V1 Trigger System Verification Probe ===\n");
    fprintf(stderr, "Source: skproject/SKULLWIN/c_trigger.cpp (trigger core)\n");
    fprintf(stderr, "        skproject/SKWIN/SkGlobal.cpp:1212-1280 (dTriggersTable)\n");
    fprintf(stderr, "        ReDMCSB TIMELINE.C:43-220 (DM1 timeline events)\n\n");

    dm2_v1_trigger_reset_state();

    /* Invariant 1: built-in count. */
    PROBE_ASSERT(dm2_v1_trigger_get_builtin_count() == DM2_TRIGGER_NUM_BUILTIN,
                 "builtin trigger count = %d (expected %d)",
                 dm2_v1_trigger_get_builtin_count(), DM2_TRIGGER_NUM_BUILTIN);

    /* Invariant 2: dump all 8 triggers with kind/target/args. */
    for (int i = 1; i <= DM2_TRIGGER_NUM_BUILTIN; i++) {
        const DM2_V1_Trigger *t = dm2_v1_trigger_get_builtin(i);
        if (t) {
            fprintf(stderr, "  Trigger %d [%s -> %s] fire_once=%d args=(item=%d time=%dms)\n",
                    i, kind_str(t->kind), target_str(t->target),
                    t->fire_once, t->arg_item_id, t->arg_time_ms);
        }
    }
    PROBE_ASSERT(dm2_v1_trigger_get_builtin(1) != NULL
              && dm2_v1_trigger_get_builtin(8) != NULL,
                 "all 8 built-in triggers retrievable");

    /* Invariant 3: SQUARE_ENTERED trigger fires on matching coords. */
    int fired = dm2_v1_trigger_signal_square_entered(15, 8, 0);
    PROBE_ASSERT(fired == 1 && dm2_v1_trigger_get_fire_count(1) == 1,
                 "SQUARE_ENTERED (15,8,0) fires trigger 1 (fired=%d, count=%d)",
                 fired, dm2_v1_trigger_get_fire_count(1));

    /* Invariant 4: SQUARE_ENTERED on wrong coords does NOT fire. */
    fired = dm2_v1_trigger_signal_square_entered(0, 0, 99);
    PROBE_ASSERT(fired == 0, "SQUARE_ENTERED (0,0,99) fires nothing (fired=%d)", fired);

    /* Invariant 5: ITEM_USED trigger fires on matching item. */
    fired = dm2_v1_trigger_signal_item_used(1001);
    PROBE_ASSERT(fired >= 1 && dm2_v1_trigger_get_fire_count(3) == 1,
                 "ITEM_USED 1001 fires trigger 3 (fired=%d, count=%d)",
                 fired, dm2_v1_trigger_get_fire_count(3));

    /* Invariant 6: COMBAT_ENDED fires both triggers 7 and 8. */
    fired = dm2_v1_trigger_signal_combat_ended(1);
    PROBE_ASSERT(fired == 2,
                 "COMBAT_ENDED fires triggers 7 + 8 (fired=%d)", fired);

    /* Invariant 7: TIME_ELAPSED periodic trigger fires. */
    fired = dm2_v1_trigger_tick(30000);
    PROBE_ASSERT(fired >= 1 && dm2_v1_trigger_get_fire_count(6) == 1,
                 "TIME_ELAPSED t=30s fires trigger 6 (fired=%d, count=%d)",
                 fired, dm2_v1_trigger_get_fire_count(6));

    /* Invariant 8: TIME_ELAPSED fire_once trigger. */
    dm2_v1_trigger_reset_state();
    fired = dm2_v1_trigger_tick(60000);
    PROBE_ASSERT(fired >= 1 && dm2_v1_trigger_get_fire_count(5) == 1,
                 "TIME_ELAPSED t=60s fires fire_once trigger 5 (fired=%d, count=%d)",
                 fired, dm2_v1_trigger_get_fire_count(5));
    fired = dm2_v1_trigger_tick(120000);
    PROBE_ASSERT(dm2_v1_trigger_get_fire_count(5) == 1,
                 "fire_once trigger 5 does NOT re-fire at t=120s (count=%d)",
                 dm2_v1_trigger_get_fire_count(5));

    /* Invariant 9: periodic re-fires. */
    dm2_v1_trigger_reset_state();
    dm2_v1_trigger_tick(30000);
    dm2_v1_trigger_tick(60000);
    dm2_v1_trigger_tick(90000);
    PROBE_ASSERT(dm2_v1_trigger_get_fire_count(6) == 3,
                 "periodic trigger 6 fires 3 times in 3 ticks (count=%d)",
                 dm2_v1_trigger_get_fire_count(6));

    /* Invariant 10: signal counts. */
    int sig_before = dm2_v1_trigger_total_signals();
    dm2_v1_trigger_signal_item_used(9999);  /* no match */
    PROBE_ASSERT(dm2_v1_trigger_total_signals() == sig_before + 1,
                 "total_signals increments even when no trigger fires");

    /* Invariant 11: fire() on unknown returns NOT_FOUND. */
    int rc = dm2_v1_trigger_fire(99);
    PROBE_ASSERT(rc == (int)DM2_TRIGGER_RESULT_NOT_FOUND,
                 "fire(99) returns NOT_FOUND (rc=%d)", rc);

    /* Invariant 12: source evidence. */
    const char *e = dm2_v1_trigger_source_evidence();
    PROBE_ASSERT(e != NULL && strstr(e, "skproject/SKULLWIN/c_trigger.cpp") != NULL
              && strstr(e, "TIMELINE.C") != NULL,
                 "source evidence mentions skproject + ReDMCSB TIMELINE.C");

    /* Invariant 13: explicit fire() works. */
    dm2_v1_trigger_reset_state();
    rc = dm2_v1_trigger_fire(2);
    PROBE_ASSERT(rc == (int)DM2_TRIGGER_RESULT_OK
              && dm2_v1_trigger_get_fire_count(2) == 1,
                 "explicit fire(2) succeeds (rc=%d, count=%d)",
                 rc, dm2_v1_trigger_get_fire_count(2));

    fprintf(stderr, "\n=== Summary ===\n");
    fprintf(stderr, "Triggers verified: %d (4 kinds x 2 each)\n",
            dm2_v1_trigger_get_builtin_count());
    fprintf(stderr, "Total fires this run: %d\n", dm2_v1_trigger_total_fires());
    fprintf(stderr, "Total signals this run: %d\n", dm2_v1_trigger_total_signals());
    fprintf(stderr, "\n%d/%d invariants PASS\n", passed, passed + errors);
    return (errors == 0) ? 0 : 1;
}
