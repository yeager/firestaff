#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_pc34_compat.h"

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
    DM1_V1_D0LD0RF0107WallOrnamentModelPc34 built;
    const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_pc34();

    expect_int("builder.null",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_builder_pc34(NULL),
               0, "defensive builder guard");
    expect_int("builder.ok",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_builder_pc34(&built),
               1, "deterministic default model builder");
    expect_int("model.present", model != NULL, 1, "default model accessor");
    if (!model) return;
    expect_int("model.hash_matches_built",
               built.deterministic_hash == model->deterministic_hash, 1,
               "default model builder is deterministic");
    expect_int("hash_model.null",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_hash_model_pc34(NULL),
               0, "hash_model null guard");
    expect_int("hash_model.matches",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_hash_model_pc34(model) ==
                   model->deterministic_hash,
               1, "hash_model stable");
    expect_int("hash_accessor.matches",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_deterministic_hash_pc34() ==
                   model->deterministic_hash,
               1, "hash accessor stable");

    expect_int("view_square.d0l", model->view_square_d0l, 1,
               "DEFS.H:2597 M610_VIEW_SQUARE_D0L");
    expect_int("view_square.d0r", model->view_square_d0r, 2,
               "DEFS.H:2598 M611_VIEW_SQUARE_D0R");
    expect_int("wall_zone.d0l", model->wall_zone_d0l, 716,
               "DEFS.H:4056 C716_ZONE_WALL_D0L");
    expect_int("wall_zone.d0r", model->wall_zone_d0r, 717,
               "DEFS.H:4057 C717_ZONE_WALL_D0R");
    expect_int("transparent.c10", model->c10_transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("slot.m550", model->m550_first_thing_slot, 2,
               "DEFS.H:2547 M550_FIRST_THING");
    expect_int("slot.m551", model->m551_right_wall_ornament_slot, 4,
               "DEFS.H:2553 M551_RIGHT_WALL_ORNAMENT_ORDINAL");
    expect_int("slot.m552", model->m552_front_wall_ornament_slot, 5,
               "DEFS.H:2554 M552_FRONT_WALL_ORNAMENT_ORDINAL");
    expect_int("slot.m553", model->m553_left_wall_ornament_slot, 6,
               "DEFS.H:2555 M553_LEFT_WALL_ORNAMENT_ORDINAL");
    expect_int("dispatch.d0l_then_d0r", model->f0128_d0l_then_d0r, 1,
               "DUNVIEW.C:8536-8541");
    expect_int("d0l.no_direct_f0107", model->d0l_direct_f0107_calls, 0,
               "DUNVIEW.C F0125 wall return");
    expect_int("d0r.no_direct_f0107", model->d0r_direct_f0107_calls, 0,
               "DUNVIEW.C F0126 wall return");
    expect_int("wall.return_before_f0107", model->d0_wall_case_returns_before_f0107, 1,
               "DUNVIEW.C:8009-8024/8119-8131");
    expect_int("zero_ordinal.false", model->f0107_zero_ordinal_returns_false, 1,
               "DUNVIEW.C:3568/3936");
    expect_int("non_alcove.false", model->f0107_non_alcove_cell_returns_false, 1,
               "DUNVIEW.C:3589/3933");
    expect_int("alcove.true", model->f0107_alcove_cell_returns_true, 1,
               "DUNVIEW.C:3589/3933");
    expect_int("cell_bits.source", model->f0107_uses_cell_content_bits, 1,
               "F0149 alcove classifier source");
    expect_int("f0107.c10", model->f0107_blit_uses_c10, 1,
               "DUNVIEW.C:3922");
    expect_int("c10.preserve", model->c10_transparent_preserves_destination, 1,
               "DEFS.H:2088 C10 transparent preservation");
    expect_int("f0108.keepout", model->f0108_floor_ceiling_keepout, 1,
               "DUNVIEW.C F0108:3940-4011 contrast");
    expect_int("f0115.d0l.order", (int)model->f0115_d0l_order_backright, 0x0002,
               "DUNVIEW.C:8005 C0x0002");
    expect_int("f0115.d0r.order", (int)model->f0115_d0r_order_backleft, 0x0001,
               "DUNVIEW.C:8115 C0x0001");
    expect_int("d0l.thing_before_ceiling", model->d0l_thing_before_ceiling, 1,
               "DUNVIEW.C:8005 before 8043");
    expect_int("d0r.ceiling_before_thing", model->d0r_ceiling_before_thing, 1,
               "DUNVIEW.C:8113 before 8115");
    expect_int("f0111.c10_half_blits", model->f0111_partly_open_uses_c10_half_blits, 1,
               "DUNVIEW.C:4322-4324");
    expect_int("no_graphics_dat", model->no_graphics_dat_reads, 1,
               "asset-free fixture");
    expect_int("contract_only", model->source_locked_contract_only, 1,
               "source-lock contract only");
    expect_int("no_dos_pixel_parity", model->no_original_dos_pixel_parity, 1,
               "no original DOS pixel parity claim");
    expect_int("helper.reused", model->helper_f0107_slot_constants_reused, 1,
               "reuses F0107 helper slot constants");
}

