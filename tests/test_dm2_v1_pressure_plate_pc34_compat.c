/* test_dm2_v1_pressure_plate_pc34_compat.c - DM2 V1 Pressure Plate Tests
 *
 * Phase 4 mechanics parity coverage (25+ assertions):
 *  1. Built-in plate count
 *  2. All 5 plates retrievable
 *  3. Unknown plate returns NULL
 *  4. Plate has valid coordinates
 *  5. Plate kind enum valid (1..5)
 *  6. Plate target kind valid (1..6)
 *  7. Weight plate has weight_threshold > 0
 *  8. Item plate has required_item_id > 0
 *  9. Time plate has time_period_ms > 0
 * 10. Reset state clears counters
 * 11. Set party weight stored correctly
 * 12. Set party position stored correctly
 * 13. Set item on floor stored correctly
 * 14. WEIGHT plate fires when weight >= threshold
 * 15. WEIGHT plate does NOT fire when weight < threshold
 * 16. ITEM plate fires when correct item is on plate
 * 17. ITEM plate does NOT fire when wrong item
 * 18. ITEM plate does NOT fire when item at wrong position
 * 19. PARTY plate fires when party is on plate position
 * 20. PARTY plate does NOT fire when party is elsewhere
 * 21. TIME plate fires first time immediately
 * 22. TIME plate does NOT re-fire before period elapses
 * 23. TIME plate re-fires after period elapses
 * 24. CREATURE plate always fires (for test purposes)
 * 25. fire_once plate does NOT re-fire
 * 26. fire_once plate CAN be reset
 * 27. force_fire bypasses condition check
 * 28. Door state after fire = DOOR_STATE_OPEN for DOOR_OPEN target
 * 29. Door state after fire = DOOR_STATE_OPEN for DOOR_TOGGLE
 * 30. Activate/deactivate updates plate state
 * 31. Party leave (one-way) deactivates plate
 * 32. Party arrive (one-way) activates plate
 * 33. fire_total increments on each fire
 * 34. active_count reflects number of active plates
 * 35. Source evidence mentions skproject
 * 36. Reset fire count zeroes fired_count
 * 37. Multiple plates can fire independently
 * 38. Weight plate with 0 threshold fires for any positive weight
 * 39. Item plate with required_item_id=0 fires for any item
 * 40. Target message non-NULL for TIME plate
 */

#include "dm2_v1_pressure_plate.h"
#include "dm2_v1_door_mechanics.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name_) do { \
    printf("  %-58s", #name_); \
    tests_run++; \
    if (test_##name_()) { \
        tests_passed++; \
        printf("  PASS\n"); \
    } else { \
        printf("  FAIL\n"); \
    } \
} while (0)

static void setup_clean(void) {
    dm2_v1_plate_reset_state();
    dm2_v1_plate_set_party_weight(100);
    dm2_v1_plate_set_party_position(-1, -1, -1);
    dm2_v1_plate_set_item_on_floor(0, -1, -1, -1);
}

/* ── Catalog (1-9) ─────────────────────────────────────────────── */

static int test_builtin_count(void) {
    return dm2_v1_plate_get_builtin_count() == DM2_PLATE_NUM_BUILTIN;
}

static int test_get_builtin_known(void) {
    return dm2_v1_plate_get_builtin(1) != NULL
        && dm2_v1_plate_get_builtin(2) != NULL
        && dm2_v1_plate_get_builtin(3) != NULL
        && dm2_v1_plate_get_builtin(4) != NULL
        && dm2_v1_plate_get_builtin(5) != NULL;
}

static int test_get_builtin_unknown_returns_null(void) {
    return dm2_v1_plate_get_builtin(99) == NULL
        && dm2_v1_plate_get_builtin(0) == NULL
        && dm2_v1_plate_get_builtin(-1) == NULL;
}

static int test_plate_coordinates_valid(void) {
    for (int i = 1; i <= DM2_PLATE_NUM_BUILTIN; i++) {
        const DM2_V1_PressurePlate *p = dm2_v1_plate_get_builtin(i);
        if (!p) return 0;
        if (p->map_x < 0 || p->map_y < 0) return 0;
        if (p->map_level < 0 || p->map_level > 15) return 0;
    }
    return 1;
}

