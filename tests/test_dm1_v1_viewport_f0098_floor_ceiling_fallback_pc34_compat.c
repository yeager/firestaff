#include "dm1_v1_viewport_f0098_floor_ceiling_fallback_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *
find_step(const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *steps,
          size_t count,
          DM1_V1_F0098FloorCeilingFallbackStepPc34 step)
{
    size_t i;

    for (i = 0; i < count; ++i) {
        if (steps[i].step == step) return &steps[i];
    }
    return NULL;
}

static void test_function_level_spec(void)
{
    const DM1_V1_F0098FloorCeilingFallbackSpecPc34 *spec =
        dm1_v1_viewport_f0098_floor_ceiling_fallback_spec_pc34();

    expect_int("spec.present", spec != NULL, 1,
               "DUNVIEW.C:F0098 function-level contract");
    if (!spec) return;

    expect_int("spec.contract_only", spec->contract_only, 1,
               "contract-only source lock");
    expect_int("spec.function_level", spec->function_level_contract, 1,
               "not a side-specific F0098 gate");
    expect_int("spec.direction_specific", spec->direction_specific_contract, 0,
               "not D0L/D0R or D2L/D2R specific");
    expect_int("spec.real_asset_required", spec->real_asset_required, 0,
               "contract-only gate has no asset dependency");
    expect_int("spec.viewport_width", spec->viewport_width, 224,
               "DUNVIEW.C:F0098 viewport width");
    expect_int("spec.viewport_height", spec->viewport_height, 136,
               "DUNVIEW.C:F0098 viewport height");
    expect_int("spec.viewport_byte_width", spec->viewport_byte_width, 112,
               "DEFS.H C112_BYTE_WIDTH_VIEWPORT");
    expect_int("spec.black_area_height", spec->black_area_height, 37,
               "DUNVIEW.C:F0098 black-area clear");
    expect_int("spec.ceiling_height", spec->ceiling_height, 29,
               "DUNVIEW.C:F0098 ceiling copy");
    expect_int("spec.floor_y", spec->floor_y, 66,
               "DUNVIEW.C:F0098 floor area starts after black band");
    expect_int("spec.floor_height", spec->floor_height, 70,
               "DUNVIEW.C:F0098 floor copy");
}

