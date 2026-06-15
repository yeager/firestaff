#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_F0098_FALLBACK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_F0098_FALLBACK_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D2L_D2R_F0098_SIDE_D2L_PC34 = 0,
    DM1_V1_D2L_D2R_F0098_SIDE_D2R_PC34 = 1
} DM1_V1_D2LD2RF0098SidePc34;

typedef enum {
    DM1_V1_D2L_D2R_F0098_ELEMENT_CORRIDOR_PC34 = 1,
    DM1_V1_D2L_D2R_F0098_ELEMENT_PIT_PC34 = 2,
    DM1_V1_D2L_D2R_F0098_ELEMENT_TELEPORTER_PC34 = 5,
    DM1_V1_D2L_D2R_F0098_ELEMENT_DOOR_SIDE_PC34 = 16,
    DM1_V1_D2L_D2R_F0098_ELEMENT_DOOR_FRONT_PC34 = 17,
    DM1_V1_D2L_D2R_F0098_ELEMENT_STAIRS_SIDE_PC34 = 18,
    DM1_V1_D2L_D2R_F0098_ELEMENT_STAIRS_FRONT_PC34 = 19
} DM1_V1_D2LD2RF0098ElementPc34;

typedef enum {
    DM1_V1_D2L_D2R_F0098_OP_F0098_FLOOR_CEILING_PC34 = 0,
    DM1_V1_D2L_D2R_F0098_OP_F0099_FLOOR_CEILING_FLIP_PC34 = 1,
    DM1_V1_D2L_D2R_F0098_OP_F0119_OR_F0120_NON_WALL_PC34 = 2,
    DM1_V1_D2L_D2R_F0098_OP_F0104_FLOOR_PIT_OR_STAIRS_PC34 = 3,
    DM1_V1_D2L_D2R_F0098_OP_F0108_FLOOR_ORNAMENT_PC34 = 4,
    DM1_V1_D2L_D2R_F0098_OP_F0112_CEILING_PIT_PC34 = 5,
    DM1_V1_D2L_D2R_F0098_OP_F0115_THINGS_PC34 = 6,
    DM1_V1_D2L_D2R_F0098_OP_F0113_TELEPORTER_FIELD_PC34 = 7,
    DM1_V1_D2L_D2R_F0098_OP_F0111_DOOR_PC34 = 8,
    DM1_V1_D2L_D2R_F0098_OP_F0097_PRESENT_PC34 = 9
} DM1_V1_D2LD2RF0098OpPc34;

typedef struct {
    DM1_V1_D2LD2RF0098OpPc34 op;
    int order_index;
    const char *function_name;
    const char *source_line;
    const char *condition;
} DM1_V1_D2LD2RF0098OrderStepPc34;

typedef struct {
    DM1_V1_D2LD2RF0098SidePc34 side;
    int depth;
    int lateral;
    int view_square_index;
    int view_floor_index;
    int wall_zone_index;
    int viewport_ceiling_zone;
    int viewport_floor_zone;
    int floor_ornament_zone_base;
    int floor_pit_zone;
    int ceiling_pit_zone;
    int stairs_up_front_zone;
    int stairs_down_front_zone;
    int stairs_side_zone;
    int corridor_cell_order;
    int door_side_cell_order;
    int door_pass1_cell_order;
    int door_pass2_cell_order;
    int f0098_order_index;
    int f0099_order_index;
    int square_dispatch_order_index;
    int f0097_present_order_index;
    bool contract_only;
    bool real_asset_graphics_dat_required;
    bool side_cell_must_not_be_wall;
    bool f0098_guarded_by_floor_ceiling_dirty_flag;
    bool f0107_wall_ornament_excluded_on_non_wall_path;
    const char *view_square_symbol;
    const char *view_floor_symbol;
    const char *wall_zone_symbol;
    const char *graphics_dat_sha256;
    const char *floor_bitmap_symbol;
    const char *ceiling_bitmap_symbol;
    const char *source_lines;
} DM1_V1_D2LD2RF0098FallbackSpecPc34;

const DM1_V1_D2LD2RF0098FallbackSpecPc34 *
dm1_v1_viewport_d2l_d2r_f0098_fallback_spec_pc34(
    DM1_V1_D2LD2RF0098SidePc34 side);

const DM1_V1_D2LD2RF0098OrderStepPc34 *
dm1_v1_viewport_d2l_d2r_f0098_fallback_order_pc34(
    DM1_V1_D2LD2RF0098SidePc34 side,
    DM1_V1_D2LD2RF0098ElementPc34 element,
    size_t *count);

bool dm1_v1_viewport_d2l_d2r_f0098_should_draw_pc34(
    bool floor_ceiling_dirty_flag);

const char *dm1_v1_viewport_d2l_d2r_f0098_fallback_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_F0098_FALLBACK_PC34_COMPAT_H */
