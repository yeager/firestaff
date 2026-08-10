/* CSBWin EDBT_DeleteDuplicateTimers runtime-consumption regression.
 * Source: CSBWin SaveGame.cpp:1978-1986 and Timer.cpp:967-1007. */

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

static struct DM1_Event_V1 make_event(uint8_t type, uint8_t cell,
                                      uint8_t effect)
{
    struct DM1_Event_V1 event;
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(3u, 200u);
    event.type = type;
    event.b_mapX = 12u;
    event.b_mapY = 7u;
    event.c_cell = cell;
    event.c_effect = effect;
    return event;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    struct DM1_Event_V1 first = make_event(DM1_EVENT_WALL, 1u, 0u);
    struct DM1_Event_V1 duplicate = make_event(DM1_EVENT_WALL, 1u, 2u);
    struct DM1_Event_V1 distinct_cell = make_event(DM1_EVENT_WALL, 2u, 1u);
    struct DM1_Event_V1 corridor = make_event(DM1_EVENT_CORRIDOR, 0u, 1u);
    struct DM1_Event_V1 corridor_duplicate =
        make_event(DM1_EVENT_CORRIDOR, 3u, 2u);
    struct DM1_Event_V1 door_same_square = make_event(DM1_EVENT_DOOR, 0u, 1u);
    struct DM1_Event_V1 door_duplicate = make_event(DM1_EVENT_DOOR, 2u, 2u);
    int first_index;
    int corridor_index;
    int door_index;

    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_delete_duplicate_timers = 1u;
    first_index = csb_v1_runtime_add_timeline_event(&profile, &first);
    check(first_index >= 0 && profile.timeline_queue.eventCount == 1,
          "CSBWin inserts the first map timer");
    check(csb_v1_runtime_add_timeline_event(&profile, &duplicate) == first_index &&
              profile.timeline_queue.eventCount == 1 &&
              profile.timeline_queue.events[first_index].c_effect == 2u,
          "CSBWin reuses a matching map-timer slot and replaces action byte");
    check(csb_v1_runtime_add_timeline_event(&profile, &distinct_cell) >= 0 &&
              profile.timeline_queue.eventCount == 2,
          "CSBWin keeps distinct TT_STONEROOM positions separate");
    corridor_index = csb_v1_runtime_add_timeline_event(&profile, &corridor);
    check(corridor_index >= 0 &&
              csb_v1_runtime_add_timeline_event(
                  &profile, &corridor_duplicate) == corridor_index &&
              profile.timeline_queue.events[corridor_index].c_effect == 2u,
          "CSBWin reuses a matching corridor timer and replaces action byte");
    door_index = csb_v1_runtime_add_timeline_event(&profile, &door_same_square);
    check(door_index >= 0 && door_index != corridor_index &&
              profile.timeline_queue.eventCount == 4,
          "CSBWin keeps different map timer functions separate on the same square");
    check(csb_v1_runtime_add_timeline_event(&profile, &door_duplicate) ==
              door_index &&
              profile.timeline_queue.events[door_index].c_effect == 2u &&
              profile.timeline_queue.eventCount == 4,
          "CSBWin reuses a matching door timer without collapsing corridor state");

    csb_v1_runtime_cleanup(&profile);
    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_delete_duplicate_timers = 0u;
    first_index = csb_v1_runtime_add_timeline_event(&profile, &first);
    check(first_index >= 0 &&
              csb_v1_runtime_add_timeline_event(&profile, &duplicate) >= 0 &&
              profile.timeline_queue.eventCount == 2,
          "CSBWin preserves duplicate map timers when the saved policy disables cleanup");

    /* SaveGame.cpp restores TimerQueue directly; it must not re-enter
     * Timer.cpp SetTimer and collapse already saved duplicate slots. */
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_delete_duplicate_timers = 1u;
    profile.csbwin_max_timers = 3u;
    profile.csbwin_num_timer = 2u;
    profile.csbwin_first_avail_timer = 2u;
    profile.csbwin_timer_summary_count = 3u;
    profile.csbwin_timer_summary_total = 3u;
    profile.csbwin_timer_queue_summary_count = 2u;
    profile.csbwin_timer_queue_summary_total = 2u;
    memset(profile.csbwin_timers, 0, sizeof(profile.csbwin_timers));
    profile.csbwin_timers[0].valid = 1;
    profile.csbwin_timers[0].source_index = 0u;
    profile.csbwin_timers[0].time = 200u;
    profile.csbwin_timers[0].level = 3u;
    profile.csbwin_timers[0].function = DM1_EVENT_WALL;
    profile.csbwin_timers[0].ubyte6 = 12u;
    profile.csbwin_timers[0].ubyte7 = 7u;
    profile.csbwin_timers[0].ubyte8 = 1u;
    profile.csbwin_timers[0].ubyte9 = 0u;
    profile.csbwin_timers[1] = profile.csbwin_timers[0];
    profile.csbwin_timers[1].source_index = 1u;
    profile.csbwin_timers[1].ubyte9 = 2u;
    profile.csbwin_timers[2].valid = 1;
    profile.csbwin_timers[2].source_index = 2u;
    profile.csbwin_timers[2].function = DM1_EVENT_NONE;
    profile.csbwin_timer_queue[0] = 0u;
    profile.csbwin_timer_queue[1] = 1u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 2 &&
              profile.timeline_queue.eventCount == 2 &&
              profile.csbwin_timeline_event_queue_slot[0] == 0u &&
              profile.csbwin_timeline_event_queue_slot[1] == 1u,
          "CSBWin restore projects only active queue slots, not free TIMER slots");

    /* TIMER::operator< (CSBWin Timer.cpp TAG00fd9e) gives the saved
     * TT_ParameterMessage (101) precedence over every other timer at the
     * same tick.  Its byte-5 sequence is ascending, unlike ordinary timer
     * functions.  This is a valid source heap that the old numeric-function
     * approximation rejected before the runtime could publish it. */
    profile.csbwin_timers[0].function = 101u; /* TT_ParameterMessage */
    profile.csbwin_timers[0].ubyte5 = 9u;
    profile.csbwin_timers[1].function = DM1_EVENT_WALL;
    profile.csbwin_timers[1].ubyte5 = 1u;
    profile.csbwin_timer_queue[0] = 0u;
    profile.csbwin_timer_queue[1] = 1u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 2 &&
              profile.timeline_queue.eventCount == 2 &&
              profile.csbwin_timeline_event_queue_slot[0] == 0u &&
              profile.csbwin_timeline_event_queue_slot[1] == 1u,
          "CSBWin restore admits same-tick parameter-message heap precedence");

    profile.csbwin_timers[0].function = DM1_EVENT_WALL;
    profile.csbwin_timers[0].ubyte5 = 0u;
    profile.csbwin_timers[1] = profile.csbwin_timers[0];
    profile.csbwin_timers[1].source_index = 1u;
    profile.csbwin_timers[1].ubyte9 = 2u;

    profile.csbwin_timer_queue[1] = 2u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) < 0 &&
              profile.timeline_queue.eventCount == 2 &&
              profile.csbwin_timeline_event_queue_slot[0] == 0u &&
              profile.csbwin_timeline_event_queue_slot[1] == 1u,
          "CSBWin restore rejects an invalid queue slot without replacing live timers");

    profile.csbwin_timer_queue[1] = 1u;
    profile.csbwin_timer_summary_total = 2u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) < 0 &&
              profile.timeline_queue.eventCount == 2,
          "CSBWin restore rejects a truncated timer summary without replacing live timers");

    profile.csbwin_timer_summary_total = 3u;
    profile.csbwin_timer_queue[0] = 1u;
    profile.csbwin_timer_queue[1] = 0u;
    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) < 0 &&
              profile.timeline_queue.eventCount == 2 &&
              profile.csbwin_timeline_event_queue_slot[0] == 0u &&
              profile.csbwin_timeline_event_queue_slot[1] == 1u,
          "CSBWin restore rejects a reordered source timer heap atomically");
    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
