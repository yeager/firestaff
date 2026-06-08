#include "dm1_v1_viewport_f0095_floor_ornament_aggregate_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        return;
    }
    printf("PASS %s=%d anchor=%s\n", id, want, anchor);
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_ge(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got < want) {
        ++g_failures;
        printf("FAIL %s got=%d want>=%d anchor=%s\n", id, got, want, anchor);
        return;
    }
    printf("PASS %s=%d >= %d anchor=%s\n", id, got, want, anchor);
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)",
               anchor);
        return;
    }
    printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
}

int main(void)
{
    const char *a_f0095 = "ReDMCSB DUNVIEW.C F0095:2124-2223";
    const char *a_f0108 = "ReDMCSB DUNVIEW.C F0108:3959-4008";
    const char *a_f0128 = "ReDMCSB DUNVIEW.C F0128:8318-8542";
    const char *a_f0098 = "ReDMCSB DUNVIEW.C F0098:2962-3002";
    const char *a_defs = "ReDMCSB DEFS.H:2088/2544/2558/2759/4042-4043";
    const char *a_lineage = "CSB-lineage Viewport.cpp:1903-1915";
    const DM1_V1_F0095FloorOrnamentBindingPc34 *binding;
    DM1_V1_F0095FloorOrnamentAggregateResultPc34 d1c_open;
    DM1_V1_F0095FloorOrnamentAggregateResultPc34 d0c_open;
    int passed = -1;
    int failed = -1;
    int rerun_passed = -1;
    int rerun_failed = -1;
    int rc;

    printf("probe=dm1_v1_viewport_f0095_floor_ornament_aggregate_pc34_compat\n");

    binding = dm1_v1_viewport_f0095_floor_ornament_aggregate_binding_pc34();
    expect_int("spec.count", dm1_v1_viewport_f0095_floor_ornament_aggregate_spec_count_pc34(),
               4, a_f0128);
    expect_bool("binding.contract_only", binding->contract_only, true, a_f0095);
    expect_bool("binding.no_real_asset_runtime_parity",
                binding->real_asset_runtime_parity, false, a_f0095);
    expect_int("binding.c10", binding->c10_transparent_color, 10, a_defs);
    expect_int("binding.m558.pc34", binding->m558_floor_slot_pc34, 4, a_defs);
    expect_int("binding.m558.i34", binding->m558_floor_slot_i34, 5, a_defs);
    expect_int("binding.floor_zone_base", binding->floor_zone_base, 1500, a_f0108);
    expect_int("binding.floor_zone_stride", binding->floor_zone_stride, 11, a_f0108);
    expect_int("binding.c702", binding->wall_zone_d3l2_c702, 702, a_defs);
    expect_int("binding.c703", binding->wall_zone_d3r2_c703, 703, a_defs);
    expect_bool("binding.g0095_wallset_keepout",
                binding->f0095_wallset_loads_g0095_native_wall_binding, true,
                a_f0095);
    expect_bool("binding.g0109_portrait_box_keepout",
                binding->g0109_champion_portrait_box_is_not_floor_ornament, true,
                a_f0095);

    expect_bool("d1c.open.eval",
                dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                    DM1_V1_F0095_CENTER_D1C_PC34,
                    DM1_V1_F0095_CENTER_ELEMENT_OPEN_PC34, 4, 2, &d1c_open),
                true, a_f0108);
    expect_bool("d1c.open.calls_f0108", d1c_open.calls_f0108, true, a_f0108);
    expect_int("d1c.open.line", d1c_open.f0108_source_line, 7926,
               "ReDMCSB DUNVIEW.C F0124:7926");
    expect_int("d1c.open.index", d1c_open.floor_ornament_index, 3, a_f0108);
    expect_int("d1c.open.zone", d1c_open.floor_ornament_zone, 1531, a_f0108);
    expect_bool("d1c.open.before_f0115", d1c_open.f0108_precedes_f0115_when_present,
                true, a_lineage);
    expect_bool("d1c.open.f0098_precedes_row", d1c_open.f0098_precedes_f0128_center_row,
                true, a_f0098);

    expect_bool("d0c.open.eval",
                dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                    DM1_V1_F0095_CENTER_D0C_PC34,
                    DM1_V1_F0095_CENTER_ELEMENT_OPEN_PC34, 4, 2, &d0c_open),
                true, a_f0098);
    expect_bool("d0c.open.no_f0108", d0c_open.calls_f0108, false,
                "ReDMCSB DUNVIEW.C F0127:8164-8310 keep-out");
    expect_int("d0c.open.no_floor_zone", d0c_open.floor_ornament_zone, -1, a_f0098);
    expect_bool("d0c.open.f0098_keepout", d0c_open.calls_f0098_inside_center_square,
                true, a_f0098);

    expect_contains("source.f0095",
                    dm1_v1_viewport_f0095_floor_ornament_aggregate_source_lock_pc34(),
                    "DUNVIEW.C F0095:2124-2223", a_f0095);
    expect_contains("source.f0108",
                    dm1_v1_viewport_f0095_floor_ornament_aggregate_source_lock_pc34(),
                    "DUNVIEW.C F0108:3959-4008", a_f0108);
    expect_contains("source.f0124",
                    dm1_v1_viewport_f0095_floor_ornament_aggregate_source_lock_pc34(),
                    "DUNVIEW.C F0124:7874-7926", a_f0108);
    expect_contains("source.f0128",
                    dm1_v1_viewport_f0095_floor_ornament_aggregate_source_lock_pc34(),
                    "DUNVIEW.C F0128:8318-8542", a_f0128);
    expect_contains("source.f0098",
                    dm1_v1_viewport_f0095_floor_ornament_aggregate_source_lock_pc34(),
                    "DUNVIEW.C F0098:2962-3002", a_f0098);
    expect_contains("source.dungeon",
                    dm1_v1_viewport_f0095_floor_ornament_aggregate_source_lock_pc34(),
                    "DUNGEON.C F0163:1769-1838", "ReDMCSB DUNGEON.C F0163");
    expect_contains("source.defs",
                    dm1_v1_viewport_f0095_floor_ornament_aggregate_source_lock_pc34(),
                    "DEFS.H:2088/2544/2558/2759/4042-4043", a_defs);
    expect_contains("source.lineage",
                    dm1_v1_viewport_f0095_floor_ornament_aggregate_source_lock_pc34(),
                    "Viewport.cpp:1903-1915", a_lineage);

    rc = dm1_v1_viewport_f0095_floor_ornament_aggregate_run_pc34(&passed, &failed);
    expect_int("run.return", rc, 0, a_f0095);
    expect_int("run.failed", failed, 0, a_f0108);
    expect_ge("run.assertions_at_least_160", passed, 160, a_f0128);

    rc = dm1_v1_viewport_f0095_floor_ornament_aggregate_run_pc34(
        &rerun_passed, &rerun_failed);
    expect_int("rerun.return", rc, 0, a_f0095);
    expect_int("rerun.failed", rerun_failed, 0, a_f0108);
    expect_int("rerun.same_assertion_count", rerun_passed, passed, a_f0128);
    rc = dm1_v1_viewport_f0095_floor_ornament_aggregate_run_pc34(NULL, NULL);
    expect_int("null_counters.return", rc, 0, a_f0095);
    rc = dm1_v1_viewport_f0095_floor_ornament_aggregate_run_pc34(&rerun_passed, NULL);
    expect_int("null_failed.return", rc, 0, a_f0095);
    expect_int("null_failed.passed", rerun_passed, passed, a_f0128);
    rc = dm1_v1_viewport_f0095_floor_ornament_aggregate_run_pc34(NULL, &rerun_failed);
    expect_int("null_passed.return", rc, 0, a_f0095);
    expect_int("null_passed.failed", rerun_failed, 0, a_f0108);

    printf("assertions=%d contract_assertions=%d failures=%d\n",
           g_assertions + passed, passed, g_failures + failed);
    if (g_failures == 0 && failed == 0) {
        printf("PASS dm1_v1_viewport_f0095_floor_ornament_aggregate "
               "assertions=%d failures=0\n", g_assertions + passed);
        return 0;
    }
    return 1;
}