static void test_lanes(void)
{
    const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_pc34();
    size_t i;

    expect_int("lane.out_of_range",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_lane_at_pc34(2) == NULL,
               1, "lane accessor bounds");
    for (i = 0; model && i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_SIDE_COUNT_PC34; ++i) {
        const DM1_V1_D0LD0RF0107LanePc34 *lane =
            dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_lane_at_pc34(i);
        char id[80];
        snprintf(id, sizeof(id), "lane.%u.present", (unsigned)i);
        expect_int(id, lane != NULL, 1, "lane accessor");
        if (!lane) continue;
        snprintf(id, sizeof(id), "lane.%u.model_ptr", (unsigned)i);
        expect_int(id, lane == &model->lanes[i], 1, "lane pointer stable");
        snprintf(id, sizeof(id), "lane.%u.first_thing_slot", (unsigned)i);
        expect_int(id, lane->first_thing_slot, 2, "DEFS.H M550_FIRST_THING");
        snprintf(id, sizeof(id), "lane.%u.no_f0107", (unsigned)i);
        expect_int(id, lane->direct_f0107_call_present, 0,
                   "D0 wall case returns before direct F0107");
        snprintf(id, sizeof(id), "lane.%u.wall_return", (unsigned)i);
        expect_int(id, lane->wall_case_returns_before_f0107, 1,
                   "wall branch return");
        snprintf(id, sizeof(id), "lane.%u.f0108_keepout", (unsigned)i);
        expect_int(id, lane->f0108_keepout, 1, "F0108 contrast keepout");
        snprintf(id, sizeof(id), "lane.%u.no_direct_f0111", (unsigned)i);
        expect_int(id, lane->f0111_direct_call_present, 0,
                   "F0111 relation is external door-front contract");
        snprintf(id, sizeof(id), "lane.%u.anchor", (unsigned)i);
        expect_contains(id, lane->redmcsb_anchor, "DUNVIEW.C", "lane source anchor");
    }

    expect_int("lane.d0l.side", model ? model->lanes[0].side : -1, 1,
               "D0L side enum");
    expect_contains("lane.d0l.name", model ? model->lanes[0].side_name : NULL,
                    "D0L", "D0L name");
    expect_int("lane.d0l.view_square", model ? model->lanes[0].view_square : -1, 1,
               "DEFS.H M610");
    expect_int("lane.d0l.wall_zone", model ? model->lanes[0].wall_zone : -1, 716,
               "DEFS.H C716");
    expect_int("lane.d0l.depth", model ? model->lanes[0].relative_depth : -1, 0,
               "F0128 relative depth");
    expect_int("lane.d0l.lateral", model ? model->lanes[0].relative_lateral : 0, -1,
               "F0128 relative lateral");
    expect_int("lane.d0l.update_line", model ? model->lanes[0].f0128_update_line : 0,
               8536, "DUNVIEW.C:8536");
    expect_int("lane.d0l.draw_line", model ? model->lanes[0].f0128_draw_line : 0,
               8537, "DUNVIEW.C:8537");
    expect_int("lane.d0l.dispatch_start", model ? model->lanes[0].dispatcher_line_start : 0,
               7960, "DUNVIEW.C F0125");
    expect_int("lane.d0l.dispatch_end", model ? model->lanes[0].dispatcher_line_end : 0,
               8062, "DUNVIEW.C F0125");
    expect_int("lane.d0l.wall_case_line", model ? model->lanes[0].wall_case_line : 0,
               8009, "DUNVIEW.C:8009");
    expect_int("lane.d0l.thing_line", model ? model->lanes[0].thing_pass_line : 0,
               8005, "DUNVIEW.C:8005");
    expect_int("lane.d0l.ceiling_line", model ? model->lanes[0].ceiling_line : 0,
               8043, "DUNVIEW.C F0125 after switch");
    expect_int("lane.d0l.thing_before_ceiling", model ? model->lanes[0].thing_before_ceiling : 0,
               1, "D0L order");
    expect_int("lane.d0l.ceiling_before_thing", model ? model->lanes[0].ceiling_before_thing : 1,
               0, "D0L order");
    expect_int("lane.d0l.order", model ? (int)model->lanes[0].thing_pass_order : 0,
               0x0002, "C0x0002_CELL_ORDER_BACKRIGHT");

    expect_int("lane.d0r.side", model ? model->lanes[1].side : -1, 2,
               "D0R side enum");
    expect_contains("lane.d0r.name", model ? model->lanes[1].side_name : NULL,
                    "D0R", "D0R name");
    expect_int("lane.d0r.view_square", model ? model->lanes[1].view_square : -1, 2,
               "DEFS.H M611");
    expect_int("lane.d0r.wall_zone", model ? model->lanes[1].wall_zone : -1, 717,
               "DEFS.H C717");
    expect_int("lane.d0r.depth", model ? model->lanes[1].relative_depth : -1, 0,
               "F0128 relative depth");
    expect_int("lane.d0r.lateral", model ? model->lanes[1].relative_lateral : 0, 1,
               "F0128 relative lateral");
    expect_int("lane.d0r.update_line", model ? model->lanes[1].f0128_update_line : 0,
               8540, "DUNVIEW.C:8540");
    expect_int("lane.d0r.draw_line", model ? model->lanes[1].f0128_draw_line : 0,
               8541, "DUNVIEW.C:8541");
    expect_int("lane.d0r.dispatch_start", model ? model->lanes[1].dispatcher_line_start : 0,
               8064, "DUNVIEW.C F0126");
    expect_int("lane.d0r.dispatch_end", model ? model->lanes[1].dispatcher_line_end : 0,
               8162, "DUNVIEW.C F0126");
    expect_int("lane.d0r.wall_case_line", model ? model->lanes[1].wall_case_line : 0,
               8119, "DUNVIEW.C:8119");
    expect_int("lane.d0r.thing_line", model ? model->lanes[1].thing_pass_line : 0,
               8115, "DUNVIEW.C:8115");
    expect_int("lane.d0r.ceiling_line", model ? model->lanes[1].ceiling_line : 0,
               8113, "DUNVIEW.C:8113");
    expect_int("lane.d0r.thing_before_ceiling", model ? model->lanes[1].thing_before_ceiling : 1,
               0, "D0R order");
    expect_int("lane.d0r.ceiling_before_thing", model ? model->lanes[1].ceiling_before_thing : 0,
               1, "D0R order");
    expect_int("lane.d0r.order", model ? (int)model->lanes[1].thing_pass_order : 0,
               0x0001, "C0x0001_CELL_ORDER_BACKLEFT");
}

