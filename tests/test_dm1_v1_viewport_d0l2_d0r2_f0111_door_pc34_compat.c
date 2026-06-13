#include "firestaff/dm1/v1/viewport/d0l2_d0r2_f0111_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;
static uint8_t g_sequence[256];
static size_t g_sequence_count;

static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8318-8486/8503-8508/8536-8541";
static const char *A_F0104_F0105 =
    "ReDMCSB DUNVIEW.C F0104/F0105 native C10:3113-3156/3185-3247";
static const char *A_F0107 =
    "ReDMCSB DUNVIEW.C F0107 wall-ornament keepout:3502-3938";
static const char *A_F0108 =
    "ReDMCSB DUNVIEW.C F0108 floor MASK0x8000 keepout:3940-4011";
static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C F0111 door-front:4311-4334/4350-4354";
static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C F0115 cell order:4547-4581/4923/5180-5188/5211-5214/5668-5671";
static const char *A_DUNGEON =
    "ReDMCSB DUNGEON.C F0163:1769-1838 F0164:1840-1905 F0172:2466-2523";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088/2596-2611/2662/2668-2677/4045-4046/4139-4153";

static void append_sequence(const DM1_V1_D0L2D0R2F0111DoorTracePc34 *trace)
{
    size_t i;

    for (i = 0; i < trace->opcode_count && g_sequence_count < sizeof(g_sequence); ++i) {
        g_sequence[g_sequence_count++] = trace->opcodes[i];
    }
}

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
    }
}

static void expect_u32(const char *id, uint32_t got, uint32_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08x want=0x%08x anchor=%s\n",
               id, got, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == 0) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)", anchor);
    }
}

static void expect_opcode(const char *id,
                          const DM1_V1_D0L2D0R2F0111DoorTracePc34 *trace,
                          size_t index,
                          uint8_t want,
                          const char *anchor)
{
    const uint8_t got = index < trace->opcode_count ? trace->opcodes[index] : 0xffu;

    expect_int(id, (int)got, (int)want, anchor);
}

static void check_source_lock(void)
{
    const char *evidence =
        dm1_v1_viewport_d0l2_d0r2_f0111_door_source_evidence_pc34();
    const DM1_V1_D0L2D0R2F0111DoorSourceLockPc34 *lock =
        dm1_v1_viewport_d0l2_d0r2_f0111_door_source_lock_pc34();

    expect_int("source_lock.present", lock != 0, 1, A_F0128);
    expect_int("source_lock.contract_only", lock ? lock->contract_only : 0, 1, A_F0111);
    expect_int("source_lock.no_game_data", lock ? lock->no_game_data : 0, 1, A_F0111);
    expect_int("source_lock.opcode_count", lock ? lock->trace_opcode_count : 0, 12, A_F0111);
    expect_int("source_lock.opcodes.present", lock && lock->trace_opcodes ? 1 : 0, 1, A_F0111);
    expect_contains("evidence.f0128", evidence, "8318-8486", A_F0128);
    expect_contains("evidence.d0l2.rel", evidence, "(2,-2)", A_F0128);
    expect_contains("evidence.d0r2.rel", evidence, "(2,+2)", A_F0128);
    expect_contains("evidence.d0.detail", evidence, "8536-8541", A_F0128);
    expect_contains("evidence.f0104", evidence, "3113-3156", A_F0104_F0105);
    expect_contains("evidence.f0105", evidence, "3185-3247", A_F0104_F0105);
    expect_contains("evidence.f0107", evidence, "3502-3938", A_F0107);
    expect_contains("evidence.f0108", evidence, "3940-4011", A_F0108);
    expect_contains("evidence.mask8000", evidence, "MASK0x8000", A_F0108);
    expect_contains("evidence.f0111", evidence, "4311-4334", A_F0111);
    expect_contains("evidence.partly", evidence, "4317-4325", A_F0111);
    expect_contains("evidence.f0112.context", evidence, "4350-4354", A_F0111);
    expect_contains("evidence.f0115", evidence, "4547-4581", A_F0115);
    expect_contains("evidence.f0163", evidence, "1769-1838", A_DUNGEON);
    expect_contains("evidence.f0164", evidence, "1840-1905", A_DUNGEON);
    expect_contains("evidence.f0172", evidence, "2466-2523", A_DUNGEON);
    expect_contains("evidence.defs.c10", evidence, "2088", A_DEFS);
    expect_contains("evidence.defs.cells", evidence, "2668-2677", A_DEFS);
    expect_contains("lock.f0128", lock ? lock->redmcsb_f0128_dispatch_anchor : 0,
                    "8503-8508", A_F0128);
    expect_contains("lock.c705", lock ? lock->redmcsb_defs_anchor : 0,
                    "4045-4046", A_DEFS);
}

