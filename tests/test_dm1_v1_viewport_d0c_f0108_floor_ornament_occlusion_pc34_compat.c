#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_pc34_compat.h"

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
    DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 built;
    const DM1_V1_D0CF0108FloorOrnamentOcclusionModelPc34 *model =
        dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_default_model_pc34();

    expect_int("builder.null",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
                   NULL) ? 1 : 0,
               0, "defensive builder guard");
    expect_int("builder.ok",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
                   &built) ? 1 : 0,
               1, "deterministic default model builder");
    expect_int("model.present", model != NULL, 1, "default model accessor");
    if (!model) return;
    expect_int("model.hash_matches_built",
               built.deterministic_hash == model->deterministic_hash, 1,
               "default model builder is deterministic");
    expect_int("hash_model.null",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_hash_model_pc34(
                   NULL),
               0, "hash_model null guard");
    expect_int("hash_model.matches",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_hash_model_pc34(
                   model) == model->deterministic_hash,
               1, "hash_model stable");
    expect_int("hash_accessor.matches",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_deterministic_hash_pc34() ==
                   model->deterministic_hash,
               1, "deterministic_hash accessor stable");

    expect_int("view_square.d0c", model->view_square_d0c, 0,
               "DEFS.H M609_VIEW_SQUARE_D0C=0");
    expect_int("view_floor.d0c", model->view_floor_d0c, 9,
               "DEFS.H M603_VIEW_FLOOR_D0C=9");
    expect_int("wall_zone.d0c", model->wall_zone_d0c, 715,
               "DEFS.H C715_ZONE_WALL_D0C");
    expect_int("transparent.c10", model->c10_transparent_color, 10,
               "DEFS.H C10_COLOR_FLESH");
    expect_int("slot.m550", model->first_thing_slot, 2,
               "DEFS.H M550_FIRST_THING");
    expect_int("slot.m554", model->pit_or_teleporter_visible_slot, 3,
               "DEFS.H M554_PIT_OR_TELEPORTER_VISIBLE");
    expect_int("slot.m555", model->stairs_up_slot, 4,
               "DEFS.H M555_STAIRS_UP");
    expect_int("slot.m558", model->floor_ornament_ordinal_slot, 5,
               "DEFS.H M558_FLOOR_ORNAMENT_ORDINAL");
    expect_int("dispatch.order.d0c", model->f0128_dispatch_order_d0c, 17,
               "DUNVIEW.C F0128 D0C after D0L/D0R");
    expect_int("dispatch.after_d0l", model->f0128_dispatch_after_d0l, 1,
               "DUNVIEW.C F0128 D0L before D0C");
    expect_int("dispatch.after_d0r", model->f0128_dispatch_after_d0r, 1,
               "DUNVIEW.C F0128 D0R before D0C");
    expect_int("dispatch.last", model->f0128_dispatch_last_in_sweep, 1,
               "DUNVIEW.C:8542 F0127 D0C is final square call");
    expect_int("door_side.f0100", model->f0127_door_side_calls_f0100, 1,
               "DUNVIEW.C:8188-8210 C16 door-side wallset path");
    expect_int("door_side.f0104", model->f0127_door_side_calls_f0104, 1,
               "DUNVIEW.C:8188-8210 C16 door-side bitmap path");
    expect_int("stairs.f0104", model->f0127_stairs_front_calls_f0104, 1,
               "DUNVIEW.C:8221-8254 stairs-front left lane");
    expect_int("stairs.f0105", model->f0127_stairs_front_calls_f0105, 1,
               "DUNVIEW.C:8221-8254 stairs-front right lane");
    expect_int("pit.f0104", model->f0127_open_pit_calls_f0104, 1,
               "DUNVIEW.C:8265-8275 C02 pit bitmap");
    expect_int("pit.visible_slot", model->f0127_open_pit_uses_pit_visible_slot, 1,
               "DUNVIEW.C:8265 M554 gate");
    expect_int("teleporter.f0113", model->f0127_teleporter_calls_f0113_field, 1,
               "DUNVIEW.C:8302-8308 teleporter field");
    expect_int("tail.f0112", model->f0127_calls_f0112_ceiling_pit, 1,
               "DUNVIEW.C:8284-8292 F0112");
    expect_int("tail.f0115", model->f0127_calls_f0115_thing_pass, 1,
               "DUNVIEW.C:8294 F0115");
    expect_int("tail.cell_order", model->f0127_d0c_cell_order, 0x0021,
               "DEFS.H C0x0021_BACKLEFT_BACKRIGHT");
    expect_int("d0c.no_f0108", model->f0127_d0c_dispatch_no_f0108, 1,
               "DUNVIEW.C F0127 has no F0108 call");
    expect_int("base.before_f0127", model->f0098_base_writes_before_f0127, 1,
               "DUNVIEW.C F0128 F0098 before F0127");
    expect_int("zone.d0c", model->f0108_zone_d0c, 1520,
               "C1500 + 1*11 + 9 PC 3.4 zone math");
    expect_int("bug0_64.inapplicable", model->bug0_64_inapplicable_to_d0c, 1,
               "no F0108 call site on D0C dispatch");
    expect_int("f0112.graphic", model->f0112_ceiling_pit_graphic, 69,
               "DEFS.H C069_GRAPHIC_CEILING_PIT_D0C");
    expect_int("f0112.zone", model->f0112_ceiling_pit_zone_d0c, 871,
               "DEFS.H C871_ZONE_CEILING_PIT_D0C");
    expect_int("no_graphics_dat", model->no_graphics_dat_reads, 1,
               "asset-free fixture");
    expect_int("contract_only", model->source_locked_contract_only, 1,
               "source-lock contract only");
    expect_int("no_real_asset_parity", model->no_real_asset_bitmap_parity, 1,
               "no original DOS pixel parity claim");
    expect_int("call_site_absent", model->f0108_d0c_call_site_absent, 1,
               "DUNVIEW.C F0127 contains no F0108 call");
}

