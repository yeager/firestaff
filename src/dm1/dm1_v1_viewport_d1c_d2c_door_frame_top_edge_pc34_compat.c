#include "firestaff/dm1/v1/viewport/d1c_d2c_door_frame_top_edge_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const char s_source_evidence[] =
    "DM1 V1 D1C/D2C door-frame-top edge source-lock gate. "
    "ReDMCSB DUNVIEW.C:605 G0174={64,159,22,24,48,3,0,0}; "
    "DUNVIEW.C:608 G0177={48,175,14,17,64,4,0,0}; "
    "DUNVIEW.C:7244 F0121 start; DUNVIEW.C:7313 C17_ELEMENT_DOOR_FRONT; "
    "DUNVIEW.C:7317 F0100(G0703,G0174); DUNVIEW.C:7323 "
    "F0104(G2115,C726); DUNVIEW.C:7328 F0104(G2115,C730); "
    "DUNVIEW.C:7727 F0124 start; DUNVIEW.C:7873 C17_ELEMENT_DOOR_FRONT; "
    "DUNVIEW.C:7877 F0100(G0704,G0177); DUNVIEW.C:7882 "
    "F0104(G2112,C729); DUNVIEW.C:7886 F0104(G2112,C733); "
    "DUNVIEW.C:8521/8533 F0128 caller sites; DEFS.H:2581/2584 and "
    "2599/2602 M603/M606 view-square indices; DEFS.H:2669 C0x0218 "
    "doorpass1 order; DEFS.H:4068-4073 and 4087-4093 door-frame-top "
    "zone families. Non-overlap: d2l_d2r_door_frame_top_edge, "
    "d1l_d1r_door_frame_top_edge, d1c_f0115_door_frame, "
    "d2l_d2r_f0111_partly_open_door, d1l_d1r_f0111_partly_open_door, "
    "d0c_door_edge_ornament. Contract-only; no real-asset or original "
    "DOS pixel-parity claim.";

static DM1_V1_D1C_D2CDoorFrameTopEdgeSelfTestResultPc34 s_last;

