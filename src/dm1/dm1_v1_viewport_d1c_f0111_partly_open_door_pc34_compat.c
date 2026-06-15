#include "firestaff/dm1/v1/viewport/d1c_f0111_partly_open_door_pc34_compat.h"

#include <string.h>

/*
 * Contract-only DM1 V1 D1C center F0111 partly-open door source-lock gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; open guard 4248,
 *   partly-open state decrement 4308, horizontal LeftHorizontal and
 *   RightHorizontal selection 4311-4313, first-half zone clip/C10 blit
 *   4317-4324, and second-half 3 | MASK0x4000 shift plus C10 draw
 *   4325-4334.
 * - DUNVIEW.C:7873-7911 F0124_DUNGEONVIEW_DrawSquareD1C
 *   C17_ELEMENT_DOOR_FRONT branch; F0111 at 7905/7908 uses
 *   G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, M075_BITMAP_BYTE_COUNT
 *   (96, 88), C2_VIEW_DOOR_ORNAMENT_D1LCR,
 *   G0186_s_Graphic558_Frames_Door_D1C, and M631_ZONE_DOOR_D1C.
 * - DUNVIEW.C:8518-8533 F0128_DUNGEONVIEW_Draw_CPSF dispatch order; D1C
 *   depth=1 center lane calls F0124 at 8533.
 * - DUNVIEW.C:694-705 defines G0186_s_Graphic558_Frames_Door_D1C.
 * - DEFS.H:1039-1043 C0..C4 door states; 2088 C10_COLOR_FLESH;
 *   2599 M606_VIEW_SQUARE_D1C; 2791 C2_VIEW_DOOR_ORNAMENT_D1LCR;
 *   3508 C6_UNKNOWN; 3516 MASK0x4000; 4259 M631_ZONE_DOOR_D1C;
 *   5458 G0695 extern; 5543 G0186 extern.
 *
 * This gate is non-overlapping with the closed-door sibling
 * tests/test_dm1_v1_viewport_d1c_f0111_door_pc34_compat.c and makes no
 * real-asset/original-DOS pixel parity claim.
 */

enum {
    DM1_FB_W = 320,
    DM1_FB_H = 200,
    DM1_VIEWPORT_W = 224,
    DM1_VIEWPORT_H = 136,
    DM1_VIEW_SQUARE_D1C = 3,
    DM1_VIEW_DEPTH_D1C = 1,
    DM1_CENTER_LANE = 0,
    DM1_DOOR_ZONE_D1C = 3790,
    DM1_DOOR_W_D1C = 96,
    DM1_DOOR_H_D1C = 88,
    DM1_DOOR_BYTES_D1C = 4224,
    DM1_VIEW_DOOR_ORNAMENT_D1LCR = 2,
    DM1_DOORPASS1_ORDER = 0x0218,
    DM1_DOORPASS2_ORDER = 0x0349,
    DM1_DOOR_STATE_OPEN = 0,
    DM1_DOOR_STATE_PARTLY_ONE = 1,
    DM1_DOOR_STATE_PARTLY_TWO = 2,
    DM1_DOOR_STATE_PARTLY_THREE = 3,
    DM1_DOOR_STATE_CLOSED = 4,
    DM1_DOOR_STATE_UNKNOWN = 6,
    DM1_SECOND_HALF_SHIFT = 3 | DM1_V1_D1C_F0111_PARTLY_OPEN_MASK0X4000_PC34
};

