#include "csb_v1_viewport_d3l2_f0115_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    const int got = haystack && needle && strstr(haystack, needle) != NULL;
    return expect_int(label, got, 1, anchor);
}

static int test_route_identity_and_non_overlap(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec =
        csb_v1_viewport_d3l2_f0115_projectile_route_spec_pc34();

    /* ReDMCSB: DEFS.H lines 2610-2611 define C14/C15 D3L2/D3R2.
     * DUNVIEW.C lines 371-373 map D3L2 to depth 3, lane -2, G2028 row 3;
     * D3R2 is row 4 and is intentionally outside this D3L2-only gate. */
    ok &= expect_int("spec.present", spec != NULL, 1,
                     "ReDMCSB DUNVIEW.C:371-373");
    ok &= expect_int("view_square.c14_d3l2", spec ? spec->view_square : -1, 14,
                     "ReDMCSB DEFS.H:2610 C14_VIEW_SQUARE_D3L2");
    ok &= expect_int("view_depth", spec ? spec->view_depth : -1, 3,
                     "ReDMCSB DUNVIEW.C:372 G2027[14]");
    ok &= expect_int("view_lane", spec ? spec->view_lane : -1, 254,
                     "ReDMCSB DUNVIEW.C:371 G2026[14]");
    ok &= expect_int("d3l2.g2028_row", spec ? spec->projectile_g2028_row : -1, 3,
                     "ReDMCSB DUNVIEW.C:373 G2028[14]");
    ok &= expect_int("d3r2.excluded_row", spec ? spec->excluded_d3r2_g2028_row : -1, 4,
                     "ReDMCSB DUNVIEW.C:373 G2028[15]");
    ok &= expect_int("source_locked_contract_only",
                     spec ? spec->source_locked_contract_only : -1, 1,
                     "no real-asset bitmap parity claimed");
    ok &= expect_int("not_wall_f0121_f0104",
                     spec ? spec->wall_f0121_f0104_route : -1, 0,
                     "non-overlap with D3L2 wall gate");
    ok &= expect_int("not_teleporter_f0113",
                     spec ? spec->teleporter_f0113_route : -1, 0,
                     "non-overlap with D3L2 field gate");

    return ok;
}

static int test_projectile_filter_and_zone_math(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec =
        csb_v1_viewport_d3l2_f0115_projectile_route_spec_pc34();

    /* ReDMCSB: F0115 lines 5668-5683 suppress depth-3 front cells, restart
     * the thing list, require C14_THING_TYPE_PROJECTILE plus cell match, and
     * compute C2900_ZONE_ + G2028 row*4 + ViewCell for the remaining cells. */
    ok &= expect_int("zone_base", spec ? spec->projectile_zone_base : -1, 2900,
                     "ReDMCSB DEFS.H:4230 C2900_ZONE_");
    ok &= expect_int("zone_stride", spec ? spec->projectile_zone_cell_stride : -1, 4,
                     "ReDMCSB DUNVIEW.C:5683 row*4+ViewCell");
    ok &= expect_int("restarts_thing_list", spec ? spec->restarts_thing_list : -1, 1,
                     "ReDMCSB DUNVIEW.C:5679");
    ok &= expect_int("requires_c14", spec ? spec->requires_projectile_type_c14 : -1, 1,
                     "ReDMCSB DUNVIEW.C:5681 C14_THING_TYPE_PROJECTILE");
    ok &= expect_int("requires_cell_match", spec ? spec->requires_cell_match : -1, 1,
                     "ReDMCSB DUNVIEW.C:5681 M011_CELL == L0139");
    ok &= expect_int("depth3_front_cell0",
                     csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(spec, 0), -1,
                     "ReDMCSB DUNVIEW.C:5672");
    ok &= expect_int("depth3_front_cell1",
                     csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(spec, 1), -1,
                     "ReDMCSB DUNVIEW.C:5672");
    ok &= expect_int("d3l2.back_left_zone",
                     csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(spec, 2), 2914,
                     "ReDMCSB DUNVIEW.C:5683 2900 + 3*4 + 2");
    ok &= expect_int("d3l2.back_right_zone",
                     csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(spec, 3), 2915,
                     "ReDMCSB DUNVIEW.C:5683 2900 + 3*4 + 3");
    ok &= expect_int("not_d3r2_row4_zone",
                     csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(spec, 3) != 2919, 1,
                     "D3R2 row 4 would be 2900 + 4*4 + 3");
    ok &= expect_int("bad_cell",
                     csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(spec, 4), -1,
                     "four F0115 view cells");
    ok &= expect_int("accepts_projectile_cell2",
                     csb_v1_viewport_d3l2_f0115_projectile_accepts_thing_pc34(
                         spec, 14, 2, 2), 1,
                     "ReDMCSB DUNVIEW.C:5681 type and cell match");
    ok &= expect_int("rejects_non_projectile",
                     csb_v1_viewport_d3l2_f0115_projectile_accepts_thing_pc34(
                         spec, 5, 2, 2), 0,
                     "ReDMCSB DUNVIEW.C:5681 C14_THING_TYPE_PROJECTILE");
    ok &= expect_int("rejects_cell_mismatch",
                     csb_v1_viewport_d3l2_f0115_projectile_accepts_thing_pc34(
                         spec, 14, 3, 2), 0,
                     "ReDMCSB DUNVIEW.C:5681 M011_CELL == L0139");
    ok &= expect_int("rejects_suppressed_front_cell",
                     csb_v1_viewport_d3l2_f0115_projectile_accepts_thing_pc34(
                         spec, 14, 1, 1), 0,
                     "ReDMCSB DUNVIEW.C:5672");
    ok &= expect_int("null_zone",
                     csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(NULL, 2), -1,
                     "route helper rejects unresolved spec");

    return ok;
}

