#include "firestaff/dm1/v1/viewport/d1l_d1r_f0107_wall_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

#define PROBE_ASSERT(id, expr)                                                   \
    do {                                                                         \
        ++g_assertions;                                                          \
        if (!(expr)) {                                                           \
            printf("FAIL %s\n", (id));                                           \
            ++g_failures;                                                        \
        }                                                                        \
    } while (0)

#define PROBE_ASSERT_EQ(id, got, want)                                           \
    do {                                                                         \
        int probe_got__ = (int)(got);                                            \
        int probe_want__ = (int)(want);                                          \
        ++g_assertions;                                                          \
        if (probe_got__ != probe_want__) {                                       \
            printf("FAIL %s got=%d want=%d\n", (id), probe_got__, probe_want__); \
            ++g_failures;                                                        \
        }                                                                        \
    } while (0)

#define PROBE_ASSERT_U32(id, got, want)                                          \
    do {                                                                         \
        uint32_t probe_got__ = (uint32_t)(got);                                  \
        uint32_t probe_want__ = (uint32_t)(want);                                \
        ++g_assertions;                                                          \
        if (probe_got__ != probe_want__) {                                       \
            printf("FAIL %s got=0x%08x want=0x%08x\n",                          \
                   (id), (unsigned)probe_got__, (unsigned)probe_want__);         \
            ++g_failures;                                                        \
        }                                                                        \
    } while (0)

static int contains(const char *haystack, const char *needle)
{
    return haystack && needle && strstr(haystack, needle) != NULL;
}

static int count_marked(const uint8_t *framebuffer, uint8_t marker)
{
    int count = 0;
    size_t i;

    for (i = 0;
         i < (size_t)DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_WIDTH_PC34 *
                 (size_t)DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_HEIGHT_PC34;
         ++i) {
        if (framebuffer[i] == marker) ++count;
    }
    return count;
}

static int count_marked_in_rect(const uint8_t *framebuffer,
                                int x,
                                int y,
                                int width,
                                int height)
{
    int count = 0;
    int yy;

    for (yy = y; yy < y + height; ++yy) {
        int xx;
        for (xx = x; xx < x + width; ++xx) {
            size_t offset =
                (size_t)yy * DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_WIDTH_PC34 + (size_t)xx;
            if (framebuffer[offset] != 0) ++count;
        }
    }
    return count;
}

