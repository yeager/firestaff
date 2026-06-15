#include "csb_v1_viewport_d3c_f0107_f0108_first_backdrop_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static const char *A_ORDER =
    "ReDMCSB DUNVIEW.C F0098:2962-3002; F0128:8337-8339,8478-8508";
static const char *A_F0107 =
    "ReDMCSB DUNVIEW.C F0107:3502-3938; D3C call F0118:6716";
static const char *A_F0108 =
    "ReDMCSB DUNVIEW.C F0108:3940-4011; D3C calls F0118:6722,6814";
static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C F0115:4547-4581";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2588-2598,2701,2749-2760,4056-4057,4222-4223";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1192-1209,1903-1915,6507-6548,6800-6840";

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

static int expect_contains(
    const char *label,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    return expect_int(label, haystack && needle &&
        strstr(haystack, needle) != NULL, 1, anchor);
}

static int pixel_at(const unsigned char *viewport, int x, int y)
{
    return viewport[(y *
        CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_WIDTH_PC34) + x];
}

static int test_contract(void)
{
    int ok = 1;
    const CSB_V1_ViewportD3cF0107F0108FirstBackdropPc34Contract *c =
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_contract_pc34();
    const char *e =
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_source_evidence_pc34();

    ok &= expect_int("contract.non_null", c != NULL, 1, A_ORDER);
    ok &= expect_int("contract.contract_only", c ? c->contract_only : 0, 1, A_ORDER);
    ok &= expect_int("contract.no_game_data", c ? c->no_game_data_dependency : 0, 1, A_ORDER);
    ok &= expect_int("contract.viewport.width", c ? c->viewport_width : -1, 224, A_DEFS);
    ok &= expect_int("contract.viewport.height", c ? c->viewport_height : -1, 136, A_DEFS);
    ok &= expect_int("contract.view_square.d3c", c ? c->view_square_d3c : -1, 11, A_DEFS);
    ok &= expect_int("contract.view_wall.d3c.front", c ? c->view_wall_d3c_front : -1, 5, A_DEFS);
    ok &= expect_int("contract.view_floor.d3c", c ? c->view_floor_d3c : -1, 3, A_DEFS);
    ok &= expect_int("contract.wall.zone.base", c ? c->wall_ornament_zone_base : -1, 1004, A_DEFS);
    ok &= expect_int("contract.wall.zone.stride", c ? c->wall_ornament_coordinate_set_stride : -1, 15, A_F0107);
    ok &= expect_int("contract.floor.zone.base", c ? c->floor_ornament_zone_base : -1, 1500, A_DEFS);
    ok &= expect_int("contract.floor.zone.stride", c ? c->floor_ornament_coordinate_set_stride : -1, 11, A_F0108);
    ok &= expect_int("contract.transparent", c ? c->transparent_color : -1, 10, A_DEFS);
    ok &= expect_int("contract.wall.ordinal", c ? c->wall_ornament_ordinal : -1, 3, A_F0107);
    ok &= expect_int("contract.wall.index", c ? c->wall_ornament_index : -1, 2, A_F0107);
    ok &= expect_int("contract.floor.ordinal", c ? c->floor_ornament_ordinal : -1, 5, A_F0108);
    ok &= expect_int("contract.floor.index", c ? c->floor_ornament_index : -1, 4, A_F0108);
    ok &= expect_int("contract.backdrop.room.slot", c ? c->first_backdrop_room_slot : -1, 0, A_LINEAGE);
    ok &= expect_int("contract.backdrop.before.routes", c ? c->first_backdrop_is_before_cell_routes : 0, 1, A_ORDER);
    ok &= expect_int("contract.f0107.before.f0108", c ? c->f0107_before_f0108 : 0, 1, A_ORDER);
    ok &= expect_int("contract.f0108.mask.preserves", c ? c->f0108_transparent_mask_preserves_destination : 0, 1, A_F0108);
    ok &= expect_int("contract.d3c.x1", c ? c->d3c_window.x1 : -1, 74, A_ORDER);
    ok &= expect_int("contract.d3c.y1", c ? c->d3c_window.y1 : -1, 25, A_ORDER);
    ok &= expect_int("contract.d3c.x2", c ? c->d3c_window.x2 : -1, 149, A_ORDER);
    ok &= expect_int("contract.d3c.y2", c ? c->d3c_window.y2 : -1, 75, A_ORDER);
    ok &= expect_int("contract.wall.x1", c ? c->wall_ornament_window.x1 : -1, 88, A_F0107);
    ok &= expect_int("contract.wall.y1", c ? c->wall_ornament_window.y1 : -1, 35, A_F0107);
    ok &= expect_int("contract.wall.x2", c ? c->wall_ornament_window.x2 : -1, 135, A_F0107);
    ok &= expect_int("contract.wall.y2", c ? c->wall_ornament_window.y2 : -1, 64, A_F0107);
    ok &= expect_int("contract.floor.x1", c ? c->floor_ornament_window.x1 : -1, 96, A_F0108);
    ok &= expect_int("contract.floor.y1", c ? c->floor_ornament_window.y1 : -1, 57, A_F0108);
    ok &= expect_int("contract.floor.x2", c ? c->floor_ornament_window.x2 : -1, 127, A_F0108);
    ok &= expect_int("contract.floor.y2", c ? c->floor_ornament_window.y2 : -1, 80, A_F0108);
    ok &= expect_contains("anchor.f0097", c ? c->redmcsb_f0097_anchor : NULL, "DRAWVIEW.C:709-722", A_ORDER);
    ok &= expect_contains("anchor.f0107", c ? c->redmcsb_f0107_anchor : NULL, "3502-3938", A_F0107);
    ok &= expect_contains("anchor.f0108", c ? c->redmcsb_f0108_anchor : NULL, "3940-4011", A_F0108);
    ok &= expect_contains("anchor.f0118", c ? c->redmcsb_f0118_anchor : NULL, "6642-6763", A_F0108);
    ok &= expect_contains("anchor.f0115", c ? c->redmcsb_f0115_anchor : NULL, "4547-4581", A_F0115);
    ok &= expect_contains("anchor.f0127", c ? c->redmcsb_f0127_anchor : NULL, "8294", A_ORDER);
    ok &= expect_contains("anchor.f0128", c ? c->redmcsb_f0128_anchor : NULL, "8478-8508", A_ORDER);
    ok &= expect_contains("anchor.defs", c ? c->redmcsb_defs_anchor : NULL, "2749-2760", A_DEFS);
    ok &= expect_contains("anchor.lineage", c ? c->csb_lineage_anchor : NULL, "6507-6548", A_LINEAGE);
    ok &= expect_contains("evidence.f0098", e, "F0098:2962-3002", A_ORDER);
    ok &= expect_contains("evidence.f0107", e, "F0107:3502-3938", A_F0107);
    ok &= expect_contains("evidence.f0108", e, "F0108:3940-4011", A_F0108);
    ok &= expect_contains("evidence.f0118", e, "F0118:6642-6763", A_F0108);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1192-1209", A_LINEAGE);

    return ok;
}

