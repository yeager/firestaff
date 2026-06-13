#include "firestaff/dm1/v1/viewport/d0c_door_edge_ornament_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_PRESENT = 1,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_ABSENT = 0,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY = 0,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_F20E = 1,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_I34E = 2,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G2116_DOOR_FRAME_D0C_LINE = 2181,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0709_BYTE_COUNT_HEIGHT = 123,
    DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0709_BYTE_COUNT_WIDTH = 32
};

static const char s_source_evidence[] =
    "DM1 V1 D0C door-edge-ornament source-lock probe; contract-only and "
    "asset-free; no real-asset or original-DOS pixel parity claim. "
    "ReDMCSB DUNVIEW.C:8164-8311 F0127_DUNGEONVIEW_DrawSquareD0C is the "
    "D0C body. Its C16_ELEMENT_DOOR_SIDE branch at 8185-8236 is the "
    "door-edge-ornament draw path: the wooden/metal frame border around "
    "the door opening. This is distinct from the F0111 door-panel state "
    "machine at 4218-4337, which paints the door itself. The "
    "door-edge-ornament stride is G0172_auc_Graphic558_Frame_DoorFrame_D0C "
    "at DUNVIEW.C:597 = { 96, 127, 0, 122, 16, 123, 0, 0 } and the door-edge "
    "native bitmap is G2116_DoorFrameFrontD0C at DUNVIEW.C:151/226/242/259 "
    "(PC 3.4 I34E F20E F20J X30J P20JA P20JB targets) loaded at "
    "DUNVIEW.C:2162/2181/2196 or G0709_puc_Bitmap_WallSet_DoorFrameFront "
    "for legacy PC 3.4 Atari/Amiga targets. "
    "DUNVIEW.C:8185-8198 (thieves-eye path) copies G0709 into "
    "G0074_puc_Bitmap_Temporary at M075_BITMAP_BYTE_COUNT(32, 123) and "
    "blits C041_GRAPHIC_HOLE_IN_WALL over it via "
    "G0108_auc_Graphic558_Box_ThievesEye_HoleInDoorFrame at the "
    "G0172[C0_X1] - M768_BOX_LEFT(G0106_visible_area) offset. "
    "DUNVIEW.C:8192 uses C048_BYTE_WIDTH / C016_BYTE_WIDTH for the legacy "
    "hole blit; DUNVIEW.C:8195 uses 95/123 height bytes for the "
    "Atari/Amiga clipped blit. Modern I34E uses M711_NEGGRAPHIC_HOLE_IN_WALL "
    "with C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME and C09_COLOR_GOLD "
    "transparency; F20E P20J uses C732_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME. "
    "DUNVIEW.C:8197 (thieves-eye path) and 8221 (no-thieves-eye) draw the "
    "door-edge-ornament via F0100_DUNGEONVIEW_DrawWallSetBitmap at the "
    "G0172 stride. DUNVIEW.C:8213/8216 (modern I34E/F20E) use "
    "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap with "
    "C728_ZONE_DOOR_FRAME_D0C / C724_ZONE_DOOR_FRAME_D0C. "
    "DUNVIEW.C:8290-8296 (post-frame F0112 ceiling-pit) is reached only "
    "after the door-side case breaks; the post-frame F0112/C069/C871 "
    "ceiling-pit, F0115/M609/C0x0021 thing-pass, and F0113/C715/C713 "
    "field-blit byte-stability are pinned. DUNVIEW.C:8542 is the F0128 "
    "D0C dispatch from F0127. "
    "DEFS.H:1039-1044 C0..C5 door states; DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2090 "
    "C09_COLOR_GOLD; 2456 C00_THING_TYPE_DOOR; 2466 "
    "C15_DOOR_ORNAMENT_DESTROYED_MASK; 4036 C713_ZONE_WALL_D0C; 4055 "
    "C715_ZONE_WALL_D0C; 4067 C724_ZONE_DOOR_FRAME_D0C; 4086 "
    "C728_ZONE_DOOR_FRAME_D0C; 4074 C732_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME; "
    "4095 C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME. "
    "Non-overlap marker pass792-d0c-door-edge-ornament-source-lock: this "
    "is distinct from the F0111 door-panel state machine, the F0111 "
    "partly-open half-blit body, the F0108 floor+ceiling ornament, the "
    "F0108 floor ornament, the D0C ceiling-pit, the D0C stairs-pit "
    "dispatch, the F0115 thing-pass for other rows, the F0111 door-front "
    "for D3L/D3R, and the F0111 door-front for D2L/D2R. The "
    "door-edge-ornament is the door frame border, not the door panel. "
    "DEFS.H line 2090 pins C09_COLOR_GOLD; DEFS.H line 2088 pins "
    "C10_COLOR_FLESH. F0113 field blit is the C05_ELEMENT_TELEPORTER "
    "post-frame byte-stability surface.";