static void test_model_core(void)
{
    DM1_V1_D1LD1RF0107WallOrnamentModelPc34 built;
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *m =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34();

    PROBE_ASSERT("builder.null",
                 !dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_builder_pc34(NULL));
    PROBE_ASSERT("builder.ok",
                 dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_builder_pc34(&built));
    PROBE_ASSERT("model.present", m != NULL);
    if (!m) return;

    PROBE_ASSERT_EQ("hash.null",
                    dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_hash_model_pc34(NULL), 0);
    PROBE_ASSERT_EQ("hash.builder_matches_default",
                    built.deterministic_hash == m->deterministic_hash, 1);
    PROBE_ASSERT_EQ("hash.accessor_matches",
                    dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_deterministic_hash_pc34() ==
                        m->deterministic_hash,
                    1);

    PROBE_ASSERT_EQ("framebuffer.width", m->framebuffer_width, 320);
    PROBE_ASSERT_EQ("framebuffer.height", m->framebuffer_height, 200);
    PROBE_ASSERT_EQ("viewport.width", m->viewport_width, 224);
    PROBE_ASSERT_EQ("viewport.height", m->viewport_height, 136);
    PROBE_ASSERT_EQ("c10", m->c10_transparent_color, 10);
    PROBE_ASSERT_EQ("zone.base", m->wall_ornament_zone_base, 1004);
    PROBE_ASSERT_EQ("zone.stride", m->wall_ornament_zone_stride, 15);
    PROBE_ASSERT_EQ("zone.coord_set", m->wall_ornament_coordinate_set, 2);
    PROBE_ASSERT_EQ("zone.d1l.f0107", m->d1l_wall_ornament_zone, 1046);
    PROBE_ASSERT_EQ("zone.d1r.f0107", m->d1r_wall_ornament_zone, 1047);
    PROBE_ASSERT_EQ("slot.m550", m->m550_first_thing_slot, 2);
    PROBE_ASSERT_EQ("slot.m551", m->m551_right_wall_ornament_slot, 4);
    PROBE_ASSERT_EQ("slot.m552", m->m552_front_wall_ornament_slot, 5);
    PROBE_ASSERT_EQ("slot.m553", m->m553_left_wall_ornament_slot, 6);
    PROBE_ASSERT_EQ("dispatch.d1l_before_d1r", m->f0128_d1l_before_d1r, 1);
    PROBE_ASSERT_EQ("dispatch.d1r_before_d1c", m->f0128_d1r_before_d1c, 1);
    PROBE_ASSERT_EQ("calls.direct_f0107", m->direct_f0107_call_count, 2);
    PROBE_ASSERT_EQ("calls.side", m->side_ornament_call_count, 2);
    PROBE_ASSERT_EQ("calls.front", m->front_ornament_call_count, 0);
    PROBE_ASSERT_EQ("alcove.zero_false", m->f0107_zero_ordinal_returns_false, 1);
    PROBE_ASSERT_EQ("alcove.non_alcove_false", m->f0107_non_alcove_returns_false, 1);
    PROBE_ASSERT_EQ("alcove.true", m->f0107_alcove_returns_true, 1);
    PROBE_ASSERT_EQ("f0107.c10", m->f0107_blit_uses_c10, 1);
    PROBE_ASSERT_EQ("c10.preserve", m->c10_transparent_preserves_destination, 1);
    PROBE_ASSERT_EQ("ordinals.c0_c5", m->c0_to_c5_ordinals_pinned, 1);
    PROBE_ASSERT_EQ("baseline.f0108", m->f0108_baseline_pinned, 1);
    PROBE_ASSERT_EQ("cell_order.f0115", m->f0115_cell_order_pinned, 1);
    PROBE_ASSERT_EQ("probe.collisions", m->synthetic_probe_collision_count, 0);
    PROBE_ASSERT_EQ("contract.only", m->source_locked_contract_only, 1);
    PROBE_ASSERT_EQ("contract.no_dos", m->no_original_dos_pixel_parity, 1);
    PROBE_ASSERT_EQ("contract.no_assets", m->no_graphics_dat_reads, 1);
}

