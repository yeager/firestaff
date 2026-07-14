#include "redmcsb_f0652_merge_event_pc34_compat.h"

#include <stddef.h>

static int same_map_time_and_xy(const struct DM1_Event_V1 *left,
                                const struct DM1_Event_V1 *right)
{
    return left->map_time == right->map_time &&
           left->b_mapX == right->b_mapX && left->b_mapY == right->b_mapY;
}

static int delete_event_if_timeline_owned(struct DM1_EventQueue_V1 *queue,
                                          int event_index)
{
    if (dm1v1_event_get_timeline_index(queue, event_index) < 0) {
        return 0;
    }
    return dm1v1_event_delete(queue, event_index);
}

int redmcsb_f0652_merge_event_pc34(
    struct DM1_EventQueue_V1 *queue, int largest_used_event_ordinal,
    struct DM1_Event_V1 *incoming_event)
{
    int event_index;
    int event_type;

    if (queue == NULL || incoming_event == NULL ||
        queue->maxEvents != DM1_EVENT_MAX_COUNT || queue->eventCount < 0 ||
        queue->eventCount > queue->maxEvents ||
        largest_used_event_ordinal < 0 ||
        largest_used_event_ordinal > queue->maxEvents) {
        return REDMCSB_F0652_PC34_INVALID_QUEUE;
    }
    event_type = incoming_event->type;
    if (event_type >= DM1_EVENT_CORRIDOR && event_type <= DM1_EVENT_DOOR) {
        for (event_index = 0; event_index < largest_used_event_ordinal;
             ++event_index) {
            struct DM1_Event_V1 *existing = &queue->events[event_index];

            if (existing->type >= DM1_EVENT_CORRIDOR &&
                existing->type <= DM1_EVENT_DOOR) {
                if (same_map_time_and_xy(incoming_event, existing) &&
                    (existing->type != DM1_EVENT_WALL ||
                     existing->c_cell == incoming_event->c_cell)) {
                    existing->c_effect = incoming_event->c_effect;
                    return event_index;
                }
            } else if (existing->type == DM1_EVENT_DOOR_ANIMATION &&
                       same_map_time_and_xy(incoming_event, existing)) {
                if (incoming_event->c_effect == DM1_EFFECT_TOGGLE) {
                    incoming_event->c_effect =
                        (uint8_t)(1U - existing->c_effect);
                }
                if (!delete_event_if_timeline_owned(queue, event_index)) {
                    return REDMCSB_F0652_PC34_INVALID_QUEUE;
                }
                break;
            }
        }
    } else if (event_type == DM1_EVENT_DOOR_ANIMATION) {
        for (event_index = 0; event_index < largest_used_event_ordinal;
             ++event_index) {
            struct DM1_Event_V1 *existing = &queue->events[event_index];

            if (!same_map_time_and_xy(incoming_event, existing)) {
                continue;
            }
            if (existing->type == DM1_EVENT_DOOR) {
                if (existing->c_effect == DM1_EFFECT_TOGGLE) {
                    existing->c_effect = (uint8_t)(1U - incoming_event->c_effect);
                }
                return event_index;
            }
            if (existing->type == DM1_EVENT_DOOR_ANIMATION) {
                existing->c_effect = incoming_event->c_effect;
                return event_index;
            }
        }
    } else if (event_type == DM1_EVENT_DOOR_DESTRUCTION) {
        for (event_index = 0; event_index < largest_used_event_ordinal;
             ++event_index) {
            struct DM1_Event_V1 *existing = &queue->events[event_index];

            if (incoming_event->b_mapX == existing->b_mapX &&
                incoming_event->b_mapY == existing->b_mapY &&
                DM1_MAP_TIME_MAP(incoming_event->map_time) ==
                    DM1_MAP_TIME_MAP(existing->map_time) &&
                (existing->type == DM1_EVENT_DOOR_ANIMATION ||
                 existing->type == DM1_EVENT_DOOR) &&
                !delete_event_if_timeline_owned(queue, event_index)) {
                return REDMCSB_F0652_PC34_INVALID_QUEUE;
            }
        }
    }
    return REDMCSB_F0652_PC34_NO_MERGE;
}

const char *redmcsb_f0652_merge_event_pc34_source_evidence(void)
{
    return "ReDMCSB TIMELINE.C F0652_MergeEvent (lines 423-495); "
           "F0237_TIMELINE_DeleteEvent";
}
