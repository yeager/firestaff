#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D0L_D0R_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D0L_D0R_F0111_DOOR_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked contract-only gate, not real-asset bitmap parity.
 * Chosen slice: D0L/D0R F0111 door absence/front-clip boundary.
 * ReDMCSB anchors: DUNVIEW.C F0125 lines 7976-8060; F0126 lines
 * 8080-8159; F0107 lines 3502-3938; F0111 lines 4218-4337.
 * CSBWin anchor: Viewport.cpp lines 1903-1915 center-door StdDrawDoor
 * dispatch. CSB-lineage anchors: Viewport.cpp lines 1930-1944 D0 side
 * returns and 6503-6551 CustomBackgrounds/ApplyDecoration separation.
 */

typedef struct {
    const char *contract_scope;
    const char *d0l_lines;
    const char *d0r_lines;
    const char *f0107_lines;
    const char *f0111_lines;
    const char *csbwin_center_door_lines;
    const char *csb_lineage_d0_side_lines;
    const char *csb_lineage_custom_backgrounds_lines;
} CSB_V1_ViewportD0LD0RF0111DoorPc34CompatEvidence;

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int view_square;
    int view_depth;
    int view_lane;
    int f0111_route_present;
    int f0107_route_present;
    int corridor_routes_f0115;
    int door_side_routes_f0115;
    int f0115_cell_order;
    int wall_zone;
    int field_zone;
    int ceiling_pit_zone;
    int door_zone_present;
    int door_frame_zone_present;
    int csbwin_d0_side_returns;
    int csbwin_center_door_is_contrast;
    int custom_backgrounds_are_separate;
    const char *route_name;
    const char *source_lines;
} CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant;

size_t csb_v1_viewport_d0l_d0r_f0111_door_pc34_count(void);
const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *
csb_v1_viewport_d0l_d0r_f0111_door_pc34_at(size_t index);
const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *
csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(int view_square);

int csb_v1_viewport_d0l_d0r_f0111_door_allows_f0111_pc34(
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *invariant,
    int element);
const char *csb_v1_viewport_d0l_d0r_f0111_door_route_label_pc34(
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *invariant);

const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatEvidence *
csb_v1_viewport_d0l_d0r_f0111_door_evidence_pc34(void);
const char *csb_v1_viewport_d0l_d0r_f0111_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