static int test_plan(void)
{
    int ok = 1;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropPlanPc34 plan;

    ok &= expect_int("plan.call",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_plan_pc34(3, 5, 0, &plan),
        0, A_ORDER);
    ok &= expect_int("plan.ok", plan.ok, 1, A_ORDER);
    ok &= expect_int("plan.draw.count", plan.draw_step_count, 3, A_ORDER);
    ok &= expect_int("plan.step0.backdrop", plan.draw_steps[0],
        CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_BACKDROP, A_ORDER);
    ok &= expect_int("plan.step1.f0107", plan.draw_steps[1],
        CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_F0107_WALL_ORNAMENT, A_F0107);
    ok &= expect_int("plan.step2.f0108", plan.draw_steps[2],
        CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_F0108_FLOOR_ORNAMENT, A_F0108);
    ok &= expect_int("plan.backdrop.before.f0107", plan.draw_steps[0] < plan.draw_steps[1], 1, A_ORDER);
    ok &= expect_int("plan.f0107.before.f0108", plan.draw_steps[1] < plan.draw_steps[2], 1, A_ORDER);
    ok &= expect_int("plan.wall.ordinal", plan.wall_ornament_ordinal, 3, A_F0107);
    ok &= expect_int("plan.wall.index", plan.wall_ornament_index, 2, A_F0107);
    ok &= expect_int("plan.floor.ordinal", plan.floor_ornament_ordinal, 5, A_F0108);
    ok &= expect_int("plan.floor.index", plan.floor_ornament_index, 4, A_F0108);
    ok &= expect_int("plan.backdrop.room", plan.first_backdrop_room_slot, 0, A_LINEAGE);
    ok &= expect_int("plan.wall.zone", plan.wall_ornament_zone, 1009, A_F0107);
    ok &= expect_int("plan.floor.zone", plan.floor_ornament_zone, 1503, A_F0108);
    ok &= expect_int("plan.backdrop.color", plan.backdrop_color, 31, A_ORDER);
    ok &= expect_int("plan.wall.color", plan.wall_ornament_color, 47, A_F0107);
    ok &= expect_int("plan.floor.color", plan.floor_ornament_color, 63, A_F0108);
    ok &= expect_int("plan.mask.color", plan.masked_floor_source_color, 10, A_F0108);
    ok &= expect_int("plan.distinct.backdrop.wall", plan.backdrop_color != plan.wall_ornament_color, 1, A_ORDER);
    ok &= expect_int("plan.distinct.wall.floor", plan.wall_ornament_color != plan.floor_ornament_color, 1, A_ORDER);
    ok &= expect_int("plan.distinct.backdrop.floor", plan.backdrop_color != plan.floor_ornament_color, 1, A_ORDER);
    ok &= expect_int("plan.distinct.flag", plan.distinct_layer_colors, 1, A_ORDER);
    ok &= expect_int("plan.f0107.before.flag", plan.f0107_before_f0108, 1, A_ORDER);
    ok &= expect_int("plan.mask.preserves.flag", plan.f0108_mask_preserves_f0107, 1, A_F0108);
    ok &= expect_int("plan.backdrop.sample.x", plan.backdrop_only_x, 80, A_ORDER);
    ok &= expect_int("plan.backdrop.sample.y", plan.backdrop_only_y, 30, A_ORDER);
    ok &= expect_int("plan.wall.sample.x", plan.wall_only_x, 90, A_F0107);
    ok &= expect_int("plan.wall.sample.y", plan.wall_only_y, 40, A_F0107);
    ok &= expect_int("plan.floor.sample.x", plan.floor_opaque_x, 110, A_F0108);
    ok &= expect_int("plan.floor.sample.y", plan.floor_opaque_y, 70, A_F0108);
    ok &= expect_int("plan.masked.sample.x", plan.overlap_masked_x, 104, A_F0108);
    ok &= expect_int("plan.masked.sample.y", plan.overlap_masked_y, 60, A_F0108);
    ok &= expect_int("plan.reject.null",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_plan_pc34(3, 5, 0, NULL),
        -1, A_ORDER);
    ok &= expect_int("plan.reject.wall",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_plan_pc34(2, 5, 0, &plan),
        1, A_F0107);
    ok &= expect_int("plan.reject.floor",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_plan_pc34(3, 4, 0, &plan),
        1, A_F0108);
    ok &= expect_int("plan.reject.backdrop",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_plan_pc34(3, 5, 1, &plan),
        1, A_LINEAGE);

    return ok;
}

