#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_pc34_compat.h"

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
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)",
               anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void test_model_core(void)
{
    DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionModelPc34 built;
    const DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionModelPc34 *model =
        dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_default_model_pc34();

    expect_int("builder.null",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_default_model_builder_pc34(
                   NULL),
               0, "defensive builder guard");
    expect_int("builder.ok",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_default_model_builder_pc34(
                   &built),
               1, "deterministic default model builder");
    expect_int("model.present", model != NULL, 1, "default model accessor");
    if (!model) return;
    expect_int("model.hash_matches_built",
               built.deterministic_hash == model->deterministic_hash, 1,
               "default model builder is deterministic");
    expect_int("hash_model.null",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_hash_model_pc34(
                   NULL),
               0, "hash_model null guard");
    expect_int("hash_model.matches",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_hash_model_pc34(
                   model) == model->deterministic_hash,
               1, "hash_model stable");
    expect_int("hash_accessor.matches",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_deterministic_hash_pc34() ==
                   model->deterministic_hash,
               1, "deterministic_hash accessor stable");

    expect_int("view_square.d3l", model->view_square_d3l, 12,
               "DEFS.H:2608 M601_VIEW_SQUARE_D3L=12");
    expect_int("view_square.d3r", model->view_square_d3r, 13,
               "DEFS.H:2609 M602_VIEW_SQUARE_D3R=13");
    expect_int("view_floor.d3l", model->view_floor_d3l, 2,
               "DEFS.H:2752 M588_VIEW_FLOOR_D3L=2");
    expect_int("view_floor.d3r", model->view_floor_d3r, 4,
               "DEFS.H:2754 M590_VIEW_FLOOR_D3R=4");
    expect_int("wall_zone.d3l", model->wall_zone_d3l, 705,
               "DEFS.H:4045 C705_ZONE_WALL_D3L");
    expect_int("wall_zone.d3r", model->wall_zone_d3r, 706,
               "DEFS.H:4046 C706_ZONE_WALL_D3R");
    expect_int("transparent.c10", model->c10_transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("slot.m558", model->floor_ornament_ordinal_slot, 5,
               "DEFS.H:2558 M558_FLOOR_ORNAMENT_ORDINAL");
    expect_int("slot.m550", model->first_thing_slot, 2,
               "DEFS.H:2549 M550_FIRST_THING");
    expect_int("slot.m554", model->pit_or_teleporter_visible_slot, 3,
               "DEFS.H:2554 M554_PIT_OR_TELEPORTER_VISIBLE");
    expect_int("slot.m555", model->stairs_up_slot, 3,
               "DEFS.H:2555 M555_STAIRS_UP");
    expect_int("slot.m556", model->door_state_slot, 3,
               "DEFS.H:2556 M556_DOOR_STATE");
    expect_int("slot.m557", model->door_thing_index_slot, 4,
               "DEFS.H:2557 M557_DOOR_THING_INDEX");
    expect_int("dispatch.after_d3c", model->f0128_dispatches_after_d3c, 1,
               "DUNVIEW.C F0128:8318-8542 F0116/F0117 after D3C");
    expect_int("dispatch.f0116_before_f0117",
               model->f0116_dispatches_before_f0117, 1,
               "DUNVIEW.C F0128 D3L before D3R");
    expect_int("dispatch.f0117_before_d2l",
               model->f0117_dispatches_before_d2l, 1,
               "DUNVIEW.C F0128 D3R before D2L/D2R");

    expect_int("f0116.door_front.f0108_with_558",
               model->f0116_door_front_calls_f0108_with_558, 1,
               "DUNVIEW.C F0116:6443 F0108 with M558 + M588_VIEW_FLOOR_D3L");
    expect_int("f0116.door_front.f0115_doorpass1",
               model->f0116_door_front_calls_f0115_doorpass1, 1,
               "DUNVIEW.C F0116:6444 F0115 with C0x0218_DOORPASS1");
    expect_int("f0116.door_front.drops_to_doorpass2",
               model->f0116_door_front_drops_to_doorpass2, 1,
               "DUNVIEW.C F0116:6443 C17 falls through to T0116017 with C0x0349");
    expect_int("f0116.wall_branch_alcove",
               model->f0116_wall_branch_returns_via_alcove, 1,
               "DUNVIEW.C F0116:6433-6437 C00 walls return via F0107 alcove");
    expect_int("f0116.stairs_front_f0104_first",
               model->f0116_stairs_front_calls_f0104_first, 1,
               "DUNVIEW.C F0116:6376-6404 C19 stairs-front via F0104");
    expect_int("f0116.corridor.cell_order",
               model->f0116_corridor_cell_order_d3l, 0x3421u,
               "DEFS.H:2676 C0x3421_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT");
    expect_int("f0116.side.cell_order",
               model->f0116_side_cell_order_d3l, 0x0321u,
               "DEFS.H:2670 C0x0321_SIDE");
    expect_int("f0116.doorpass2.cell_order",
               model->f0116_doorpass2_cell_order_d3l, 0x0349u,
               "DEFS.H:2672 C0x0349_DOORPASS2");

    expect_int("f0117.door_front.f0108_with_558",
               model->f0117_door_front_calls_f0108_with_558, 1,
               "DUNVIEW.C F0117:6579 F0108 with M558 + M590_VIEW_FLOOR_D3R");
    expect_int("f0117.door_front.f0115_doorpass1",
               model->f0117_door_front_calls_f0115_doorpass1, 1,
               "DUNVIEW.C F0117:6580 F0115 with C0x0128_DOORPASS1_BRBL");
    expect_int("f0117.door_front.drops_to_doorpass2",
               model->f0117_door_front_drops_to_doorpass2, 1,
               "DUNVIEW.C F0117:6579 C17 falls through to T0117018 with C0x0439");
    expect_int("f0117.wall_branch_alcove",
               model->f0117_wall_branch_returns_via_alcove, 1,
               "DUNVIEW.C F0117:6568-6576 C00 walls return via F0107 alcove");
    expect_int("f0117.stairs_front_f0104_first",
               model->f0117_stairs_front_calls_f0104_first, 1,
               "DUNVIEW.C F0117:6515-6545 C19 stairs-front via F0105 flip");
    expect_int("f0117.corridor.cell_order",
               model->f0117_corridor_cell_order_d3r, 0x4312u,
               "DEFS.H:2677 C0x4312_BACKRIGHT_BACKLEFT_FRONTRIGHT_FRONTLEFT");
    expect_int("f0117.side.cell_order",
               model->f0117_side_cell_order_d3r, 0x0412u,
               "DEFS.H:2671 C0x0412_SIDE_BRBL_FL");
    expect_int("f0117.doorpass2.cell_order",
               model->f0117_doorpass2_cell_order_d3r, 0x0439u,
               "DEFS.H:2673 C0x0439_DOORPASS2");

    expect_int("f0108.ordinal_zero_skips_blit",
               model->f0108_ordinal_zero_skips_blit, 1,
               "DUNVIEW.C F0108:3945-3948 ordinal=0 short-circuit");
    expect_int("f0108.footprint_mask_recurses",
               model->f0108_footprint_mask_recurses, 1,
               "DUNVIEW.C F0108:4008 T0108005 footprint recursion");
    expect_int("f0108.footprint_only_skips_primary",
               model->f0108_footprint_only_skips_primary, 1,
               "DUNVIEW.C F0108:4008 fp_only-no-primary branch");
    expect_int("f0108.blit_uses_c10",
               model->f0108_blit_uses_c10_transparent, 1,
               "DUNVIEW.C F0108:3988-3993 F0791 C10 transparent blit");
    expect_int("f0108.zone_uses_11_stride",
               model->f0108_zone_uses_11_stride, 1,
               "DEFS.H:4223 PC 3.4 CoordinateSet*11 floor zone stride");
    expect_int("f0108.zone.d3l", model->f0108_zone_d3l, 1502,
               "C1500 + CoordinateSet*11 + M588_VIEW_FLOOR_D3L=2");
    expect_int("f0108.zone.d3r", model->f0108_zone_d3r, 1504,
               "C1500 + CoordinateSet*11 + M590_VIEW_FLOOR_D3R=4");
    expect_int("bug0_64.guard_absent", model->bug0_64_occlusion_guard, 0,
               "DUNVIEW.C F0116:6478 + F0117:6620 BUG0_64 source comment");
    expect_int("no.graphics_dat_reads", model->no_graphics_dat_reads, 1,
               "asset-free source-locked contract");
    expect_int("source.locked.contract_only",
               model->source_locked_contract_only, 1,
               "no original-DOS or real-asset pixel-parity claim");
    expect_int("no.real_asset_bitmap_parity",
               model->no_real_asset_bitmap_parity, 1,
               "honest non-claim");
    expect_int("footprint_index", model->footprint_index, 15,
               "DUNVIEW.C F0108:4008 C15_FLOOR_ORNAMENT_FOOTPRINTS");
}

static void test_decoders_and_blend(void)
{
    bool fp_set;
    unsigned int cleared;
    bool primary_draws;
    bool recurse_fp;
    int primary_index;
    int recurse_index;

    fp_set = false;
    cleared = 0xffffffffu;
    primary_draws = true;
    recurse_fp = true;
    primary_index = -1;
    recurse_index = -1;
    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        7u, &fp_set, &cleared, &primary_draws, &primary_index, &recurse_fp,
        &recurse_index);
    expect_int("decode.simple.primary_draws", primary_draws ? 1 : 0, 1,
               "DUNVIEW.C F0108:3949-3964 simple primary draw");
    expect_int("decode.simple.cleared", (int)cleared, 7,
               "mask clears without footprint");
    expect_int("decode.simple.no_recursion", recurse_fp ? 1 : 0, 0,
               "primary alone does not recurse");
    expect_int("decode.simple.primary_index", primary_index, 6,
               "ordinal-1 -> index 6");

    fp_set = false;
    cleared = 0xffffffffu;
    primary_draws = true;
    recurse_fp = false;
    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        0x8000u, &fp_set, &cleared, &primary_draws, NULL, NULL, NULL);
    expect_int("decode.fp_only.fp_set", fp_set ? 1 : 0, 1,
               "0x8000 sets footprint flag");
    expect_int("decode.fp_only.no_primary", primary_draws ? 1 : 0, 0,
               "fp_only skips primary");
    expect_int("decode.fp_only.cleared", (int)cleared, 0,
               "fp_only cleared ordinal is 0");

    fp_set = false;
    cleared = 0u;
    primary_draws = false;
    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        0u, &fp_set, &cleared, &primary_draws, NULL, NULL, NULL);
    expect_int("decode.zero.no_fp", fp_set ? 1 : 0, 0,
               "ordinal=0 short-circuits");
    expect_int("decode.zero.cleared", (int)cleared, 0,
               "ordinal=0 returns 0 cleared");
    expect_int("decode.zero.no_primary", primary_draws ? 1 : 0, 0,
               "ordinal=0 skips primary");

    expect_int("blend.c10_preserves_dest",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_blend_c10_pc34(
                   0xaau, 10u),
               0xaau, "C10_COLOR_FLESH passes through destination");
    expect_int("blend.opaque_overwrites",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_blend_c10_pc34(
                   0xaau, 0x52u),
               0x52u, "non-C10 source pixel wins");
}