static void test_constants_and_fallback_selection(void)
{
    const DM1_V1_F0098FloorCeilingFallbackSpecPc34 *spec =
        dm1_v1_viewport_f0098_floor_ceiling_fallback_spec_pc34();

    expect_int("constant.c10", spec->c10_transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("constant.cm1", spec->no_transparency_color, -1,
               "DEFS.H:2076 CM1_COLOR_NO_TRANSPARENCY");
    expect_int("bitmap.floor", spec->floor_native_bitmap_index, -1,
               "DUNVIEW.C global G2108_Floor");
    expect_int("bitmap.ceiling", spec->ceiling_native_bitmap_index, -2,
               "DUNVIEW.C global G2109_Ceiling");
    expect_contains("symbol.floor", spec->floor_symbol, "G2108_Floor",
                    "DUNVIEW.C:F0098 floor source");
    expect_contains("symbol.ceiling", spec->ceiling_symbol, "G2109_Ceiling",
                    "DUNVIEW.C:F0098 ceiling source");

    expect_int("selection.d0l.view_square", spec->d0l_view_square, 10,
               "DEFS.H:2588 M610_VIEW_SQUARE_D0L");
    expect_int("selection.d0r.view_square", spec->d0r_view_square, 11,
               "DEFS.H:2589 M611_VIEW_SQUARE_D0R");
    expect_int("selection.d0l.wall_zone", spec->d0l_wall_zone, 716,
               "DEFS.H:4056 C716_ZONE_WALL_D0L");
    expect_int("selection.d0r.wall_zone", spec->d0r_wall_zone, 717,
               "DEFS.H:4057 C717_ZONE_WALL_D0R");
    expect_int("selection.d0l.wall_bitmap_index", spec->d0l_wall_bitmap_index, 1,
               "DEFS.H C01_WALL_D0L");
    expect_int("selection.d0r.wall_bitmap_index", spec->d0r_wall_bitmap_index, 0,
               "DEFS.H C00_WALL_D0R");
    expect_contains("symbol.d0l.view_square", spec->d0l_view_square_symbol,
                    "M610_VIEW_SQUARE_D0L", "DUNVIEW.C:F0125 F0115 route");
    expect_contains("symbol.d0r.view_square", spec->d0r_view_square_symbol,
                    "M611_VIEW_SQUARE_D0R", "DUNVIEW.C:F0126 F0115 route");
    expect_contains("symbol.d0l.zone", spec->d0l_wall_zone_symbol,
                    "C716_ZONE_WALL_D0L", "DUNVIEW.C:F0125 wall fallback");
    expect_contains("symbol.d0r.zone", spec->d0r_wall_zone_symbol,
                    "C717_ZONE_WALL_D0R", "DUNVIEW.C:F0126 wall fallback");
}

static void test_dispatch_contract(void)
{
    size_t count = 0;
    const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *steps =
        dm1_v1_viewport_f0098_floor_ceiling_fallback_dispatch_pc34(&count);
    const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *guard;
    const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *ceiling;
    const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *floor;
    const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *exit_step;
    const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *d0l;
    const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *d0r;
    const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *present;

    expect_int("dispatch.count", (int)count, 9,
               "F0098 + F0128 function-level fallback contract");
    guard = find_step(steps, count,
        DM1_V1_F0098_FALLBACK_STEP_F0128_DIRTY_GUARD_PC34);
    ceiling = find_step(steps, count,
        DM1_V1_F0098_FALLBACK_STEP_F0098_COPY_CEILING_PC34);
    floor = find_step(steps, count,
        DM1_V1_F0098_FALLBACK_STEP_F0098_COPY_FLOOR_PC34);
    exit_step = find_step(steps, count,
        DM1_V1_F0098_FALLBACK_STEP_F0098_CLEAR_DIRTY_FLAG_PC34);
    d0l = find_step(steps, count,
        DM1_V1_F0098_FALLBACK_STEP_F0128_ENUMERATE_D0L_PC34);
    d0r = find_step(steps, count,
        DM1_V1_F0098_FALLBACK_STEP_F0128_ENUMERATE_D0R_PC34);
    present = find_step(steps, count,
        DM1_V1_F0098_FALLBACK_STEP_F0128_PRESENT_AND_PREFILL_PC34);

    expect_int("dispatch.guard.present", guard != NULL, 1,
               "DUNVIEW.C:8337-8338 dirty guard");
    expect_int("dispatch.ceiling.present", ceiling != NULL, 1,
               "DUNVIEW.C:2995 ceiling copy");
    expect_int("dispatch.floor.present", floor != NULL, 1,
               "DUNVIEW.C:2996 floor copy");
    expect_int("dispatch.exit.present", exit_step != NULL, 1,
               "DUNVIEW.C:3002 dirty flag clear");
    expect_int("dispatch.guard.before_ceiling", guard && ceiling &&
               guard->order_index < ceiling->order_index, 1,
               "DUNVIEW.C:F0128 enters F0098 before copies");
    expect_int("dispatch.ceiling.before_floor", ceiling && floor &&
               ceiling->order_index < floor->order_index, 1,
               "DUNVIEW.C:F0098 ceiling copy precedes floor copy");
    expect_int("dispatch.floor.before_exit", floor && exit_step &&
               floor->order_index < exit_step->order_index, 1,
               "DUNVIEW.C:F0098 copies before dirty flag clear");
    expect_int("dispatch.exit.before_d0l", exit_step && d0l &&
               exit_step->order_index < d0l->order_index, 1,
               "DUNVIEW.C:F0128 square enumeration after F0098");
    expect_int("dispatch.d0l.before_d0r", d0l && d0r &&
               d0l->order_index < d0r->order_index, 1,
               "DUNVIEW.C:F0128 D0L then D0R enumeration");
    expect_int("dispatch.d0r.before_present", d0r && present &&
               d0r->order_index < present->order_index, 1,
               "DUNVIEW.C:F0128 present after viewport enumeration");
    expect_contains("dispatch.d0l.contract", d0l ? d0l->contract : NULL,
                    "M610", "DUNVIEW.C:F0125 view-square selection");
    expect_contains("dispatch.d0r.contract", d0r ? d0r->contract : NULL,
                    "M611", "DUNVIEW.C:F0126 view-square selection");
}

static void test_predicates_and_transparency(void)
{
    expect_int("guard.false",
               dm1_v1_viewport_f0098_floor_ceiling_should_enter_pc34(false), 0,
               "DUNVIEW.C:8337 dirty flag guard");
    expect_int("guard.true",
               dm1_v1_viewport_f0098_floor_ceiling_should_enter_pc34(true), 1,
               "DUNVIEW.C:8337 dirty flag guard");
    expect_int("exit.dirty_flag",
               dm1_v1_viewport_f0098_floor_ceiling_dirty_after_exit_pc34(), 0,
               "DUNVIEW.C:3002 clears G0297");
    expect_int("zero_ordinal.no_draw",
               dm1_v1_viewport_f0098_floor_ceiling_zero_ordinal_draws_pc34(0), 0,
               "DUNVIEW.C:F0108 if (P0118_ui_FloorOrnamentOrdinal)");
    expect_int("nonzero_ordinal.draw",
               dm1_v1_viewport_f0098_floor_ceiling_zero_ordinal_draws_pc34(1), 1,
               "DUNVIEW.C:F0108 non-zero floor ornament ordinal");
    expect_int("c10.transparent_preserves_dest",
               dm1_v1_viewport_f0098_floor_ceiling_blit_pixel_pc34(0x44, 10, 10),
               0x44, "DUNVIEW.C:F0104 C10_COLOR_FLESH transparency");
    expect_int("c10.opaque_writes_source",
               dm1_v1_viewport_f0098_floor_ceiling_blit_pixel_pc34(0x44, 9, 10),
               9, "DUNVIEW.C:F0104 opaque overlay pixel");
    expect_int("cm1.no_transparency_writes_c10",
               dm1_v1_viewport_f0098_floor_ceiling_blit_pixel_pc34(0x44, 10, -1),
               10, "DUNVIEW.C:F0792 CM1_COLOR_NO_TRANSPARENCY");
}

static void test_source_evidence(void)
{
    const char *e =
        dm1_v1_viewport_f0098_floor_ceiling_fallback_source_evidence_pc34();

    expect_contains("evidence.contract", e, "contract_only=1",
                    "source evidence");
    expect_contains("evidence.function_level", e, "function_level=1",
                    "source evidence");
    expect_contains("evidence.not_direction_specific", e, "direction_specific=0",
                    "source evidence");
    expect_contains("evidence.f0098", e, "F0098:2962-3002",
                    "DUNVIEW.C F0098 function body");
    expect_contains("evidence.f0128.guard", e, "F0128:8337-8338",
                    "DUNVIEW.C F0128 dirty guard");
    expect_contains("evidence.f0128.enumeration", e, "F0128:8564-8571",
                    "DUNVIEW.C F0128 viewport enumeration");
    expect_contains("evidence.m610", e, "M610_VIEW_SQUARE_D0L",
                    "DEFS.H M610");
    expect_contains("evidence.m611", e, "M611_VIEW_SQUARE_D0R",
                    "DEFS.H M611");
    expect_contains("evidence.c716", e, "C716_ZONE_WALL_D0L",
                    "DEFS.H C716");
    expect_contains("evidence.c717", e, "C717_ZONE_WALL_D0R",
                    "DEFS.H C717");
    expect_contains("evidence.c10", e, "C10_COLOR_FLESH",
                    "DEFS.H C10 transparency");
    expect_contains("evidence.zero_ordinal", e, "zero floor ornament ordinal is no-draw",
                    "DUNVIEW.C F0108 zero ordinal");
    expect_contains("evidence.prefill", e, "presents then pre-fills with F0098",
                    "DUNVIEW.C F0128 exit prefill");
}

int main(void)
{
    test_function_level_spec();
    test_constants_and_fallback_selection();
    test_dispatch_contract();
    test_predicates_and_transparency();
    test_source_evidence();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_f0098_floor_ceiling_fallback_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_f0098_floor_ceiling_fallback_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