static void check_cell_metadata(void)
{
    const DM1_V1_D0L2D0R2F0111DoorCellPc34 *left =
        dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_for_side_pc34(
            DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0L2_PC34);
    const DM1_V1_D0L2D0R2F0111DoorCellPc34 *right =
        dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_for_side_pc34(
            DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0R2_PC34);

    expect_int("cell.count", (int)dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_count_pc34(),
               2, A_F0128);
    expect_int("cell.at0.left",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_at_pc34(0) == left,
               1, A_F0128);
    expect_int("cell.at1.right",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_at_pc34(1) == right,
               1, A_F0128);
    expect_int("cell.at2.null",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_at_pc34(2) == 0,
               1, A_F0128);
    expect_int("cell.bad.null",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_cell_for_side_pc34(7) == 0,
               1, A_F0128);

    expect_int("left.order", left ? left->f0128_order : -1, 0, A_F0128);
    expect_int("right.order", right ? right->f0128_order : -1, 1, A_F0128);
    expect_int("left.depth", left ? left->relative_depth : -1, 2, A_F0128);
    expect_int("right.depth", right ? right->relative_depth : -1, 2, A_F0128);
    expect_int("left.lateral", left ? left->relative_lateral : 0, -2, A_F0128);
    expect_int("right.lateral", right ? right->relative_lateral : 0, 2, A_F0128);
    expect_int("left.update_line", left ? left->f0128_update_line : 0, 8503, A_F0128);
    expect_int("left.draw_line", left ? left->f0128_draw_line : 0, 8504, A_F0128);
    expect_int("right.update_line", right ? right->f0128_update_line : 0, 8507, A_F0128);
    expect_int("right.draw_line", right ? right->f0128_draw_line : 0, 8508, A_F0128);
    expect_int("left.requested_square", left ? left->requested_view_square : 0, 9, A_DEFS);
    expect_int("right.requested_square", right ? right->requested_view_square : 0, 10, A_DEFS);
    expect_int("left.source_square", left ? left->source_view_square : 0, 1, A_DEFS);
    expect_int("right.source_square", right ? right->source_view_square : 0, 2, A_DEFS);
    expect_int("left.wall_zone", left ? left->wall_zone : 0, 716, A_DEFS);
    expect_int("right.wall_zone", right ? right->wall_zone : 0, 717, A_DEFS);
    expect_int("left.wall_set_pcfix", left ? left->wall_set_after_pc_fix : 0, 6,
               "ReDMCSB DUNVIEW.C:8025-8033 PC_FIX_CODE_SIZE C05/C06 swap");
    expect_int("right.wall_set_pcfix", right ? right->wall_set_after_pc_fix : 0, 5,
               "ReDMCSB DUNVIEW.C:8135-8139 PC_FIX_CODE_SIZE C05/C06 swap");
    expect_int("left.door_zone", left ? left->door_zone : 0, 3720, A_F0111);
    expect_int("right.door_zone", right ? right->door_zone : 0, 3740, A_F0111);
    expect_int("left.pass1", left ? (int)left->f0115_pass1_cell_order : 0,
               0x0218, A_F0115);
    expect_int("right.pass1", right ? (int)right->f0115_pass1_cell_order : 0,
               0x0128, A_F0115);
    expect_int("left.pass2", left ? (int)left->f0115_pass2_cell_order : 0,
               0x0349, A_F0115);
    expect_int("right.pass2", right ? (int)right->f0115_pass2_cell_order : 0,
               0x0439, A_F0115);
    expect_int("left.contract", left ? left->source_locked_contract_only : 0, 1, A_F0111);
    expect_int("right.contract", right ? right->source_locked_contract_only : 0, 1, A_F0111);
    expect_int("left.no_data", left ? left->no_game_data_load : 0, 1, A_F0111);
    expect_int("right.no_data", right ? right->no_game_data_load : 0, 1, A_F0111);
}

