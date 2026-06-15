#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1c_f0107_wall_ornament_pc34_compat.h"

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

static void expect_u32(const char *id, uint32_t got, uint32_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%08x want=0x%08x anchor=%s\n",
               id, (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == 0x%08x anchor=%s\n", id, (unsigned)want, anchor);
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

static void test_model_core(void)
{
    DM1_V1_D1CF0107WallOrnamentModelPc34 built;
    const DM1_V1_D1CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_pc34();

    expect_int("builder.null",
               dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_builder_pc34(NULL),
               0, "defensive builder guard");
    expect_int("builder.ok",
               dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_builder_pc34(&built),
               1, "deterministic default model builder");
    expect_int("model.present", model != NULL, 1, "default model accessor");
    if (!model) return;
    expect_int("model.hash_matches_built",
               built.deterministic_hash == model->deterministic_hash, 1,
               "default model builder is deterministic");
    expect_int("hash_model.null",
               dm1_v1_viewport_d1c_f0107_wall_ornament_hash_model_pc34(NULL),
               0, "hash_model null guard");
    expect_int("hash_model.matches",
               dm1_v1_viewport_d1c_f0107_wall_ornament_hash_model_pc34(model) ==
                   model->deterministic_hash,
               1, "hash_model stable");
    expect_int("hash_accessor.matches",
               dm1_v1_viewport_d1c_f0107_wall_ornament_deterministic_hash_pc34() ==
                   model->deterministic_hash,
               1, "deterministic_hash accessor stable");

    expect_int("view_square.d1c", model->view_square_d1c, 3,
               "DEFS.H:2600 M606_VIEW_SQUARE_D1C");
    expect_int("view_wall.d1c_front", model->view_wall_d1c_front, 14,
               "DEFS.H:2710 M587_VIEW_WALL_D1C_FRONT");
    expect_int("wall_zone.d1c", model->wall_zone_d1c, 712,
               "DEFS.H:4049 C712_ZONE_WALL_D1C");
    expect_int("transparent.c10", model->c10_transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("slot.m552", model->front_wall_ornament_slot, 5,
               "DEFS.H:2554 M552_FRONT_WALL_ORNAMENT_ORDINAL");
    expect_int("slot.m551", model->right_wall_ornament_slot, 4,
               "DEFS.H:2553 M551_RIGHT_WALL_ORNAMENT_ORDINAL");
    expect_int("slot.m553", model->left_wall_ornament_slot, 6,
               "DEFS.H:2555 M553_LEFT_WALL_ORNAMENT_ORDINAL");
    expect_int("slot.m550", model->first_thing_slot, 2,
               "DEFS.H:2550 M550_FIRST_THING");
    expect_int("alcove.cell_order", (int)model->alcove_cell_order, 0,
               "F0115 C0x0000_CELL_ORDER_ALCOVE");
    expect_int("dispatch.order.d1c", model->f0128_dispatch_order_d1c, 14,
               "DUNVIEW.C F0128:8536");
    expect_int("dispatch.after_d1l", model->f0128_dispatch_after_d1l, 1,
               "DUNVIEW.C:8526 before 8536");
    expect_int("dispatch.after_d1r", model->f0128_dispatch_after_d1r, 1,
               "DUNVIEW.C:8531 before 8536");
    expect_int("dispatch.before_d0l", model->f0128_dispatch_before_d0l, 1,
               "DUNVIEW.C:8536 before 8541");
    expect_int("wall.uses_f0107", model->f0124_wall_case_uses_f0107, 1,
               "DUNVIEW.C:7842 F0107 call");
    expect_int("wall.no_f0108", model->f0124_wall_case_uses_f0108, 0,
               "DUNVIEW.C:7790-7848 wall route excludes F0108");
    expect_int("wall.no_f0111", model->f0124_wall_case_uses_f0111, 0,
               "DUNVIEW.C:7790-7848 wall route excludes F0111");
    expect_int("wall.f0115_only_alcove",
               model->f0124_wall_case_uses_f0115_only_for_alcove, 1,
               "DUNVIEW.C:7842-7845 F0115 gated by F0107 return");
    expect_int("zero_ordinal.false", model->f0107_zero_ordinal_returns_false, 1,
               "DUNVIEW.C:3568/3936 zero ordinal returns false");
    expect_int("non_alcove.false", model->f0107_non_alcove_returns_false, 1,
               "DUNVIEW.C:3589/3933 classifier false");
    expect_int("alcove.true", model->f0107_alcove_returns_true, 1,
               "DUNVIEW.C:3589/3933 classifier true");
    expect_int("sets.facing_alcove", model->f0107_sets_facing_alcove, 1,
               "DUNVIEW.C:3649 G0286_B_FacingAlcove");
    expect_int("sets.vi_altar", model->f0107_sets_vi_altar, 1,
               "DUNVIEW.C:3650 G0287_B_FacingViAltar");
    expect_int("sets.fountain", model->f0107_sets_fountain, 1,
               "DUNVIEW.C:3667 G0288_B_FacingFountain");
    expect_int("f0107.c10", model->f0107_blit_uses_c10, 1,
               "DUNVIEW.C:3922 F0791 C10 blit");
    expect_int("zone.base", model->wall_ornament_zone_base, 1004,
               "DEFS.H:4225 C1004_ZONE_WALL_ORNAMENT");
    expect_int("zone.stride", model->wall_ornament_zone_stride, 15,
               "DUNVIEW.C:3586 MEDIA720 C15_UNKNOWN");
    expect_int("zone.coord_set", model->wall_ornament_coordinate_set, 2,
               "synthetic current-map wall ornament coordinate set");
    expect_int("zone.d1c", model->wall_ornament_zone_d1c_front, 1048,
               "C1004 + CoordinateSet*15 + M587");
    expect_int("bitmap.incremented", model->wall_ornament_native_bitmap_incremented_for_front,
               1, "DUNVIEW.C:3618/3630 native bitmap increment for front wall");
    expect_int("palette.native", model->wall_ornament_palette_d1c_native, 1,
               "D1C uses native wall-ornament C10 route, not D2/D3 shrink palette");
    expect_int("f0115.first_thing", model->f0115_alcove_uses_first_thing, 1,
               "DUNVIEW.C:7843 L0218[M550_FIRST_THING]");
    expect_int("f0115.view_square", model->f0115_alcove_uses_d1c_view_square, 1,
               "DUNVIEW.C:7844 M606_VIEW_SQUARE_D1C");
    expect_int("no_graphics_dat", model->no_graphics_dat_reads, 1,
               "asset-free fixture");
    expect_int("contract_only", model->source_locked_contract_only, 1,
               "source-lock contract only");
    expect_int("no_real_asset_parity", model->no_real_asset_bitmap_parity, 1,
               "no original DOS pixel parity claim");
    expect_int("helper.reused", model->helper_f0107_slot_constants_reused, 1,
               "reuses existing F0107 helper slot constants");
}

static void test_slot_flow(void)
{
    size_t i;
    const DM1_V1_D1CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_pc34();

    expect_int("slot.at.out_of_range",
               dm1_v1_viewport_d1c_f0107_wall_ornament_slot_flow_at_pc34(3) == NULL,
               1, "slot accessor bounds");
    for (i = 0; model && i < DM1_V1_D1C_F0107_WALL_ORNAMENT_SLOT_COUNT_PC34; ++i) {
        const DM1_V1_D1CF0107SlotFlowPc34 *slot =
            dm1_v1_viewport_d1c_f0107_wall_ornament_slot_flow_at_pc34(i);
        char id[64];
        snprintf(id, sizeof(id), "slot.%u.present", (unsigned)i);
        expect_int(id, slot != NULL, 1, "slot accessor");
        if (!slot) continue;
        snprintf(id, sizeof(id), "slot.%u.model_ptr", (unsigned)i);
        expect_int(id, slot == &model->slots[i], 1, "model slot pointer stable");
        snprintf(id, sizeof(id), "slot.%u.index", (unsigned)i);
        expect_int(id, slot->expected_ord_flow_index, (int)i,
                   "M551/M552/M553 deterministic flow order");
        snprintf(id, sizeof(id), "slot.%u.view_wall", (unsigned)i);
        expect_int(id, slot->view_wall, 14, "DEFS.H:2710 M587 D1C front");
        snprintf(id, sizeof(id), "slot.%u.anchor.has_dunview", (unsigned)i);
        expect_contains(id, slot->redmcsb_anchor, "DUNVIEW.C", "slot source anchor");
    }

    expect_int("slot.front.aspect", model ? model->slots[0].aspect_slot : -1, 5,
               "DEFS.H:2554 M552 front");
    expect_contains("slot.front.name", model ? model->slots[0].slot_name : NULL,
                    "M552_FRONT", "front slot name");
    expect_int("slot.front.reaches", model ? model->slots[0].reaches_d1c_f0107 : 0,
               1, "DUNVIEW.C:7842 reads M552");
    expect_int("slot.front.alcove_pass",
               model ? model->slots[0].can_trigger_alcove_thing_pass : 0,
               1, "DUNVIEW.C:7842-7845");
    expect_int("slot.front.not_rejected",
               model ? model->slots[0].side_slot_rejected_by_d1c : 1,
               0, "M552 is the D1C route");

    expect_int("slot.right.aspect", model ? model->slots[1].aspect_slot : -1, 4,
               "DEFS.H:2553 M551 right");
    expect_int("slot.right.no_d1c", model ? model->slots[1].reaches_d1c_f0107 : 1,
               0, "D1C F0124 does not read M551");
    expect_int("slot.right.rejected", model ? model->slots[1].side_slot_rejected_by_d1c : 0,
               1, "M551 belongs to side-wall routes");

    expect_int("slot.left.aspect", model ? model->slots[2].aspect_slot : -1, 6,
               "DEFS.H:2555 M553 left");
    expect_int("slot.left.no_d1c", model ? model->slots[2].reaches_d1c_f0107 : 1,
               0, "D1C F0124 does not read M553");
    expect_int("slot.left.rejected", model ? model->slots[2].side_slot_rejected_by_d1c : 0,
               1, "M553 belongs to side-wall routes");
}

static void test_steps(void)
{
    size_t i;
    int present_count = 0;
    int keepout_count = 0;

    expect_int("step.out_of_range",
               dm1_v1_viewport_d1c_f0107_wall_ornament_step_at_pc34(7) == NULL,
               1, "step accessor bounds");
    for (i = 0; i < DM1_V1_D1C_F0107_WALL_ORNAMENT_STEP_COUNT_PC34; ++i) {
        const DM1_V1_D1CF0107StepPc34 *step =
            dm1_v1_viewport_d1c_f0107_wall_ornament_step_at_pc34(i);
        char id[64];
        snprintf(id, sizeof(id), "step.%u.present", (unsigned)i);
        expect_int(id, step != NULL, 1, "step accessor");
        if (!step) continue;
        present_count += step->expected_present ? 1 : 0;
        keepout_count += step->expected_present ? 0 : 1;
        snprintf(id, sizeof(id), "step.%u.order", (unsigned)i);
        expect_int(id, step->order_index, (int)i, "step order stable");
        snprintf(id, sizeof(id), "step.%u.anchor", (unsigned)i);
        expect_contains(id, step->redmcsb_anchor, "DUNVIEW.C", "step anchor");
    }
    expect_int("steps.present_count", present_count, 5,
               "F0128/F0124/F0765/F0107/F0115 present");
    expect_int("steps.keepout_count", keepout_count, 2,
               "F0108 and F0111 keepout");
    expect_int("step0.kind",
               dm1_v1_viewport_d1c_f0107_wall_ornament_step_at_pc34(0)->step,
               DM1_V1_D1C_F0107_STEP_F0128_DISPATCH_D1C_PC34,
               "DUNVIEW.C:8536");
    expect_int("step3.kind",
               dm1_v1_viewport_d1c_f0107_wall_ornament_step_at_pc34(3)->step,
               DM1_V1_D1C_F0107_STEP_F0107_FRONT_WALL_ORNAMENT_PC34,
               "DUNVIEW.C:7842");
    expect_int("step5.keepout",
               dm1_v1_viewport_d1c_f0107_wall_ornament_step_at_pc34(5)->expected_present,
               0, "DUNVIEW.C F0108 not in wall case");
    expect_int("step6.keepout",
               dm1_v1_viewport_d1c_f0107_wall_ornament_step_at_pc34(6)->expected_present,
               0, "DUNVIEW.C F0111 not in wall case");
}

static void test_f0107_alcove_guard_and_pixels(void)
{
    const DM1_V1_D1CF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_pc34();
    size_t i;

    expect_int("alcove.zero.true",
               dm1_v1_viewport_d1c_f0107_wall_ornament_returns_alcove_pc34(0, true),
               0, "DUNVIEW.C:3568 zero ordinal guard");
    expect_int("alcove.zero.false",
               dm1_v1_viewport_d1c_f0107_wall_ornament_returns_alcove_pc34(0, false),
               0, "DUNVIEW.C:3568 zero ordinal guard");
    expect_int("alcove.nonzero.false",
               dm1_v1_viewport_d1c_f0107_wall_ornament_returns_alcove_pc34(7, false),
               0, "DUNVIEW.C:3589 F0149 false");
    expect_int("alcove.nonzero.true",
               dm1_v1_viewport_d1c_f0107_wall_ornament_returns_alcove_pc34(7, true),
               1, "DUNVIEW.C:3933 returns L0096");
    expect_int("blend.transparent",
               dm1_v1_viewport_d1c_f0107_wall_ornament_blend_pixel_pc34(0xaa, 10, 10),
               0xaa, "DEFS.H:2088 C10 preserves destination");
    expect_int("blend.opaque",
               dm1_v1_viewport_d1c_f0107_wall_ornament_blend_pixel_pc34(0xaa, 0x51, 10),
               0x51, "F0107 opaque ornament pixel writes");

    for (i = 0; model && i < DM1_V1_D1C_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        const DM1_V1_D1CF0107PixelPc34 *p = &model->pixels[i];
        char id[64];
        snprintf(id, sizeof(id), "pixel.%u.after", (unsigned)i);
        expect_int(id, p->after,
                   dm1_v1_viewport_d1c_f0107_wall_ornament_blend_pixel_pc34(
                       p->before, p->source, 10),
                   "DUNVIEW.C:3922 C10 transparent blit");
        snprintf(id, sizeof(id), "pixel.%u.skip_xor_write", (unsigned)i);
        expect_int(id, p->transparent_skip + p->writes_pixel, 1,
                   "each pixel either skips or writes");
        snprintf(id, sizeof(id), "pixel.%u.anchor", (unsigned)i);
        expect_contains(id, p->anchor, "F0107", "pixel anchor");
    }
}

static void test_source_evidence_and_disjointness(void)
{
    const char *e = dm1_v1_viewport_d1c_f0107_wall_ornament_source_evidence_pc34();
    const char *d = dm1_v1_viewport_d1c_f0107_wall_ornament_disjointness_note_pc34();

    expect_contains("evidence.f0107", e, "DUNVIEW.C F0107:3502-3938",
                    "required F0107 source anchor");
    expect_contains("evidence.f0104", e, "F0104:3113-3156",
                    "required native C10 source anchor");
    expect_contains("evidence.f0108", e, "F0108:3940-4011",
                    "required F0108 contrast anchor");
    expect_contains("evidence.f0115", e, "F0115:4547-4581",
                    "required F0115 source anchor");
    expect_contains("evidence.f0124", e, "F0124:7727-7924",
                    "D1C dispatch body");
    expect_contains("evidence.f0128", e, "F0128:8318-8542",
                    "D1C dispatch path");
    expect_contains("evidence.m552", e, "M552_FRONT_WALL_ORNAMENT_ORDINAL",
                    "M552 flow");
    expect_contains("evidence.m587", e, "M587_VIEW_WALL_D1C_FRONT",
                    "D1C wall ordinal");
    expect_contains("evidence.m550", e, "M550_FIRST_THING",
                    "F0115 alcove thing source");
    expect_contains("evidence.c10", e, "DEFS.H:2088",
                    "C10 transparency");
    expect_contains("evidence.view_square", e, "M606_VIEW_SQUARE_D1C=3",
                    "D1C view square");
    expect_contains("evidence.c705", e, "C705/C706",
                    "adjacent wall zone parity");
    expect_contains("evidence.cell_zone", e, "DEFS.H:4139-4153",
                    "cell-order zone band");
    expect_contains("evidence.m551", e, "M550/M551/M552/M553",
                    "per-element wall ornament flow");
    expect_contains("evidence.m587.defs", e, "M587_VIEW_WALL_D1C_FRONT=14",
                    "D1C view-wall ordinal");

    expect_contains("disjoint.d1c", d, "D1C F0107",
                    "disjointness note");
    expect_contains("disjoint.no_d0", d, "D0C/D0L/D0R",
                    "does not touch D0 files");
    expect_contains("disjoint.no_f0108", d, "F0108",
                    "does not duplicate F0108 gates");
    expect_contains("disjoint.no_d1c_wall", d, "dm1_v1_viewport_d1c_wall_pc34_compat",
                    "does not touch existing D1C wall gate");
    expect_contains("disjoint.no_other_games", d, "CSB/Nexus/Theron/DM2",
                    "other game files untouched");
    expect_contains("disjoint.no_assets", d, "GRAPHICS.DAT",
                    "asset-free claim");
}

static void test_hash(void)
{
    uint32_t hash = dm1_v1_viewport_d1c_f0107_wall_ornament_deterministic_hash_pc34();

    expect_int("hash.nonzero", hash != 0u, 1, "deterministic hash exists");
    expect_u32("hash.stable", hash, 0x67ab6d74u,
               "deterministic D1C F0107 wall-ornament source-lock hash");
}

int main(void)
{
    test_model_core();
    test_slot_flow();
    test_steps();
    test_f0107_alcove_guard_and_pixels();
    test_source_evidence_and_disjointness();
    test_hash();

    if (g_failures) {
        printf("FAIL DM1_V1_VIEWPORT_D1C_F0107_WALL_ORNAMENT_PC34_COMPAT assertions=%d failures=%d deterministic_hash=0x%08x\n",
               g_assertions, g_failures,
               (unsigned)dm1_v1_viewport_d1c_f0107_wall_ornament_deterministic_hash_pc34());
        return 1;
    }
    printf("DM1_V1_VIEWPORT_D1C_F0107_WALL_ORNAMENT_PC34_COMPAT_OK assertions=%d failures=0 deterministic_hash=0x%08x\n",
           g_assertions,
           (unsigned)dm1_v1_viewport_d1c_f0107_wall_ornament_deterministic_hash_pc34());
    return 0;
}