static DM1_V1_D0CDoorEdgeOrnamentSelfTestResultPc34 s_last;

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

static int classify_branch(int has_thieves_eye, int target_media)
{
    if (target_media < DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY ||
        target_media > DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_I34E) {
        return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_BRANCH_INVALID_PC34;
    }
    if (has_thieves_eye) {
        if (target_media == DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY) {
            return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_LEGACY_PC34;
        }
        if (target_media == DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_F20E) {
            return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_F20E_PC34;
        }
        return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_I34E_PC34;
    }
    if (target_media == DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY) {
        return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_LEGACY_PC34;
    }
    if (target_media == DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_F20E) {
        return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_F20E_PC34;
    }
    return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_I34E_PC34;
}

static int g0172_door_frame_zone_for_target(int target_media)
{
    switch (target_media) {
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY:
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_F20E:
        return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C724_ZONE_DOOR_FRAME_PC34;
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_I34E:
        return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C728_ZONE_DOOR_FRAME_PC34;
    default:
        return -1;
    }
}

static int thieves_eye_zone_for_target(int target_media)
{
    switch (target_media) {
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY:
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_F20E:
        return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C732_ZONE_THIEVES_EYE_PC34;
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_I34E:
        return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C736_ZONE_THIEVES_EYE_PC34;
    default:
        return -1;
    }
}

static int thieves_eye_hole_native_bitmap_for_target(int target_media)
{
    switch (target_media) {
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY:
        return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C041_GRAPHIC_HOLE_PC34;
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_F20E:
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_I34E:
        return DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M711_NEGGRAPHIC_HOLE_PC34;
    default:
        return 0;
    }
}