static void test_context_occlusion(void)
{
    int d3l_paths = 0;
    int d3r_paths = 0;

    /* D3L covers all 7 contexts (corridor, open-pit, teleporter,
     * stairs-side, door-front, stairs-front, door-side). BUG0_64
     * applies to all shared-tail contexts. */
    d3l_paths =
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3L_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_CORRIDOR_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3L_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_OPEN_PIT_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3L_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_TELEPORTER_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3L_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_STAIRS_SIDE_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3L_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_DOOR_FRONT_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3L_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_STAIRS_FRONT_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3L_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_DOOR_SIDE_PC34, 5u) ? 1 : 0);
    expect_int("context.d3l.all_paths", d3l_paths, 7,
               "BUG0_64 F0108 occludes all 7 contexts on D3L");

    d3r_paths =
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3R_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_CORRIDOR_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3R_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_OPEN_PIT_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3R_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_TELEPORTER_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3R_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_STAIRS_SIDE_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3R_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_DOOR_FRONT_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3R_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_STAIRS_FRONT_PC34, 5u) ? 1 : 0) +
        (dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
             DM1_V1_D3L_D3R_FOCCL_SIDE_D3R_PC34,
             DM1_V1_D3L_D3R_FOCCL_CONTEXT_DOOR_SIDE_PC34, 5u) ? 1 : 0);
    expect_int("context.d3r.all_paths", d3r_paths, 7,
               "BUG0_64 F0108 occludes all 7 contexts on D3R");

    expect_int("context.zero_ordinal_no_occlusion",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D3L_D3R_FOCCL_SIDE_D3L_PC34,
                   DM1_V1_D3L_D3R_FOCCL_CONTEXT_OPEN_PIT_PC34, 0u),
               0, "ordinal=0 short-circuit on D3L pit");
    expect_int("context.unknown_side_rejected",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   (DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionSidePc34)99,
                   DM1_V1_D3L_D3R_FOCCL_CONTEXT_OPEN_PIT_PC34, 5u),
               0, "unknown side rejected");
    expect_int("context.unknown_context_rejected",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D3L_D3R_FOCCL_SIDE_D3L_PC34,
                   (DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionContextPc34)99, 5u),
               0, "unknown context rejected");
}