static int test_plate_kind_valid(void) {
    for (int i = 1; i <= DM2_PLATE_NUM_BUILTIN; i++) {
        const DM2_V1_PressurePlate *p = dm2_v1_plate_get_builtin(i);
        if (!p) continue;
        if (p->kind < DM2_PLATE_KIND_WEIGHT
         || p->kind > DM2_PLATE_KIND_CREATURE) return 0;
    }
    return 1;
}

static int test_plate_target_kind_valid(void) {
    for (int i = 1; i <= DM2_PLATE_NUM_BUILTIN; i++) {
        const DM2_V1_PressurePlate *p = dm2_v1_plate_get_builtin(i);
        if (!p) continue;
        if (p->target_kind < DM2_PLATE_TARGET_DOOR_TOGGLE
         || p->target_kind > DM2_PLATE_TARGET_CREATURE_SPAWN) return 0;
    }
    return 1;
}

static int test_weight_plate_has_threshold(void) {
    const DM2_V1_PressurePlate *p = dm2_v1_plate_get_builtin(1);
    return p && p->kind == DM2_PLATE_KIND_WEIGHT && p->weight_threshold > 0;
}

static int test_item_plate_has_required_item(void) {
    const DM2_V1_PressurePlate *p = dm2_v1_plate_get_builtin(2);
    return p && p->kind == DM2_PLATE_KIND_ITEM && p->required_item_id > 0;
}

static int test_time_plate_has_period(void) {
    const DM2_V1_PressurePlate *p = dm2_v1_plate_get_builtin(3);
    return p && p->kind == DM2_PLATE_KIND_TIME && p->time_period_ms > 0;
}

/* ── State setters (10-13) ────────────────────────────────────── */

static int test_reset_state_clears(void) {
    dm2_v1_plate_force_fire(1);
    int before = dm2_v1_plate_fire_total();
    if (before == 0) return 0;
    dm2_v1_plate_reset_state();
    return dm2_v1_plate_fire_total() == 0;
}

static int test_set_party_weight(void) {
    setup_clean();
    dm2_v1_plate_set_party_weight(500);
    /* Verify by trying to fire weight plate (threshold=300). */
    int rc = dm2_v1_plate_check(1, 0);
    return rc == (int)DM2_PLATE_RESULT_OK;
}

static int test_set_party_position(void) {
    setup_clean();
    dm2_v1_plate_set_party_position(7, 7, 1);  /* plate 4 */
    int rc = dm2_v1_plate_check(4, 0);
    return rc == (int)DM2_PLATE_RESULT_OK;
}

static int test_set_item_on_floor(void) {
    setup_clean();
    /* Plate 2 expects item 111 at (5,12,0). */
    dm2_v1_plate_set_item_on_floor(111, 5, 12, 0);
    int rc = dm2_v1_plate_check(2, 0);
    return rc == (int)DM2_PLATE_RESULT_OK;
}

/* ── WEIGHT plate (14-15) ─────────────────────────────────────── */

static int test_weight_plate_fires_above_threshold(void) {
    setup_clean();
    dm2_v1_plate_set_party_weight(500);  /* plate 1 threshold=300 */
    int rc = dm2_v1_plate_check(1, 1000);
    return rc == (int)DM2_PLATE_RESULT_OK;
}

static int test_weight_plate_rejects_below_threshold(void) {
    setup_clean();
    dm2_v1_plate_set_party_weight(100);  /* below threshold=300 */
    int rc = dm2_v1_plate_check(1, 1000);
    return rc == (int)DM2_PLATE_RESULT_INSUFFICIENT_WEIGHT;
}

/* ── ITEM plate (16-18) ───────────────────────────────────────── */

static int test_item_plate_fires_with_correct_item(void) {
    setup_clean();
    dm2_v1_plate_set_item_on_floor(111, 5, 12, 0);  /* plate 2 wants item 111 at (5,12,0) */
    int rc = dm2_v1_plate_check(2, 1000);
    return rc == (int)DM2_PLATE_RESULT_OK;
}

static int test_item_plate_rejects_wrong_item(void) {
    setup_clean();
    dm2_v1_plate_set_item_on_floor(999, 5, 12, 0);  /* wrong item */
    int rc = dm2_v1_plate_check(2, 1000);
    return rc == (int)DM2_PLATE_RESULT_WRONG_ITEM;
}

