#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1L_D1R_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1L_D1R_F0111_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked contract-only gate, not real-asset bitmap parity.
 * ReDMCSB anchors: DUNVIEW.C F0122_DUNGEONVIEW_DrawSquareD1L lines
 * 7492-7508 and 7520-7536; F0123_DUNGEONVIEW_DrawSquareD1R lines
 * 7660-7676 and 7688-7704; F0111_DUNGEONVIEW_DrawDoor lines 4218-4337.
 * DEFS.H anchors: lines 2599-2601, 2791, 4258-4260, and 2078-2088.
 * COORD.C anchors: lines 780-877 and 1548-1567. CSB-lineage
 * Viewport.cpp anchors: lines 1892-1900 and 1919-1927.
 */

typedef struct {
    const char *contract_scope;
    const char *f0122_d1l_lines;
    const char *f0123_d1r_lines;
    const char *f0111_lines;
    const char *defs_lines;
    const char *coord_lines;
    const char *lineage_open;
    const char *lineage_teleporter;
    const char *frame_binding;
} CSB_V1_ViewportD1LD1RF0111DoorPc34CompatEvidence;

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int view_square;
    int view_depth;
    int view_lane;
    int f0111_route_present;
    int door_front_rear_f0115_order;
    int door_front_f0111_order;
    int door_front_front_f0115_order;
    int floor_ornament_before_rear_pass;
    int door_frame_top_zone;
    int door_zone_base;
    int view_door_ornament_index;
    int door_graphic_depth_index;
    int field_zone;
    int open_state_skips_f0111;
    int closed_state_uses_base_zone;
    int destroyed_state_uses_base_zone;
    int destroyed_state_applies_c15_mask;
    int partial_state_shifts_zone;
    int horizontal_second_half_mask;
    int transparent_color;
    const char *route_name;
    const char *source_lines;
} CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant;

size_t csb_v1_viewport_d1l_d1r_f0111_door_pc34_count(void);
const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_at(size_t index);
const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(int view_square);

int csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *invariant,
    int door_state);
int csb_v1_viewport_d1l_d1r_f0111_door_horizontal_half_zone_pc34(
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *invariant,
    int door_state,
    int right_half);
int csb_v1_viewport_d1l_d1r_f0111_door_apply_c10_blit_pc34(
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *invariant,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatEvidence *
csb_v1_viewport_d1l_d1r_f0111_door_evidence_pc34(void);
const char *csb_v1_viewport_d1l_d1r_f0111_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
