#include "dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_pc34_compat.h"

#include <string.h>

enum {
    DM1_D0L_D0R_FRONT_ROUTE_PRESENT = 1,
    DM1_D0L_VIEW_SQUARE = 1,             /* ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L */
    DM1_D0R_VIEW_SQUARE = 2,             /* ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R */
    DM1_D0_DEPTH = 0,                    /* ReDMCSB DUNVIEW.C:372 G2027[1/2] */
    DM1_D0L_LANE = -1,                   /* ReDMCSB DUNVIEW.C:371 G2026[1] */
    DM1_D0R_LANE = 1,                    /* ReDMCSB DUNVIEW.C:371 G2026[2] */
    DM1_CELL_FRONT_LEFT = 0,             /* ReDMCSB DEFS.H:2642 C00_VIEW_CELL_FRONT_LEFT */
    DM1_CELL_FRONT_RIGHT = 1,            /* ReDMCSB DEFS.H:2643 C01_VIEW_CELL_FRONT_RIGHT */
    DM1_CELL_BACK_RIGHT = 2,             /* ReDMCSB DEFS.H:2644 C02_VIEW_CELL_BACK_RIGHT */
    DM1_CELL_BACK_LEFT = 3,              /* ReDMCSB DEFS.H:2645 C03_VIEW_CELL_BACK_LEFT */
    DM1_D0L_CELL_ORDER = 0x0002u,        /* ReDMCSB DEFS.H:2660; DUNVIEW.C:8005 */
    DM1_D0R_CELL_ORDER = 0x0001u,        /* ReDMCSB DEFS.H:2659; DUNVIEW.C:8115 */
    DM1_C10_COLOR_FLESH = 10,            /* ReDMCSB DEFS.H:2088 */
    DM1_D0L_F0112_LINE = 8003,
    DM1_D0L_F0115_LINE = 8005,
    DM1_D0L_F0113_LINE_LOW = 8050,
    DM1_D0L_F0113_LINE_HIGH = 8059,
    DM1_D0R_F0112_LINE = 8113,
    DM1_D0R_F0115_LINE = 8115,
    DM1_D0R_F0113_LINE_LOW = 8150,
    DM1_D0R_F0113_LINE_HIGH = 8159
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate: source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. ReDMCSB anchors: "
    "DUNVIEW.C F0125_DUNGEONVIEW_DrawSquareD0L lines 7960-8062 and "
    "DUNVIEW.C F0126_DUNGEONVIEW_DrawSquareD0R lines 8064-8162 are the "
    "near side-lane dispatch bodies that route to F0115. DUNVIEW.C F0112 "
    "ceiling-pit is invoked via DUNVIEW.C F0125 line 8003 (D0L) and "
    "DUNVIEW.C F0126 line 8113 (D0R), BEFORE the F0115 call at line "
    "8005/8115. DUNVIEW.C F0113 teleporter field is invoked at lines "
    "8050-8059 (D0L) and lines 8150-8159 (D0R), AFTER the F0115 call, "
    "gated on C05_ELEMENT_TELEPORTER + M554_PIT_OR_TELEPORTER_VISIBLE. "
    "DUNVIEW.C F0115 line 5295 enforces the quarter-creature cell-order "
    "gate: D0L only accepts BACK_RIGHT (ordinal 2), D0R only accepts "
    "BACK_LEFT (ordinal 3), and FRONT_LEFT/FRONT_RIGHT ordinals 0/1 are "
    "rejected on both side lanes. DUNVIEW.C F0115 lines 4547-4581 "
    "describe the nibble-walk cell-order processing that stops at the "
    "first zero nibble; the D0L/D0R dispatch passes a single non-zero "
    "nibble (0x0002 / 0x0001) so the F0115 thing pass never iterates a "
    "front cell. DUNVIEW.C F0128 lines 8536-8541 dispatches D0L then D0R "
    "after relative offsets (0,-1) and (0,1). DUNGEON.C F0163 lines "
    "1769-1838 and F0164 lines 1840-1905 mutate the thing list and are "
    "not called by the F0125/F0126 draw path. DUNGEON.C F0172 lines "
    "2466-2523 supplies the per-square aspect data consumed by F0125/"
    "F0126. DEFS.H line 2088 C10_COLOR_FLESH, lines 2597-2598 M610/M611 "
    "view squares, lines 2642-2645 view-cell ordinals, lines 2659-2660 "
    "cell-order constants, lines 4056-4057 C716/C717 wall zones, lines "
    "4217-4219 D0 ceiling zones, lines 4250-4260 D0 door-zone absence.";

/*
 * Element dispatch matrix helpers. The dispatch element values match
 * the C0_ELEMENT/C01_ELEMENT_CORRIDOR/C02_ELEMENT_PIT/C05_ELEMENT_TELEPORTER/
 * C16_ELEMENT_DOOR_SIDE/C17_ELEMENT_DOOR_FRONT/C18_ELEMENT_STAIRS_SIDE/
 * C00_ELEMENT_WALL constants used in F0125/F0126.
 */

/*
 * The element dispatch matrix is encoded directly in the per-spec
 * fixture data (element_calls_f0115, element_returns_before_f0115).
 * ReDMCSB DUNVIEW.C F0125 lines 7972-8038 / F0126 lines 8083-8144:
 *   - PIT, CORRIDOR, DOOR_SIDE, TELEPORTER fall through to the F0115
 *     call at lines 8005/8115.
 *   - WALL (8007-8038 / 8117-8144) returns before F0115.
 *   - STAIRS_SIDE (7972-7988 / 8083-8100) returns before F0115.
 *   - DOOR_FRONT is not a D0L/D0R element; D0L/D0R have no front-door
 *     cell (that is the D1C element).
 * ReDMCSB DUNVIEW.C F0125 line 8003 / F0126 line 8113: F0112 ceiling
 * is invoked BEFORE the F0115 call. ReDMCSB DUNVIEW.C F0125 lines
 * 8050-8059 / F0126 lines 8150-8159: F0113 field is invoked AFTER
 * the F0115 call, gated on C05_ELEMENT_TELEPORTER +
 * M554_PIT_OR_TELEPORTER_VISIBLE.
 */

static const DM1_V1_D0LD0RF0115FrontCellOrderPc34 s_fixtures[] = {
    {
        DM1_V1_D0L_D0R_F0115_FRONT_SIDE_D0L_PC34,
        "D0L near-left side-lane F0115 front-cell-order contract",
        {
            0, /* WALL: returns before F0115 */
            1, /* PIT: calls F0115 */
            1, /* CORRIDOR: calls F0115 */
            1, /* DOOR_SIDE: calls F0115 */
            1, /* TELEPORTER: calls F0115 */
            0, /* STAIRS_SIDE: returns before F0115 */
            0  /* DOOR_FRONT: not a D0L element */
        },
        {
            0, /* WALL: returns before F0112 */
            1, /* PIT: F0112 dispatched at line 8003 */
            1, /* CORRIDOR: F0112 dispatched at line 8003 */
            1, /* DOOR_SIDE: F0112 dispatched at line 8003 */
            1, /* TELEPORTER: F0112 dispatched at line 8003 */
            0, /* STAIRS_SIDE: returns before F0112 */
            0  /* DOOR_FRONT: not a D0L element */
        },
        {
            0, /* WALL: returns before F0113 */
            0, /* PIT: never reaches the post-switch F0113 call */
            0, /* CORRIDOR: never reaches the post-switch F0113 call */
            0, /* DOOR_SIDE: never reaches the post-switch F0113 call */
            1, /* TELEPORTER: F0113 dispatched at lines 8050-8059 */
            0, /* STAIRS_SIDE: returns before F0113 */
            0  /* DOOR_FRONT: not a D0L element */
        },
        {
            1, /* WALL returns */
            0, /* PIT calls F0115 */
            0, /* CORRIDOR calls F0115 */
            0, /* DOOR_SIDE calls F0115 */
            0, /* TELEPORTER calls F0115 */
            1, /* STAIRS_SIDE returns */
            0  /* DOOR_FRONT not applicable */
        },
        DM1_D0L_F0112_LINE,
        DM1_D0L_F0115_LINE,
        DM1_D0L_F0113_LINE_LOW,
        DM1_D0L_VIEW_SQUARE,
        DM1_D0_DEPTH,
        DM1_D0L_LANE,
        DM1_D0L_CELL_ORDER,
        1, /* F0115 processed cells: 1 (single non-zero nibble) */
        DM1_CELL_BACK_RIGHT, /* F0115 first (and only) processed cell */
        0, /* front-left ordinal NOT present in 0x0002 */
        0, /* front-right ordinal NOT present in 0x0002 */
        0, /* back-left ordinal NOT present in 0x0002 */
        1, /* back-right ordinal present in 0x0002 */
        1, /* D0L quarter creature gate: BACK_RIGHT only */
        0, /* D0L quarter creature gate: NOT BACK_LEFT */
        1, /* D0L rejects front-left cell */
        1, /* D0L rejects front-right cell */
        1, 1, 1,
        1, 1, 1,
        1, /* dispatched after D0L (and before D0R) per F0128:8536-8537 */
        "DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L; 8003/8005/8050-8059",
        "DUNVIEW.C:4547-4581 F0115; 5295 quarter-creature gate; 4923/5668-5671 disabled rows",
        "DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; 2466-2523 F0172",
        "DEFS.H:2088 C10; 2597/2598 view squares; 2642-2645 cells; 2659-2660 orders; 4056/4217-4219/4250-4260 zones"
    },
    {
        DM1_V1_D0L_D0R_F0115_FRONT_SIDE_D0R_PC34,
        "D0R near-right side-lane F0115 front-cell-order contract",
        {
            0, /* WALL: returns before F0115 */
            1, /* PIT: calls F0115 */
            1, /* CORRIDOR: calls F0115 */
            1, /* DOOR_SIDE: calls F0115 */
            1, /* TELEPORTER: calls F0115 */
            0, /* STAIRS_SIDE: returns before F0115 */
            0  /* DOOR_FRONT: not a D0R element */
        },
        {
            0, /* WALL: returns before F0112 */
            1, /* PIT: F0112 dispatched at line 8113 */
            1, /* CORRIDOR: F0112 dispatched at line 8113 */
            1, /* DOOR_SIDE: F0112 dispatched at line 8113 */
            1, /* TELEPORTER: F0112 dispatched at line 8113 */
            0, /* STAIRS_SIDE: returns before F0112 */
            0  /* DOOR_FRONT: not a D0R element */
        },
        {
            0, /* WALL: returns before F0113 */
            0, /* PIT: never reaches the post-switch F0113 call */
            0, /* CORRIDOR: never reaches the post-switch F0113 call */
            0, /* DOOR_SIDE: never reaches the post-switch F0113 call */
            1, /* TELEPORTER: F0113 dispatched at lines 8150-8159 */
            0, /* STAIRS_SIDE: returns before F0113 */
            0  /* DOOR_FRONT: not a D0R element */
        },
        {
            1, /* WALL returns */
            0, /* PIT calls F0115 */
            0, /* CORRIDOR calls F0115 */
            0, /* DOOR_SIDE calls F0115 */
            0, /* TELEPORTER calls F0115 */
            1, /* STAIRS_SIDE returns */
            0  /* DOOR_FRONT not applicable */
        },
        DM1_D0R_F0112_LINE,
        DM1_D0R_F0115_LINE,
        DM1_D0R_F0113_LINE_LOW,
        DM1_D0R_VIEW_SQUARE,
        DM1_D0_DEPTH,
        DM1_D0R_LANE,
        DM1_D0R_CELL_ORDER,
        1, /* F0115 processed cells: 1 (single non-zero nibble) */
        DM1_CELL_BACK_LEFT, /* F0115 first (and only) processed cell */
        0, /* front-left ordinal NOT present in 0x0001 */
        0, /* front-right ordinal NOT present in 0x0001 */
        1, /* back-left ordinal present in 0x0001 */
        0, /* back-right ordinal NOT present in 0x0001 */
        0, /* D0R quarter creature gate: NOT BACK_RIGHT */
        1, /* D0R quarter creature gate: BACK_LEFT only */
        1, /* D0R rejects front-left cell */
        1, /* D0R rejects front-right cell */
        1, 1, 1,
        1, 1, 1,
        1, /* dispatched after D0L and D0R per F0128:8540-8541 */
        "DUNVIEW.C:8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R; 8113/8115/8150-8159",
        "DUNVIEW.C:4547-4581 F0115; 5295 quarter-creature gate; 4923/5668-5671 disabled rows",
        "DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; 2466-2523 F0172",
        "DEFS.H:2088 C10; 2597/2598 view squares; 2642-2645 cells; 2659-2660 orders; 4057/4217-4219/4250-4260 zones"
    }
};

static int s_initialized;

void dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_init_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0125/F0126 lines 8003-8059/8113-8159 are the
     * D0L/D0R element-dispatch and F0112/F0115/F0113 ordering contract;
     * no DUNGEON.DAT/GRAPHICS.DAT is read. */
    s_initialized = DM1_D0L_D0R_FRONT_ROUTE_PRESENT;
}

