#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_pc34_compat.h"

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
    DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 built;
    const DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 *model =
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_pc34();

    expect_int("builder.null",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
                   NULL),
               0, "defensive builder guard");
    expect_int("builder.ok",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
                   &built),
               1, "deterministic default model builder");
    expect_int("model.present", model != NULL, 1, "default model accessor");
    if (!model) return;
    expect_int("model.hash_matches_built",
               built.deterministic_hash == model->deterministic_hash, 1,
               "default model builder is deterministic");
    expect_int("hash_model.null",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_hash_model_pc34(
                   NULL),
               0, "hash_model null guard");
    expect_int("hash_model.matches",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_hash_model_pc34(
                   model) == model->deterministic_hash,
               1, "hash_model stable");
    expect_int("hash_accessor.matches",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_deterministic_hash_pc34() ==
                   model->deterministic_hash,
               1, "deterministic_hash accessor stable");

    expect_int("view_square.d1c", model->view_square_d1c, 3,
               "DEFS.H:2599 M606_VIEW_SQUARE_D1C=3");
    expect_int("view_floor.d1c", model->view_floor_d1c, 7,
               "DEFS.H:2746 M595_VIEW_FLOOR_D1C=7");
    expect_int("view_wall.d1c_front", model->view_wall_d1c_front, 14,
               "DEFS.H:2710 M587_VIEW_WALL_D1C_FRONT=14");
    expect_int("wall_zone.d1c", model->wall_zone_d1c, 712,
               "DEFS.H:4052 C712_ZONE_WALL_D1C");
    expect_int("transparent.c10", model->c10_transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("slot.m558", model->floor_ornament_ordinal_slot, 5,
               "DEFS.H:2558 M558_FLOOR_ORNAMENT_ORDINAL");
    expect_int("slot.m550", model->first_thing_slot, 2,
               "DEFS.H:2550 M550_FIRST_THING");
    expect_int("slot.m554", model->pit_or_teleporter_visible_slot, 3,
               "DEFS.H:2554 M554_PIT_OR_TELEPORTER_VISIBLE");
    expect_int("slot.m555", model->stairs_up_slot, 4,
               "DEFS.H:2555 M555_STAIRS_UP");
    expect_int("slot.m556", model->door_state_slot, 7,
               "DEFS.H:2556 M556_DOOR_STATE");
    expect_int("slot.m557", model->door_thing_index_slot, 8,
               "DEFS.H:2557 M557_DOOR_THING_INDEX");
    expect_int("slot.m552", model->front_wall_ornament_slot, 5,
               "DEFS.H:2554 M552_FRONT_WALL_ORNAMENT_ORDINAL");
    expect_int("dispatch.order.d1c", model->f0128_dispatch_order_d1c, 14,
               "DUNVIEW.C:8536 D1C dispatch line");
    expect_int("dispatch.after_d1l", model->f0128_dispatch_after_d1l, 1,
               "DUNVIEW.C:8526 D1L before 8536 D1C");
    expect_int("dispatch.after_d1r", model->f0128_dispatch_after_d1r, 1,
               "DUNVIEW.C:8531 D1R before 8536 D1C");
    expect_int("dispatch.before_d0l", model->f0128_dispatch_before_d0l, 1,
               "DUNVIEW.C:8541 D0L after 8536 D1C");
    expect_int("door_front.calls_f0108", model->f0124_door_front_calls_f0108, 1,
               "DUNVIEW.C:7874 F0108 at C17 door-front");
    expect_int("door_front.calls_f0115", model->f0124_door_front_calls_f0115, 1,
               "DUNVIEW.C:7875 F0115 with C0x0218");
    expect_int("door_front.cell_order", model->f0124_door_front_cell_order,
               0x0218,
               "DEFS.H:2669 C0x0218_CELL_ORDER_DOORPASS1");
    expect_int("corridor.calls_f0108", model->f0124_corridor_calls_f0108, 1,
               "DUNVIEW.C:7926 F0108 at T0124017");
    expect_int("corridor.calls_f0112", model->f0124_corridor_calls_f0112, 1,
               "DUNVIEW.C:7929 F0112 ceiling pit");
    expect_int("corridor.calls_f0115", model->f0124_corridor_calls_f0115, 1,
               "DUNVIEW.C:7937 F0115 thing pass");
    expect_int("corridor.cell_order", model->f0124_corridor_cell_order, 0x3421,
               "DEFS.H:2676 C0x3421_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT");
    expect_int("pit.f0104_first", model->f0124_pit_f0104_fires_first, 1,
               "DUNVIEW.C:7906 F0104 floor-pit before F0108");
    expect_int("pit.f0108_occludes", model->f0124_pit_f0108_occludes_pit, 1,
               "DUNVIEW.C:7926 BUG0_64 F0108 over pit");
    expect_int("teleporter.f0108_occludes",
               model->f0124_teleporter_f0108_occludes_teleporter, 1,
               "DUNVIEW.C:7926 BUG0_64 F0108 over teleporter");
    expect_int("stairs.f0104_first", model->f0124_stairs_front_f0104_fires_first, 1,
               "DUNVIEW.C:7868 F0104 stairs before T0124017");
    expect_int("stairs.f0108_occludes", model->f0124_stairs_front_f0108_occludes_stairs,
               1, "DUNVIEW.C:7926 BUG0_64 F0108 over stairs-front");
    expect_int("f0108.zero_skips", model->f0108_ordinal_zero_skips_blit, 1,
               "DUNVIEW.C:3951 `if (P0118_ui_FloorOrnamentOrdinal)`");
    expect_int("f0108.fp_recurses", model->f0108_footprint_mask_recurses, 1,
               "DUNVIEW.C:4008 T0108005 self-recursion");
    expect_int("f0108.fp_only_no_primary",
               model->f0108_footprint_only_skips_primary, 1,
               "DUNVIEW.C:3956-3959 `if (!M009_CLEAR) goto T0108005`");
    expect_int("f0108.c10_blit", model->f0108_blit_uses_c10_transparent, 1,
               "DUNVIEW.C:3988-3993 F0791 C10 transparency");
    expect_int("f0108.d1c_flip_branch",
               model->f0108_d1c_in_horizontal_flip_branch, 1,
               "DUNVIEW.C:3967/3977/3980 M595 in flip branch");
    expect_int("f0108.zone_11_stride", model->f0108_d1c_zone_uses_11_stride, 1,
               "DUNVIEW.C:3989/3991 CoordinateSet * 11 + ViewFloor");
    expect_int("f0108.zone_9_stride", model->f0108_d1c_zone_uses_9_stride, 1,
               "DUNVIEW.C:3984 CoordinateSet * 9 (older build)");
    expect_int("f0108.zone_d1c", model->f0108_zone_d1c, 1518,
               "C1500 + 1*11 + 7 PC 3.4 zone");
    expect_int("bug0_64.guard_absent", model->bug0_64_occlusion_guard, 0,
               "DUNVIEW.C:7926 has no occlusion guard");
    expect_int("f0112.ceiling_graphic", model->f0112_ceiling_pit_graphic, 67,
               "DUNVIEW.C:7932 C067_GRAPHIC_CEILING_PIT_D1C");
    expect_int("f0112.ceiling_zone", model->f0112_ceiling_pit_zone_d1c, 868,
               "DUNVIEW.C:7932 C868_ZONE_CEILING_PIT_D1C");
    expect_int("no_graphics_dat", model->no_graphics_dat_reads, 1,
               "asset-free fixture");
    expect_int("contract_only", model->source_locked_contract_only, 1,
               "source-lock contract only");
    expect_int("no_real_asset_parity", model->no_real_asset_bitmap_parity, 1,
               "no original DOS pixel parity claim");
}

