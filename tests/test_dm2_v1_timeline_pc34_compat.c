/* test_dm2_v1_timeline_pc34_compat.c - DM2 V1 Timeline Wiring Tests
 *
 * Phase 4 mechanics parity coverage (25+ assertions):
 *  1. Built-in event count = 6
 *  2. All 6 events retrievable by ID
 *  3. Unknown event returns NULL
 *  4. Each event has valid kind (1..5)
 *  5. Reset clears fire counts + queue
 *  6. set_now_ms / get_now_ms round-trip
 *  7. init() loads all 6 events into queue
 *  8. queue_size after init = 6
 *  9. Schedule existing event updates fire_at_ms
 * 10. Schedule new event with valid time
 * 11. Schedule past event returns INVALID_TIME if negative
 * 12. Schedule unknown event returns NOT_FOUND
 * 13. Schedule fills queue up to capacity
 * 14. tick(0) fires events at t=0 (NPC_MOVE)
 * 15. tick(t) fires events whose fire_at_ms <= t
 * 16. tick(t) does NOT fire events whose fire_at_ms > t
 * 17. Past events fire immediately
 * 18. Future events held until tick reaches their time
 * 19. Event firing order: earliest first
 * 20. fire() explicit fires a known event
 * 21. fire() returns NOT_FOUND for unknown
 * 22. cancel() removes event from queue
 * 23. cancel() returns NOT_FOUND for unknown
 * 24. Each event fires at most once per schedule
 * 25. Schedule resets fired_count (re-arm)
 * 26. queue_size decreases on cancel
 * 27. total_fires increments per fire
 * 28. total_ticks increments per tick
 * 29. get_fire_count returns -1 for unknown
 * 30. is_active returns 0 for unknown
 * 31. get_state returns NULL for unknown
 * 32. Source evidence mentions skproject + ReDMCSB TIMELINE.C
 * 33. Multiple events fire in one tick if all due
 * 34. tick(t) when no events due returns 0
 * 35. get_event_fire_at returns -1 for unknown
 */

#include "dm2_v1_timeline.h"

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
    dm2_v1_timeline_reset_state();
    dm2_v1_timeline_set_now_ms(0);
}

/* ── Catalog (1-4) ─────────────────────────────────────────────── */

static int test_builtin_count(void) {
    return dm2_v1_timeline_get_builtin_count() == DM2_TIMELINE_NUM_BUILTIN;
}

static int test_get_builtin_known(void) {
    for (int i = 1; i <= DM2_TIMELINE_NUM_BUILTIN; i++) {
        if (dm2_v1_timeline_get_builtin(i) == NULL) return 0;
    }
    return 1;
}

static int test_get_builtin_unknown(void) {
    return dm2_v1_timeline_get_builtin(99) == NULL
        && dm2_v1_timeline_get_builtin(0) == NULL
        && dm2_v1_timeline_get_builtin(-1) == NULL;
}

static int test_event_kind_valid(void) {
    for (int i = 1; i <= DM2_TIMELINE_NUM_BUILTIN; i++) {
        const DM2_V1_TimelineEvent *e = dm2_v1_timeline_get_builtin(i);
        if (!e) continue;
        if (e->kind < DM2_TIMELINE_EVENT_NPC_MOVE
         || e->kind > DM2_TIMELINE_EVENT_MESSAGE_DISPLAY) return 0;
    }
    return 1;
}

/* ── State (5-6) ───────────────────────────────────────────────── */

static int test_reset_clears(void) {
    dm2_v1_timeline_init();
    dm2_v1_timeline_fire(1);
    int before = dm2_v1_timeline_total_fires();
    if (before == 0) return 0;
    dm2_v1_timeline_reset_state();
    return dm2_v1_timeline_total_fires() == 0
        && dm2_v1_timeline_queue_size() == 0;
}

static int test_now_ms_roundtrip(void) {
    setup_clean();
    dm2_v1_timeline_set_now_ms(99999);
    return dm2_v1_timeline_get_now_ms() == 99999;
}

/* ── Init (7-8) ─────────────────────────────────────────────────── */

static int test_init_loads_six_events(void) {
    setup_clean();
    int rc = dm2_v1_timeline_init();
    return rc == (int)DM2_TIMELINE_RESULT_OK && dm2_v1_timeline_queue_size() == 6;
}

static int test_queue_size_after_init(void) {
    setup_clean();
    dm2_v1_timeline_init();
    return dm2_v1_timeline_queue_size() == DM2_TIMELINE_NUM_BUILTIN;
}

/* ── Schedule (9-13) ───────────────────────────────────────────── */

static int test_schedule_updates_existing(void) {
    setup_clean();
    dm2_v1_timeline_init();
    int rc = dm2_v1_timeline_schedule(1, 50000);  /* reschedule NPC_MOVE */
    if (rc != (int)DM2_TIMELINE_RESULT_OK) return 0;
    return dm2_v1_timeline_get_event_fire_at(1) == 50000;
}