static void test_ordinals_steps_pixels_and_doors(void)
{
    const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int direct_count = 0;
    int rejection_count = 0;
    int present_count = 0;
    int c10_skips = 0;
    int c10_writes = 0;

    expect_int("ordinal.out_of_range",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_ordinal_at_pc34(6) == NULL,
               1, "ordinal accessor bounds");
    for (i = 0; model && i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        const DM1_V1_D0LD0RF0107OrdinalFlowPc34 *o =
            dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_ordinal_at_pc34(i);
        char id[80];
        snprintf(id, sizeof(id), "ordinal.%u.present", (unsigned)i);
        expect_int(id, o != NULL, 1, "ordinal accessor");
        if (!o) continue;
        direct_count += o->reaches_d0l_d0r_directly;
        rejection_count += o->expected_rejection;
        snprintf(id, sizeof(id), "ordinal.%u.rejected", (unsigned)i);
        expect_int(id, o->expected_rejection, 1, "D0L/D0R direct F0107 keepout");
        snprintf(id, sizeof(id), "ordinal.%u.direct", (unsigned)i);
        expect_int(id, o->reaches_d0l_d0r_directly, 0, "no direct D0 F0107");
        snprintf(id, sizeof(id), "ordinal.%u.anchor", (unsigned)i);
        expect_contains(id, o->redmcsb_anchor, "DUNVIEW.C", "ordinal source anchor");
    }
    expect_int("ordinal.direct_count", direct_count, 0, "all ordinals are D0 keepout");
    expect_int("ordinal.rejection_count", rejection_count, 6, "C0..C5 rejected by D0");
    expect_int("ordinal.c0.slot", model ? model->ordinals[0].ordinal_slot : 0, 4,
               "M551 C0");
    expect_int("ordinal.c1.slot", model ? model->ordinals[1].ordinal_slot : 0, 6,
               "M553 C1");
    expect_int("ordinal.c2.slot", model ? model->ordinals[2].ordinal_slot : 0, 5,
               "M552 C2");
    expect_int("ordinal.c3.view_wall", model ? model->ordinals[3].nearest_source_view_wall : 0,
               12, "M585 D1L right source");
    expect_int("ordinal.c4.view_wall", model ? model->ordinals[4].nearest_source_view_wall : 0,
               13, "M586 D1R left source");
    expect_int("ordinal.c5.view_wall", model ? model->ordinals[5].nearest_source_view_wall : 0,
               14, "M587 D1C contrast source");
    expect_contains("ordinal.c0.name", model ? model->ordinals[0].slot_name : NULL,
                    "C0", "C0 ordinal chain");
    expect_contains("ordinal.c5.name", model ? model->ordinals[5].slot_name : NULL,
                    "C5", "C5 ordinal chain");

    expect_int("step.out_of_range",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_step_at_pc34(9) == NULL,
               1, "step accessor bounds");
    for (i = 0; i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_STEP_COUNT_PC34; ++i) {
        const DM1_V1_D0LD0RF0107StepPc34 *step =
            dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_step_at_pc34(i);
        char id[80];
        snprintf(id, sizeof(id), "step.%u.present", (unsigned)i);
        expect_int(id, step != NULL, 1, "step accessor");
        if (!step) continue;
        present_count += step->expected_present;
        snprintf(id, sizeof(id), "step.%u.order", (unsigned)i);
        expect_int(id, step->order_index, (int)i, "step order stable");
        snprintf(id, sizeof(id), "step.%u.expected", (unsigned)i);
        expect_int(id, step->expected_present, 1, "source-lock step present");
        snprintf(id, sizeof(id), "step.%u.anchor", (unsigned)i);
        expect_contains(id, step->redmcsb_anchor, "DUNVIEW.C", "step anchor");
    }
    expect_int("steps.present_count", present_count, 9, "all D0 F0107 contract steps");
    expect_int("step0.kind",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_step_at_pc34(0)->step,
               DM1_V1_D0L_D0R_F0107_STEP_F0128_DISPATCH_D0L_PC34,
               "DUNVIEW.C:8536");
    expect_int("step8.kind",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_step_at_pc34(8)->step,
               DM1_V1_D0L_D0R_F0107_STEP_F0111_PARTLY_OPEN_RELATION_PC34,
               "DUNVIEW.C F0111");

    for (i = 0; model && i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        const DM1_V1_D0LD0RF0107PixelPc34 *p = &model->pixels[i];
        char id[80];
        snprintf(id, sizeof(id), "pixel.%u.ordinal", (unsigned)i);
        expect_int(id, p->ordinal_index, (int)i, "C0..C5 pixel ordinal chain");
        snprintf(id, sizeof(id), "pixel.%u.after", (unsigned)i);
        expect_int(id, p->after,
                   dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_blend_pixel_pc34(
                       p->before, p->source, 10),
                   "DUNVIEW.C:3922 C10 transparent blit");
        snprintf(id, sizeof(id), "pixel.%u.skip_xor_write", (unsigned)i);
        expect_int(id, p->transparent_skip + p->writes_pixel, 1,
                   "each pixel either skips or writes");
        snprintf(id, sizeof(id), "pixel.%u.anchor", (unsigned)i);
        expect_contains(id, p->anchor, "F0107", "pixel anchor");
        c10_skips += p->transparent_skip;
        c10_writes += p->writes_pixel;
    }
    expect_int("pixel.skip_count", c10_skips, 3, "three C10 transparent skips");
    expect_int("pixel.write_count", c10_writes, 3, "three non-C10 writes");

    for (i = 0; model && i < DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_DOOR_STATE_COUNT_PC34; ++i) {
        const DM1_V1_D0LD0RF0107DoorRelationPc34 *d = &model->door_states[i];
        char id[80];
        snprintf(id, sizeof(id), "door.%u.state", (unsigned)i);
        expect_int(id, d->door_state, (int)i + 1, "partly-open F0111 states");
        snprintf(id, sizeof(id), "door.%u.partly_open", (unsigned)i);
        expect_int(id, d->f0111_partly_open, 1, "DUNVIEW.C:4308-4325");
        snprintf(id, sizeof(id), "door.%u.c10", (unsigned)i);
        expect_int(id, d->horizontal_half_blit_uses_c10, 1, "DUNVIEW.C:4322-4324");
        snprintf(id, sizeof(id), "door.%u.mask", (unsigned)i);
        expect_int(id, d->mask0x4000_shift_applied, 1, "DUNVIEW.C:4325");
        snprintf(id, sizeof(id), "door.%u.shared", (unsigned)i);
        expect_int(id, d->shares_f0107_c10_transparency, 1,
                   "same C10 preservation relationship as F0107");
        snprintf(id, sizeof(id), "door.%u.anchor", (unsigned)i);
        expect_contains(id, d->anchor, "DUNVIEW.C", "door relation anchor");
    }
}

