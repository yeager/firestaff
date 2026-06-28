/*
 * Focused DM1 V1 D3C F0115 back-wall item thing-pass probe.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - DUNVIEW.C F0115:4547-4581 function signature, nibble structure,
 *   back/door/front two-pass split.
 * - DUNVIEW.C F0115:4794-4800 door-front pass nibble strip
 *   (MASK 0x0008_DOOR_FRONT, low bit-3) and the pass=bit0+1 rule.
 * - DUNVIEW.C F0115:4853-4860 view-square M600..M609 (D3C..D0C)
 *   thing-pass gate.
 * - DUNVIEW.C F0115:4920-4923 item visibility predicate; view_cell > 1
 *   at depth 3 means back cells visible, front cells clipped.
 * - DUNVIEW.C F0115:5180-5188 C10_COLOR_FLESH transparent blit.
 * - DUNVIEW.C:6723 D3C door-front F0115 call with
 *   C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT.
 * - DUNVIEW.C:6816 D3C corridor/pit/teleporter F0115 call with
 *   L0204_i_Order = C0x3421 (normal) and C0x0349 (door pass 2).
 * - DUNVIEW.C F0128:8499 D3C dispatch at relative depth 3/lateral 0.
 * - DEFS.H:2549 M550_FIRST_THING ordinal = 2.
 * - DEFS.H:2607 M600_VIEW_SQUARE_D3C = 11.
 * - DEFS.H:2642-2645 view cell numbering (0=front-left, 1=front-right,
 *   2=back-right, 3=back-left).
 * - DEFS.H:2669/2672/2676 cell-order constants.
 *
 * Asset-free, contract-only, no original DOS pixel parity claim.
 */

#include <stdio.h>
#include <string.h>

#include "firestaff/dm1/v1/viewport/d3c_back_wall_item_pc34_compat.h"