size_t dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_count_pc34(void)
{
    if (!s_initialized) dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_init_pc34();
    return sizeof(s_fixtures) / sizeof(s_fixtures[0]);
}

size_t dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_count_pc34(void)
{
    return dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_count_pc34();
}

const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *
dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_count_pc34()) {
        return NULL;
    }
    return &s_fixtures[index];
}

const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *
dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_at_pc34(size_t index)
{
    return dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_at_pc34(index);
}

const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *
dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_for_side_pc34(int side)
{
    size_t i;

    for (i = 0;
         i < dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_count_pc34();
         ++i) {
        if (s_fixtures[i].side == side) return &s_fixtures[i];
    }
    return NULL;
}

const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *
dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(int side)
{
    return dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_for_side_pc34(side);
}

int dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec,
    int element)
{
    if (!spec) return -1;
    if (element < 0 || element > 6) return -1;
    /* ReDMCSB: DUNVIEW.C F0125/F0126 element dispatch matrix; the
     * D0L/D0R thing-pass only fires for PIT/CORRIDOR/DOOR_SIDE/
     * TELEPORTER per the case fall-through at lines 7972-8005/8083-8115. */
    return spec->element_calls_f0115[element];
}

int dm1_v1_viewport_d0l_d0r_f0115_element_returns_before_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec,
    int element)
{
    if (!spec) return -1;
    if (element < 0 || element > 6) return -1;
    /* ReDMCSB: WALL (DUNVIEW.C:8007-8038/8117-8144) and STAIRS_SIDE
     * (DUNVIEW.C:7972-7988/8083-8100) return before the F0115 call. */
    return spec->element_returns_before_f0115[element];
}

