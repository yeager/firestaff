/*
 * DM1 V1 PC 3.4 contract-only source-lock gate for the D0L2/D0R2
 * corridor-side F0111 partly-open door pair.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:4218-4339 F0111_DUNGEONVIEW_DrawDoor; line 4248 skips
 *   fully-open doors, line 4308 decrements partly-open states, lines
 *   4312-4313 select LeftHorizontal/RightHorizontal frame bitmaps, lines
 *   4317-4325 do the C10 transparent side-pair zone flow, and line 4334
 *   performs the final C10 draw.
 * - DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L and 8064-8162
 *   F0126_DUNGEONVIEW_DrawSquareD0R are the D0L/D0R corridor-side callers.
 * - DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF dispatches the left
 *   side before the right side; lines 8536-8541 are the D0L/D0R pair, and
 *   the 8511-8521 L/R pattern is the same source-locked side-pair ordering.
 * - DUNGEON.C:2466-2722 F0172_DUNGEON_SetSquareAspect provides the
 *   door-side C16 path, fakewall conversion guard, and M556/M557 door
 *   state/index slots.
 * - DEFS.H:1039-1047 C0..C5 door states and M036_DOOR_STATE; 2088
 *   C10_COLOR_FLESH; 2596-2601 M609/M610/M611 view-square ordinals;
 *   2657-2675 cell-order constants including C0x0218/C0x0349.
 *
 * This gate is synthetic and asset-free. It makes no real-asset or
 * original-DOS pixel-parity claim.
 */
#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D0L2_D0R2_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0L2_D0R2_F0111_PARTLY_OPEN_MASK0X4000_PC34 0x4000
#define DM1_V1_D0L2_D0R2_F0111_PARTLY_OPEN_C0X0218_PC34 0x0218
#define DM1_V1_D0L2_D0R2_F0111_PARTLY_OPEN_C0X0349_PC34 0x0349

typedef enum {
    DM1_V1_D0L2_D0R2_F0111_PARTLY_OPEN_SIDE_D0L2_PC34 = 1,
    DM1_V1_D0L2_D0R2_F0111_PARTLY_OPEN_SIDE_D0R2_PC34 = 2
} DM1_V1_D0L2D0R2F0111PartlyOpenSidePc34;

typedef enum {
    DM1_V1_D0L2_D0R2_F0111_BRANCH_OPEN_PC34 = 0,
    DM1_V1_D0L2_D0R2_F0111_BRANCH_PARTLY_OPEN_PC34 = 1,
    DM1_V1_D0L2_D0R2_F0111_BRANCH_CLOSED_PC34 = 2,
    DM1_V1_D0L2_D0R2_F0111_BRANCH_DESTROYED_PC34 = 3,
    DM1_V1_D0L2_D0R2_F0111_BRANCH_FAKEWALL_REJECT_PC34 = 4,
    DM1_V1_D0L2_D0R2_F0111_BRANCH_INVALID_PC34 = -1
} DM1_V1_D0L2D0R2F0111BranchPc34;

typedef enum {
    DM1_V1_D0L2_D0R2_F0111_THING_ITEM_PC34 = 1,
    DM1_V1_D0L2_D0R2_F0111_THING_CREATURE_PC34 = 2,
    DM1_V1_D0L2_D0R2_F0111_THING_PROJECTILE_PC34 = 3
} DM1_V1_D0L2D0R2F0111ThingKindPc34;

typedef struct {
    int side;
    const char *route_name;
    int source_locked_contract_only;
    int no_real_asset_pixel_parity;
    int no_game_data_load;
    int view_square;
    int f0125_f0126_function_number;
    int f0128_dispatch_order;
    int f0128_dispatch_line;
    int relative_depth;
    int relative_lateral;
    int rear_cell_order;
    int front_cell_order;
    int door_zone_base;
    int wall_zone;
    int open_state;
    int partly_open_state_one;
    int partly_open_state_two;
    int partly_open_state_three;
    int closed_state;
    int destroyed_state;
    int open_degree_closed;
    int open_degree_step_one;
    int open_degree_step_two;
    int open_degree_step_three;
    int open_degree_step_four;
    int open_degree_open;
    int first_half_zone_offset;
    int second_half_zone_offset;
    int second_half_zone_mask;
    int c10_transparent_color;
    int row_guard_item;
    int row_guard_creature;
    int row_guard_projectile;
    const char *left_horizontal_frame_bitmap;
    const char *right_horizontal_frame_bitmap;
    const char *f0111_anchor;
    const char *f0125_f0126_anchor;
    const char *f0128_anchor;
    const char *dungeon_anchor;
    const char *defs_anchor;
    const char *door_bash_anchor;
} DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34;

typedef struct {
    int ok;
    int branch;
    int side;
    int open_degree;
    int door_band_count;
    int first_half_zone;
    int second_half_zone;
    int first_half_c10_skips;
    int second_half_c10_skips;
    int copied_pixels;
    int row_guard_item;
    int row_guard_creature;
    int row_guard_projectile;
    int mutation_rejections;
    int door_bash_chain_ok;
    uint32_t deterministic_hash;
} DM1_V1_D0L2D0R2F0111TracePc34;

typedef struct {
    int assertions;
    int failures;
    int ramp_ticks_checked;
    int total_door_bands;
    int d0l2_first_dispatch_ok;
    int d0r2_second_dispatch_ok;
    int c10_transparency_ok;
    int row_guards_ok;
    int fakewall_rejection_ok;
    int closed_edge_ok;
    int open_edge_ok;
    int door_bash_chain_ok;
    uint32_t deterministic_hash;
} DM1_V1_D0L2D0R2F0111SelfTestResultPc34;

size_t dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34(void);

const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_at_pc34(size_t index);

const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(int side);

int dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_branch_pc34(
    const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *spec,
    int open_degree,
    int neighbor_wall_is_fakewall);

int dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_row_guard_pc34(
    const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *spec,
    int thing_kind,
    int behind_partly_open_door_band);

int dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_trace_pc34(
    const DM1_V1_D0L2D0R2F0111PartlyOpenSpecPc34 *spec,
    int open_degree,
    int neighbor_wall_is_fakewall,
    DM1_V1_D0L2D0R2F0111TracePc34 *out_trace);

uint32_t dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_hash_pc34(void);

const char *
dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_source_evidence_pc34(void);

int run_dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_self_test(void);

const DM1_V1_D0L2D0R2F0111SelfTestResultPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_partly_open_door_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
