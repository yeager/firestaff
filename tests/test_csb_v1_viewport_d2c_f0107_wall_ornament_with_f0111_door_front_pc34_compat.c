#include "csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static const char *A_F0107 =
    "ReDMCSB DUNVIEW.C:3502-3938 F0107 wall ornament; D2C call 7308";
static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4339 F0111 closed door; 4298/4334";
static const char *A_F0121 =
    "ReDMCSB DUNVIEW.C:7244-7342 F0121_DUNGEONVIEW_DrawSquareD2C";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2537-2539,2581,2657-2677,2688-2690,4030-4049,4238-4257";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1016-1024,1865-1879,2596-2616,2949-2955";

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_int(label, haystack && needle &&
                         strstr(haystack, needle) != NULL, 1, anchor);
}

static int test_source_evidence(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0107F0111SpecPc34 *s =
        csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_spec_pc34();
    const char *e =
        csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_source_evidence_pc34();

    ok &= expect_contains("evidence.f0107 contains", e, "F0107_DUNGEONVIEW", A_F0107);
    ok &= expect_contains("evidence.f0111 contains", e, "F0111_DUNGEONVIEW", A_F0111);
    ok &= expect_contains("evidence.f0121 contains", e, "F0121_DUNGEONVIEW_DrawSquareD2C", A_F0121);
    ok &= expect_contains("evidence.defs contains", e, "DEFS.H:2537-2539", A_DEFS);
    ok &= expect_contains("evidence.lineage.f2stone", e, "Viewport.cpp:1016-1024", A_LINEAGE);
    ok &= expect_contains("evidence.lineage.f2door", e, "Viewport.cpp:1865-1879", A_LINEAGE);
    ok &= expect_contains("evidence.lineage.stddrawdoor", e, "Viewport.cpp:2596-2616", A_LINEAGE);
    ok &= expect_contains("evidence.lineage.decoration", e, "2949-2955", A_LINEAGE);
    ok &= expect_contains("spec.f0107.anchor", s ? s->redmcsb_f0107_anchor : NULL,
                          "7308", A_F0107);
    ok &= expect_contains("spec.f0111.anchor", s ? s->redmcsb_f0111_anchor : NULL,
                          "4334", A_F0111);
    ok &= expect_contains("spec.f0121.anchor", s ? s->redmcsb_f0121_anchor : NULL,
                          "7313-7341", A_F0121);
    ok &= expect_contains("spec.defs.anchor", s ? s->redmcsb_defs_anchor : NULL,
                          "2657-2677", A_DEFS);
    ok &= expect_contains("spec.lineage.anchor", s ? s->csb_lineage_viewport_anchor : NULL,
                          "StdDrawWallDecoration", A_LINEAGE);

    return ok;
}

static int test_spec_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0107F0111SpecPc34 *s =
        csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_spec_pc34();

    ok &= expect_int("spec.non_null", s != NULL, 1, A_F0121);
    ok &= expect_int("spec.contract.only", s ? s->source_locked_contract_only : 0, 1, A_F0121);
    ok &= expect_int("spec.no_game_data", s ? s->no_game_data_load : 0, 1, A_F0121);
    ok &= expect_int("spec.view_square", s ? s->view_square_d2c : -1, 6, A_DEFS);
    ok &= expect_int("spec.depth", s ? s->relative_depth : -1, 2, A_DEFS);
    ok &= expect_int("spec.lateral", s ? s->relative_lateral : -9, 0, A_DEFS);
    ok &= expect_int("spec.wall.element", s ? s->wall_element : -1, 0, A_F0121);
    ok &= expect_int("spec.door.element", s ? s->door_front_element : -1, 17, A_F0121);
    ok &= expect_int("spec.front.ornament.slot", s ? s->front_wall_ornament_ordinal_slot : -1, 5, A_DEFS);
    ok &= expect_int("spec.front.wall.view", s ? s->front_wall_view_index : -1, 10, A_DEFS);
    ok &= expect_int("spec.wall.bitmap", s ? s->wall_bitmap_index : -1, 9, A_DEFS);
    ok &= expect_int("spec.wall.zone", s ? s->wall_zone : -1, 709, A_DEFS);
    ok &= expect_int("spec.wall.frame.view_square", s ? s->wall_frame_view_square : -1, 6, A_DEFS);
    ok &= expect_int("spec.f0107.before.f0111", s ? s->f0107_before_f0111 : 0, 1, A_F0121);
    ok &= expect_int("spec.alcove.order", s ? s->f0107_alcove_cell_order : -1, 0x0000, A_DEFS);
    ok &= expect_int("spec.pass1.order", s ? s->f0111_doorpass1_cell_order : -1, 0x0218, A_DEFS);
    ok &= expect_int("spec.pass2.order", s ? s->f0111_doorpass2_cell_order : -1, 0x0349, A_DEFS);
    ok &= expect_int("spec.closed.state", s ? s->f0111_closed_door_state : -1, 4, A_F0111);
    ok &= expect_int("spec.door.zone", s ? s->f0111_door_zone : -1, 3760, A_DEFS);
    ok &= expect_int("spec.door.ornament.view", s ? s->f0111_door_ornament_view : -1, 1, A_DEFS);
    ok &= expect_int("spec.door.width", s ? s->f0111_door_bitmap_width : -1, 64, A_F0111);
    ok &= expect_int("spec.door.height", s ? s->f0111_door_bitmap_height : -1, 61, A_F0111);
    ok &= expect_int("spec.c10", s ? s->f0111_transparent_color : -1, 10, A_F0111);

    return ok;
}

