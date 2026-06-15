#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_pc34_compat.h"

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

static void test_core(void)
{
    DM1_V1_D2LD2RF0107WallOrnamentModelPc34 built;
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *m =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34();

    expect_int("builder.null",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_builder_pc34(NULL),
               0, "builder guard");
    expect_int("builder.ok",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_builder_pc34(&built),
               1, "builder deterministic");
    expect_int("model.present", m != NULL, 1, "model accessor");
    if (!m) return;
    expect_int("hash.built", built.deterministic_hash == m->deterministic_hash, 1,
               "builder hash stable");
    expect_int("hash.null", dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_hash_model_pc34(NULL),
               0, "hash null guard");
    expect_int("hash.accessor",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_deterministic_hash_pc34() ==
                   m->deterministic_hash,
               1, "hash accessor");

    expect_int("view_square.d2l", m->view_square_d2l, 7, "DEFS.H:2603 M604");
    expect_int("view_square.d2r", m->view_square_d2r, 8, "DEFS.H:2604 M605");
    expect_int("view_wall.d2l.right", m->view_wall_d2l_right, 7, "DEFS.H:2703 M580");
    expect_int("view_wall.d2l.front", m->view_wall_d2l_front, 9, "DEFS.H:2705 M582");
    expect_int("view_wall.d2r.left", m->view_wall_d2r_left, 8, "DEFS.H:2704 M581");
    expect_int("view_wall.d2r.front", m->view_wall_d2r_front, 11, "DEFS.H:2707 M584");
    expect_int("zone.wall.d2l", m->wall_zone_d2l, 710, "DEFS.H:4050 C710");
    expect_int("zone.wall.d2r", m->wall_zone_d2r, 711, "DEFS.H:4051 C711");
    expect_int("zone.door.d2l", m->door_zone_d2l, 3750, "DEFS.H:4255 M627");
    expect_int("zone.door.d2r", m->door_zone_d2r, 3770, "DEFS.H:4257 M629");
    expect_int("c10", m->c10_transparent_color, 10, "DEFS.H:2088 C10");
    expect_int("slot.m550", m->first_thing_slot, 2, "DEFS.H:2549 M550");
    expect_int("slot.m551", m->right_wall_ornament_slot, 4, "DEFS.H:2551 M551");
    expect_int("slot.m552", m->front_wall_ornament_slot, 5, "DEFS.H:2552 M552");
    expect_int("slot.m553", m->left_wall_ornament_slot, 6, "DEFS.H:2553 M553");
    expect_int("dispatch.d2l_before_d2r", m->f0128_d2l_before_d2r, 1,
               "DUNVIEW.C:8512-8517");
    expect_int("f0108.baseline", m->f0108_baseline_before_f0107_contract, 1,
               "DUNVIEW.C F0108:3940-4011");
    expect_int("alcove.zero_false", m->f0107_zero_ordinal_returns_false, 1,
               "DUNVIEW.C:3571-3573");
    expect_int("alcove.non_alcove_false", m->f0107_non_alcove_returns_false, 1,
               "DUNVIEW.C:3589");
    expect_int("alcove.true", m->f0107_alcove_returns_true, 1, "DUNVIEW.C:3933");
    expect_int("f0107.c10", m->f0107_blit_uses_c10, 1, "DUNVIEW.C:3922");
    expect_int("c10.preserve", m->c10_preserves_destination, 1, "C10 preservation");
    expect_int("f0111.open_reject", m->f0111_open_rejects_blit, 1, "DUNVIEW.C:4248");
    expect_int("f0111.non_open", m->f0111_non_open_accepts_blit, 1, "DUNVIEW.C:4334");
    expect_int("f0111.partly", m->f0111_partly_open_uses_c10, 1, "DUNVIEW.C:4322-4324");
    expect_int("ordinals.c0_c5", m->all_call_sites_accept_c0_to_c5, 1,
               "DUNGEON.C F0172 sensor ordinals");
    expect_int("cells.front", m->front_cells_are_0_1, 1, "requested mapping");
    expect_int("cells.back", m->back_cells_are_2_3, 1, "requested mapping");
    expect_int("contract", m->source_locked_contract_only, 1, "contract only");
    expect_int("no_dos", m->no_original_dos_pixel_parity, 1, "no DOS parity claim");
    expect_int("no_assets", m->no_graphics_dat_reads, 1, "asset-free");
    expect_int("disjoint", m->disjoint_from_d0l_d0r_and_d1c, 1, "non-duplicative");
    expect_int("helper", m->helper_f0107_slot_constants_reused, 1, "helper constants");
}

