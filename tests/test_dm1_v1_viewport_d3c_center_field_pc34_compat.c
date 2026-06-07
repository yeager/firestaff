#include "dm1_v1_viewport_d3c_center_field_pc34_compat.h"

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

static void expect_nonempty(const char *id, const char *got, const char *anchor)
{
    ++g_assertions;
    if (!got || got[0] == '\0') {
        printf("FAIL %s got=empty at %s\n", id, anchor);
        ++g_failures;
    } else {
        printf("PASS %s nonempty (%s)\n", id, anchor);
    }
}

static void expect_contains(const char *id,
                            const char *haystack,
                            const char *needle,
                            const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void test_contract_scalars(
    const Dm1V1ViewportD3cCenterFieldContractPc34Compat *contract)
{
    expect_int("d3c.contract_only", contract->contract_only, 1,
               "ReDMCSB DUNVIEW.C F0128:8488-8499");
    expect_int("d3c.view_square_index", contract->view_square_index_d3c,
               DM1_V1_D3C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX_D3C,
               "ReDMCSB DEFS.H:2607 M600_VIEW_SQUARE_D3C");
    expect_int("d3c.view_square_index_value", contract->view_square_index_d3c,
               11, "ReDMCSB DEFS.H:2607 M600_VIEW_SQUARE_D3C");
    expect_int("d3c.lane", contract->lane, 0,
               "ReDMCSB DUNVIEW.C F0128:8498");
    expect_int("d3c.depth", contract->depth, 3,
               "ReDMCSB DUNVIEW.C F0128:8490-8499");
    expect_int("d3c.field_aspect", contract->field_aspect, 2,
               "ReDMCSB DUNVIEW.C:377 G2035[M600_VIEW_SQUARE_D3C]");
    expect_int("d3c.wall_case_returns", contract->wall_case_returns, 1,
               "ReDMCSB DUNVIEW.C F0118:6716-6720");
    expect_int("d3c.g0163_frame_clip", contract->g0163_frame_clip_applies, 1,
               "ReDMCSB DUNVIEW.C:581-583 G0163 D3C frame");
}

static void test_routes(
    const Dm1V1ViewportD3cCenterFieldContractPc34Compat *contract)
{
    expect_int("d3c.no_f0100", contract->routes_through_f0100, 0,
               "ReDMCSB DUNVIEW.C F0118:6811-6831 excludes wall arm 6699");
    expect_int("d3c.no_f0105", contract->routes_through_f0105, 0,
               "ReDMCSB DUNVIEW.C F0118:6811-6831 excludes door arm 6735");
    expect_int("d3c.no_f0107", contract->routes_through_f0107, 0,
               "ReDMCSB DUNVIEW.C F0118:6811-6831 excludes wall arm 6716");
    expect_int("d3c.no_f0111", contract->routes_through_f0111, 0,
               "ReDMCSB DUNVIEW.C F0118:6811-6831 excludes door arm 6744");
    expect_int("d3c.f0113", contract->routes_through_f0113, 1,
               "ReDMCSB DUNVIEW.C F0118:6825-6831");
    expect_int("d3c.c10_transparency",
               contract->preserves_c10_color_flesh_transparency, 1,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("d3c.floor_ceiling_f0098",
               contract->floor_ceiling_ownership_f0098, 1,
               "ReDMCSB DUNVIEW.C F0098:2962-3003");
    expect_int("d3c.f0115_after_f0113_guard",
               contract->things_pass_includes_f0115_only_after_f0113, 1,
               "ReDMCSB DUNVIEW.C F0118:6816 and F0118:6825-6831");
}

static void test_anchor_strings(
    const Dm1V1ViewportD3cCenterFieldContractPc34Compat *contract)
{
    expect_nonempty("d3c.anchor.f0128", contract->redmcsb_f0128_anchor,
                    "ReDMCSB DUNVIEW.C F0128:8488-8499");
    expect_nonempty("d3c.anchor.f0124", contract->redmcsb_f0124_anchor,
                    "ReDMCSB DUNVIEW.C F0118:6811-6831; F0124:7922-7955");
    expect_nonempty("d3c.anchor.f0113", contract->redmcsb_f0113_anchor,
                    "ReDMCSB DUNVIEW.C F0113:6213-6219");
    expect_nonempty("d3c.anchor.view_square",
                    contract->redmcsb_defs_view_square_anchor,
                    "ReDMCSB DEFS.H:2595-2611");
    expect_nonempty("d3c.anchor.zone", contract->redmcsb_defs_zone_anchor,
                    "ReDMCSB DEFS.H:3432-3437; 4030-4049");
    expect_nonempty("d3c.anchor.c10", contract->redmcsb_defs_c10_anchor,
                    "ReDMCSB DEFS.H:2088");
    expect_contains("d3c.anchor.f0128.line",
                    contract->redmcsb_f0128_anchor, "F0128:8488-8499",
                    "ReDMCSB DUNVIEW.C F0128:8488-8499");
    expect_contains("d3c.anchor.f0124.line",
                    contract->redmcsb_f0124_anchor, "F0124:7922-7955",
                    "ReDMCSB DUNVIEW.C F0124:7922-7955");
    expect_contains("d3c.anchor.f0113.line",
                    contract->redmcsb_f0113_anchor, "F0113:6213-6219",
                    "ReDMCSB DUNVIEW.C F0113:6213-6219");
}

static void test_required_phrases(
    const Dm1V1ViewportD3cCenterFieldContractPc34Compat *contract)
{
    static const char *const phrases[] = {
        "contract_only=1",
        "DUNVIEW.C F0128",
        "DUNVIEW.C F0124",
        "DUNVIEW.C F0113",
        "DEFS.H:2595-2611",
        "DEFS.H:2088 C10_COLOR_FLESH",
        "DEFS.H:3432/4030/4049",
        "no F0100/F0105/F0107/F0111",
        "F0115 thing-pass only after F0113"
    };
    size_t i;

    for (i = 0; i < sizeof(phrases) / sizeof(phrases[0]); ++i) {
        expect_contains("d3c.non_overlap.required_phrase",
                        contract->non_overlap_note, phrases[i],
                        "ReDMCSB DUNVIEW.C F0118:6811-6831");
        expect_contains("d3c.summary.required_phrase",
                        contract->source_summary, phrases[i],
                        "ReDMCSB DUNVIEW.C F0118:6811-6831");
    }
}

static void test_steps(void)
{
    Dm1V1ViewportD3cCenterFieldStepPc34Compat steps[5];
    size_t count = dm1_v1_viewport_d3c_center_field_steps_pc34_compat(
        steps, sizeof(steps) / sizeof(steps[0]));
    size_t i;

    expect_int("d3c.steps.count", (int)count, 5,
               "ReDMCSB DUNVIEW.C F0098/F0128/F0118/F0115/F0113");
    for (i = 0; i < count && i < sizeof(steps) / sizeof(steps[0]); ++i) {
        expect_nonempty("d3c.step.name", steps[i].name,
                        "ReDMCSB DUNVIEW.C F0118:6811-6831");
        expect_nonempty("d3c.step.anchor", steps[i].redmcsb_anchor,
                        "ReDMCSB DUNVIEW.C F0118:6811-6831");
    }
}

static int run_tests(void)
{
    const Dm1V1ViewportD3cCenterFieldContractPc34Compat *contract =
        dm1_v1_viewport_d3c_center_field_contract_pc34_compat();

    if (!contract) {
        printf("FAIL d3c.contract got=NULL at ReDMCSB DUNVIEW.C F0118:6642\n");
        ++g_assertions;
        ++g_failures;
        return 0;
    }
    printf("PASS d3c.contract nonnull (ReDMCSB DUNVIEW.C F0118:6642)\n");
    ++g_assertions;

    test_contract_scalars(contract);
    test_routes(contract);
    test_anchor_strings(contract);
    test_required_phrases(contract);
    test_steps();
    return g_failures == 0;
}

int main(void)
{
    (void)run_tests();
    printf("assertions=%d\n", g_assertions);
    return g_failures == 0 ? 0 : 1;
}
