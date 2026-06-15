#include "firestaff/dm1/v1/viewport/d0l2_d0r2_f0111_door_pc34_compat.h"

#include <string.h>

enum {
    DM1_D0L2_VIEW_SQUARE = 9,          /* ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2, the relative (2,-2) side slot. */
    DM1_D0R2_VIEW_SQUARE = 10,         /* ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2, the relative (2,+2) side slot. */
    DM1_D0L_VIEW_SQUARE = 1,           /* ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L, the D0 side-source order. */
    DM1_D0R_VIEW_SQUARE = 2,           /* ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R, the D0 side-source order. */
    DM1_D0L2_WALL_ZONE = 716,          /* ReDMCSB DEFS.H:4056 C716_ZONE_WALL_D0L. */
    DM1_D0R2_WALL_ZONE = 717,          /* ReDMCSB DEFS.H:4057 C717_ZONE_WALL_D0R. */
    DM1_D0L2_WALL_SET_BEFORE_FIX = 5,  /* ReDMCSB DUNVIEW.C:8033 PC_FIX_CODE_SIZE D0L wall-set side. */
    DM1_D0L2_WALL_SET_AFTER_FIX = 6,   /* ReDMCSB DUNVIEW.C:8033 PC_FIX_CODE_SIZE C05/C06 swap evidence. */
    DM1_D0R2_WALL_SET_BEFORE_FIX = 6,  /* ReDMCSB DUNVIEW.C:8139 PC_FIX_CODE_SIZE D0R wall-set side. */
    DM1_D0R2_WALL_SET_AFTER_FIX = 5,   /* ReDMCSB DUNVIEW.C:8139 PC_FIX_CODE_SIZE C05/C06 swap evidence. */
    DM1_D3L_DOOR_ZONE = 3720,          /* ReDMCSB DEFS.H:4252 M624_ZONE_DOOR_D3L, F0111 D0L/D0R subset source. */
    DM1_D3R_DOOR_ZONE = 3740,          /* ReDMCSB DEFS.H:4254 M626_ZONE_DOOR_D3R, F0111 D0L/D0R subset source. */
    DM1_D3L_FRAME_LEFT_ZONE = 718,     /* ReDMCSB DUNVIEW.C:6453 C718_ZONE_DOOR_FRAME_LEFT_D3L. */
    DM1_D3L_FRAME_RIGHT_ZONE = 719,    /* ReDMCSB DUNVIEW.C:6454 C719_ZONE_DOOR_FRAME_RIGHT_D3L. */
    DM1_D3R_FRAME_LEFT_ZONE = 720,     /* ReDMCSB DUNVIEW.C:6589 C720_ZONE_DOOR_FRAME_LEFT_D3R. */
    DM1_D3R_FRAME_RIGHT_ZONE = 721,    /* ReDMCSB DUNVIEW.C:6590 C721_ZONE_DOOR_FRAME_RIGHT_D3R. */
    DM1_D0L2_PASS1_ORDER = 0x0218,     /* ReDMCSB DEFS.H:2669 door-pass1 back-left/back-right. */
    DM1_D0R2_PASS1_ORDER = 0x0128,     /* ReDMCSB DEFS.H:2668 door-pass1 back-right/back-left. */
    DM1_D0L2_PASS2_ORDER = 0x0349,     /* ReDMCSB DEFS.H:2672 door-pass2 front-left/front-right. */
    DM1_D0R2_PASS2_ORDER = 0x0439      /* ReDMCSB DEFS.H:2675 door-pass2 front-right/front-left. */
};