static void test_steps(void)
{
    unsigned int n = dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_context_count_pc34();
    unsigned int f0104_count = 0;
    unsigned int f0105_count = 0;
    unsigned int f0108_count = 0;
    unsigned int f0112_count = 0;
    unsigned int f0113_count = 0;
    unsigned int f0115_count = 0;
    unsigned int no_f0108_count = 0;
    unsigned int i;

    expect_int("step.count", (int)n, 9,
               "D0C base/branches/F0112/F0115/F0108-absent sequence");
    expect_int("step.out_of_range",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_step_at_pc34(
                   (size_t)n) == NULL,
               1, "step accessor bounds");
    for (i = 0; i < n; ++i) {
        const DM1_V1_D0CF0108FloorOrnamentOcclusionStepPc34 *step =
            dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_step_at_pc34(
                (size_t)i);
        char id[64];
        snprintf(id, sizeof(id), "step.%u.present", i);
        expect_int(id, step != NULL, 1, "step accessor");
        if (!step) continue;
        f0104_count += step->calls_f0104_floor_pit_stairs ? 1u : 0u;
        f0105_count += step->calls_f0105_floor_pit_stairs_flipped ? 1u : 0u;
        f0108_count += step->calls_f0108_floor_ornament ? 1u : 0u;
        f0112_count += step->calls_f0112_ceiling_pit ? 1u : 0u;
        f0113_count += step->calls_f0113_field ? 1u : 0u;
        f0115_count += step->calls_f0115_thing_pass ? 1u : 0u;
        no_f0108_count += step->expected_d0c_no_f0108_contract ? 1u : 0u;
        snprintf(id, sizeof(id), "step.%u.order", i);
        expect_int(id, step->order_index, (int)i,
                   "step order_index matches position");
        snprintf(id, sizeof(id), "step.%u.anchor", i);
        expect_contains(id, step->redmcsb_anchor, "DUNVIEW.C", "step anchor");
        snprintf(id, sizeof(id), "step.%u.view_square", i);
        expect_int(id, step->expected_view_square, 0,
                   "M609_VIEW_SQUARE_D0C");
    }
    expect_int("step.f0104_count", (int)f0104_count, 3,
               "door/stairs/pit branch F0104 coverage");
    expect_int("step.f0105_count", (int)f0105_count, 1,
               "stairs-front flipped lane coverage");
    expect_int("step.f0108_count", (int)f0108_count, 0,
               "D0C F0127 has no F0108 call");
    expect_int("step.f0112_count", (int)f0112_count, 1,
               "unconditional D0C F0112 tail");
    expect_int("step.f0113_count", (int)f0113_count, 1,
               "teleporter-only D0C field");
    expect_int("step.f0115_count", (int)f0115_count, 1,
               "unconditional D0C F0115 tail");
    expect_int("step.no_f0108_count", (int)no_f0108_count, 9,
               "every D0C step preserves no-F0108 contract");
}

static void test_no_occlusion_contexts(void)
{
    expect_int("occlusion.door_side",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D0C_F0108_FOCCL_CONTEXT_DOOR_SIDE_PC34) ? 1 : 0,
               0, "C16 door-side no F0108");
    expect_int("occlusion.stairs_front",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D0C_F0108_FOCCL_CONTEXT_STAIRS_FRONT_PC34) ? 1 : 0,
               0, "C19 stairs-front no F0108");
    expect_int("occlusion.open_pit",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D0C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34) ? 1 : 0,
               0, "C02 pit no BUG0_64 on D0C");
    expect_int("occlusion.teleporter",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D0C_F0108_FOCCL_CONTEXT_TELEPORTER_PC34) ? 1 : 0,
               0, "C05 teleporter no F0108");
    expect_int("occlusion.corridor",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D0C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34) ? 1 : 0,
               0, "corridor/base no F0108 in D0C F0127");
}

static void test_blend_zone_and_evidence(void)
{
    expect_int("blend.transparent",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_blend_c10_pc34(
                   0xaau, 10u),
               0xaa, "DEFS.H C10 preserves destination");
    expect_int("blend.opaque",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_blend_c10_pc34(
                   0xaau, 0x52u),
               0x52, "F0108 helper remains source-locked");
    expect_int("zone.d0c_11_stride",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_zone_d0c_pc34(1, 9),
               1520, "C1500 + 1*11 + 9 = 1520");
    expect_int("zone.clamp_coord",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_zone_d0c_pc34(-1, 9),
               1509, "negative coordinate_set defensive clamp");
    expect_int("zone.clamp_view",
               dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_zone_d0c_pc34(1, -1),
               1511, "negative view_floor defensive clamp");

    expect_contains("evidence.f0127",
                    dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "F0127:8184-8310",
                    "source evidence cites D0C dispatch");
    expect_contains("evidence.no_f0108",
                    dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_source_evidence_pc34(),
                    "contains no F0108",
                    "source evidence cites no-call contract");
    expect_contains("disjoint.d1c",
                    dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_disjointness_note_pc34(),
                    "D1C BUG0_64",
                    "disjoint from D1C occlusion gate");
}

int main(void)
{
    test_model_core();
    test_steps();
    test_no_occlusion_contexts();
    test_blend_zone_and_evidence();
    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_pc34_compat assertions=%d hash=0x%08x\n",
           g_assertions,
           (unsigned)dm1_v1_viewport_d0c_f0108_floor_ornament_occlusion_deterministic_hash_pc34());
    return 0;
}
