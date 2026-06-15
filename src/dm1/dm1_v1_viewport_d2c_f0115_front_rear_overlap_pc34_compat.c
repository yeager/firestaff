#include "dm1_v1_viewport_d2c_f0115_front_rear_overlap_pc34_compat.h"

#include <string.h>

enum {
    DM1_D2C_FRONT_REAR_OVERLAP_PRESENT = 1,
    DM1_D2C_FRONT_REAR_OVERLAP_ABSENT = 0
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: contract_only=1, "
    "no_real_asset_bitmap_parity=1, no_game_data_load=1. ReDMCSB "
    "DUNVIEW.C:4547-4582 F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectiles"
    "Explosions_CPSEF documents the door-marker low-nibble semantics: "
    "bit 3 (MASK0x0008_DOOR_FRONT) is the door-front marker and bits 2-0 "
    "encode the door drawing pass (1 or 2). DUNVIEW.C:4795-4800 stores the "
    "per-call door pass in L0175_i_DoorFrontViewDrawingPass = "
    "(OrderedViewCellOrdinals & 0x0001) + 1, and shifts the cell-order "
    "value right by 4 to expose the cell nibbles. DUNVIEW.C:4561-4564 then "
    "walks the cell nibbles until a terminator (0) is reached. DEFS.H:2669 "
    "locks C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT (door marker 8, "
    "BACKRIGHT cell 2, BACKLEFT cell 1, terminator 0). DEFS.H:2672 locks "
    "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT (door marker 9, "
    "FRONTLEFT cell 3, FRONTRIGHT cell 4, terminator 0). DEFS.H:2088 "
    "locks C10_COLOR_FLESH. DEFS.H:2602 locks M603_VIEW_SQUARE_D2C=6 and "
    "DUNVIEW.C:370-377 maps M603 index 6 to lane 0, depth 2, and field "
    "aspect 7. DEFS.H:4049 locks C709_ZONE_WALL_D2C; DEFS.H:4256 locks "
    "M628_ZONE_DOOR_D2C=3760 for the door blit zone. DUNVIEW.C:7313-7342 "
    "F0121_DUNGEONVIEW_DrawSquareD2C door-front route issues the rear "
    "F0115 with C0x0218 at 7315, then draws the door body/frame at "
    "7317-7339, then the front F0115 with C0x0349 at 7341-7342; both "
    "F0115 calls receive the same P0141_T_Thing = L0212_ai_SquareAspect"
    "[M550_FIRST_THING]. F0115 walks the thing list once per call, "
    "dispatching each thing to the cell ordinals of that pass (BACK for "
    "pass 1, FRONT for pass 2). The fluxcage explosion is recorded only "
    "during pass 1 (L0192_ps_FluxcageExplosion) so it is not drawn twice; "
    "DUNVIEW.C:6199-6219 draws the fluxcage field after the explosion "
    "loop, but only when L0175_i_DoorFrontViewDrawingPass != 1 (the pass-1 "
    "guard). DUNVIEW.C:5295-5297 and 5615-5617 binds the C3200+row*5+cell "
    "creature zones; DUNVIEW.C:4923 and 5668-5671 read G2028[viewSquare] "
    "for the per-call item/projectile visibility; DUNVIEW.C:6107 and "
    "6110-6122 read G2034[viewSquare] for the per-call explosion zone; "
    "DUNVIEW.C:6219 reads G2035[viewSquare] for the per-call field aspect. "
    "DUNVIEW.C:8520-8521 F0128 dispatches D2C after D2L and D2R; the two "
    "F0115 calls for the D2C door front case consume the same dungeon "
    "state, so the F0115 internal-state machine must reset "
    "L0168_B_DrawingLastBackRowCell and L0167_B_TwoHalfSquareCreaturesFrontView"
    " per pass. The overlap is therefore: (a) door marker nibble at low "
    "position (0x8 for pass 1, 0x9 for pass 2), (b) disjoint cell sets "
    "(BACK for pass 1, FRONT for pass 2), (c) the same P0141_T_Thing "
    "thing-list pointer passed unchanged, and (d) the door body/frame "
    "pixels drawn between the two passes. This contract gate pins the "
    "door-marker decode, the per-pass cell-order list, the two-pass "
    "F0121 dispatch, and the F0115 internal-state invariants.";

static const DM1_V1_D2CF0115FrontRearOverlapPc34 s_fixtures[] = {
    {
        /* pass 1 (rear): DUNVIEW.C:7315; DEFS.H:2669 */
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_REAR_PASS_VALUE,
        "D2C door-front rear F0115 pass 1 (BACK cells)",
        "rear_pass1",
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_REAR_ORDER,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_REAR_MARKER,
        0x0218U >> DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_NIBBLE_SHIFT,
        1, /* door marker bit 3 set */
        0, /* low bit of 0x8 */
        2, /* two cells: BACKLEFT=1, BACKRIGHT=2 (F0115 nibble order) */
        {
            DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_BACKLEFT,
            DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_BACKRIGHT
        },
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_VIEW_SQUARE_D2C,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_VIEW_DEPTH,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_VIEW_LANE,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_WALL_ZONE,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_DOOR_ZONE,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_C10_COLOR_FLESH,
        1, /* fluxcage recorded on pass 1 */
        1, /* creature zone table reused (C3200 + row*5 + cell) */
        1, /* projectile zone table reused (C2900 + row*4 + cell) */
        1, /* item zone table reused (C2500 + row*4 + cell) */
        1, /* explosion zone table reused (C3014/C3031 + row) */
        1, /* field aspect lookup reused (G2035[viewSquare]) */
        1, /* P0141_T_Thing passed through unchanged */
        1, /* same thing list walked twice (F0121 issues two F0115 calls) */
        1, /* contract_only */
        1, /* no_real_asset_bitmap_parity */
        1, /* no_game_data_load */
        1, /* f0115_internal_dispatch_anchor_set */
        "ReDMCSB DUNVIEW.C:7313-7315 F0121_DUNGEONVIEW_DrawSquareD2C door rear",
        "ReDMCSB DUNVIEW.C:4547-4582/4795-4800 F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "ReDMCSB DEFS.H:2669 C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT; 2088 C10_COLOR_FLESH; 2602 M603_VIEW_SQUARE_D2C=6; 4049 C709_ZONE_WALL_D2C; 4256 M628_ZONE_DOOR_D2C=3760",
        s_source_evidence
    },
    {
        /* pass 2 (front): DUNVIEW.C:7341-7342; DEFS.H:2672 */
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_FRONT_PASS_VALUE,
        "D2C door-front front F0115 pass 2 (FRONT cells)",
        "front_pass2",
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_FRONT_ORDER,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_FRONT_MARKER,
        0x0349U >> DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_NIBBLE_SHIFT,
        1, /* door marker bit 3 set */
        1, /* low bit of 0x9 */
        2, /* two cells: FRONTRIGHT=4, FRONTLEFT=3 (F0115 nibble order) */
        {
            DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_FRONTRIGHT,
            DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_FRONTLEFT
        },
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_VIEW_SQUARE_D2C,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_VIEW_DEPTH,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_VIEW_LANE,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_WALL_ZONE,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_DOOR_ZONE,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_C10_COLOR_FLESH,
        0, /* fluxcage NOT recorded on pass 2 (pass 1 already recorded) */
        1, /* creature zone table reused */
        1, /* projectile zone table reused */
        1, /* item zone table reused */
        1, /* explosion zone table reused */
        1, /* field aspect lookup reused */
        1, /* P0141_T_Thing passed through unchanged */
        1, /* same thing list walked twice */
        1, /* contract_only */
        1, /* no_real_asset_bitmap_parity */
        1, /* no_game_data_load */
        1, /* f0115_internal_dispatch_anchor_set */
        "ReDMCSB DUNVIEW.C:7341-7342 F0121_DUNGEONVIEW_DrawSquareD2C door front",
        "ReDMCSB DUNVIEW.C:4547-4582/4795-4800 F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "ReDMCSB DEFS.H:2672 C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT; 2088 C10_COLOR_FLESH; 2602 M603_VIEW_SQUARE_D2C=6; 4049 C709_ZONE_WALL_D2C; 4256 M628_ZONE_DOOR_D2C=3760",
        s_source_evidence
    }
};

static int s_initialized;

void dm1_v1_viewport_d2c_f0115_front_rear_overlap_init_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0121 lines 7313-7342 and F0115 lines 4547-4582
     * seed the two-pass dispatch contract; no DUNGEON.DAT/GRAPHICS.DAT
     * is read. */
    s_initialized = DM1_D2C_FRONT_REAR_OVERLAP_PRESENT;
}

