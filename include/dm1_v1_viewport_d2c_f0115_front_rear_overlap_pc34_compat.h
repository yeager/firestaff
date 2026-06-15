#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2C_F0115_FRONT_REAR_OVERLAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2C_F0115_FRONT_REAR_OVERLAP_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM1 V1 D2C F0115 door-front front/rear cell-order overlap contract.
 *
 * Lane: dm1_v1_refill_viewport_d2c_f0115_front_rear_overlap_gate.
 *
 * ReDMCSB DUNVIEW.C F0121_DUNGEONVIEW_DrawSquareD2C:7313-7342 issues two
 * F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF calls
 * for the door-front route: rear pass 1 with P0146_ui_OrderedViewCellOrdinals
 * = C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT (DUNVIEW.C:7315) and
 * front pass 2 with C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT
 * (DUNVIEW.C:7341-7342). Between the two calls, the door body and door
 * frame are drawn (DUNVIEW.C:7317-7339). F0115 itself (DUNVIEW.C:4547-4582,
 * 4795-4800) decodes a door-marker nibble at bit 3 of the low nibble
 * (`MASK0x0008_DOOR_FRONT`) and stores the per-call door pass in
 * `L0175_i_DoorFrontViewDrawingPass = (OrderedViewCellOrdinals & 0x0001) + 1`
 * (DUNVIEW.C:4795-4800). The high nibbles (after `>>= 4`) are the cell
 * ordinals (BACKLEFT=1, BACKRIGHT=2 for pass 1; FRONTLEFT=3, FRONTRIGHT=4
 * for pass 2 per DEFS.H:2669/2672). The "overlap" is that:
 *
 *   1. The same F0115 internal state machine handles both passes with the
 *      same first-thing pointer P0141_T_Thing; the F0115 function is
 *      called twice, once for each cell order, walking the thing list
 *      twice and dispatching each thing to the cell ordinals of that pass.
 *   2. The two cell orders share the door marker nibble at position 0
 *      (0x8 for pass 1, 0x9 for pass 2) but the remaining nibbles (cells)
 *      are disjoint (BACK cells for pass 1, FRONT cells for pass 2), so
 *      the visual overlap of the two passes is the door + door-frame
 *      pixels drawn between them plus the union of the two cell regions
 *      (BACK cells + FRONT cells = all four view cells of D2C).
 *   3. The F0115 dispatches the same creature/projectile/explosion zone
 *      tables for both passes, so a creature in the BACK cells drawn by
 *      the rear pass must not be redrawn by the front pass, and the
 *      F0115 internal `L0168_B_DrawingLastBackRowCell` and
 *      `L0167_B_TwoHalfSquareCreaturesFrontView` guards must be
 *      initialized per pass.
 *   4. The fluxcage explosion (L0192_ps_FluxcageExplosion) is recorded
 *      only on pass 1 so it is not drawn twice; the F0115 pass-1 guard
 *      must short-circuit fluxcage drawing on pass 2.
 *
 * This contract-only gate pins (a) the door marker decode, (b) the
 * per-pass cell-order cell list, (c) the F0121 dispatch between the two
 * passes, and (d) the F0115 per-pass internal-state invariants. It does
 * NOT cover the F0100/F0101 center wall, F0108 floor ornament, F0107 wall
 * ornament, F0111 door body, or F0113 field tail (those are owned by
 * the F0121 dispatch / D2C center wall composition gates).
 */

#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_C10_COLOR_FLESH 10
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MASK0x0008_DOOR_FRONT 0x0008U
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MASK0x0001_DOOR_PASS 0x0001U
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MASK0x000F_CELL_NIBBLE 0x000FU
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_NIBBLE_SHIFT 4
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MAX_CELLS_PER_PASS 4

#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_TERMINATOR 0U
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_BACKLEFT 1U
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_BACKRIGHT 2U
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_FRONTLEFT 3U
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_FRONTRIGHT 4U

#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_REAR_ORDER 0x0218U
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_FRONT_ORDER 0x0349U
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_REAR_MARKER 0x0008U
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_FRONT_MARKER 0x0009U
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_REAR_PASS_VALUE 1
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_FRONT_PASS_VALUE 2
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_NON_DOOR_PASS_VALUE 0

#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_VIEW_SQUARE_D2C 6
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_VIEW_DEPTH 2
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_VIEW_LANE 0
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_WALL_ZONE 709
#define DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_DOOR_ZONE 3760