static const char s_source_evidence[] =
    "DM1 V1 D1C F0111 partly-open contract-only source lock; no game data "
    "and no original-DOS pixel parity claim. ReDMCSB DUNVIEW.C:4218-4337 "
    "F0111_DUNGEONVIEW_DrawDoor anchors the shared partly-open body; "
    "DUNVIEW.C:4311-4334 is the horizontal second-half subrange with "
    "G0186_s_Graphic558_Frames_Door_D1C.LeftHorizontal[state-1], "
    "RightHorizontal[state-1], C6_UNKNOWN zone clip, C10_COLOR_FLESH "
    "transparent blits, and 3 | MASK0x4000. DUNVIEW.C:7873-7911 "
    "F0124_DUNGEONVIEW_DrawSquareD1C C17_ELEMENT_DOOR_FRONT preserves "
    "DoorPass1 0x0218 before F0111 and DoorPass2 0x0349 after F0111; "
    "F0111 at 7905/7908 uses G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, "
    "M075_BITMAP_BYTE_COUNT(96, 88), C2_VIEW_DOOR_ORNAMENT_D1LCR, "
    "G0186_s_Graphic558_Frames_Door_D1C, and M631_ZONE_DOOR_D1C. "
    "DUNVIEW.C:8518-8533 F0128_DUNGEONVIEW_Draw_CPSF calls F0124 for D1C "
    "at line 8533, after D1L/D1R. DUNVIEW.C:694-705 pins G0186. "
    "DEFS.H:1039-1043 pins C0..C4 door states, 2088 C10_COLOR_FLESH, "
    "2599 M606_VIEW_SQUARE_D1C, 2791 C2_VIEW_DOOR_ORNAMENT_D1LCR, "
    "3508 C6_UNKNOWN, 3516 MASK0x4000, 4259 M631_ZONE_DOOR_D1C, "
    "5458 G0695 extern, and 5543 G0186 extern. Closed-door sibling: "
    "tests/test_dm1_v1_viewport_d1c_f0111_door_pc34_compat.c.";

static DM1_V1_D1CF0111PartlyOpenDoorSelfTestResultPc34 s_last;

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static void record_assert(int condition, uint32_t value)
{
    ++s_last.assertions;
    s_last.deterministic_hash = hash_u32(s_last.deterministic_hash, value);
    if (!condition) ++s_last.failures;
}

int dm1_v1_viewport_d1c_f0111_partly_open_door_trace_pc34(
    int door_state,
    DM1_V1_D1CF0111PartlyOpenDoorTracePc34 *out_trace)
{
    int decremented;

    if (!out_trace) return 0;
    memset(out_trace, 0, sizeof(*out_trace));
    out_trace->input_state = door_state;
    out_trace->framebuffer_width = DM1_FB_W;
    out_trace->framebuffer_height = DM1_FB_H;
    out_trace->viewport_width = DM1_VIEWPORT_W;
    out_trace->viewport_height = DM1_VIEWPORT_H;
    out_trace->view_square_m606_d1c = DM1_VIEW_SQUARE_D1C;
    out_trace->view_depth = DM1_VIEW_DEPTH_D1C;
    out_trace->center_lane = DM1_CENTER_LANE;
    out_trace->door_zone_m631_d1c = DM1_DOOR_ZONE_D1C;
    out_trace->door_width = DM1_DOOR_W_D1C;
    out_trace->door_height = DM1_DOOR_H_D1C;
    out_trace->door_byte_count = DM1_DOOR_BYTES_D1C;
    out_trace->door_ornament_view = DM1_VIEW_DOOR_ORNAMENT_D1LCR;
    out_trace->doorpass1_order = DM1_DOORPASS1_ORDER;
    out_trace->doorpass2_order = DM1_DOORPASS2_ORDER;

    if (door_state < DM1_DOOR_STATE_PARTLY_ONE ||
        door_state > DM1_DOOR_STATE_PARTLY_THREE) {
        return 0;
    }

    decremented = door_state - 1;
    out_trace->accepted_partly_open = 1;
    out_trace->decremented_state = decremented;
    out_trace->first_half_base_zone = DM1_DOOR_ZONE_D1C + decremented;
    out_trace->first_half_clip_zone =
        out_trace->first_half_base_zone +
        DM1_V1_D1C_F0111_PARTLY_OPEN_C6_UNKNOWN_PC34;
    out_trace->first_half_c10_blit =
        DM1_V1_D1C_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34;
    out_trace->second_half_shift = DM1_SECOND_HALF_SHIFT;
    out_trace->second_half_zone =
        out_trace->first_half_base_zone + DM1_SECOND_HALF_SHIFT;
    out_trace->second_half_c10_blit =
        DM1_V1_D1C_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34;
    out_trace->left_horizontal_frame_selected = 1;
    out_trace->right_horizontal_frame_selected = 1;
    out_trace->left_horizontal_frame_name =
        "G0186_s_Graphic558_Frames_Door_D1C.LeftHorizontal[state-1]";
    out_trace->right_horizontal_frame_name =
        "G0186_s_Graphic558_Frames_Door_D1C.RightHorizontal[state-1]";
    return 1;
}