size_t dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_count_pc34(void)
{
    if (!s_initialized) {
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_init_pc34();
    }
    return sizeof(s_fixtures) / sizeof(s_fixtures[0]);
}

const DM1_V1_D2CF0115FrontRearOverlapPc34 *
dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_at_pc34(size_t index)
{
    if (!s_initialized) {
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_init_pc34();
    }
    if (index >= dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_count_pc34()) {
        return 0;
    }
    return &s_fixtures[index];
}

const DM1_V1_D2CF0115FrontRearOverlapPc34 *
dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_for_pass_pc34(int pass)
{
    size_t i;
    if (!s_initialized) {
        dm1_v1_viewport_d2c_f0115_front_rear_overlap_init_pc34();
    }
    for (i = 0;
         i < dm1_v1_viewport_d2c_f0115_front_rear_overlap_spec_count_pc34();
         ++i) {
        if (s_fixtures[i].pass == pass) return &s_fixtures[i];
    }
    return 0;
}

int dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_marker_pc34(
    unsigned int cell_order,
    int *out_door_marker_set,
    int *out_door_pass_bit,
    unsigned int *out_remaining_ordinals)
{
    if (!out_door_marker_set || !out_door_pass_bit || !out_remaining_ordinals) {
        return -1;
    }
    /* ReDMCSB: DUNVIEW.C F0115 lines 4795-4800 inspect bit 3 of the low
     * nibble (MASK0x0008_DOOR_FRONT) and then extract bit 0 of the low
     * nibble for the per-call door pass. */
    *out_door_marker_set =
        (cell_order & DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MASK0x0008_DOOR_FRONT) ? 1 : 0;
    *out_door_pass_bit =
        (cell_order & DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MASK0x0001_DOOR_PASS) ? 1 : 0;
    *out_remaining_ordinals =
        cell_order >> DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_NIBBLE_SHIFT;
    return 0;
}

int dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_cells_pc34(
    unsigned int remaining_ordinals,
    int *out_cells,
    size_t out_cells_len)
{
    int count = 0;
    unsigned int shift;

    if (!out_cells ||
        out_cells_len < DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MAX_CELLS_PER_PASS) {
        return -1;
    }
    /* ReDMCSB: DUNVIEW.C F0115 lines 4561-4564 walk the cell nibbles
     * least-significant-nibble first and stop at the first terminator
     * (0). DEFS.H:2656-2660/2662-2667/2669/2672/2676 cell ordinals. */
    for (shift = 0; shift < 16; shift += 4) {
        unsigned int cell =
            (remaining_ordinals >> shift) &
            DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MASK0x000F_CELL_NIBBLE;
        if (cell == DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_TERMINATOR) break;
        if (count >= DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MAX_CELLS_PER_PASS) {
            return -1;
        }
        if (cell != DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_BACKLEFT &&
            cell != DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_BACKRIGHT &&
            cell != DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_FRONTLEFT &&
            cell != DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_CELL_FRONTRIGHT) {
            return -1;
        }
        out_cells[count++] = (int)cell;
    }
    if (count == 0) return -1;
    return count;
}

int dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
    unsigned int cell_order,
    DM1_V1_D2CF0115FrontRearOverlapTracePc34 *out_trace)
{
    int door_marker_set = 0;
    int door_pass_bit = 0;
    unsigned int remaining = 0;
    int cell_count = 0;
    int cells[DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MAX_CELLS_PER_PASS] = {0, 0};
    int decoded_pass = DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_NON_DOOR_PASS_VALUE;

    if (!out_trace) return -1;
    memset(out_trace, 0, sizeof(*out_trace));

    if (dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_marker_pc34(
            cell_order, &door_marker_set, &door_pass_bit, &remaining) != 0) {
        return -1;
    }
    if (door_marker_set) {
        /* ReDMCSB: DUNVIEW.C F0115 line 4799 stores
         * L0175_i_DoorFrontViewDrawingPass = (cell_order & 0x0001) + 1. */
        decoded_pass = door_pass_bit + 1;
    }
    cell_count = dm1_v1_viewport_d2c_f0115_front_rear_overlap_decode_cells_pc34(
        remaining, cells,
        DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_MAX_CELLS_PER_PASS);
    if (cell_count < 0) {
        return -1;
    }

    out_trace->ok = 1;
    out_trace->decoded_pass = decoded_pass;
    out_trace->remaining_ordinals = remaining;
    out_trace->cell_count = cell_count;
    for (int i = 0; i < cell_count; ++i) {
        out_trace->cell_ordinals[i] = cells[i];
    }
    out_trace->door_marker_set = door_marker_set;
    out_trace->door_pass_bit = door_pass_bit;
    /* ReDMCSB: DUNVIEW.C F0115 line 6006-6015 records the fluxcage
     * explosion only on the rear pass; the F0115 fluxcage-field branch
     * at 6199-6219 is gated by L0175_i_DoorFrontViewDrawingPass != 1. */
    out_trace->fluxcage_recorded_this_pass =
        (decoded_pass == DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_REAR_PASS_VALUE) ? 1 : 0;
    return 0;
}