static int test_scale_and_blit_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec =
        csb_v1_viewport_d3l2_f0115_projectile_route_spec_pc34();
    const uint8_t source[6] = { 1, 10, 2, 3, 10, 4 };
    uint8_t destination[6] = { 77, 77, 77, 77, 77, 77 };

    /* ReDMCSB: F0115 lines 5710-5722 use (ViewDepth << 1) -
     * (ViewCell >> 1), then kinetic scaling.  Lines 5881-5882 preserve
     * dynamic flip flags and C10 transparency through F0791. */
    ok &= expect_int("scale_index.cell2",
                     csb_v1_viewport_d3l2_f0115_projectile_scale_index_pc34(spec, 2), 5,
                     "ReDMCSB DUNVIEW.C:5715");
    ok &= expect_int("scale_index.cell3",
                     csb_v1_viewport_d3l2_f0115_projectile_scale_index_pc34(spec, 3), 5,
                     "ReDMCSB DUNVIEW.C:5715");
    ok &= expect_int("scale_index.front_suppressed",
                     csb_v1_viewport_d3l2_f0115_projectile_scale_index_pc34(spec, 1), -1,
                     "ReDMCSB DUNVIEW.C:5672 before scaling");
    ok &= expect_int("kinetic_minimum_scale",
                     csb_v1_viewport_d3l2_f0115_projectile_apply_kinetic_scale_pc34(
                         spec, 64, 0, 1), 24,
                     "ReDMCSB DUNVIEW.C:5720 max(96, energy+1)");
    ok &= expect_int("kinetic_floor",
                     csb_v1_viewport_d3l2_f0115_projectile_apply_kinetic_scale_pc34(
                         spec, 3, 0, 1), 2,
                     "ReDMCSB DUNVIEW.C:5720 max(2, scaled)");
    ok &= expect_int("kinetic_full_scale",
                     csb_v1_viewport_d3l2_f0115_projectile_apply_kinetic_scale_pc34(
                         spec, 64, 255, 1), 64,
                     "ReDMCSB DUNVIEW.C:5720 energy 255");
    ok &= expect_int("kinetic_not_scaled",
                     csb_v1_viewport_d3l2_f0115_projectile_apply_kinetic_scale_pc34(
                         spec, 64, 0, 0), 64,
                     "ReDMCSB DUNVIEW.C:5711 GraphicInfo scale flag");
    ok &= expect_int("derived_bitmap_none",
                     spec ? spec->projectile_derived_bitmap_none : 0, -1,
                     "ReDMCSB DUNVIEW.C:5885 CM1_DERIVED_BITMAP_NONE");
    ok &= expect_int("uses_f0791",
                     spec ? spec->projectile_uses_f0791_blit : -1, 1,
                     "ReDMCSB DUNVIEW.C:5881-5882");
    ok &= expect_int("dynamic_flip_preserved",
                     spec ? spec->dynamic_flip_flags_preserved : -1, 1,
                     "ReDMCSB DUNVIEW.C:5882 L0143_B_FlipHorizontal");
    ok &= expect_int("transparent_color",
                     spec ? spec->transparent_color : -1, 10,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("blit.flip_copied",
                     csb_v1_viewport_d3l2_f0115_projectile_apply_c10_blit_pc34(
                         spec, source, 3, destination, 3, 3, 2, 1), 4,
                     "ReDMCSB DUNVIEW.C:5881-5882 F0791 C10");
    ok &= expect_int("blit.flip_pixel0", destination[0], 2,
                     "dynamic flip copies source x2 to destination x0");
    ok &= expect_int("blit.flip_transparent1", destination[1], 77,
                     "ReDMCSB DEFS.H:2088 C10 transparent");
    ok &= expect_int("blit.flip_pixel2", destination[2], 1,
                     "dynamic flip copies source x0 to destination x2");
    ok &= expect_int("blit.flip_pixel3", destination[3], 4,
                     "dynamic flip copies source x2 to row 1 x0");
    ok &= expect_int("blit.flip_transparent4", destination[4], 77,
                     "ReDMCSB DEFS.H:2088 C10 transparent");
    ok &= expect_int("blit.flip_pixel5", destination[5], 3,
                     "dynamic flip copies source x0 to row 1 x2");
    ok &= expect_int("blit.reject_null",
                     csb_v1_viewport_d3l2_f0115_projectile_apply_c10_blit_pc34(
                         NULL, source, 3, destination, 3, 3, 2, 0), -1,
                     "route helper rejects unresolved spec");

    return ok;
}

