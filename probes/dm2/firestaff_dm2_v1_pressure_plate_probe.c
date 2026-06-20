/*
 * firestaff_dm2_v1_pressure_plate_probe.c — DM2 V1 Pressure Plate Probe
 *
 * Exercises the dm2_v1_pressure_plate.c API against the built-in plate
 * catalog.  Reports actual plate IDs, kinds, and a sample weight +
 * item + time + party fire sequence.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_sensor.cpp       - pressure plate logic
 *   skproject/SKWIN/SkGlobal.cpp:1112-1170 - dPressurePlatesTable
 *   skproject/SKWIN/DME.h:1456-1504       - pressure_plate_descriptor_t
 *   ReDMCSB MOVESENS.C:1000-1100          - DM1 sensor parity
 */

#include "dm2_v1_pressure_plate.h"
#include "dm2_v1_door_mechanics.h"

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

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    fprintf(stderr, "=== DM2 V1 Pressure Plate Verification Probe ===\n");
    fprintf(stderr, "Source: skproject/SKULLWIN/c_sensor.cpp (plate logic)\n");
    fprintf(stderr, "        skproject/SKWIN/SkGlobal.cpp:1112-1170 (dPressurePlatesTable)\n");
    fprintf(stderr, "        ReDMCSB MOVESENS.C:1000-1100 (DM1 sensor parity)\n\n");

    dm2_v1_plate_reset_state();

    /* Invariant 1: built-in plate count matches DM2_PLATE_NUM_BUILTIN. */
    PROBE_ASSERT(dm2_v1_plate_get_builtin_count() == DM2_PLATE_NUM_BUILTIN,
                 "builtin plate count = %d (expected %d)",
                 dm2_v1_plate_get_builtin_count(), DM2_PLATE_NUM_BUILTIN);

    /* Invariant 2: dump all 5 plates with kind/target/threshold info. */
    for (int i = 1; i <= DM2_PLATE_NUM_BUILTIN; i++) {
        const DM2_V1_PressurePlate *p = dm2_v1_plate_get_builtin(i);
        if (p) {
            const char *kind_s = "?";
            switch (p->kind) {
                case DM2_PLATE_KIND_WEIGHT:    kind_s = "WEIGHT"; break;
                case DM2_PLATE_KIND_ITEM:      kind_s = "ITEM"; break;
                case DM2_PLATE_KIND_TIME:      kind_s = "TIME"; break;
                case DM2_PLATE_KIND_PARTY:     kind_s = "PARTY"; break;
                case DM2_PLATE_KIND_CREATURE:  kind_s = "CREATURE"; break;
            }
            const char *tgt_s = "?";
            switch (p->target_kind) {
                case DM2_PLATE_TARGET_DOOR_TOGGLE:     tgt_s = "DOOR_TOGGLE"; break;
                case DM2_PLATE_TARGET_DOOR_OPEN:       tgt_s = "DOOR_OPEN"; break;
                case DM2_PLATE_TARGET_DOOR_CLOSE:      tgt_s = "DOOR_CLOSE"; break;
                case DM2_PLATE_TARGET_PIT_TOGGLE:      tgt_s = "PIT_TOGGLE"; break;
                case DM2_PLATE_TARGET_MESSAGE:         tgt_s = "MESSAGE"; break;
                case DM2_PLATE_TARGET_CREATURE_SPAWN:  tgt_s = "CREATURE_SPAWN"; break;
            }
            fprintf(stderr, "  Plate %d [%s] at (lvl %d, %d,%d) target=%s threshold=%d\n",
                    i, kind_s, p->map_level, p->map_x, p->map_y,
                    tgt_s, p->weight_threshold);
        }
    }
    PROBE_ASSERT(dm2_v1_plate_get_builtin(1) != NULL
              && dm2_v1_plate_get_builtin(2) != NULL
              && dm2_v1_plate_get_builtin(3) != NULL
              && dm2_v1_plate_get_builtin(4) != NULL
              && dm2_v1_plate_get_builtin(5) != NULL,
                 "all 5 built-in plates retrievable by ID");

    /* Invariant 3: WEIGHT plate fires when weight >= threshold. */
    dm2_v1_plate_set_party_weight(500);  /* threshold=300 */
    int rc = dm2_v1_plate_check(1, 1000);
    PROBE_ASSERT(rc == (int)DM2_PLATE_RESULT_OK,
                 "WEIGHT plate fires when weight 500 >= threshold 300 (rc=%d)", rc);
    PROBE_ASSERT(dm2_v1_plate_get_fire_count(1) == 1,
                 "fire_count after WEIGHT plate fire = 1 (got %d)",
                 dm2_v1_plate_get_fire_count(1));

    /* Invariant 4: ITEM plate fires when correct item on plate. */
    dm2_v1_plate_reset_state();
    dm2_v1_plate_set_item_on_floor(111, 5, 12, 0);  /* plate 2 wants 111 at (5,12,0) */
    rc = dm2_v1_plate_check(2, 2000);
    PROBE_ASSERT(rc == (int)DM2_PLATE_RESULT_OK,
                 "ITEM plate fires when item 111 placed on plate (rc=%d)", rc);
    /* Should be fire_once: second call must fail with ALREADY_FIRED. */
    rc = dm2_v1_plate_check(2, 3000);
    PROBE_ASSERT(rc == (int)DM2_PLATE_RESULT_ALREADY_FIRED,
                 "fire_once plate returns ALREADY_FIRED on 2nd call (rc=%d)", rc);

    /* Invariant 5: TIME plate fires immediately + re-fires after period. */
    dm2_v1_plate_reset_state();
    rc = dm2_v1_plate_check(3, 0);  /* first call: immediate */
    PROBE_ASSERT(rc == (int)DM2_PLATE_RESULT_OK,
                 "TIME plate first call fires immediately (rc=%d)", rc);
    rc = dm2_v1_plate_check(3, 2000);  /* 2s, period=5s */
    PROBE_ASSERT(rc == (int)DM2_PLATE_RESULT_NOT_TIME_YET,
                 "TIME plate rejects re-fire before period elapses (rc=%d)", rc);
    rc = dm2_v1_plate_check(3, 6000);  /* 6s, > 5s period */
    PROBE_ASSERT(rc == (int)DM2_PLATE_RESULT_OK,
                 "TIME plate re-fires after period elapses (rc=%d)", rc);

    /* Invariant 6: PARTY plate fires when party on plate. */
    dm2_v1_plate_reset_state();
    dm2_v1_plate_set_party_position(7, 7, 1);  /* plate 4 */
    rc = dm2_v1_plate_check(4, 1000);
    PROBE_ASSERT(rc == (int)DM2_PLATE_RESULT_OK,
                 "PARTY plate fires when party is on plate (rc=%d)", rc);

    /* Invariant 7: Door state after fire. */
    int ds = dm2_v1_plate_get_door_state_after_fire(1);
    PROBE_ASSERT(ds == DM2_DOOR_STATE_OPEN,
                 "DOOR_TOGGLE target → door state OPEN after fire (got %d)", ds);

    /* Invariant 8: Failure path: weight below threshold. */
    dm2_v1_plate_reset_state();
    dm2_v1_plate_set_party_weight(50);
    rc = dm2_v1_plate_check(1, 0);
    PROBE_ASSERT(rc == (int)DM2_PLATE_RESULT_INSUFFICIENT_WEIGHT,
                 "WEIGHT plate rejects weight 50 < threshold 300 (rc=%d)", rc);

    /* Invariant 9: Failure path: wrong item. */
    dm2_v1_plate_reset_state();
    dm2_v1_plate_set_item_on_floor(999, 5, 12, 0);
    rc = dm2_v1_plate_check(2, 0);
    PROBE_ASSERT(rc == (int)DM2_PLATE_RESULT_WRONG_ITEM,
                 "ITEM plate rejects wrong item id (rc=%d)", rc);

    /* Invariant 10: One-way plate resets on party leave. */
    dm2_v1_plate_reset_state();
    dm2_v1_plate_set_party_position(7, 7, 1);
    dm2_v1_plate_check(4, 1000);
    int state_before_leave = dm2_v1_plate_get_state_for(4);
    dm2_v1_plate_set_party_position(0, 0, 0);  /* leave */
    int state_after_leave = dm2_v1_plate_get_state_for(4);
    fprintf(stderr, "  one-way plate: state before leave=%d, after leave=%d\n",
            state_before_leave, state_after_leave);
    PROBE_ASSERT(state_before_leave == 1 && state_after_leave == 0,
                 "one-way PARTY plate resets on party leave");

    /* Invariant 11: Target message for TIME plate. */
    const char *msg = dm2_v1_plate_get_target_message(3);
    PROBE_ASSERT(msg != NULL && msg[0] != '\0',
                 "TIME plate has non-empty target message (got '%s')",
                 msg ? msg : "(null)");

    /* Invariant 12: Source evidence mentions skproject. */
    const char *e = dm2_v1_pressure_plate_source_evidence();
    PROBE_ASSERT(e != NULL && strstr(e, "skproject/SKULLWIN/c_sensor.cpp") != NULL
              && strstr(e, "dPressurePlatesTable") != NULL,
                 "source evidence mentions skproject + dPressurePlatesTable");

    /* Invariant 13: Force-fire bypasses condition check. */
    dm2_v1_plate_reset_state();
    dm2_v1_plate_set_party_weight(0);
    rc = dm2_v1_plate_force_fire(1);
    PROBE_ASSERT(rc == (int)DM2_PLATE_RESULT_OK && dm2_v1_plate_get_fire_count(1) == 1,
                 "force_fire bypasses weight check (rc=%d, fire_count=%d)",
                 rc, dm2_v1_plate_get_fire_count(1));

    fprintf(stderr, "\n=== Summary ===\n");
    fprintf(stderr, "Plates verified: %d (weight/item/time/party/creature)\n",
            dm2_v1_plate_get_builtin_count());
    fprintf(stderr, "Total fires observed this run: %d\n", dm2_v1_plate_fire_total());
    fprintf(stderr, "\n%d/%d invariants PASS\n", passed, passed + errors);
    return (errors == 0) ? 0 : 1;
}
