#include "csb/csb_v1_viewport_f0115_wall_text_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C:F0115:4547-4581";
static const char *A_F0107 =
    "ReDMCSB DUNVIEW.C:F0107:3502-3938";
static const char *A_F0124 =
    "ReDMCSB DUNVIEW.C:F0124 D1C wall:7825-7843";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C:F0128:8318-8486";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:C10:2088 C5:2527 zones:4139-4153 G0208:5576";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1192-1209,1903-1915,1930-1944";

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

static int expect_u32(const char *label, uint32_t got, uint32_t want,
                      const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%u want=%u anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%u anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_int(label, haystack && needle &&
                         strstr(haystack, needle) != NULL, 1, anchor);
}

static int test_spec_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportF0115WallTextOrnamentPc34Spec *s =
        csb_v1_viewport_f0115_wall_text_ornament_pc34_spec();

    ok &= expect_int("spec.non_null", s != NULL, 1, A_F0124);
    ok &= expect_int("spec.contract.only", s ? s->source_locked_contract_only : 0, 1, A_F0124);
    ok &= expect_int("spec.no_dos_parity", s ? s->no_original_dos_pixel_parity_claim : 0, 1, A_F0124);
    ok &= expect_int("spec.no_game_data", s ? s->no_game_data_load : 0, 1, A_F0124);
    ok &= expect_int("framebuffer.width", s ? s->framebuffer_width : -1, 320, A_DEFS);
    ok &= expect_int("framebuffer.height", s ? s->framebuffer_height : -1, 200, A_DEFS);
    ok &= expect_int("viewport.width", s ? s->viewport_width : -1, 224, A_DEFS);
    ok &= expect_int("viewport.height", s ? s->viewport_height : -1, 136, A_DEFS);
    ok &= expect_int("view_square.d1c", s ? s->view_square_d1c : -1, 3, A_DEFS);
    ok &= expect_int("view_depth.d1", s ? s->view_depth_d1 : -1, 1, A_F0115);
    ok &= expect_int("view_lane.center", s ? s->view_lane_center : -9, 0, A_F0115);
    ok &= expect_int("element.wall", s ? s->wall_element : -1, 0, A_F0124);
    ok &= expect_int("front_wall_ornament_slot", s ? s->front_wall_ornament_ordinal_slot : -1, 5, A_F0124);
    ok &= expect_int("d1c.front.wall.view", s ? s->d1c_front_wall_view_index : -1, 14, A_DEFS);
    ok &= expect_int("wall_text.ordinal", s ? s->wall_text_ornament_ordinal : -1, 1, A_F0107);
    ok &= expect_int("wall_text.index", s ? s->wall_text_ornament_index : -1, 0, A_F0107);
    ok &= expect_int("wall_text.coordinate_set", s ? s->wall_text_coordinate_set : -1, 0, A_F0107);
    ok &= expect_int("coordinate.height.index.c5", s ? s->coordinate_height_index_c5 : -1, 5, A_DEFS);
    ok &= expect_int("transparent.c10", s ? s->transparent_color_c10 : -1, 10, A_DEFS);
    ok &= expect_int("f0107.alcove", s ? s->f0107_reports_alcove : 0, 1, A_F0107);
    ok &= expect_int("f0115.alcove.order", s ? s->f0115_cell_order_alcove : -1, 0, A_F0115);
    ok &= expect_int("f0115.first_nibble.alcove", s ? s->f0115_first_nibble_alcove : 0, 1, A_F0115);

    return ok;
}

static int test_route_gate(void)
{
    int ok = 1;
    const CSB_V1_ViewportF0115WallTextOrnamentPc34Spec *s =
        csb_v1_viewport_f0115_wall_text_ornament_pc34_spec();

    ok &= expect_int("route.enabled",
                     csb_v1_viewport_f0115_wall_text_ornament_route_enabled_pc34(
                         s, 1, 14),
                     1, A_F0124);
    ok &= expect_int("route.reject.ordinal.zero",
                     csb_v1_viewport_f0115_wall_text_ornament_route_enabled_pc34(
                         s, 0, 14),
                     0, A_F0107);
    ok &= expect_int("route.reject.ordinal.other",
                     csb_v1_viewport_f0115_wall_text_ornament_route_enabled_pc34(
                         s, 2, 14),
                     0, A_F0107);
    ok &= expect_int("route.reject.view.d1l",
                     csb_v1_viewport_f0115_wall_text_ornament_route_enabled_pc34(
                         s, 1, 12),
                     0, A_F0107);
    ok &= expect_int("route.reject.null",
                     csb_v1_viewport_f0115_wall_text_ornament_route_enabled_pc34(
                         NULL, 1, 14),
                     0, A_F0107);
    ok &= expect_int("text.box.x", s ? s->text_box_x : -1, 88, A_F0107);
    ok &= expect_int("text.box.y", s ? s->text_box_y : -1, 48, A_F0107);
    ok &= expect_int("text.box.width", s ? s->text_box_width : -1, 48, A_F0107);
    ok &= expect_int("text.box.height", s ? s->text_box_height : -1, 24, A_F0107);

    return ok;
}