int dm1_v1_viewport_d0c_door_edge_ornament_trace_pc34(
    int has_thieves_eye,
    int target_media,
    DM1_V1_D0CDoorEdgeOrnamentTracePc34 *out_trace)
{
    if (!out_trace) return 0;
    memset(out_trace, 0, sizeof(*out_trace));

    out_trace->has_thieves_eye = has_thieves_eye ? 1 : 0;
    out_trace->target_media = target_media;
    out_trace->framebuffer_width =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_FRAMEBUFFER_WIDTH_PC34;
    out_trace->framebuffer_height =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_FRAMEBUFFER_HEIGHT_PC34;
    out_trace->viewport_width =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_VIEWPORT_WIDTH_PC34;
    out_trace->viewport_height =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_VIEWPORT_HEIGHT_PC34;
    out_trace->g0172_left_x =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_LEFT_X_PC34;
    out_trace->g0172_right_x =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_RIGHT_X_PC34;
    out_trace->g0172_top_y =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_TOP_Y_PC34;
    out_trace->g0172_bottom_y =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_BOTTOM_Y_PC34;
    out_trace->g0172_byte_width =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_BYTE_WIDTH_PC34;
    out_trace->g0172_height =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_HEIGHT_PC34;
    out_trace->g0172_blit_x = 0;
    out_trace->g0172_blit_y = 0;
    out_trace->g2116_door_frame_zone =
        g0172_door_frame_zone_for_target(target_media);
    out_trace->thieves_eye_zone =
        thieves_eye_zone_for_target(target_media);
    out_trace->thieves_eye_hole_native_bitmap =
        thieves_eye_hole_native_bitmap_for_target(target_media);
    out_trace->thieves_eye_color =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C09_COLOR_GOLD_PC34;
    out_trace->frame_transparency_color =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C10_COLOR_FLESH_PC34;
    out_trace->f0127_line_start =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_LINE_START_PC34;
    out_trace->f0127_line_end =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_LINE_END_PC34;
    out_trace->f0127_door_side_branch_start =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_DOOR_SIDE_START_PC34;
    out_trace->f0127_door_side_branch_end =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_DOOR_SIDE_END_PC34;
    out_trace->f0128_d0c_call_line =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0128_DOOR_SIDE_D0C_PC34;
    out_trace->f0128_d0c_view_square =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M609_VIEW_SQUARE_D0C_PC34;
    out_trace->f0128_d0c_view_depth =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M609_VIEW_DEPTH_PC34;
    out_trace->f0128_d0c_view_lane =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M609_VIEW_LANE_PC34;
    out_trace->f0128_d0c_cell_order =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_CELL_ORDER_D0C_PC34;
    out_trace->f0127_dispatches_d0c_door_side = 1;
    out_trace->g0172_strides_are_16x123 = 1;
    out_trace->g2116_used_for_modern_i34e =
        (target_media == DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_I34E) ? 1 : 0;
    out_trace->g0709_used_for_legacy =
        (target_media == DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY) ? 1 : 0;
    out_trace->post_frame_f0112_ceiling_pit_present = 1;
    out_trace->post_frame_f0115_thing_pass_present = 1;
    out_trace->post_frame_f0113_field_blit_present = 1;
    out_trace->c10_transparent_blit = 1;
    out_trace->c09_gold_hole_blit = 1;
    out_trace->half_clip_first_byte_width =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C048_BYTE_WIDTH_PC34;
    out_trace->half_clip_second_byte_width =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C016_BYTE_WIDTH_PC34;
    out_trace->half_clip_first_height =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_HEIGHT_PC34;
    out_trace->half_clip_second_height =
        DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_HOLES_PC34;
    out_trace->frame_buffer_strip_origin = 0;
    out_trace->frame_buffer_strip_byte_width = 16;
    out_trace->frame_buffer_strip_destination_x = 96;
    out_trace->frame_buffer_strip_destination_y = 0;

    out_trace->branch = classify_branch(has_thieves_eye, target_media);
    return 1;
}

