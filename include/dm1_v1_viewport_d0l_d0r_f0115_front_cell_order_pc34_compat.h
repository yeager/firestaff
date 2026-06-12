#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L_D0R_F0115_FRONT_CELL_ORDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L_D0R_F0115_FRONT_CELL_ORDER_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 D0L/D0R F0115 front-cell-order gate.
 *
 * ReDMCSB anchors:
 *   DUNVIEW.C F0125_DUNGEONVIEW_DrawSquareD0L lines 7960-8062 and
 *   DUNVIEW.C F0126_DUNGEONVIEW_DrawSquareD0R lines 8064-8162 each
 *   dispatch F0115 with a SINGLE-nibble cell-order constant
 *   (C0x0002_CELL_ORDER_BACKRIGHT for D0L, C0x0001_CELL_ORDER_BACKLEFT
 *   for D0R). This is the unique "front cell order" aspect of the
 *   D0L/D0R thing-pass: the F0115 nibble processing stops at ordinal
 *   1, so no FRONT_LEFT (0) or FRONT_RIGHT (1) view cell is ever
 *   iterated by the thing pass on these side lanes.  ReDMCSB F0115
 *   line 5295 reinforces the same contract: a quarter-square creature
 *   is rejected for D0L unless the cell equals BACK_RIGHT and for D0R
 *   unless the cell equals BACK_LEFT.
 *
 *   DUNVIEW.C F0112_DUNGEONVIEW_DrawCeilingPit at lines 8003/8113
 *   draws the ceiling-pit bitmap BEFORE the F0115 call.  DUNVIEW.C
 *   F0113_DUNGEONVIEW_DrawField at lines 8050-8059/8150-8159 draws
 *   the teleporter field bitmap AFTER the F0115 call, gated on
 *   C05_ELEMENT_TELEPORTER + M554_PIT_OR_TELEPORTER_VISIBLE.
 *
 *   DUNVIEW.C F0128 lines 8536-8541 dispatches D0L before D0R when
 *   the relative player position is (0,-1) then (0,1).  DUNVIEW.C
 *   F0163 lines 1769-1838 and F0164 lines 1840-1905 mutate the thing
 *   list and are not called by the D0L/D0R F0115 draw path.  DUNGEON.C
 *   F0172 lines 2466-2523 supplies the per-square aspect data.
 *
 *   DEFS.H line 2088 C10_COLOR_FLESH, lines 2597-2598 M610/M611 view
 *   squares, lines 2642-2645 cell ordinals, lines 2659-2660 cell
 *   orders, lines 4056-4057 C716/C717 wall zones, lines 4139-4153
 *   cell-order zone band, lines 4217-4219 D0 ceiling zones, lines
 *   4250-4260 door-zone table has no D0 door-front zone.
 *
 * This gate is intentionally non-duplicative with the D0L2/D0R2 F0115
 * thing-pass gate: it pins the F0125/F0126 dispatch body (element
 * matrix, F0112-before-F0115, F0113-after-F0115), the SINGLE-nibble
 * cell-order contract that proves the front cells are never iterated,
 * and the F0115:5295 quarter-creature gate that rejects the front
 * cells 0/1 even if a code path attempts to use them.
 */

typedef enum {
    DM1_V1_D0L_D0R_F0115_FRONT_SIDE_D0L_PC34 = 1,
    DM1_V1_D0L_D0R_F0115_FRONT_SIDE_D0R_PC34 = 2
} DM1_V1_D0LD0RF0115FrontCellOrderSidePc34;

typedef enum {
    DM1_V1_D0L_D0R_F0115_ELEMENT_WALL_PC34 = 0,
    DM1_V1_D0L_D0R_F0115_ELEMENT_PIT_PC34 = 1,
    DM1_V1_D0L_D0R_F0115_ELEMENT_CORRIDOR_PC34 = 2,
    DM1_V1_D0L_D0R_F0115_ELEMENT_DOOR_SIDE_PC34 = 3,
    DM1_V1_D0L_D0R_F0115_ELEMENT_TELEPORTER_PC34 = 4,
    DM1_V1_D0L_D0R_F0115_ELEMENT_STAIRS_SIDE_PC34 = 5,
    DM1_V1_D0L_D0R_F0115_ELEMENT_DOOR_FRONT_PC34 = 6
} DM1_V1_D0LD0RF0115ElementPc34;

typedef struct {
    int side;
    const char *lane_name;
    int element_calls_f0115[7];
    int element_dispatches_f0112_ceiling[7];
    int element_dispatches_f0113_field[7];
    int element_returns_before_f0115[7];
    int f0112_call_line;
    int f0115_call_line;
    int f0113_call_line;
    int view_square_index;
    int view_depth;
    int view_lane;
    unsigned int f0115_cell_order;
    int f0115_processed_cells;
    int f0115_first_processed_cell;
    int f0115_front_left_ordinal_present;
    int f0115_front_right_ordinal_present;
    int f0115_back_left_ordinal_present;
    int f0115_back_right_ordinal_present;
    int creature_quarter_cell_back_right_only;
    int creature_quarter_cell_back_left_only;
    int creature_zone_rejects_front_left;
    int creature_zone_rejects_front_right;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int f0163_not_called_by_draw;
    int f0164_not_called_by_draw;
    int f0172_square_aspect_source;
    int dispatched_after_d0l_before_d0r;
    const char *redmcsb_dispatch_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *redmcsb_defs_anchor;
} DM1_V1_D0LD0RF0115FrontCellOrderPc34;

typedef struct {
    int ok;
    int f0115_calls;
    int f0112_ceiling_pit_calls;
    int f0113_field_calls;
    int f0112_f0115_order_ok;
    int f0115_f0113_order_ok;
    int rejected_front_cell_ordinal;
    int accepted_back_cell_ordinal;
    uint8_t after_f0112_ceiling_pit;
    uint8_t after_f0115_thing_pass;
    uint8_t after_f0113_field;
    int f0112_ceiling_pit_transparent;
    int f0115_thing_pass_transparent;
    int f0113_field_transparent;
} DM1_V1_D0LD0RF0115FrontCellOrderTracePc34;

void dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_init_pc34(void);

size_t dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_count_pc34(void);

size_t dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_count_pc34(void);

const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *
dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_at_pc34(size_t index);

const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *
dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_at_pc34(size_t index);

const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *
dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_for_side_pc34(int side);

const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *
dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(int side);

int dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec,
    int element);

int dm1_v1_viewport_d0l_d0r_f0115_element_returns_before_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec,
    int element);

int dm1_v1_viewport_d0l_d0r_f0115_processed_cell_count_pc34(
    unsigned int cell_order);

int dm1_v1_viewport_d0l_d0r_f0115_processed_cell_ordinal_pc34(
    unsigned int cell_order,
    int ordinal);

int dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec,
    int view_cell);

int dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec,
    int view_cell);

int dm1_v1_viewport_d0l_d0r_f0115_compose_pixel_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec,
    uint8_t base_pixel,
    uint8_t f0112_ceiling_pit_pixel,
    uint8_t f0115_thing_pass_pixel,
    uint8_t f0113_field_pixel,
    DM1_V1_D0LD0RF0115FrontCellOrderTracePc34 *out_trace);

int dm1_v1_viewport_d0l_d0r_f0115_is_draw_mutating_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec);

const char *dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0L_D0R_F0115_FRONT_CELL_ORDER_PC34_COMPAT_H */