static void test_sides_calls_steps(void)
{
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *m =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int accepted_calls = 0;
    int front_alcove_calls = 0;
    int present_steps = 0;

    expect_int("side.bounds",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_side_at_pc34(2) == NULL,
               1, "side bounds");
    expect_int("call.bounds",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_call_at_pc34(4) == NULL,
               1, "call bounds");
    expect_int("step.bounds",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_step_at_pc34(10) == NULL,
               1, "step bounds");

    for (i = 0; m && i < DM1_V1_D2L_D2R_F0107_SIDE_COUNT_PC34; ++i) {
        const DM1_V1_D2LD2RF0107SideSpecPc34 *s =
            dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_side_at_pc34(i);
        char id[96];
        snprintf(id, sizeof(id), "side.%u.present", (unsigned)i);
        expect_int(id, s != NULL, 1, "side accessor");
        if (!s) continue;
        snprintf(id, sizeof(id), "side.%u.ptr", (unsigned)i);
        expect_int(id, s == &m->sides[i], 1, "side pointer");
        snprintf(id, sizeof(id), "side.%u.depth", (unsigned)i);
        expect_int(id, s->relative_depth, 2, "D2 relative depth");
        snprintf(id, sizeof(id), "side.%u.anchor", (unsigned)i);
        expect_contains(id, s->redmcsb_anchor, "DUNVIEW.C", "side anchor");
    }
    expect_contains("side.d2l.name", m ? m->sides[0].name : NULL, "D2L", "D2L");
    expect_contains("side.d2r.name", m ? m->sides[1].name : NULL, "D2R", "D2R");
    expect_int("side.d2l.lateral", m ? m->sides[0].relative_lateral : 0, -1, "D2L lateral");
    expect_int("side.d2r.lateral", m ? m->sides[1].relative_lateral : 0, 1, "D2R lateral");
    expect_int("side.d2l.update", m ? m->sides[0].f0128_update_line : 0, 8512, "DUNVIEW.C:8512");
    expect_int("side.d2l.draw", m ? m->sides[0].f0128_draw_line : 0, 8513, "DUNVIEW.C:8513");
    expect_int("side.d2r.update", m ? m->sides[1].f0128_update_line : 0, 8516, "DUNVIEW.C:8516");
    expect_int("side.d2r.draw", m ? m->sides[1].f0128_draw_line : 0, 8517, "DUNVIEW.C:8517");
    expect_int("side.d2l.body_start", m ? m->sides[0].body_start_line : 0, 6900, "F0119");
    expect_int("side.d2l.body_end", m ? m->sides[0].body_end_line : 0, 7049, "F0119");
    expect_int("side.d2r.body_start", m ? m->sides[1].body_start_line : 0, 7051, "F0120");
    expect_int("side.d2r.body_end", m ? m->sides[1].body_end_line : 0, 7225, "F0120");
    expect_int("side.d2l.wall_case", m ? m->sides[0].wall_case_line : 0, 6945, "wall case");
    expect_int("side.d2r.wall_case", m ? m->sides[1].wall_case_line : 0, 7096, "wall case");
    expect_int("side.d2l.wall_draw", m ? m->sides[0].wall_draw_line : 0, 6963, "wall draw");
    expect_int("side.d2r.wall_draw", m ? m->sides[1].wall_draw_line : 0, 7114, "wall draw");
    expect_int("side.d2l.side_f0107", m ? m->sides[0].side_f0107_line : 0, 6968, "F0107");
    expect_int("side.d2l.front_f0107", m ? m->sides[0].front_f0107_line : 0, 6969, "F0107");
    expect_int("side.d2r.side_f0107", m ? m->sides[1].side_f0107_line : 0, 7119, "F0107");
    expect_int("side.d2r.front_f0107", m ? m->sides[1].front_f0107_line : 0, 7120, "F0107");
    expect_int("side.d2l.order", m ? (int)m->sides[0].corridor_order : 0, 0x3421, "DEFS.H:2676");
    expect_int("side.d2r.order", m ? (int)m->sides[1].corridor_order : 0, 0x4312, "DEFS.H:2677");
    expect_int("side.d2l.door1", m ? (int)m->sides[0].door_pass1_order : 0, 0x0218, "DEFS.H:2669");
    expect_int("side.d2r.door1", m ? (int)m->sides[1].door_pass1_order : 0, 0x0128, "DEFS.H:2668");
    expect_int("side.d2l.door2", m ? (int)m->sides[0].door_pass2_order : 0, 0x0349, "DEFS.H:2672");
    expect_int("side.d2r.door2", m ? (int)m->sides[1].door_pass2_order : 0, 0x0439, "DEFS.H:2675");

    for (i = 0; m && i < DM1_V1_D2L_D2R_F0107_CALL_COUNT_PC34; ++i) {
        const DM1_V1_D2LD2RF0107CallPc34 *c =
            dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_call_at_pc34(i);
        char id[96];
        snprintf(id, sizeof(id), "call.%u.present", (unsigned)i);
        expect_int(id, c != NULL, 1, "call accessor");
        if (!c) continue;
        accepted_calls += c->accepts_c0_to_c5;
        front_alcove_calls += c->alcove_enables_f0115;
        snprintf(id, sizeof(id), "call.%u.index", (unsigned)i);
        expect_int(id, c->call_index, (int)i, "call index");
        snprintf(id, sizeof(id), "call.%u.zone", (unsigned)i);
        expect_int(id, c->zone, 1004 + 2 * 15 + c->view_wall, "wall ornament zone math");
        snprintf(id, sizeof(id), "call.%u.anchor", (unsigned)i);
        expect_contains(id, c->redmcsb_anchor, "DUNVIEW.C", "call anchor");
    }
    expect_int("call.accepted_count", accepted_calls, 4, "all four D2 call sites");
    expect_int("call.alcove_count", front_alcove_calls, 2, "front calls gate F0115");
    expect_int("call.0.slot", m ? m->calls[0].aspect_slot : 0, 4, "M551");
    expect_int("call.1.slot", m ? m->calls[1].aspect_slot : 0, 5, "M552");
    expect_int("call.2.slot", m ? m->calls[2].aspect_slot : 0, 6, "M553");
    expect_int("call.3.slot", m ? m->calls[3].aspect_slot : 0, 5, "M552");

    for (i = 0; i < DM1_V1_D2L_D2R_F0107_STEP_COUNT_PC34; ++i) {
        const DM1_V1_D2LD2RF0107StepPc34 *step =
            dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_step_at_pc34(i);
        char id[96];
        snprintf(id, sizeof(id), "step.%u.present", (unsigned)i);
        expect_int(id, step != NULL, 1, "step accessor");
        if (!step) continue;
        present_steps += step->expected_present;
        snprintf(id, sizeof(id), "step.%u.order", (unsigned)i);
        expect_int(id, step->order_index, (int)i, "step order");
        snprintf(id, sizeof(id), "step.%u.anchor", (unsigned)i);
        expect_contains(id, step->redmcsb_anchor, "DUNVIEW.C", "step anchor");
    }
    expect_int("steps.present_count", present_steps, 10, "all source-lock steps");
}