static uint32_t mix_u32(uint32_t hash, uint32_t value)
{
    int i;
    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t mix_string(uint32_t hash, const char *text)
{
    if (!text) return mix_u32(hash, 0xffffffffu);
    while (*text) {
        hash ^= (uint8_t)*text++;
        hash *= 16777619u;
    }
    return hash;
}

static void assert_int(const char *id, int got, int want)
{
    ++s_last.assertions;
    s_last.deterministic_hash = mix_string(s_last.deterministic_hash, id);
    s_last.deterministic_hash = mix_u32(s_last.deterministic_hash, (uint32_t)got);
    s_last.deterministic_hash = mix_u32(s_last.deterministic_hash, (uint32_t)want);
    if (got != want) ++s_last.failures;
}

static void assert_contains(const char *id, const char *haystack, const char *needle)
{
    assert_int(id, haystack && needle && strstr(haystack, needle) ? 1 : 0, 1);
}

static int zone_for(int square, int target)
{
    if (target == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34) {
        return DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34;
    }
    if (square == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34) {
        return target == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34
            ? DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C726_D2C_F20E_PC34
            : DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C730_D2C_I34E_PC34;
    }
    return target == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34
        ? DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C729_D1C_F20E_PC34
        : DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C733_D1C_I34E_PC34;
}

int dm1_v1_viewport_d1c_d2c_door_frame_top_edge_trace_pc34(
    int square,
    int target_media,
    DM1_V1_D1C_D2CDoorFrameTopEdgeTracePc34 *out_trace)
{
    if (!out_trace) return 0;
    if (square != DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34 &&
        square != DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D1C_PC34) {
        return 0;
    }
    if (target_media < DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34 ||
        target_media > DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34) {
        return 0;
    }

    memset(out_trace, 0, sizeof(*out_trace));
    out_trace->square = square;
    out_trace->target_media = target_media;
    out_trace->pass1_cell_order =
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_PASS1_CELL_ORDER_PC34;
    out_trace->zone_id = zone_for(square, target_media);
    out_trace->c10_transparency =
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34;

    if (square == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34) {
        out_trace->left_x =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_LEFT_X_PC34;
        out_trace->right_x =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_RIGHT_X_PC34;
        out_trace->top_y =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_TOP_Y_PC34;
        out_trace->bottom_y =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_BOTTOM_Y_PC34;
        out_trace->byte_width =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_BYTE_WIDTH_PC34;
        out_trace->height =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_HEIGHT_PC34;
        out_trace->x_offset =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_X_OFFSET_PC34;
        out_trace->y_offset =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_Y_OFFSET_PC34;
        out_trace->view_square =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_M603_D2C_MODERN_PC34;
        out_trace->legacy_bitmap_id =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G0703_D2_TOP_BITMAP_PC34;
        out_trace->legacy_stride_id =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G0174_D2C_STRIDE_PC34;
        out_trace->native_bitmap_id =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G2115_D2_TOP_NATIVE_PC34;
        out_trace->line_start =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_F0121_START_LINE_PC34;
        out_trace->door_front_line =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_DOOR_FRONT_LINE_PC34;
        out_trace->legacy_line =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_LEGACY_LINE_PC34;
        out_trace->f20e_line =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_F20E_LINE_PC34;
        out_trace->i34e_line =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_I34E_LINE_PC34;
        out_trace->f0128_line =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_F0128_LINE_PC34;
    } else {
        out_trace->left_x =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_LEFT_X_PC34;
        out_trace->right_x =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_RIGHT_X_PC34;
        out_trace->top_y =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_TOP_Y_PC34;
        out_trace->bottom_y =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_BOTTOM_Y_PC34;
        out_trace->byte_width =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_BYTE_WIDTH_PC34;
        out_trace->height =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_HEIGHT_PC34;
        out_trace->x_offset =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_X_OFFSET_PC34;
        out_trace->y_offset =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_Y_OFFSET_PC34;
        out_trace->view_square =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_M606_D1C_MODERN_PC34;
        out_trace->legacy_bitmap_id =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G0704_D1_TOP_BITMAP_PC34;
        out_trace->legacy_stride_id =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G0177_D1C_STRIDE_PC34;
        out_trace->native_bitmap_id =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G2112_D1_TOP_NATIVE_PC34;
        out_trace->line_start =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_F0124_START_LINE_PC34;
        out_trace->door_front_line =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_DOOR_FRONT_LINE_PC34;
        out_trace->legacy_line =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_LEGACY_LINE_PC34;
        out_trace->f20e_line =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_F20E_LINE_PC34;
        out_trace->i34e_line =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_I34E_LINE_PC34;
        out_trace->f0128_line =
            DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_F0128_LINE_PC34;
    }

    out_trace->band_inside_viewport =
        out_trace->left_x >= 0 &&
        out_trace->right_x < DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_VIEWPORT_WIDTH_PC34 &&
        out_trace->top_y >= 0 &&
        out_trace->bottom_y < DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_VIEWPORT_HEIGHT_PC34;
    out_trace->center_has_no_side_shift =
        out_trace->x_offset == 0 && out_trace->y_offset == 0;

    return 1;
}

const char *
dm1_v1_viewport_d1c_d2c_door_frame_top_edge_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static void validate_trace(int square, int target)
{
    DM1_V1_D1C_D2CDoorFrameTopEdgeTracePc34 t;
    const int ok =
        dm1_v1_viewport_d1c_d2c_door_frame_top_edge_trace_pc34(square, target, &t);
    assert_int("trace_ok", ok, 1);
    if (!ok) return;

    assert_int("square", t.square, square);
    assert_int("target", t.target_media, target);
    assert_int("pass1_cell_order", t.pass1_cell_order, 0x0218);
    assert_int("band_inside_viewport", t.band_inside_viewport, 1);
    assert_int("center_has_no_side_shift", t.center_has_no_side_shift, 1);
    assert_int("c10_transparency", t.c10_transparency, 10);
    s_last.order_checks += 1;
    s_last.viewport_band_checks += 1;

    if (square == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34) {
        assert_int("d2c_left", t.left_x, 64);
        assert_int("d2c_right", t.right_x, 159);
        assert_int("d2c_top", t.top_y, 22);
        assert_int("d2c_bottom", t.bottom_y, 24);
        assert_int("d2c_byte_width", t.byte_width, 48);
        assert_int("d2c_height", t.height, 3);
        assert_int("d2c_view_square", t.view_square, 6);
        assert_int("d2c_legacy_bitmap", t.legacy_bitmap_id, 703);
        assert_int("d2c_stride_id", t.legacy_stride_id, 174);
        assert_int("d2c_native", t.native_bitmap_id, 2115);
        assert_int("d2c_start", t.line_start, 7244);
        assert_int("d2c_door_front", t.door_front_line, 7313);
        assert_int("d2c_legacy_line", t.legacy_line, 7317);
        assert_int("d2c_f20e_line", t.f20e_line, 7323);
        assert_int("d2c_i34e_line", t.i34e_line, 7328);
        assert_int("d2c_f0128_line", t.f0128_line, 8521);
        if (target == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34)
            s_last.d2c_legacy_count += 1;
        if (target == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34)
            s_last.d2c_f20e_count += 1;
        if (target == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34)
            s_last.d2c_i34e_count += 1;
    } else {
        assert_int("d1c_left", t.left_x, 48);
        assert_int("d1c_right", t.right_x, 175);
        assert_int("d1c_top", t.top_y, 14);
        assert_int("d1c_bottom", t.bottom_y, 17);
        assert_int("d1c_byte_width", t.byte_width, 64);
        assert_int("d1c_height", t.height, 4);
        assert_int("d1c_view_square", t.view_square, 3);
        assert_int("d1c_legacy_bitmap", t.legacy_bitmap_id, 704);
        assert_int("d1c_stride_id", t.legacy_stride_id, 177);
        assert_int("d1c_native", t.native_bitmap_id, 2112);
        assert_int("d1c_start", t.line_start, 7727);
        assert_int("d1c_door_front", t.door_front_line, 7873);
        assert_int("d1c_legacy_line", t.legacy_line, 7877);
        assert_int("d1c_f20e_line", t.f20e_line, 7882);
        assert_int("d1c_i34e_line", t.i34e_line, 7886);
        assert_int("d1c_f0128_line", t.f0128_line, 8533);
        if (target == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34)
            s_last.d1c_legacy_count += 1;
        if (target == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34)
            s_last.d1c_f20e_count += 1;
        if (target == DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34)
            s_last.d1c_i34e_count += 1;
    }

    assert_int("zone", t.zone_id, zone_for(square, target));
    assert_int("legacy_route", t.legacy_bitmap_id == 703 || t.legacy_bitmap_id == 704, 1);
    assert_int("native_route", t.native_bitmap_id == 2115 || t.native_bitmap_id == 2112, 1);
    s_last.stride_checks += 1;
    s_last.zone_checks += 1;
    s_last.dispatch_checks += 1;
    s_last.bitmap_route_checks += 1;
}

int run_dm1_v1_viewport_d1c_d2c_door_frame_top_edge_self_test(void)
{
    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;

    validate_trace(
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34);
    validate_trace(
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34);
    validate_trace(
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34);
    validate_trace(
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D1C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34);
    validate_trace(
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D1C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34);
    validate_trace(
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D1C_PC34,
        DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34);

    {
        DM1_V1_D1C_D2CDoorFrameTopEdgeTracePc34 t;
        assert_int("null_out_reject",
            dm1_v1_viewport_d1c_d2c_door_frame_top_edge_trace_pc34(0, 0, NULL), 0);
        assert_int("bad_square_reject",
            dm1_v1_viewport_d1c_d2c_door_frame_top_edge_trace_pc34(99, 0, &t), 0);
        assert_int("bad_target_reject",
            dm1_v1_viewport_d1c_d2c_door_frame_top_edge_trace_pc34(0, 99, &t), 0);
        s_last.invalid_count = 3;
    }

    assert_contains("evidence_g0174", s_source_evidence, "G0174");
    assert_contains("evidence_g0177", s_source_evidence, "G0177");
    assert_contains("evidence_d2c_line", s_source_evidence, "7317");
    assert_contains("evidence_d1c_line", s_source_evidence, "7877");
    assert_contains("evidence_non_overlap_d2lr", s_source_evidence,
        "d2l_d2r_door_frame_top_edge");
    assert_contains("evidence_non_overlap_d1lr", s_source_evidence,
        "d1l_d1r_door_frame_top_edge");
    s_last.non_overlap_checks = 6;

    return s_last.failures == 0 ? 0 : 1;
}

const DM1_V1_D1C_D2CDoorFrameTopEdgeSelfTestResultPc34 *
dm1_v1_viewport_d1c_d2c_door_frame_top_edge_last_self_test_result_pc34(void)
{
    return &s_last;
}