static int expect_int(const char *label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_contains(const char *label, const char *hay, const char *needle)
{
    if (!hay || !needle || !strstr(hay, needle)) {
        fprintf(stderr, "FAIL %s missing=%s\n", label, needle);
        return 0;
    }
    return 1;
}

static int run_wall_route_invariants(void)
{
    /* ReDMCSB DUNVIEW.C:6697-6720 D3C wall branch draws C704_WALL_D3C,
     * calls F0107 alcove, returns before F0115. The back-wall item is
     * not visible because the wall ornament / alcove takes the front.
     * Verified by re-running the library self-test and inspecting
     * wall_route_skips_f0115 + deterministic_hash stability. */
    const DM1_V1_D3CBackWallItemSelfTestResultPc34 *result =
        dm1_v1_viewport_d3c_back_wall_item_last_self_test_result_pc34();
    int ok = 1;
    if (!result) {
        fprintf(stderr, "FAIL wall_route no_result_pointer\n");
        return 0;
    }
    ok &= expect_int("wall_route_skips_f0115",
        result->wall_route_skips_f0115, 1);
    ok &= expect_int("wall_route_assertions_nonzero",
        result->assertions > 0 ? 1 : 0, 1);
    printf("wallRoute skipsF0115=%d assertions=%d hash=0x%08x "
           "source=DUNVIEW.C:6697-6720\n",
           result->wall_route_skips_f0115,
           result->assertions,
           (unsigned)result->deterministic_hash);
    return ok;
}

static int run_door_front_pass_invariants(void)
{
    /* ReDMCSB DUNVIEW.C:6723 / :6816 D3C door-front F0115 call pair:
     * pass 1 uses C0x0218 (cells 3,2 = back-left, back-right ordered
     * as 0,1 after the strip), pass 2 uses C0x0349. The library
     * self-test reports f0115_call_count and back_wall_item_zones_seen
     * to confirm both passes were taken. */
    const DM1_V1_D3CBackWallItemSelfTestResultPc34 *result =
        dm1_v1_viewport_d3c_back_wall_item_last_self_test_result_pc34();
    int ok = 1;
    if (!result) {
        fprintf(stderr, "FAIL door_front_pass no_result_pointer\n");
        return 0;
    }
    ok &= expect_int("door_front_pass_f0115_call_count",
        result->f0115_call_count, 2);
    ok &= expect_int("door_front_pass_zones_seen",
        result->back_wall_item_zones_seen, 2);
    printf("doorFrontPass f0115Calls=%d backWallZones=%d hash=0x%08x "
           "source=DUNVIEW.C:6723,6816\n",
           result->f0115_call_count,
           result->back_wall_item_zones_seen,
           (unsigned)result->deterministic_hash);
    return ok;
}

static int run_corridor_pit_teleporter_invariants(void)
{
    /* ReDMCSB DUNVIEW.C:6816 L0204_i_Order = C0x3421 encodes all 4
     * cells in nibble order; at D3C depth 3 the back cells (3,2) are
     * visible, the front cells (0,1) are clipped. The library
     * self-test exposes corridor_pit_teleporter_back_then_front +
     * back_cells_visible_at_d3c + front_cells_clipped_at_d3c. */
    const DM1_V1_D3CBackWallItemSelfTestResultPc34 *result =
        dm1_v1_viewport_d3c_back_wall_item_last_self_test_result_pc34();
    int ok = 1;
    if (!result) {
        fprintf(stderr, "FAIL corridor_pit_teleporter no_result_pointer\n");
        return 0;
    }
    ok &= expect_int("corridor_pit_teleporter_back_then_front",
        result->corridor_pit_teleporter_back_then_front, 1);
    ok &= expect_int("back_cells_visible_at_d3c",
        result->back_cells_visible_at_d3c, 1);
    ok &= expect_int("front_cells_clipped_at_d3c",
        result->front_cells_clipped_at_d3c, 1);
    printf("corridorPitTeleporter backThenFront=%d backVisible=%d "
           "frontClipped=%d c10Skips=%d hash=0x%08x "
           "source=DUNVIEW.C:6816 F0115:4920-4923\n",
           result->corridor_pit_teleporter_back_then_front,
           result->back_cells_visible_at_d3c,
           result->front_cells_clipped_at_d3c,
           result->c10_transparent_skip,
           (unsigned)result->deterministic_hash);
    return ok;
}

static int run_source_evidence_invariants(void)
{
    const char *evidence =
        dm1_v1_viewport_d3c_back_wall_item_source_evidence_pc34();
    const char *disjoint =
        dm1_v1_viewport_d3c_back_wall_item_disjointness_note_pc34();
    int ok = 1;

    /* Source evidence must cite every ReDMCSB anchor we depend on. */
    ok &= expect_contains("src_F0115_signature", evidence,
        "DUNVIEW.C F0115:4547-4581");
    ok &= expect_contains("src_door_pass_strip", evidence,
        "DUNVIEW.C F0115:4794-4800");
    ok &= expect_contains("src_view_square_range", evidence,
        "DUNVIEW.C F0115:4853-4860");
    ok &= expect_contains("src_visibility_predicate", evidence,
        "DUNVIEW.C F0115:4920-4923");
    ok &= expect_contains("src_c10_blend", evidence,
        "DUNVIEW.C F0115:5180-5188");
    ok &= expect_contains("src_d3c_door_pass1_call", evidence,
        "DUNVIEW.C:6723");
    ok &= expect_contains("src_d3c_corridor_call", evidence,
        "DUNVIEW.C:6816");
    ok &= expect_contains("src_d3c_dispatch", evidence,
        "DUNVIEW.C F0128:8499");
    ok &= expect_contains("src_defs_first_thing", evidence,
        "M550_FIRST_THING");
    ok &= expect_contains("src_defs_d3c_view_square", evidence,
        "M600_VIEW_SQUARE_D3C = 11");
    ok &= expect_contains("src_no_original_parity_claim", evidence,
        "no original DOS pixel parity");
    ok &= expect_contains("src_c10_color_flesh", evidence,
        "C10_COLOR_FLESH");

    /* Disjointness must mention every sibling gate we are
     * deliberately NOT re-covering. */
    ok &= expect_contains("disjoint_d3c_f0107", disjoint,
        "D3C F0107 wall-ornament");
    ok &= expect_contains("disjoint_d3c_f0108", disjoint,
        "D3C F0108 floor");
    ok &= expect_contains("disjoint_d3c_f0111", disjoint,
        "D3C F0111 door-front");
    ok &= expect_contains("disjoint_d3l2_d3r2_f0115", disjoint,
        "D3L2/D3R2 F0115");
    ok &= expect_contains("disjoint_d1l2_d1r2_f0115", disjoint,
        "D1L2/D1R2 F0115");
    ok &= expect_contains("disjoint_d0l2_d0r2_f0115", disjoint,
        "D0L2/D0R2 F0115");
    ok &= expect_contains("disjoint_d1c_f0115", disjoint,
        "D1C F0115");
    ok &= expect_contains("disjoint_alcove_helper", disjoint,
        "F0107 alcove helper");
    ok &= expect_contains("disjoint_projectile_metadata", disjoint,
        "F0115 projectile metadata");
    ok &= expect_contains("disjoint_f0128_dispatch_order", disjoint,
        "F0128 dispatch order");

    return ok;
}

int main(void)
{
    int ok = 1;
    int lib_ok;
    uint32_t hash_before = 0;
    uint32_t hash_after = 0;
    const DM1_V1_D3CBackWallItemSelfTestResultPc34 *result;

    printf("probe=firestaff_dm1_v1_viewport_d3c_back_wall_item_gate_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("sourceEvidence="
           "DUNVIEW.C F0115:4547-4581,4794-4800,4853-4860,4920-4923,5180-5188; "
           "DUNVIEW.C:6723,6816,8499; "
           "DEFS.H:2549,2607,2642-2645,2669,2672,2676\n");

    lib_ok = run_dm1_v1_viewport_d3c_back_wall_item_self_test_pc34();
    ok &= expect_int("library_self_test_ok", lib_ok, 1);
    result = dm1_v1_viewport_d3c_back_wall_item_last_self_test_result_pc34();
    if (result) hash_before = result->deterministic_hash;

    /* Re-run for stability across separate invocations of the
     * self-test: the deterministic_hash must be byte-identical. */
    (void)run_dm1_v1_viewport_d3c_back_wall_item_self_test_pc34();
    result = dm1_v1_viewport_d3c_back_wall_item_last_self_test_result_pc34();
    if (result) hash_after = result->deterministic_hash;
    ok &= expect_int("deterministic_hash_stable",
        (int)(hash_before == hash_after && hash_after != 0), 1);
    ok &= expect_int("self_test_failures_zero",
        result && result->failures == 0 ? 1 : 0, 1);
    ok &= expect_int("self_test_assertions_nonzero",
        result && result->assertions > 0 ? 1 : 0, 1);

    ok &= run_wall_route_invariants();
    ok &= run_door_front_pass_invariants();
    ok &= run_corridor_pit_teleporter_invariants();
    ok &= run_source_evidence_invariants();

    if (!ok) {
        fprintf(stderr, "probe failed hash=0x%08x\n",
                (unsigned)(result ? result->deterministic_hash : 0U));
        return 1;
    }
    printf("result=pass assertions=%d failures=0 hash=0x%08x\n",
           result ? result->assertions : 0,
           (unsigned)(result ? result->deterministic_hash : 0U));
    return 0;
}