static int test_source_evidence(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d3l2_f0115_projectile_source_evidence_pc34();

    ok &= expect_contains("evidence.f0115_order", e, "DUNVIEW.C:4547-4581",
                          "ReDMCSB F0115 order");
    ok &= expect_contains("evidence.g2028_row3", e, "G2028 row 3",
                          "ReDMCSB DUNVIEW.C:371-373");
    ok &= expect_contains("evidence.d3r2_excluded", e, "row 4",
                          "ReDMCSB DUNVIEW.C:373 G2028[15]");
    ok &= expect_contains("evidence.projectile_gate", e, "DUNVIEW.C:5668-5683",
                          "ReDMCSB F0115 projectile gate");
    ok &= expect_contains("evidence.scale", e, "DUNVIEW.C:5710-5722",
                          "ReDMCSB F0115 projectile scaling");
    ok &= expect_contains("evidence.blit", e, "DUNVIEW.C:5881-5882",
                          "ReDMCSB F0791 projectile blit");
    ok &= expect_contains("evidence.c10", e, "C10_COLOR_FLESH",
                          "ReDMCSB DEFS.H:2088");
    ok &= expect_contains("evidence.no_wall", e, "D3L2 wall F0121/F0104",
                          "non-overlap with wall gate");
    ok &= expect_contains("evidence.no_teleporter", e, "D3L2 teleporter F0113",
                          "non-overlap with teleporter gate");
    ok &= expect_contains("evidence.no_asset_claim", e, "no real-asset bitmap parity",
                          "bounded source-lock gate");

    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_route_identity_and_non_overlap();
    ok &= test_projectile_filter_and_zone_math();
    ok &= test_scale_and_blit_contract();
    ok &= test_source_evidence();

    printf("%s csb_v1_viewport_d3l2_f0115_projectile_pc34_compat assertions=%d\n",
           ok ? "PASS" : "FAIL", g_assertions);
    return ok ? 0 : 1;
}
