#ifndef DM1_V1_GROUP_ACTIVE_STATE_PC34_COMPAT_H
#define DM1_V1_GROUP_ACTIVE_STATE_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_F0183_AddActiveGroupReceipt_PC34 {
    int valid;
    int active_index;
    int group_index;
    int map_x;
    int map_y;
    int cells;
    int directions;
    int last_move_time;
    int source_line_start;
    int source_line_end;
    const char *source_symbol;
} DM1_V1_F0183_AddActiveGroupReceipt_PC34;

typedef struct DM1_V1_F0184_RemoveActiveGroupReceipt_PC34 {
    int valid;
    int group_index;
    int restored_cells;
    int restored_direction;
    int reset_behavior_to_wander;
    int retired_active_slot;
    int source_line_start;
    int source_line_end;
    const char *source_symbol;
} DM1_V1_F0184_RemoveActiveGroupReceipt_PC34;

typedef struct DM1_V1_F0180_StartWanderingReceipt_PC34 {
    int valid;
    int group_index;
    int creature_type;
    int map_index;
    int map_x;
    int map_y;
    uint32_t fire_at_tick;
    int event_kind;
    int event_type;
    int source_line_start;
    int source_line_end;
    const char *source_symbol;
} DM1_V1_F0180_StartWanderingReceipt_PC34;

int F0180_DM1_GROUP_StartWandering_Compat(
    int group_index,
    int creature_type,
    int map_index,
    int map_x,
    int map_y,
    uint32_t game_time,
    struct TimelineEvent_Compat *out_event,
    DM1_V1_F0180_StartWanderingReceipt_PC34 *out_receipt);

int F0183_DM1_GROUP_AddActiveGroup_Compat(
    struct DM1ActiveGroup_Compat *active_groups,
    int active_group_capacity,
    int *current_active_group_count,
    const struct DungeonGroup_Compat *group,
    int group_index,
    int map_x,
    int map_y,
    uint32_t game_time,
    DM1_V1_F0183_AddActiveGroupReceipt_PC34 *out_receipt);

int F0184_DM1_GROUP_RemoveActiveGroup_Compat(
    struct DM1ActiveGroup_Compat *active_group,
    struct DungeonGroup_Compat *groups,
    int group_count,
    DM1_V1_F0184_RemoveActiveGroupReceipt_PC34 *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
