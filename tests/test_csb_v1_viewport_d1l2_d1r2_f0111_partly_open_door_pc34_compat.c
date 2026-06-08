#include "csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C F0111 lines 4218-4337";
static const char *A_F0111_FRAME =
    "ReDMCSB DUNVIEW.C F0111 lines 4311-4313";
static const char *A_F0111_FIRST =
    "ReDMCSB DUNVIEW.C F0111 lines 4317-4324";
static const char *A_F0111_SECOND =
    "ReDMCSB DUNVIEW.C F0111 lines 4325-4334";
static const char *A_F0122 =
    "ReDMCSB DUNVIEW.C F0122 lines 7391-7557";
static const char *A_F0123 =
    "ReDMCSB DUNVIEW.C F0123 lines 7559-7725";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C F0128 lines 8524-8542";
static const char *A_F0127 =
    "ReDMCSB DUNVIEW.C F0127 line 8294";
static const char *A_DEFS =
    "ReDMCSB DEFS.H lines 2088,2600-2601,2605-2606,4047-4048,4053-4054,4258,4260";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp lines 1903-1915";

static int g_assertions;

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

static int expect_true(const char *label, int got, const char *anchor)
{
    return expect_int(label, got ? 1 : 0, 1, anchor);
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_true(label, haystack && needle && strstr(haystack, needle),
                       anchor);
}

static int test_spec_identity(void)
{
    int ok = 1;
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *d1l2 =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(
            CSB_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1L2_PC34);
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *d1r2 =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(
            CSB_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_SIDE_D1R2_PC34);

    ok &= expect_int("spec.count",
                     (int)csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34(),
                     2, A_F0128);
    ok &= expect_true("spec.d1l2.present", d1l2 != 0, A_F0122);
    ok &= expect_true("spec.d1r2.present", d1r2 != 0, A_F0123);
    ok &= expect_true("spec.at0.d1l2",
                      csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_at_pc34(0) == d1l2,
                      A_F0128);
    ok &= expect_true("spec.at1.d1r2",
                      csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_at_pc34(1) == d1r2,
                      A_F0128);
    ok &= expect_true("spec.at2.null",
                      csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_at_pc34(2) == 0,
                      A_F0128);
    ok &= expect_true("spec.unknown_side.null",
                      csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(99) == 0,
                      "D1L2/D1R2 only route table");
    ok &= expect_true("spec.square4.d1l2",
                      csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_square_pc34(4) == d1l2,
                      "ReDMCSB DEFS.H line 2600 M607_VIEW_SQUARE_D1L");
    ok &= expect_true("spec.square5.d1r2",
                      csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_square_pc34(5) == d1r2,
                      "ReDMCSB DEFS.H line 2601 M608_VIEW_SQUARE_D1R");
    ok &= expect_true("spec.square9.excluded",
                      csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_square_pc34(9) == 0,
                      "non-duplicative with existing D2L2/D2R2 partly-open gate");
    ok &= expect_int("d1l2.contract_only",
                     d1l2 ? d1l2->source_locked_contract_only : -1, 1, A_F0111);
    ok &= expect_int("d1r2.no_game_data",
                     d1r2 ? d1r2->no_game_data_load : -1, 1,
                     "contract-only no CSB game-data load");
    ok &= expect_int("d1l2.no_pixel",
                     d1l2 ? d1l2->no_real_asset_pixel_parity : -1, 1,
                     "no real-asset pixel parity");

    return ok;
}