static void test_decode_ordinal(void)
{
    bool fp_set = true;
    unsigned int cleared = 0xffffffffu;
    bool primary_draws = false;
    int primary_index = -42;
    bool recurse_fp = true;
    int recurse_index = -42;
    unsigned int kept;

    expect_int("decode.null_arg",
               (int)dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
                   5u, NULL, NULL, NULL, NULL, NULL, NULL),
               5, "decode with NULL outputs still returns ordinal");
    kept = dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        0u, &fp_set, &cleared, &primary_draws, &primary_index, &recurse_fp,
        &recurse_index);
    expect_int("decode.zero.kept", (int)kept, 0,
               "zero ordinal returns 0");
    expect_int("decode.zero.fp", fp_set ? 1 : 0, 0, "no ordinal no fp");
    expect_int("decode.zero.primary", primary_draws ? 1 : 0, 0,
               "no ordinal no primary");

    fp_set = false;
    cleared = 0;
    primary_draws = false;
    primary_index = -1;
    recurse_fp = false;
    recurse_index = -1;
    kept = dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        7u, &fp_set, &cleared, &primary_draws, &primary_index, &recurse_fp,
        &recurse_index);
    expect_int("decode.simple.kept", (int)kept, 7,
               "non-fp ordinal returns input value");
    expect_int("decode.simple.fp", fp_set ? 1 : 0, 0,
               "non-fp ordinal has fp flag clear");
    expect_int("decode.simple.primary", primary_draws ? 1 : 0, 1,
               "non-fp ordinal primary draws");
    expect_int("decode.simple.primary_index", primary_index, 6,
               "non-fp primary_index = ordinal - 1");
    expect_int("decode.simple.recurse", recurse_fp ? 1 : 0, 0,
               "non-fp ordinal no recursion");
    expect_int("decode.simple.recurse_index", recurse_index, -1,
               "no recursion index");

    fp_set = false;
    cleared = 0;
    primary_draws = false;
    primary_index = -1;
    recurse_fp = false;
    recurse_index = -1;
    kept = dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        0x8000u, &fp_set, &cleared, &primary_draws, &primary_index, &recurse_fp,
        &recurse_index);
    expect_int("decode.fp_only.kept", (int)kept, 0x8000u,
               "fp-only ordinal returns input value when primary suppressed");
    expect_int("decode.fp_only.fp", fp_set ? 1 : 0, 1, "fp flag set");
    expect_int("decode.fp_only.cleared", (int)cleared, 0,
               "fp-only cleared ordinal is 0");
    expect_int("decode.fp_only.primary", primary_draws ? 1 : 0, 0,
               "fp-only suppresses primary");
    expect_int("decode.fp_only.primary_index", primary_index, -1,
               "fp-only has no primary index");
    expect_int("decode.fp_only.recurse", recurse_fp ? 1 : 0, 1,
               "fp-only recurses into T0108005");
    expect_int("decode.fp_only.recurse_index", recurse_index, 15,
               "fp-only recurses into C15_FLOOR_ORNAMENT_FOOTPRINTS");

    fp_set = false;
    cleared = 0;
    primary_draws = false;
    primary_index = -1;
    recurse_fp = false;
    recurse_index = -1;
    kept = dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
        0x8007u, &fp_set, &cleared, &primary_draws, &primary_index, &recurse_fp,
        &recurse_index);
    expect_int("decode.fp_with_primary.kept", (int)kept, 0x8007u,
               "fp+primary ordinal returns input value");
    expect_int("decode.fp_with_primary.fp", fp_set ? 1 : 0, 1, "fp flag set");
    expect_int("decode.fp_with_primary.cleared", (int)cleared, 7,
               "fp+primary cleared ordinal = 7");
    expect_int("decode.fp_with_primary.primary", primary_draws ? 1 : 0, 1,
               "fp+primary primary draws");
    expect_int("decode.fp_with_primary.primary_index", primary_index, 6,
               "fp+primary primary_index = cleared - 1");
    expect_int("decode.fp_with_primary.recurse", recurse_fp ? 1 : 0, 0,
               "fp+primary has cleared>0 so no pure recursion");
}

