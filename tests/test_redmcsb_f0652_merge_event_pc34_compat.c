#include "redmcsb_f0652_merge_event_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(label, condition) do { if (!(condition)) { ++failures; fprintf(stderr, "FAIL: %s\n", label); } } while (0)

static struct DM1_Event_V1 make_event(uint8_t type, uint8_t effect,
                                      uint32_t map_time, uint8_t x, uint8_t y,
                                      uint8_t cell)
{
    struct DM1_Event_V1 event;

    memset(&event, 0, sizeof(event));
    event.type = type;
    event.c_effect = effect;
    event.map_time = map_time;
    event.b_mapX = x;
    event.b_mapY = y;
    event.c_cell = cell;
    return event;
}

static void initialize_queue(struct DM1_EventQueue_V1 *queue,
                             const struct DM1_Event_V1 *events,
                             int event_count)
{
    int index;

    (void)dm1v1_event_queue_init(queue, 0U);
    for (index = 0; index < event_count; ++index) {
        queue->events[index] = events[index];
        queue->timeline[index] = (uint16_t)index;
    }
    queue->eventCount = event_count;
    queue->firstUnusedIndex = event_count;
}

int main(void)
{
    struct DM1_EventQueue_V1 queue;
    struct DM1_Event_V1 events[3];
    struct DM1_Event_V1 incoming;

    events[0] = make_event(DM1_EVENT_WALL, 0U, DM1_MAP_TIME_MAKE(2U, 50U),
                           4U, 5U, 3U);
    initialize_queue(&queue, events, 1);
    incoming = make_event(DM1_EVENT_WALL, 1U, DM1_MAP_TIME_MAKE(2U, 50U),
                          4U, 5U, 3U);
    CHECK("F0652 merges C05..C10 matching source wall records",
          redmcsb_f0652_merge_event_pc34(&queue, 1, &incoming) == 0 &&
          queue.events[0].c_effect == 1U && queue.eventCount == 1);

    events[0] = make_event(DM1_EVENT_DOOR_ANIMATION, 1U,
                           DM1_MAP_TIME_MAKE(1U, 30U), 2U, 3U, 0U);
    initialize_queue(&queue, events, 1);
    incoming = make_event(DM1_EVENT_CORRIDOR, DM1_EFFECT_TOGGLE,
                          DM1_MAP_TIME_MAKE(1U, 30U), 2U, 3U, 0U);
    CHECK("F0652 converts toggle against door animation then deletes it",
          redmcsb_f0652_merge_event_pc34(&queue, 1, &incoming) ==
              REDMCSB_F0652_PC34_NO_MERGE && incoming.c_effect == 0U &&
          queue.events[0].type == DM1_EVENT_NONE && queue.eventCount == 0);

    events[0] = make_event(DM1_EVENT_DOOR, DM1_EFFECT_TOGGLE,
                           DM1_MAP_TIME_MAKE(3U, 70U), 6U, 7U, 0U);
    initialize_queue(&queue, events, 1);
    incoming = make_event(DM1_EVENT_DOOR_ANIMATION, 0U,
                          DM1_MAP_TIME_MAKE(3U, 70U), 6U, 7U, 0U);
    CHECK("F0652 merges C01 into C10 and resolves source toggle",
          redmcsb_f0652_merge_event_pc34(&queue, 1, &incoming) == 0 &&
          queue.events[0].c_effect == 1U && queue.eventCount == 1);

    events[0] = make_event(DM1_EVENT_DOOR_ANIMATION, 0U,
                           DM1_MAP_TIME_MAKE(5U, 10U), 8U, 9U, 0U);
    events[1] = make_event(DM1_EVENT_DOOR, 1U, DM1_MAP_TIME_MAKE(5U, 99U),
                           8U, 9U, 0U);
    events[2] = make_event(DM1_EVENT_DOOR, 1U, DM1_MAP_TIME_MAKE(4U, 99U),
                           8U, 9U, 0U);
    initialize_queue(&queue, events, 3);
    incoming = make_event(DM1_EVENT_DOOR_DESTRUCTION, 0U,
                          DM1_MAP_TIME_MAKE(5U, 1U), 8U, 9U, 0U);
    CHECK("F0652 C02 deletes C01/C10 on the same source map and square",
          redmcsb_f0652_merge_event_pc34(&queue, 3, &incoming) ==
              REDMCSB_F0652_PC34_NO_MERGE &&
          queue.events[0].type == DM1_EVENT_NONE &&
          queue.events[1].type == DM1_EVENT_NONE &&
          queue.events[2].type == DM1_EVENT_DOOR && queue.eventCount == 1);

    CHECK("F0652 rejects a caller queue without source-native capacity",
          redmcsb_f0652_merge_event_pc34(&queue, DM1_EVENT_MAX_COUNT + 1,
                                          &incoming) ==
              REDMCSB_F0652_PC34_INVALID_QUEUE);
    CHECK("source evidence is available",
          strstr(redmcsb_f0652_merge_event_pc34_source_evidence(), "F0652") != NULL);

    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("PASSED: ReDMCSB F0652 event merge transaction");
    return 0;
}