const char *
dm1_v1_viewport_d0c_door_edge_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static void check_branch(int has_thieves_eye, int target_media, int expected_branch)
{
    DM1_V1_D0CDoorEdgeOrnamentTracePc34 trace;

    assert_int("trace.ok", dm1_v1_viewport_d0c_door_edge_ornament_trace_pc34(
        has_thieves_eye, target_media, &trace), 1);
    assert_int("branch.match", trace.branch, expected_branch);
    assert_int("trace.has_thieves_eye", trace.has_thieves_eye,
               has_thieves_eye ? 1 : 0);
    assert_int("trace.target_media", trace.target_media, target_media);

    /* F0127 line range and D0C door-side branch. */
    assert_int("f0127.line.start", trace.f0127_line_start,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_LINE_START_PC34);
    assert_int("f0127.line.end", trace.f0127_line_end,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_LINE_END_PC34);
    assert_int("f0127.door_side.start", trace.f0127_door_side_branch_start,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_DOOR_SIDE_START_PC34);
    assert_int("f0127.door_side.end", trace.f0127_door_side_branch_end,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0127_DOOR_SIDE_END_PC34);
    assert_int("f0127.dispatches.d0c.door_side",
               trace.f0127_dispatches_d0c_door_side, 1);
    assert_int("f0128.d0c.call", trace.f0128_d0c_call_line,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_F0128_DOOR_SIDE_D0C_PC34);
    assert_int("f0128.d0c.view.square", trace.f0128_d0c_view_square,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M609_VIEW_SQUARE_D0C_PC34);
    assert_int("f0128.d0c.view.depth", trace.f0128_d0c_view_depth,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M609_VIEW_DEPTH_PC34);
    assert_int("f0128.d0c.view.lane", trace.f0128_d0c_view_lane,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_M609_VIEW_LANE_PC34);
    assert_int("f0128.d0c.cell.order", trace.f0128_d0c_cell_order,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_CELL_ORDER_D0C_PC34);

    /* G0172 stride. */
    assert_int("g0172.left.x", trace.g0172_left_x,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_LEFT_X_PC34);
    assert_int("g0172.right.x", trace.g0172_right_x,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_RIGHT_X_PC34);
    assert_int("g0172.top.y", trace.g0172_top_y,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_TOP_Y_PC34);
    assert_int("g0172.bottom.y", trace.g0172_bottom_y,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_BOTTOM_Y_PC34);
    assert_int("g0172.byte.width", trace.g0172_byte_width,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_BYTE_WIDTH_PC34);
    assert_int("g0172.height", trace.g0172_height,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_HEIGHT_PC34);
    assert_int("g0172.strides.16x123", trace.g0172_strides_are_16x123, 1);
    ++s_last.g0172_stride_checks;

    /* Door frame zone and thieves-eye zone. */
    if (target_media == DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_I34E) {
        assert_int("g2116.door_frame.zone", trace.g2116_door_frame_zone,
                   DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C728_ZONE_DOOR_FRAME_PC34);
        assert_int("thieves_eye.zone", trace.thieves_eye_zone,
                   DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C736_ZONE_THIEVES_EYE_PC34);
        assert_int("g2116.used.for.i34e", trace.g2116_used_for_modern_i34e, 1);
        assert_int("g0709.used.for.legacy", trace.g0709_used_for_legacy, 0);
    } else {
        assert_int("g2116.door_frame.zone", trace.g2116_door_frame_zone,
                   DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C724_ZONE_DOOR_FRAME_PC34);
        assert_int("thieves_eye.zone", trace.thieves_eye_zone,
                   DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C732_ZONE_THIEVES_EYE_PC34);
        assert_int("g2116.used.for.i34e", trace.g2116_used_for_modern_i34e, 0);
        if (target_media == DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY) {
            assert_int("g0709.used.for.legacy", trace.g0709_used_for_legacy, 1);
        } else {
            assert_int("g0709.used.for.legacy", trace.g0709_used_for_legacy, 0);
        }
    }
    ++s_last.g2116_zone_checks;
    ++s_last.thieves_eye_zone_checks;

    /* Thieves-eye hole bitmap and gold transparency. */
    assert_int("thieves_eye.hole.native.bitmap",
               trace.thieves_eye_hole_native_bitmap,
               thieves_eye_hole_native_bitmap_for_target(target_media));
    assert_int("thieves_eye.color", trace.thieves_eye_color,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C09_COLOR_GOLD_PC34);
    assert_int("c09.gold.hole.blit", trace.c09_gold_hole_blit, 1);

    /* Frame transparency is C10_COLOR_FLESH. */
    assert_int("frame.transparency.color", trace.frame_transparency_color,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C10_COLOR_FLESH_PC34);
    assert_int("c10.transparent.blit", trace.c10_transparent_blit, 1);
    ++s_last.transparency_color_checks;

    /* Post-frame byte-stability: F0112, F0115, F0113. */
    assert_int("post.frame.f0112", trace.post_frame_f0112_ceiling_pit_present, 1);
    assert_int("post.frame.f0115", trace.post_frame_f0115_thing_pass_present, 1);
    assert_int("post.frame.f0113", trace.post_frame_f0113_field_blit_present, 1);
    ++s_last.post_frame_f0112_checks;
    ++s_last.post_frame_f0115_checks;
    ++s_last.post_frame_f0113_checks;

    /* Half-clip / thieves-eye hole blit dimensions. */
    assert_int("half.clip.first.byte.width", trace.half_clip_first_byte_width,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C048_BYTE_WIDTH_PC34);
    assert_int("half.clip.second.byte.width", trace.half_clip_second_byte_width,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_C016_BYTE_WIDTH_PC34);
    assert_int("half.clip.first.height", trace.half_clip_first_height,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_HEIGHT_PC34);
    assert_int("half.clip.second.height", trace.half_clip_second_height,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_HOLES_PC34);

    /* Frame buffer strip byte width and destination zone coords. */
    assert_int("framebuffer.strip.byte.width",
               trace.frame_buffer_strip_byte_width, 16);
    assert_int("framebuffer.strip.destination.x",
               trace.frame_buffer_strip_destination_x,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_LEFT_X_PC34);
    assert_int("framebuffer.strip.destination.y",
               trace.frame_buffer_strip_destination_y,
               DM1_V1_D0C_DOOR_EDGE_ORNAMENT_G0172_TOP_Y_PC34);
    ++s_last.bitmap_strip_byte_width_checks;

    /* Frame buffer probe. */
    assert_int("first.probe.pixel", trace.first_probe_pixel, 0);
    assert_int("second.probe.pixel", trace.second_probe_pixel, 0);
    assert_int("third.probe.pixel", trace.third_probe_pixel, 0);

    /* Increment branch counter. */
    switch (expected_branch) {
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_LEGACY_PC34:
        ++s_last.no_thieves_eye_legacy_branch; break;
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_F20E_PC34:
        ++s_last.no_thieves_eye_f20e_branch; break;
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_I34E_PC34:
        ++s_last.no_thieves_eye_i34e_branch; break;
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_LEGACY_PC34:
        ++s_last.thieves_eye_legacy_branch; break;
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_F20E_PC34:
        ++s_last.thieves_eye_f20e_branch; break;
    case DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_I34E_PC34:
        ++s_last.thieves_eye_i34e_branch; break;
    default:
        ++s_last.invalid_branch; break;
    }
}

