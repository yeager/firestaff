#include "csb/csb_v1_viewport_f0108_footprints_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;

static const char *A_F0108 =
    "ReDMCSB DUNVIEW.C:F0108_DUNGEONVIEW_DrawFloorOrnament:3940-4011";
static const char *A_MASK =
    "ReDMCSB DUNVIEW.C:F0108:3959-3966 and 4007-4008; "
    "DEFS.H:2465/2561; COMPILE.H:1038";
static const char *A_FLIP =
    "ReDMCSB DUNVIEW.C:F0108:3980-3983 and 3998; DEFS.H:2749-2760";
static const char *A_DUNGEON =
    "ReDMCSB DUNGEON.C:F0172:2666-2718 and F0174:2755-2760";
static const char *A_CSB =
    "ReDMCSB DEFS.H:2749-2760 CSB/I34 floor views; DUNVIEW.C:F0108:3998";

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

static int test_contract_metadata(void)
{
    int ok = 1;
    const CSB_V1_ViewportF0108FootprintsPc34Contract *c =
        csb_v1_viewport_f0108_footprints_contract_pc34();

    ok &= expect_int("contract.present", c != NULL, 1, A_F0108);
    ok &= expect_int("contract.contract_only", c ? c->contract_only : 0, 1, A_F0108);
    ok &= expect_int("contract.footprint_mask", c ? c->footprint_mask : 0, 0x8000, A_MASK);
    ok &= expect_int("contract.footprint_index", c ? c->footprint_index : -1, 15, A_MASK);
    ok &= expect_int("contract.footprint_ordinal", c ? c->footprint_ordinal : -1, 16, A_MASK);
    ok &= expect_int("contract.zero_skips", c ? c->ordinal_zero_skips_blit : 0, 1, A_F0108);
    ok &= expect_int("contract.clears_mask", c ? c->clears_mask_before_base_draw : 0, 1, A_MASK);
    ok &= expect_int("contract.mask_only_skips_base", c ? c->mask_only_skips_base_draw : 0, 1, A_MASK);
    ok &= expect_int("contract.recurse_after_base", c ? c->footprints_recurse_after_base : 0, 1, A_MASK);
    ok &= expect_int("contract.preserve_view", c ? c->recursion_preserves_view_floor : 0, 1, A_MASK);
    ok &= expect_int("contract.recursion_stops", c ? c->recursion_stops_after_footprints : 0, 1, A_MASK);
    ok &= expect_int("contract.d3l2_view", c ? c->csb_i34_floor_view_d3l2 : -1, 0, A_CSB);
    ok &= expect_int("contract.d3r2_view", c ? c->csb_i34_floor_view_d3r2 : -1, 1, A_CSB);
    ok &= expect_int("contract.d3c_view", c ? c->csb_i34_floor_view_d3c : -1, 3, A_FLIP);
    ok &= expect_int("contract.d2c_view", c ? c->csb_i34_floor_view_d2c : -1, 6, A_FLIP);
    ok &= expect_int("contract.d1c_view", c ? c->csb_i34_floor_view_d1c : -1, 9, A_FLIP);
    ok &= expect_int("contract.zone_base", c ? c->zone_base : -1, 1500, A_CSB);
    ok &= expect_int("contract.coord_stride", c ? c->coordinate_set_stride : -1, 11, A_CSB);
    ok &= expect_int("contract.coord_set", c ? c->coordinate_set_index : -1, 0, A_CSB);
    ok &= expect_int("contract.transparent", c ? c->transparent_color : -1, 10, A_F0108);
    ok &= expect_int("contract.flip_mask", c ? c->flip_horizontal_mask : -1, 1, A_FLIP);

    return ok;
}

static int test_zero_and_plain_base(void)
{
    int ok = 1;
    CSB_V1_ViewportF0108FootprintsPc34Plan plan;

    /* ReDMCSB: DUNVIEW.C F0108 line 3959 skips zero ordinals entirely. */
    ok &= expect_int("zero.plan",
                     csb_v1_viewport_f0108_footprints_plan_pc34(0, 0, 0, &plan),
                     0, A_F0108);
    ok &= expect_int("zero.draw_count", plan.draw_count, 0, A_F0108);
    ok &= expect_int("zero.base", plan.base_drawn, 0, A_F0108);
    ok &= expect_int("zero.footprints", plan.footprints_drawn, 0, A_F0108);
    ok &= expect_int("zero.zone", plan.base_zone, -1, A_F0108);

    /* ReDMCSB: DUNVIEW.C F0108 lines 3965-3966 pre-decrement ordinal into
     * a map floor-ornament index, then line 3998 uses C1500 + view floor. */
    ok &= expect_int("plain.plan",
                     csb_v1_viewport_f0108_footprints_plan_pc34(3, 0, 0, &plan),
                     0, A_F0108);
    ok &= expect_int("plain.draw_count", plan.draw_count, 1, A_F0108);
    ok &= expect_int("plain.base", plan.base_drawn, 1, A_F0108);
    ok &= expect_int("plain.base_index", plan.base_ornament_index, 2, A_F0108);
    ok &= expect_int("plain.footprints", plan.footprints_drawn, 0, A_MASK);
    ok &= expect_int("plain.zone", plan.base_zone, 1500, A_CSB);
    ok &= expect_int("plain.flip", plan.base_flip, 0, A_FLIP);
    ok &= expect_int("plain.transparent", plan.base_transparent_color, 10, A_F0108);

    return ok;
}