static void check_decode_and_blend(void)
{
    expect_int("decode.left.pass1.0",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0218u, 0),
               0, A_F0115);
    expect_int("decode.left.pass1.1",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0218u, 1),
               1, A_F0115);
    expect_int("decode.right.pass1.0",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0128u, 0),
               1, A_F0115);
    expect_int("decode.right.pass1.1",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0128u, 1),
               0, A_F0115);
    expect_int("decode.left.pass2.0",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0349u, 0),
               3, A_F0115);
    expect_int("decode.left.pass2.1",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0349u, 1),
               2, A_F0115);
    expect_int("decode.right.pass2.0",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0439u, 0),
               2, A_F0115);
    expect_int("decode.right.pass2.1",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0439u, 1),
               3, A_F0115);
    expect_int("decode.terminator",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0000u, 0),
               -1, A_F0115);
    expect_int("decode.bad.high",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0218u, 4),
               -1, A_F0115);
    expect_int("decode.bad.low",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_decode_cell_pc34(0x0218u, -1),
               -1, A_F0115);

    expect_int("blend.c10.transparent",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_blend_pc34(0x33u, 10u),
               0x33, A_DEFS);
    expect_int("blend.opaque",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_blend_pc34(0x33u, 0x44u),
               0x44, A_F0104_F0105);
}

static DM1_V1_D0L2D0R2F0111DoorTracePc34 run_model(
    int side,
    int door_state,
    int door_on_cell,
    int door_vertical,
    int wall_keepout,
    int floor_keepout,
    uint8_t native_pixel,
    uint8_t door_pixel,
    uint8_t pass2_pixel)
{
    DM1_V1_D0L2D0R2F0111DoorInputPc34 input;
    DM1_V1_D0L2D0R2F0111DoorTracePc34 trace;

    memset(&input, 0, sizeof(input));
    memset(&trace, 0, sizeof(trace));
    input.side = side;
    input.door_on_cell = door_on_cell;
    input.door_state = door_state;
    input.door_vertical = door_vertical;
    input.wall_ornament_keepout = wall_keepout;
    input.floor_ornament_mask0x8000 = floor_keepout;
    input.pc_fix_code_size = 1;
    input.destination_pixel = 0x11u;
    input.native_blit_pixel = native_pixel;
    input.door_pixel = door_pixel;
    input.pass2_pixel = pass2_pixel;
    expect_int("model.ok",
               dm1_v1_viewport_d0l2_d0r2_f0111_door_model_pc34(&input, &trace),
               0, A_F0111);
    append_sequence(&trace);
    return trace;
}