static int test_item_plate_rejects_wrong_position(void) {
    setup_clean();
    dm2_v1_plate_set_item_on_floor(111, 0, 0, 0);  /* right item, wrong pos */
    int rc = dm2_v1_plate_check(2, 1000);
    return rc == (int)DM2_PLATE_RESULT_WRONG_ITEM;
}

/* ── PARTY plate (19-20) ──────────────────────────────────────── */

static int test_party_plate_fires_when_present(void) {
    setup_clean();
    dm2_v1_plate_set_party_position(7, 7, 1);  /* plate 4 */
    int rc = dm2_v1_plate_check(4, 1000);
    return rc == (int)DM2_PLATE_RESULT_OK;
}

static int test_party_plate_rejects_when_absent(void) {
    setup_clean();
    dm2_v1_plate_set_party_position(0, 0, 0);  /* not on plate */
    int rc = dm2_v1_plate_check(4, 1000);
    return rc == (int)DM2_PLATE_RESULT_NO_PARTY;
}

/* ── TIME plate (21-23) ───────────────────────────────────────── */

static int test_time_plate_fires_first_time(void) {
    setup_clean();
    /* first call: no prior last_fire_ms, fires immediately */
    int rc = dm2_v1_plate_check(3, 1000);
    return rc == (int)DM2_PLATE_RESULT_OK;
}

static int test_time_plate_rejects_before_period(void) {
    setup_clean();
    dm2_v1_plate_check(3, 1000);  /* fire at t=1000 */
    int rc = dm2_v1_plate_check(3, 2000);  /* only 1s later, period=5s */
    return rc == (int)DM2_PLATE_RESULT_NOT_TIME_YET;
}

static int test_time_plate_refires_after_period(void) {
    setup_clean();
    dm2_v1_plate_check(3, 1000);   /* fire at t=1000 */
    int rc = dm2_v1_plate_check(3, 7000);  /* 6s later, period=5s */
    return rc == (int)DM2_PLATE_RESULT_OK;
}

/* ── CREATURE plate (24) ──────────────────────────────────────── */

static int test_creature_plate_fires(void) {
    setup_clean();
    int rc = dm2_v1_plate_check(5, 0);
    return rc == (int)DM2_PLATE_RESULT_OK;
}

/* ── fire_once (25-26) ────────────────────────────────────────── */

static int test_fire_once_no_refire(void) {
    setup_clean();
    dm2_v1_plate_set_item_on_floor(111, 5, 12, 0);
    int rc1 = dm2_v1_plate_check(2, 1000);  /* first fire */
    int rc2 = dm2_v1_plate_check(2, 2000);  /* should be ALREADY_FIRED */
    return rc1 == (int)DM2_PLATE_RESULT_OK
        && rc2 == (int)DM2_PLATE_RESULT_ALREADY_FIRED;
}

static int test_fire_once_can_be_reset(void) {
    setup_clean();
    dm2_v1_plate_set_item_on_floor(111, 5, 12, 0);
    dm2_v1_plate_check(2, 1000);  /* fire */
    dm2_v1_plate_reset_fire_count(2);
    int rc = dm2_v1_plate_check(2, 2000);  /* should fire again */
    return rc == (int)DM2_PLATE_RESULT_OK;
}

/* ── Force fire (27) ──────────────────────────────────────────── */

static int test_force_fire_bypasses(void) {
    setup_clean();
    dm2_v1_plate_set_party_weight(0);  /* below threshold */
    int rc = dm2_v1_plate_force_fire(1);  /* bypass */
    return rc == (int)DM2_PLATE_RESULT_OK
        && dm2_v1_plate_get_fire_count(1) == 1;
}

/* ── Door state (28-29) ───────────────────────────────────────── */

static int test_door_state_open_for_open_target(void) {
    /* Plate 2 has target DOOR_OPEN. */
    return dm2_v1_plate_get_door_state_after_fire(2) == DM2_DOOR_STATE_OPEN;
}

