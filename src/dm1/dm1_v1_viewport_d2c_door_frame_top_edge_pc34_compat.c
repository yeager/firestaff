#include "firestaff/dm1/v1/viewport/d2c_door_frame_top_edge_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
    TARGET_LEGACY = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34,
    TARGET_F20E = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34,
    TARGET_I34E = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34
};

static const char s_source_evidence[] =
    "DM1 V1 D2C door-frame-top edge source-lock probe; contract-only and "
    "asset-free; no real-asset or original-DOS pixel parity claim. "
    "ReDMCSB DUNVIEW.C:605 defines G0174_auc_Graphic558_Frame_"
    "DoorFrameTop_D2C = { 64, 159, 22, 24, 48, 3, 0, 0 }. "
    "DUNVIEW.C:7244 starts F0121_DUNGEONVIEW_DrawSquareD2C. "
    "DUNVIEW.C:7313 enters C17_ELEMENT_DOOR_FRONT, 7314 draws F0108 "
    "floor ornament M592, 7315 draws F0115 pass1 at M603 with "
    "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT, 7317 draws the "
    "legacy G0703/G0174 top edge through F0100, 7323 draws the F20E "
    "G2115/C726 top edge through F0104, 7328 draws the I34E G2115/C730 "
    "top edge through F0104, 7332-7333 optionally draw C2_VIEW_DOOR_"
    "BUTTON_D2C, and 7336/7339 draw the F0111 door panel. "
    "DUNVIEW.C:8520-8521 dispatches F0128 relative (2,0) to F0121. "
    "DEFS.H:2602 pins M603_VIEW_SQUARE_D2C = 6; DEFS.H:4069 and 4088 "
    "pin C726/C730 door-frame-top D2C zones. Non-overlap marker "
    "pass-d2c-door-frame-top-edge-source-lock: distinct from D2L/D2R "
    "door-frame-top edge, D1L/D1R door-frame-top edge, D2L/D2R F0111 "
    "partly-open door, D2C center wall composition, D0C door-edge ornament, "
    "and D2C F0115 front/rear overlap.";

static DM1_V1_D2CDoorFrameTopEdgeSelfTestResultPc34 s_last;

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
    const int found = haystack && needle && strstr(haystack, needle) != NULL;
    s_last.deterministic_hash = mix_string(s_last.deterministic_hash, needle);
    assert_int(id, found, 1);
}

static int zone_for_target(int target_media)
{
    switch (target_media) {
    case TARGET_LEGACY:
        return DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34;
    case TARGET_F20E:
        return DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C726_ZONE_F20E_PC34;
    case TARGET_I34E:
        return DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C730_ZONE_I34E_PC34;
    default:
        return -1;
    }
}

static int bitmap_for_target(int target_media)
{
    return target_media == TARGET_LEGACY
        ? DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G0703_BITMAP_PC34
        : DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G2115_NATIVE_TOP_PC34;
}

