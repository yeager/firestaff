#ifndef FIRESTAFF_DM1_V1_VIEWPORT_F0111_DOOR_TRANSPARENCY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_F0111_DOOR_TRANSPARENCY_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 F0111 door transparency source-lock gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0111:4218-4337 draws the door panel; lines 4317-4318
 *   add the decremented partly-open state to P2084_i_ZoneIndex, lines
 *   4320-4324 perform the first horizontal half blit through zone +
 *   C6_UNKNOWN with C10_COLOR_FLESH transparency, line 4325 shifts the
 *   final half by 3 | MASK0x4000, and line 4334 performs the final C10
 *   transparent viewport blit.
 * - DUNVIEW.C F0121:7244-7389 is cited only for the non-D2C row check:
 *   D2C owns M628 and must stay disjoint from D1C and D2L/D2R row routes.
 * - DUNVIEW.C F0128:8508-8533 dispatches D2L/D2R, D2C, then D1L/D1R/D1C.
 * - DUNGEON.C F0163:1769-1838 and F0164:1840-1905 pin the map-square
 *   thing-list byte used by door cells before F0172 exposes M556/M557.
 * - DEFS.H:2088 pins C10_COLOR_FLESH; 2605-2606 pin the D2L2/D2R2 wall
 *   ordinals that must not claim the D2L/D2R transparency route; 4047-4048
 *   pin the D2L2/D2R2 wall zones; 3508 and 3516 pin C6_UNKNOWN and
 *   MASK0x4000.
 *
 * This gate is asset-free. It does not claim real-asset or original-DOS
 * pixel parity, and it does not duplicate the per-square D2C, D1C, D0C,
 * D1L2/D1R2, D2L/D2R, D2L2/D2R2, or D3C F0111 composition gates.
 */

#define DM1_V1_F0111_DOOR_TRANSPARENCY_C10_COLOR_FLESH_PC34 10
#define DM1_V1_F0111_DOOR_TRANSPARENCY_C6_UNKNOWN_PC34 6
#define DM1_V1_F0111_DOOR_TRANSPARENCY_MASK0X4000_PC34 0x4000
#define DM1_V1_F0111_DOOR_TRANSPARENCY_THING_LIST_PRESENT_PC34 0x0010

typedef enum {
    DM1_V1_F0111_DOOR_TRANSPARENCY_ROUTE_D1C_VERTICAL_PC34 = 0,
    DM1_V1_F0111_DOOR_TRANSPARENCY_ROUTE_D1C_HORIZONTAL_PC34 = 1,
    DM1_V1_F0111_DOOR_TRANSPARENCY_ROUTE_D2L_HORIZONTAL_PC34 = 2,
    DM1_V1_F0111_DOOR_TRANSPARENCY_ROUTE_D2R_HORIZONTAL_PC34 = 3
} DM1_V1_F0111DoorTransparencyRoutePc34;

typedef enum {
    DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_OPEN_PC34 = 0,
    DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_PARTLY_VERTICAL_PC34 = 1,
    DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_PARTLY_HORIZONTAL_PC34 = 2,
    DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_CLOSED_PC34 = 3,
    DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_INVALID_PC34 = -1
} DM1_V1_F0111DoorTransparencyBranchPc34;

typedef struct {
    DM1_V1_F0111DoorTransparencyRoutePc34 route;
    const char *label;
    int view_square;
    int relative_depth;
    int relative_lateral;
    int door_zone;
    int f0128_dispatch_line;
    int f0121_is_only_d2c_anchor;
    int uses_d1c_vertical_slot;
    int horizontal_uses_left_right_halves;
    const char *frame_symbol;
} DM1_V1_F0111DoorTransparencyRouteSpecPc34;

typedef struct {
    int input_state;
    DM1_V1_F0111DoorTransparencyBranchPc34 branch;
    int map_square_had_thing_list;
    int f0163_links_door_to_cell;
    int f0164_unlinks_door_from_cell;
    int door_state_from_m556;
    int door_thing_from_m557;
    int base_zone_from_p2084;
    int decremented_state;
    int first_half_zone_with_c6;
    int first_half_uses_c10;
    int second_half_shift;
    int final_zone;
    int final_uses_c10;
    int left_horizontal_bitmap;
    int right_horizontal_bitmap;
    int vertical_bitmap;
    uint8_t first_pass_pixel;
    uint8_t final_pass_pixel;
} DM1_V1_F0111DoorTransparencyTracePc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int route_count;
    int d1c_vertical_partly;
    int horizontal_partly;
    int open_rejections;
    int closed_rejections;
    int map_cell_checks;
    int d2l2_d2r2_wall_exclusions;
    int c10_transparency_checks;
} DM1_V1_F0111DoorTransparencySelfTestResultPc34;

const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *
dm1_v1_viewport_f0111_door_transparency_route_at_pc34(unsigned int index);

const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *
dm1_v1_viewport_f0111_door_transparency_route_for_square_pc34(int view_square);

int dm1_v1_viewport_f0111_door_transparency_trace_pc34(
    const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *spec,
    int door_state,
    int map_square_flags,
    int door_thing_index,
    DM1_V1_F0111DoorTransparencyTracePc34 *out);

uint8_t dm1_v1_viewport_f0111_door_transparency_blend_pc34(
    uint8_t destination,
    uint8_t source);

int dm1_v1_viewport_f0111_door_transparency_is_d2l2_d2r2_wall_pc34(
    int view_square,
    int zone);

const char *
dm1_v1_viewport_f0111_door_transparency_source_evidence_pc34(void);

int run_dm1_v1_viewport_f0111_door_transparency_self_test(void);

const DM1_V1_F0111DoorTransparencySelfTestResultPc34 *
dm1_v1_viewport_f0111_door_transparency_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
