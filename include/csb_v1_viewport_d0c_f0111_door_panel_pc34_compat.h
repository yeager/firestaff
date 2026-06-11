/*
 * CSB V1 D0C F0111 door-panel source-lock gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0111_DUNGEONVIEW_DrawDoor:4218-4339; base ornament at
 *   4262; Thieves-Eye branch at 4291-4294, gated by M631_ZONE_DOOR_D1C.
 * - DUNVIEW.C local D0C dispatch is F0127_DUNGEONVIEW_DrawSquareD0C:
 *   prototype 1983-1989 and body 8164-8311. The requested F0118 D0C name
 *   is not present in this Common/Source snapshot.
 * - DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions:
 *   4547-4581 describes the thing-pass loop. D0C calls it at 8294; this
 *   F0111 closed-door D0C contract does not introduce an F0115 call inside
 *   the door-panel path.
 * - DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8478-8508 and 8534-8542 keep the
 *   post-dispatch wall/near-square order stable before D0C.
 * - DUNVIEW.C F0104:3113-3156, F0105:3185-3247, F0107:3502-3938, and
 *   F0127:8164-8311 are cited as non-F0111 D0C wall/frame/field routes.
 * - DEFS.H:2088 C10_COLOR_FLESH; 4040-4057 wall zone ids; 4249-4261 door
 *   zone ids including M631_ZONE_DOOR_D1C.
 * - CSB-lineage Viewport.cpp:1903-1906 is the thing-pass anchor that must
 *   remain stable before StdDrawDoor in the center-door script.
 */
#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D0C_F0111_DOOR_PANEL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D0C_F0111_DOOR_PANEL_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_D0C_F0111_LINE_START_PC34 4218
#define CSB_V1_D0C_F0111_LINE_END_PC34 4339
#define CSB_V1_D0C_F0127_DISPATCH_LINE_START_PC34 8164
#define CSB_V1_D0C_F0127_DISPATCH_LINE_END_PC34 8311
#define CSB_V1_D0C_F0115_LINE_START_PC34 4547
#define CSB_V1_D0C_F0115_LINE_END_PC34 4581
#define CSB_V1_D0C_F0128_WALL_FOLLOWUP_START_PC34 8478
#define CSB_V1_D0C_F0128_WALL_FOLLOWUP_END_PC34 8508
#define CSB_V1_D0C_F0128_POST_DISPATCH_START_PC34 8534
#define CSB_V1_D0C_F0128_POST_DISPATCH_END_PC34 8542

#define CSB_V1_D0C_F0111_C10_COLOR_FLESH_PC34 10
#define CSB_V1_D0C_F0111_C6_UNKNOWN_PC34 6
#define CSB_V1_D0C_F0111_MASK0X4000_PC34 0x4000
#define CSB_V1_D0C_F0111_C15_DESTROYED_MASK_PC34 15
#define CSB_V1_D0C_F0111_C16_THIEVES_EYE_MASK_PC34 16
#define CSB_V1_D0C_F0111_DOOR_ZONE_PC34 608
#define CSB_V1_D0C_F0111_D1C_THIEVES_EYE_ZONE_PC34 3790
#define CSB_V1_D0C_F0111_WALL_ZONE_D0C_PC34 715
#define CSB_V1_D0C_F0111_VIEW_SQUARE_D0C_PC34 0
#define CSB_V1_D0C_F0111_F0115_CELL_ORDER_PC34 0x0021

typedef enum {
    CSB_V1_D0C_F0111_ELEMENT_EMPTY_CENTER_FIELD_PC34 = 1,
    CSB_V1_D0C_F0111_ELEMENT_DOOR_FRONT_PC34 = 17
} CSB_V1_D0CF0111DoorPanelElementPc34;

typedef enum {
    CSB_V1_D0C_F0111_BRANCH_NO_DOOR_PC34 = 0,
    CSB_V1_D0C_F0111_BRANCH_PARTLY_OPEN_PC34 = 1,
    CSB_V1_D0C_F0111_BRANCH_CLOSED_PC34 = 2,
    CSB_V1_D0C_F0111_BRANCH_DESTROYED_PC34 = 3,
    CSB_V1_D0C_F0111_BRANCH_OPEN_SKIP_PC34 = 4,
    CSB_V1_D0C_F0111_BRANCH_INVALID_PC34 = -1
} CSB_V1_D0CF0111DoorPanelBranchPc34;

typedef struct {
    bool source_locked_contract_only;
    bool no_real_asset_bitmap_parity;
    bool no_game_data_load;
    bool f0111_base_ornament_dispatches;
    bool d0c_thieves_eye_mask_rejected_by_zone;
    bool f0115_thing_pass_anchor_stable;
    bool csb_lineage_viewport_1903_1906_stable;
    bool partly_open_stays_on_f0111_not_wall_path;
    int f0111_line_start;
    int f0111_line_end;
    int f0127_line_start;
    int f0127_line_end;
    int f0115_line_start;
    int f0115_line_end;
    int f0128_wall_followup_start;
    int f0128_wall_followup_end;
    int f0128_post_dispatch_start;
    int f0128_post_dispatch_end;
    int door_zone_d0c;
    int thieves_eye_zone_d1c;
    int wall_zone_d0c;
    int view_square_d0c;
    int f0115_cell_order_d0c;
    int transparent_color;
    int c6_unknown;
    int mask0x4000_shift;
    int destroyed_mask_ordinal;
    int thieves_eye_mask_ordinal;
    int closed_ornament_ordinal_first;
    int closed_ornament_ordinal_last;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_f0127_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_anchor;
    const char *source_evidence;
} CSB_V1_D0CF0111DoorPanelContractPc34;

typedef struct {
    int element;
    int door_present;
    int door_state;
    int ornament_ordinal;
    int event73_count_thieves_eye;
    int horizontal_door;
} CSB_V1_D0CF0111DoorPanelInputPc34;

typedef struct {
    int capturedF0111;
    int capturedF0118WallPath;
    int capturedBaseOrnament;
    int capturedThievesEyeBranch;
    int capturedF0115InsideF0111;
    int f0115ThingPassAnchorStable;
    int csbLineageViewport1903ThingPassStable;
    int branch;
    int door_zone;
    int final_zone;
    int first_half_zone;
    int second_half_zone;
    int transparent_color;
    int ornament_ordinal;
    int destroyed_mask_ordinal;
    int no_door_center_field_sane;
    int wall_zone_d0c;
    const char *source_evidence;
} CSB_V1_D0CF0111DoorPanelStatePc34;

const CSB_V1_D0CF0111DoorPanelContractPc34 *
csb_v1_viewport_d0c_f0111_door_panel_contract_pc34(void);

const char *
csb_v1_viewport_d0c_f0111_door_panel_source_evidence_pc34(void);

CSB_V1_D0CF0111DoorPanelStatePc34
csb_v1_viewport_d0c_f0111_door_panel_dispatch_pc34(
    CSB_V1_D0CF0111DoorPanelInputPc34 input);

int csb_v1_viewport_d0c_f0111_door_panel_branch_pc34(int door_state);

int csb_v1_viewport_d0c_f0111_door_panel_closed_ornament_pc34(
    int door_state,
    int ornament_ordinal);

uint8_t csb_v1_viewport_d0c_f0111_door_panel_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_VIEWPORT_D0C_F0111_DOOR_PANEL_PC34_COMPAT_H */