static int test_geometry_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2CF0107F0111SpecPc34 *s =
        csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_spec_pc34();

    ok &= expect_int("viewport.width", CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34, 112, A_DEFS);
    ok &= expect_int("viewport.height", CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34, 61, A_DEFS);
    ok &= expect_int("transparent.color", CSB_V1_D2C_F0107_F0111_TRANSPARENT_COLOR_PC34, 10, A_F0111);
    ok &= expect_int("ornament.x1", s ? s->ornament_x1 : -1, 8, A_F0107);
    ok &= expect_int("ornament.x2", s ? s->ornament_x2 : -1, 103, A_F0107);
    ok &= expect_int("ornament.center.x1", s ? s->ornament_center_x1 : -1, 24, A_F0107);
    ok &= expect_int("ornament.center.x2", s ? s->ornament_center_x2 : -1, 87, A_F0107);
    ok &= expect_int("door.x1", s ? s->door_x1 : -1, 24, A_F0111);
    ok &= expect_int("door.x2", s ? s->door_x2 : -1, 87, A_F0111);
    ok &= expect_int("left.visible.x1", s ? s->left_visible_x1 : -1, 8, A_F0107);
    ok &= expect_int("left.visible.x2", s ? s->left_visible_x2 : -1, 23, A_F0107);
    ok &= expect_int("right.visible.x1", s ? s->right_visible_x1 : -1, 88, A_F0107);
    ok &= expect_int("right.visible.x2", s ? s->right_visible_x2 : -1, 103, A_F0107);
    ok &= expect_int("door.covers.center.x1",
                     s ? s->door_x1 == s->ornament_center_x1 : 0, 1, A_F0111);
    ok &= expect_int("door.covers.center.x2",
                     s ? s->door_x2 == s->ornament_center_x2 : 0, 1, A_F0111);

    return ok;
}

static int test_render_trace(void)
{
    int ok = 1;
    uint8_t canvas[CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34 *
                   CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34];
    CSB_V1_ViewportD2CF0107F0111TracePc34 t;
    const size_t canvas_size = sizeof(canvas);

    ok &= expect_int("render.result",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_render_pc34(
                         canvas, canvas_size, &t),
                     0, A_F0121);
    ok &= expect_int("trace.ok", t.ok, 1, A_F0121);
    ok &= expect_int("trace.wall.pixels", t.wall_pixels, 112 * 61, A_DEFS);
    ok &= expect_int("trace.ornament.before", t.ornament_pixels_before_door, 96 * 61, A_F0107);
    ok &= expect_int("trace.door.pixels", t.door_pixels, 64 * 61, A_F0111);
    ok &= expect_int("trace.center.covered", t.ornament_center_pixels_covered_by_door,
                     64 * 61, A_F0111);
    ok &= expect_int("trace.left.visible", t.ornament_left_pixels_visible_after_door,
                     16 * 61, A_F0107);
    ok &= expect_int("trace.right.visible", t.ornament_right_pixels_visible_after_door,
                     16 * 61, A_F0107);
    ok &= expect_int("trace.center.samples.opaque", t.center_samples_opaque, 1, A_F0111);
    ok &= expect_int("trace.left.samples.visible", t.left_samples_visible, 1, A_F0107);
    ok &= expect_int("trace.right.samples.visible", t.right_samples_visible, 1, A_F0107);
    ok &= expect_int("trace.draw.order", t.draw_order_f0107 < t.draw_order_f0111, 1, A_F0121);
    ok &= expect_contains("trace.evidence.f0107 contains", t.source_evidence,
                          "F0107_DUNGEONVIEW", A_F0107);
    ok &= expect_contains("trace.evidence.f0111 contains", t.source_evidence,
                          "F0111_DUNGEONVIEW", A_F0111);

    return ok;
}