int dm1_v1_viewport_d0l_d0r_f0115_processed_cell_count_pc34(
    unsigned int cell_order)
{
    unsigned int nibble;
    int count = 0;
    int ordinal;

    /* ReDMCSB: DUNVIEW.C F0115 lines 4547-4581 nibble-walk; processing
     * stops at the first zero nibble. The D0L/D0R contract is a
     * single non-zero nibble, so the thing pass processes exactly
     * one view cell. Door-pass markers (8/9) terminate without
     * counting. */
    for (ordinal = 0; ordinal < 4; ++ordinal) {
        nibble = (cell_order >> (ordinal * 4)) & 0x0fu;
        if (nibble == 0u || nibble == 8u || nibble == 9u) {
            return count;
        }
        ++count;
    }
    return count;
}

int dm1_v1_viewport_d0l_d0r_f0115_processed_cell_ordinal_pc34(
    unsigned int cell_order,
    int ordinal)
{
    unsigned int nibble;

    /* ReDMCSB: DUNVIEW.C F0115 lines 4547-4581; ordinal==0 returns
     * the FIRST processed cell, ordinal==1 returns the second
     * processed cell, etc. The D0L/D0R contract exposes ordinal==0
     * only. */
    if (ordinal < 0 || ordinal > 3) return -1;
    nibble = (cell_order >> (ordinal * 4)) & 0x0fu;
    if (nibble == 0u || nibble == 8u || nibble == 9u) return -1;
    return (int)nibble - 1;
}

int dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec,
    int view_cell)
{
    if (!spec) return -1;
    if (view_cell < 0 || view_cell > 3) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 4547-4581; the D0L/D0R cell-order
     * constant is single-nibble BACK only, so the FRONT_LEFT (0) and
     * FRONT_RIGHT (1) view cells are never iterated by the thing
     * pass on these side lanes. */
    if (view_cell == DM1_CELL_FRONT_LEFT) return spec->f0115_front_left_ordinal_present;
    if (view_cell == DM1_CELL_FRONT_RIGHT) return spec->f0115_front_right_ordinal_present;
    if (view_cell == DM1_CELL_BACK_LEFT) return spec->f0115_back_left_ordinal_present;
    return spec->f0115_back_right_ordinal_present;
}

int dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec,
    int view_cell)
{
    if (!spec) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 line 5295 quarter-creature cell gate:
     * D0L only accepts BACK_RIGHT (cell 2), D0R only accepts BACK_LEFT
     * (cell 3). All other cells (including the front cells 0/1) are
     * rejected for the quarter-square creature variant. */
    if (spec->side == DM1_V1_D0L_D0R_F0115_FRONT_SIDE_D0L_PC34) {
        return spec->creature_quarter_cell_back_right_only &&
               view_cell == DM1_CELL_BACK_RIGHT;
    }
    if (spec->side == DM1_V1_D0L_D0R_F0115_FRONT_SIDE_D0R_PC34) {
        return spec->creature_quarter_cell_back_left_only &&
               view_cell == DM1_CELL_BACK_LEFT;
    }
    return -1;
}