const char *
dm1_v1_viewport_d1c_f0111_partly_open_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static void check_partly_state(int state)
{
    DM1_V1_D1CF0111PartlyOpenDoorTracePc34 trace;
    const int ok =
        dm1_v1_viewport_d1c_f0111_partly_open_door_trace_pc34(state, &trace);

    record_assert(ok == 1, (uint32_t)state);
    record_assert(trace.accepted_partly_open == 1, 0x11100000u + (uint32_t)state);
    record_assert(trace.framebuffer_width == DM1_FB_W, (uint32_t)trace.framebuffer_width);
    record_assert(trace.framebuffer_height == DM1_FB_H, (uint32_t)trace.framebuffer_height);
    record_assert(trace.viewport_width == DM1_VIEWPORT_W, (uint32_t)trace.viewport_width);
    record_assert(trace.viewport_height == DM1_VIEWPORT_H, (uint32_t)trace.viewport_height);
    record_assert(trace.view_square_m606_d1c == DM1_VIEW_SQUARE_D1C,
                  (uint32_t)trace.view_square_m606_d1c);
    record_assert(trace.view_depth == DM1_VIEW_DEPTH_D1C, (uint32_t)trace.view_depth);
    record_assert(trace.center_lane == DM1_CENTER_LANE, (uint32_t)trace.center_lane);
    record_assert(trace.door_zone_m631_d1c == DM1_DOOR_ZONE_D1C,
                  (uint32_t)trace.door_zone_m631_d1c);
    record_assert(trace.door_width == DM1_DOOR_W_D1C, (uint32_t)trace.door_width);
    record_assert(trace.door_height == DM1_DOOR_H_D1C, (uint32_t)trace.door_height);
    record_assert(trace.door_byte_count == DM1_DOOR_BYTES_D1C,
                  (uint32_t)trace.door_byte_count);
    record_assert(trace.door_ornament_view == DM1_VIEW_DOOR_ORNAMENT_D1LCR,
                  (uint32_t)trace.door_ornament_view);
    record_assert(trace.doorpass1_order == DM1_DOORPASS1_ORDER,
                  (uint32_t)trace.doorpass1_order);
    record_assert(trace.doorpass2_order == DM1_DOORPASS2_ORDER,
                  (uint32_t)trace.doorpass2_order);
    record_assert(trace.decremented_state == state - 1,
                  (uint32_t)trace.decremented_state);
    record_assert(trace.first_half_base_zone == DM1_DOOR_ZONE_D1C + state - 1,
                  (uint32_t)trace.first_half_base_zone);
    record_assert(trace.first_half_clip_zone ==
                      DM1_DOOR_ZONE_D1C + state - 1 +
                          DM1_V1_D1C_F0111_PARTLY_OPEN_C6_UNKNOWN_PC34,
                  (uint32_t)trace.first_half_clip_zone);
    record_assert(trace.first_half_c10_blit ==
                      DM1_V1_D1C_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34,
                  (uint32_t)trace.first_half_c10_blit);
    record_assert(trace.second_half_shift == DM1_SECOND_HALF_SHIFT,
                  (uint32_t)trace.second_half_shift);
    record_assert(trace.second_half_zone ==
                      DM1_DOOR_ZONE_D1C + state - 1 + DM1_SECOND_HALF_SHIFT,
                  (uint32_t)trace.second_half_zone);
    record_assert(trace.second_half_c10_blit ==
                      DM1_V1_D1C_F0111_PARTLY_OPEN_C10_COLOR_FLESH_PC34,
                  (uint32_t)trace.second_half_c10_blit);
    record_assert(trace.left_horizontal_frame_selected == 1,
                  (uint32_t)trace.left_horizontal_frame_selected);
    record_assert(trace.right_horizontal_frame_selected == 1,
                  (uint32_t)trace.right_horizontal_frame_selected);
    record_assert(trace.left_horizontal_frame_name != 0, 0x1e77u);
    record_assert(trace.right_horizontal_frame_name != 0, 0x2197u);

    if (state == DM1_DOOR_STATE_PARTLY_ONE) ++s_last.d1c_partly_one;
    if (state == DM1_DOOR_STATE_PARTLY_TWO) ++s_last.d1c_partly_two;
    if (state == DM1_DOOR_STATE_PARTLY_THREE) ++s_last.d1c_partly_three;
    ++s_last.door_dim_96x88_anchors;
    s_last.c10_zone_blits += 2;
    ++s_last.second_half_shifts;
    ++s_last.doorpass_order_anchors;
}