static void test_lanes_steps(void)
{
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *m =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int present = 0;

    PROBE_ASSERT("lane.oob",
                 dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_lane_at_pc34(2) == NULL);
    PROBE_ASSERT("step.oob",
                 dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_step_at_pc34(10) == NULL);
    for (i = 0; m && i < DM1_V1_D1L_D1R_F0107_SIDE_COUNT_PC34; ++i) {
        const DM1_V1_D1LD1RF0107LanePc34 *lane =
            dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_lane_at_pc34(i);
        PROBE_ASSERT("lane.present", lane != NULL);
        if (!lane) continue;
        PROBE_ASSERT("lane.ptr", lane == &m->lanes[i]);
        PROBE_ASSERT_EQ("lane.depth", lane->relative_depth, 1);
        PROBE_ASSERT("lane.anchor", contains(lane->redmcsb_anchor, "DUNVIEW.C"));
    }

    PROBE_ASSERT_EQ("lane.d1l.side", m ? m->lanes[0].side : 0, 1);
    PROBE_ASSERT("lane.d1l.name", contains(m ? m->lanes[0].side_name : NULL, "D1L"));
    PROBE_ASSERT_EQ("lane.d1l.view_square", m ? m->lanes[0].view_square : 0, 4);
    PROBE_ASSERT_EQ("lane.d1l.lateral", m ? m->lanes[0].relative_lateral : 0, -1);
    PROBE_ASSERT_EQ("lane.d1l.wall_zone", m ? m->lanes[0].wall_zone : 0, 713);
    PROBE_ASSERT_EQ("lane.d1l.floor_view", m ? m->lanes[0].floor_view : 0, 8);
    PROBE_ASSERT_EQ("lane.d1l.door_zone", m ? m->lanes[0].door_zone : 0, 3780);
    PROBE_ASSERT_EQ("lane.d1l.body_start", m ? m->lanes[0].body_start_line : 0, 7391);
    PROBE_ASSERT_EQ("lane.d1l.body_end", m ? m->lanes[0].body_end_line : 0, 7557);
    PROBE_ASSERT_EQ("lane.d1l.wall_case", m ? m->lanes[0].wall_case_line : 0, 7436);
    PROBE_ASSERT_EQ("lane.d1l.wall_draw", m ? m->lanes[0].wall_draw_line : 0, 7454);
    PROBE_ASSERT_EQ("lane.d1l.f0107", m ? m->lanes[0].f0107_line : 0, 7459);
    PROBE_ASSERT_EQ("lane.d1l.slot", m ? m->lanes[0].f0107_aspect_slot : 0, 4);
    PROBE_ASSERT_EQ("lane.d1l.view_wall", m ? m->lanes[0].f0107_view_wall : 0, 12);
    PROBE_ASSERT_EQ("lane.d1l.update", m ? m->lanes[0].f0128_update_line : 0, 8524);
    PROBE_ASSERT_EQ("lane.d1l.draw", m ? m->lanes[0].f0128_draw_line : 0, 8525);
    PROBE_ASSERT_EQ("lane.d1l.order", m ? (int)m->lanes[0].corridor_order : 0, 0x0032);
    PROBE_ASSERT_EQ("lane.d1l.door1", m ? (int)m->lanes[0].door_pass1_order : 0, 0x0028);
    PROBE_ASSERT_EQ("lane.d1l.door2", m ? (int)m->lanes[0].door_pass2_order : 0, 0x0039);

    PROBE_ASSERT_EQ("lane.d1r.side", m ? m->lanes[1].side : 0, 2);
    PROBE_ASSERT("lane.d1r.name", contains(m ? m->lanes[1].side_name : NULL, "D1R"));
    PROBE_ASSERT_EQ("lane.d1r.view_square", m ? m->lanes[1].view_square : 0, 5);
    PROBE_ASSERT_EQ("lane.d1r.lateral", m ? m->lanes[1].relative_lateral : 0, 1);
    PROBE_ASSERT_EQ("lane.d1r.wall_zone", m ? m->lanes[1].wall_zone : 0, 714);
    PROBE_ASSERT_EQ("lane.d1r.floor_view", m ? m->lanes[1].floor_view : 0, 10);
    PROBE_ASSERT_EQ("lane.d1r.door_zone", m ? m->lanes[1].door_zone : 0, 3800);
    PROBE_ASSERT_EQ("lane.d1r.body_start", m ? m->lanes[1].body_start_line : 0, 7559);
    PROBE_ASSERT_EQ("lane.d1r.body_end", m ? m->lanes[1].body_end_line : 0, 7725);
    PROBE_ASSERT_EQ("lane.d1r.wall_case", m ? m->lanes[1].wall_case_line : 0, 7604);
    PROBE_ASSERT_EQ("lane.d1r.wall_draw", m ? m->lanes[1].wall_draw_line : 0, 7622);
    PROBE_ASSERT_EQ("lane.d1r.f0107", m ? m->lanes[1].f0107_line : 0, 7627);
    PROBE_ASSERT_EQ("lane.d1r.slot", m ? m->lanes[1].f0107_aspect_slot : 0, 6);
    PROBE_ASSERT_EQ("lane.d1r.view_wall", m ? m->lanes[1].f0107_view_wall : 0, 13);
    PROBE_ASSERT_EQ("lane.d1r.update", m ? m->lanes[1].f0128_update_line : 0, 8528);
    PROBE_ASSERT_EQ("lane.d1r.draw", m ? m->lanes[1].f0128_draw_line : 0, 8529);
    PROBE_ASSERT_EQ("lane.d1r.order", m ? (int)m->lanes[1].corridor_order : 0, 0x0041);
    PROBE_ASSERT_EQ("lane.d1r.door1", m ? (int)m->lanes[1].door_pass1_order : 0, 0x0018);
    PROBE_ASSERT_EQ("lane.d1r.door2", m ? (int)m->lanes[1].door_pass2_order : 0, 0x0049);

    for (i = 0; i < DM1_V1_D1L_D1R_F0107_STEP_COUNT_PC34; ++i) {
        const DM1_V1_D1LD1RF0107StepPc34 *step =
            dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_step_at_pc34(i);
        PROBE_ASSERT("step.present", step != NULL);
        if (!step) continue;
        present += step->expected_present;
        PROBE_ASSERT_EQ("step.order", step->order_index, (int)i);
        PROBE_ASSERT("step.anchor", contains(step->redmcsb_anchor, "DUNVIEW.C") ||
                                    contains(step->redmcsb_anchor, "D0/D1C/D2/D3"));
    }
    PROBE_ASSERT_EQ("steps.present_count", present, 10);
}

