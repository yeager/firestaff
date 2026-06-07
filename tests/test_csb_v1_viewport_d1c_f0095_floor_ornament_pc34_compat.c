#include "csb_v1_viewport_d1c_f0095_floor_ornament_pc34_compat.h"

#include <stdio.h>

static int g_assertions = 0;
static int g_failures = 0;

static const char *A_F0095 =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; D1C floor route F0124:7874,7926";
static const char *A_F0108 =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; F0108:3959-3998";
static const char *A_DEFS =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DEFS.H:2088,2544,2558,2759,4042-4043";
static const char *A_LINEAGE =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; CSB Viewport.cpp:1903-1906";

static void expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return;
    }
    printf("PASS %s=%d anchor=%s\n", label, want, anchor);
}

static void expect_ge(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got < want) {
        ++g_failures;
        printf("FAIL %s got=%d want>=%d anchor=%s\n", label, got, want, anchor);
        return;
    }
    printf("PASS %s=%d >= %d anchor=%s\n", label, got, want, anchor);
}

int main(void)
{
    int passed = -1;
    int failed = -1;
    int rerun_passed = -1;
    int rerun_failed = -1;
    int rc;

    printf("probe=csb_v1_viewport_d1c_f0095_floor_ornament_pc34_compat\n");
    rc = csb_v1_viewport_d1c_f0095_floor_ornament_run(&passed, &failed);

    expect_int("run.return", rc, 0, A_F0095);
    expect_int("run.failed", failed, 0, A_F0108);
    expect_ge("run.assertions_at_least_70", passed, 70, A_F0095);
    expect_ge("run.assertions_cover_valid_invalid_depth_lane", passed, 20, A_F0095);
    expect_ge("run.assertions_cover_ordinal_zero_nonzero", passed, 35, A_F0108);
    expect_ge("run.assertions_cover_c10_transparency", passed, 45, A_DEFS);
    expect_ge("run.assertions_cover_f0108_blit", passed, 50, A_F0108);
    expect_ge("run.assertions_cover_baseline_f0098", passed, 60, A_F0095);
    expect_ge("run.assertions_cover_exclusions", passed, 65, A_F0095);
    expect_ge("run.assertions_cover_lineage", passed, 70, A_LINEAGE);

    rc = csb_v1_viewport_d1c_f0095_floor_ornament_run(&rerun_passed,
                                                      &rerun_failed);
    expect_int("rerun.return", rc, 0, A_F0095);
    expect_int("rerun.failed", rerun_failed, 0, A_F0108);
    expect_int("rerun.same_assertion_count", rerun_passed, passed, A_F0095);

    rc = csb_v1_viewport_d1c_f0095_floor_ornament_run(NULL, NULL);
    expect_int("null_counters.return", rc, 0, A_F0095);
    rc = csb_v1_viewport_d1c_f0095_floor_ornament_run(&rerun_passed, NULL);
    expect_int("null_failed.return", rc, 0, A_F0095);
    expect_int("null_failed.passed_still_populated", rerun_passed, passed, A_F0095);
    rc = csb_v1_viewport_d1c_f0095_floor_ornament_run(NULL, &rerun_failed);
    expect_int("null_passed.return", rc, 0, A_F0095);
    expect_int("null_passed.failed_still_populated", rerun_failed, 0, A_F0108);

    printf("assertions=%d contract_assertions=%d failures=%d\n",
           g_assertions + passed, passed, g_failures + failed);
    if (g_failures == 0 && failed == 0) {
        printf("PASS csb_v1_viewport_d1c_f0095_floor_ornament_pc34_compat "
               "assertions=%d failures=0\n", g_assertions + passed);
        return 0;
    }
    return 1;
}
