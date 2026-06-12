#include "dm1/dm1_v1_viewport_d1c_f0111_door_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * DM1 V1 D1C F0111 door transparency contract-only gate.
 *
 * Source anchors:
 * - ReDMCSB DUNVIEW.C F0111:4218-4337: open skip, C4 closed frame,
 *   C1..C3 state decrement at 4308, D1C LeftHorizontal/RightHorizontal
 *   selection at 4311-4313, C6 first-half C10 blit at 4317-4324, and
 *   3 | MASK0x4000 second-half C10 blit at 4325-4334.
 * - ReDMCSB DUNVIEW.C F0124:7727-7937: actual D1C body. The
 *   C17_ELEMENT_DOOR_FRONT branch at 7873-7911 dispatches F0111 with
 *   G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT,
 *   G0186_s_Graphic558_Frames_Door_D1C, and PC34 zone M631.
 * - ReDMCSB DUNVIEW.C F0128:8318-8542: D1C is dispatched at 8533 after
 *   D1L/D1R. DUNVIEW.C F0121:7244-7389 is D2C only and is cited here as
 *   a sibling/non-duplication anchor, not as the D1C body.
 * - ReDMCSB DEFS.H:1039-1044 C0..C5 door states, 2088 C10_COLOR_FLESH,
 *   2375/2431 door graphic set indexing, 2599 D1C view square, and
 *   4259 M631_ZONE_DOOR_D1C.
 */

static int g_assertions;
static int g_failures;
static uint32_t g_hash = 2166136261u;

static void hash_u32(uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        g_hash ^= (value >> (i * 8)) & 0xffu;
        g_hash *= 16777619u;
    }
}

static void check_int(const char *label, int got, int want,
                      const char *anchor)
{
    ++g_assertions;
    hash_u32((uint32_t)got);
    hash_u32((uint32_t)want);
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               anchor);
    }
}

static void check_true(const char *label, int got, const char *anchor)
{
    check_int(label, got ? 1 : 0, 1, anchor);
}

static void check_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    check_true(label, haystack && needle && strstr(haystack, needle), anchor);
}

static void check_frame(const char *label,
                        const DM1_V1_D1CF0111DoorFramePc34 *frame,
                        int x1, int x2, int y1, int y2, int byte_width,
                        int height, int source_x, int source_y,
                        const char *anchor)
{
    check_int(label, frame ? frame->x1 : -1, x1, anchor);
    check_int(label, frame ? frame->x2 : -1, x2, anchor);
    check_int(label, frame ? frame->y1 : -1, y1, anchor);
    check_int(label, frame ? frame->y2 : -1, y2, anchor);
    check_int(label, frame ? frame->byte_width : -1, byte_width, anchor);
    check_int(label, frame ? frame->height : -1, height, anchor);
    check_int(label, frame ? frame->source_x : -1, source_x, anchor);
    check_int(label, frame ? frame->source_y : -1, source_y, anchor);
}

static void test_identity_and_anchors(void)
{
    const DM1_V1_D1CF0111DoorSpecPc34 *spec =
        dm1_v1_viewport_d1c_f0111_door_spec_for_square_pc34(3);
    const char *e = dm1_v1_viewport_d1c_f0111_door_source_evidence_pc34();

    check_int("spec.count",
              (int)dm1_v1_viewport_d1c_f0111_door_spec_count_pc34(), 1,
              "DUNVIEW.C F0124 D1C single center-front route");
    check_true("spec.present", spec != 0, "DEFS.H:2599");
    check_true("spec.at0",
               dm1_v1_viewport_d1c_f0111_door_spec_at_pc34(0) == spec,
               "DUNVIEW.C:7908");
    check_true("spec.at1.null",
               dm1_v1_viewport_d1c_f0111_door_spec_at_pc34(1) == 0,
               "D1C single-route gate");
    check_true("spec.d2c.absent",
               dm1_v1_viewport_d1c_f0111_door_spec_for_square_pc34(6) == 0,
               "DUNVIEW.C:F0121 is D2C sibling only");

    check_int("contract_only", spec ? spec->source_locked_contract_only : -1,
              1, "no original DOS parity claim");
    check_int("no_assets", spec ? spec->no_real_asset_pixel_parity : -1, 1,
              "contract-only no asset pixel parity");
    check_int("no_game_data", spec ? spec->no_game_data_load : -1, 1,
              "contract-only no game-data load");
    check_int("view_square", spec ? spec->view_square_d1c : -1, 3,
              "DEFS.H:2599 M606_VIEW_SQUARE_D1C");
    check_int("element_door_front", spec ? spec->element_door_front : -1, 17,
              "DUNVIEW.C:7873 C17_ELEMENT_DOOR_FRONT");
    check_int("f0128_order", spec ? spec->f0128_dispatch_order : -1, 13,
              "DUNVIEW.C:8533");
    check_int("depth", spec ? spec->f0128_relative_depth : -1, 1,
              "DUNVIEW.C:8532");
    check_int("lane", spec ? spec->f0128_relative_lane : -9, 0,
              "DUNVIEW.C:8532");
    check_int("f0124_function", spec ? spec->f0124_function_number : -1, 124,
              "DUNVIEW.C:7727");
    check_int("f0121.sibling", spec ? spec->sibling_f0121_is_d2c_not_d1c : -1,
              1, "DUNVIEW.C:7244-7389");

    check_contains("evidence.f0111", e, "F0111:4218-4337",
                   "DUNVIEW.C F0111");
    check_contains("evidence.partly", e, "LeftHorizontal[state-1]",
                   "DUNVIEW.C:4311-4313");
    check_contains("evidence.c10", e, "C10_COLOR_FLESH", "DEFS.H:2088");
    check_contains("evidence.mask", e, "MASK0x4000", "DUNVIEW.C:4325");
    check_contains("evidence.f0124", e, "F0124:7727-7937",
                   "DUNVIEW.C actual D1C body");
    check_contains("evidence.f0128", e, "F0128:8318-8542",
                   "DUNVIEW.C viewport dispatch");
    check_contains("evidence.f0118", e, "F0118:6642-6763",
                   "DUNVIEW.C D3C hidden branch context");
    check_contains("evidence.f0121", e, "F0121:7244-7389",
                   "DUNVIEW.C D2C sibling context");
    check_contains("evidence.states", e, "DEFS.H:1039-1044",
                   "DEFS.H door states");
}