static uint8_t apply_transparent_pixel(uint8_t destination,
                                       uint8_t source,
                                       uint8_t transparent_color)
{
    /* ReDMCSB: DUNVIEW.C F0115 line 4605+ object/creature/projectile
     * blits, F0100 lines 3048-3058 wall blits, and F0111 lines 4218-4337
     * door blits all preserve C10 (DEFS.H:2088 C10_COLOR_FLESH) as the
     * transparent pixel. */
    return source == transparent_color ? destination : source;
}

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
    uint8_t *out_after_front)
{
    uint8_t pixel;

    if (!out_rear_trace || !out_front_trace ||
        !out_after_rear || !out_after_door || !out_after_front) {
        return -1;
    }
    if (dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
            DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_REAR_ORDER,
            out_rear_trace) != 0) {
        return -1;
    }
    if (dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
            DM1_V1_D2C_F0115_FRONT_REAR_OVERLAP_PC34_FRONT_ORDER,
            out_front_trace) != 0) {
        return -1;
    }

    pixel = apply_transparent_pixel(initial_pixel, rear_f0115_pixel,
                                    transparent_color);
    *out_after_rear = pixel;

    pixel = apply_transparent_pixel(pixel, door_body_pixel, transparent_color);
    *out_after_door = pixel;

    pixel = apply_transparent_pixel(pixel, front_f0115_pixel, transparent_color);
    *out_after_front = pixel;
    return 0;
}

int dm1_v1_viewport_d2c_f0115_front_rear_overlap_cells_disjoint_pc34(
    unsigned int rear_order,
    unsigned int front_order)
{
    DM1_V1_D2CF0115FrontRearOverlapTracePc34 rear_trace = {0};
    DM1_V1_D2CF0115FrontRearOverlapTracePc34 front_trace = {0};
    int i;
    int j;

    if (dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
            rear_order, &rear_trace) != 0) {
        return -1;
    }
    if (dm1_v1_viewport_d2c_f0115_front_rear_overlap_trace_pc34(
            front_order, &front_trace) != 0) {
        return -1;
    }
    for (i = 0; i < rear_trace.cell_count; ++i) {
        for (j = 0; j < front_trace.cell_count; ++j) {
            if (rear_trace.cell_ordinals[i] == front_trace.cell_ordinals[j]) {
                return 0;
            }
        }
    }
    return 1;
}

const char *
dm1_v1_viewport_d2c_f0115_front_rear_overlap_source_evidence_pc34(void)
{
    return s_source_evidence;
}