static int test_run_and_pixels(void)
{
    int ok = 1;
    unsigned char viewport[
        CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_WIDTH_PC34 *
        CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_HEIGHT_PC34];
    CSB_V1_ViewportD3cF0107F0108FirstBackdropPlanPc34 plan;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropResultPc34 result;
    const size_t viewport_len = sizeof(viewport);
    const int backdrop_samples[][2] = {
        {74, 25}, {80, 30}, {149, 25}, {74, 75}, {149, 75},
        {86, 34}, {136, 64}, {95, 75}
    };
    const int wall_samples[][2] = {
        {88, 35}, {90, 40}, {95, 56}, {135, 35}, {135, 64},
        {104, 60}, {105, 60}, {104, 61}, {105, 61}
    };
    const int floor_samples[][2] = {
        {96, 57}, {127, 57}, {96, 80}, {127, 80}, {110, 70},
        {100, 60}, {120, 64}, {110, 75}
    };
    const int outside_samples[][2] = {
        {0, 0}, {73, 25}, {150, 75}, {223, 135}
    };

    (void)csb_v1_viewport_d3c_f0107_f0108_first_backdrop_plan_pc34(3, 5, 0, &plan);
    ok &= expect_int("run.call",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_run_pc34(
            &plan, viewport, viewport_len, &result),
        0, A_ORDER);
    ok &= expect_int("run.ok", result.ok, 1, A_ORDER);
    ok &= expect_int("run.draw.count", result.draw_step_count, 3, A_ORDER);
    ok &= expect_int("run.step0", result.draw_steps[0],
        CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_BACKDROP, A_ORDER);
    ok &= expect_int("run.step1", result.draw_steps[1],
        CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_F0107_WALL_ORNAMENT, A_F0107);
    ok &= expect_int("run.step2", result.draw_steps[2],
        CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_F0108_FLOOR_ORNAMENT, A_F0108);
    ok &= expect_int("run.backdrop.pixels", result.backdrop_pixels, 76 * 51, A_ORDER);
    ok &= expect_int("run.wall.pixels", result.wall_ornament_pixels, 48 * 30, A_F0107);
    ok &= expect_int("run.floor.opaque", result.floor_ornament_opaque_pixels, (32 * 24) - 4, A_F0108);
    ok &= expect_int("run.floor.masked", result.floor_ornament_masked_pixels, 4, A_F0108);
    ok &= expect_int("run.floor.total",
        result.floor_ornament_opaque_pixels + result.floor_ornament_masked_pixels,
        32 * 24, A_F0108);
    ok &= expect_int("run.overlap.pixels", result.overlap_pixels, 32 * 8, A_F0108);
    ok &= expect_int("run.final.backdrop", result.final_backdrop_only_pixel, 31, A_ORDER);
    ok &= expect_int("run.final.wall", result.final_wall_only_pixel, 47, A_F0107);
    ok &= expect_int("run.final.floor", result.final_floor_opaque_pixel, 63, A_F0108);
    ok &= expect_int("run.final.masked.overlap", result.final_overlap_masked_pixel, 47, A_F0108);
    ok &= expect_int("run.mask.before", result.pixel_before_f0108_at_masked_overlap, 47, A_F0107);
    ok &= expect_int("run.mask.after", result.pixel_after_f0108_at_masked_overlap, 47, A_F0108);
    ok &= expect_int("run.floor.before", result.pixel_before_f0108_at_opaque_floor, 31, A_ORDER);
    ok &= expect_int("run.floor.after", result.pixel_after_f0108_at_opaque_floor, 63, A_F0108);
    ok &= expect_int("run.mask.did.not.erase", result.f0108_mask_did_not_erase_f0107, 1, A_F0108);
    ok &= expect_int("run.floor.overwrote", result.f0108_opaque_pixel_overwrote_destination, 1, A_F0108);
    ok &= expect_contains("run.evidence.f0107", result.source_evidence, "F0107", A_F0107);
    ok &= expect_contains("run.evidence.f0108", result.source_evidence, "F0108", A_F0108);

    for (size_t i = 0; i < sizeof(backdrop_samples) / sizeof(backdrop_samples[0]); ++i) {
        char label[64];
        snprintf(label, sizeof(label), "pixel.backdrop.%zu", i);
        ok &= expect_int(label,
            pixel_at(viewport, backdrop_samples[i][0], backdrop_samples[i][1]),
            31, A_ORDER);
    }
    for (size_t i = 0; i < sizeof(wall_samples) / sizeof(wall_samples[0]); ++i) {
        char label[64];
        snprintf(label, sizeof(label), "pixel.wall.%zu", i);
        ok &= expect_int(label,
            pixel_at(viewport, wall_samples[i][0], wall_samples[i][1]),
            47, A_F0107);
    }
    for (size_t i = 0; i < sizeof(floor_samples) / sizeof(floor_samples[0]); ++i) {
        char label[64];
        snprintf(label, sizeof(label), "pixel.floor.%zu", i);
        ok &= expect_int(label,
            pixel_at(viewport, floor_samples[i][0], floor_samples[i][1]),
            63, A_F0108);
    }
    for (size_t i = 0; i < sizeof(outside_samples) / sizeof(outside_samples[0]); ++i) {
        char label[64];
        snprintf(label, sizeof(label), "pixel.outside.%zu", i);
        ok &= expect_int(label,
            pixel_at(viewport, outside_samples[i][0], outside_samples[i][1]),
            0, A_ORDER);
    }

    ok &= expect_int("run.reject.null.plan",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_run_pc34(
            NULL, viewport, viewport_len, &result),
        -1, A_ORDER);
    ok &= expect_int("run.reject.null.viewport",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_run_pc34(
            &plan, NULL, viewport_len, &result),
        -1, A_ORDER);
    ok &= expect_int("run.reject.null.result",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_run_pc34(
            &plan, viewport, viewport_len, NULL),
        -1, A_ORDER);
    ok &= expect_int("run.reject.short.viewport",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_run_pc34(
            &plan, viewport, viewport_len - 1, &result),
        -1, A_ORDER);
    plan.ok = 0;
    ok &= expect_int("run.reject.bad.plan",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_run_pc34(
            &plan, viewport, viewport_len, &result),
        -1, A_ORDER);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d3c_f0107_f0108_first_backdrop_pc34_compat\n");
    printf("source_evidence=%s\n",
        csb_v1_viewport_d3c_f0107_f0108_first_backdrop_source_evidence_pc34());

    ok &= test_contract();
    ok &= test_plan();
    ok &= test_run_and_pixels();

    if (ok && g_failures == 0) {
        printf("PASS csb_v1_viewport_d3c_f0107_f0108_first_backdrop_pc34_compat assertions=%d failures=0\n",
            g_assertions);
        return 0;
    }

    printf("FAIL csb_v1_viewport_d3c_f0107_f0108_first_backdrop_pc34_compat assertions=%d failures=%d\n",
        g_assertions, g_failures);
    return 1;
}
