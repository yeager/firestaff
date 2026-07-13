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
    int first_index;

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

    csb_v1_runtime_cleanup(&profile);
    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_delete_duplicate_timers = 0u;
    first_index = csb_v1_runtime_add_timeline_event(&profile, &first);
    check(first_index >= 0 &&
              csb_v1_runtime_add_timeline_event(&profile, &duplicate) >= 0 &&
              profile.timeline_queue.eventCount == 2,
          "CSBWin preserves duplicate map timers when the saved policy disables cleanup");
    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