static int test_f0128_dispatch_and_followup(void)
{
    int ok = 1;
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *d1l2 =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_square_pc34(4);
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *d1r2 =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_square_pc34(5);

    ok &= expect_int("d1l2.function", d1l2 ? d1l2->f0122_f0123_function_number : -1,
                     122, A_F0122);
    ok &= expect_int("d1r2.function", d1r2 ? d1r2->f0122_f0123_function_number : -1,
                     123, A_F0123);
    ok &= expect_int("d1l2.dispatch_order",
                     d1l2 ? d1l2->f0128_dispatch_order : -1, 13, A_F0128);
    ok &= expect_int("d1r2.dispatch_order",
                     d1r2 ? d1r2->f0128_dispatch_order : -1, 14, A_F0128);
    ok &= expect_true("dispatch.left_before_right",
                      d1l2 && d1r2 &&
                          d1l2->f0128_dispatch_order < d1r2->f0128_dispatch_order,
                      A_F0128);
    ok &= expect_int("d1l2.depth", d1l2 ? d1l2->f0128_relative_depth : -1,
                     1, A_F0128);
    ok &= expect_int("d1r2.depth", d1r2 ? d1r2->f0128_relative_depth : -1,
                     1, A_F0128);
    ok &= expect_int("d1l2.lane", d1l2 ? d1l2->f0128_relative_lateral : 0,
                     -1, A_F0128);
    ok &= expect_int("d1r2.lane", d1r2 ? d1r2->f0128_relative_lateral : 0,
                     1, A_F0128);
    ok &= expect_int("follow.d1c", d1l2 ? d1l2->f0128_d1c_followup_order : -1,
                     15, "ReDMCSB DUNVIEW.C lines 8530-8533");
    ok &= expect_int("follow.d0l", d1l2 ? d1l2->f0128_d0l_followup_order : -1,
                     16, "ReDMCSB DUNVIEW.C lines 8534-8537");
    ok &= expect_int("follow.d0r", d1l2 ? d1l2->f0128_d0r_followup_order : -1,
                     17, "ReDMCSB DUNVIEW.C lines 8538-8541");
    ok &= expect_int("follow.f0127", d1l2 ? d1l2->f0127_followup_order : -1,
                     18, "ReDMCSB DUNVIEW.C line 8542");
    ok &= expect_int("follow.f0127.object_line",
                     d1r2 ? d1r2->f0127_object_pass_line : -1, 8294, A_F0127);

    return ok;
}

static int test_d1_body_bindings(void)
{
    int ok = 1;
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *d1l2 =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(1);
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *d1r2 =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(2);

    ok &= expect_int("d1l2.view_square", d1l2 ? d1l2->view_square : -1,
                     4, "ReDMCSB DEFS.H line 2600");
    ok &= expect_int("d1r2.view_square", d1r2 ? d1r2->view_square : -1,
                     5, "ReDMCSB DEFS.H line 2601");
    ok &= expect_int("d1l2.door_zone", d1l2 ? d1l2->door_zone_base : -1,
                     3780, "ReDMCSB DEFS.H line 4258 M630_ZONE_DOOR_D1L");
    ok &= expect_int("d1r2.door_zone", d1r2 ? d1r2->door_zone_base : -1,
                     3800, "ReDMCSB DEFS.H line 4260 M632_ZONE_DOOR_D1R");
    ok &= expect_int("d1l2.top_zone", d1l2 ? d1l2->door_frame_top_zone : -1,
                     732, "ReDMCSB DUNVIEW.C line 7503 C732");
    ok &= expect_int("d1r2.top_zone", d1r2 ? d1r2->door_frame_top_zone : -1,
                     734, "ReDMCSB DUNVIEW.C line 7671 C734");
    ok &= expect_int("d1l2.wall_zone", d1l2 ? d1l2->wall_zone : -1,
                     713, "ReDMCSB DEFS.H line 4053 C713_ZONE_WALL_D1L");
    ok &= expect_int("d1r2.wall_zone", d1r2 ? d1r2->wall_zone : -1,
                     714, "ReDMCSB DEFS.H line 4054 C714_ZONE_WALL_D1R");

    return ok;
}