static void test_context_occlusion(void)
{
    expect_int("occlusion.corridor",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D1C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34, 5u) ? 1 : 0,
               1, "DUNVIEW.C:7926 corridor tail F0108 fires");
    expect_int("occlusion.open_pit",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34, 5u) ? 1 : 0,
               1, "DUNVIEW.C:7926 BUG0_64 F0108 over open pit");
    expect_int("occlusion.teleporter",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D1C_F0108_FOCCL_CONTEXT_TELEPORTER_PC34, 5u) ? 1 : 0,
               1, "DUNVIEW.C:7926 BUG0_64 F0108 over teleporter");
    expect_int("occlusion.stairs_front",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D1C_F0108_FOCCL_CONTEXT_STAIRS_FRONT_PC34, 5u) ? 1 : 0,
               1, "DUNVIEW.C:7926 BUG0_64 F0108 over stairs-front");
    expect_int("occlusion.door_front",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D1C_F0108_FOCCL_CONTEXT_DOOR_FRONT_PC34, 5u) ? 1 : 0,
               1, "DUNVIEW.C:7874 door-front F0108 fires");
    expect_int("occlusion.zero_ordinal",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D1C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34, 0u) ? 1 : 0,
               0, "F0108 zero ordinal skip");
    expect_int("occlusion.fp_only_keeps_occlusion",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
                   DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34, 0x800fu) ? 1 : 0,
               1, "fp-only ordinal still occludes via T0108005 recursion");
}