static void test_zones(void)
{
    expect_int("zone.d3l.cs0",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_zone_d3l_pc34(
                   0, 0),
               1502, "1500 + 0*11 + 2");
    expect_int("zone.d3l.cs1",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_zone_d3l_pc34(
                   1, 0),
               1513, "1500 + 1*11 + 2");
    expect_int("zone.d3l.negative_inputs",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_zone_d3l_pc34(
                   -1, -1),
               1502, "negative coordinates clamp to zero");

    expect_int("zone.d3r.cs0",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_zone_d3r_pc34(
                   0, 0),
               1504, "1500 + 0*11 + 4");
    expect_int("zone.d3r.cs3",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_zone_d3r_pc34(
                   3, 0),
               1537, "1500 + 3*11 + 4");
    expect_int("zone.d3r.negative_inputs",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_zone_d3r_pc34(
                   -1, -1),
               1504, "negative coordinates clamp to zero");
}

static void test_steps(void)
{
    unsigned int n;
    unsigned int i;
    int bug0_64_count = 0;

    n = dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_count_pc34();
    expect_int("step.count", (int)n, 10,
               "10 source-locked dispatch steps cover dispatch + door-front + shared-tail + ordinal + footprint + C10 + BUG0_64");
    expect_int("step.out_of_range_null",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_step_at_pc34(
                   n) == NULL ? 1 : 0,
               1, "out-of-range step returns NULL");

    for (i = 0; i < n; ++i) {
        const DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionStepPc34 *step =
            dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_step_at_pc34(i);
        char id[64];
        if (!step) continue;
        snprintf(id, sizeof(id), "step.%u.order_index", i);
        expect_int(id, step->order_index, (int)i, "step order matches index");
        if (step->bug0_64_occlusion_present) ++bug0_64_count;
        if (step->redmcsb_anchor) {
            snprintf(id, sizeof(id), "step.%u.dunview_anchor", i);
            expect_contains(id, step->redmcsb_anchor, "DUNVIEW.C",
                            "every step cites a DUNVIEW.C line range");
        }
    }
    expect_int("step.bug0_64_count", bug0_64_count, 5,
               "5 BUG0_64 markers: F0116 + F0117 dispatch + door-front plus BUG0_64 steps");
}