static const DM1_V1_D0L2D0R2F0111DoorOpcodePc34 s_opcodes[] = {
    { DM1_V1_D0L2_D0R2_TRACE_F0128_RELATIVE_PC34, "F0128 relative (2,+/-2) dispatch", "ReDMCSB DUNVIEW.C F0128 lines 8318-8486 and 8503-8508" },
    { DM1_V1_D0L2_D0R2_TRACE_F0104_F0105_NATIVE_C10_PC34, "F0104/F0105 native C10 transparent blit", "ReDMCSB DUNVIEW.C F0104 lines 3113-3156; F0105 lines 3185-3247" },
    { DM1_V1_D0L2_D0R2_TRACE_F0107_WALL_KEEP_OUT_PC34, "F0107 wall-ornament keepout", "ReDMCSB DUNVIEW.C F0107 lines 3502-3938" },
    { DM1_V1_D0L2_D0R2_TRACE_F0108_MASK8000_KEEP_OUT_PC34, "F0108 MASK0x8000 keepout", "ReDMCSB DUNVIEW.C F0108 lines 3940-4011" },
    { DM1_V1_D0L2_D0R2_TRACE_F0115_PASS1_PC34, "F0115 pass1 cell order", "ReDMCSB DUNVIEW.C F0115 lines 4547-4581, 4923, 5180-5188, 5211-5214, 5668-5671" },
    { DM1_V1_D0L2_D0R2_TRACE_F0111_ENTER_PC34, "F0111 door-front dispatch", "ReDMCSB DUNVIEW.C F0111 lines 4311-4334" },
    { DM1_V1_D0L2_D0R2_TRACE_F0111_CLOSED_PC34, "F0111 closed/destroyed door", "ReDMCSB DUNVIEW.C F0111 lines 4297-4305" },
    { DM1_V1_D0L2_D0R2_TRACE_F0111_PARTLY_OPEN_PC34, "F0111 partly-open door decrement", "ReDMCSB DUNVIEW.C F0111 lines 4307-4318 and 4350-4354 context" },
    { DM1_V1_D0L2_D0R2_TRACE_F0111_C6_HALF_BLIT_PC34, "F0111 C6 half transparent blit", "ReDMCSB DUNVIEW.C F0111 lines 4322-4325" },
    { DM1_V1_D0L2_D0R2_TRACE_F0111_FINAL_C10_PC34, "F0111 final C10 transparent blit", "ReDMCSB DUNVIEW.C F0111 line 4334; DEFS.H line 2088" },
    { DM1_V1_D0L2_D0R2_TRACE_F0115_PASS2_PC34, "F0115 pass2 cell order", "ReDMCSB DEFS.H lines 2668-2677; DUNVIEW.C F0115 lines 4547-4581" },
    { DM1_V1_D0L2_D0R2_TRACE_NO_DOOR_ON_CELL_PC34, "negative no door on side cell", "ReDMCSB DUNGEON.C F0172 lines 2466-2523 square-aspect source" }
};

static const DM1_V1_D0L2D0R2F0111DoorCellPc34 s_cells[] = {
    {
        DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0L2_PC34,
        "D0L2 relative (2,-2) very-near side door-front source-lock",
        0,
        2,
        -2,
        8503,
        8504,
        DM1_D0L2_VIEW_SQUARE,
        DM1_D0L_VIEW_SQUARE,
        DM1_D0L2_WALL_ZONE,
        DM1_D0L2_WALL_SET_BEFORE_FIX,
        DM1_D0L2_WALL_SET_AFTER_FIX,
        DM1_D3L_DOOR_ZONE,
        DM1_D3L_FRAME_LEFT_ZONE,
        DM1_D3L_FRAME_RIGHT_ZONE,
        DM1_D0L2_PASS1_ORDER,
        DM1_D0L2_PASS2_ORDER,
        0,
        3,
        DM1_V1_D0L2_D0R2_F0111_DOOR_CONTRACT_ONLY_PC34,
        1,
        DM1_V1_D0L2_D0R2_F0111_DOOR_NO_GAME_DATA_PC34
    },
    {
        DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0R2_PC34,
        "D0R2 relative (2,+2) very-near side door-front source-lock",
        1,
        2,
        2,
        8507,
        8508,
        DM1_D0R2_VIEW_SQUARE,
        DM1_D0R_VIEW_SQUARE,
        DM1_D0R2_WALL_ZONE,
        DM1_D0R2_WALL_SET_BEFORE_FIX,
        DM1_D0R2_WALL_SET_AFTER_FIX,
        DM1_D3R_DOOR_ZONE,
        DM1_D3R_FRAME_LEFT_ZONE,
        DM1_D3R_FRAME_RIGHT_ZONE,
        DM1_D0R2_PASS1_ORDER,
        DM1_D0R2_PASS2_ORDER,
        1,
        2,
        DM1_V1_D0L2_D0R2_F0111_DOOR_CONTRACT_ONLY_PC34,
        1,
        DM1_V1_D0L2_D0R2_F0111_DOOR_NO_GAME_DATA_PC34
    }
};