/* Source-locked route spec for one of the two F0115 door-front passes. */
typedef struct {
    int pass;                              /* 1=rear, 2=front, 0=non-door. */
    const char *lane_name;                 /* Human-readable lane label. */
    const char *pass_role;                 /* "rear_pass1" or "front_pass2". */
    unsigned int cell_order;               /* Full P0146_ui_OrderedViewCellOrdinals. */
    unsigned int door_marker_nibble;       /* Low 4 bits of cell_order. */
    unsigned int cells_nibble;             /* cell_order with the door nibble stripped. */
    int door_marker_set;                   /* bit 3 of door marker nibble. */
    int door_pass_bit;                     /* bit 0 of door marker nibble. */
    int cell_count;                        /* Number of cell ordinals (1..2). */
    int cell_ordinals[DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MAX_CELLS_PER_PASS];
    int view_square_index;
    int view_depth;
    int view_lane;
    int wall_zone;
    int door_zone;
    int transparent_color;
    /* F0115 internal state machine invariants for this pass. */
    int fluxcage_recorded_this_pass;       /* Pass 1 records; pass 2 must not redraw. */
    int creature_zone_table_reused;        /* Same C3200 + row*5 + cell table. */
    int projectile_zone_table_reused;      /* Same C2900 + row*4 + cell table. */
    int item_zone_table_reused;            /* Same C2500 + row*4 + cell table. */
    int explosion_zone_table_reused;       /* Same C3014/C3031 + row table. */
    int field_aspect_lookup_reused;        /* Same G2035[viewSquare]. */
    int first_thing_pointer_passed_through;/* P0141_T_Thing passed unchanged. */
    int same_thing_list_walked_twice;      /* F0121 issues two F0115 calls. */
    int contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int f0115_internal_dispatch_anchor_set;
    const char *redmcsb_f0121_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_defs_anchor;
    const char *source_lines;
} DM1_V1_D2CF0115FrontRearOverlapPc34;

/* Per-pass pixel overlap trace: shows the door marker strip + cell
 * ordinal decode + the resulting cell list. */
typedef struct {
    int ok;
    int decoded_pass;
    unsigned int remaining_ordinals;       /* cell_order with door nibble stripped. */
    int cell_count;
    int cell_ordinals[DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MAX_CELLS_PER_PASS];
    int door_marker_set;
    int door_pass_bit;
    int fluxcage_recorded_this_pass;
} DM1_V1_D2CF0115FrontRearOverlapTracePc34;

void dm1_v1_viewport_d2c_f0115_front_rear_overlap_init_pc34(void);

size_t dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_count_pc34(void);

const DM1_V1_D2CF0115FrontRearOverlapPc34 *
dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_at_pc34(size_t index);

const DM1_V1_D2CF0115FrontRearOverlapPc34 *
dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(int pass);

/* Decode a single F0115 P0146_ui_OrderedViewCellOrdinals value into a
 * (door_marker_set, door_pass_bit, remaining_ordinals) tuple. The
 * remaining ordinals are the high nibbles that drive the F0115 cell loop.
 * Returns 0 on success, -1 on bad input. The output fields are always
 * initialized. */
int dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_marker_pc34(
    unsigned int cell_order,
    int *out_door_marker_set,
    int *out_door_pass_bit,
    unsigned int *out_remaining_ordinals);

/* Decode the per-pass cell ordinal list after the door marker has been
 * stripped. Cells are returned in the same order F0115 will process
 * them. Returns the number of cells decoded (1..2) or -1 on bad input. */
int dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_cells_pc34(
    unsigned int remaining_ordinals,
    int *out_cells,
    size_t out_cells_len);

/* Run the full F0115 door-pass trace for a single cell_order: decode
 * the door marker, strip it, decode the cell list, and copy the
 * fluxcage-record-this-pass invariant. */
int dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
    unsigned int cell_order,
    DM1_V1_D2CF0115FrontRearOverlapTracePc34 *out_trace);

/* Compose a single D2C F0115 door-front pixel across the two F0115
 * passes (rear then front) and the door body drawn between them. The
 * rear and front F0115 cells are disjoint (BACK vs FRONT), and the
 * F0115 internal state (creature, projectile, item, explosion tables,
 * field aspect) is read fresh per pass, so the C10 transparent blit
 * must not carry over C10 pixels between passes. The function returns
 * 0 on success and -1 on bad input. */
int dm1_v1_viewport_d2c_f0115_front_rear_overlap_compose_pixel_pc34(
    uint8_t initial_pixel,
    uint8_t rear_f0115_pixel,
    uint8_t door_body_pixel,
    uint8_t front_f0115_pixel,
    uint8_t transparent_color,
    DM1_V1_D2CF0115FrontRearOverlapTracePc34 *out_rear_trace,
    DM1_V1_D2CF0115FrontRearOverlapTracePc34 *out_front_trace,
    uint8_t *out_after_rear,
    uint8_t *out_after_door,
    uint8_t *out_after_front);

/* True iff the two F0115 door-front cell orders are disjoint (BACK
 * cells vs FRONT cells); they never share a cell ordinal. The overlap
 * is only the door body/frame pixels drawn between the two F0115 calls
 * plus the union of the cell regions. */
int dm1_v1_viewport_d2c_f0115_front_rear_overlap_cells_disjoint_pc34(
    unsigned int rear_order,
    unsigned int front_order);

const char *
dm1_v1_viewport_d2c_f0115_front_rear_overlap_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2C_F0115_FRONT_REAR_OVERLAP_PC34_COMPAT_H */