static void test_ordinals_pixels_probe(void)
{
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *m =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34();
    uint8_t framebuffer[DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_WIDTH_PC34 *
                        DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_HEIGHT_PC34];
    size_t i;
    int skips = 0;
    int writes = 0;

    PROBE_ASSERT("ordinal.oob",
                 dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_ordinal_at_pc34(6) == NULL);
    for (i = 0; m && i < DM1_V1_D1L_D1R_F0107_ORDINAL_COUNT_PC34; ++i) {
        const DM1_V1_D1LD1RF0107OrdinalPc34 *ordinal =
            dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_ordinal_at_pc34(i);
        PROBE_ASSERT("ordinal.present", ordinal != NULL);
        if (!ordinal) continue;
        PROBE_ASSERT_EQ("ordinal.index", ordinal->ordinal_index_c0_to_c5, (int)i);
        PROBE_ASSERT_EQ("ordinal.sensor", ordinal->sensor_ordinal, (int)i + 1);
        PROBE_ASSERT_EQ("ordinal.d1l", ordinal->accepted_at_d1l, 1);
        PROBE_ASSERT_EQ("ordinal.d1r", ordinal->accepted_at_d1r, 1);
        PROBE_ASSERT("ordinal.anchor", contains(ordinal->redmcsb_anchor, "F0172"));
        PROBE_ASSERT("ordinal.accept.d1l",
                     dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
                         0, (int)i));
        PROBE_ASSERT("ordinal.accept.d1r",
                     dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
                         1, (int)i));
    }
    PROBE_ASSERT("ordinal.reject.side_low",
                 !dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(-1, 0));
    PROBE_ASSERT("ordinal.reject.side_high",
                 !dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(2, 0));
    PROBE_ASSERT("ordinal.reject.low",
                 !dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(0, -1));
    PROBE_ASSERT("ordinal.reject.high",
                 !dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(0, 6));

    PROBE_ASSERT("alcove.zero",
                 !dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_returns_alcove_pc34(0, true));
    PROBE_ASSERT("alcove.no",
                 !dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_returns_alcove_pc34(7, false));
    PROBE_ASSERT("alcove.yes",
                 dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_returns_alcove_pc34(7, true));
    PROBE_ASSERT_EQ("blend.transparent",
                    dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_blend_pixel_pc34(0xaa, 10, 10),
                    0xaa);
    PROBE_ASSERT_EQ("blend.opaque",
                    dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_blend_pixel_pc34(0xaa, 0x51, 10),
                    0x51);

    for (i = 0; m && i < DM1_V1_D1L_D1R_F0107_PIXEL_COUNT_PC34; ++i) {
        const DM1_V1_D1LD1RF0107PixelPc34 *p = &m->pixels[i];
        PROBE_ASSERT_EQ("pixel.after", p->after,
                        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_blend_pixel_pc34(
                            p->before, p->source, 10));
        PROBE_ASSERT_EQ("pixel.xor", p->transparent_skip + p->writes_pixel, 1);
        PROBE_ASSERT("pixel.anchor", contains(p->redmcsb_anchor, "F0107"));
        skips += p->transparent_skip;
        writes += p->writes_pixel;
    }
    PROBE_ASSERT_EQ("pixel.skip_count", skips, 3);
    PROBE_ASSERT_EQ("pixel.write_count", writes, 3);

    PROBE_ASSERT_EQ("render.null",
                    dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_render_probe_pc34(NULL, 0),
                    -1);
    PROBE_ASSERT_EQ("render.short",
                    dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_render_probe_pc34(
                        framebuffer, sizeof(framebuffer) - 1),
                    -1);
    PROBE_ASSERT_EQ("render.writes",
                    dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_render_probe_pc34(
                        framebuffer, sizeof(framebuffer)),
                    m ? (30 * 88 + 34 * 88) : 0);
    PROBE_ASSERT_EQ("render.d1l_marker", count_marked(framebuffer, 0x51), 30 * 88);
    PROBE_ASSERT_EQ("render.d1r_marker", count_marked(framebuffer, 0x52), 34 * 88);
    PROBE_ASSERT_EQ("render.d0_rect_clear", count_marked_in_rect(framebuffer, 4, 58, 18, 74), 0);
    PROBE_ASSERT_EQ("render.d1c_rect_clear", count_marked_in_rect(framebuffer, 86, 24, 46, 90), 0);
    PROBE_ASSERT_EQ("render.d2_rect_clear", count_marked_in_rect(framebuffer, 54, 62, 24, 55), 0);
    PROBE_ASSERT_EQ("render.d3_rect_clear", count_marked_in_rect(framebuffer, 74, 76, 16, 37), 0);
}