static const DM1_V1_D0L2D0R2F0111DoorSourceLockPc34 s_source_lock = {
    DM1_V1_D0L2_D0R2_F0111_DOOR_CONTRACT_ONLY_PC34,
    DM1_V1_D0L2_D0R2_F0111_DOOR_NO_GAME_DATA_PC34,
    (int)(sizeof(s_opcodes) / sizeof(s_opcodes[0])),
    s_opcodes,
    "ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF lines 8318-8486; D0L2/D0R2 caller lines 8503-8508",
    "ReDMCSB DUNVIEW.C F0128 lines 8536-8541 D0L/D0R two-pass detail",
    "ReDMCSB DUNVIEW.C F0104 lines 3113-3156 and F0105 lines 3185-3247 native C10 transparent blits",
    "ReDMCSB DUNVIEW.C F0107 lines 3502-3938 wall-ornament keepout/alcove result",
    "ReDMCSB DUNVIEW.C F0108 lines 3940-4011 floor-ornament MASK0x8000 keepout and C10 blit",
    "ReDMCSB DUNVIEW.C F0111 lines 4311-4334, partly-open lines 4317-4325, and F0112 signature lines 4350-4354 context",
    "ReDMCSB DUNVIEW.C F0115 lines 4547-4581, 4923, 5180-5188, 5211-5214, 5668-5671",
    "ReDMCSB DUNGEON.C F0163 lines 1769-1838, F0164 lines 1840-1905, F0172 lines 2466-2523",
    "ReDMCSB DEFS.H line 2088 C10_COLOR_FLESH; lines 2596-2611 view squares; 2662 and 2668-2677 cell orders; 4045-4046 C705/C706; 4139-4153 zone band"
};

static const char s_source_evidence[] =
    "DM1 V1 D0L2/D0R2 F0111 door-front source-lock contract-only=1, "
    "no_game_data=1. ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF "
    "lines 8318-8486 anchors the draw caller; lines 8503-8508 pin the "
    "relative (2,-2)/(2,+2) D0L2/D0R2 order and lines 8536-8541 pin the "
    "D0L/D0R two-pass side detail used by the source subset. DUNVIEW.C "
    "F0104 lines 3113-3156 and F0105 lines 3185-3247 bind native C10 "
    "transparent blits; F0107 lines 3502-3938 binds wall-ornament keepout; "
    "F0108 lines 3940-4011 binds MASK0x8000 floor-ornament keepout and "
    "C10 transparency. DUNVIEW.C F0111 lines 4311-4334 and partly-open "
    "lines 4317-4325 bind closed/partly-open door-front state, C6 half "
    "blit, MASK0x4000 shift, and final C10 blit; lines 4350-4354 retain "
    "the following function-signature context. DUNVIEW.C F0115 lines "
    "4547-4581, 4923, 5180-5188, 5211-5214, and 5668-5671 bind thing-pass "
    "cell ordering around the door-on-side surface. DUNGEON.C F0163 "
    "1769-1838 and F0164 1840-1905 are mutation anchors not called by draw; "
    "F0172 2466-2523 supplies square aspect. DEFS.H:2088,2596-2611,2662, "
    "2668-2677,4045-4046,4139-4153 are pinned.";

static void trace_add(DM1_V1_D0L2D0R2F0111DoorTracePc34 *trace, uint8_t opcode)
{
    if (trace->opcode_count < sizeof(trace->opcodes)) {
        trace->opcodes[trace->opcode_count++] = opcode;
    }
}

const char *
dm1_v1_viewport_d0l2_d0r2_f0111_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_D0L2D0R2F0111DoorSourceLockPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_door_source_lock_pc34(void)
{
    return &s_source_lock;
}

size_t dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_count_pc34(void)
{
    return sizeof(s_cells) / sizeof(s_cells[0]);
}

const DM1_V1_D0L2D0R2F0111DoorCellPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_count_pc34()) {
        return 0;
    }
    return &s_cells[index];
}

const DM1_V1_D0L2D0R2F0111DoorCellPc34 *
dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_count_pc34(); ++i) {
        if (s_cells[i].side == side) return &s_cells[i];
    }
    return 0;
}

uint8_t dm1_v1_viewport_d0l2_d0r2_f0111_door_blend_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    /* ReDMCSB: DEFS.H line 2088 defines C10_COLOR_FLESH; DUNVIEW.C F0104
     * lines 3128/3145/3148/3151, F0105 lines 3201/3218/3239/3242, F0108
     * lines 3988-4004, F0111 line 4334, and F0115 lines 5180-5188 use it
     * as transparent color on these native blit paths. */
    return source_pixel == (uint8_t)DM1_V1_D0L2_D0R2_F0111_DOOR_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

int dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(
    unsigned int cell_order,
    int ordinal)
{
    unsigned int nibble;

    /* ReDMCSB: DUNVIEW.C F0115 lines 4547-4581 consumes DEFS.H lines
     * 2662/2668-2677 low-to-high nibbles; 8/9 are door-pass markers. */
    if (ordinal < 0 || ordinal >= 4) return -1;
    if ((cell_order & 0x0fu) == 8u || (cell_order & 0x0fu) == 9u) {
        cell_order >>= 4;
    }
    nibble = (cell_order >> ((unsigned int)ordinal * 4u)) & 0x0fu;
    if (nibble == 0u || nibble == 8u || nibble == 9u) return -1;
    return (int)nibble - 1;
}

