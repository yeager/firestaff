#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_pc34_compat.h"

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
    DM1_V1_D3LD3RF0107WallOrnamentModelPc34 built;
    const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_pc34();

    expect_int("builder.null",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_builder_pc34(NULL),
               0, "defensive builder guard");
    expect_int("builder.ok",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_builder_pc34(&built),
               1, "deterministic default model builder");
    expect_int("model.present", model != NULL, 1, "default model accessor");
    if (!model) return;
    expect_int("model.hash_matches_built",
               built.deterministic_hash == model->deterministic_hash, 1,
               "default model builder is deterministic");
    expect_int("hash_model.null",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_hash_model_pc34(NULL),
               0, "hash null guard");
    expect_int("hash_model.matches",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_hash_model_pc34(model) ==
                   model->deterministic_hash,
               1, "hash_model stable");
    expect_int("hash_accessor.matches",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_deterministic_hash_pc34() ==
                   model->deterministic_hash,
               1, "hash accessor stable");

    expect_int("view_square.d3l", model->view_square_d3l, 12,
               "DEFS.H:2608 M601_VIEW_SQUARE_D3L");
    expect_int("view_square.d3r", model->view_square_d3r, 13,
               "DEFS.H:2609 M602_VIEW_SQUARE_D3R");
    expect_int("wall_zone.d3l", model->wall_zone_d3l, 705,
               "DEFS.H:4045 C705_ZONE_WALL_D3L");
    expect_int("wall_zone.d3r", model->wall_zone_d3r, 706,
               "DEFS.H:4046 C706_ZONE_WALL_D3R");
    expect_int("floor_view.d3l", model->floor_view_d3l, 2,
               "DEFS.H:2752 M588_VIEW_FLOOR_D3L");
    expect_int("floor_view.d3r", model->floor_view_d3r, 4,
               "DEFS.H:2754 M590_VIEW_FLOOR_D3R");
    expect_int("field.d3l", model->field_aspect_d3l, 3,
               "DUNVIEW.C:377 G2035[M601]");
    expect_int("field.d3r", model->field_aspect_d3r, 4,
               "DUNVIEW.C:377 G2035[M602]");
    expect_int("transparent.c10", model->c10_transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("zone.wall.base", model->c1004_wall_ornament_zone_base, 1004,
               "DEFS.H:4222 C1004_ZONE_WALL_ORNAMENT");
    expect_int("zone.wall.stride", model->wall_ornament_zone_stride, 15,
               "DUNVIEW.C:3586-3587 C15 stride");
    expect_int("zone.floor.base", model->c1500_floor_ornament_zone_base, 1500,
               "DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT");
    expect_int("g0205.wall_sets", model->g0205_wall_ornament_coordinate_sets, 1,
               "DUNVIEW.C:3578 G0205");
    expect_int("g0206.floor_sets", model->g0206_floor_ornament_coordinate_sets, 1,
               "DUNVIEW.C:3966 G0206");
    expect_int("g0207.door_ornament_sets", model->g0207_door_ornament_coordinate_sets, 1,
               "DUNVIEW.C:4046 G0207");
    expect_int("g0208.door_button_sets", model->g0208_door_button_coordinate_sets, 1,
               "DUNVIEW.C:4163 G0208");
    expect_int("dispatch.d3l_then_d3r", model->f0128_d3l_then_d3r, 1,
               "DUNVIEW.C:8491 before 8495");
    expect_int("dispatch.before_d3c", model->f0128_d3_pair_before_d3c, 1,
               "DUNVIEW.C:8491/8495 before 8499");
    expect_int("dispatch.before_d2", model->f0128_d3_pair_before_d2_pair, 1,
               "DUNVIEW.C:8491/8495 before 8513/8517");
    expect_int("spatial.deeper_than_d2", model->spatially_deeper_than_d2_pair, 1,
               "relative depth 3 is terminal side-lane depth");
    expect_int("f0107.direct_count", model->direct_f0107_call_count, 4,
               "DUNVIEW.C:6432/6433/6568/6569");
    expect_int("sensor.direct_positions", model->sensor_position_count, 4,
               "M551/M552/M553 sensor positions");
    expect_int("side.call_count", model->side_ornament_call_count, 2,
               "D3L side + D3R side");
    expect_int("front.call_count", model->front_ornament_call_count, 2,
               "D3L front + D3R front");
    expect_int("zero_ordinal.false", model->f0107_zero_ordinal_returns_false, 1,
               "DUNVIEW.C:3568/3571-3573");
    expect_int("non_alcove.false", model->f0107_non_alcove_returns_false, 1,
               "DUNVIEW.C:3589");
    expect_int("alcove.true", model->f0107_alcove_returns_true, 1,
               "DUNVIEW.C:3933");
    expect_int("f0107.c10", model->f0107_blit_uses_c10, 1,
               "DUNVIEW.C:3922");
    expect_int("c10.preserve", model->c10_transparent_preserves_destination, 1,
               "C10 transparent pixel preservation");
    expect_int("f0108.before_f0115", model->f0108_floor_baseline_before_f0115, 1,
               "DUNVIEW.C:6478 before 6480 and 6620 before 6622");
    expect_int("f0112.absent", model->f0112_ceiling_pit_before_f0115, 0,
               "D3L/D3R do not call F0112");
    expect_int("f0113.after_f0115", model->f0113_teleporter_field_after_f0115, 1,
               "DUNVIEW.C:6480 before 6495 and 6622 before 6637");
    expect_int("d3l.open_order", model->d3l_cell_order_terminal_depth, 0x3421,
               "DEFS.H:2676 C0x3421");
    expect_int("d3r.open_order", model->d3r_cell_order_terminal_depth, 0x4312,
               "DEFS.H:2677 C0x4312");
    expect_int("no_graphics_dat", model->no_graphics_dat_reads, 1,
               "asset-free contract fixture");
    expect_int("contract_only", model->source_locked_contract_only, 1,
               "source-lock contract only");
    expect_int("no_dos_pixel_parity", model->no_original_dos_pixel_parity, 1,
               "no original DOS pixel parity claim");
}

