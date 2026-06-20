/*
 * firestaff_dm2_v1_timeline_probe.c — DM2 V1 Timeline Wiring Probe
 *
 * Exercises the dm2_v1_timeline.c API: init/schedule/tick/fire/cancel.
 * Reports queue size, fire counts, and a sample tick sequence.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_timeline.cpp        - timeline core
 *   skproject/SKWIN/DME.h:1780-1850          - timeline_event_t
 *   skproject/SKWIN/SkGlobal.cpp:1280-1350   - dTimelineTable
 *   ReDMCSB TIMELINE.C:43-220                - DM1 timeline events
 *   ReDMCSB GAMELOOP.C:69                    - F0261_TIMELINE_Process_CPSEF
 */

#include "dm2_v1_timeline.h"

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
        case DM2_TIMELINE_EVENT_NPC_MOVE:       return "NPC_MOVE";
        case DM2_TIMELINE_EVENT_CREATURE_SPAWN: return "CREATURE_SPAWN";
        case DM2_TIMELINE_EVENT_DOOR_LOCK:      return "DOOR_LOCK";
        case DM2_TIMELINE_EVENT_DOOR_UNLOCK:    return "DOOR_UNLOCK";
        case DM2_TIMELINE_EVENT_MESSAGE_DISPLAY: return "MESSAGE_DISPLAY";
        default: return "?";
    }
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    fprintf(stderr, "=== DM2 V1 Timeline Wiring Verification Probe ===\n");
    fprintf(stderr, "Source: skproject/SKULLWIN/c_timeline.cpp (timeline core)\n");
    fprintf(stderr, "        ReDMCSB TIMELINE.C:43-220 (F0261 timeline events)\n");
    fprintf(stderr, "        ReDMCSB GAMELOOP.C:69 (F0261_TIMELINE_Process_CPSEF)\n\n");

    dm2_v1_timeline_reset_state();
    dm2_v1_timeline_init();

    /* Invariant 1: built-in count + queue size after init. */
    PROBE_ASSERT(dm2_v1_timeline_get_builtin_count() == DM2_TIMELINE_NUM_BUILTIN
              && dm2_v1_timeline_queue_size() == DM2_TIMELINE_NUM_BUILTIN,
                 "init loaded %d events into queue (count=%d)",
                 DM2_TIMELINE_NUM_BUILTIN, dm2_v1_timeline_queue_size());

    /* Invariant 2: dump all 6 events. */
    for (int i = 1; i <= DM2_TIMELINE_NUM_BUILTIN; i++) {
        const DM2_V1_TimelineEvent *e = dm2_v1_timeline_get_builtin(i);
        if (e) {
            fprintf(stderr, "  Event %d [%s] fire_at=%dms%s\n",
                    i, kind_str(e->kind), e->fire_at_ms,
                    e->message ? " (with message)" : "");
        }
    }
    PROBE_ASSERT(dm2_v1_timeline_get_builtin(1) != NULL
              && dm2_v1_timeline_get_builtin(6) != NULL,
                 "all 6 built-in events retrievable");

    /* Invariant 3: tick(0) fires NPC_MOVE (event 1 at t=0). */
    int fired = dm2_v1_timeline_tick(0);
    PROBE_ASSERT(fired >= 1 && dm2_v1_timeline_get_fire_count(1) == 1,
                 "tick(0) fires NPC_MOVE (fired=%d, count=%d)",
                 fired, dm2_v1_timeline_get_fire_count(1));

    /* Invariant 4: tick(6000) fires 2 remaining events due by t=6000.
     * Event 1 already fired at t=0. Events 5 (t=2000), 2 (t=5000) fire now. */
    fired = dm2_v1_timeline_tick(6000);
    PROBE_ASSERT(fired == 2
              && dm2_v1_timeline_get_fire_count(5) == 1
              && dm2_v1_timeline_get_fire_count(2) == 1,
                 "tick(6000) fires MESSAGE/CREATURE (fired=%d)", fired);

    /* Invariant 5: tick(30000) fires 3 remaining events 6, 3, 4. */
    fired = dm2_v1_timeline_tick(30000);
    PROBE_ASSERT(fired == 3
              && dm2_v1_timeline_get_fire_count(6) == 1
              && dm2_v1_timeline_get_fire_count(3) == 1
              && dm2_v1_timeline_get_fire_count(4) == 1,
                 "tick(30000) fires 3 remaining events (fired=%d)", fired);

    /* Invariant 6: tick(100000) fires nothing (all already fired). */
    fired = dm2_v1_timeline_tick(100000);
    PROBE_ASSERT(fired == 0,
                 "tick(100000) fires 0 (all already fired) (fired=%d)", fired);

    /* Invariant 7: schedule reschedules an event. */
    dm2_v1_timeline_reset_state();
    int rc = dm2_v1_timeline_schedule(1, 50000);
    PROBE_ASSERT(rc == (int)DM2_TIMELINE_RESULT_OK
              && dm2_v1_timeline_get_event_fire_at(1) == 50000,
                 "schedule(1, 50000) reschedules event 1 (rc=%d, fire_at=%d)",
                 rc, dm2_v1_timeline_get_event_fire_at(1));

    /* Invariant 8: schedule + tick fires at scheduled time. */
    fired = dm2_v1_timeline_tick(60000);  /* past 50000 */
    PROBE_ASSERT(fired >= 1 && dm2_v1_timeline_get_fire_count(1) == 1,
                 "tick(60000) fires rescheduled event 1 (fired=%d, count=%d)",
                 fired, dm2_v1_timeline_get_fire_count(1));

    /* Invariant 9: schedule future event held. */
    dm2_v1_timeline_schedule(2, 100000);
    fired = dm2_v1_timeline_tick(50000);
    PROBE_ASSERT(fired == 0 && dm2_v1_timeline_get_fire_count(2) == 0,
                 "future event 2 (t=100000) held at tick(50000) (fired=%d)", fired);

    /* Invariant 10: cancel removes from queue. */
    dm2_v1_timeline_reset_state();
    dm2_v1_timeline_init();
    int n_before = dm2_v1_timeline_queue_size();
    dm2_v1_timeline_cancel(5);
    PROBE_ASSERT(dm2_v1_timeline_queue_size() == n_before - 1,
                 "cancel(5) reduces queue from %d to %d",
                 n_before, dm2_v1_timeline_queue_size());

    /* Invariant 11: explicit fire. */
    dm2_v1_timeline_reset_state();
    dm2_v1_timeline_init();
    rc = dm2_v1_timeline_fire(3);
    PROBE_ASSERT(rc == (int)DM2_TIMELINE_RESULT_OK
              && dm2_v1_timeline_get_fire_count(3) == 1,
                 "fire(3) explicit (rc=%d, count=%d)",
                 rc, dm2_v1_timeline_get_fire_count(3));

    /* Invariant 12: source evidence. */
    const char *e = dm2_v1_timeline_source_evidence();
    PROBE_ASSERT(e != NULL && strstr(e, "skproject/SKULLWIN/c_timeline.cpp") != NULL
              && strstr(e, "TIMELINE.C") != NULL
              && strstr(e, "F0261_TIMELINE_Process_CPSEF") != NULL,
                 "source evidence mentions skproject + ReDMCSB");

    fprintf(stderr, "\n=== Summary ===\n");
    fprintf(stderr, "Events verified: %d (NPC_MOVE/CREATURE_SPAWN/DOOR_LOCK/DOOR_UNLOCK/MESSAGE_DISPLAY)\n",
            dm2_v1_timeline_get_builtin_count());
    fprintf(stderr, "Total fires this run: %d\n", dm2_v1_timeline_total_fires());
    fprintf(stderr, "Total ticks this run: %d\n", dm2_v1_timeline_total_ticks());
    fprintf(stderr, "\n%d/%d invariants PASS\n", passed, passed + errors);
    return (errors == 0) ? 0 : 1;
}