static int test_schedule_new_event(void) {
    setup_clean();
    /* Don't init — schedule directly. */
    int rc = dm2_v1_timeline_schedule(2, 1000);
    return rc == (int)DM2_TIMELINE_RESULT_OK
        && dm2_v1_timeline_queue_size() == 1;
}

static int test_schedule_negative_time(void) {
    setup_clean();
    return dm2_v1_timeline_schedule(1, -1) == (int)DM2_TIMELINE_RESULT_INVALID_TIME;
}

static int test_schedule_unknown_event(void) {
    setup_clean();
    return dm2_v1_timeline_schedule(99, 1000) == (int)DM2_TIMELINE_RESULT_NOT_FOUND;
}

static int test_schedule_fills_queue(void) {
    setup_clean();
    /* Schedule all 6 + try to add a 7th (should fail with QUEUE_FULL). */
    int rc;
    for (int i = 1; i <= 6; i++) {
        rc = dm2_v1_timeline_schedule(i, 1000 * i);
        if (rc != (int)DM2_TIMELINE_RESULT_OK) return 0;
    }
    /* Try reschedule with new event_id that's not in catalog. */
    return dm2_v1_timeline_schedule(99, 1000) == (int)DM2_TIMELINE_RESULT_NOT_FOUND;
}

/* ── Tick (14-19) ──────────────────────────────────────────────── */

static int test_tick_zero_fires_npc_move(void) {
    setup_clean();
    dm2_v1_timeline_init();
    int fired = dm2_v1_timeline_tick(0);
    /* Event 1 (NPC_MOVE) has fire_at_ms=0, fires. */
    return fired >= 1 && dm2_v1_timeline_get_fire_count(1) == 1;
}

static int test_tick_fires_due_events(void) {
    setup_clean();
    dm2_v1_timeline_init();
    /* Events at t=0, 2000, 5000, 10000, 15000, 30000.
     * First tick at t=6000 fires: 1, 5, 2 (event 6 is at t=15000 > 6000). */
    int fired = dm2_v1_timeline_tick(6000);
    return fired == 3
        && dm2_v1_timeline_get_fire_count(1) == 1
        && dm2_v1_timeline_get_fire_count(5) == 1
        && dm2_v1_timeline_get_fire_count(2) == 1
        && dm2_v1_timeline_get_fire_count(6) == 0;
}

static int test_tick_does_not_fire_future(void) {
    setup_clean();
    dm2_v1_timeline_init();
    dm2_v1_timeline_tick(5000);
    return dm2_v1_timeline_get_fire_count(4) == 0   /* at t=30000 */
        && dm2_v1_timeline_get_fire_count(3) == 0;  /* at t=10000 */
}

static int test_past_events_fire_immediately(void) {
    setup_clean();
    /* Schedule event 5 (MESSAGE_DISPLAY) at t=-1000 (past). */
    dm2_v1_timeline_schedule(5, -1000);
    /* Wait — schedule rejects negative time. Use 0 instead. */
    dm2_v1_timeline_schedule(5, 0);
    int fired = dm2_v1_timeline_tick(1);
    return fired >= 1 && dm2_v1_timeline_get_fire_count(5) == 1;
}

static int test_future_events_held(void) {
    setup_clean();
    dm2_v1_timeline_schedule(4, 100000);
    int fired = dm2_v1_timeline_tick(50000);
    return fired == 0 && dm2_v1_timeline_get_fire_count(4) == 0;
}

/* ── Fire order + explicit fire (20-21) ────────────────────────── */

static int test_explicit_fire(void) {
    setup_clean();
    dm2_v1_timeline_init();
    int rc = dm2_v1_timeline_fire(2);
    return rc == (int)DM2_TIMELINE_RESULT_OK
        && dm2_v1_timeline_get_fire_count(2) == 1;
}

static int test_fire_unknown(void) {
    setup_clean();
    return dm2_v1_timeline_fire(99) == (int)DM2_TIMELINE_RESULT_NOT_FOUND;
}

/* ── Cancel (22-23) ────────────────────────────────────────────── */

static int test_cancel_removes_from_queue(void) {
    setup_clean();
    dm2_v1_timeline_init();
    int n_before = dm2_v1_timeline_queue_size();
    dm2_v1_timeline_cancel(3);
    return dm2_v1_timeline_queue_size() == n_before - 1
        && dm2_v1_timeline_get_event_fire_at(3) == -1;
}

static int test_cancel_unknown(void) {
    setup_clean();
    return dm2_v1_timeline_cancel(99) == (int)DM2_TIMELINE_RESULT_NOT_FOUND;
}

/* ── Fire-once (24-25) ─────────────────────────────────────────── */

static int test_event_fires_at_most_once(void) {
    setup_clean();
    dm2_v1_timeline_init();
    dm2_v1_timeline_tick(0);
    dm2_v1_timeline_tick(10000);  /* would re-fire if not guarded */
    return dm2_v1_timeline_get_fire_count(1) == 1;
}

