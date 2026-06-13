#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_F0111_PARTLY_OPEN_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 PC 3.4 contract-only source-lock gate for F0111 front doors at
 * D1L/D1R (depth 1, lateral +/-1).
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; 4248 skips open
 *   doors, 4297-4305 draws closed/destroyed through ClosedOrDestroyed,
 *   4308 decrements C1/C2/C3 partly-open states, 4310-4313 selects
 *   vertical or LeftHorizontal/RightHorizontal frames, 4317-4324 applies
 *   state-adjusted zone + C6_UNKNOWN for the first half with
 *   C10_COLOR_FLESH, and 4325-4334 applies 3|MASK0x4000 before the final
 *   C10 F0791_DUNGEONVIEW_DrawBitmapXX blit.
 * - DUNVIEW.C:7391-7557 F0122_DUNGEONVIEW_DrawSquareD1L and
 *   7559-7725 F0123_DUNGEONVIEW_DrawSquareD1R; door-front branches are
 *   7492-7508 and 7660-7676, with F0115 pass-2 at 7536 and 7704.
 * - DUNVIEW.C:4788-4804 and 4916-4923 F0115 door-front cell-order pass
 *   decoding, object visibility guards, and C10 blit at 5176-5188.
 * - DUNVIEW.C:8524-8533 F0128 dispatches D1L, D1R, then D1C.
 * - DUNVIEW.C:3113-3156 F0104, 3185-3247 F0105, 3502-3938 F0107, and
 *   3940-4011 F0108 are the surrounding floor/wall ornament keepouts.
 * - DUNGEON.C:1769-1838 F0163, 1840-1905 F0164, 2466-2523 F0172 provide
 *   thing-list and square-aspect inputs for F0115/F0122/F0123.
 * - DEFS.H:2088 C10_COLOR_FLESH; 2596-2611 view-square ordinals;
 *   2661-2667/2672-2675 cell orders; 2789-2791 door ornament views;
 *   3508 C6_UNKNOWN; 3516 MASK0x4000; 4091-4093 frame-top zones;
 *   4258-4260 D1 door zones; 5458 G0695; 5542/5544 G0185/G0187.
 *
 * This gate is synthetic and asset-free. It makes no real-asset or
 * original-DOS pixel parity claim.
 */

#define DM1_V1_D1L_D1R_F0111_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D1L_D1R_F0111_C6_UNKNOWN_PC34 6
#define DM1_V1_D1L_D1R_F0111_MASK0X4000_PC34 0x4000

typedef enum {
    DM1_V1_D1L_D1R_F0111_SIDE_D1L_PC34 = 1,
    DM1_V1_D1L_D1R_F0111_SIDE_D1R_PC34 = 2
} DM1_V1_D1LD1RF0111SidePc34;

typedef enum {
    DM1_V1_D1L_D1R_F0111_BRANCH_REJECTED_PC34 = -1,
    DM1_V1_D1L_D1R_F0111_BRANCH_OPEN_PC34 = 0,
    DM1_V1_D1L_D1R_F0111_BRANCH_PARTLY_OPEN_PC34 = 1,
    DM1_V1_D1L_D1R_F0111_BRANCH_CLOSED_PC34 = 2
} DM1_V1_D1LD1RF0111BranchPc34;

typedef struct {
    int side;
    const char *name;
    int view_square;
    int view_depth;
    int relative_lateral;
    int floor_ornament_view;
    int door_zone;
    int door_frame_top_zone;
    int doorpass1_order;
    int doorpass2_order;
    int corridor_order;
    int f0128_dispatch_line;
    int f012x_door_front_line;
    int f012x_f0111_line;
    int f012x_f0115_pass2_line;
    int door_ornament_view;
    int door_width;
    int door_height;
    int door_byte_count;
    const char *frames_symbol;
} DM1_V1_D1LD1RF0111DoorSpecPc34;

typedef struct {
    int accepted;
    int side;
    int input_state;
    int branch;
    int decremented_state;
    int view_square;
    int door_zone;
    int first_half_base_zone;
    int first_half_clip_zone;
    int second_half_zone;
    int second_half_shift;
    int c10_color;
    int closed_frame_selected;
    int left_horizontal_frame_selected;
    int right_horizontal_frame_selected;
    int doorpass1_order;
    int doorpass2_order;
    int first_half_c10_skips;
    int second_half_c10_skips;
    int first_half_writes;
    int second_half_writes;
    uint8_t framebuffer_probe_a;
    uint8_t framebuffer_probe_b;
} DM1_V1_D1LD1RF0111DoorTracePc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int d1l_closed;
    int d1r_closed;
    int d1l_partly;
    int d1r_partly;
    int open_rejections;
    int destroyed_rejections;
    int unknown_rejections;
    int out_of_range_square_rejections;
    int unsupported_element_rejections;
    int c10_first_half_skips;
    int c10_second_half_skips;
    int f0115_doorpass_anchors;
    int f0128_dispatch_anchors;
} DM1_V1_D1LD1RF0111DoorSelfTestResultPc34;

size_t dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_count_pc34(void);

const DM1_V1_D1LD1RF0111DoorSpecPc34 *
dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_at_pc34(size_t index);

const DM1_V1_D1LD1RF0111DoorSpecPc34 *
dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_side_pc34(int side);

const DM1_V1_D1LD1RF0111DoorSpecPc34 *
dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_spec_for_square_pc34(
    int view_square);

int dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_supported_element_pc34(
    int element);

int dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_trace_pc34(
    int side,
    int door_state,
    int element,
    DM1_V1_D1LD1RF0111DoorTracePc34 *out_trace);

const char *
dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_source_evidence_pc34(void);

int run_dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_self_test(void);

const DM1_V1_D1LD1RF0111DoorSelfTestResultPc34 *
dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
