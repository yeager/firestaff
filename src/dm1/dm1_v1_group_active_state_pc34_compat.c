#include "dm1_v1_group_active_state_pc34_compat.h"

#include <string.h>

static int pack_direction_for_group_pc34(int direction, int creature_count)
{
    int packed = 0;
    int creature_index;
    int last_index = creature_count;

    if (last_index < 0) last_index = 0;
    if (last_index > 3) last_index = 3;
    for (creature_index = 0; creature_index <= last_index; ++creature_index) {
        packed |= (direction & 3) << (creature_index * 2);
    }
    return packed;
}

int F0180_DM1_GROUP_StartWandering_Compat(
    int group_index,
    int creature_type,
    int map_index,
    int map_x,
    int map_y,
    uint32_t game_time,
    struct TimelineEvent_Compat *out_event,
    DM1_V1_F0180_StartWanderingReceipt_PC34 *out_receipt)
{
    struct TimelineEvent_Compat event;

    if (out_event) memset(out_event, 0, sizeof(*out_event));
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_event || !out_receipt || group_index < 0 ||
        creature_type < 0 || map_index < 0 || map_x < 0 || map_y < 0) {
        return 0;
    }

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_TICK;
    event.fireAtTick = game_time + 1u;
    event.mapIndex = map_index;
    event.mapX = map_x;
    event.mapY = map_y;
    event.aux0 = group_index;
    event.aux1 = creature_type;
    event.aux2 = DM1_BEHAVIOR_WANDER;
    *out_event = event;

    out_receipt->valid = 1;
    out_receipt->group_index = group_index;
    out_receipt->creature_type = creature_type;
    out_receipt->map_index = map_index;
    out_receipt->map_x = map_x;
    out_receipt->map_y = map_y;
    out_receipt->fire_at_tick = event.fireAtTick;
    out_receipt->event_kind = event.kind;
    out_receipt->event_type = event.aux2;
    out_receipt->source_line_start = 311;
    out_receipt->source_line_end = 340;
    out_receipt->source_symbol = "F0180_GROUP_StartWandering";
    return 1;
}

int F0183_DM1_GROUP_AddActiveGroup_Compat(
    struct DM1ActiveGroup_Compat *active_groups,
    int active_group_capacity,
    int *current_active_group_count,
    const struct DungeonGroup_Compat *group,
    int group_index,
    int map_x,
    int map_y,
    uint32_t game_time,
    DM1_V1_F0183_AddActiveGroupReceipt_PC34 *out_receipt)
{
    struct DM1ActiveGroup_Compat *active;
    int active_index;
    int directions;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!active_groups || !current_active_group_count || !group ||
        !out_receipt || active_group_capacity < 0 ||
        *current_active_group_count < 0 ||
        *current_active_group_count > active_group_capacity ||
        group_index < 0 || group->count > 3) {
        return 0;
    }
    if (*current_active_group_count >= active_group_capacity) return 0;

    active_index = *current_active_group_count;
    directions = pack_direction_for_group_pc34(group->direction, group->count);
    active = &active_groups[active_index];
    memset(active, 0, sizeof(*active));
    active->groupThingIndex = group_index;
    active->directions = directions;
    active->cells = group->cells;
    active->lastMoveTime = (int)(game_time & 0xffu);
    active->targetMapX = map_x;
    active->targetMapY = map_y;
    active->priorMapX = map_x;
    active->priorMapY = map_y;
    active->homeMapX = map_x;
    active->homeMapY = map_y;
    ++*current_active_group_count;

    out_receipt->valid = 1;
    out_receipt->active_index = active_index;
    out_receipt->group_index = group_index;
    out_receipt->map_x = map_x;
    out_receipt->map_y = map_y;
    out_receipt->cells = group->cells;
    out_receipt->directions = directions;
    out_receipt->last_move_time = active->lastMoveTime;
    out_receipt->source_line_start = 414;
    out_receipt->source_line_end = 447;
    out_receipt->source_symbol = "F0183_GROUP_AddActiveGroup";
    return 1;
}

int F0184_DM1_GROUP_RemoveActiveGroup_Compat(
    struct DM1ActiveGroup_Compat *active_group,
    struct DungeonGroup_Compat *groups,
    int group_count,
    DM1_V1_F0184_RemoveActiveGroupReceipt_PC34 *out_receipt)
{
    struct DungeonGroup_Compat *group;
    int group_index;
    int reset_behavior;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!active_group || !groups || !out_receipt || group_count < 0) {
        return 0;
    }
    group_index = active_group->groupThingIndex;
    if (group_index < 0 || group_index >= group_count) return 0;

    group = &groups[group_index];
    reset_behavior = group->behavior >= DM1_BEHAVIOR_USELESS4;
    group->cells = (unsigned char)(active_group->cells & 0xff);
    group->direction = (unsigned char)(active_group->directions & 0x03);
    if (reset_behavior) group->behavior = DM1_BEHAVIOR_WANDER;
    active_group->groupThingIndex = -1;

    out_receipt->valid = 1;
    out_receipt->group_index = group_index;
    out_receipt->restored_cells = group->cells;
    out_receipt->restored_direction = group->direction;
    out_receipt->reset_behavior_to_wander = reset_behavior;
    out_receipt->retired_active_slot = 1;
    out_receipt->source_line_start = 447;
    out_receipt->source_line_end = 479;
    out_receipt->source_symbol = "F0184_GROUP_RemoveActiveGroup";
    return 1;
}