int dm1_v1_viewport_d0l2_d0r2_f0111_door_model_pc34(
    const DM1_V1_D0L2D0R2F0111DoorInputPc34 *input,
    DM1_V1_D0L2D0R2F0111DoorTracePc34 *out_trace)
{
    const DM1_V1_D0L2D0R2F0111DoorCellPc34 *cell;
    uint8_t pixel;

    if (!input || !out_trace) return -1;
    cell = dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_for_side_pc34(input->side);
    if (!cell) return -1;
    memset(out_trace, 0, sizeof(*out_trace));
    out_trace->cell = cell;
    out_trace->wall_set_selected = input->pc_fix_code_size ?
        cell->wall_set_after_pc_fix : cell->wall_set_before_pc_fix;
    out_trace->f0111_zone = cell->door_zone;
    out_trace->f0111_c6_zone = -1;
    out_trace->f0111_final_zone = cell->door_zone;

    /* ReDMCSB: DUNVIEW.C F0128 lines 8318-8486 calls this side pair through
     * relative movement; lines 8503-8508 pin (2,-2) before (2,+2), while
     * lines 8536-8541 retain the D0L/D0R side-source ordering detail. */
    trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0128_RELATIVE_PC34);

    out_trace->f0104_f0105_native_c10_called = 1;
    trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0104_F0105_NATIVE_C10_PC34);
    pixel = dm1_v1_viewport_d0l2_d0r2_f0111_door_blend_pc34(
        input->destination_pixel, input->native_blit_pixel);
    out_trace->after_native_blit = pixel;

    if (input->wall_ornament_keepout) {
        out_trace->f0107_keepout_called = 1;
        trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0107_WALL_KEEP_OUT_PC34);
        out_trace->after_door = pixel;
        out_trace->after_pass2 = pixel;
        return 0;
    }
    if (input->floor_ornament_mask0x8000) {
        out_trace->f0108_mask0x8000_keepout_called = 1;
        trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0108_MASK8000_KEEP_OUT_PC34);
    }
    if (!input->door_on_cell || input->door_state == DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_OPEN_PC34) {
        trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_NO_DOOR_ON_CELL_PC34);
        out_trace->after_door = pixel;
        out_trace->after_pass2 = pixel;
        return 0;
    }

    out_trace->f0115_pass1_called = 1;
    trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0115_PASS1_PC34);
    out_trace->door_called = 1;
    trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0111_ENTER_PC34);
    if (input->door_state == DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_CLOSED_PC34 ||
        input->door_state == DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_DESTROYED_PC34) {
        out_trace->closed_or_destroyed_selected = 1;
        trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0111_CLOSED_PC34);
    } else {
        out_trace->partly_open_selected = 1;
        trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0111_PARTLY_OPEN_PC34);
        if (!input->door_vertical) {
            out_trace->c6_half_blit_called = 1;
            out_trace->f0111_c6_zone = cell->door_zone + (input->door_state - 1) + 6;
            out_trace->f0111_final_zone = cell->door_zone + (input->door_state - 1) +
                3 + DM1_V1_D0L2_D0R2_F0111_DOOR_MASK0X4000_PC34;
            trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0111_C6_HALF_BLIT_PC34);
        } else {
            out_trace->f0111_final_zone = cell->door_zone + (input->door_state - 1);
        }
    }

    out_trace->final_c10_blit_called = 1;
    trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0111_FINAL_C10_PC34);
    pixel = dm1_v1_viewport_d0l2_d0r2_f0111_door_blend_pc34(pixel,
                                                             input->door_pixel);
    out_trace->after_door = pixel;
    out_trace->f0115_pass2_called = 1;
    trace_add(out_trace, DM1_V1_D0L2_D0R2_TRACE_F0115_PASS2_PC34);
    pixel = dm1_v1_viewport_d0l2_d0r2_f0111_door_blend_pc34(pixel,
                                                             input->pass2_pixel);
    out_trace->after_pass2 = pixel;
    return 0;
}

uint32_t dm1_v1_viewport_d0l2_d0r2_f0111_door_fnv1a_pc34(
    const uint8_t *bytes,
    size_t count)
{
    size_t i;
    uint32_t hash = 2166136261u;

    if (!bytes && count != 0u) return 0u;
    for (i = 0; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}