static void check_closed_trace_one(int side)
{
    DM1_V1_D0L2D0R2F0111DoorTracePc34 trace =
        run_model(side,
                  DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_CLOSED_PC34,
                  1, 1, 0, 0, 0x22u, 0x44u, 0x55u);

    expect_int("closed.count", (int)trace.opcode_count, 7, A_F0111);
    expect_opcode("closed.op0", &trace, 0,
                  DM1_V1_D0L2_D0R2_TRACE_F0128_RELATIVE_PC34, A_F0128);
    expect_opcode("closed.op1", &trace, 1,
                  DM1_V1_D0L2_D0R2_TRACE_F0104_F0105_NATIVE_C10_PC34,
                  A_F0104_F0105);
    expect_opcode("closed.op2", &trace, 2,
                  DM1_V1_D0L2_D0R2_TRACE_F0115_PASS1_PC34, A_F0115);
    expect_opcode("closed.op3", &trace, 3,
                  DM1_V1_D0L2_D0R2_TRACE_F0111_ENTER_PC34, A_F0111);
    expect_opcode("closed.op4", &trace, 4,
                  DM1_V1_D0L2_D0R2_TRACE_F0111_CLOSED_PC34, A_F0111);
    expect_opcode("closed.op5", &trace, 5,
                  DM1_V1_D0L2_D0R2_TRACE_F0111_FINAL_C10_PC34, A_F0111);
    expect_opcode("closed.op6", &trace, 6,
                  DM1_V1_D0L2_D0R2_TRACE_F0115_PASS2_PC34, A_F0115);
    expect_int("closed.door_called", trace.door_called, 1, A_F0111);
    expect_int("closed.closed_selected", trace.closed_or_destroyed_selected, 1, A_F0111);
    expect_int("closed.partly_open", trace.partly_open_selected, 0, A_F0111);
    expect_int("closed.after_native", trace.after_native_blit, 0x22, A_F0104_F0105);
    expect_int("closed.after_door", trace.after_door, 0x44, A_F0111);
    expect_int("closed.after_pass2", trace.after_pass2, 0x55, A_F0115);
    expect_int("closed.wallset",
               trace.wall_set_selected,
               side == DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0L2_PC34 ? 6 : 5,
               "ReDMCSB DUNVIEW.C:8025-8033/8135-8139 PC_FIX_CODE_SIZE C05/C06 swap");
}

static DM1_V1_D0L2D0R2F0111DoorTracePc34 check_partly_open_trace_one(int side)
{
    DM1_V1_D0L2D0R2F0111DoorTracePc34 trace =
        run_model(side,
                  DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_CLOSED_HALF_PC34,
                  1, 0, 0, 0, 0x22u, 10u, 0x77u);

    expect_int("partly.count", (int)trace.opcode_count, 8, A_F0111);
    expect_opcode("partly.op5", &trace, 5,
                  DM1_V1_D0L2_D0R2_TRACE_F0111_C6_HALF_BLIT_PC34, A_F0111);
    expect_int("partly.selected", trace.partly_open_selected, 1, A_F0111);
    expect_int("partly.c6", trace.c6_half_blit_called, 1, A_F0111);
    expect_int("partly.c10_final", trace.final_c10_blit_called, 1, A_F0111);
    expect_int("partly.door_transparent_keeps_native", trace.after_door, 0x22,
               A_F0111);
    expect_int("partly.pass2_writes", trace.after_pass2, 0x77, A_F0115);
    expect_int("partly.c6_zone", trace.f0111_c6_zone,
               (side == DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0L2_PC34 ? 3720 : 3740) + 1 + 6,
               A_F0111);
    expect_int("partly.final_zone",
               trace.f0111_final_zone,
               (side == DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0L2_PC34 ? 3720 : 3740) +
                   1 + 3 + DM1_V1_D0L2_D0R2_F0111_DOOR_MASK0X4000_PC34,
               A_F0111);
    return trace;
}