static void test_lanes(void)
{
    const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_pc34();
    size_t i;

    expect_int("lane.out_of_range",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_lane_at_pc34(2) == NULL,
               1, "lane accessor bounds");
    for (i = 0; model && i < DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_LANE_COUNT_PC34; ++i) {
        const DM1_V1_D3LD3RF0107LanePc34 *lane =
            dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_lane_at_pc34(i);
        char id[96];
        snprintf(id, sizeof(id), "lane.%u.present", (unsigned)i);
        expect_int(id, lane != NULL, 1, "lane accessor");
        if (!lane) continue;
        snprintf(id, sizeof(id), "lane.%u.model_ptr", (unsigned)i);
        expect_int(id, lane == &model->lanes[i], 1, "lane pointer stable");
        snprintf(id, sizeof(id), "lane.%u.depth", (unsigned)i);
        expect_int(id, lane->relative_depth, 3, "terminal side-lane depth");
        snprintf(id, sizeof(id), "lane.%u.alcove_order", (unsigned)i);
        expect_int(id, lane->alcove_order, 0, "C0x0000_CELL_ORDER_ALCOVE");
        snprintf(id, sizeof(id), "lane.%u.f0108_before_f0112", (unsigned)i);
        expect_int(id, lane->f0108_before_f0112, 1, "F0108 precedes optional ceiling-pit slot");
        snprintf(id, sizeof(id), "lane.%u.no_f0112", (unsigned)i);
        expect_int(id, lane->f0112_ceiling_line, 0, "D3L/D3R no F0112 call");
        snprintf(id, sizeof(id), "lane.%u.f0112_before_f0115", (unsigned)i);
        expect_int(id, lane->f0112_before_f0115, 0, "F0112 not applicable on D3L/D3R");
        snprintf(id, sizeof(id), "lane.%u.f0115_before_f0113", (unsigned)i);
        expect_int(id, lane->f0115_before_f0113, 1, "F0113 teleporter field follows F0115");
        snprintf(id, sizeof(id), "lane.%u.anchor", (unsigned)i);
        expect_contains(id, lane->redmcsb_anchor, "DUNVIEW.C", "lane source anchor");
    }

    expect_int("lane.d3l.side", model ? model->lanes[0].side : -1, 1,
               "D3L side enum");
    expect_contains("lane.d3l.name", model ? model->lanes[0].side_name : NULL,
                    "D3L", "D3L name");
    expect_int("lane.d3l.view_square", model ? model->lanes[0].view_square : -1, 12,
               "DEFS.H M601");
    expect_int("lane.d3l.lateral", model ? model->lanes[0].relative_lateral : 0, -1,
               "relative lateral left");
    expect_int("lane.d3l.wall_zone", model ? model->lanes[0].wall_zone : 0, 705,
               "DEFS.H C705");
    expect_int("lane.d3l.update_line", model ? model->lanes[0].f0128_update_line : 0,
               8490, "DUNVIEW.C:8490");
    expect_int("lane.d3l.draw_line", model ? model->lanes[0].f0128_draw_line : 0,
               8491, "DUNVIEW.C:8491");
    expect_int("lane.d3l.dispatch_start", model ? model->lanes[0].dispatcher_line_start : 0,
               6361, "DUNVIEW.C F0116");
    expect_int("lane.d3l.dispatch_end", model ? model->lanes[0].dispatcher_line_end : 0,
               6498, "DUNVIEW.C F0116");
    expect_int("lane.d3l.wall_case", model ? model->lanes[0].wall_case_line : 0,
               6406, "DUNVIEW.C:6406");
    expect_int("lane.d3l.wall_draw", model ? model->lanes[0].wall_zone_draw_line : 0,
               6427, "DUNVIEW.C:6427");
    expect_int("lane.d3l.side_call", model ? model->lanes[0].side_f0107_line : 0,
               6432, "DUNVIEW.C:6432");
    expect_int("lane.d3l.side_slot", model ? model->lanes[0].side_ornament_slot : 0,
               4, "M551_RIGHT_WALL_ORNAMENT_ORDINAL");
    expect_int("lane.d3l.side_view_wall", model ? model->lanes[0].side_view_wall : 0,
               2, "M575_VIEW_WALL_D3L_RIGHT");
    expect_int("lane.d3l.front_call", model ? model->lanes[0].front_f0107_line : 0,
               6433, "DUNVIEW.C:6433");
    expect_int("lane.d3l.front_slot", model ? model->lanes[0].front_ornament_slot : 0,
               5, "M552_FRONT_WALL_ORNAMENT_ORDINAL");
    expect_int("lane.d3l.front_view_wall", model ? model->lanes[0].front_view_wall : 0,
               4, "M577_VIEW_WALL_D3L_FRONT");
    expect_int("lane.d3l.corridor_order", model ? model->lanes[0].corridor_order : 0,
               0x3421, "DUNVIEW.C:6476");
    expect_int("lane.d3l.door_side_order", model ? model->lanes[0].door_side_order : 0,
               0x0321, "DUNVIEW.C:6440");
    expect_int("lane.d3l.door_pass1", model ? model->lanes[0].door_pass1_order : 0,
               0x0218, "DUNVIEW.C:6444");
    expect_int("lane.d3l.door_pass2", model ? model->lanes[0].door_pass2_order : 0,
               0x0349, "DUNVIEW.C:6459");
    expect_int("lane.d3l.f0108_open", model ? model->lanes[0].f0108_open_path_line : 0,
               6478, "DUNVIEW.C:6478");
    expect_int("lane.d3l.f0115", model ? model->lanes[0].f0115_line : 0,
               6480, "DUNVIEW.C:6480");
    expect_int("lane.d3l.f0113", model ? model->lanes[0].f0113_field_line : 0,
               6495, "DUNVIEW.C:6495");
    expect_int("lane.d3l.door_zone", model ? model->lanes[0].f0111_door_zone : 0,
               3720, "DEFS.H:4252 M624_ZONE_DOOR_D3L");

    expect_int("lane.d3r.side", model ? model->lanes[1].side : -1, 2,
               "D3R side enum");
    expect_contains("lane.d3r.name", model ? model->lanes[1].side_name : NULL,
                    "D3R", "D3R name");
    expect_int("lane.d3r.view_square", model ? model->lanes[1].view_square : -1, 13,
               "DEFS.H M602");
    expect_int("lane.d3r.lateral", model ? model->lanes[1].relative_lateral : 0, 1,
               "relative lateral right");
    expect_int("lane.d3r.wall_zone", model ? model->lanes[1].wall_zone : 0, 706,
               "DEFS.H C706");
    expect_int("lane.d3r.update_line", model ? model->lanes[1].f0128_update_line : 0,
               8494, "DUNVIEW.C:8494");
    expect_int("lane.d3r.draw_line", model ? model->lanes[1].f0128_draw_line : 0,
               8495, "DUNVIEW.C:8495");
    expect_int("lane.d3r.dispatch_start", model ? model->lanes[1].dispatcher_line_start : 0,
               6500, "DUNVIEW.C F0117");
    expect_int("lane.d3r.dispatch_end", model ? model->lanes[1].dispatcher_line_end : 0,
               6640, "DUNVIEW.C F0117");
    expect_int("lane.d3r.wall_case", model ? model->lanes[1].wall_case_line : 0,
               6545, "DUNVIEW.C:6545");
    expect_int("lane.d3r.wall_draw", model ? model->lanes[1].wall_zone_draw_line : 0,
               6563, "DUNVIEW.C:6563");
    expect_int("lane.d3r.side_call", model ? model->lanes[1].side_f0107_line : 0,
               6568, "DUNVIEW.C:6568");
    expect_int("lane.d3r.side_slot", model ? model->lanes[1].side_ornament_slot : 0,
               6, "M553_LEFT_WALL_ORNAMENT_ORDINAL");
    expect_int("lane.d3r.side_view_wall", model ? model->lanes[1].side_view_wall : 0,
               3, "M576_VIEW_WALL_D3R_LEFT");
    expect_int("lane.d3r.front_call", model ? model->lanes[1].front_f0107_line : 0,
               6569, "DUNVIEW.C:6569");
    expect_int("lane.d3r.front_slot", model ? model->lanes[1].front_ornament_slot : 0,
               5, "M552_FRONT_WALL_ORNAMENT_ORDINAL");
    expect_int("lane.d3r.front_view_wall", model ? model->lanes[1].front_view_wall : 0,
               6, "M579_VIEW_WALL_D3R_FRONT");
    expect_int("lane.d3r.corridor_order", model ? model->lanes[1].corridor_order : 0,
               0x4312, "DUNVIEW.C:6618");
    expect_int("lane.d3r.door_side_order", model ? model->lanes[1].door_side_order : 0,
               0x0412, "DUNVIEW.C:6576");
    expect_int("lane.d3r.door_pass1", model ? model->lanes[1].door_pass1_order : 0,
               0x0128, "DUNVIEW.C:6580");
    expect_int("lane.d3r.door_pass2", model ? model->lanes[1].door_pass2_order : 0,
               0x0439, "DUNVIEW.C:6601");
    expect_int("lane.d3r.f0108_open", model ? model->lanes[1].f0108_open_path_line : 0,
               6620, "DUNVIEW.C:6620");
    expect_int("lane.d3r.f0115", model ? model->lanes[1].f0115_line : 0,
               6622, "DUNVIEW.C:6622");
    expect_int("lane.d3r.f0113", model ? model->lanes[1].f0113_field_line : 0,
               6637, "DUNVIEW.C:6637");
    expect_int("lane.d3r.door_zone", model ? model->lanes[1].f0111_door_zone : 0,
               3740, "DEFS.H:4254 M626_ZONE_DOOR_D3R");
}

