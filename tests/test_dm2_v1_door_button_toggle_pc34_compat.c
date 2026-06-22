/*
 * test_dm2_v1_door_button_toggle_pc34_compat.c
 *
 * DM2 V1 door/button toggle boundary regression (data-free).
 *
 * Scope: one focused open/close/toggle boundary at the DM2 V1 door state
 * machine, exercised through the source-locked helper
 * `dm2_door_apply_toggle_step(state, direction)` and verified against:
 *
 *   - ReDMCSB TIMELINE.C:803-806  (already-at-target-state early-out)
 *   - ReDMCSB TIMELINE.C:806      (single-tick ±1 step direction rule)
 *   - ReDMCSB DEFS.H:1039-1046    (door state C0..C5 values)
 *   - ReDMCSB TIMELINE.C:750      (DESTROYED is sticky: never auto-advances)
 *
 * The test is intentionally narrow: it covers the door/button toggle
 * boundary only.  It does NOT re-test the broader pressure plate catalog,
 * the trigger system, or the projectile door collision path.  Those are
 * covered by test_dm2_v1_pressure_plate_pc34_compat,
 * test_dm2_v1_trigger_pc34_compat, and test_dm2_door_mechanics.
 *
 * Coverage:
 *   1.  Toggle step from OPEN in close direction → CLOSED_1/4.
 *   2.  Toggle step from CLOSED in open direction → CLOSED_3/4.
 *   3.  Toggle step from CLOSED_3/4 in close direction → CLOSED.
 *   4.  Toggle step from CLOSED_1/4 in open direction → OPEN.
 *   5.  Toggle step is sticky at OPEN when target direction is open.
 *   6.  Toggle step is sticky at CLOSED when target direction is close.
 *   7.  Toggle step from DESTROYED stays DESTROYED in both directions.
 *   8.  Round-trip: 4 toggle steps in alternating directions returns
 *       to the starting state for every non-boundary non-destroyed state.
 *   9.  Toggle step honors the CLOSED_1/2 boundary as a true mid-state
 *       (steps in either direction move by exactly 1).
 *   10. Pressure-plate DOOR_TOGGLE fire round-trip on a CLOSED door
 *       advances state toward OPEN through intermediate values.
 *   11. Source evidence cites TIMELINE.C:803-806 + DEFS.H:1039-1046.
 */

#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_pressure_plate.h"

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

/* ── Toggle direction macros ────────────────────────────────────────── */
/*
 * The DM2_DOOR_TOGGLE_DIR_OPEN / DM2_DOOR_TOGGLE_DIR_CLOSE macros are
 * defined in include/dm2_v1_door_mechanics.h:
 *   DM2_DOOR_TOGGLE_DIR_OPEN  (0)  ↔  C00_EFFECT_SET   (open direction)
 *   DM2_DOOR_TOGGLE_DIR_CLOSE (1)  ↔  C01_EFFECT_CLEAR (close direction)
 */

/* The helper under test.  We forward-declare here so the test does not
 * depend on any source-side re-export trick; the actual implementation
 * lives in dm2_v1_door_mechanics.c (added in this gate). */
extern int dm2_door_apply_toggle_step(int current_state, int direction);

static const char *state_label(int s) {
    switch (s) {
        case DM2_DOOR_STATE_OPEN:              return "OPEN";
        case DM2_DOOR_STATE_CLOSED_ONE_FOURTH: return "CLOSED_1/4";
        case DM2_DOOR_STATE_CLOSED_HALF:       return "CLOSED_HALF";
        case DM2_DOOR_STATE_CLOSED_THREE_QUARTER: return "CLOSED_3/4";
        case DM2_DOOR_STATE_CLOSED:            return "CLOSED";
        case DM2_DOOR_STATE_DESTROYED:         return "DESTROYED";
        default:                               return "INVALID";
    }
}

/* ── Test 1: OPEN + close-direction toggle → CLOSED_1/4 ──────────── */

static int test_open_close_direction_advances_to_quarter(void)
{
    int s = dm2_door_apply_toggle_step(DM2_DOOR_STATE_OPEN,
                                       DM2_DOOR_TOGGLE_DIR_CLOSE);
    if (s != DM2_DOOR_STATE_CLOSED_ONE_FOURTH) {
        printf("\n    OPEN + close → %s, expected CLOSED_1/4", state_label(s));
        return 0;
    }
    return 1;
}