static void test_blend_and_zone(void)
{
    expect_int("blend.transparent",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_blend_c10_pc34(
                   0xaau, 10u),
               0xaa, "DEFS.H:2088 C10 preserves destination");
    expect_int("blend.opaque",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_blend_c10_pc34(
                   0xaau, 0x52u),
               0x52, "F0108 opaque ornament pixel writes");
    expect_int("zone.d1c_11_stride",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_zone_d1c_pc34(1, 7),
               1518, "C1500 + 1*11 + 7 = 1518");
    expect_int("zone.d1c_9_stride",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_zone_d1c_pc34(0, 9),
               1500 + 9, "older build CoordinateSet*9 path still synthetic");
    expect_int("zone.d1c_alt",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_zone_d1c_pc34(2, 7),
               1500 + 2 * 11 + 7, "stride 11 for second coordinate set");
}

static void test_steps(void)
{
    unsigned int n = dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_count_pc34();
    unsigned int occlusion_count = 0;
    unsigned int f0108_count = 0;
    unsigned int f0112_count = 0;
    unsigned int f0115_count = 0;
    unsigned int bug0_64_count = 0;
    unsigned int i;
    size_t out_of_range = (size_t)-1;

    expect_int("step.count", (int)n, 8,
               "F0128/F0124/F0108 call/F0108 decode/F0108 recursion/F0108 C10/F0112/F0115/BUG0_64");
    expect_int("step.out_of_range",
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_step_at_pc34(
                   (size_t)n) == NULL &&
                   dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_step_at_pc34(
                       out_of_range) == NULL,
               1, "step accessor bounds");

    for (i = 0; i < n; ++i) {
        const DM1_V1_D1CF0108FloorOrnamentOcclusionStepPc34 *step =
            dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_step_at_pc34(
                (size_t)i);
        char id[64];
        snprintf(id, sizeof(id), "step.%u.present", i);
        expect_int(id, step != NULL, 1, "step accessor");
        if (!step) continue;
        f0108_count += step->calls_f0108 ? 1u : 0u;
        f0112_count += step->calls_f0112 ? 1u : 0u;
        f0115_count += step->calls_f0115 ? 1u : 0u;
        bug0_64_count += step->bug0_64_occlusion_present ? 1u : 0u;
        if (step->f0108_occludes_cell &&
            (step->context == DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34 ||
             step->context == DM1_V1_D1C_F0108_FOCCL_CONTEXT_TELEPORTER_PC34 ||
             step->context == DM1_V1_D1C_F0108_FOCCL_CONTEXT_STAIRS_FRONT_PC34)) {
            ++occlusion_count;
        }
        snprintf(id, sizeof(id), "step.%u.order", i);
        expect_int(id, step->order_index, (int)i,
                   "step order_index matches position");
        snprintf(id, sizeof(id), "step.%u.anchor", i);
        expect_contains(id, step->redmcsb_anchor, "DUNVIEW.C", "step anchor");
    }
    expect_int("step.f0108_count", (int)f0108_count, 1,
               "F0108 call attributed to F0124 step only");
    expect_int("step.f0112_count", (int)f0112_count, 1,
               "F0112 ceiling-pit only in shared-tail step");
    expect_int("step.f0115_count", (int)f0115_count, 1,
               "F0115 thing pass only in shared-tail step");
    expect_int("step.bug0_64_count", (int)bug0_64_count, 5,
               "BUG0_64 marker covers F0108 call, decode, recursion, C10, BUG0_64 guard step");
    expect_int("step.occlusion_count", (int)occlusion_count, 1,
               "F0124 F0108 call is the BUG0_64 occlusion entry point");
}