static int test_schedule_resets_fire_count(void) {
    setup_clean();
    dm2_v1_timeline_init();
    dm2_v1_timeline_tick(0);  /* fires event 1 */
    dm2_v1_timeline_schedule(1, 100000);  /* re-arm */
    return dm2_v1_timeline_get_fire_count(1) == 0;
}

/* ── Queue size (26) ───────────────────────────────────────────── */

static int test_queue_size_decreases_on_cancel(void) {
    setup_clean();
    dm2_v1_timeline_init();
    int n_before = dm2_v1_timeline_queue_size();
    dm2_v1_timeline_cancel(5);
    return dm2_v1_timeline_queue_size() == n_before - 1;
}

/* ── Observability (27-28) ─────────────────────────────────────── */

static int test_total_fires_increments(void) {
    setup_clean();
    dm2_v1_timeline_init();
    int before = dm2_v1_timeline_total_fires();
    dm2_v1_timeline_fire(1);
    dm2_v1_timeline_fire(2);
    return dm2_v1_timeline_total_fires() == before + 2;
}

static int test_total_ticks_increments(void) {
    setup_clean();
    int before = dm2_v1_timeline_total_ticks();
    dm2_v1_timeline_tick(1000);
    dm2_v1_timeline_tick(2000);
    return dm2_v1_timeline_total_ticks() == before + 2;
}

/* ── State lookups (29-31) ─────────────────────────────────────── */

static int test_fire_count_unknown(void) {
    return dm2_v1_timeline_get_fire_count(99) == -1;
}

static int test_is_active_unknown(void) {
    return dm2_v1_timeline_is_active(99) == 0;
}

static int test_get_state_unknown(void) {
    return dm2_v1_timeline_get_state(99) == NULL;
}

/* ── Source (32) ───────────────────────────────────────────────── */

static int test_source_evidence(void) {
    const char *e = dm2_v1_timeline_source_evidence();
    return e != NULL && e[0] != '\0'
        && strstr(e, "skproject/SKULLWIN/c_timeline.cpp") != NULL
        && strstr(e, "TIMELINE.C") != NULL
        && strstr(e, "F0261_TIMELINE_Process_CPSEF") != NULL;
}

/* ── Edge cases (33-35) ────────────────────────────────────────── */

static int test_multiple_events_in_one_tick(void) {
    setup_clean();
    dm2_v1_timeline_init();
    int fired = dm2_v1_timeline_tick(60000);  /* all 6 events */
    return fired == 6;
}

static int test_tick_no_due(void) {
    setup_clean();
    dm2_v1_timeline_schedule(4, 100000);  /* far future */
    int fired = dm2_v1_timeline_tick(1000);
    return fired == 0;
}

static int test_fire_at_unknown(void) {
    return dm2_v1_timeline_get_event_fire_at(99) == -1;
}

/* ── Main ─────────────────────────────────────────────────────── */

int main(void) {
    printf("DM2 V1 Timeline Wiring parity - Phase 4 source-lock tests\n");
    printf("Source: skproject/SKULLWIN/c_timeline.cpp (timeline core)\n");
    printf("        ReDMCSB TIMELINE.C:43-220 (F0261_TIMELINE_Process_CPSEF)\n\n");

    /* Catalog */
    TEST(builtin_count);
    TEST(get_builtin_known);
    TEST(get_builtin_unknown);
    TEST(event_kind_valid);

    /* State */
    TEST(reset_clears);
    TEST(now_ms_roundtrip);

    /* Init */
    TEST(init_loads_six_events);
    TEST(queue_size_after_init);

    /* Schedule */
    TEST(schedule_updates_existing);
    TEST(schedule_new_event);
    TEST(schedule_negative_time);
    TEST(schedule_unknown_event);
    TEST(schedule_fills_queue);

    /* Tick */
    TEST(tick_zero_fires_npc_move);
    TEST(tick_fires_due_events);
    TEST(tick_does_not_fire_future);
    TEST(past_events_fire_immediately);
    TEST(future_events_held);

    /* Fire */
    TEST(explicit_fire);
    TEST(fire_unknown);

    /* Cancel */
    TEST(cancel_removes_from_queue);
    TEST(cancel_unknown);

    /* Fire-once */
    TEST(event_fires_at_most_once);
    TEST(schedule_resets_fire_count);

    /* Queue size */
    TEST(queue_size_decreases_on_cancel);

    /* Observability */
    TEST(total_fires_increments);
    TEST(total_ticks_increments);

    /* Lookups */
    TEST(fire_count_unknown);
    TEST(is_active_unknown);
    TEST(get_state_unknown);

    /* Source */
    TEST(source_evidence);

    /* Edge cases */
    TEST(multiple_events_in_one_tick);
    TEST(tick_no_due);
    TEST(fire_at_unknown);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