static int test_f0111_state_frame_and_blit_math(void)
{
    int ok = 1;
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *d1l2 =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(1);
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *d1r2 =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(2);

    ok &= expect_int("branch.open",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(d1l2, 0),
                     CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_OPEN_PC34,
                     "ReDMCSB DUNVIEW.C line 4248");
    ok &= expect_int("branch.partly1",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(d1l2, 1),
                     CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34,
                     A_F0111);
    ok &= expect_int("branch.partly2",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(d1l2, 2),
                     CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34,
                     A_F0111);
    ok &= expect_int("branch.partly3",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(d1l2, 3),
                     CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34,
                     A_F0111);
    ok &= expect_int("branch.closed",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(d1l2, 4),
                     CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_CLOSED_PC34,
                     "ReDMCSB DUNVIEW.C lines 4297-4299");
    ok &= expect_int("branch.destroyed",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(d1l2, 5),
                     CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_DESTROYED_PC34,
                     "ReDMCSB DUNVIEW.C lines 4301-4304");
    ok &= expect_int("branch.invalid",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(d1l2, 6),
                     CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34, A_F0111);
    ok &= expect_int("branch.null",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(0, 2),
                     CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34, A_F0111);

    ok &= expect_contains("frame.left.d1l2",
                          csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(d1l2, 2, 0),
                          "D1L.LeftHorizontal", A_F0111_FRAME);
    ok &= expect_contains("frame.right.d1l2",
                          csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(d1l2, 2, 1),
                          "D1L.RightHorizontal", A_F0111_FRAME);
    ok &= expect_contains("frame.left.d1r2",
                          csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(d1r2, 2, 0),
                          "D1R.LeftHorizontal", A_F0111_FRAME);
    ok &= expect_contains("frame.right.d1r2",
                          csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(d1r2, 2, 1),
                          "D1R.RightHorizontal", A_F0111_FRAME);
    ok &= expect_true("frame.open.null",
                      csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(d1l2, 0, 0) == 0,
                      "ReDMCSB DUNVIEW.C line 4248");

    ok &= expect_int("d1l2.first.state1",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_first_half_zone_pc34(d1l2, 1, 1),
                     3787, A_F0111_FIRST);
    ok &= expect_int("d1l2.first.state2",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_first_half_zone_pc34(d1l2, 2, 1),
                     3788, A_F0111_FIRST);
    ok &= expect_int("d1r2.first.state2",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_first_half_zone_pc34(d1r2, 2, 1),
                     3808, A_F0111_FIRST);
    ok &= expect_int("first.vertical.rejected",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_first_half_zone_pc34(d1l2, 2, 0),
                     -1, A_F0111_FIRST);
    ok &= expect_int("first.open.rejected",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_first_half_zone_pc34(d1l2, 0, 1),
                     -1, "ReDMCSB DUNVIEW.C line 4248");

    ok &= expect_int("d1l2.second.state1",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_second_half_zone_pc34(d1l2, 1, 1),
                     20168, A_F0111_SECOND);
    ok &= expect_int("d1l2.second.state2",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_second_half_zone_pc34(d1l2, 2, 1),
                     20169, A_F0111_SECOND);
    ok &= expect_int("d1r2.second.state2",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_second_half_zone_pc34(d1r2, 2, 1),
                     20189, A_F0111_SECOND);
    ok &= expect_int("second.vertical.state2",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_second_half_zone_pc34(d1l2, 2, 0),
                     3782, "ReDMCSB DUNVIEW.C lines 4317-4318");
    ok &= expect_int("second.open.rejected",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_second_half_zone_pc34(d1l2, 0, 1),
                     -1, "ReDMCSB DUNVIEW.C line 4248");

    return ok;
}

static int test_synthetic_c10_blit(void)
{
    int ok = 1;
    uint8_t source[6] = { 10, 1, 2, 10, 3, 4 };
    uint8_t dest[6] = { 99, 99, 99, 99, 99, 99 };
    int skipped = -1;
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(1);

    ok &= expect_int("defs.c10.macro",
                     CSB_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_C10_COLOR_FLESH_PC34,
                     10, A_DEFS);
    ok &= expect_int("defs.mask.macro",
                     CSB_V1_D1L2_D1R2_F0111_PARTLY_OPEN_DOOR_MASK0X4000_PC34,
                     0x4000, A_DEFS);
    ok &= expect_int("blit.copied",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_synthetic_blit_pc34(
                         spec, 2, source, 3, 2, 3, dest, 3, 2, 3, &skipped),
                     4, A_F0111_SECOND);
    ok &= expect_int("blit.skipped", skipped, 2,
                     "ReDMCSB DEFS.H line 2088 C10_COLOR_FLESH");
    ok &= expect_int("blit.preserve.transparent0", dest[0], 99,
                     "C10 transparent source preserved");
    ok &= expect_int("blit.copy.pixel1", dest[1], 1, A_F0111_SECOND);
    ok &= expect_int("blit.open.skip",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_synthetic_blit_pc34(
                         spec, 0, source, 3, 2, 3, dest, 3, 2, 3, &skipped),
                     0, "ReDMCSB DUNVIEW.C line 4248");
    ok &= expect_int("blit.reject.bad_stride",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_synthetic_blit_pc34(
                         spec, 2, source, 3, 2, 2, dest, 3, 2, 3, 0),
                     -1, "synthetic source stride guard");
    ok &= expect_int("blit.reject.too_small_dest",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_synthetic_blit_pc34(
                         spec, 2, source, 3, 2, 3, dest, 2, 2, 2, 0),
                     -1, "synthetic destination bounds guard");

    return ok;
}