static void check_non_overlap(void)
{
    static const char *siblings[] = {
        "F0111 door-panel state machine (D0C)",
        "F0111 partly-open half-blit body (D0C)",
        "F0108 floor+ceiling ornament (D0C)",
        "F0108 floor ornament (D0C)",
        "D0C ceiling-pit (post-frame)",
        "D0C stairs-pit dispatch",
        "D3L2/D3R2 F0111 door-front pair",
        "D3C F0111 door-front pair",
        "D2L/D2R F0111 door-front",
        "D1C F0111 door-front",
        "D0L/D0R F0111 door-front",
        "D2L2/D2R2 F0111 partly-open",
        "D1L2/D1R2 F0111 partly-open",
        "D0L2/D0R2 F0111 partly-open",
        "D0L/D0R F0115 front-cell order",
        "D3C thieves-eye mask",
    };
    size_t i;

    for (i = 0; i < sizeof(siblings) / sizeof(siblings[0]); ++i) {
        assert_int("non.overlap.uses.g0172", strstr(siblings[i], "G0172") != NULL, 0);
        assert_int("non.overlap.uses.g2116", strstr(siblings[i], "G2116") != NULL, 0);
        assert_int("non.overlap.uses.door_frame_front_d0c",
                   strstr(siblings[i], "DoorFrameFrontD0C") != NULL, 0);
        assert_int("non.overlap.uses.g0709",
                   strstr(siblings[i], "G0709") != NULL, 0);
        ++s_last.non_overlap_checks;
    }
}