static void test_ordinals_steps_pixels_and_cell_order(void)
{
    const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_pc34();
    size_t i;
    int direct_count = 0;
    int sensor_count = 0;
    int present_count = 0;
    int absent_count = 0;
    int skips = 0;
    int writes = 0;

    expect_int("ordinal.out_of_range",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_ordinal_at_pc34(7) == NULL,
               1, "ordinal accessor bounds");
    for (i = 0; model && i < DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_FLOW_COUNT_PC34; ++i) {
        const DM1_V1_D3LD3RF0107OrdinalFlowPc34 *o =
            dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_ordinal_at_pc34(i);
        char id[96];
        snprintf(id, sizeof(id), "ordinal.%u.present", (unsigned)i);
        expect_int(id, o != NULL, 1, "ordinal accessor");
        if (!o) continue;
        direct_count += o->reaches_d3l_d3r_f0107;
        sensor_count += o->sensor_provided;
        snprintf(id, sizeof(id), "ordinal.%u.position", (unsigned)i);
        expect_int(id, o->ordinal_position, (int)i, "DEFS.H wall-position order");
        snprintf(id, sizeof(id), "ordinal.%u.sensor", (unsigned)i);
        expect_int(id, o->sensor_provided, 1, "sensor-provided position metadata");
        snprintf(id, sizeof(id), "ordinal.%u.anchor", (unsigned)i);
        expect_contains(id, o->redmcsb_anchor, "D", "ordinal anchor");
    }
    expect_int("ordinal.direct_count", direct_count, 4,
               "D3L/D3R direct positions C2/C3/C4/C6");
    expect_int("ordinal.sensor_count", sensor_count, 7,
               "C0..C6 sensor-position map");
    expect_int("ordinal.c0.keepout", model ? model->ordinals[0].reaches_d3l_d3r_f0107 : 1,
               0, "D3L2 excluded");
    expect_int("ordinal.c1.keepout", model ? model->ordinals[1].reaches_d3l_d3r_f0107 : 1,
               0, "D3R2 excluded");
    expect_int("ordinal.c2.view_wall", model ? model->ordinals[2].view_wall : 0,
               2, "M575 D3L side");
    expect_int("ordinal.c3.view_wall", model ? model->ordinals[3].view_wall : 0,
               3, "M576 D3R side");
    expect_int("ordinal.c4.view_wall", model ? model->ordinals[4].view_wall : 0,
               4, "M577 D3L front");
    expect_int("ordinal.c5.keepout", model ? model->ordinals[5].reaches_d3l_d3r_f0107 : 1,
               0, "D3C sibling owns M578");
    expect_int("ordinal.c6.view_wall", model ? model->ordinals[6].view_wall : 0,
               6, "M579 D3R front");
    expect_int("ordinal.c2.slot", model ? model->ordinals[2].aspect_slot : 0,
               4, "M551 D3L side");
    expect_int("ordinal.c3.slot", model ? model->ordinals[3].aspect_slot : 0,
               6, "M553 D3R side");
    expect_int("ordinal.c4.slot", model ? model->ordinals[4].aspect_slot : 0,
               5, "M552 D3L front");
    expect_int("ordinal.c6.slot", model ? model->ordinals[6].aspect_slot : 0,
               5, "M552 D3R front");

    expect_int("step.out_of_range",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_step_at_pc34(12) == NULL,
               1, "step accessor bounds");
    for (i = 0; i < DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_STEP_COUNT_PC34; ++i) {
        const DM1_V1_D3LD3RF0107StepPc34 *step =
            dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_step_at_pc34(i);
        char id[96];
        snprintf(id, sizeof(id), "step.%u.present", (unsigned)i);
        expect_int(id, step != NULL, 1, "step accessor");
        if (!step) continue;
        present_count += step->expected_present ? 1 : 0;
        absent_count += step->expected_present ? 0 : 1;
        snprintf(id, sizeof(id), "step.%u.order", (unsigned)i);
        expect_int(id, step->order_index, (int)i, "step order stable");
        snprintf(id, sizeof(id), "step.%u.anchor", (unsigned)i);
        expect_contains(id, step->redmcsb_anchor, "DUNVIEW.C", "step anchor");
    }
    expect_int("steps.present_count", present_count, 11,
               "all except F0112 present");
    expect_int("steps.absent_count", absent_count, 1,
               "F0112 absent on D3L/D3R");
    expect_int("step0.kind",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_step_at_pc34(0)->step,
               DM1_V1_D3L_D3R_F0107_STEP_F0128_DISPATCH_D3L_PC34,
               "DUNVIEW.C:8491");
    expect_int("step9.absent",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_step_at_pc34(9)->expected_present,
               0, "no F0112 call in F0116/F0117");
    expect_int("step11.kind",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_step_at_pc34(11)->step,
               DM1_V1_D3L_D3R_F0107_STEP_TERMINAL_DEPTH_SIDE_PAIR_PC34,
               "terminal-depth side-lane pair");

    expect_int("cell.d3l.0", dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x3421, 0),
               0, "C0x3421 BACKLEFT first");
    expect_int("cell.d3l.1", dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x3421, 1),
               1, "C0x3421 BACKRIGHT second");
    expect_int("cell.d3l.2", dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x3421, 2),
               3, "C0x3421 FRONTLEFT third");
    expect_int("cell.d3l.3", dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x3421, 3),
               2, "C0x3421 FRONTRIGHT fourth");
    expect_int("cell.d3r.0", dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x4312, 0),
               1, "C0x4312 BACKRIGHT first");
    expect_int("cell.d3r.1", dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x4312, 1),
               0, "C0x4312 BACKLEFT second");
    expect_int("cell.d3r.2", dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x4312, 2),
               2, "C0x4312 FRONTRIGHT third");
    expect_int("cell.d3r.3", dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x4312, 3),
               3, "C0x4312 FRONTLEFT fourth");
    expect_int("cell.doorpass1.d3l.0",
               dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x0218, 0),
               0, "door pass marker skips low nibble");
    expect_int("cell.doorpass1.d3r.0",
               dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x0128, 0),
               1, "door pass marker skips low nibble");
    expect_int("cell.invalid.negative",
               dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x3421, -1),
               -1, "decode guard");
    expect_int("cell.invalid.high",
               dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(0x3421, 4),
               -1, "decode guard");

    for (i = 0; model && i < DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34; ++i) {
        const DM1_V1_D3LD3RF0107PixelPc34 *p = &model->pixels[i];
        char id[96];
        snprintf(id, sizeof(id), "pixel.%u.after", (unsigned)i);
        expect_int(id, p->after,
                   dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_blend_pixel_pc34(
                       p->before, p->source, 10),
                   "DUNVIEW.C:3922 C10 transparent blit");
        snprintf(id, sizeof(id), "pixel.%u.skip_xor_write", (unsigned)i);
        expect_int(id, p->transparent_skip + p->writes_pixel, 1,
                   "each pixel either skips or writes");
        snprintf(id, sizeof(id), "pixel.%u.anchor", (unsigned)i);
        expect_contains(id, p->anchor, "F0107", "pixel anchor");
        skips += p->transparent_skip;
        writes += p->writes_pixel;
    }
    expect_int("pixel.skip_count", skips, 4, "C10 transparent skips");
    expect_int("pixel.write_count", writes, 4, "non-C10 writes");
}