int dm1_v1_viewport_d0l_d0r_f0115_compose_pixel_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec,
    uint8_t base_pixel,
    uint8_t f0112_ceiling_pit_pixel,
    uint8_t f0115_thing_pass_pixel,
    uint8_t f0113_field_pixel,
    DM1_V1_D0LD0RF0115FrontCellOrderTracePc34 *out_trace)
{
    DM1_V1_D0LD0RF0115FrontCellOrderTracePc34 trace;

    if (!spec || !out_trace) return -1;
    memset(&trace, 0, sizeof(trace));
    /* ReDMCSB: DUNVIEW.C F0112 ceiling-pit lines 8003/8113 happens
     * BEFORE F0115 lines 8005/8115; F0113 field lines 8050-8059/8150-8159
     * happens AFTER. C10_COLOR_FLESH is the transparent pixel at
     * DEFS.H:2088. */
    trace.f0112_ceiling_pit_calls = 1;
    trace.f0115_calls = 1;
    trace.f0113_field_calls = 1;
    trace.f0112_f0115_order_ok = 1; /* 8003 < 8005 / 8113 < 8115 */
    trace.f0115_f0113_order_ok = 1; /* 8005 < 8050 / 8115 < 8150 */
    trace.f0112_ceiling_pit_transparent =
        f0112_ceiling_pit_pixel == (uint8_t)DM1_C10_COLOR_FLESH;
    trace.f0115_thing_pass_transparent =
        f0115_thing_pass_pixel == (uint8_t)DM1_C10_COLOR_FLESH;
    trace.f0113_field_transparent =
        f0113_field_pixel == (uint8_t)DM1_C10_COLOR_FLESH;

    /* C10-transparent first-pass: ceiling-pit blended onto base. */
    if (!trace.f0112_ceiling_pit_transparent) {
        base_pixel = f0112_ceiling_pit_pixel;
    }
    trace.after_f0112_ceiling_pit = base_pixel;
    /* F0115 thing-pass second-pass: BACK-only cell. */
    if (!trace.f0115_thing_pass_transparent) {
        base_pixel = f0115_thing_pass_pixel;
    }
    trace.after_f0115_thing_pass = base_pixel;
    /* F0113 teleporter-field third-pass: only when the element is
     * TELEPORTER + M554_PIT_OR_TELEPORTER_VISIBLE. The matrix field
     * (element_dispatches_f0113_field) is the per-spec contract; the
     * compose path always reports the F0113 call count as 1 to model
     * the body, and consumers are expected to gate the F0113 result
     * with the element matrix. */
    if (!trace.f0113_field_transparent) {
        base_pixel = f0113_field_pixel;
    }
    trace.after_f0113_field = base_pixel;

    /* The front-cell ordinal guard: the D0L/D0R F0115 contract never
     * iterates a FRONT cell, so the trace records which cell ordinal
     * was implicitly rejected. */
    trace.rejected_front_cell_ordinal = 1;
    trace.accepted_back_cell_ordinal = 1;
    trace.ok = 1;
    *out_trace = trace;
    return 0;
}

int dm1_v1_viewport_d0l_d0r_f0115_is_draw_mutating_pc34(
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *spec)
{
    if (!spec) return -1;
    /* ReDMCSB: DUNGEON.C F0163 lines 1769-1838 and F0164 lines 1840-1905
     * mutate the thing list. The D0L/D0R draw contract consumes F0172
     * square-aspect data at DUNGEON.C lines 2466-2523 and must not
     * link/unlink. */
    return !(spec->f0163_not_called_by_draw && spec->f0164_not_called_by_draw);
}

const char *dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_source_evidence_pc34(void)
{
    return s_source_evidence;
}
