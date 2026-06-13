#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1l_d1r_f0115_thing_pass_pc34_compat.h"

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
               id, (unsigned int)got, (unsigned int)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == 0x%08x anchor=%s\n", id, (unsigned int)want, anchor);
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

static void test_lane_accessors(void)
{
    const DM1V1D1LD1RF0115LanePc34Data *d1l;
    const DM1V1D1LD1RF0115LanePc34Data *d1r;

    dm1_v1_viewport_d1l_d1r_f0115_thing_pass_init_pc34();
    d1l = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_for_view_square_pc34(4);
    d1r = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_for_view_square_pc34(5);

    expect_int("lane.count", (int)dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:8524-8529 D1L/D1R pair");
    expect_int("lane.at0.is.d1l",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0) == d1l,
               1, "ReDMCSB DUNVIEW.C:8524-8525 D1L before D1R");
    expect_int("lane.at1.is.d1r",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(1) == d1r,
               1, "ReDMCSB DUNVIEW.C:8528-8529 D1R after D1L");
    expect_int("lane.at2.null",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(2) == NULL,
               1, "contract accessor bounds");
    expect_int("view.square.unknown.null",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_for_view_square_pc34(6) == NULL,
               1, "ReDMCSB DEFS.H:2602 M603_VIEW_SQUARE_D2C is disjoint");
    expect_int("d1l.present", d1l != NULL, 1,
               "ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L");
    expect_int("d1r.present", d1r != NULL, 1,
               "ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R");
}