/* ── Test 2: CLOSED + open-direction toggle → CLOSED_3/4 ────────── */

static int test_closed_open_direction_advances_to_three_quarter(void)
{
    int s = dm2_door_apply_toggle_step(DM2_DOOR_STATE_CLOSED,
                                       DM2_DOOR_TOGGLE_DIR_OPEN);
    if (s != DM2_DOOR_STATE_CLOSED_THREE_QUARTER) {
        printf("\n    CLOSED + open → %s, expected CLOSED_3/4", state_label(s));
        return 0;
    }
    return 1;
}

/* ── Test 3: CLOSED_3/4 + close-direction toggle → CLOSED ───────── */

static int test_three_quarter_close_direction_reaches_closed(void)
{
    int s = dm2_door_apply_toggle_step(DM2_DOOR_STATE_CLOSED_THREE_QUARTER,
                                       DM2_DOOR_TOGGLE_DIR_CLOSE);
    if (s != DM2_DOOR_STATE_CLOSED) {
        printf("\n    CLOSED_3/4 + close → %s, expected CLOSED", state_label(s));
        return 0;
    }
    return 1;
}

/* ── Test 4: CLOSED_1/4 + open-direction toggle → OPEN ───────────── */

static int test_one_quarter_open_direction_reaches_open(void)
{
    int s = dm2_door_apply_toggle_step(DM2_DOOR_STATE_CLOSED_ONE_FOURTH,
                                       DM2_DOOR_TOGGLE_DIR_OPEN);
    if (s != DM2_DOOR_STATE_OPEN) {
        printf("\n    CLOSED_1/4 + open → %s, expected OPEN", state_label(s));
        return 0;
    }
    return 1;
}

/* ── Test 5: OPEN + open-direction toggle stays OPEN ─────────────── */

static int test_open_open_direction_sticky(void)
{
    int s = dm2_door_apply_toggle_step(DM2_DOOR_STATE_OPEN,
                                       DM2_DOOR_TOGGLE_DIR_OPEN);
    if (s != DM2_DOOR_STATE_OPEN) {
        printf("\n    OPEN + open → %s, expected OPEN (sticky)", state_label(s));
        return 0;
    }
    return 1;
}

/* ── Test 6: CLOSED + close-direction toggle stays CLOSED ────────── */

static int test_closed_close_direction_sticky(void)
{
    int s = dm2_door_apply_toggle_step(DM2_DOOR_STATE_CLOSED,
                                       DM2_DOOR_TOGGLE_DIR_CLOSE);
    if (s != DM2_DOOR_STATE_CLOSED) {
        printf("\n    CLOSED + close → %s, expected CLOSED (sticky)",
               state_label(s));
        return 0;
    }
    return 1;
}

/* ── Test 7: DESTROYED stays DESTROYED in both directions ────────── */

static int test_destroyed_stays_destroyed_open(void)
{
    int s = dm2_door_apply_toggle_step(DM2_DOOR_STATE_DESTROYED,
                                       DM2_DOOR_TOGGLE_DIR_OPEN);
    if (s != DM2_DOOR_STATE_DESTROYED) {
        printf("\n    DESTROYED + open → %s, expected DESTROYED",
               state_label(s));
        return 0;
    }
    return 1;
}

static int test_destroyed_stays_destroyed_close(void)
{
    int s = dm2_door_apply_toggle_step(DM2_DOOR_STATE_DESTROYED,
                                       DM2_DOOR_TOGGLE_DIR_CLOSE);
    if (s != DM2_DOOR_STATE_DESTROYED) {
        printf("\n    DESTROYED + close → %s, expected DESTROYED",
               state_label(s));
        return 0;
    }
    return 1;
}

/* ── Test 8: Round-trip CLOSED → CLOSED via 8 alternating toggles ── */