static void test_cells_zones_ordinals_pixels_doors(void)
{
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *m =
        dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int front = 0;
    int back = 0;
    int skips = 0;
    int writes = 0;
    int open_rejects = 0;
    int non_open_accepts = 0;
    int partly_c10 = 0;

    expect_int("cell.bounds",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_cell_at_pc34(4) == NULL,
               1, "cell bounds");
    expect_int("zone.bounds",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_zone_at_pc34(8) == NULL,
               1, "zone bounds");

    for (i = 0; m && i < DM1_V1_D2L_D2R_F0107_CELL_COUNT_PC34; ++i) {
        const DM1_V1_D2LD2RF0107CellPc34 *cell =
            dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_cell_at_pc34(i);
        char id[96];
        snprintf(id, sizeof(id), "cell.%u.present", (unsigned)i);
        expect_int(id, cell != NULL, 1, "cell accessor");
        if (!cell) continue;
        front += cell->is_front;
        back += cell->is_back;
        snprintf(id, sizeof(id), "cell.%u.index", (unsigned)i);
        expect_int(id, cell->requested_cell_index, (int)i, "requested cell mapping");
        snprintf(id, sizeof(id), "cell.%u.xor", (unsigned)i);
        expect_int(id, cell->is_front + cell->is_back, 1, "front/back exclusive");
        snprintf(id, sizeof(id), "cell.%u.anchor", (unsigned)i);
        expect_contains(id, cell->redmcsb_anchor, "DEFS.H", "cell source anchor");
    }
    expect_int("cells.front_count", front, 2, "cell 0/1 are front");
    expect_int("cells.back_count", back, 2, "cell 2/3 are back");
    expect_contains("cell.0.name", m ? m->cells[0].cell_name : NULL, "FRONT_LEFT", "cell 0");
    expect_contains("cell.1.name", m ? m->cells[1].cell_name : NULL, "FRONT_RIGHT", "cell 1");
    expect_contains("cell.2.name", m ? m->cells[2].cell_name : NULL, "BACK_LEFT", "cell 2");
    expect_contains("cell.3.name", m ? m->cells[3].cell_name : NULL, "BACK_RIGHT", "cell 3");
    expect_int("cell.0.nibble", m ? m->cells[0].f0115_nibble : 0, 4, "FRONTLEFT nibble");
    expect_int("cell.1.nibble", m ? m->cells[1].f0115_nibble : 0, 3, "FRONTRIGHT nibble");
    expect_int("cell.2.nibble", m ? m->cells[2].f0115_nibble : 0, 1, "BACKLEFT nibble");
    expect_int("cell.3.nibble", m ? m->cells[3].f0115_nibble : 0, 2, "BACKRIGHT nibble");

    for (i = 0; m && i < DM1_V1_D2L_D2R_F0107_ZONE_COUNT_PC34; ++i) {
        const DM1_V1_D2LD2RF0107ZonePc34 *z =
            dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_zone_at_pc34(i);
        char id[96];
        snprintf(id, sizeof(id), "zone.%u.present", (unsigned)i);
        expect_int(id, z != NULL, 1, "zone accessor");
        if (!z) continue;
        snprintf(id, sizeof(id), "zone.%u.expected", (unsigned)i);
        expect_int(id, z->expected_zone, z->base_zone + z->coordinate_set * z->stride + z->view_index,
                   "zone = base + coordinate_set * stride + view");
        snprintf(id, sizeof(id), "zone.%u.anchor", (unsigned)i);
        expect_contains(id, z->redmcsb_anchor, "DUNVIEW.C", "zone anchor");
    }
    expect_int("zone.wall.d2l.right", m ? m->zones[0].expected_zone : 0, 1041, "M580 zone");
    expect_int("zone.wall.d2l.front", m ? m->zones[1].expected_zone : 0, 1043, "M582 zone");
    expect_int("zone.wall.d2r.left", m ? m->zones[2].expected_zone : 0, 1042, "M581 zone");
    expect_int("zone.wall.d2r.front", m ? m->zones[3].expected_zone : 0, 1045, "M584 zone");
    expect_int("zone.g0206.d2l", m ? m->zones[4].expected_zone : 0, 1505, "G0206 D2L");
    expect_int("zone.g0206.d2r", m ? m->zones[5].expected_zone : 0, 1507, "G0206 D2R");
    expect_int("zone.g0207", m ? m->zones[6].expected_zone : 0, 2010, "G0207");
    expect_int("zone.g0208", m ? m->zones[7].expected_zone : 0, 1956, "G0208");

    for (i = 0; m && i < DM1_V1_D2L_D2R_F0107_ORDINAL_COUNT_PC34; ++i) {
        size_t call_index;
        char id[96];
        snprintf(id, sizeof(id), "ordinal.%u.sensor", (unsigned)i);
        expect_int(id, m->ordinals[i].sensor_ordinal, (int)i + 1, "one-based F0107 ordinal");
        snprintf(id, sizeof(id), "ordinal.%u.all", (unsigned)i);
        expect_int(id, m->ordinals[i].accepted_at_all_call_sites, 1, "C0..C5 all D2 call sites");
        for (call_index = 0; call_index < DM1_V1_D2L_D2R_F0107_CALL_COUNT_PC34; ++call_index) {
            snprintf(id, sizeof(id), "ordinal.%u.call.%u", (unsigned)i, (unsigned)call_index);
            expect_int(id,
                       dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
                           (int)call_index, (int)i),
                       1, "C0..C5 sensor position acceptance");
        }
    }
    expect_int("ordinal.reject.call_low",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(-1, 0),
               0, "call bounds");
    expect_int("ordinal.reject.call_high",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(4, 0),
               0, "call bounds");
    expect_int("ordinal.reject.ordinal_low",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(0, -1),
               0, "ordinal bounds");
    expect_int("ordinal.reject.ordinal_high",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(0, 6),
               0, "ordinal bounds");

    expect_int("alcove.zero",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_returns_alcove_pc34(0, true),
               0, "zero ordinal");
    expect_int("alcove.no",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_returns_alcove_pc34(7, false),
               0, "non-alcove");
    expect_int("alcove.yes",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_returns_alcove_pc34(7, true),
               1, "alcove");
    expect_int("blend.transparent",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_blend_pixel_pc34(0xaa, 10, 10),
               0xaa, "C10 preserves");
    expect_int("blend.opaque",
               dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_blend_pixel_pc34(0xaa, 0x51, 10),
               0x51, "opaque writes");

    for (i = 0; m && i < DM1_V1_D2L_D2R_F0107_PIXEL_COUNT_PC34; ++i) {
        const DM1_V1_D2LD2RF0107PixelPc34 *p = &m->pixels[i];
        char id[96];
        snprintf(id, sizeof(id), "pixel.%u.after", (unsigned)i);
        expect_int(id, p->after,
                   dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_blend_pixel_pc34(
                       p->before, p->source, 10),
                   "C10 blend");
        snprintf(id, sizeof(id), "pixel.%u.xor", (unsigned)i);
        expect_int(id, p->transparent_skip + p->writes_pixel, 1, "skip/write exclusive");
        snprintf(id, sizeof(id), "pixel.%u.anchor", (unsigned)i);
        expect_contains(id, p->anchor, "F0107", "pixel anchor");
        skips += p->transparent_skip;
        writes += p->writes_pixel;
    }
    expect_int("pixel.skip_count", skips, 3, "three C10 skips");
    expect_int("pixel.write_count", writes, 3, "three opaque writes");

    for (i = 0; m && i < DM1_V1_D2L_D2R_F0107_DOOR_STATE_COUNT_PC34; ++i) {
        const DM1_V1_D2LD2RF0107DoorStatePc34 *d = &m->door_states[i];
        char id[96];
        snprintf(id, sizeof(id), "door.%u.state", (unsigned)i);
        expect_int(id, d->door_state, (int)i, "door state");
        snprintf(id, sizeof(id), "door.%u.reject_or_draw", (unsigned)i);
        expect_int(id, d->open_rejects_blit + d->draws_c10_blit, 1, "reject/draw exclusive");
        snprintf(id, sizeof(id), "door.%u.d2l_eq_d2r", (unsigned)i);
        expect_int(id, d->accepted_for_d2l, d->accepted_for_d2r, "same on D2L/D2R");
        snprintf(id, sizeof(id), "door.%u.anchor", (unsigned)i);
        expect_contains(id, d->redmcsb_anchor, "DUNVIEW.C", "door anchor");
        open_rejects += d->open_rejects_blit;
        non_open_accepts += d->draws_c10_blit;
        partly_c10 += d->partly_open_half_blit_uses_c10;
    }
    expect_int("door.open_rejects", open_rejects, 1, "C0 open rejects");
    expect_int("door.non_open_accepts", non_open_accepts, 5, "C1..C5 draw");
    expect_int("door.partly_c10", partly_c10, 3, "C1..C3 partly-open C10");
}