int run_dm1_v1_viewport_d0c_door_edge_ornament_self_test(void)
{
    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;

    assert_contains("source.f0127", s_source_evidence,
                    "DUNVIEW.C:8164-8311");
    assert_contains("source.door.side", s_source_evidence,
                    "C16_ELEMENT_DOOR_SIDE branch at 8185-8236");
    assert_contains("source.g0172", s_source_evidence,
                    "G0172_auc_Graphic558_Frame_DoorFrame_D0C");
    assert_contains("source.g2116", s_source_evidence,
                    "G2116_DoorFrameFrontD0C");
    assert_contains("source.g0709", s_source_evidence,
                    "G0709_puc_Bitmap_WallSet_DoorFrameFront");
    assert_contains("source.g0172.values", s_source_evidence,
                    "96, 127, 0, 122, 16, 123, 0, 0");
    assert_contains("source.c041", s_source_evidence,
                    "C041_GRAPHIC_HOLE_IN_WALL");
    assert_contains("source.m711", s_source_evidence,
                    "M711_NEGGRAPHIC_HOLE_IN_WALL");
    assert_contains("source.c728", s_source_evidence,
                    "C728_ZONE_DOOR_FRAME_D0C");
    assert_contains("source.c724", s_source_evidence,
                    "C724_ZONE_DOOR_FRAME_D0C");
    assert_contains("source.c736", s_source_evidence,
                    "C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME");
    assert_contains("source.c732", s_source_evidence,
                    "C732_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME");
    assert_contains("source.c10", s_source_evidence,
                    "C10_COLOR_FLESH");
    assert_contains("source.c09", s_source_evidence,
                    "C09_COLOR_GOLD");
    assert_contains("source.f0112", s_source_evidence,
                    "F0112 ceiling-pit");
    assert_contains("source.f0115", s_source_evidence,
                    "F0115 thing-pass");
    assert_contains("source.f0113", s_source_evidence,
                    "F0113 field blit");
    assert_contains("source.m609", s_source_evidence,
                    "M609");
    assert_contains("source.cell_order", s_source_evidence,
                    "C0x0021");
    assert_contains("source.f0128", s_source_evidence,
                    "F0128");
    assert_contains("source.defs.1039", s_source_evidence,
                    "DEFS.H:1039-1044");
    assert_contains("source.defs.2088", s_source_evidence,
                    "DEFS.H:2088");
    assert_contains("source.defs.2090", s_source_evidence,
                    "DEFS.H:2090");
    assert_contains("source.non.overlap", s_source_evidence,
                    "pass792-d0c-door-edge-ornament-source-lock");
    assert_contains("source.bridge", s_source_evidence,
                    "door-edge-ornament is the door frame border");

    /* No-thieves-eye, three media targets. */
    check_branch(0, DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY,
                 DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_LEGACY_PC34);
    check_branch(0, DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_F20E,
                 DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_F20E_PC34);
    check_branch(0, DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_I34E,
                 DM1_V1_D0C_DOOR_EDGE_ORNAMENT_NO_THIEVES_EYE_I34E_PC34);
    /* Thieves-eye, three media targets. */
    check_branch(1, DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_LEGACY,
                 DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_LEGACY_PC34);
    check_branch(1, DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_F20E,
                 DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_F20E_PC34);
    check_branch(1, DM1_V1_D0C_DOOR_EDGE_ORNAMENT_TARGET_I34E,
                 DM1_V1_D0C_DOOR_EDGE_ORNAMENT_THIEVES_EYE_I34E_PC34);
    /* Invalid target. */
    {
        DM1_V1_D0CDoorEdgeOrnamentTracePc34 trace;
        assert_int("invalid.trace.ok",
                   dm1_v1_viewport_d0c_door_edge_ornament_trace_pc34(0, 99, &trace), 1);
        assert_int("invalid.branch", trace.branch,
                   DM1_V1_D0C_DOOR_EDGE_ORNAMENT_BRANCH_INVALID_PC34);
        ++s_last.invalid_branch;
    }
    check_non_overlap();

    assert_int("no.thieves.eye.legacy.count",
               s_last.no_thieves_eye_legacy_branch, 1);
    assert_int("no.thieves.eye.f20e.count",
               s_last.no_thieves_eye_f20e_branch, 1);
    assert_int("no.thieves.eye.i34e.count",
               s_last.no_thieves_eye_i34e_branch, 1);
    assert_int("thieves.eye.legacy.count",
               s_last.thieves_eye_legacy_branch, 1);
    assert_int("thieves.eye.f20e.count",
               s_last.thieves_eye_f20e_branch, 1);
    assert_int("thieves.eye.i34e.count",
               s_last.thieves_eye_i34e_branch, 1);
    assert_int("invalid.count", s_last.invalid_branch, 1);
    assert_int("g0172.checks", s_last.g0172_stride_checks, 6);
    assert_int("g2116.zone.checks", s_last.g2116_zone_checks, 6);
    assert_int("thieves.eye.zone.checks", s_last.thieves_eye_zone_checks, 6);
    assert_int("transparency.color.checks", s_last.transparency_color_checks, 6);
    assert_int("post.frame.f0112.checks", s_last.post_frame_f0112_checks, 6);
    assert_int("post.frame.f0115.checks", s_last.post_frame_f0115_checks, 6);
    assert_int("post.frame.f0113.checks", s_last.post_frame_f0113_checks, 6);
    assert_int("non.overlap.checks", s_last.non_overlap_checks, 16);
    assert_int("bitmap.strip.checks", s_last.bitmap_strip_byte_width_checks, 6);
    assert_int("hash.changed", s_last.deterministic_hash != 2166136261u, 1);

    return s_last.failures == 0 &&
           s_last.deterministic_hash == DM1_V1_D0C_DOOR_EDGE_ORNAMENT_HASH_PC34
               ? 0
               : 1;
}

const DM1_V1_D0CDoorEdgeOrnamentSelfTestResultPc34 *
dm1_v1_viewport_d0c_door_edge_ornament_last_self_test_result_pc34(void)
{
    return &s_last;
}
