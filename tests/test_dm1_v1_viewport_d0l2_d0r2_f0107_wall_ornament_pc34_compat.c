#include "firestaff/dm1/v1/viewport/d0l2_d0r2_f0107_wall_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

#define EXPECT_TRUE(id, expr)                                                   \
    do {                                                                        \
        ++g_assertions;                                                         \
        if (!(expr)) {                                                          \
            printf("FAIL %s\n", (id));                                          \
            ++g_failures;                                                       \
        }                                                                       \
    } while (0)

#define EXPECT_EQ(id, got, want)                                                \
    do {                                                                        \
        int got__ = (int)(got);                                                 \
        int want__ = (int)(want);                                               \
        ++g_assertions;                                                         \
        if (got__ != want__) {                                                  \
            printf("FAIL %s got=%d want=%d\n", (id), got__, want__);            \
            ++g_failures;                                                       \
        }                                                                       \
    } while (0)

#define EXPECT_U32(id, got, want)                                               \
    do {                                                                        \
        uint32_t got__ = (uint32_t)(got);                                       \
        uint32_t want__ = (uint32_t)(want);                                     \
        ++g_assertions;                                                         \
        if (got__ != want__) {                                                  \
            printf("FAIL %s got=0x%08x want=0x%08x\n",                         \
                   (id), (unsigned)got__, (unsigned)want__);                    \
            ++g_failures;                                                       \
        }                                                                       \
    } while (0)

static int contains(const char *haystack, const char *needle)
{
    return haystack && needle && strstr(haystack, needle) != NULL;
}

static void test_core_model(void)
{
    DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 built_a;
    DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 built_b;
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34();

    EXPECT_EQ("builder.null",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_builder_pc34(NULL),
              0);
    EXPECT_EQ("builder.a",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_builder_pc34(
                  &built_a),
              1);
    EXPECT_EQ("builder.b",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_builder_pc34(
                  &built_b),
              1);
    EXPECT_TRUE("model.present", model != NULL);
    if (!model) return;

    EXPECT_EQ("field.byte_stability", memcmp(&built_a, &built_b, sizeof(built_a)), 0);
    EXPECT_U32("hash.null",
               dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_hash_model_pc34(NULL),
               0u);
    EXPECT_U32("hash.builder_stable", built_a.deterministic_hash, built_b.deterministic_hash);
    EXPECT_U32("hash.builder_model", built_a.deterministic_hash, model->deterministic_hash);
    EXPECT_U32("hash.accessor", model->deterministic_hash,
               dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_deterministic_hash_pc34());
    EXPECT_U32("hash.recomputed", model->deterministic_hash,
               dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_hash_model_pc34(model));
    EXPECT_U32("hash.expected", model->deterministic_hash, 0x2cebb06cu);
    EXPECT_TRUE("hash.nonzero", model->deterministic_hash != 0u);

    EXPECT_EQ("framebuffer.width", model->framebuffer_width, 320);
    EXPECT_EQ("framebuffer.height", model->framebuffer_height, 200);
    EXPECT_EQ("viewport.width", model->viewport_width, 224);
    EXPECT_EQ("viewport.height", model->viewport_height, 136);
    EXPECT_EQ("c10", model->c10_transparent_color, 10);
    EXPECT_EQ("zone.base", model->wall_ornament_zone_base, 1004);
    EXPECT_EQ("zone.stride", model->wall_ornament_zone_stride, 15);
    EXPECT_EQ("zone.coord_set", model->wall_ornament_coordinate_set, 0);
    EXPECT_EQ("zone.d0l2.front", model->d0l2_front_wall_zone, 1016);
    EXPECT_EQ("zone.d0r2.front", model->d0r2_front_wall_zone, 1017);
    EXPECT_EQ("slot.m550", model->m550_first_thing_slot, 2);
    EXPECT_EQ("slot.m551", model->m551_right_wall_ornament_slot, 4);
    EXPECT_EQ("slot.m552", model->m552_front_wall_ornament_slot, 5);
    EXPECT_EQ("slot.m553", model->m553_left_wall_ornament_slot, 6);
    EXPECT_EQ("dispatch.d0l2_before_d0r2", model->f0128_d0l2_before_d0r2, 1);
    EXPECT_EQ("dispatch.after_f0116_f0117",
              model->f0128_after_f0116_f0117_wall_composition, 1);
    EXPECT_EQ("terminal.side_pair_correction",
              model->terminal_depth_side_pair_correction, 1);
    EXPECT_EQ("calls.direct_f0107", model->direct_f0107_call_count, 0);
    EXPECT_EQ("calls.candidate", model->f0107_candidate_call_count, 8);
    EXPECT_EQ("f0107.zero", model->zero_ordinal_returns_false, 1);
    EXPECT_EQ("f0107.non_alcove", model->non_alcove_returns_false, 1);
    EXPECT_EQ("f0107.alcove", model->alcove_returns_true, 1);
    EXPECT_EQ("ordinals.c0_c5", model->c0_to_c5_ordinals_pinned, 1);
    EXPECT_EQ("c10.preserve", model->c10_transparent_preserves_destination, 1);
    EXPECT_EQ("field.byte_stability_flag", model->field_level_byte_stability, 1);
    EXPECT_EQ("seed", model->deterministic_seed, 0x1070d02);
    EXPECT_EQ("contract.only", model->source_locked_contract_only, 1);
    EXPECT_EQ("contract.no_dos", model->no_original_dos_pixel_parity, 1);
    EXPECT_EQ("contract.no_assets", model->no_graphics_dat_reads, 1);
    EXPECT_TRUE("evidence.f0107", contains(model->source_evidence, "F0107:3502-3938"));
    EXPECT_TRUE("evidence.f0125", contains(model->source_evidence, "F0125:7960-8062"));
    EXPECT_TRUE("evidence.f0126", contains(model->source_evidence, "F0126:8064-8162"));
    EXPECT_TRUE("evidence.f0128", contains(model->source_evidence, "F0128:8536-8541"));
    EXPECT_TRUE("note.disjoint", contains(model->disjointness_note, "contract only"));
    EXPECT_TRUE("source.accessor",
                contains(dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_source_evidence_pc34(),
                         "DEFS.H:2088"));
    EXPECT_TRUE("note.accessor",
                contains(
                    dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_disjointness_note_pc34(),
                    "GRAPHICS.DAT"));
}