static void test_evidence_and_hash(void)
{
    const char *e = dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_source_evidence_pc34();
    const char *d = dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_disjointness_note_pc34();
    uint32_t hash = dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_deterministic_hash_pc34();

    expect_contains("evidence.f0107", e, "DUNVIEW.C F0107:3502-3938", "F0107 anchor");
    expect_contains("evidence.f0119", e, "F0119:6900-7049", "D2L anchor");
    expect_contains("evidence.f0120", e, "F0120:7051-7225", "D2R anchor");
    expect_contains("evidence.f0128", e, "F0128:8503-8517", "dispatch anchor");
    expect_contains("evidence.f0108", e, "F0108:3940-4011", "F0108 anchor");
    expect_contains("evidence.f0111", e, "F0111:4218-4337", "F0111 anchor");
    expect_contains("evidence.f0115", e, "F0115:4547-4581", "F0115 anchor");
    expect_contains("evidence.f0163", e, "F0163:1769-1838", "F0163 anchor");
    expect_contains("evidence.f0164", e, "F0164:1840-1905", "F0164 anchor");
    expect_contains("evidence.f0172", e, "F0172:2466-2523", "F0172 anchor");
    expect_contains("evidence.g0206", e, "G0206", "G0206");
    expect_contains("evidence.g0207", e, "G0207", "G0207");
    expect_contains("evidence.g0208", e, "G0208", "G0208");
    expect_contains("evidence.m580", e, "M580", "M580");
    expect_contains("evidence.m584", e, "M584", "M584");
    expect_contains("evidence.c10", e, "DEFS.H:2088", "C10");
    expect_contains("evidence.cell_order", e, "C0x3421", "cell order");
    expect_contains("disjoint.d2", d, "D2L/D2R F0107", "D2 route");
    expect_contains("disjoint.d0", d, "D0L/D0R F0107", "D0 disjoint");
    expect_contains("disjoint.d1c", d, "D1C F0107", "D1C disjoint");
    expect_contains("disjoint.c0c5", d, "C0..C5", "ordinals");
    expect_contains("disjoint.no_assets", d, "GRAPHICS.DAT", "asset-free");
    expect_contains("disjoint.no_dos", d, "original DOS pixel parity", "no DOS parity");
    expect_int("hash.nonzero", hash != 0u, 1, "hash exists");
    expect_u32("hash.stable", hash, 0x818f1089u,
               "deterministic D2L/D2R F0107 wall-ornament hash");
}

int main(void)
{
    test_core();
    test_sides_calls_steps();
    test_cells_zones_ordinals_pixels_doors();
    test_evidence_and_hash();

    if (g_failures) {
        printf("FAIL DM1_V1_VIEWPORT_D2L_D2R_F0107_WALL_ORNAMENT_PC34_COMPAT assertions=%d failures=%d deterministic_hash=0x%08x\n",
               g_assertions, g_failures,
               (unsigned)dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_deterministic_hash_pc34());
        return 1;
    }
    printf("DM1_V1_VIEWPORT_D2L_D2R_F0107_WALL_ORNAMENT_PC34_COMPAT_OK assertions=%d failures=0 deterministic_hash=0x%08x\n",
           g_assertions,
           (unsigned)dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_deterministic_hash_pc34());
    return 0;
}