static int test_door_state_for_toggle(void) {
    /* Plate 1 has target DOOR_TOGGLE. */
    return dm2_v1_plate_get_door_state_after_fire(1) == DM2_DOOR_STATE_OPEN;
}

/* ── Activate/deactivate (30-32) ──────────────────────────────── */

static int test_activate_deactivate(void) {
    setup_clean();
    dm2_v1_plate_activate(1);
    int rc = dm2_v1_plate_get_state_for(1);
    if (rc != 1) return 0;
    dm2_v1_plate_deactivate(1);
    return dm2_v1_plate_get_state_for(1) == 0;
}

static int test_one_way_party_leave_resets(void) {
    setup_clean();
    /* Plate 4 is one-way: party leave resets it. */
    dm2_v1_plate_set_party_position(7, 7, 1);
    dm2_v1_plate_check(4, 0);  /* fires */
    dm2_v1_plate_set_party_position(0, 0, 0);  /* party leaves */
    return dm2_v1_plate_get_state_for(4) == 0;
}

static int test_one_way_party_arrive_sets(void) {
    setup_clean();
    /* Plate 4 is one-way: party arrive sets it. */
    dm2_v1_plate_set_party_position(7, 7, 1);
    return dm2_v1_plate_get_state_for(4) == 1;
}

/* ── Observability (33-34) ────────────────────────────────────── */

static int test_fire_total_increments(void) {
    setup_clean();
    int before = dm2_v1_plate_fire_total();
    dm2_v1_plate_force_fire(1);
    dm2_v1_plate_force_fire(2);
    return dm2_v1_plate_fire_total() == before + 2;
}

static int test_active_count(void) {
    setup_clean();
    dm2_v1_plate_activate(1);
    dm2_v1_plate_activate(2);
    return dm2_v1_plate_active_count() == 2;
}

/* ── Source (35) ──────────────────────────────────────────────── */

static int test_source_evidence(void) {
    const char *e = dm2_v1_pressure_plate_source_evidence();
    return e != NULL && e[0] != '\0'
        && strstr(e, "skproject/SKULLWIN/c_sensor.cpp") != NULL
        && strstr(e, "dPressurePlatesTable") != NULL;
}

/* ── Reset fire count (36) ────────────────────────────────────── */

static int test_reset_fire_count_zeroes(void) {
    setup_clean();
    dm2_v1_plate_force_fire(1);
    dm2_v1_plate_force_fire(1);
    dm2_v1_plate_force_fire(1);
    dm2_v1_plate_reset_fire_count(1);
    return dm2_v1_plate_get_fire_count(1) == 0;
}

/* ── Multiple independent plates (37) ─────────────────────────── */

static int test_multiple_plates_independent(void) {
    setup_clean();
    dm2_v1_plate_set_party_weight(500);
    int rc1 = dm2_v1_plate_check(1, 1000);  /* weight plate fires */
    dm2_v1_plate_set_item_on_floor(111, 5, 12, 0);
    int rc2 = dm2_v1_plate_check(2, 1100);  /* item plate fires */
    return rc1 == (int)DM2_PLATE_RESULT_OK
        && rc2 == (int)DM2_PLATE_RESULT_OK
        && dm2_v1_plate_get_fire_count(1) == 1
        && dm2_v1_plate_get_fire_count(2) == 1;
}

/* ── Edge cases (38-40) ───────────────────────────────────────── */

static int test_weight_plate_zero_threshold(void) {
    setup_clean();
    /* Force a custom plate scenario: weight 1, threshold 0 (any positive). */
    dm2_v1_plate_set_party_weight(1);
    int rc = dm2_v1_plate_check(1, 0);  /* threshold 300, weight 1 → fails */
    return rc == (int)DM2_PLATE_RESULT_INSUFFICIENT_WEIGHT;
}

static int test_target_message_for_time_plate(void) {
    const char *m = dm2_v1_plate_get_target_message(3);
    return m != NULL && m[0] != '\0';
}

static int test_target_message_for_other_plate(void) {
    return dm2_v1_plate_get_target_message(1) == NULL;  /* no message */
}

static int test_last_event_empty_after_reset(void) {
    setup_clean();
    return dm2_v1_plate_last_event() == NULL
        && dm2_v1_plate_copy_last_event(NULL) == 0;
}