static void test_closed_d1c_door(void)
{
    DM1_V1_D1CF0111DoorStateTracePc34 trace;
    const DM1_V1_D1CF0111DoorSpecPc34 *spec =
        dm1_v1_viewport_d1c_f0111_door_spec_for_square_pc34(3);

    check_true("closed.trace",
               dm1_v1_viewport_d1c_f0111_door_state_trace_pc34(
                   spec, 4, 1, 0, 1, 0, &trace),
               "DUNVIEW.C:4297 C4 closed branch");
    check_int("closed.branch", trace.branch,
              DM1_V1_D1C_F0111_DOOR_BRANCH_CLOSED_PC34,
              "DEFS.H:1043 C4_DOOR_STATE_CLOSED");
    check_int("closed.draws", trace.draws_any_bitmap, 1,
              "DUNVIEW.C:4298 F0102/F0791 closed route");
    check_int("closed.frame", trace.closed_frame_selected, 1,
              "DUNVIEW.C:4297-4298 ClosedOrDestroyed");
    check_int("closed.native_bitmap", trace.native_bitmap_index, 248,
              "DUNVIEW.C:2651-2658 G0695 door type 0");
    check_int("closed.zone", trace.second_half_zone, 3790,
              "DEFS.H:4259 M631_ZONE_DOOR_D1C");
    check_frame("closed.frame.rect", &trace.closed_or_destroyed, 64, 159, 17,
                102, 48, 88, 0, 0,
                "DUNVIEW.C:694 G0186 ClosedOrDestroyed");
}

static void test_partly_open_d1c_door(void)
{
    DM1_V1_D1CF0111DoorStateTracePc34 trace;
    uint8_t src[6] = { 1, 10, 2, 10, 3, 4 };
    uint8_t dst[6] = { 9, 8, 7, 6, 5, 4 };
    int skipped = 0;
    const DM1_V1_D1CF0111DoorSpecPc34 *spec =
        dm1_v1_viewport_d1c_f0111_door_spec_for_square_pc34(3);

    check_true("partly.trace",
               dm1_v1_viewport_d1c_f0111_door_state_trace_pc34(
                   spec, 2, 1, 0, 1, 0, &trace),
               "DUNVIEW.C:4308-4334 partly-open branch");
    check_int("partly.branch", trace.branch,
              DM1_V1_D1C_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34,
              "DEFS.H:1041 C2_DOOR_STATE_CLOSED_HALF");
    check_int("partly.decremented", trace.decremented_state, 1,
              "DUNVIEW.C:4308 P0125_ui_DoorState--");
    check_int("partly.left", trace.left_horizontal_selected, 1,
              "DUNVIEW.C:4312 LeftHorizontal");
    check_int("partly.right", trace.right_horizontal_selected, 1,
              "DUNVIEW.C:4313 RightHorizontal");
    check_int("partly.first.zone", trace.first_half_zone, 3791,
              "DUNVIEW.C:4317 P2084 += decremented state");
    check_int("partly.c6.zone", trace.first_half_clip_zone, 3797,
              "DUNVIEW.C:4322 zone + C6_UNKNOWN");
    check_int("partly.second.zone", trace.second_half_zone, 20178,
              "DUNVIEW.C:4325 3|MASK0x4000");
    check_int("partly.mask", trace.horizontal_mask_applied, 1,
              "DUNVIEW.C:4325 MASK0x4000");
    check_int("partly.c10.first", trace.first_half_transparent_color, 10,
              "DEFS.H:2088 C10_COLOR_FLESH");
    check_int("partly.c10.second", trace.second_half_transparent_color, 10,
              "DUNVIEW.C:4334 F0791 C10");
    check_frame("partly.left.rect", &trace.selected_left_horizontal, 64, 87,
                17, 102, 48, 88, 24, 0,
                "DUNVIEW.C:694 G0186 LeftHorizontal[1]");
    check_frame("partly.right.rect", &trace.selected_right_horizontal, 136,
                159, 17, 102, 48, 88, 48, 0,
                "DUNVIEW.C:694 G0186 RightHorizontal[1]");
    check_contains("partly.frame.left",
                   dm1_v1_viewport_d1c_f0111_door_frame_name_pc34(
                       spec, 2, 0),
                   "LeftHorizontal", "DUNVIEW.C:4312");
    check_contains("partly.frame.right",
                   dm1_v1_viewport_d1c_f0111_door_frame_name_pc34(
                       spec, 2, 1),
                   "RightHorizontal", "DUNVIEW.C:4313");

    check_int("blit.copied",
              dm1_v1_viewport_d1c_f0111_door_synthetic_blit_pc34(
                  src, dst, 6, &skipped),
              4, "DUNVIEW.C:4322-4334 C10 transparent blit");
    check_int("blit.skipped", skipped, 2, "DEFS.H:2088 C10_COLOR_FLESH");
    check_int("blit.dst0", dst[0], 1, "copied non-C10 pixel");
    check_int("blit.dst1", dst[1], 8, "preserved destination under C10");
    check_int("blit.dst3", dst[3], 6, "preserved destination under C10");
}