static int test_masked_base_and_mask_only(void)
{
    int ok = 1;
    CSB_V1_ViewportF0108FootprintsPc34Plan plan;

    /* ReDMCSB: DUNVIEW.C F0108 lines 3960-3966 draw the base ordinal after
     * clearing MASK0x8000, then lines 4007-4008 recurse for footprints. */
    ok &= expect_int("masked.plan",
                     csb_v1_viewport_f0108_footprints_plan_pc34(
                         (uint16_t)(0x8000u | 3u), 0, 0, &plan),
                     0, A_MASK);
    ok &= expect_int("masked.draw_count", plan.draw_count, 2, A_MASK);
    ok &= expect_int("masked.cleared", plan.cleared_base_ordinal, 3, A_MASK);
    ok &= expect_int("masked.base", plan.base_drawn, 1, A_MASK);
    ok &= expect_int("masked.base_index", plan.base_ornament_index, 2, A_MASK);
    ok &= expect_int("masked.footprints", plan.footprints_drawn, 1, A_MASK);
    ok &= expect_int("masked.footprint_index", plan.footprints_ornament_index, 15, A_MASK);
    ok &= expect_int("masked.recursive_ordinal", plan.recursive_ordinal, 16, A_MASK);
    ok &= expect_int("masked.recursive_view", plan.recursive_view_floor, 0, A_MASK);
    ok &= expect_int("masked.base_zone", plan.base_zone, 1500, A_CSB);
    ok &= expect_int("masked.footprints_zone", plan.footprints_zone, 1500, A_CSB);
    ok &= expect_int("masked.recursion_stops", plan.recursion_stops, 1, A_MASK);

    /* ReDMCSB: DUNVIEW.C F0108 lines 3961-3962 jump over the base blit
     * when only MASK0x8000 remains, but still hit T0108005 footprints. */
    ok &= expect_int("mask_only.plan",
                     csb_v1_viewport_f0108_footprints_plan_pc34(0x8000u, 1, 0, &plan),
                     0, A_MASK);
    ok &= expect_int("mask_only.draw_count", plan.draw_count, 1, A_MASK);
    ok &= expect_int("mask_only.cleared", plan.cleared_base_ordinal, 0, A_MASK);
    ok &= expect_int("mask_only.base", plan.base_drawn, 0, A_MASK);
    ok &= expect_int("mask_only.base_index", plan.base_ornament_index, -1, A_MASK);
    ok &= expect_int("mask_only.footprints", plan.footprints_drawn, 1, A_MASK);
    ok &= expect_int("mask_only.recursive_ordinal", plan.recursive_ordinal, 16, A_MASK);
    ok &= expect_int("mask_only.recursive_view", plan.recursive_view_floor, 1, A_MASK);
    ok &= expect_int("mask_only.footprints_zone", plan.footprints_zone, 1501, A_CSB);

    return ok;
}