static void test_source_evidence_and_disjointness(void)
{
    const char *e =
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_source_evidence_pc34();
    const char *d =
        dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_disjointness_note_pc34();

    expect_contains("evidence.f0124", e, "F0124:7727-7924", "D1C dispatch body");
    expect_contains("evidence.f0108", e, "F0108:3940-4011", "F0108 anchor");
    expect_contains("evidence.f0128", e, "F0128:8318-8542", "D1C dispatch path");
    expect_contains("evidence.f0112", e, "F0112", "ceiling-pit dispatch");
    expect_contains("evidence.f0115", e, "F0115", "thing pass dispatch");
    expect_contains("evidence.f0104", e, "F0104", "floor-pit/stairs bitmap");
    expect_contains("evidence.m558", e, "M558_FLOOR_ORNAMENT_ORDINAL",
                    "M558 floor ornament slot");
    expect_contains("evidence.m587", e, "M587_VIEW_WALL_D1C_FRONT=14",
                    "D1C front wall ordinal");
    expect_contains("evidence.m595", e, "M595_VIEW_FLOOR_D1C=7",
                    "D1C view-floor ordinal");
    expect_contains("evidence.m606", e, "M606_VIEW_SQUARE_D1C=3",
                    "D1C view-square ordinal");
    expect_contains("evidence.c712", e, "C712_ZONE_WALL_D1C",
                    "D1C wall zone anchor");
    expect_contains("evidence.cell_order", e,
                    "C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT",
                    "T0124017 cell order");
    expect_contains("evidence.cell_order_door", e, "C0x0218_CELL_ORDER_DOORPASS1",
                    "door-front cell order");
    expect_contains("evidence.c10", e, "DEFS.H:2088", "C10 transparency");
    expect_contains("evidence.c1500", e, "C1500", "floor-ornament zone base");
    expect_contains("evidence.footprint", e, "MASK0x8000_FOOTPRINTS",
                    "footprint recursion mask");
    expect_contains("evidence.t0108005", e, "T0108005", "F0108 self-recursion label");
    expect_contains("evidence.t0124017", e, "T0124017",
                    "D1C pit/teleporter/corridor shared tail");
    expect_contains("evidence.bug0_64", e, "BUG0_64", "occlusion source comment");
    expect_contains("evidence.stairs_goto", e,
                    "C19_ELEMENT_STAIRS_FRONT",
                    "D1C stairs-front case");
    expect_contains("evidence.door_front", e,
                    "C17_ELEMENT_DOOR_FRONT",
                    "D1C door-front case");
    expect_contains("evidence.m555", e, "M555_STAIRS_UP", "stairs-up slot");
    expect_contains("evidence.m556", e, "M556_DOOR_STATE", "door-state slot");
    expect_contains("evidence.m557", e, "M557_DOOR_THING_INDEX", "door-thing-index slot");
    expect_contains("evidence.m554", e, "M554_PIT_OR_TELEPORTER_VISIBLE",
                    "pit/teleporter visibility slot");

    expect_contains("disjoint.d1c", d, "D1C F0108",
                    "disjointness note");
    expect_contains("disjoint.no_d0", d, "D0C/D0L/D0R",
                    "does not touch D0 files");
    expect_contains("disjoint.no_d1c_wall", d, "D1C F0107",
                    "does not duplicate D1C wall gate");
    expect_contains("disjoint.no_d1c_f0111", d, "D1C F0111",
                    "does not duplicate D1C door panel gate");
    expect_contains("disjoint.no_d1c_f0115", d, "D1C F0115",
                    "does not duplicate D1C thing-pass gate");
    expect_contains("disjoint.no_d1c_stairs", d, "D1C stairs",
                    "does not duplicate D1C stairs/pit dispatch");
    expect_contains("disjoint.no_d0c_f0108", d, "D0C F0108",
                    "does not duplicate D0C F0108 floor+ceiling+ornament");
    expect_contains("disjoint.no_d2c_f0108", d, "D2C F0108",
                    "does not duplicate D2C F0108");
    expect_contains("disjoint.no_d0c_keepout", d, "D0C F0108 floor-ornament keepout",
                    "does not duplicate D0C keepout");
    expect_contains("disjoint.no_other_games", d, "CSB/Nexus/Theron/DM2",
                    "other game files untouched");
    expect_contains("disjoint.no_assets", d, "GRAPHICS.DAT",
                    "asset-free claim");
    expect_contains("disjoint.bug0_64", d, "BUG0_64",
                    "BUG0_64 source anchor");
}

int main(void)
{
    test_model_core();
    test_decode_ordinal();
    test_context_occlusion();
    test_blend_and_zone();
    test_steps();
    test_source_evidence_and_disjointness();

    if (g_failures) {
        printf("FAIL DM1_V1_VIEWPORT_D1C_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT "
               "assertions=%d failures=%d deterministic_hash=0x%08x\n",
               g_assertions, g_failures,
               (unsigned)
                   dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_deterministic_hash_pc34());
        return 1;
    }
    printf("DM1_V1_VIEWPORT_D1C_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_OK "
           "assertions=%d failures=0 deterministic_hash=0x%08x\n",
           g_assertions,
           (unsigned)
               dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_deterministic_hash_pc34());
    return 0;
}