static int test_pixel_samples(void)
{
    int ok = 1;
    uint8_t canvas[CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34 *
                   CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34];
    CSB_V1_ViewportD2CF0107F0111TracePc34 t;
    const size_t canvas_size = sizeof(canvas);

    (void)csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_render_pc34(
        canvas, canvas_size, &t);

    ok &= expect_int("pixel.left.8.0",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 8, 0),
                     21, A_F0107);
    ok &= expect_int("pixel.left.16.30",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 16, 30),
                     21, A_F0107);
    ok &= expect_int("pixel.left.23.60",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 23, 60),
                     21, A_F0107);
    ok &= expect_int("pixel.door.24.0",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 24, 0),
                     40, A_F0111);
    ok &= expect_int("pixel.door.32.12",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 32, 12),
                     40, A_F0111);
    ok &= expect_int("pixel.door.56.30",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 56, 30),
                     40, A_F0111);
    ok &= expect_int("pixel.door.80.48",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 80, 48),
                     40, A_F0111);
    ok &= expect_int("pixel.door.87.60",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 87, 60),
                     40, A_F0111);
    ok &= expect_int("pixel.right.88.0",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 88, 0),
                     23, A_F0107);
    ok &= expect_int("pixel.right.96.30",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 96, 30),
                     23, A_F0107);
    ok &= expect_int("pixel.right.103.60",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 103, 60),
                     23, A_F0107);
    ok &= expect_int("pixel.wall.0.0",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 0, 0),
                     1, A_F0121);
    ok &= expect_int("pixel.wall.111.60",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, canvas_size, 111, 60),
                     1, A_F0121);

    return ok;
}

static int test_rejections(void)
{
    int ok = 1;
    uint8_t canvas[CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34 *
                   CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34];
    CSB_V1_ViewportD2CF0107F0111TracePc34 t;

    ok &= expect_int("render.reject.null.canvas",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_render_pc34(
                         NULL, sizeof(canvas), &t),
                     -1, A_F0121);
    ok &= expect_int("render.reject.null.trace",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_render_pc34(
                         canvas, sizeof(canvas), NULL),
                     -1, A_F0121);
    ok &= expect_int("render.reject.short.canvas",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_render_pc34(
                         canvas, sizeof(canvas) - 1, &t),
                     -1, A_F0121);
    ok &= expect_int("pixel.reject.null",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         NULL, sizeof(canvas), 0, 0),
                     -1, A_F0121);
    ok &= expect_int("pixel.reject.xneg",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, sizeof(canvas), -1, 0),
                     -1, A_F0121);
    ok &= expect_int("pixel.reject.xwide",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, sizeof(canvas), 112, 0),
                     -1, A_F0121);
    ok &= expect_int("pixel.reject.yneg",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, sizeof(canvas), 0, -1),
                     -1, A_F0121);
    ok &= expect_int("pixel.reject.yhigh",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, sizeof(canvas), 0, 61),
                     -1, A_F0121);
    ok &= expect_int("pixel.reject.short",
                     csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
                         canvas, sizeof(canvas) - 1, 0, 0),
                     -1, A_F0121);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_source_evidence_pc34());

    ok &= test_source_evidence();
    ok &= test_spec_contract();
    ok &= test_geometry_contract();
    ok &= test_render_trace();
    ok &= test_pixel_samples();
    ok &= test_rejections();

    ok &= expect_int("assertion_count_between_80_and_120",
                     g_assertions >= 80 && g_assertions <= 120, 1, A_F0121);
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pc34_compat assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 0;
    }
    printf("FAIL csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pc34_compat assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return 1;
}
