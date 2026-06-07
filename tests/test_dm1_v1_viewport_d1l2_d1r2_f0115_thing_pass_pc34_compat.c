#include "dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_pc34_compat.h"

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
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *d1l2;
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *d1r2;

    dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_init_pc34();
    d1l2 = dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1);
    d1r2 = dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("count", (int)dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:7523/7691 D1L/D1R two side-lane corridor routes");
    expect_int("d1l2.present", d1l2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:7391-7557 F0122_DUNGEONVIEW_DrawSquareD1L");
    expect_int("d1r2.present", d1r2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:7559-7725 F0123_DUNGEONVIEW_DrawSquareD1R");
    expect_int("unknown.null",
               dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(9) == NULL,
               1, "ReDMCSB DEFS.H:2596-2606 only side=1/2 are this gate");
    expect_int("at0.d1l2",
               dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_at_pc34(0) == d1l2,
               1, "ReDMCSB DUNVIEW.C:7523 left side first fixture");
    expect_int("at1.d1r2",
               dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_at_pc34(1) == d1r2,
               1, "ReDMCSB DUNVIEW.C:7691 right side second fixture");
    expect_int("past.end.null",
               dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_at_pc34(2) == NULL,
               1, "contract-only accessor bounds");
    expect_int("d1l2.contract_only", d1l2 ? d1l2->source_locked_contract_only : 0,
               1, "ReDMCSB DUNVIEW.C:4547 F0115 contract-only marker");
    expect_int("d1r2.contract_only", d1r2 ? d1r2->source_locked_contract_only : 0,
               1, "ReDMCSB DUNVIEW.C:4547 F0115 contract-only marker");
    expect_int("d1l2.no_asset_parity", d1l2 ? d1l2->no_real_asset_bitmap_parity : 0,
               1, "contract-only no real-asset bitmap parity");
    expect_int("d1r2.no_asset_parity", d1r2 ? d1r2->no_real_asset_bitmap_parity : 0,
               1, "contract-only no real-asset bitmap parity");
    expect_int("d1l2.no_game_data", d1l2 ? d1l2->no_game_data_load : 0,
               1, "contract-only synthetic fixture, no DUNGEON.DAT/GRAPHICS.DAT load");
    expect_int("d1r2.no_game_data", d1r2 ? d1r2->no_game_data_load : 0,
               1, "contract-only synthetic fixture, no DUNGEON.DAT/GRAPHICS.DAT load");
}

static void test_route_metadata(void)
{
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *d1l2 =
        dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *d1r2 =
        dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d1l2.side", d1l2 ? d1l2->side : -1, 1,
               "ReDMCSB DUNVIEW.C:6789/7523 F0122 D1L side");
    expect_int("d1r2.side", d1r2 ? d1r2->side : -1, 2,
               "ReDMCSB DUNVIEW.C:6773/7691 F0123 D1R side");
    expect_int("d1l2.route_count", d1l2 ? d1l2->route_count : -1, 1,
               "ReDMCSB DUNVIEW.C:7520-7536 single corridor/open-floor F0115 route");
    expect_int("d1r2.route_count", d1r2 ? d1r2->route_count : -1, 1,
               "ReDMCSB DUNVIEW.C:7688-7704 single corridor/open-floor F0115 route");
    expect_int("d1l2.f0115_call_count", d1l2 ? d1l2->f0115_call_count : -1, 1,
               "ReDMCSB DUNVIEW.C:7536 one F0115 call after F0108/F0112");
    expect_int("d1r2.f0115_call_count", d1r2 ? d1r2->f0115_call_count : -1, 1,
               "ReDMCSB DUNVIEW.C:7704 one F0115 call after F0108/F0112");
    expect_int("d1l2.no_wall", d1l2 ? d1l2->wall_no_wall_flag : -1, 0,
               "ReDMCSB DUNVIEW.C:7436-7460 wall branch returns before this route");
    expect_int("d1r2.no_wall", d1r2 ? d1r2->wall_no_wall_flag : -1, 0,
               "ReDMCSB DUNVIEW.C:7604-7628 wall branch returns before this route");
    expect_int("d1l2.no_f0107", d1l2 ? d1l2->no_f0107_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:7459 F0107 is wall-only before return");
    expect_int("d1r2.no_f0107", d1r2 ? d1r2->no_f0107_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:7627 F0107 is wall-only before return");
    expect_int("d1l2.no_f0111", d1l2 ? d1l2->no_f0111_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:7492-7507 F0111 only in door-front branch");
    expect_int("d1r2.no_f0111", d1r2 ? d1r2->no_f0111_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:7660-7675 F0111 only in door-front branch");
    expect_int("d1l2.c10_flag", d1l2 ? d1l2->c10_transparency_flag : 0, 1,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("d1r2.c10_flag", d1r2 ? d1r2->c10_transparency_flag : 0, 1,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
}

static void test_view_square_and_order_metadata(void)
{
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *d1l2 =
        dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *d1r2 =
        dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d1l2.view_square", d1l2 ? d1l2->view_square_index : -1, 4,
               "ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L");
    expect_int("d1r2.view_square", d1r2 ? d1r2->view_square_index : -1, 5,
               "ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R");
    expect_int("d1l2.depth", d1l2 ? d1l2->view_depth : -1, 1,
               "ReDMCSB DUNVIEW.C:372 G2027[4]");
    expect_int("d1r2.depth", d1r2 ? d1r2->view_depth : -1, 1,
               "ReDMCSB DUNVIEW.C:372 G2027[5]");
    expect_int("d1l2.lane", d1l2 ? d1l2->view_lane : 99, -1,
               "ReDMCSB DUNVIEW.C:371 G2026[4]");
    expect_int("d1r2.lane", d1r2 ? d1r2->view_lane : 99, 1,
               "ReDMCSB DUNVIEW.C:371 G2026[5]");
    expect_int("d1l2.order", d1l2 ? (int)d1l2->f0115_cell_order : -1, 0x0032,
               "ReDMCSB DUNVIEW.C:7523; DEFS.H:2664 C0x0032");
    expect_int("d1r2.order", d1r2 ? (int)d1r2->f0115_cell_order : -1, 0x0041,
               "ReDMCSB DUNVIEW.C:7691; DEFS.H:2666 C0x0041");
    expect_int("d1l2.first_cell", d1l2 ? d1l2->f0115_first_cell : -1, 2,
               "ReDMCSB DEFS.H:2644 C02_VIEW_CELL_BACK_RIGHT");
    expect_int("d1l2.second_cell", d1l2 ? d1l2->f0115_second_cell : -1, 1,
               "ReDMCSB DEFS.H:2643 C01_VIEW_CELL_FRONT_RIGHT");
    expect_int("d1r2.first_cell", d1r2 ? d1r2->f0115_first_cell : -1, 3,
               "ReDMCSB DEFS.H:2645 C03_VIEW_CELL_BACK_LEFT");
    expect_int("d1r2.second_cell", d1r2 ? d1r2->f0115_second_cell : -1, 0,
               "ReDMCSB DEFS.H:2642 C00_VIEW_CELL_FRONT_LEFT");
    expect_int("d1l2.cell_count", d1l2 ? d1l2->f0115_cell_count : -1, 2,
               "ReDMCSB DUNVIEW.C:4561-4564 F0115 nibbles until zero");
    expect_int("d1r2.cell_count", d1r2 ? d1r2->f0115_cell_count : -1, 2,
               "ReDMCSB DUNVIEW.C:4561-4564 F0115 nibbles until zero");
}

static void test_zone_bindings(void)
{
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *d1l2 =
        dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *d1r2 =
        dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2);

    expect_contains("zone.tag.d1l2", d1l2 ? d1l2->zone_binding_tag : NULL,
                    "C2500 item / C2900 projectile / C3200 creature / C3000 explosion",
                    "ReDMCSB DEFS.H:4228-4236 zone family names");
    expect_contains("zone.tag.d1r2", d1r2 ? d1r2->zone_binding_tag : NULL,
                    "C2500 item / C2900 projectile / C3200 creature / C3000 explosion",
                    "ReDMCSB DEFS.H:4228-4236 zone family names");
    expect_int("d1l2.item_base", d1l2 ? d1l2->item_zone_base : -1, 2500,
               "ReDMCSB DEFS.H:4228 C2500_ZONE_");
    expect_int("d1r2.item_base", d1r2 ? d1r2->item_zone_base : -1, 2500,
               "ReDMCSB DEFS.H:4228 C2500_ZONE_");
    expect_int("d1l2.projectile_base", d1l2 ? d1l2->projectile_zone_base : -1, 2900,
               "ReDMCSB DEFS.H:4230 C2900_ZONE_");
    expect_int("d1r2.projectile_base", d1r2 ? d1r2->projectile_zone_base : -1, 2900,
               "ReDMCSB DEFS.H:4230 C2900_ZONE_");
    expect_int("d1l2.creature_base", d1l2 ? d1l2->creature_zone_base : -1, 3200,
               "ReDMCSB DEFS.H:4236 C3200_ZONE_");
    expect_int("d1r2.creature_base", d1r2 ? d1r2->creature_zone_base : -1, 3200,
               "ReDMCSB DEFS.H:4236 C3200_ZONE_");
    expect_int("d1l2.explosion_base", d1l2 ? d1l2->explosion_zone_base : -1, 3000,
               "ReDMCSB DEFS.H:4232 C3000_ZONE_");
    expect_int("d1r2.explosion_base", d1r2 ? d1r2->explosion_zone_base : -1, 3000,
               "ReDMCSB DEFS.H:4232 C3000_ZONE_");
    expect_int("d1l2.item_row", d1l2 ? d1l2->item_projectile_row : -1, 9,
               "ReDMCSB DUNVIEW.C:373/4811 G2028[M607]");
    expect_int("d1r2.item_row", d1r2 ? d1r2->item_projectile_row : -1, 10,
               "ReDMCSB DUNVIEW.C:373/4811 G2028[M608]");
    expect_int("d1l2.creature_row", d1l2 ? d1l2->creature_row : -1, 9,
               "ReDMCSB DUNVIEW.C:375/5211 G2033[M607]");
    expect_int("d1r2.creature_row", d1r2 ? d1r2->creature_row : -1, 10,
               "ReDMCSB DUNVIEW.C:375/5211 G2033[M608]");
    expect_int("d1l2.explosion_row", d1l2 ? d1l2->explosion_row : -1, 12,
               "ReDMCSB DUNVIEW.C:376/5920 G2034[M607]");
    expect_int("d1r2.explosion_row", d1r2 ? d1r2->explosion_row : -1, 13,
               "ReDMCSB DUNVIEW.C:376/5920 G2034[M608]");
    expect_int("d1l2.item.zone.backright",
               dm1_v1_viewport_d1l2_d1r2_f0115_item_zone_pc34(d1l2, 2),
               (0x8000 | (2500 + 9 * 4 + 2)),
               "ReDMCSB DUNVIEW.C:5075 C2500 + row*4 + ViewCell");
    expect_int("d1r2.item.zone.backleft",
               dm1_v1_viewport_d1l2_d1r2_f0115_item_zone_pc34(d1r2, 3),
               (0x8000 | (2500 + 10 * 4 + 3)),
               "ReDMCSB DUNVIEW.C:5075 C2500 + row*4 + ViewCell");
    expect_int("d1l2.projectile.zone.frontright",
               dm1_v1_viewport_d1l2_d1r2_f0115_projectile_zone_pc34(d1l2, 1),
               2900 + 9 * 4 + 1,
               "ReDMCSB DUNVIEW.C:5668-5683 C2900 + row*4 + ViewCell");
    expect_int("d1r2.projectile.zone.frontleft",
               dm1_v1_viewport_d1l2_d1r2_f0115_projectile_zone_pc34(d1r2, 0),
               2900 + 10 * 4,
               "ReDMCSB DUNVIEW.C:5668-5683 C2900 + row*4 + ViewCell");
    expect_int("d1l2.creature.zone.backright",
               dm1_v1_viewport_d1l2_d1r2_f0115_creature_zone_pc34(d1l2, 2),
               (0x8000 | (3200 + 9 * 5 + 2)),
               "ReDMCSB DUNVIEW.C:5615-5617 C3200 + row*5 + ViewCell");
    expect_int("d1r2.creature.zone.backleft",
               dm1_v1_viewport_d1l2_d1r2_f0115_creature_zone_pc34(d1r2, 3),
               (0x8000 | (3200 + 10 * 5 + 3)),
               "ReDMCSB DUNVIEW.C:5615-5617 C3200 + row*5 + ViewCell");
    expect_int("d1l2.explosion.zone",
               dm1_v1_viewport_d1l2_d1r2_f0115_explosion_zone_pc34(d1l2),
               3012, "ReDMCSB DUNVIEW.C:5916-5923/5998-5999 C3000 + row");
    expect_int("d1r2.explosion.zone",
               dm1_v1_viewport_d1l2_d1r2_f0115_explosion_zone_pc34(d1r2),
               3013, "ReDMCSB DUNVIEW.C:5916-5923/5998-5999 C3000 + row");
    expect_int("bad.item.cell",
               dm1_v1_viewport_d1l2_d1r2_f0115_item_zone_pc34(d1l2, 4),
               -1, "ReDMCSB DEFS.H:2642-2645 only cells 0..3 bind visible thing zones");
    expect_int("bad.projectile.null",
               dm1_v1_viewport_d1l2_d1r2_f0115_projectile_zone_pc34(NULL, 0),
               -1, "ReDMCSB DUNVIEW.C:5668 requires resolved fixture metadata");
    expect_int("bad.creature.cell",
               dm1_v1_viewport_d1l2_d1r2_f0115_creature_zone_pc34(d1r2, -1),
               -1, "ReDMCSB DUNVIEW.C:5615 requires valid view cell");
    expect_int("bad.explosion.null",
               dm1_v1_viewport_d1l2_d1r2_f0115_explosion_zone_pc34(NULL),
               -1, "ReDMCSB DUNVIEW.C:5920 requires resolved view square");
}

static void test_evidence_mentions_required_anchors(void)
{
    const char *e =
        dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_source_evidence_pc34();
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *d1l2 =
        dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D1L2D1R2F0115ThingPassPc34 *d1r2 =
        dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(2);

    expect_contains("evidence.contract_only", e, "source_locked_contract_only=1",
                    "contract-only marker required by task");
    expect_contains("evidence.no_asset", e, "no_real_asset_bitmap_parity=1",
                    "contract-only marker required by task");
    expect_contains("evidence.no_game_data", e, "no_game_data_load=1",
                    "contract-only marker required by task");
    expect_contains("evidence.f0122.prototype", e, "DUNVIEW.C:1948-1961",
                    "ReDMCSB DUNVIEW.C F0122/F0123 declarations");
    expect_contains("evidence.depth2.dispatch", e, "DUNVIEW.C:6773-6793",
                    "ReDMCSB DUNVIEW.C view-depth-2 dispatch around 6789");
    expect_contains("evidence.f0122", e, "DUNVIEW.C:7391-7557",
                    "ReDMCSB DUNVIEW.C F0122_DUNGEONVIEW_DrawSquareD1L");
    expect_contains("evidence.f0123", e, "DUNVIEW.C:7559-7725",
                    "ReDMCSB DUNVIEW.C F0123_DUNGEONVIEW_DrawSquareD1R");
    expect_contains("evidence.d1l2.call", e, "7536",
                    "ReDMCSB DUNVIEW.C:7536 F0115 D1L call");
    expect_contains("evidence.d1r2.call", e, "7704",
                    "ReDMCSB DUNVIEW.C:7704 F0115 D1R call");
    expect_contains("evidence.f0115", e, "DUNVIEW.C:4547-4581",
                    "ReDMCSB DUNVIEW.C F0115 draw order");
    expect_contains("evidence.item", e, "4923",
                    "ReDMCSB DUNVIEW.C:4923 item gate");
    expect_contains("evidence.item.zone", e, "5075",
                    "ReDMCSB DUNVIEW.C:5075 item zone binding");
    expect_contains("evidence.creature", e, "5615-5617",
                    "ReDMCSB DUNVIEW.C:5615-5617 creature zone binding");
    expect_contains("evidence.projectile", e, "5668-5683",
                    "ReDMCSB DUNVIEW.C:5668-5683 projectile zone binding");
    expect_contains("evidence.explosion", e, "5998-5999",
                    "ReDMCSB DUNVIEW.C:5998-5999 explosion zone binding");
    expect_contains("evidence.d1c.order", e, "DUNVIEW.C:7873-7911",
                    "ReDMCSB DUNVIEW.C F0124_DUNGEONVIEW_DrawSquareD1C");
    expect_contains("evidence.defs.c10", e, "DEFS.H:2088",
                    "ReDMCSB DEFS.H C10_COLOR_FLESH");
    expect_contains("evidence.defs.view_squares", e, "DEFS.H:2596-2606",
                    "ReDMCSB DEFS.H D1/D2 view-square contrast");
    expect_contains("evidence.defs.zones", e, "DEFS.H:4228-4236",
                    "ReDMCSB DEFS.H C2500/C2900/C3000/C3200");
    expect_contains("evidence.defs.door_zones", e, "DEFS.H:4250-4260",
                    "ReDMCSB DEFS.H door-zone metadata");
    expect_contains("d1l2.dispatch.anchor", d1l2 ? d1l2->redmcsb_dispatch_anchor : NULL,
                    "F0122_DUNGEONVIEW_DrawSquareD1L",
                    "ReDMCSB DUNVIEW.C:7391-7557 fixture anchor");
    expect_contains("d1r2.dispatch.anchor", d1r2 ? d1r2->redmcsb_dispatch_anchor : NULL,
                    "F0123_DUNGEONVIEW_DrawSquareD1R",
                    "ReDMCSB DUNVIEW.C:7559-7725 fixture anchor");
    expect_contains("d1l2.f0115.anchor", d1l2 ? d1l2->redmcsb_f0115_anchor : NULL,
                    "DUNVIEW.C:4547-4581",
                    "ReDMCSB DUNVIEW.C F0115 fixture anchor");
    expect_contains("d1r2.f0115.anchor", d1r2 ? d1r2->redmcsb_f0115_anchor : NULL,
                    "DUNVIEW.C:4547-4581",
                    "ReDMCSB DUNVIEW.C F0115 fixture anchor");
    expect_int("d1l2.evidence.pointer", d1l2 ? (d1l2->source_lines == e) : 0,
               1, "source evidence pointer");
    expect_int("d1r2.evidence.pointer", d1r2 ? (d1r2->source_lines == e) : 0,
               1, "source evidence pointer");
}

int main(void)
{
    test_accessors_and_contract_markers();
    test_route_metadata();
    test_view_square_and_order_metadata();
    test_zone_bindings();
    test_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_pc34_compat "
               "failures=%d assertionCount=%d\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d1l2_d1r2_f0115_thing_pass_pc34_compat "
           "assertionCount=%d\n", g_assertions);
    return 0;
}
