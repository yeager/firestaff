#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void test_model(void)
{
    DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 built;
    const DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *model =
        dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_pc34();

    expect_int("builder.null",
               dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_builder_pc34(NULL),
               0, "defensive builder guard");
    expect_int("builder.ok",
               dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_builder_pc34(&built),
               1, "deterministic model builder");
    expect_int("model.present", model != NULL, 1, "default model accessor");
    if (!model) return;

    expect_int("hash.null",
               dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_hash_model_pc34(NULL),
               0, "hash null guard");
    expect_int("hash.stable", built.deterministic_hash == model->deterministic_hash, 1,
               "builder hash stable");
    expect_int("hash.accessor",
               dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_deterministic_hash_pc34() ==
                   model->deterministic_hash,
               1, "hash accessor stable");
    expect_int("anchor.count", (int)dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_anchor_count_pc34(),
               4, "D3C/D2 uncovered BUG0_64 anchors");
    expect_int("model.bug0_64_count", model->bug0_64_anchor_count, 4,
               "DUNVIEW.C:6814/7020/7213/7357");
    expect_int("model.no_graphics_dat", model->no_graphics_dat_reads, 1,
               "contract-only no asset reads");
    expect_int("model.no_original_parity", model->no_original_dos_pixel_parity, 1,
               "no original DOS parity claim");
}

static void test_anchors(void)
{
    static const struct {
        const char *name;
        int view_square;
        int view_floor;
        int line;
        int order;
        int zone;
        const char *function_name;
    } expected[] = {
        { "D3C", 11, 3, 6814, 0x3421, 1503, "F0118" },
        { "D2L", 7, 5, 7020, 0x3421, 1505, "F0119" },
        { "D2R", 8, 7, 7213, 0x4312, 1507, "F0120" },
        { "D2C", 6, 6, 7357, 0x3421, 1506, "F0121" }
    };
    size_t i;

    expect_int("anchor.oob",
               dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_anchor_at_pc34(4) == NULL,
               1, "anchor_at out-of-range guard");
    for (i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const DM1_V1_D3CD2F0108FloorOrnamentOcclusionAnchorPc34 *a =
            dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_anchor_at_pc34(i);
        char id[64];
        snprintf(id, sizeof(id), "anchor.%s.present", expected[i].name);
        expect_int(id, a != NULL, 1, "anchor table entry");
        if (!a) continue;
        snprintf(id, sizeof(id), "anchor.%s.view_square", expected[i].name);
        expect_int(id, a->view_square, expected[i].view_square, a->redmcsb_function);
        snprintf(id, sizeof(id), "anchor.%s.view_floor", expected[i].name);
        expect_int(id, a->view_floor, expected[i].view_floor, a->redmcsb_function);
        snprintf(id, sizeof(id), "anchor.%s.line", expected[i].name);
        expect_int(id, a->source_line, expected[i].line, a->f0108_source_lines);
        snprintf(id, sizeof(id), "anchor.%s.order", expected[i].name);
        expect_int(id, a->cell_order, expected[i].order, a->f0115_source_lines);
        snprintf(id, sizeof(id), "anchor.%s.zone", expected[i].name);
        expect_int(id, a->floor_zone_at_coordinate_zero, expected[i].zone,
                   "F0108 C1500 + coordinateSet*11 + viewFloor");
        snprintf(id, sizeof(id), "anchor.%s.function", expected[i].name);
        expect_contains(id, a->redmcsb_function, expected[i].function_name,
                        "ReDMCSB DUNVIEW function");
        snprintf(id, sizeof(id), "anchor.%s.bug0", expected[i].name);
        expect_contains(id, a->f0108_source_lines, "BUG0_64", "BUG0_64 source comment");
    }
}

static void test_behavior(void)
{
    expect_int("zone.d3c", dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_zone_pc34(0, 3),
               1503, "DUNVIEW.C F0108 zone math");
    expect_int("zone.d2c.coord1", dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_zone_pc34(1, 6),
               1517, "C1500 + 1*11 + M592");
    expect_int("occludes.d3c.nonzero",
               dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_lane_occludes_pc34(
                   DM1_V1_D3C_D2_FOCCL_LANE_D3C_PC34, 1u),
               1, "DUNVIEW.C:6814 BUG0_64");
    expect_int("occludes.d2r.footprint_mask",
               dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_lane_occludes_pc34(
                   DM1_V1_D3C_D2_FOCCL_LANE_D2R_PC34,
                   DM1_V1_D3C_D2_FOCCL_FOOTPRINT_MASK_PC34),
               1, "F0108 MASK0x8000 footprints still draw");
    expect_int("occludes.zero",
               dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_lane_occludes_pc34(
                   DM1_V1_D3C_D2_FOCCL_LANE_D2L_PC34, 0u),
               0, "F0108 ordinal zero skips blit");
    expect_int("occludes.invalid_lane",
               dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_lane_occludes_pc34(
                   (DM1_V1_D3CD2F0108FloorOrnamentOcclusionLanePc34)99, 1u),
               0, "lane guard");
}

static void test_self_and_notes(void)
{
    const DM1_V1_D3CD2F0108FloorOrnamentOcclusionSelfTestResultPc34 *r;
    expect_int("self.ok", dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_self_test_pc34(),
               1, "module self-test");
    r = dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_last_self_test_result_pc34();
    expect_int("self.result.present", r != NULL, 1, "self-test result accessor");
    if (r) {
        expect_int("self.failures", r->failures, 0, "self-test failures");
        expect_int("self.bug0_64_count_four", r->bug0_64_count_four, 1,
                   "four BUG0_64 anchors");
    }
    expect_contains("evidence.bug0",
                    dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "BUG0_64", "source evidence");
    expect_contains("evidence.lines",
                    dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "6814", "D3C BUG0_64 source line");
    expect_contains("evidence.d2c",
                    dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "7357", "D2C BUG0_64 source line");
    expect_contains("disjoint.existing",
                    dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_disjointness_note_pc34(),
                    "D3L/D3R", "does not repeat existing gate");
}

int main(void)
{
    test_model();
    test_anchors();
    test_behavior();
    test_self_and_notes();
    printf("probe=test_dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_pc34_compat\n");
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
