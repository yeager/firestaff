#include "dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

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
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void test_accessors_and_contract_markers(void)
{
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *d3l2;
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *d3r2;

    dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_init_pc34();
    d3l2 = dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(1);
    d3r2 = dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("count", (int)dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:6235/6293 two D3 side helper routes");
    expect_int("d3l2.present", d3l2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:6235-6290 F0676_DrawD3L2");
    expect_int("d3r2.present", d3r2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:6293-6357 F0677_DrawD3R2");
    expect_int("unknown.null",
               dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(9) == NULL,
               1, "ReDMCSB DEFS.H:2610-2611 only side=1/2 are this gate");
    expect_int("at0.d3l2",
               dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_at_pc34(0) == d3l2,
               1, "ReDMCSB DUNVIEW.C:8479-8482 D3L2 dispatch first");
    expect_int("at1.d3r2",
               dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_at_pc34(1) == d3r2,
               1, "ReDMCSB DUNVIEW.C:8483-8486 D3R2 dispatch second");
    expect_int("past.end.null",
               dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_at_pc34(2) == NULL,
               1, "contract-only accessor bounds");
    expect_int("d3l2.contract_only", d3l2 ? d3l2->source_locked_contract_only : 0,
               1, "contract-only source-lock marker");
    expect_int("d3r2.contract_only", d3r2 ? d3r2->source_locked_contract_only : 0,
               1, "contract-only source-lock marker");
    expect_int("d3l2.no_asset_parity", d3l2 ? d3l2->no_real_asset_bitmap_parity : 0,
               1, "contract-only no real-asset bitmap parity");
    expect_int("d3r2.no_asset_parity", d3r2 ? d3r2->no_real_asset_bitmap_parity : 0,
               1, "contract-only no real-asset bitmap parity");
    expect_int("d3l2.no_game_data", d3l2 ? d3l2->no_game_data_load : 0,
               1, "contract-only synthetic fixture");
    expect_int("d3r2.no_game_data", d3r2 ? d3r2->no_game_data_load : 0,
               1, "contract-only synthetic fixture");
}

static void test_fixture_metadata_one(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *f,
    int side,
    int view_square,
    int lane,
    int rel_right,
    int f0128_order,
    int wall_prepass_order,
    int x_first,
    int x_last,
    int row,
    int explosion_row,
    int field_aspect,
    int wall_zone)
{
    expect_int("fixture.side", f ? f->side : -1, side,
               "ReDMCSB DUNVIEW.C F0676/F0677 side route");
    expect_int("fixture.route_count", f ? f->route_count : -1, 5,
               "ReDMCSB DUNVIEW.C:6253-6286/6320-6353 route families");
    expect_int("fixture.f0115_routes", f ? f->f0115_call_routes : -1, 4,
               "ReDMCSB wall route returns; all non-wall routes call F0115");
    expect_int("fixture.f0172_count", f ? f->f0172_square_aspect_count : 0, 1,
               "ReDMCSB DUNGEON.C:2466-2589 F0172");
    expect_int("fixture.view_square", f ? f->view_square_index : -1, view_square,
               "ReDMCSB DEFS.H:2610-2611 C14/C15");
    expect_int("fixture.depth", f ? f->view_depth : -1, 3,
               "ReDMCSB DUNVIEW.C:372 G2027[14/15]");
    expect_int("fixture.lane", f ? f->view_lane : 99, lane,
               "ReDMCSB DUNVIEW.C:371 G2026[14/15]");
    expect_int("fixture.forward", f ? f->relative_forward : -1, 3,
               "ReDMCSB DUNVIEW.C:8481/8485 relative forward");
    expect_int("fixture.right", f ? f->relative_right : 99, rel_right,
               "ReDMCSB DUNVIEW.C:8481/8485 relative lateral");
    expect_int("fixture.f0128_order", f ? f->f0128_draw_order_index : -1, f0128_order,
               "ReDMCSB DUNVIEW.C:8478-8486 F0128 draw order");
    expect_int("fixture.wall_prepass_order", f ? f->wall_prepass_draw_order_index : -1,
               wall_prepass_order, "ReDMCSB DUNVIEW.C:8446-8464 wall-set prepass");
    expect_int("fixture.x_first", f ? f->frame_viewport_x_first : -1, x_first,
               "ReDMCSB DUNVIEW.C:579-580 G0711/G0712 frame x1");
    expect_int("fixture.x_last", f ? f->frame_viewport_x_last : -1, x_last,
               "ReDMCSB DUNVIEW.C:579-580 G0711/G0712 frame x2");
    expect_int("fixture.y_first", f ? f->frame_viewport_y_first : -1, 25,
               "ReDMCSB DUNVIEW.C:579-580 frame y1");
    expect_int("fixture.y_last", f ? f->frame_viewport_y_last : -1, 73,
               "ReDMCSB DUNVIEW.C:579-580 frame y2");
    expect_int("fixture.source_x", f ? f->frame_source_x_first : -1, 0,
               "ReDMCSB DUNVIEW.C:579-580 frame source x");
    expect_int("fixture.source_y", f ? f->frame_source_y_first : -1, 0,
               "ReDMCSB DUNVIEW.C:579-580 frame source y");
    expect_int("fixture.frame_width", f ? f->frame_width : -1, 16,
               "ReDMCSB DUNVIEW.C:579-580 frame visible width");
    expect_int("fixture.frame_height", f ? f->frame_height : -1, 49,
               "ReDMCSB DUNVIEW.C:579-580 frame height");
    expect_int("fixture.item_row", f ? f->item_projectile_row : -1, row,
               "ReDMCSB DUNVIEW.C:373 G2028[14/15]");
    expect_int("fixture.creature_row", f ? f->creature_row : -1, row,
               "ReDMCSB DUNVIEW.C:375 G2033[14/15]");
    expect_int("fixture.explosion_row", f ? f->explosion_row : -1, explosion_row,
               "ReDMCSB DUNVIEW.C:376 G2034[14/15]");
    expect_int("fixture.field_aspect", f ? f->field_aspect_index : -1, field_aspect,
               "ReDMCSB DUNVIEW.C:377 G2035[14/15]");
    expect_int("fixture.wall_zone", f ? f->wall_zone : -1, wall_zone,
               "ReDMCSB DEFS.H:4042-4043 C702/C703");
    expect_int("fixture.wall_return", f ? f->wall_case_returns_before_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:6264/6331 wall returns before F0115");
    expect_int("fixture.wall_f0107", f ? f->wall_case_calls_f0107 : 0, 1,
               "ReDMCSB DUNVIEW.C:6263/6330 F0107 wall ornament");
    expect_int("fixture.side_f0108", f ? f->side_route_uses_f0108 : 0, 1,
               "ReDMCSB DUNVIEW.C:6284/6351 F0108 side route");
    expect_int("fixture.front_two_passes", f ? f->front_door_two_f0115_passes : 0, 1,
               "ReDMCSB DUNVIEW.C:6271-6274/6338-6341 two F0115 passes");
    expect_int("fixture.corridor_f0108", f ? f->corridor_route_uses_f0108 : 0, 1,
               "ReDMCSB DUNVIEW.C:6282-6286/6349-6353 corridor route");
    expect_int("fixture.teleporter_field", f ? f->teleporter_field_after_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:6288-6290/6355-6357 field after F0115");
}

static void test_fixture_metadata(void)
{
    test_fixture_metadata_one(
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(1),
        1, 14, -2, -2, 3, 0, 0, 15, 3, 6, 0, 702);
    test_fixture_metadata_one(
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(2),
        2, 15, 2, 2, 4, 1, 208, 223, 4, 7, 1, 703);
}

static void test_route_table_one(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *f,
    int route_kind,
    unsigned int order,
    int calls_f0115,
    int calls_f0108,
    int calls_f0111,
    int pass,
    int cell_count,
    int first,
    int second,
    int third,
    int fourth)
{
    const DM1_V1_D3L2D3R2F0115RoutePc34 *r =
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_route_pc34(f, route_kind);

    expect_int("route.present", r != NULL, 1,
               "ReDMCSB DUNVIEW.C:6253-6286/6320-6353 route exists");
    expect_int("route.kind", r ? r->route_kind : -1, route_kind,
               "route lookup preserves route kind");
    expect_int("route.order", r ? (int)r->cell_order : -1, (int)order,
               "ReDMCSB DEFS.H:2658-2677 cell order");
    expect_int("route.calls_f0115", r ? r->calls_f0115 : -1, calls_f0115,
               "ReDMCSB DUNVIEW.C:6286/6353 F0115 call count");
    expect_int("route.calls_f0108", r ? r->calls_f0108_before_f0115 : -1, calls_f0108,
               "ReDMCSB DUNVIEW.C:6284/6351 F0108 before F0115");
    expect_int("route.calls_f0111", r ? r->calls_f0111_between_passes : -1, calls_f0111,
               "ReDMCSB DUNVIEW.C:6272/6339 F0111 between door passes");
    expect_int("route.pass", r ? r->f0115_pass : -1, pass,
               "ReDMCSB DUNVIEW.C:4794-4800 door-front pass nibble");
    expect_int("route.cell_count", r ? r->cell_count : -1, cell_count,
               "ReDMCSB DUNVIEW.C:4561-4564 cell-order nibbles");
    expect_int("route.first_cell", r ? r->first_view_cell : -2, first,
               "ReDMCSB DUNVIEW.C:4826 M001 ordinal-to-index");
    expect_int("route.second_cell", r ? r->second_view_cell : -2, second,
               "ReDMCSB DUNVIEW.C:4826 M001 ordinal-to-index");
    expect_int("route.third_cell", r ? r->third_view_cell : -2, third,
               "ReDMCSB DUNVIEW.C:4828 next nibble");
    expect_int("route.fourth_cell", r ? r->fourth_view_cell : -2, fourth,
               "ReDMCSB DUNVIEW.C:4828 next nibble");
    expect_int("decode.first", dm1_v1_viewport_d3l2_d3r2_f0115_decode_cell_pc34(order, 0),
               first, "ReDMCSB DUNVIEW.C:4794-4828 decode first cell");
    expect_int("decode.second", dm1_v1_viewport_d3l2_d3r2_f0115_decode_cell_pc34(order, 1),
               second, "ReDMCSB DUNVIEW.C:4794-4828 decode second cell");
    expect_contains("route.anchor", r ? r->redmcsb_anchor : NULL, "DUNVIEW.C:",
                    "route carries ReDMCSB anchor");
}

static void test_route_tables(void)
{
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *d3l2 =
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *d3r2 =
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(2);

    test_route_table_one(d3l2, DM1_V1_D3L2_D3R2_F0115_ROUTE_WALL_PC34,
                         0, 0, 0, 0, 0, 0, -1, -1, -1, -1);
    test_route_table_one(d3l2, DM1_V1_D3L2_D3R2_F0115_ROUTE_SIDE_DOOR_OR_STAIRS_PC34,
                         0x0321, 1, 1, 0, 0, 3, 0, 1, 2, -1);
    test_route_table_one(d3l2, DM1_V1_D3L2_D3R2_F0115_ROUTE_FRONT_DOOR_PASS1_PC34,
                         0x0218, 1, 1, 0, 1, 2, 0, 1, -1, -1);
    test_route_table_one(d3l2, DM1_V1_D3L2_D3R2_F0115_ROUTE_FRONT_DOOR_PASS2_PC34,
                         0x0349, 1, 0, 1, 2, 2, 3, 2, -1, -1);
    test_route_table_one(d3l2, DM1_V1_D3L2_D3R2_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34,
                         0x3421, 1, 1, 0, 0, 4, 0, 1, 3, 2);
    test_route_table_one(d3r2, DM1_V1_D3L2_D3R2_F0115_ROUTE_WALL_PC34,
                         0, 0, 0, 0, 0, 0, -1, -1, -1, -1);
    test_route_table_one(d3r2, DM1_V1_D3L2_D3R2_F0115_ROUTE_SIDE_DOOR_OR_STAIRS_PC34,
                         0x0412, 1, 1, 0, 0, 3, 1, 0, 3, -1);
    test_route_table_one(d3r2, DM1_V1_D3L2_D3R2_F0115_ROUTE_FRONT_DOOR_PASS1_PC34,
                         0x0128, 1, 1, 0, 1, 2, 1, 0, -1, -1);
    test_route_table_one(d3r2, DM1_V1_D3L2_D3R2_F0115_ROUTE_FRONT_DOOR_PASS2_PC34,
                         0x0439, 1, 0, 1, 2, 2, 2, 3, -1, -1);
    test_route_table_one(d3r2, DM1_V1_D3L2_D3R2_F0115_ROUTE_CORRIDOR_PIT_TELEPORTER_PC34,
                         0x4312, 1, 1, 0, 0, 4, 1, 0, 2, 3);
    expect_int("route.null_fixture",
               dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_route_pc34(NULL, 0) == NULL,
               1, "invalid fixture guard");
    expect_int("route.unknown_kind",
               dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_route_pc34(d3l2, 99) == NULL,
               1, "invalid route guard");
}

static void test_thing_zone_bindings_one(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *f,
    int item_zone_cell2,
    int item_zone_cell3,
    int projectile_zone_cell2,
    int creature_zone_cell4,
    int explosion_zone_cell0)
{
    expect_int("cell0.clipped",
               dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(f, 0),
               0, "ReDMCSB DUNVIEW.C:4923/5672 depth-3 front-left clipped");
    expect_int("cell1.clipped",
               dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(f, 1),
               0, "ReDMCSB DUNVIEW.C:4923/5672 depth-3 front-right clipped");
    expect_int("cell2.visible",
               dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(f, 2),
               1, "ReDMCSB DUNVIEW.C:4923/5672 depth-3 back-right visible");
    expect_int("cell3.visible",
               dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(f, 3),
               1, "ReDMCSB DUNVIEW.C:4923/5672 depth-3 back-left visible");
    expect_int("cell4.invalid",
               dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(f, 4),
               0, "invalid view-cell guard");
    expect_int("item.cell0.clipped",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(f, 0),
               -1, "ReDMCSB DUNVIEW.C:4923 clipped item cell");
    expect_int("item.cell1.clipped",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(f, 1),
               -1, "ReDMCSB DUNVIEW.C:4923 clipped item cell");
    expect_int("item.cell2.zone",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(f, 2),
               item_zone_cell2, "ReDMCSB DUNVIEW.C:5075 C2500 zone");
    expect_int("item.cell3.zone",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(f, 3),
               item_zone_cell3, "ReDMCSB DUNVIEW.C:5075 C2500 zone");
    expect_int("projectile.cell0.clipped",
               dm1_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(f, 0),
               -1, "ReDMCSB DUNVIEW.C:5668-5675 clipped projectile cell");
    expect_int("projectile.cell2.zone",
               dm1_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(f, 2),
               projectile_zone_cell2, "ReDMCSB DUNVIEW.C:5683 C2900 zone");
    expect_int("creature.coord0.cell0",
               dm1_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(f, 0, 0),
               creature_zone_cell4 - 4, "ReDMCSB DUNVIEW.C:5616 C3200 zone");
    expect_int("creature.coord0.cell4",
               dm1_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(f, 0, 4),
               creature_zone_cell4, "ReDMCSB DUNVIEW.C:5616 C3200 zone");
    expect_int("creature.coord2.cell4",
               dm1_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(f, 2, 4),
               creature_zone_cell4 + 130, "ReDMCSB DUNVIEW.C:5616 coordinate stride 65");
    expect_int("explosion.cell0.zone",
               dm1_v1_viewport_d3l2_d3r2_f0115_explosion_zone_pc34(f, 0),
               explosion_zone_cell0, "ReDMCSB DUNVIEW.C:6122 C3031 zone");
    expect_int("explosion.cell1.zone",
               dm1_v1_viewport_d3l2_d3r2_f0115_explosion_zone_pc34(f, 1),
               explosion_zone_cell0 + 1, "ReDMCSB DUNVIEW.C:6122 C3031 zone");
}

static void test_thing_zone_bindings(void)
{
    test_thing_zone_bindings_one(
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(1),
        2514, 2515, 2914, 3219, 3043);
    test_thing_zone_bindings_one(
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(2),
        2518, 2519, 2918, 3224, 3045);
    expect_int("null.cell_visible",
               dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(NULL, 2),
               0, "invalid input guard");
    expect_int("null.item",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(NULL, 2),
               -1, "invalid input guard");
    expect_int("null.projectile",
               dm1_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(NULL, 2),
               -1, "invalid input guard");
    expect_int("null.creature",
               dm1_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(NULL, 0, 2),
               -1, "invalid input guard");
    expect_int("null.explosion",
               dm1_v1_viewport_d3l2_d3r2_f0115_explosion_zone_pc34(NULL, 0),
               -1, "invalid input guard");
}

static void test_pixel_contract_one(
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *f,
    int x_first,
    int x_last)
{
    uint8_t source[DM1_V1_D3L2_D3R2_F0115_SOURCE_WIDTH_PC34 *
                   DM1_V1_D3L2_D3R2_F0115_SOURCE_HEIGHT_PC34];
    uint8_t viewport[DM1_V1_D3L2_D3R2_F0115_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D3L2_D3R2_F0115_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D3L2D3R2F0115PixelPc34 p;
    size_t write_offset;

    memset(source, DM1_V1_D3L2_D3R2_F0115_C10_COLOR_FLESH_PC34, sizeof(source));
    memset(viewport, 0xee, sizeof(viewport));
    source[1] = 0x31;
    source[(48 * DM1_V1_D3L2_D3R2_F0115_SOURCE_WIDTH_PC34) + 15] = 0x32;

    expect_int("blend.transparent",
               dm1_v1_viewport_d3l2_d3r2_f0115_blend_pixel_pc34(0xee, 10, 10),
               0xee, "ReDMCSB DUNVIEW.C:5180-5188 C10 transparent blit");
    expect_int("blend.opaque",
               dm1_v1_viewport_d3l2_d3r2_f0115_blend_pixel_pc34(0xee, 0x31, 10),
               0x31, "ReDMCSB DUNVIEW.C:5180-5188 opaque source writes");
    expect_int("pixel.c10.apply",
               dm1_v1_viewport_d3l2_d3r2_f0115_apply_pixel_pc34(
                   f, x_first, 25, source, sizeof(source), viewport, sizeof(viewport), &p),
               1, "synthetic C10 pixel");
    expect_int("pixel.c10.in_clip", p.in_viewport_clip ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:579-580 frame clip");
    expect_int("pixel.c10.source_x", p.source_x, 0,
               "ReDMCSB DUNVIEW.C:579-580 source x");
    expect_int("pixel.c10.source_y", p.source_y, 0,
               "ReDMCSB DUNVIEW.C:579-580 source y");
    expect_int("pixel.c10.transparent", p.transparent_skip ? 1 : 0, 1,
               "ReDMCSB DUNVIEW.C:5180-5188 C10 skip");
    expect_int("pixel.c10.no_write", p.writes_pixel ? 1 : 0, 0,
               "C10 preserves destination");
    expect_int("pixel.c10.after", p.destination_after, 0xee,
               "C10 transparent destination preserved");

    expect_int("pixel.write.apply",
               dm1_v1_viewport_d3l2_d3r2_f0115_apply_pixel_pc34(
                   f, x_first + 1, 25, source, sizeof(source), viewport,
                   sizeof(viewport), &p),
               1, "synthetic opaque pixel");
    write_offset = (size_t)25 * DM1_V1_D3L2_D3R2_F0115_VIEWPORT_WIDTH_PC34 +
        (size_t)(x_first + 1);
    expect_int("pixel.write.source_x", p.source_x, 1,
               "ReDMCSB DUNVIEW.C:579-580 source x");
    expect_int("pixel.write.source_y", p.source_y, 0,
               "ReDMCSB DUNVIEW.C:579-580 source y");
    expect_int("pixel.write.source_offset", (int)p.source_offset, 1,
               "source offset source_y * width + source_x");
    expect_int("pixel.write.viewport_offset", (int)p.viewport_offset, (int)write_offset,
               "viewport offset y * viewport width + x");
    expect_int("pixel.write.transparent", p.transparent_skip ? 1 : 0, 0,
               "opaque source pixel");
    expect_int("pixel.write.writes", p.writes_pixel ? 1 : 0, 1,
               "opaque source writes");
    expect_int("pixel.write.after", p.destination_after, 0x31,
               "opaque source overwrites destination");

    expect_int("pixel.bottom.apply",
               dm1_v1_viewport_d3l2_d3r2_f0115_apply_pixel_pc34(
                   f, x_last, 73, source, sizeof(source), viewport, sizeof(viewport), &p),
               1, "synthetic bottom-edge pixel");
    expect_int("pixel.bottom.source_x", p.source_x, 15,
               "ReDMCSB DUNVIEW.C:579-580 source x last");
    expect_int("pixel.bottom.source_y", p.source_y, 48,
               "ReDMCSB DUNVIEW.C:579-580 source y last");
    expect_int("pixel.bottom.source_offset", (int)p.source_offset,
               (48 * DM1_V1_D3L2_D3R2_F0115_SOURCE_WIDTH_PC34) + 15,
               "source bottom-edge offset");
    expect_int("pixel.bottom.after", p.destination_after, 0x32,
               "opaque bottom-edge source writes");

    expect_int("pixel.xclip.apply",
               dm1_v1_viewport_d3l2_d3r2_f0115_apply_pixel_pc34(
                   f, x_last + 1, 73, source, sizeof(source), viewport, sizeof(viewport), &p),
               1, "x outside frame");
    expect_int("pixel.xclip.no_write_metadata", p.no_write_metadata ? 1 : 0, 1,
               "contract records no-write metadata for viewport x clip");
    expect_int("pixel.xclip.in_clip", p.in_viewport_clip ? 1 : 0, 0,
               "x outside viewport frame");
    expect_int("pixel.yclip.apply",
               dm1_v1_viewport_d3l2_d3r2_f0115_apply_pixel_pc34(
                   f, x_first, 74, source, sizeof(source), viewport, sizeof(viewport), &p),
               1, "source y outside frame");
    expect_int("pixel.yclip.no_write_metadata", p.no_write_metadata ? 1 : 0, 1,
               "contract records no-write metadata for source y clip");
    expect_int("pixel.yclip.source_y_clipped", p.source_y_clipped ? 1 : 0, 1,
               "source y clip marker");
    expect_int("pixel.null.fixture",
               dm1_v1_viewport_d3l2_d3r2_f0115_apply_pixel_pc34(
                   NULL, x_first, 25, source, sizeof(source), viewport, sizeof(viewport), &p),
               0, "invalid fixture guard");
}

static void test_pixel_contract(void)
{
    test_pixel_contract_one(
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(1), 0, 15);
    test_pixel_contract_one(
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(2), 208, 223);
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_source_evidence_pc34();
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *d3l2 =
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *d3r2 =
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(2);

    expect_contains("evidence.contract_only", e, "source_locked_contract_only=1",
                    "source evidence");
    expect_contains("evidence.no_asset_parity", e, "no_real_asset_bitmap_parity=1",
                    "source evidence");
    expect_contains("evidence.no_game_data", e, "no_game_data_load=1",
                    "source evidence");
    expect_contains("evidence.f0674", e, "DUNVIEW.C:1943",
                    "required F0674_F0128_sub prototype anchor");
    expect_contains("evidence.f0115", e, "DUNVIEW.C:4547-4581 F0115",
                    "required F0115 anchor");
    expect_contains("evidence.door_nibble", e, "DUNVIEW.C:4794-4800",
                    "door-front nibble anchor");
    expect_contains("evidence.item", e, "DUNVIEW.C:4923",
                    "item visibility anchor");
    expect_contains("evidence.blit", e, "DUNVIEW.C:5180-5188",
                    "C10 blit anchor");
    expect_contains("evidence.creature", e, "DUNVIEW.C:5211-5214",
                    "creature row anchor");
    expect_contains("evidence.projectile", e, "DUNVIEW.C:5668-5675",
                    "projectile clip anchor");
    expect_contains("evidence.explosion", e, "DUNVIEW.C:5920-5923",
                    "explosion row anchor");
    expect_contains("evidence.frames", e, "DUNVIEW.C:579-580",
                    "per-frame bitmap anchor");
    expect_contains("evidence.g0711", e, "G0711",
                    "D3L2 per-frame bitmap");
    expect_contains("evidence.g0712", e, "G0712",
                    "D3R2 per-frame bitmap");
    expect_contains("evidence.not_g0163", e, "does not use G0163[M602/M603]",
                    "D3L2/D3R2 frame ordinal correction");
    expect_contains("evidence.defs_c14_c15", e, "DEFS.H:2610-2611",
                    "C14/C15 D3L2/D3R2 view-square symbols");
    expect_contains("evidence.f0676", e, "DUNVIEW.C:6235-6290 F0676_DrawD3L2",
                    "actual D3L2 helper");
    expect_contains("evidence.f0677", e, "6293-6357 F0677_DrawD3R2",
                    "actual D3R2 helper");
    expect_contains("evidence.wall_prepass", e, "DUNVIEW.C:8446-8464",
                    "wall-set draw order");
    expect_contains("evidence.f0128_dispatch", e, "DUNVIEW.C:8478-8486 F0128",
                    "F0128 thing-pass dispatch");
    expect_contains("evidence.view_maps", e, "DUNVIEW.C:371-377",
                    "view-square maps");
    expect_contains("evidence.f0163", e, "DUNGEON.C:1769-1838 F0163",
                    "DUNGEON.C F0163 anchor");
    expect_contains("evidence.f0164", e, "1840-1937 F0164",
                    "DUNGEON.C F0164 anchor");
    expect_contains("evidence.f0172", e, "2466-2589 F0172",
                    "DUNGEON.C F0172 anchor");
    expect_contains("evidence.cell_orders", e, "DEFS.H:2642-2677",
                    "view cell/order definitions");
    expect_contains("evidence.wall_zones", e, "DEFS.H:4042-4043",
                    "C702/C703 wall zone definitions");
    expect_contains("evidence.zone_families", e, "DEFS.H:4228-4236",
                    "F0115 zone family definitions");
    expect_contains("d3l2.dispatch.anchor", d3l2 ? d3l2->redmcsb_dispatch_anchor : NULL,
                    "F0676_DrawD3L2", "fixture D3L2 dispatch anchor");
    expect_contains("d3r2.dispatch.anchor", d3r2 ? d3r2->redmcsb_dispatch_anchor : NULL,
                    "F0677_DrawD3R2", "fixture D3R2 dispatch anchor");
    expect_contains("d3l2.f0115.anchor", d3l2 ? d3l2->redmcsb_f0115_anchor : NULL,
                    "DUNVIEW.C:4547-4581", "fixture F0115 anchor");
    expect_contains("d3r2.f0115.anchor", d3r2 ? d3r2->redmcsb_f0115_anchor : NULL,
                    "5668-5675", "fixture projectile anchor");
    expect_contains("d3l2.bitmap.anchor", d3l2 ? d3l2->redmcsb_bitmap_anchor : NULL,
                    "G0711", "fixture bitmap anchor");
    expect_contains("d3r2.bitmap.anchor", d3r2 ? d3r2->redmcsb_bitmap_anchor : NULL,
                    "G0712", "fixture bitmap anchor");
    expect_contains("d3l2.dungeon.anchor", d3l2 ? d3l2->redmcsb_dungeon_anchor : NULL,
                    "F0163", "fixture DUNGEON.C anchor");
    expect_contains("d3r2.dungeon.anchor", d3r2 ? d3r2->redmcsb_dungeon_anchor : NULL,
                    "F0172", "fixture DUNGEON.C anchor");
    expect_int("d3l2.evidence.pointer", d3l2 ? (d3l2->source_lines == e) : 0,
               1, "source evidence pointer");
    expect_int("d3r2.evidence.pointer", d3r2 ? (d3r2->source_lines == e) : 0,
               1, "source evidence pointer");
}

int main(void)
{
    test_accessors_and_contract_markers();
    test_fixture_metadata();
    test_route_tables();
    test_thing_zone_bindings();
    test_pixel_contract();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
