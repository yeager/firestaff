#ifndef FIRESTAFF_CSB_V1_F0797_STARTEND_ENTRANCE_MICRO_DUNGEON_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0797_STARTEND_ENTRANCE_MICRO_DUNGEON_PC34_COMPAT_H

#include <stdint.h>

#include "csb_v1_f0439_f0441_f0442_startend_entrance_boundaries_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_F0797_ENTRANCE_MAP_INDEX_PC34 = 255,
    CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34 = 5,
    CSB_V1_F0797_MICRO_DUNGEON_HEIGHT_PC34 = 5,
    CSB_V1_F0797_MICRO_DUNGEON_SQUARE_COUNT_PC34 = 25,
    CSB_V1_F0797_MICRO_DUNGEON_CORRIDOR_COUNT_PC34 = 6,
    CSB_V1_F0797_MICRO_DUNGEON_WALL_COUNT_PC34 = 19,
    CSB_V1_F0797_CORRIDOR_ROW_Y_PC34 = 2,
    CSB_V1_F0797_CORRIDOR_SPUR_X_PC34 = 2,
    CSB_V1_F0797_CORRIDOR_SPUR_Y_PC34 = 1,
    CSB_V1_F0797_VIEW_DIRECTION_SOUTH_PC34 = 2,
    CSB_V1_F0797_VIEW_X_PC34 = 2,
    CSB_V1_F0797_VIEW_Y_PC34 = 0
};

typedef enum CSB_V1_F0797_MicroDungeonSquareKind_PC34 {
    CSB_V1_F0797_MICRO_DUNGEON_SQUARE_WALL_PC34 = 0,
    CSB_V1_F0797_MICRO_DUNGEON_SQUARE_CORRIDOR_PC34 = 1
} CSB_V1_F0797_MicroDungeonSquareKind_PC34;

typedef struct CSB_V1_F0797_EntranceMicroDungeonFacts_PC34 {
    int valid;
    int entrance_map_index;
    int map_width;
    int map_height;
    int square_count;
    int wall_square_count;
    int corridor_square_count;
    uint32_t corridor_square_mask;
    int current_map_data_row_count;
    int draw_floor_and_ceiling_requested;
    int current_map_pointer_owned_by_micro_dungeon;
    int current_map_not_loaded_dungeon;
    int source_wall_fill_reviewed;
    int source_corridor_row_reviewed;
    int source_corridor_spur_reviewed;
    int draw_cpsf_route_reviewed;
    int view_direction;
    int view_x;
    int view_y;
    int no_loaded_dungeon_substitute;
    int no_synthetic_viewport_pixels;
    int no_legacy_micro_dungeon_wrapper;
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 entrance_boundary;
} CSB_V1_F0797_EntranceMicroDungeonFacts_PC34;

typedef struct CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34 {
    int valid;
    int entrance_map_index;
    int map_width;
    int map_height;
    int square_count;
    int wall_square_count;
    int corridor_square_count;
    uint32_t corridor_square_mask;
    int draw_floor_and_ceiling_requested;
    int view_direction;
    int view_x;
    int view_y;
    int entrance_boundary_consumed;
    int no_loaded_dungeon_substitute;
    int no_synthetic_viewport_pixels;
    int no_legacy_micro_dungeon_wrapper;
    const char *source_evidence;
} CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34;

void csb_v1_f0797_entrance_micro_dungeon_receipt_init_pc34(
    CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34 *receipt);

int F0797_STARTEND_DrawEntranceMicroDungeon(
    const CSB_V1_F0797_EntranceMicroDungeonFacts_PC34 *facts,
    CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34 *out_receipt);

int csb_v1_f0797_entrance_micro_dungeon_square_kind_pc34(int x, int y);
uint32_t csb_v1_f0797_entrance_micro_dungeon_corridor_mask_pc34(void);
const char *csb_v1_f0797_entrance_micro_dungeon_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0797_STARTEND_ENTRANCE_MICRO_DUNGEON_PC34_COMPAT_H */