static void test_lanes(void)
{
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34();
    size_t i;

    EXPECT_TRUE("lane.oob",
                dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_lane_at_pc34(2) == NULL);
    for (i = 0; model && i < DM1_V1_D0L2_D0R2_F0107_SIDE_COUNT_PC34; ++i) {
        const DM1_V1_D0L2D0R2F0107LanePc34 *lane =
            dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_lane_at_pc34(i);
        EXPECT_TRUE("lane.present", lane != NULL);
        if (!lane) continue;
        EXPECT_TRUE("lane.ptr", lane == &model->lanes[i]);
        EXPECT_EQ("lane.side", lane->side, (int)i);
        EXPECT_EQ("lane.depth", lane->relative_depth, 0);
        EXPECT_EQ("lane.pass", lane->terminal_side_pair_pass, (int)i + 1);
        EXPECT_EQ("lane.order", lane->pair_dispatch_order, (int)i);
        EXPECT_EQ("lane.m550", lane->first_thing_slot, 2);
        EXPECT_EQ("lane.m551", lane->right_wall_ornament_slot, 4);
        EXPECT_EQ("lane.m552", lane->front_wall_ornament_slot, 5);
        EXPECT_EQ("lane.m553", lane->left_wall_ornament_slot, 6);
        EXPECT_TRUE("lane.anchor", contains(lane->redmcsb_anchor, "DUNVIEW.C"));
    }

    EXPECT_TRUE("lane.d0l2.name", contains(model ? model->lanes[0].side_name : NULL, "D0L2"));
    EXPECT_TRUE("lane.d0l2.function",
                contains(model ? model->lanes[0].function_name : NULL, "F0125"));
    EXPECT_EQ("lane.d0l2.update", model ? model->lanes[0].dispatcher_update_line : 0, 8536);
    EXPECT_EQ("lane.d0l2.draw", model ? model->lanes[0].dispatcher_draw_line : 0, 8537);
    EXPECT_EQ("lane.d0l2.start", model ? model->lanes[0].function_start_line : 0, 7960);
    EXPECT_EQ("lane.d0l2.end", model ? model->lanes[0].function_end_line : 0, 8062);
    EXPECT_EQ("lane.d0l2.view_square", model ? model->lanes[0].view_square : 0, 1);
    EXPECT_EQ("lane.d0l2.lateral", model ? model->lanes[0].relative_lateral : 0, -1);
    EXPECT_EQ("lane.d0l2.wall_zone", model ? model->lanes[0].wall_zone : 0, 716);
    EXPECT_EQ("lane.d0l2.ceiling_zone", model ? model->lanes[0].ceiling_pit_zone : 0, 870);
    EXPECT_EQ("lane.d0l2.thing_line", model ? model->lanes[0].thing_pass_line : 0, 8005);
    EXPECT_EQ("lane.d0l2.order", model ? model->lanes[0].thing_pass_cell_order : 0, 2);

    EXPECT_TRUE("lane.d0r2.name", contains(model ? model->lanes[1].side_name : NULL, "D0R2"));
    EXPECT_TRUE("lane.d0r2.function",
                contains(model ? model->lanes[1].function_name : NULL, "F0126"));
    EXPECT_EQ("lane.d0r2.update", model ? model->lanes[1].dispatcher_update_line : 0, 8540);
    EXPECT_EQ("lane.d0r2.draw", model ? model->lanes[1].dispatcher_draw_line : 0, 8541);
    EXPECT_EQ("lane.d0r2.start", model ? model->lanes[1].function_start_line : 0, 8064);
    EXPECT_EQ("lane.d0r2.end", model ? model->lanes[1].function_end_line : 0, 8162);
    EXPECT_EQ("lane.d0r2.view_square", model ? model->lanes[1].view_square : 0, 2);
    EXPECT_EQ("lane.d0r2.lateral", model ? model->lanes[1].relative_lateral : 0, 1);
    EXPECT_EQ("lane.d0r2.wall_zone", model ? model->lanes[1].wall_zone : 0, 717);
    EXPECT_EQ("lane.d0r2.ceiling_zone", model ? model->lanes[1].ceiling_pit_zone : 0, 872);
    EXPECT_EQ("lane.d0r2.thing_line", model ? model->lanes[1].thing_pass_line : 0, 8115);
    EXPECT_EQ("lane.d0r2.order", model ? model->lanes[1].thing_pass_cell_order : 0, 1);
}