static int test_render_trace_and_hash(void)
{
    int ok = 1;
    uint8_t framebuffer[
        CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_WIDTH_PC34 *
        CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_HEIGHT_PC34];
    CSB_V1_ViewportF0115WallTextOrnamentPc34Trace trace;

    memset(framebuffer, 0xee, sizeof(framebuffer));

    ok &= expect_int("render.result",
                     csb_v1_viewport_f0115_wall_text_ornament_render_pc34(
                         framebuffer, sizeof(framebuffer), &trace),
                     0, A_F0124);
    ok &= expect_int("trace.ok", trace.ok, 1, A_F0124);
    ok &= expect_int("trace.route.enabled", trace.route_enabled, 1, A_F0124);
    ok &= expect_int("trace.wall_pixels", trace.wall_pixels, 224 * 136, A_DEFS);
    ok &= expect_int("trace.text_non_zero", trace.text_pixels_non_zero, 497, A_F0107);
    ok &= expect_int("trace.transparent_preserved", trace.transparent_pixels_preserved,
                     (48 * 24) - 497, A_DEFS);
    ok &= expect_int("trace.outside_viewport_preserved",
                     trace.outside_viewport_preserved, 1, A_DEFS);
    ok &= expect_int("trace.order", trace.f0107_order < trace.f0115_order, 1, A_F0115);
    ok &= expect_int("trace.first_text_x", trace.first_text_x, 88, A_F0107);
    ok &= expect_int("trace.first_text_y", trace.first_text_y, 48, A_F0107);
    ok &= expect_u32("trace.hash", trace.framebuffer_hash, 1933097097u, A_F0124);
    ok &= expect_u32("hash.func", csb_v1_viewport_f0115_wall_text_ornament_hash_pc34(
                         framebuffer, sizeof(framebuffer)),
                     trace.framebuffer_hash, A_F0124);
    ok &= expect_int("hash.nonzero", trace.framebuffer_hash != 0u, 1, A_F0124);
    ok &= expect_contains("trace.evidence", trace.source_evidence,
                          "no original DOS pixel parity claim", A_F0124);

    return ok;
}

static int test_pixel_samples(void)
{
    int ok = 1;
    uint8_t framebuffer[
        CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_WIDTH_PC34 *
        CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_HEIGHT_PC34];
    CSB_V1_ViewportF0115WallTextOrnamentPc34Trace trace;

    memset(framebuffer, 0xee, sizeof(framebuffer));
    (void)csb_v1_viewport_f0115_wall_text_ornament_render_pc34(
        framebuffer, sizeof(framebuffer), &trace);

    ok &= expect_int("pixel.wall.0.0",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 0, 0),
                     3, A_F0124);
    ok &= expect_int("pixel.wall.223.135",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 223, 135),
                     3, A_F0124);
    ok &= expect_int("pixel.outside.224.0",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 224, 0),
                     0xee, A_DEFS);
    ok &= expect_int("pixel.outside.319.199",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 319, 199),
                     0xee, A_DEFS);
    ok &= expect_int("pixel.text.88.48",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 88, 48),
                     15, A_F0107);
    ok &= expect_int("pixel.text.99.55",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 99, 55),
                     15, A_F0107);
    ok &= expect_int("pixel.text.transparent.94.52",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 94, 52),
                     3, A_DEFS);
    ok &= expect_int("pixel.text.transparent.102.53",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 102, 53),
                     3, A_DEFS);
    ok &= expect_int("pixel.reject.null",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         NULL, sizeof(framebuffer), 0, 0),
                     -1, A_DEFS);
    ok &= expect_int("pixel.reject.short",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer) - 1, 0, 0),
                     -1, A_DEFS);
    ok &= expect_int("pixel.reject.xneg",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), -1, 0),
                     -1, A_DEFS);
    ok &= expect_int("pixel.reject.xwide",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 320, 0),
                     -1, A_DEFS);
    ok &= expect_int("pixel.reject.yneg",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 0, -1),
                     -1, A_DEFS);
    ok &= expect_int("pixel.reject.yhigh",
                     csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
                         framebuffer, sizeof(framebuffer), 0, 200),
                     -1, A_DEFS);

    return ok;
}