static void test_sibling_rejects_and_evidence(void)
{
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *m =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34();
    const char *e = dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_source_evidence_pc34();
    const char *d = dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_disjointness_note_pc34();
    size_t i;

    PROBE_ASSERT("sibling.oob",
                 dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_sibling_reject_at_pc34(4) == NULL);
    for (i = 0; m && i < DM1_V1_D1L_D1R_F0107_SIBLING_REJECT_COUNT_PC34; ++i) {
        const DM1_V1_D1LD1RF0107SiblingRejectPc34 *reject =
            dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_sibling_reject_at_pc34(i);
        PROBE_ASSERT("sibling.present", reject != NULL);
        if (!reject) continue;
        PROBE_ASSERT_EQ("sibling.reject.cell", reject->reject_cell_position, 1);
        PROBE_ASSERT_EQ("sibling.reject.zone", reject->reject_carrier_zone, 1);
        PROBE_ASSERT_EQ("sibling.reject.view_wall", reject->reject_view_wall, 1);
        PROBE_ASSERT_EQ("sibling.reject.aspect", reject->reject_aspect_ratio, 1);
        PROBE_ASSERT("sibling.not_d1l_zone", reject->left_carrier_zone != 713);
        PROBE_ASSERT("sibling.not_d1r_zone", reject->right_carrier_zone != 714);
        PROBE_ASSERT("sibling.not_d1l_view", reject->left_view_wall != 12);
        PROBE_ASSERT("sibling.not_d1r_view", reject->right_view_wall != 13);
        PROBE_ASSERT("sibling.aspect_diff",
                     reject->aspect_width * 88 != reject->aspect_height * 30);
        PROBE_ASSERT("sibling.anchor", contains(reject->redmcsb_anchor, "DUNVIEW.C"));
    }
    PROBE_ASSERT("sibling.d0", contains(m ? m->sibling_rejects[0].sibling_name : NULL, "D0L/D0R"));
    PROBE_ASSERT("sibling.d1c", contains(m ? m->sibling_rejects[1].sibling_name : NULL, "D1C"));
    PROBE_ASSERT("sibling.d2", contains(m ? m->sibling_rejects[2].sibling_name : NULL, "D2L/D2R"));
    PROBE_ASSERT("sibling.d3", contains(m ? m->sibling_rejects[3].sibling_name : NULL, "D3L/D3R"));
    PROBE_ASSERT_EQ("sibling.d3.c705", m ? m->sibling_rejects[3].left_carrier_zone : 0, 705);
    PROBE_ASSERT_EQ("sibling.d3.c706", m ? m->sibling_rejects[3].right_carrier_zone : 0, 706);

    PROBE_ASSERT("evidence.f0107", contains(e, "DUNVIEW.C F0107:3502-3938"));
    PROBE_ASSERT("evidence.contract.f0114", contains(e, "F0114:6651-6740"));
    PROBE_ASSERT("evidence.contract.f0115", contains(e, "F0115:6742-6886"));
    PROBE_ASSERT("evidence.local.f0122", contains(e, "F0122:7391-7557"));
    PROBE_ASSERT("evidence.local.f0123", contains(e, "F0123:7559-7725"));
    PROBE_ASSERT("evidence.d1l.call", contains(e, "7459"));
    PROBE_ASSERT("evidence.d1r.call", contains(e, "7627"));
    PROBE_ASSERT("evidence.f0128", contains(e, "F0128:8503-8517"));
    PROBE_ASSERT("evidence.f0108", contains(e, "F0108:3940-4011"));
    PROBE_ASSERT("evidence.f0163", contains(e, "F0163:1769-1838"));
    PROBE_ASSERT("evidence.f0164", contains(e, "F0164:1840-1905"));
    PROBE_ASSERT("evidence.f0172", contains(e, "F0172:2466-2523"));
    PROBE_ASSERT("evidence.c10", contains(e, "DEFS.H:2088"));
    PROBE_ASSERT("evidence.c0c5", contains(e, "C0..C5"));
    PROBE_ASSERT("evidence.c713", contains(e, "C713"));
    PROBE_ASSERT("evidence.c714", contains(e, "C714"));

    PROBE_ASSERT("disjoint.d0", contains(d, "D0L/D0R"));
    PROBE_ASSERT("disjoint.d1c", contains(d, "D1C"));
    PROBE_ASSERT("disjoint.d2", contains(d, "D2L/D2R"));
    PROBE_ASSERT("disjoint.d3", contains(d, "D3L/D3R"));
    PROBE_ASSERT("disjoint.c705", contains(d, "C705/C706"));
    PROBE_ASSERT("disjoint.aspect", contains(d, "aspect ratio"));
    PROBE_ASSERT("disjoint.no_assets", contains(d, "GRAPHICS.DAT"));
    PROBE_ASSERT("disjoint.no_dos", contains(d, "original DOS pixel parity"));
}

static void test_hash(void)
{
    uint32_t hash =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_deterministic_hash_pc34();

    PROBE_ASSERT("hash.nonzero", hash != 0u);
    PROBE_ASSERT_U32("hash.stable", hash, 0x2a5a608du);
}

int main(void)
{
    test_model_core();
    test_lanes_steps();
    test_ordinals_pixels_probe();
    test_sibling_rejects_and_evidence();
    test_hash();

    if (g_failures) {
        printf("assertions=%d failures=%d hash=0x%08x\n",
               g_assertions,
               g_failures,
               (unsigned)dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_deterministic_hash_pc34());
        return 1;
    }
    printf("assertions=%d failures=0 hash=0x%08x\n",
           g_assertions,
           (unsigned)dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_deterministic_hash_pc34());
    return 0;
}
