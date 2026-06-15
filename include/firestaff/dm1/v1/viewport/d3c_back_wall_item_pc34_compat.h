/*
 * DM1 V1 D3C F0115 back-wall item thing-pass source-lock gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0115:4547-4581: function signature, nibble processing,
 *   item/creature/projectile/explosion structure for back-wall items.
 * - DUNVIEW.C F0115:4794-4800: door-front pass nibble strip
 *   (MASK 0x0008) and the BACK/FRONT two-pass split.
 * - DUNVIEW.C F0115:4853-4860: front-cell M602..M609 visibility gate
 *   (back wall items on view cells 2 and 3 only at D3C depth 3).
 * - DUNVIEW.C F0115:4920-4923: item visibility predicate; cell > 1 at
 *   depth 3 means back cells visible, front cells clipped.
 * - DUNVIEW.C F0115:5180-5188: C10_COLOR_FLESH transparent blit.
 * - DUNVIEW.C:6723: M600_VIEW_SQUARE_D3C, C0x0218_CELL_ORDER_DOORPASS1
 *   back-left/back-right thing pass for door front.
 * - DUNVIEW.C:6816: M600_VIEW_SQUARE_D3C, L0204_i_Order = C0x3421 for
 *   corridor / pit / teleporter back-wall item pass.
 * - DUNVIEW.C F0128:8499: F0128 dispatches the D3C thing pass after
 *   D3L and D3R at relative depth 3/lateral 0.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, F0172:2466-2523:
 *   thing-list maintenance and square-aspect classification.
 * - DEFS.H:2547-2554: M550_FIRST_THING, M551_FIRST_THING_END_3, etc.
 * - DEFS.H:2607: M600_VIEW_SQUARE_D3C = 11.
 * - DEFS.H:2676: C0x3421 cell order.
 * - DEFS.H:2669: C0x0218 door-pass1 back-left/back-right cell order.
 *
 * Contract-only synthetic 320x200 framebuffer probe; no original DOS
 * pixel parity or real GRAPHICS.DAT asset comparison.
 */
#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3C_BACK_WALL_ITEM_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3C_BACK_WALL_ITEM_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D3C_BACK_WALL_ITEM_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3C_BACK_WALL_ITEM_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3C_BACK_WALL_ITEM_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_D3C_BACK_WALL_ITEM_ROUTE_WALL_PC34 = 0,
    DM1_V1_D3C_BACK_WALL_ITEM_ROUTE_DOOR_FRONT_PASS1_PC34 = 1,
    DM1_V1_D3C_BACK_WALL_ITEM_ROUTE_DOOR_FRONT_PASS2_PC34 = 2,
    DM1_V1_D3C_BACK_WALL_ITEM_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34 = 3
} DM1_V1_D3CBackWallItemRouteKindPc34;

typedef enum {
    DM1_V1_D3C_BACK_WALL_ITEM_ELEMENT_WALL_PC34 = 0,
    DM1_V1_D3C_BACK_WALL_ITEM_ELEMENT_CORRIDOR_PC34 = 1,
    DM1_V1_D3C_BACK_WALL_ITEM_ELEMENT_PIT_PC34 = 2,
    DM1_V1_D3C_BACK_WALL_ITEM_ELEMENT_TELEPORTER_PC34 = 5,
    DM1_V1_D3C_BACK_WALL_ITEM_ELEMENT_DOOR_FRONT_PC34 = 17
} DM1_V1_D3CBackWallItemElementPc34;

typedef struct {
    int route_kind;
    const char *route_name;
    int square_element;
    int back_wall_items_visible;
    int calls_f0115;
    int f0115_pass;
    unsigned int cell_order;
    int back_left_view_cell;
    int back_right_view_cell;
    int front_left_view_cell;
    int front_right_view_cell;
    int back_wall_item_zone;
    const char *redmcsb_anchor;
} DM1_V1_D3CBackWallItemRoutePc34;

typedef struct {
    int ok;
    int wall_route_skips_f0115;
    int door_pass1_back_cells_only;
    int door_pass2_front_cells_only;
    int corridor_pit_teleporter_back_then_front;
    int front_cells_clipped_at_d3c;
    int back_cells_visible_at_d3c;
    int c10_transparent_skip;
    int f0115_call_count;
    int back_wall_item_zones_seen;
    int assertions;
    int failures;
    uint32_t deterministic_hash;
} DM1_V1_D3CBackWallItemSelfTestResultPc34;

int run_dm1_v1_viewport_d3c_back_wall_item_self_test_pc34(void);

const DM1_V1_D3CBackWallItemSelfTestResultPc34 *
dm1_v1_viewport_d3c_back_wall_item_last_self_test_result_pc34(void);

const char *dm1_v1_viewport_d3c_back_wall_item_source_evidence_pc34(void);

const char *dm1_v1_viewport_d3c_back_wall_item_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D3C_BACK_WALL_ITEM_PC34_COMPAT_H */