static int test_render_rejections(void)
{
    int ok = 1;
    uint8_t framebuffer[
        CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_WIDTH_PC34 *
        CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_HEIGHT_PC34];
    CSB_V1_ViewportF0115WallTextOrnamentPc34Trace trace;

    memset(framebuffer, 0xee, sizeof(framebuffer));

    ok &= expect_int("render.reject.null.framebuffer",
                     csb_v1_viewport_f0115_wall_text_ornament_render_pc34(
                         NULL, sizeof(framebuffer), &trace),
                     -1, A_F0124);
    ok &= expect_int("render.reject.short.framebuffer",
                     csb_v1_viewport_f0115_wall_text_ornament_render_pc34(
                         framebuffer, sizeof(framebuffer) - 1, &trace),
                     -1, A_F0124);
    ok &= expect_int("render.reject.null.trace",
                     csb_v1_viewport_f0115_wall_text_ornament_render_pc34(
                         framebuffer, sizeof(framebuffer), NULL),
                     -1, A_F0124);
    ok &= expect_u32("hash.reject.null",
                     csb_v1_viewport_f0115_wall_text_ornament_hash_pc34(
                         NULL, sizeof(framebuffer)),
                     0u, A_DEFS);

    return ok;
}

static int test_evidence_strings(void)
{
    int ok = 1;
    const CSB_V1_ViewportF0115WallTextOrnamentPc34Spec *s =
        csb_v1_viewport_f0115_wall_text_ornament_pc34_spec();
    const char *e =
        csb_v1_viewport_f0115_wall_text_ornament_pc34_source_evidence();

    ok &= expect_contains("spec.anchor.f0115", s ? s->redmcsb_f0115_anchor : NULL,
                          "4547-4581", A_F0115);
    ok &= expect_contains("spec.anchor.f0107", s ? s->redmcsb_f0107_anchor : NULL,
                          "3502-3938", A_F0107);
    ok &= expect_contains("spec.anchor.f0124", s ? s->redmcsb_f0124_anchor : NULL,
                          "7825-7843", A_F0124);
    ok &= expect_contains("spec.anchor.f0128", s ? s->redmcsb_f0128_anchor : NULL,
                          "8318-8486", A_F0128);
    ok &= expect_contains("spec.anchor.defs", s ? s->redmcsb_defs_anchor : NULL,
                          "4139-4153", A_DEFS);
    ok &= expect_contains("spec.anchor.g0208", s ? s->redmcsb_defs_anchor : NULL,
                          "G0208", A_DEFS);
    ok &= expect_contains("spec.anchor.lineage",
                          s ? s->csb_lineage_viewport_anchor : NULL,
                          "1903-1915", A_LINEAGE);
    ok &= expect_contains("evidence.contract", e, "contract-only", A_F0124);
    ok &= expect_contains("evidence.no_dos", e,
                          "no original DOS pixel parity claim", A_F0124);
    ok &= expect_contains("evidence.f0124", e,
                          "F0124_DUNGEONVIEW_DrawSquareD1C", A_F0124);
    ok &= expect_contains("evidence.f0107", e,
                          "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
                          A_F0107);
    ok &= expect_contains("evidence.f0115", e,
                          "first nibble zero is the wall-alcove object path",
                          A_F0115);
    ok &= expect_contains("evidence.f0128", e,
                          "DUNVIEW.C:8318-8486", A_F0128);
    ok &= expect_contains("evidence.c10", e, "DEFS.H:2088 anchors C10", A_DEFS);
    ok &= expect_contains("evidence.c5", e, "DEFS.H:2527 anchors C5_HEIGHT", A_DEFS);
    ok &= expect_contains("evidence.zones", e, "DEFS.H:4139-4153", A_DEFS);
    ok &= expect_contains("evidence.g0208", e, "G0208", A_DEFS);
    ok &= expect_contains("evidence.lineage.open", e,
                          "Viewport.cpp:1192-1209", A_LINEAGE);
    ok &= expect_contains("evidence.lineage.door", e,
                          "1903-1915", A_LINEAGE);
    ok &= expect_contains("evidence.lineage.f0door", e,
                          "1930-1944", A_LINEAGE);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_f0115_wall_text_ornament_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_f0115_wall_text_ornament_pc34_source_evidence());

    ok &= test_spec_contract();
    ok &= test_route_gate();
    ok &= test_render_trace_and_hash();
    ok &= test_pixel_samples();
    ok &= test_render_rejections();
    ok &= test_evidence_strings();
    ok &= expect_int("assertion_count_between_80_and_120",
                     g_assertions >= 80 && g_assertions <= 120, 1, A_F0124);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_f0115_wall_text_ornament_pc34_compat assertions=%d failures=0\n",
               g_assertions);
        return 0;
    }
    printf("FAIL csb_v1_viewport_f0115_wall_text_ornament_pc34_compat assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return 1;
}