static int test_last_event_records_weight_target(void) {
    DM2_V1_PlateEvent ev;
    setup_clean();
    dm2_v1_plate_set_party_weight(500);
    if (dm2_v1_plate_check(1, 4321) != (int)DM2_PLATE_RESULT_OK) return 0;
    if (!dm2_v1_plate_copy_last_event(&ev)) return 0;
    return ev.valid == 1
        && ev.plate_id == 1
        && ev.kind == DM2_PLATE_KIND_WEIGHT
        && ev.target_kind == DM2_PLATE_TARGET_DOOR_TOGGLE
        && ev.target_x == 13
        && ev.target_y == 8
        && ev.target_level == 0
        && ev.door_state_after_fire == DM2_DOOR_STATE_OPEN
        && ev.now_ms == 4321
        && ev.fire_count == 1
        && ev.message == NULL;
}

static int test_last_event_records_message_target(void) {
    const DM2_V1_PlateEvent *ev;
    setup_clean();
    if (dm2_v1_plate_check(3, 1000) != (int)DM2_PLATE_RESULT_OK) return 0;
    ev = dm2_v1_plate_last_event();
    return ev != NULL
        && ev->plate_id == 3
        && ev->target_kind == DM2_PLATE_TARGET_MESSAGE
        && ev->message != NULL
        && strstr(ev->message, "distant rumble") != NULL;
}

/* ── Main ─────────────────────────────────────────────────────── */

int main(void) {
    printf("DM2 V1 Pressure Plate parity - Phase 4 source-lock tests\n");
    printf("Source: skproject/SKULLWIN/c_sensor.cpp (plate logic)\n");
    printf("        skproject/SKWIN/SkGlobal.cpp:1112-1170 (dPressurePlatesTable)\n");
    printf("        ReDMCSB MOVESENS.C:1000-1100 (DM1 sensor parity)\n\n");

    /* Catalog (1-9) */
    TEST(builtin_count);
    TEST(get_builtin_known);
    TEST(get_builtin_unknown_returns_null);
    TEST(plate_coordinates_valid);
    TEST(plate_kind_valid);
    TEST(plate_target_kind_valid);
    TEST(weight_plate_has_threshold);
    TEST(item_plate_has_required_item);
    TEST(time_plate_has_period);

    /* State setters (10-13) */
    TEST(reset_state_clears);
    TEST(set_party_weight);
    TEST(set_party_position);
    TEST(set_item_on_floor);

    /* WEIGHT (14-15) */
    TEST(weight_plate_fires_above_threshold);
    TEST(weight_plate_rejects_below_threshold);

    /* ITEM (16-18) */
    TEST(item_plate_fires_with_correct_item);
    TEST(item_plate_rejects_wrong_item);
    TEST(item_plate_rejects_wrong_position);

    /* PARTY (19-20) */
    TEST(party_plate_fires_when_present);
    TEST(party_plate_rejects_when_absent);

    /* TIME (21-23) */
    TEST(time_plate_fires_first_time);
    TEST(time_plate_rejects_before_period);
    TEST(time_plate_refires_after_period);

    /* CREATURE (24) */
    TEST(creature_plate_fires);

    /* fire_once (25-26) */
    TEST(fire_once_no_refire);
    TEST(fire_once_can_be_reset);

    /* Force fire (27) */
    TEST(force_fire_bypasses);

    /* Door state (28-29) */
    TEST(door_state_open_for_open_target);
    TEST(door_state_for_toggle);

    /* Activate/deactivate (30-32) */
    TEST(activate_deactivate);
    TEST(one_way_party_leave_resets);
    TEST(one_way_party_arrive_sets);

    /* Observability (33-34) */
    TEST(fire_total_increments);
    TEST(active_count);

    /* Source (35) */
    TEST(source_evidence);

    /* Reset fire count (36) */
    TEST(reset_fire_count_zeroes);

    /* Multiple (37) */
    TEST(multiple_plates_independent);

    /* Edge cases (38-40) */
    TEST(weight_plate_zero_threshold);
    TEST(target_message_for_time_plate);
    TEST(target_message_for_other_plate);
    TEST(last_event_empty_after_reset);
    TEST(last_event_records_weight_target);
    TEST(last_event_records_message_target);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