static void test_routes_calls(void)
{
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int returned = 0;
    int open_tail = 0;
    int candidates = 0;

    EXPECT_TRUE("route.oob",
                dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_route_at_pc34(7) == NULL);
    EXPECT_TRUE("call.oob",
                dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_call_at_pc34(8) == NULL);

    for (i = 0; model && i < DM1_V1_D0L2_D0R2_F0107_ELEMENT_COUNT_PC34; ++i) {
        const DM1_V1_D0L2D0R2F0107ElementRoutePc34 *route =
            dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_route_at_pc34(i);
        EXPECT_TRUE("route.present", route != NULL);
        if (!route) continue;
        EXPECT_TRUE("route.ptr", route == &model->routes[i]);
        EXPECT_TRUE("route.name", route->element_name != NULL);
        EXPECT_TRUE("route.anchor", contains(route->redmcsb_anchor, "DUNVIEW.C"));
        if (route->returns_before_tail) ++returned;
        if (route->has_thing_pass_tail) ++open_tail;
        EXPECT_EQ("route.short_circuit", route->f0107_short_circuit_ordinal_zero, 1);
    }
    EXPECT_EQ("routes.return_count", returned, 3);
    EXPECT_EQ("routes.open_tail_count", open_tail, 4);

    EXPECT_EQ("route.wall.element", model ? model->routes[0].element : -1, 0);
    EXPECT_EQ("route.wall.returns", model ? model->routes[0].returns_before_tail : 0, 1);
    EXPECT_EQ("route.wall.d0l.start", model ? model->routes[0].d0l_line_start : 0, 8007);
    EXPECT_EQ("route.wall.d0r.end", model ? model->routes[0].d0r_line_end : 0, 8144);
    EXPECT_EQ("route.corridor.element", model ? model->routes[1].element : 0, 1);
    EXPECT_EQ("route.corridor.tail", model ? model->routes[1].has_thing_pass_tail : 0, 1);
    EXPECT_EQ("route.pit.element", model ? model->routes[2].element : 0, 2);
    EXPECT_EQ("route.pit.ceiling", model ? model->routes[2].has_ceiling_tail : 0, 1);
    EXPECT_EQ("route.teleporter.element", model ? model->routes[3].element : 0, 5);
    EXPECT_EQ("route.teleporter.field", model ? model->routes[3].has_teleporter_field_tail : 0, 1);
    EXPECT_EQ("route.door_side.element", model ? model->routes[4].element : 0, 16);
    EXPECT_EQ("route.door_side.supported", model ? model->routes[4].supported_by_f0125_f0126 : 0, 1);
    EXPECT_EQ("route.door_front.element", model ? model->routes[5].element : 0, 17);
    EXPECT_EQ("route.door_front.supported", model ? model->routes[5].supported_by_f0125_f0126 : 1, 0);
    EXPECT_EQ("route.stairs.element", model ? model->routes[6].element : 0, 18);
    EXPECT_EQ("route.stairs.returns", model ? model->routes[6].returns_before_tail : 0, 1);

    for (i = 0; model && i < DM1_V1_D0L2_D0R2_F0107_CALL_COUNT_PC34; ++i) {
        const DM1_V1_D0L2D0R2F0107CallPc34 *call =
            dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_call_at_pc34(i);
        EXPECT_TRUE("call.present", call != NULL);
        if (!call) continue;
        ++candidates;
        EXPECT_TRUE("call.ptr", call == &model->calls[i]);
        EXPECT_EQ("call.index", call->call_index, (int)i);
        EXPECT_TRUE("call.side", call->side == 0 || call->side == 1);
        EXPECT_EQ("call.slot", call->aspect_slot, 5);
        EXPECT_TRUE("call.slot_name", contains(call->aspect_slot_name, "M552"));
        EXPECT_TRUE("call.view_name", contains(call->view_wall_name, "VIEW_WALL"));
        EXPECT_EQ("call.coord_set", call->coordinate_set, 0);
        EXPECT_EQ("call.zone_math",
                  call->zone,
                  1004 + call->coordinate_set * 15 + call->view_wall);
        EXPECT_EQ("call.short_circuit", call->ordinal_short_circuits, 1);
        EXPECT_EQ("call.alcove", call->alcove_returns_true, 1);
        EXPECT_EQ("call.c10", call->uses_c10_transparency, 1);
        EXPECT_TRUE("call.anchor", contains(call->redmcsb_anchor, "F0107"));
    }
    EXPECT_EQ("call.count", candidates, 8);
}