static void test_fully_open_d1c_door(void)
{
    DM1_V1_D1CF0111DoorStateTracePc34 trace;
    const DM1_V1_D1CF0111DoorSpecPc34 *spec =
        dm1_v1_viewport_d1c_f0111_door_spec_for_square_pc34(3);

    check_true("open.trace",
               dm1_v1_viewport_d1c_f0111_door_state_trace_pc34(
                   spec, 0, 1, 0, 1, 0, &trace),
               "DUNVIEW.C:4248 state C0 open skips draw");
    check_int("open.branch", trace.branch,
              DM1_V1_D1C_F0111_DOOR_BRANCH_OPEN_PC34,
              "DEFS.H:1039 C0_DOOR_STATE_OPEN");
    check_int("open.draws", trace.draws_any_bitmap, 0,
              "DUNVIEW.C:4248 no F0791 route");
    check_int("open.frame.index", trace.selected_frame_state_index, 0,
              "open has no selected door frame");
    check_true("open.frame.name.null",
               dm1_v1_viewport_d1c_f0111_door_frame_name_pc34(
                   spec, 0, 0) == 0,
               "open does not select LeftHorizontal");
}

static void test_bitmap_index_progression_through_states(void)
{
    int state;
    const DM1_V1_D1CF0111DoorSpecPc34 *spec =
        dm1_v1_viewport_d1c_f0111_door_spec_for_square_pc34(3);

    check_int("bitmap.type0.set0",
              dm1_v1_viewport_d1c_f0111_door_native_bitmap_index_pc34(
                  spec, 0, 1, 0),
              248, "DUNVIEW.C:2651-2654 G0695[0]");
    check_int("bitmap.type1.set1",
              dm1_v1_viewport_d1c_f0111_door_native_bitmap_index_pc34(
                  spec, 0, 1, 1),
              251, "DUNVIEW.C:2655-2658 G0695[1]");
    check_int("bitmap.invalid",
              dm1_v1_viewport_d1c_f0111_door_native_bitmap_index_pc34(
                  spec, 0, 1, 2),
              -1, "door Type indexes only G0695[0..1]");

    for (state = 0; state <= 5; ++state) {
        DM1_V1_D1CF0111DoorStateTracePc34 trace;
        const int ok = dm1_v1_viewport_d1c_f0111_door_state_trace_pc34(
            spec, state, 1, 0, 1, 1, &trace);
        check_int("progress.trace.ok", ok, 1,
                  "DUNVIEW.C:4248-4334 state branches");
        check_int("progress.native.index", trace.native_bitmap_index, 251,
                  "DUNVIEW.C:2655-2658 native bitmap stable by door Type");
        if (state >= 1 && state <= 3) {
            check_int("progress.frame.state", trace.selected_frame_state_index,
                      state - 1, "DUNVIEW.C:4308 state decrement");
        }
    }
}

int main(void)
{
    test_identity_and_anchors();
    test_closed_d1c_door();
    test_partly_open_d1c_door();
    test_fully_open_d1c_door();
    test_bitmap_index_progression_through_states();

    if (g_failures != 0) {
        printf("DM1 D1C F0111 door contract FAIL assertions=%d failures=%d hash=0x%08x\n",
               g_assertions, g_failures, g_hash);
        return 1;
    }
    printf("DM1 D1C F0111 door contract PASS assertions=%d hash=0x%08x\n",
           g_assertions, g_hash);
    return 0;
}