static void test_self_test_and_evidence(void)
{
    const DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionSelfTestResultPc34 *result;

    expect_int("self_test.runs",
               dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_self_test_pc34(),
               1, "internal self_test returns ok=1");
    result = dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_last_self_test_result_pc34();
    expect_int("self_test.result_present", result != NULL, 1,
               "last_self_test_result accessor populated");
    if (!result) return;
    expect_int("self_test.no_failures", result->failures, 0,
               "self_test runs with 0 failures");
    expect_int("self_test.steps_count", result->step_count_ten, 1,
               "self_test step_count_ten=1");
    expect_int("self_test.bug0_64_count", result->bug0_64_marker_count, 5,
               "self_test bug0_64_marker_count=5");
    expect_int("self_test.f0116_door_front",
               result->f0116_door_front_branch_present, 1,
               "self_test F0116 door-front branch detected");
    expect_int("self_test.f0117_door_front",
               result->f0117_door_front_branch_present, 1,
               "self_test F0117 door-front branch detected");
    expect_int("self_test.hash_nonzero", result->deterministic_hash != 0, 1,
               "self_test deterministic_hash != 0");

    expect_contains("evidence.F0116",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "F0116:6361-6499",
                    "F0116 DUNVIEW.C body covered");
    expect_contains("evidence.F0117",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "F0117:6500-6641",
                    "F0117 DUNVIEW.C body covered");
    expect_contains("evidence.F0108",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "F0108:3940-4011",
                    "F0108 floor-ornament ordinal handler covered");
    expect_contains("evidence.BUG0_64",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "BUG0_64",
                    "BUG0_64 open-pit overdraw comment surfaced");
    expect_contains("evidence.C1500",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "C1500",
                    "PC 3.4 C1500 floor-ornament zone base covered");
    expect_contains("evidence.c10",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "C10_COLOR_FLESH",
                    "C10_COLOR_FLESH transparency covered");
    expect_contains("evidence.M558",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "M558",
                    "M558_FLOOR_ORNAMENT_ORDINAL slot covered");
    expect_contains("evidence.F0128",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "F0128",
                    "F0128 dispatch hook covered");
    expect_contains("evidence.M601",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "M601_VIEW_SQUARE_D3L",
                    "M601_VIEW_SQUARE_D3L ordinal covered");
    expect_contains("evidence.M602",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "M602_VIEW_SQUARE_D3R",
                    "M602_VIEW_SQUARE_D3R ordinal covered");

    expect_contains("disjoint.D1C",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_disjointness_note_pc34(),
                    "D1C F0108",
                    "disjoint from D1C F0108 floor-ornament occlusion sibling");
    expect_contains("disjoint.D3L2_D3R2",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_disjointness_note_pc34(),
                    "D3L2/D3R2",
                    "disjoint note names the D3L2/D3R2 lanes");
    expect_contains("disjoint.GRAPHICS_DAT",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_disjointness_note_pc34(),
                    "GRAPHICS.DAT",
                    "asset-free claim surfaces in disjoint note");
    expect_contains("disjoint.cross_game",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_disjointness_note_pc34(),
                    "CSB/Nexus/Theron/DM2",
                    "cross-game disjointness surfaces");
    expect_contains("disjoint.F0116_F0117",
                    dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_disjointness_note_pc34(),
                    "F0116",
                    "F0116/F0117 siblings separated");
}

int main(void)
{
    printf("probe=test_dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_pc34_compat\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceLocks=F0116:6361-6499,F0117:6500-6641,F0108:3940-4011,F0128:8318-8542\n");
    printf("claim=synthetic source-locked gate for D3L/D3R BUG0_64 F0108 floor-ornament occlusion\n");
    printf("honestScope=data-free contract-only, no GRAPHICS.DAT reads, no original-DOS parity claim\n");

    test_model_core();
    test_decoders_and_blend();
    test_context_occlusion();
    test_zones();
    test_steps();
    test_self_test_and_evidence();

    if (g_failures) {
        printf("result=fail assertions=%d failures=%d\n", g_assertions,
               g_failures);
        return 1;
    }
    printf("result=pass assertions=%d\n", g_assertions);
    return 0;
}