static void test_alcove_guard_evidence_and_hash(void)
{
    const char *e = dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_source_evidence_pc34();
    const char *d = dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_disjointness_note_pc34();
    uint32_t hash = dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_deterministic_hash_pc34();

    expect_int("alcove.zero.with_bits",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_returns_alcove_pc34(
                   0, DM1_V1_D0L_D0R_F0107_ALCOVE_CELL_CONTENT_MASK_PC34),
               0, "DUNVIEW.C:3568 zero ordinal guard");
    expect_int("alcove.nonzero.no_bits",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_returns_alcove_pc34(7, 0u),
               0, "DUNVIEW.C:3589 F0149 false");
    expect_int("alcove.nonzero.with_bits",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_returns_alcove_pc34(
                   7, DM1_V1_D0L_D0R_F0107_ALCOVE_CELL_CONTENT_MASK_PC34),
               1, "DUNVIEW.C:3933 returns alcove boolean");
    expect_int("blend.transparent",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_blend_pixel_pc34(0xaa, 10, 10),
               0xaa, "DEFS.H:2088 C10 preserves destination");
    expect_int("blend.opaque",
               dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_blend_pixel_pc34(0xaa, 0x51, 10),
               0x51, "F0107 opaque ornament pixel writes");

    expect_contains("evidence.f0107", e, "DUNVIEW.C F0107:3502-3938",
                    "required F0107 anchor");
    expect_contains("evidence.f0125", e, "F0125:7960-8062",
                    "D0L dispatch body");
    expect_contains("evidence.f0126", e, "F0126:8064-8162",
                    "D0R dispatch body");
    expect_contains("evidence.f0128", e, "F0128:8536-8541",
                    "D0L then D0R dispatch");
    expect_contains("evidence.f0108", e, "F0108:3940-4011",
                    "F0108 keepout contrast");
    expect_contains("evidence.f0115", e, "F0115:4547-4581",
                    "F0115 ordering");
    expect_contains("evidence.f0111", e, "F0111:4218-4337",
                    "F0111 relation");
    expect_contains("evidence.c10", e, "DEFS.H:2088",
                    "C10 transparency");
    expect_contains("evidence.m550", e, "M550/M551/M552/M553",
                    "ordinal slots");
    expect_contains("evidence.m610", e, "M610/M611",
                    "D0 view squares");
    expect_contains("evidence.c716", e, "C716/C717",
                    "D0 wall zones");
    expect_contains("evidence.cell_order", e, "DEFS.H:4139-4153",
                    "cell-order band");
    expect_contains("evidence.no_direct", e, "do not directly call F0107",
                    "D0 F0107 keepout");

    expect_contains("disjoint.d0", d, "D0L/D0R F0107",
                    "disjointness note");
    expect_contains("disjoint.d1c", d, "D1C F0107",
                    "does not duplicate D1C gate");
    expect_contains("disjoint.f0108", d, "F0108",
                    "D0 F0108 relation");
    expect_contains("disjoint.f0115", d, "F0115",
                    "D0 F0115 relation");
    expect_contains("disjoint.f0111", d, "F0111",
                    "D0 F0111 relation");
    expect_contains("disjoint.synthetic", d, "synthetic framebuffer",
                    "contract-only framebuffer");
    expect_contains("disjoint.no_dos", d, "original DOS pixel parity",
                    "no DOS pixel parity claim");
    expect_contains("disjoint.no_assets", d, "GRAPHICS.DAT",
                    "asset-free claim");

    expect_int("hash.nonzero", hash != 0u, 1, "deterministic hash exists");
    expect_u32("hash.stable", hash, 0x759eea11u,
               "deterministic D0L/D0R F0107 wall-ornament source-lock hash");
}

int main(void)
{
    test_model_core();
    test_lanes();
    test_ordinals_steps_pixels_and_doors();
    test_alcove_guard_evidence_and_hash();

    if (g_failures) {
        printf("FAIL DM1_V1_VIEWPORT_D0L_D0R_F0107_WALL_ORNAMENT_PC34_COMPAT assertions=%d failures=%d deterministic_hash=0x%08x\n",
               g_assertions, g_failures,
               (unsigned)dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_deterministic_hash_pc34());
        return 1;
    }
    printf("DM1_V1_VIEWPORT_D0L_D0R_F0107_WALL_ORNAMENT_PC34_COMPAT_OK assertions=%d failures=0 deterministic_hash=0x%08x\n",
           g_assertions,
           (unsigned)dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_deterministic_hash_pc34());
    return 0;
}