static void test_lane_metadata(void)
{
    const DM1V1D1LD1RF0115LanePc34Data *lanes[2];
    int i;

    lanes[0] = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    lanes[1] = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(1);
    for (i = 0; i < 2; ++i) {
        const DM1V1D1LD1RF0115LanePc34Data *lane = lanes[i];
        const int is_d1l = (i == 0);
        expect_int(is_d1l ? "d1l.lane" : "d1r.lane", lane ? lane->lane : -1, i,
                   "lane enum is the two-lane pair only");
        expect_int(is_d1l ? "d1l.relative.depth" : "d1r.relative.depth",
                   lane ? lane->relative_depth : -1, 1,
                   "ReDMCSB DUNVIEW.C:8524/8528 relative depth 1");
        expect_int(is_d1l ? "d1l.relative.lateral" : "d1r.relative.lateral",
                   lane ? lane->relative_lateral : 0, is_d1l ? -1 : 1,
                   "ReDMCSB DUNVIEW.C:8524/8528 lateral +/-1");
        expect_int(is_d1l ? "d1l.f0128.depth" : "d1r.f0128.depth",
                   lane ? lane->f0128_relative_depth : -1, 1,
                   "ReDMCSB F0150 call arguments in DUNVIEW.C:8524/8528");
        expect_int(is_d1l ? "d1l.f0128.lateral" : "d1r.f0128.lateral",
                   lane ? lane->f0128_relative_lateral : 0, is_d1l ? -1 : 1,
                   "ReDMCSB F0150 call arguments in DUNVIEW.C:8524/8528");
        expect_int(is_d1l ? "d1l.update.line" : "d1r.update.line",
                   lane ? lane->f0128_update_line : 0, is_d1l ? 8524 : 8528,
                   "ReDMCSB DUNVIEW.C:F0128 update coordinates");
        expect_int(is_d1l ? "d1l.dispatch.line" : "d1r.dispatch.line",
                   lane ? lane->f0128_dispatch_line : 0, is_d1l ? 8525 : 8529,
                   "ReDMCSB DUNVIEW.C:F0128 D1L/D1R dispatch order");
        expect_int(is_d1l ? "d1l.view.square" : "d1r.view.square",
                   lane ? lane->view_square : -1, is_d1l ? 4 : 5,
                   "ReDMCSB DEFS.H:2600-2601 M607/M608");
        expect_int(is_d1l ? "d1l.view.lane" : "d1r.view.lane",
                   lane ? lane->view_lane : 0, is_d1l ? -1 : 1,
                   "ReDMCSB DUNVIEW.C:371 G2026");
        expect_int(is_d1l ? "d1l.view.depth" : "d1r.view.depth",
                   lane ? lane->view_depth : -1, 1,
                   "ReDMCSB DUNVIEW.C:372 G2027");
        expect_int(is_d1l ? "d1l.item.row" : "d1r.item.row",
                   lane ? lane->item_projectile_row : -1, is_d1l ? 9 : 10,
                   "ReDMCSB DUNVIEW.C:373 G2028");
        expect_int(is_d1l ? "d1l.creature.row" : "d1r.creature.row",
                   lane ? lane->creature_row : -1, is_d1l ? 9 : 10,
                   "ReDMCSB DUNVIEW.C:375 G2033");
        expect_int(is_d1l ? "d1l.explosion.row" : "d1r.explosion.row",
                   lane ? lane->explosion_row : -1, is_d1l ? 12 : 13,
                   "ReDMCSB DUNVIEW.C:376 G2034");
        expect_int(is_d1l ? "d1l.door.zone" : "d1r.door.zone",
                   lane ? lane->door_zone : -1, is_d1l ? 3780 : 3800,
                   "ReDMCSB DEFS.H:4258/4260 M630/M632");
        expect_int(is_d1l ? "d1l.c10" : "d1r.c10",
                   lane ? lane->c10_transparent_color : -1, 10,
                   "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
        expect_int(is_d1l ? "d1l.m550.pc34" : "d1r.m550.pc34",
                   lane ? lane->m550_first_thing_square_aspect_slot : -1, 2,
                   "ReDMCSB DEFS.H:2547-2549 M550_FIRST_THING");
        expect_int(is_d1l ? "d1l.m550.media020" : "d1r.m550.media020",
                   lane ? lane->m550_media020_square_aspect_slot : -1, 1,
                   "ReDMCSB DEFS.H:2535-2536 M550_FIRST_THING");
        expect_int(is_d1l ? "d1l.contract.only" : "d1r.contract.only",
                   lane ? lane->source_locked_contract_only : 0, 1,
                   "contract-only source-lock gate");
        expect_int(is_d1l ? "d1l.no.asset" : "d1r.no.asset",
                   lane ? lane->no_real_asset_bitmap_parity : 0, 1,
                   "no real-asset pixel parity claim");
        expect_int(is_d1l ? "d1l.no.data" : "d1r.no.data",
                   lane ? lane->no_game_data_load : 0, 1,
                   "no GRAPHICS.DAT/DUNGEON.DAT load");
        expect_int(is_d1l ? "d1l.route.count" : "d1r.route.count",
                   lane ? (int)lane->route_count : -1, 3,
                   "door pass1, door pass2, corridor/pit/teleporter");
    }
}

static void test_door_routes(void)
{
    const DM1V1D1LD1RF0115LanePc34Data *d1l =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    const DM1V1D1LD1RF0115LanePc34Data *d1r =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(1);
    const DM1V1D1LD1RF0115RoutePc34 *d1l_p1 =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_route_pc34(
            d1l, DM1_V1_D1L_D1R_F0115_ROUTE_DOOR_PASS1_PC34);
    const DM1V1D1LD1RF0115RoutePc34 *d1l_p2 =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_route_pc34(
            d1l, DM1_V1_D1L_D1R_F0115_ROUTE_DOOR_PASS2_PC34);
    const DM1V1D1LD1RF0115RoutePc34 *d1r_p1 =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_route_pc34(
            d1r, DM1_V1_D1L_D1R_F0115_ROUTE_DOOR_PASS1_PC34);
    const DM1V1D1LD1RF0115RoutePc34 *d1r_p2 =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_route_pc34(
            d1r, DM1_V1_D1L_D1R_F0115_ROUTE_DOOR_PASS2_PC34);

    expect_int("d1l.p1.present", d1l_p1 != NULL, 1, "ReDMCSB DUNVIEW.C:7494");
    expect_int("d1l.p2.present", d1l_p2 != NULL, 1, "ReDMCSB DUNVIEW.C:7508/7536");
    expect_int("d1r.p1.present", d1r_p1 != NULL, 1, "ReDMCSB DUNVIEW.C:7662");
    expect_int("d1r.p2.present", d1r_p2 != NULL, 1, "ReDMCSB DUNVIEW.C:7676/7704");
    expect_int("d1l.p1.caller", d1l_p1 ? d1l_p1->caller_line : 0, 7494,
               "ReDMCSB DUNVIEW.C:7494 F0115 before F0111");
    expect_int("d1l.p1.f0111", d1l_p1 ? d1l_p1->f0111_door_line : 0, 7506,
               "ReDMCSB DUNVIEW.C:7506 F0111 door body");
    expect_int("d1l.p1.raw", d1l_p1 ? d1l_p1->raw_cell_order : 0, 0x0028,
               "ReDMCSB DEFS.H:2663 C0x0028");
    expect_int("d1l.p1.marker", d1l_p1 ? d1l_p1->door_front_marker_nibble : 0, 8,
               "ReDMCSB DUNVIEW.C:4794 MASK0x0008_DOOR_FRONT");
    expect_int("d1l.p1.pass", d1l_p1 ? d1l_p1->door_front_pass : 0, 1,
               "ReDMCSB DUNVIEW.C:4795 (order & 1)+1");
    expect_int("d1l.p1.strip", d1l_p1 ? d1l_p1->stripped_cell_order : 0, 0x0002,
               "ReDMCSB DUNVIEW.C:4796 order >>= 4");
    expect_int("d1l.p1.strip.fn",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_strip_door_order_pc34(0x0028),
               0x0002, "ReDMCSB DUNVIEW.C:4796");
    expect_int("d1l.p1.pass.fn",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_door_pass_from_order_pc34(0x0028),
               1, "ReDMCSB DUNVIEW.C:4795");
    expect_int("d1l.p1.named.cell", d1l_p1 ? d1l_p1->source_named_cells[0] : -1, 2,
               "ReDMCSB DEFS.H:2663 DOORPASS1_BACKRIGHT");
    expect_int("d1l.p1.decoded.cell", d1l_p1 ? d1l_p1->decoded_cells[0] : -1, 1,
               "ReDMCSB DUNVIEW.C:4826 M001_ORDINAL_TO_INDEX");
    expect_int("d1l.p2.caller", d1l_p2 ? d1l_p2->caller_line : 0, 7536,
               "ReDMCSB DUNVIEW.C:7536 F0115 after F0111");
    expect_int("d1l.p2.order.line", d1l_p2 ? d1l_p2->order_assign_line : 0, 7508,
               "ReDMCSB DUNVIEW.C:7508 pass2 order");
    expect_int("d1l.p2.raw", d1l_p2 ? d1l_p2->raw_cell_order : 0, 0x0039,
               "ReDMCSB DEFS.H:2665 C0x0039");
    expect_int("d1l.p2.pass", d1l_p2 ? d1l_p2->door_front_pass : 0, 2,
               "ReDMCSB DUNVIEW.C:4795 (order & 1)+1");
    expect_int("d1l.p2.strip", d1l_p2 ? d1l_p2->stripped_cell_order : 0, 0x0003,
               "ReDMCSB DUNVIEW.C:4796 order >>= 4");
    expect_int("d1l.p2.strip.fn",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_strip_door_order_pc34(0x0039),
               0x0003, "ReDMCSB DUNVIEW.C:4796");
    expect_int("d1l.p2.pass.fn",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_door_pass_from_order_pc34(0x0039),
               2, "ReDMCSB DUNVIEW.C:4795");
    expect_int("d1r.p1.caller", d1r_p1 ? d1r_p1->caller_line : 0, 7662,
               "ReDMCSB DUNVIEW.C:7662 F0115 before F0111");
    expect_int("d1r.p1.f0111", d1r_p1 ? d1r_p1->f0111_door_line : 0, 7674,
               "ReDMCSB DUNVIEW.C:7674 F0111 door body");
    expect_int("d1r.p1.raw", d1r_p1 ? d1r_p1->raw_cell_order : 0, 0x0018,
               "ReDMCSB DEFS.H:2661 C0x0018");
    expect_int("d1r.p1.pass", d1r_p1 ? d1r_p1->door_front_pass : 0, 1,
               "ReDMCSB DUNVIEW.C:4795");
    expect_int("d1r.p1.strip", d1r_p1 ? d1r_p1->stripped_cell_order : 0, 0x0001,
               "ReDMCSB DUNVIEW.C:4796");
    expect_int("d1r.p1.named.cell", d1r_p1 ? d1r_p1->source_named_cells[0] : -1, 3,
               "ReDMCSB DEFS.H:2661 DOORPASS1_BACKLEFT");
    expect_int("d1r.p1.decoded.cell", d1r_p1 ? d1r_p1->decoded_cells[0] : -1, 0,
               "ReDMCSB DUNVIEW.C:4826 M001_ORDINAL_TO_INDEX");
    expect_int("d1r.p2.caller", d1r_p2 ? d1r_p2->caller_line : 0, 7704,
               "ReDMCSB DUNVIEW.C:7704 F0115 after F0111");
    expect_int("d1r.p2.order.line", d1r_p2 ? d1r_p2->order_assign_line : 0, 7676,
               "ReDMCSB DUNVIEW.C:7676 pass2 order");
    expect_int("d1r.p2.raw", d1r_p2 ? d1r_p2->raw_cell_order : 0, 0x0049,
               "ReDMCSB DEFS.H:2667 C0x0049");
    expect_int("d1r.p2.pass", d1r_p2 ? d1r_p2->door_front_pass : 0, 2,
               "ReDMCSB DUNVIEW.C:4795");
    expect_int("d1r.p2.strip", d1r_p2 ? d1r_p2->stripped_cell_order : 0, 0x0004,
               "ReDMCSB DUNVIEW.C:4796");
}

static void test_corridor_pit_teleporter_routes(void)
{
    const DM1V1D1LD1RF0115LanePc34Data *d1l =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    const DM1V1D1LD1RF0115LanePc34Data *d1r =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(1);
    const DM1V1D1LD1RF0115RoutePc34 *routes[2];
    int i;

    routes[0] = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_route_pc34(
        d1l, DM1_V1_D1L_D1R_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34);
    routes[1] = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_route_pc34(
        d1r, DM1_V1_D1L_D1R_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34);
    for (i = 0; i < 2; ++i) {
        const int is_d1l = (i == 0);
        const DM1V1D1LD1RF0115LanePc34Data *lane = is_d1l ? d1l : d1r;
        const DM1V1D1LD1RF0115RoutePc34 *route = routes[i];
        expect_int(is_d1l ? "d1l.cpt.present" : "d1r.cpt.present", route != NULL, 1,
                   "ReDMCSB DUNVIEW.C:7520-7536/7688-7704");
        expect_int(is_d1l ? "d1l.cpt.order.line" : "d1r.cpt.order.line",
                   route ? route->order_assign_line : 0, is_d1l ? 7523 : 7691,
                   "ReDMCSB DUNVIEW.C:7523/7691");
        expect_int(is_d1l ? "d1l.cpt.caller" : "d1r.cpt.caller",
                   route ? route->caller_line : 0, is_d1l ? 7536 : 7704,
                   "ReDMCSB DUNVIEW.C:7536/7704");
        expect_int(is_d1l ? "d1l.cpt.raw" : "d1r.cpt.raw",
                   route ? route->raw_cell_order : 0, is_d1l ? 0x0032 : 0x0041,
                   "ReDMCSB DEFS.H:2664/2666");
        expect_int(is_d1l ? "d1l.cpt.strip.same" : "d1r.cpt.strip.same",
                   route ? route->stripped_cell_order : 0, route ? route->raw_cell_order : -1,
                   "non-door F0115 order is not stripped");
        expect_int(is_d1l ? "d1l.cpt.door.pass.zero" : "d1r.cpt.door.pass.zero",
                   route ? route->door_front_pass : -1, 0,
                   "ReDMCSB DUNVIEW.C:4797-4799 non-door pass");
        expect_int(is_d1l ? "d1l.cpt.requires.corridor" : "d1r.cpt.requires.corridor",
                   route ? route->required_for_corridor : 0, 1,
                   "ReDMCSB DUNVIEW.C:7520-7523/7688-7691");
        expect_int(is_d1l ? "d1l.cpt.requires.pit" : "d1r.cpt.requires.pit",
                   route ? route->required_for_pit : 0, 1,
                   "ReDMCSB DUNVIEW.C:7510-7523/7678-7691");
        expect_int(is_d1l ? "d1l.cpt.requires.teleporter" : "d1r.cpt.requires.teleporter",
                   route ? route->required_for_teleporter : 0, 1,
                   "ReDMCSB DUNVIEW.C:7520-7523/7688-7691");
        expect_int(is_d1l ? "d1l.cpt.named.count" : "d1r.cpt.named.count",
                   route ? (int)route->source_named_cell_count : -1, 2,
                   "ReDMCSB DEFS.H cell order names");
        expect_int(is_d1l ? "d1l.cpt.decoded.count" : "d1r.cpt.decoded.count",
                   route ? (int)route->decoded_cell_count : -1, 2,
                   "ReDMCSB DUNVIEW.C:4826 ordinal-to-index decode");
        expect_int(is_d1l ? "d1l.cpt.named.0" : "d1r.cpt.named.0",
                   route ? route->source_named_cells[0] : -1, is_d1l ? 2 : 3,
                   "ReDMCSB DEFS.H:2664/2666 first source-named cell");
        expect_int(is_d1l ? "d1l.cpt.named.1" : "d1r.cpt.named.1",
                   route ? route->source_named_cells[1] : -1, is_d1l ? 1 : 0,
                   "ReDMCSB DEFS.H:2664/2666 second source-named cell");
        expect_int(is_d1l ? "d1l.cpt.decoded.0" : "d1r.cpt.decoded.0",
                   route ? route->decoded_cells[0] : -1, is_d1l ? 1 : 0,
                   "ReDMCSB DUNVIEW.C:4826 low-to-high first nibble");
        expect_int(is_d1l ? "d1l.cpt.decoded.1" : "d1r.cpt.decoded.1",
                   route ? route->decoded_cells[1] : -1, is_d1l ? 2 : 3,
                   "ReDMCSB DUNVIEW.C:4828 low-to-high next nibble");
        expect_int(is_d1l ? "d1l.validate.cpt.order" : "d1r.validate.cpt.order",
                   dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_order_pc34(
                       lane, route ? route->raw_cell_order : 0),
                   1, "supported route order");
    }
}

static void test_depth1_clip_and_m550_slots(void)
{
    const DM1V1D1LD1RF0115LanePc34Data *lanes[2];
    int i;
    int cell;

    lanes[0] = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    lanes[1] = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(1);
    for (i = 0; i < 2; ++i) {
        const DM1V1D1LD1RF0115LanePc34Data *lane = lanes[i];
        const int is_d1l = (i == 0);
        for (cell = 0; cell < 4; ++cell) {
            char id[64];
            snprintf(id, sizeof(id), "%s.depth1.cell%d.kept",
                     is_d1l ? "d1l" : "d1r", cell);
            expect_int(id,
                       dm1_v1_viewport_d1l_d1r_f0115_thing_pass_depth1_keeps_cell_pc34(cell),
                       1, "ReDMCSB DUNVIEW.C:4920-4923 depth-1 keeps all cells");
            snprintf(id, sizeof(id), "%s.depth1.fixture%d.kept",
                     is_d1l ? "d1l" : "d1r", cell);
            expect_int(id, lane ? lane->depth1_cells[cell].kept_by_depth1_clip : 0, 1,
                       "ReDMCSB DUNVIEW.C:4923 no D1 clipping branch");
            snprintf(id, sizeof(id), "%s.m550.cell%d.slot",
                     is_d1l ? "d1l" : "d1r", cell);
            expect_int(id,
                       dm1_v1_viewport_d1l_d1r_f0115_thing_pass_first_thing_slot_pc34(lane, cell),
                       2 + cell, "ReDMCSB DEFS.H:2549 M550_FIRST_THING + cell");
            snprintf(id, sizeof(id), "%s.m550.fixture%d.slot",
                     is_d1l ? "d1l" : "d1r", cell);
            expect_int(id, lane ? lane->depth1_cells[cell].first_thing_square_aspect_slot : 0,
                       2 + cell, "ReDMCSB DEFS.H:2549 M550_FIRST_THING + cell");
            snprintf(id, sizeof(id), "%s.ordinal%d",
                     is_d1l ? "d1l" : "d1r", cell);
            expect_int(id, lane ? lane->depth1_cells[cell].ordinal_low_to_high : 0,
                       cell + 1, "ReDMCSB DUNVIEW.C:4826 M001_ORDINAL_TO_INDEX");
        }
    }
    expect_int("bad.cell.minus1.valid",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_view_cell_pc34(-1),
               0, "reject out-of-range view cell");
    expect_int("bad.cell.4.valid",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_view_cell_pc34(4),
               0, "reject alcove/out-of-lane view cell");
    expect_int("bad.cell.minus1.kept",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_depth1_keeps_cell_pc34(-1),
               0, "reject out-of-range view cell");
    expect_int("bad.cell.4.kept",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_depth1_keeps_cell_pc34(4),
               0, "reject out-of-range view cell");
    expect_int("bad.m550.null",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_first_thing_slot_pc34(NULL, 0),
               -1, "reject missing lane");
    expect_int("bad.m550.cell",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_first_thing_slot_pc34(lanes[0], 4),
               -1, "reject out-of-range view cell");
}

static void test_rejections_and_disjointness(void)
{
    const DM1V1D1LD1RF0115LanePc34Data *d1l =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    const DM1V1D1LD1RF0115LanePc34Data *d1r =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(1);

    expect_int("accept.corridor",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_accepts_element_pc34(1), 1,
               "ReDMCSB DUNVIEW.C:7521/7689 C01_ELEMENT_CORRIDOR");
    expect_int("accept.pit",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_accepts_element_pc34(2), 1,
               "ReDMCSB DUNVIEW.C:7510/7678 C02_ELEMENT_PIT");
    expect_int("accept.teleporter",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_accepts_element_pc34(5), 1,
               "ReDMCSB DUNVIEW.C:7520/7688 C05_ELEMENT_TELEPORTER");
    expect_int("accept.door.side",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_accepts_element_pc34(16), 1,
               "ReDMCSB DUNVIEW.C:7489/7657 C16_ELEMENT_DOOR_SIDE");
    expect_int("accept.door.front",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_accepts_element_pc34(17), 1,
               "ReDMCSB DUNVIEW.C:7492/7660 C17_ELEMENT_DOOR_FRONT");
    expect_int("reject.wall",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_rejects_element_pc34(0), 1,
               "ReDMCSB DUNVIEW.C:7436-7460/7604-7628 wall returns");
    expect_int("reject.stairs.side",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_rejects_element_pc34(18), 1,
               "stairs side is not this F0115 contract lane");
    expect_int("reject.stairs.front",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_rejects_element_pc34(19), 1,
               "stairs front jumps to F0115 only through T0122019/T0123019 contrast");
    expect_int("reject.unknown.element",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_rejects_element_pc34(99), 1,
               "reject unsupported elements");
    expect_int("reject.missing.order.d1l",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_order_pc34(d1l, 0),
               0, "F0115:4800 zero is alcove, not D1L/D1R side pair");
    expect_int("reject.missing.order.d1r",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_order_pc34(d1r, 0),
               0, "F0115:4800 zero is alcove, not D1L/D1R side pair");
    expect_int("reject.d1c.order.on.d1l",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_order_pc34(d1l, 0x0218),
               0, "D1C/D2/D3 door order is disjoint");
    expect_int("reject.d1c.order.on.d1r",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_order_pc34(d1r, 0x0128),
               0, "D1C/D2/D3 door order is disjoint");
    expect_int("reject.null.order",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_validate_order_pc34(NULL, 0x0032),
               0, "reject missing cell order/lane");
    expect_int("non.door.pass.zero",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_door_pass_from_order_pc34(0x0032),
               0, "ReDMCSB DUNVIEW.C:4797-4799 non-door");
    expect_int("non.door.strip.same",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_strip_door_order_pc34(0x0041),
               0x0041, "non-door order is not stripped");
}

static void test_evidence_and_hash(void)
{
    const char *e =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_source_evidence_pc34();
    const DM1V1D1LD1RF0115LanePc34Data *d1l =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    const DM1V1D1LD1RF0115LanePc34Data *d1r =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(1);

    expect_u32("deterministic.hash",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_hash_pc34(),
               0x5da7a2edu,
               "deterministic FNV-1a 32-bit model hash");
    expect_contains("evidence.contract", e, "contract-only",
                    "contract-only required by task");
    expect_contains("evidence.no.graphics", e, "no real GRAPHICS.DAT/DUNGEON.DAT",
                    "no real-asset pixel parity claim");
    expect_contains("evidence.no.dos", e, "no original-DOS pixel parity",
                    "contract-only no pixel parity claim");
    expect_contains("evidence.f0115.comment", e, "DUNVIEW.C:4547-4581",
                    "ReDMCSB F0115 thing pass anchor");
    expect_contains("evidence.door.strip", e, "DUNVIEW.C:4794-4800",
                    "ReDMCSB F0115 door-pass nibble strip");
    expect_contains("evidence.depth.clip", e, "DUNVIEW.C:4920-4923",
                    "ReDMCSB F0115 depth clip");
    expect_contains("evidence.c10.blit", e, "DUNVIEW.C:5180-5188",
                    "ReDMCSB F0115 C10 transparent blit");
    expect_contains("evidence.creature.row", e, "DUNVIEW.C:5208-5214",
                    "ReDMCSB F0115 creature row");
    expect_contains("evidence.projectile.row", e, "DUNVIEW.C:5668-5674",
                    "ReDMCSB F0115 projectile row");
    expect_contains("evidence.d1l.door", e, "DUNVIEW.C:7494/7506/7508/7536",
                    "ReDMCSB D1L F0115/F0111 callers");
    expect_contains("evidence.d1r.door", e, "DUNVIEW.C:7662/7674/7676/7704",
                    "ReDMCSB D1R F0115/F0111 callers");
    expect_contains("evidence.f0128", e, "DUNVIEW.C:8524-8529",
                    "ReDMCSB F0128 D1L/D1R dispatch order");
    expect_contains("evidence.defs.c10", e, "DEFS.H:2088",
                    "ReDMCSB C10_COLOR_FLESH");
    expect_contains("evidence.defs.m550", e, "2535-2549 M550_FIRST_THING",
                    "ReDMCSB M550_FIRST_THING");
    expect_contains("evidence.defs.views", e, "2596-2611 view squares",
                    "ReDMCSB D1L/D1R view square ordinals");
    expect_contains("evidence.defs.cells", e, "2642-2677 cell",
                    "ReDMCSB cell orders");
    expect_contains("evidence.defs.wall.contrast", e, "4045-4046",
                    "ReDMCSB D3L/D3R wall-zone contrast");
    expect_contains("evidence.defs.zone.band", e, "4139-4153",
                    "ReDMCSB cell-order zone band");
    expect_contains("evidence.defs.door.zones", e, "4258/4260",
                    "ReDMCSB D1L/D1R door zone");
    expect_contains("evidence.dungeon.f0163", e, "DUNGEON.C:1769-1838",
                    "ReDMCSB F0163 first thing provenance");
    expect_contains("evidence.dungeon.f0164", e, "1840-1905",
                    "ReDMCSB F0164 first thing provenance");
    expect_contains("evidence.dungeon.f0172", e, "2466-2523",
                    "ReDMCSB F0172 square aspect provenance");
    expect_contains("d1l.dispatch.anchor", d1l ? d1l->redmcsb_dispatch_anchor : NULL,
                    "relative (1,-1)", "F0128 D1L relative dispatch");
    expect_contains("d1r.dispatch.anchor", d1r ? d1r->redmcsb_dispatch_anchor : NULL,
                    "relative (1,+1)", "F0128 D1R relative dispatch");
    expect_contains("d1l.f0115.anchor", d1l ? d1l->redmcsb_f0115_anchor : NULL,
                    "4920-4923", "F0115 depth clip in fixture");
    expect_contains("d1r.f0115.anchor", d1r ? d1r->redmcsb_f0115_anchor : NULL,
                    "5180-5188", "F0115 transparent blit in fixture");
}

int main(void)
{
    test_lane_accessors();
    test_lane_metadata();
    test_door_routes();
    test_corridor_pit_teleporter_routes();
    test_depth1_clip_and_m550_slots();
    test_rejections_and_disjointness();
    test_evidence_and_hash();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d1l_d1r_f0115_thing_pass_pc34_compat "
               "failures=%d assertionCount=%d hash=0x%08x\n",
               g_failures, g_assertions,
               (unsigned int)dm1_v1_viewport_d1l_d1r_f0115_thing_pass_hash_pc34());
        return 1;
    }
    printf("PASS dm1_v1_viewport_d1l_d1r_f0115_thing_pass_pc34_compat "
           "assertionCount=%d hash=0x%08x\n",
           g_assertions,
           (unsigned int)dm1_v1_viewport_d1l_d1r_f0115_thing_pass_hash_pc34());
    return 0;
}