static void test_ordinals_pixels_probe(void)
{
    const DM1_V1_D0L2D0R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_default_model_pc34();
    static uint8_t framebuffer[(size_t)DM1_V1_D0L2_D0R2_F0107_FRAMEBUFFER_WIDTH_PC34 *
                               (size_t)DM1_V1_D0L2_D0R2_F0107_FRAMEBUFFER_HEIGHT_PC34];
    size_t i;
    int ordinal_sum = 0;
    int transparent = 0;
    int writes = 0;

    EXPECT_TRUE("ordinal.oob",
                dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_ordinal_at_pc34(6) == NULL);
    for (i = 0; model && i < DM1_V1_D0L2_D0R2_F0107_ORDINAL_COUNT_PC34; ++i) {
        const DM1_V1_D0L2D0R2F0107OrdinalPc34 *ordinal =
            dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_ordinal_at_pc34(i);
        EXPECT_TRUE("ordinal.present", ordinal != NULL);
        if (!ordinal) continue;
        ordinal_sum += ordinal->sensor_ordinal;
        EXPECT_EQ("ordinal.index", ordinal->ordinal_index_c0_to_c5, (int)i);
        EXPECT_EQ("ordinal.sensor", ordinal->sensor_ordinal, (int)i + 1);
        EXPECT_EQ("ordinal.d0l2", ordinal->accepted_at_d0l2_front, 1);
        EXPECT_EQ("ordinal.d0r2", ordinal->accepted_at_d0r2_front, 1);
        EXPECT_EQ("ordinal.decremented", ordinal->decremented_index, (int)i);
        EXPECT_EQ("ordinal.slot_m552", ordinal->source_slot_m552, 5);
        EXPECT_TRUE("ordinal.anchor", contains(ordinal->redmcsb_anchor, "F0107"));
    }
    EXPECT_EQ("ordinal.sum", ordinal_sum, 21);

    for (i = 0; model && i < DM1_V1_D0L2_D0R2_F0107_PIXEL_COUNT_PC34; ++i) {
        const DM1_V1_D0L2D0R2F0107PixelPc34 *pixel = &model->pixels[i];
        const uint8_t blended =
            dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_blend_pixel_pc34(
                pixel->before, pixel->source, 10);
        EXPECT_U32("pixel.blend", blended, pixel->after);
        EXPECT_EQ("pixel.transparent_flag", pixel->transparent_skip, pixel->source == 10u);
        EXPECT_EQ("pixel.write_flag", pixel->writes_pixel, pixel->source != 10u);
        EXPECT_TRUE("pixel.side", pixel->side == 0 || pixel->side == 1);
        EXPECT_TRUE("pixel.element",
                    pixel->element == DM1_V1_D0L2_D0R2_F0107_ELEMENT_CORRIDOR_PC34 ||
                        pixel->element == DM1_V1_D0L2_D0R2_F0107_ELEMENT_TELEPORTER_PC34);
        EXPECT_TRUE("pixel.anchor", contains(pixel->redmcsb_anchor, "C10"));
        if (pixel->transparent_skip) ++transparent;
        if (pixel->writes_pixel) ++writes;
    }
    EXPECT_EQ("pixel.transparent_count", transparent, 3);
    EXPECT_EQ("pixel.write_count", writes, 5);

    EXPECT_EQ("alcove.zero.false",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_returns_alcove_pc34(0, true),
              0);
    EXPECT_EQ("alcove.non_alcove.false",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_returns_alcove_pc34(1, false),
              0);
    EXPECT_EQ("alcove.true",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_returns_alcove_pc34(1, true),
              1);
    EXPECT_EQ("zone.invalid.coord",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_zone_pc34(-1, 14),
              -1);
    EXPECT_EQ("zone.invalid.view",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_zone_pc34(0, -1),
              -1);
    EXPECT_EQ("zone.d1l.right",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_zone_pc34(0, 12),
              1016);
    EXPECT_EQ("zone.d1r.left",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_zone_pc34(0, 13),
              1017);
    EXPECT_EQ("zone.d1c.front",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_zone_pc34(0, 14),
              1018);

    EXPECT_EQ("probe.null",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_render_probe_pc34(
                  NULL, sizeof(framebuffer)),
              -1);
    EXPECT_EQ("probe.short",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_render_probe_pc34(
                  framebuffer, sizeof(framebuffer) - 1u),
              -1);
    EXPECT_EQ("probe.ok",
              dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_render_probe_pc34(
                  framebuffer, sizeof(framebuffer)),
              8);
}

int main(void)
{
    test_core_model();
    test_lanes();
    test_routes_calls();
    test_ordinals_pixels_probe();

    if (g_failures != 0) {
        printf("FAIL dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_source_lock "
               "assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_source_lock "
           "assertions=%d failures=0 hash=0x%08x\n",
           g_assertions,
           (unsigned)dm1_v1_viewport_d0l2_d0r2_f0107_wall_ornament_deterministic_hash_pc34());
    return 0;
}