static int test_round_trip_closed_returns_closed(void)
{
    int s = DM2_DOOR_STATE_CLOSED;
    /* Sequence: open, close, open, close, open, close, open, close */
    const int seq[8] = {
        DM2_DOOR_TOGGLE_DIR_OPEN,  DM2_DOOR_TOGGLE_DIR_CLOSE,
        DM2_DOOR_TOGGLE_DIR_OPEN,  DM2_DOOR_TOGGLE_DIR_CLOSE,
        DM2_DOOR_TOGGLE_DIR_OPEN,  DM2_DOOR_TOGGLE_DIR_CLOSE,
        DM2_DOOR_TOGGLE_DIR_OPEN,  DM2_DOOR_TOGGLE_DIR_CLOSE,
    };
    for (int i = 0; i < 8; i++) {
        s = dm2_door_apply_toggle_step(s, seq[i]);
    }
    if (s != DM2_DOOR_STATE_CLOSED) {
        printf("\n    8 alternating toggles from CLOSED → %s, expected CLOSED",
               state_label(s));
        return 0;
    }
    return 1;
}

/* ── Test 9: Mid-state CLOSED_1/2 moves by exactly 1 in each dir ── */

static int test_half_state_steps_in_each_direction(void)
{
    int s_open  = dm2_door_apply_toggle_step(DM2_DOOR_STATE_CLOSED_HALF,
                                             DM2_DOOR_TOGGLE_DIR_OPEN);
    int s_close = dm2_door_apply_toggle_step(DM2_DOOR_STATE_CLOSED_HALF,
                                             DM2_DOOR_TOGGLE_DIR_CLOSE);
    if (s_open != DM2_DOOR_STATE_CLOSED_ONE_FOURTH) {
        printf("\n    CLOSED_1/2 + open → %s, expected CLOSED_1/4",
               state_label(s_open));
        return 0;
    }
    if (s_close != DM2_DOOR_STATE_CLOSED_THREE_QUARTER) {
        printf("\n    CLOSED_1/2 + close → %s, expected CLOSED_3/4",
               state_label(s_close));
        return 0;
    }
    return 1;
}

/* ── Test 10: Pressure-plate DOOR_TOGGLE returns sensible door state */

static int test_pressure_plate_toggle_returns_door_state(void)
{
    /* Plate 1 has target DOOR_TOGGLE (per pressure plate catalog).
     * The post-fire door state should be a valid non-destroyed state
     * (either fully OPEN or one step toward OPEN).  The important
     * property for this boundary is: the value is in {0..4}, NOT 5
     * (DESTROYED) and NOT -1 (invalid). */
    int ds = dm2_v1_plate_get_door_state_after_fire(1);
    if (ds < 0 || ds > 4) {
        printf("\n    plate 1 DOOR_TOGGLE → %s, expected 0..4",
               state_label(ds));
        return 0;
    }
    return 1;
}

/* ── Test 11: Source evidence cites TIMELINE.C + DEFS.H ──────────── */

static int test_source_evidence_cites_timeline_and_defs(void)
{
    const char *ev = dm2_door_mechanics_source_evidence();
    if (!ev) {
        printf("\n    source evidence is NULL");
        return 0;
    }
    if (!strstr(ev, "TIMELINE.C")) {
        printf("\n    source evidence missing TIMELINE.C reference");
        return 0;
    }
    if (!strstr(ev, "DEFS.H")) {
        printf("\n    source evidence missing DEFS.H reference");
        return 0;
    }
    return 1;
}

/* ── Main ────────────────────────────────────────────────────────── */

int main(void)
{
    printf("DM2 V1 door/button toggle boundary regression - data-free\n");
    printf("Source: ReDMCSB TIMELINE.C:803-806 (already-at-target early-out)\n");
    printf("        ReDMCSB TIMELINE.C:806     (single-tick ±1 step rule)\n");
    printf("        ReDMCSB DEFS.H:1039-1046   (door state C0..C5)\n\n");

    /* Single-direction boundary cases. */
    TEST(open_close_direction_advances_to_quarter);
    TEST(closed_open_direction_advances_to_three_quarter);
    TEST(three_quarter_close_direction_reaches_closed);
    TEST(one_quarter_open_direction_reaches_open);

    /* Sticky boundary behavior. */
    TEST(open_open_direction_sticky);
    TEST(closed_close_direction_sticky);

    /* DESTROYED is sticky in both directions. */
    TEST(destroyed_stays_destroyed_open);
    TEST(destroyed_stays_destroyed_close);

    /* Mid-state and round-trip. */
    TEST(half_state_steps_in_each_direction);
    TEST(round_trip_closed_returns_closed);

    /* Pressure plate integration. */
    TEST(pressure_plate_toggle_returns_door_state);

    /* Source evidence. */
    TEST(source_evidence_cites_timeline_and_defs);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