static void check_negative_and_keepout(void)
{
    DM1_V1_D0L2D0R2F0111DoorTracePc34 no_door =
        run_model(DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0L2_PC34,
                  DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_CLOSED_PC34,
                  0, 0, 0, 0, 0x20u, 0x44u, 0x55u);
    DM1_V1_D0L2D0R2F0111DoorTracePc34 wall_keepout =
        run_model(DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0R2_PC34,
                  DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_CLOSED_PC34,
                  1, 0, 1, 0, 10u, 0x44u, 0x55u);
    DM1_V1_D0L2D0R2F0111DoorTracePc34 floor_keepout =
        run_model(DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0L2_PC34,
                  DM1_V1_D0L2_D0R2_F0111_DOOR_STATE_CLOSED_PC34,
                  1, 1, 0, 1, 0x20u, 0x44u, 10u);

    expect_int("no_door.count", (int)no_door.opcode_count, 3, A_DUNGEON);
    expect_opcode("no_door.op2", &no_door, 2,
                  DM1_V1_D0L2_D0R2_TRACE_NO_DOOR_ON_CELL_PC34, A_DUNGEON);
    expect_int("no_door.door_called", no_door.door_called, 0, A_DUNGEON);
    expect_int("no_door.final", no_door.after_pass2, 0x20, A_DUNGEON);

    expect_int("wall_keepout.count", (int)wall_keepout.opcode_count, 3, A_F0107);
    expect_opcode("wall_keepout.op2", &wall_keepout, 2,
                  DM1_V1_D0L2_D0R2_TRACE_F0107_WALL_KEEP_OUT_PC34, A_F0107);
    expect_int("wall_keepout.called", wall_keepout.f0107_keepout_called, 1, A_F0107);
    expect_int("wall_keepout.door_not_called", wall_keepout.door_called, 0, A_F0107);
    expect_int("wall_keepout.c10_native", wall_keepout.after_pass2, 0x11, A_F0104_F0105);

    expect_int("floor_keepout.count", (int)floor_keepout.opcode_count, 8, A_F0108);
    expect_opcode("floor_keepout.op2", &floor_keepout, 2,
                  DM1_V1_D0L2_D0R2_TRACE_F0108_MASK8000_KEEP_OUT_PC34, A_F0108);
    expect_int("floor_keepout.called", floor_keepout.f0108_mask0x8000_keepout_called,
               1, A_F0108);
    expect_int("floor_keepout.pass2_c10_keeps_door", floor_keepout.after_pass2,
               0x44, A_F0108);
}

static void check_mirror_symmetry(void)
{
    DM1_V1_D0L2D0R2F0111DoorTracePc34 left =
        check_partly_open_trace_one(DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0L2_PC34);
    DM1_V1_D0L2D0R2F0111DoorTracePc34 right =
        check_partly_open_trace_one(DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0R2_PC34);
    uint32_t left_hash =
        dm1_v1_viewport_d0l2_d0r2_f0111_door_fnv1a_pc34(left.opcodes,
                                                         left.opcode_count);
    uint32_t right_hash =
        dm1_v1_viewport_d0l2_d0r2_f0111_door_fnv1a_pc34(right.opcodes,
                                                         right.opcode_count);

    expect_int("mirror.count", (int)left.opcode_count, (int)right.opcode_count, A_F0111);
    expect_int("mirror.bytes", memcmp(left.opcodes, right.opcodes, left.opcode_count), 0,
               A_F0111);
    expect_u32("mirror.hash", left_hash, right_hash,
               "mirror symmetry: same F0111 byte pattern with C05/C06 wall-set swap");
    expect_int("mirror.wallset.swap",
               left.wall_set_selected == 6 && right.wall_set_selected == 5,
               1, "ReDMCSB DUNVIEW.C:8025-8033/8135-8139 PC_FIX_CODE_SIZE C05/C06 swap");
}

int main(void)
{
    uint32_t hash;

    printf("probe=firestaff_dm1_v1_viewport_d0l2_d0r2_f0111_door\n");
    check_source_lock();
    check_cell_metadata();
    check_decode_and_blend();
    check_closed_trace_one(DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0L2_PC34);
    check_closed_trace_one(DM1_V1_D0L2_D0R2_F0111_DOOR_SIDE_D0R2_PC34);
    check_mirror_symmetry();
    check_negative_and_keepout();

    hash = dm1_v1_viewport_d0l2_d0r2_f0111_door_fnv1a_pc34(g_sequence,
                                                            g_sequence_count);
    printf("dispatch_fnv1a=0x%08x bytes=%zu\n", hash, g_sequence_count);
    printf("summary: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures ? 1 : 0;
}