static void test_helpers_evidence_and_hash(void)
{
    const char *e = dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_source_evidence_pc34();
    const char *d = dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_disjointness_note_pc34();
    uint32_t hash = dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_deterministic_hash_pc34();

    expect_int("alcove.zero.true",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_returns_alcove_pc34(0, true),
               0, "DUNVIEW.C:3568/3571 zero ordinal guard");
    expect_int("alcove.nonzero.false",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_returns_alcove_pc34(3, false),
               0, "DUNVIEW.C:3589 classifier false");
    expect_int("alcove.nonzero.true",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_returns_alcove_pc34(3, true),
               1, "DUNVIEW.C:3933 returns alcove boolean");
    expect_int("zone.d3l.side",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_zone_pc34(2, 2),
               1036, "C1004 + 2*C15 + M575");
    expect_int("zone.d3r.front",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_zone_pc34(2, 6),
               1040, "C1004 + 2*C15 + M579");
    expect_int("zone.invalid",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_zone_pc34(-1, 6),
               -1, "zone helper guard");
    expect_int("blend.transparent",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_blend_pixel_pc34(0xaa, 10, 10),
               0xaa, "DEFS.H:2088 C10 preserves destination");
    expect_int("blend.opaque",
               dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_blend_pixel_pc34(0xaa, 0x51, 10),
               0x51, "F0107 opaque ornament pixel writes");

    expect_contains("evidence.f0107", e, "DUNVIEW.C F0107:3502-3938",
                    "required F0107 anchor");
    expect_contains("evidence.f0116", e, "F0116:6361-6498",
                    "D3L body");
    expect_contains("evidence.f0117", e, "F0117:6500-6640",
                    "D3R body");
    expect_contains("evidence.f0128", e, "F0128:8491-8499",
                    "D3L then D3R then D3C");
    expect_contains("evidence.f0128_d2", e, "8503-8517",
                    "D2 order contrast");
    expect_contains("evidence.f0108", e, "F0108:3940-4011",
                    "F0108 baseline");
    expect_contains("evidence.f0112", e, "F0112",
                    "F0112 ordering note");
    expect_contains("evidence.f0113", e, "F0113",
                    "F0113 teleporter ordering");
    expect_contains("evidence.f0115", e, "F0115:4547-4581",
                    "F0115 nibble-walk");
    expect_contains("evidence.f0163", e, "F0163:1769-1838",
                    "DUNGEON.C F0163");
    expect_contains("evidence.f0164", e, "F0164:1840-1905",
                    "DUNGEON.C F0164");
    expect_contains("evidence.f0172", e, "F0172:2466-2523",
                    "DUNGEON.C F0172");
    expect_contains("evidence.c10", e, "DEFS.H:2088",
                    "C10 transparency");
    expect_contains("evidence.m550", e, "M550/M551/M552/M553",
                    "sensor slots");
    expect_contains("evidence.m575", e, "C0/C1/M575..M579",
                    "wall positions");
    expect_contains("evidence.c705", e, "C705/C706",
                    "wall zones");
    expect_contains("evidence.g0206", e, "G0205/G0206/G0207/G0208",
                    "zone math arrays");
    expect_contains("evidence.terminal", e, "terminal-depth side pair",
                    "terminal-depth correction");

    expect_contains("disjoint.d3", d, "D3L/D3R F0107",
                    "disjointness note");
    expect_contains("disjoint.d0", d, "D0L/D0R",
                    "does not duplicate D0L/D0R");
    expect_contains("disjoint.d1c", d, "D1C",
                    "does not duplicate D1C");
    expect_contains("disjoint.d2", d, "D2L/D2R",
                    "does not duplicate D2L/D2R");
    expect_contains("disjoint.no_dos", d, "original DOS pixel parity",
                    "no DOS pixel parity claim");
    expect_contains("disjoint.no_assets", d, "GRAPHICS.DAT",
                    "asset-free contract");

    expect_int("hash.nonzero", hash != 0u, 1, "deterministic hash exists");
    expect_u32("hash.stable", hash, 0xef6f1322u,
               "deterministic D3L/D3R F0107 wall-ornament source-lock hash");
}

int main(void)
{
    test_model_core();
    test_lanes();
    test_ordinals_steps_pixels_and_cell_order();
    test_helpers_evidence_and_hash();

    if (g_failures) {
        printf("FAIL DM1_V1_VIEWPORT_D3L_D3R_F0107_WALL_ORNAMENT_PC34_COMPAT assertions=%d failures=%d deterministic_hash=0x%08x\n",
               g_assertions, g_failures,
               (unsigned)dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_deterministic_hash_pc34());
        return 1;
    }
    printf("DM1_V1_VIEWPORT_D3L_D3R_F0107_WALL_ORNAMENT_PC34_COMPAT_OK assertions=%d failures=0 deterministic_hash=0x%08x\n",
           g_assertions,
           (unsigned)dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_deterministic_hash_pc34());
    return 0;
}