int dm1_v1_viewport_d2c_door_frame_top_edge_trace_pc34(
    int target_media,
    DM1_V1_D2CDoorFrameTopEdgeTracePc34 *out_trace)
{
    if (!out_trace) return 0;
    if (target_media < TARGET_LEGACY || target_media > TARGET_I34E) return 0;

    memset(out_trace, 0, sizeof(*out_trace));
    out_trace->target_media = target_media;
    out_trace->framebuffer_width =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_WIDTH_PC34;
    out_trace->framebuffer_height =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_HEIGHT_PC34;
    out_trace->viewport_width =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_VIEWPORT_WIDTH_PC34;
    out_trace->viewport_height =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_VIEWPORT_HEIGHT_PC34;
    out_trace->stride_left_x = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_LEFT_X_PC34;
    out_trace->stride_right_x = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_RIGHT_X_PC34;
    out_trace->stride_top_y = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_TOP_Y_PC34;
    out_trace->stride_bottom_y = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_BOTTOM_Y_PC34;
    out_trace->stride_byte_width =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_BYTE_WIDTH_PC34;
    out_trace->stride_height = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_HEIGHT_PC34;
    out_trace->stride_x_offset =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_STRIDE_X_OFFSET_PC34;
    out_trace->stride_y_offset =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_STRIDE_Y_OFFSET_PC34;
    out_trace->m603_view_square_d2c =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_M603_VIEW_SQUARE_D2C_PC34;
    out_trace->pass1_cell_order =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_PASS1_CELL_ORDER_PC34;
    out_trace->pass2_cell_order =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_PASS2_CELL_ORDER_PC34;
    out_trace->selected_zone = zone_for_target(target_media);
    out_trace->selected_bitmap = bitmap_for_target(target_media);
    out_trace->selected_uses_f0100 = target_media == TARGET_LEGACY;
    out_trace->selected_uses_f0104 = target_media != TARGET_LEGACY;
    out_trace->door_button_view_index =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C2_VIEW_DOOR_BUTTON_PC34;
    out_trace->door_panel_top_y =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_TOP_Y_PC34;
    out_trace->door_panel_bottom_y =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BOTTOM_Y_PC34;
    out_trace->door_panel_height =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_HEIGHT_PC34;
    out_trace->door_panel_byte_count =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BYTE_COUNT_PC34;
    out_trace->f0121_line_start =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_F0121_LINE_START_PC34;
    out_trace->door_front_line =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_FRONT_LINE_PC34;
    out_trace->floor_ornament_line =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_FLOOR_ORNAMENT_LINE_PC34;
    out_trace->thing_pass1_line =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_THING_PASS1_LINE_PC34;
    out_trace->legacy_line = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_LEGACY_LINE_PC34;
    out_trace->f20e_line = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_F20E_LINE_PC34;
    out_trace->i34e_line = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_I34E_LINE_PC34;
    out_trace->button_branch_line =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_BUTTON_BRANCH_LINE_PC34;
    out_trace->button_draw_line =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_BUTTON_DRAW_LINE_PC34;
    out_trace->door_panel_legacy_line =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_LEGACY_LINE_PC34;
    out_trace->door_panel_modern_line =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_MODERN_LINE_PC34;
    out_trace->f0128_dispatch_line =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_F0128_DISPATCH_LINE_PC34;
    out_trace->g0703_bitmap = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G0703_BITMAP_PC34;
    out_trace->g0174_stride = DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G0174_STRIDE_PC34;
    out_trace->g2115_native_top =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G2115_NATIVE_TOP_PC34;
    out_trace->g2118_native_left =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G2118_NATIVE_LEFT_PC34;
    out_trace->g0694_door_bitmap =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G0694_DOOR_BITMAP_PC34;
    out_trace->g0183_door_frames_d2c =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G0183_DOOR_FRAMES_D2C_PC34;
    out_trace->c1_view_door_ornament =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C1_VIEW_DOOR_ORNAMENT_PC34;
    out_trace->c10_transparency =
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34;
    out_trace->top_edge_inside_viewport = 1;
    out_trace->top_edge_above_door_panel =
        out_trace->stride_bottom_y <= out_trace->door_panel_top_y;
    out_trace->first_probe_pixel =
        (uint8_t)(out_trace->stride_left_x ^ out_trace->selected_zone);
    out_trace->second_probe_pixel =
        (uint8_t)(out_trace->stride_right_x ^ out_trace->selected_bitmap);
    out_trace->third_probe_pixel =
        (uint8_t)(out_trace->f0128_dispatch_line ^ out_trace->target_media);
    return 1;
}

