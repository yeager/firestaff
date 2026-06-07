#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_F0115_ITEM_EXPLOSION_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_F0115_ITEM_EXPLOSION_PC34_COMPAT_H

#include "csb_v1_viewport_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int view_lane;
    int object_g2028_row;
    int explosion_g2034_row;
    int field_aspect_index;
    int f0678_f0679_has_f0115_route;
    int f0678_f0679_has_f0111_route;
    int f0111_door_front_skips_item_explosion;
    int wall_zone;
    int non_f0107_back_wall_ornament_contract;
    int object_zone_base;
    int object_zone_cell_stride;
    int object_shift_mask;
    int object_requires_type_weapon_to_junk;
    int object_requires_cell_match;
    int object_rejects_missing_row;
    int object_pile_shift_advances;
    int object_uses_f0791_blit;
    int explosion_restarts_thing_list_after_cells;
    int explosion_rejects_missing_row_for_rebirth;
    int explosion_rebirth_step1_zone_base;
    int explosion_rebirth_step2_zone_base;
    int explosion_centered_zone_base;
    int explosion_side_zone_base;
    int explosion_side_zone_cell_stride;
    int explosion_uses_f0791_blit;
    int fluxcage_defers_to_field;
    int fluxcage_field_zone;
    int transparent_color;
    int csb_lineage_relative_cell;
    int csb_lineage_contents_opcode;
    int csb_lineage_std_draw_room_objects_opcode;
    int csb_lineage_door_front_rear_order_opcode;
    int csb_lineage_door_front_front_order_opcode;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec;

size_t csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_count_pc34(void);

const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *
csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_at_pc34(size_t index);

const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *
csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_for_square_pc34(int view_square);

int csb_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec,
    unsigned char view_cell);

int csb_v1_viewport_d2l2_d2r2_f0115_item_layout_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec,
    unsigned char view_cell);

int csb_v1_viewport_d2l2_d2r2_f0115_explosion_rebirth_step1_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec);

int csb_v1_viewport_d2l2_d2r2_f0115_explosion_rebirth_step2_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec);

int csb_v1_viewport_d2l2_d2r2_f0115_explosion_centered_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec);

int csb_v1_viewport_d2l2_d2r2_f0115_explosion_side_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec,
    unsigned char view_cell);

int csb_v1_viewport_d2l2_d2r2_f0115_apply_c10_blit_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

const char *csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