static int test_probe_and_evidence(void)
{
    int ok = 1;
    CSB_V1_D1L2D1R2F0111PartlyOpenDoorProbePc34 probe;
    const char *e =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_source_evidence_pc34();
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(1);

    ok &= expect_int("probe.run",
                     csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_probe_pc34_compat(&probe),
                     0, "synthetic PASS651 probe");
    ok &= expect_int("probe.route_count", probe.route_count, 2, A_F0128);
    ok &= expect_int("probe.dispatch", probe.dispatch_order_ok, 1, A_F0128);
    ok &= expect_int("probe.branch", probe.branch_state_ok, 1, A_F0111);
    ok &= expect_int("probe.frame", probe.frame_selection_ok, 1, A_F0111_FRAME);
    ok &= expect_int("probe.first_half", probe.first_half_zone, 3788, A_F0111_FIRST);
    ok &= expect_int("probe.second_half", probe.second_half_zone, 20169, A_F0111_SECOND);
    ok &= expect_int("probe.followup", probe.f0128_followup_ok, 1, A_F0128);
    ok &= expect_int("probe.copied", probe.copied_pixels, 4, A_F0111_SECOND);
    ok &= expect_int("probe.skipped", probe.c10_skipped_pixels, 2, A_DEFS);
    ok &= expect_int("probe.no_pixel", probe.no_real_asset_pixel_parity, 1,
                     "no real-asset pixel parity");
    ok &= expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4337", A_F0111);
    ok &= expect_contains("evidence.f0122", e, "DUNVIEW.C:7391-7557", A_F0122);
    ok &= expect_contains("evidence.f0123", e, "DUNVIEW.C:7559-7725", A_F0123);
    ok &= expect_contains("evidence.f0128", e, "DUNVIEW.C:8524-8542", A_F0128);
    ok &= expect_contains("evidence.f0127", e, "DUNVIEW.C:8294", A_F0127);
    ok &= expect_contains("evidence.defs_d2_baseline", e, "DEFS.H:2088,2605-2606,4047-4048", A_DEFS);
    ok &= expect_contains("evidence.lineage", e, "Viewport.cpp:1903-1915", A_LINEAGE);
    ok &= expect_contains("spec.f0111_anchor", spec ? spec->f0111_anchor : 0,
                          "4218-4337", A_F0111);
    ok &= expect_contains("spec.d1_anchor", spec ? spec->d1_body_anchor : 0,
                          "7391-7557", A_F0122);
    ok &= expect_contains("spec.f0128_anchor", spec ? spec->f0128_anchor : 0,
                          "8524-8542", A_F0128);
    ok &= expect_contains("spec.f0127_anchor", spec ? spec->f0127_anchor : 0,
                          "8294", A_F0127);

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_source_evidence_pc34());

    ok &= test_spec_identity();
    ok &= test_f0128_dispatch_and_followup();
    ok &= test_d1_body_bindings();
    ok &= test_f0111_state_frame_and_blit_math();
    ok &= test_synthetic_c10_blit();
    ok &= test_probe_and_evidence();
    ok &= expect_true("assertion_count_at_least_70", g_assertions >= 70,
                      "assigned PASS651 D1L2/D1R2 F0111 partly-open door gate");

    printf("assertions=%d\n", g_assertions);
    if (ok) {
        printf("PASS csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat assertions=%d\n",
               g_assertions);
    }
    return ok ? 0 : 1;
}