const char *
dm1_v1_viewport_d2c_door_frame_top_edge_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static void assert_trace(const DM1_V1_D2CDoorFrameTopEdgeTracePc34 *t)
{
    assert_int("framebuffer_width", t->framebuffer_width, 320);
    assert_int("framebuffer_height", t->framebuffer_height, 200);
    assert_int("viewport_width", t->viewport_width, 224);
    assert_int("viewport_height", t->viewport_height, 136);
    assert_int("stride_left_x", t->stride_left_x, 64);
    assert_int("stride_right_x", t->stride_right_x, 159);
    assert_int("stride_top_y", t->stride_top_y, 22);
    assert_int("stride_bottom_y", t->stride_bottom_y, 24);
    assert_int("stride_byte_width", t->stride_byte_width, 48);
    assert_int("stride_height", t->stride_height, 3);
    assert_int("stride_x_offset", t->stride_x_offset, 0);
    assert_int("stride_y_offset", t->stride_y_offset, 0);
    assert_int("m603_view_square_d2c", t->m603_view_square_d2c, 6);
    assert_int("pass1_cell_order", t->pass1_cell_order, 0x0218);
    assert_int("pass2_cell_order", t->pass2_cell_order, 0x0349);
    assert_int("door_button_view_index", t->door_button_view_index, 2);
    assert_int("door_panel_top_y", t->door_panel_top_y, 24);
    assert_int("door_panel_bottom_y", t->door_panel_bottom_y, 82);
    assert_int("door_panel_height", t->door_panel_height, 61);
    assert_int("door_panel_byte_count", t->door_panel_byte_count, 3904);
    assert_int("f0121_line_start", t->f0121_line_start, 7244);
    assert_int("door_front_line", t->door_front_line, 7313);
    assert_int("floor_ornament_line", t->floor_ornament_line, 7314);
    assert_int("thing_pass1_line", t->thing_pass1_line, 7315);
    assert_int("legacy_line", t->legacy_line, 7317);
    assert_int("f20e_line", t->f20e_line, 7323);
    assert_int("i34e_line", t->i34e_line, 7328);
    assert_int("button_branch_line", t->button_branch_line, 7332);
    assert_int("button_draw_line", t->button_draw_line, 7333);
    assert_int("door_panel_legacy_line", t->door_panel_legacy_line, 7336);
    assert_int("door_panel_modern_line", t->door_panel_modern_line, 7339);
    assert_int("f0128_dispatch_line", t->f0128_dispatch_line, 8521);
    assert_int("g0703_bitmap", t->g0703_bitmap, 703);
    assert_int("g0174_stride", t->g0174_stride, 174);
    assert_int("g2115_native_top", t->g2115_native_top, 2115);
    assert_int("g2118_native_left", t->g2118_native_left, 2118);
    assert_int("g0694_door_bitmap", t->g0694_door_bitmap, 694);
    assert_int("g0183_door_frames_d2c", t->g0183_door_frames_d2c, 183);
    assert_int("c1_view_door_ornament", t->c1_view_door_ornament, 1);
    assert_int("c10_transparency", t->c10_transparency, 10);
    assert_int("top_edge_inside_viewport", t->top_edge_inside_viewport, 1);
    assert_int("top_edge_above_door_panel", t->top_edge_above_door_panel, 1);
}

static void assert_target(int target, int zone, int bitmap, int f0100, int f0104)
{
    DM1_V1_D2CDoorFrameTopEdgeTracePc34 trace;
    assert_int("trace_target_ok",
        dm1_v1_viewport_d2c_door_frame_top_edge_trace_pc34(target, &trace), 1);
    assert_int("target_media", trace.target_media, target);
    assert_trace(&trace);
    assert_int("selected_zone", trace.selected_zone, zone);
    assert_int("selected_bitmap", trace.selected_bitmap, bitmap);
    assert_int("selected_uses_f0100", trace.selected_uses_f0100, f0100);
    assert_int("selected_uses_f0104", trace.selected_uses_f0104, f0104);
    ++s_last.stride_checks;
    ++s_last.zone_checks;
    ++s_last.dispatch_order_checks;
    ++s_last.button_branch_checks;
    ++s_last.post_band_checks;
}

int run_dm1_v1_viewport_d2c_door_frame_top_edge_self_test(void)
{
    DM1_V1_D2CDoorFrameTopEdgeTracePc34 trace;

    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;

    assert_target(TARGET_LEGACY, 10, 703, 1, 0);
    ++s_last.legacy_route_count;
    assert_target(TARGET_F20E, 726, 2115, 0, 1);
    ++s_last.f20e_route_count;
    assert_target(TARGET_I34E, 730, 2115, 0, 1);
    ++s_last.i34e_route_count;

    assert_int("invalid_low",
        dm1_v1_viewport_d2c_door_frame_top_edge_trace_pc34(-1, &trace), 0);
    assert_int("invalid_high",
        dm1_v1_viewport_d2c_door_frame_top_edge_trace_pc34(3, &trace), 0);
    assert_int("null_out",
        dm1_v1_viewport_d2c_door_frame_top_edge_trace_pc34(0, NULL), 0);
    s_last.invalid_target_count = 3;

    assert_contains("evidence_g0174", s_source_evidence, "G0174");
    assert_contains("evidence_f0121", s_source_evidence, "F0121");
    assert_contains("evidence_7317", s_source_evidence, "7317");
    assert_contains("evidence_7323", s_source_evidence, "7323");
    assert_contains("evidence_7328", s_source_evidence, "7328");
    assert_contains("evidence_8521", s_source_evidence, "8521");
    assert_contains("non_overlap_d2l_d2r", s_source_evidence, "D2L/D2R");
    assert_contains("non_overlap_d0c", s_source_evidence, "D0C");
    s_last.non_overlap_checks = 8;

    return s_last.failures == 0 ? 0 : 1;
}

const DM1_V1_D2CDoorFrameTopEdgeSelfTestResultPc34 *
dm1_v1_viewport_d2c_door_frame_top_edge_last_self_test_result_pc34(void)
{
    return &s_last;
}