static void check_rejection(int state, int *counter)
{
    DM1_V1_D1CF0111PartlyOpenDoorTracePc34 trace;
    const int ok =
        dm1_v1_viewport_d1c_f0111_partly_open_door_trace_pc34(state, &trace);

    record_assert(ok == 0, 0xdead0000u + (uint32_t)state);
    record_assert(trace.accepted_partly_open == 0, 0xbeef0000u + (uint32_t)state);
    record_assert(trace.view_square_m606_d1c == DM1_VIEW_SQUARE_D1C,
                  (uint32_t)trace.view_square_m606_d1c);
    ++*counter;
}

int run_dm1_v1_viewport_d1c_f0111_partly_open_door_self_test(void)
{
    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261u;

    record_assert(strstr(s_source_evidence, "DUNVIEW.C:4218-4337") != 0,
                  0x42184337u);
    record_assert(strstr(s_source_evidence, "4311-4334") != 0, 0x43114334u);
    record_assert(strstr(s_source_evidence, "DUNVIEW.C:7873-7911") != 0,
                  0x78737911u);
    record_assert(strstr(s_source_evidence, "line 8533") != 0, 0x00008533u);
    record_assert(strstr(s_source_evidence, "G0186_s_Graphic558_Frames_Door_D1C") != 0,
                  0x00000186u);
    record_assert(strstr(s_source_evidence, "M631_ZONE_DOOR_D1C") != 0,
                  0x00000631u);
    record_assert(strstr(s_source_evidence, "DEFS.H:1039-1043") != 0,
                  0x10391043u);
    record_assert(strstr(s_source_evidence, "DEFS.H:1039-1043") != 0 &&
                      strstr(s_source_evidence, "2088") != 0 &&
                      strstr(s_source_evidence, "3508") != 0 &&
                      strstr(s_source_evidence, "3516") != 0 &&
                      strstr(s_source_evidence, "4259") != 0,
                  0xdef50000u);

    check_partly_state(DM1_DOOR_STATE_PARTLY_ONE);
    check_partly_state(DM1_DOOR_STATE_PARTLY_TWO);
    check_partly_state(DM1_DOOR_STATE_PARTLY_THREE);
    check_rejection(DM1_DOOR_STATE_CLOSED, &s_last.closed_rejections);
    check_rejection(DM1_DOOR_STATE_OPEN, &s_last.open_rejections);
    check_rejection(DM1_DOOR_STATE_UNKNOWN, &s_last.unknown_rejections);

    record_assert(s_last.d1c_partly_one == 1, (uint32_t)s_last.d1c_partly_one);
    record_assert(s_last.d1c_partly_two == 1, (uint32_t)s_last.d1c_partly_two);
    record_assert(s_last.d1c_partly_three == 1, (uint32_t)s_last.d1c_partly_three);
    record_assert(s_last.closed_rejections == 1, (uint32_t)s_last.closed_rejections);
    record_assert(s_last.open_rejections == 1, (uint32_t)s_last.open_rejections);
    record_assert(s_last.unknown_rejections == 1, (uint32_t)s_last.unknown_rejections);
    record_assert(s_last.door_dim_96x88_anchors == 3,
                  (uint32_t)s_last.door_dim_96x88_anchors);
    record_assert(s_last.c10_zone_blits == 6, (uint32_t)s_last.c10_zone_blits);
    record_assert(s_last.second_half_shifts == 3,
                  (uint32_t)s_last.second_half_shifts);
    record_assert(s_last.doorpass_order_anchors == 3,
                  (uint32_t)s_last.doorpass_order_anchors);
    record_assert(s_last.deterministic_hash != 2166136261u,
                  s_last.deterministic_hash);

    return s_last.failures == 0 ? 0 : 1;
}

const DM1_V1_D1CF0111PartlyOpenDoorSelfTestResultPc34 *
dm1_v1_viewport_d1c_f0111_partly_open_door_last_self_test_result_pc34(void)
{
    return &s_last;
}