static int test_flip_and_csb_floor_views(void)
{
    int ok = 1;
    CSB_V1_ViewportF0108FootprintsPc34Plan plan;

    /* ReDMCSB: DUNVIEW.C F0108 lines 3980-3983 flip C01_VIEW_FLOOR_D3R2
     * and other right-side floor views through MASK0x0001. */
    ok &= expect_int("d3r2.mask_only.plan",
                     csb_v1_viewport_f0108_footprints_plan_pc34(0x8000u, 1, 0, &plan),
                     0, A_FLIP);
    ok &= expect_int("d3r2.footprint_flip", plan.footprints_flip, 1, A_FLIP);
    ok &= expect_int("d3r2.footprint_zone", plan.footprints_zone, 1501, A_CSB);
    ok &= expect_int("d3r2.base_flip_absent", plan.base_flip, 0, A_FLIP);

    ok &= expect_int("d1c.flipped.plan",
                     csb_v1_viewport_f0108_footprints_plan_pc34(0x8000u, 9, 1, &plan),
                     0, A_FLIP);
    ok &= expect_int("d1c.footprint_flip", plan.footprints_flip, 1, A_FLIP);
    ok &= expect_int("d1c.footprint_zone", plan.footprints_zone, 1509, A_CSB);
    ok &= expect_int("d1c.recursive_view", plan.recursive_view_floor, 9, A_MASK);

    ok &= expect_int("d1c.not_flipped.plan",
                     csb_v1_viewport_f0108_footprints_plan_pc34(0x8000u, 9, 0, &plan),
                     0, A_FLIP);
    ok &= expect_int("d1c.not_flipped.footprint_flip", plan.footprints_flip, 0, A_FLIP);
    ok &= expect_int("d2c.flipped.plan",
                     csb_v1_viewport_f0108_footprints_plan_pc34(0x8000u, 6, 1, &plan),
                     0, A_FLIP);
    ok &= expect_int("d2c.footprint_flip", plan.footprints_flip, 1, A_FLIP);
    ok &= expect_int("d3c.flipped.plan",
                     csb_v1_viewport_f0108_footprints_plan_pc34(0x8000u, 3, 1, &plan),
                     0, A_FLIP);
    ok &= expect_int("d3c.footprint_flip", plan.footprints_flip, 1, A_FLIP);

    return ok;
}

static int test_evidence_and_rejects(void)
{
    int ok = 1;
    CSB_V1_ViewportF0108FootprintsPc34Plan plan;
    const CSB_V1_ViewportF0108FootprintsPc34Contract *c =
        csb_v1_viewport_f0108_footprints_contract_pc34();
    const char *e = csb_v1_viewport_f0108_footprints_source_evidence_pc34();

    ok &= expect_contains("evidence.contract_only", e, "Contract-only synthetic gate", A_F0108);
    ok &= expect_contains("evidence.no_pixel_claim", e, "no real-asset pixel parity", A_F0108);
    ok &= expect_contains("evidence.f0108.lines", e, "3940-4011", A_F0108);
    ok &= expect_contains("evidence.mask", e, "MASK0x8000_FOOTPRINTS", A_MASK);
    ok &= expect_contains("evidence.recursive", e, "4007-4008", A_MASK);
    ok &= expect_contains("evidence.dungeon", e, "2666-2718", A_DUNGEON);
    ok &= expect_contains("evidence.current_map", e, "2755-2760", A_DUNGEON);
    ok &= expect_contains("evidence.csb_i34", e, "C00/C01 D3L2/D3R2", A_CSB);
    ok &= expect_contains("evidence.firestaff_path", e, "src/csb/csb_v1_dungeon_loader", A_CSB);
    ok &= expect_contains("contract.f0108_anchor", c ? c->redmcsb_f0108_anchor : "",
                          "F0108_DUNGEONVIEW_DrawFloorOrnament", A_F0108);
    ok &= expect_contains("contract.dungeon_anchor", c ? c->redmcsb_dungeon_anchor : "",
                          "F0172_DUNGEON_SetSquareAspect", A_DUNGEON);
    ok &= expect_contains("contract.defs_anchor", c ? c->redmcsb_defs_anchor : "",
                          "C15_FLOOR_ORNAMENT_FOOTPRINTS", A_MASK);
    ok &= expect_contains("contract.csb_anchor", c ? c->csb_specific_anchor : "",
                          "CSB/I34 D3L2/D3R2", A_CSB);

    ok &= expect_int("reject.null_out",
                     csb_v1_viewport_f0108_footprints_plan_pc34(1, 0, 0, NULL),
                     -1, A_F0108);
    ok &= expect_int("reject.bad_view",
                     csb_v1_viewport_f0108_footprints_plan_pc34(1, -1, 0, &plan),
                     -1, A_F0108);
    ok &= expect_int("result.evidence.plan",
                     csb_v1_viewport_f0108_footprints_plan_pc34(0x8001u, 0, 0, &plan),
                     0, A_MASK);
    ok &= expect_contains("result.evidence", plan.source_evidence, "4007-4008", A_MASK);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_f0108_footprints_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_f0108_footprints_source_evidence_pc34());

    ok &= test_contract_metadata();
    ok &= test_zero_and_plain_base();
    ok &= test_masked_base_and_mask_only();
    ok &= test_flip_and_csb_floor_views();
    ok &= test_evidence_and_rejects();

    printf("assertions=%d\n", g_assertions);
    ok &= expect_int("assertion_count_at_least_70", g_assertions >= 70, 1, A_F0108);

    if (ok) {
        printf("PASS csb_v1_viewport_f0108_footprints_pc34_compat assertions=%d\n",
               g_assertions);
        return 0;
    }

    printf("FAIL csb_v1_viewport_f0108_footprints_pc34_compat assertions=%d\n",
           g_assertions);
    return 1;
}
