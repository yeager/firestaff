#include "dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_pc34_compat.h"

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
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2l2;
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2r2;

    dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_init_pc34();
    d2l2 = dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(1);
    d2r2 = dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("count", (int)dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:6837/6868 two D2 side helper routes");
    expect_int("d2l2.present", d2l2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:6837-6865 F0678_DrawD2L2");
    expect_int("d2r2.present", d2r2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:6868-6896 F0679_DrawD2R2");
    expect_int("unknown.null",
               dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(9) == NULL,
               1, "ReDMCSB DEFS.H:2605-2606 only side=1/2 are this gate");
    expect_int("at0.d2l2",
               dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_at_pc34(0) == d2l2,
               1, "ReDMCSB DUNVIEW.C:8503 D2L2 dispatch first");
    expect_int("at1.d2r2",
               dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_at_pc34(1) == d2r2,
               1, "ReDMCSB DUNVIEW.C:8507 D2R2 dispatch second");
    expect_int("past.end.null",
               dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_at_pc34(2) == NULL,
               1, "contract-only accessor bounds");
    expect_int("d2l2.contract_only", d2l2 ? d2l2->source_locked_contract_only : 0,
               1, "ReDMCSB DUNVIEW.C:4547 F0115 contract-only marker");
    expect_int("d2r2.contract_only", d2r2 ? d2r2->source_locked_contract_only : 0,
               1, "ReDMCSB DUNVIEW.C:4547 F0115 contract-only marker");
    expect_int("d2l2.no_asset_parity", d2l2 ? d2l2->no_real_asset_bitmap_parity : 0,
               1, "contract-only no real-asset bitmap parity");
    expect_int("d2r2.no_asset_parity", d2r2 ? d2r2->no_real_asset_bitmap_parity : 0,
               1, "contract-only no real-asset bitmap parity");
    expect_int("d2l2.no_game_data", d2l2 ? d2l2->no_game_data_load : 0,
               1, "contract-only synthetic fixture");
    expect_int("d2r2.no_game_data", d2r2 ? d2r2->no_game_data_load : 0,
               1, "contract-only synthetic fixture");
}

static void test_route_metadata_and_not_routes(void)
{
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2l2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2r2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d2l2.side", d2l2 ? d2l2->side : -1, 1,
               "ReDMCSB DUNVIEW.C:6837 F0678 D2L2 route");
    expect_int("d2r2.side", d2r2 ? d2r2->side : -1, 2,
               "ReDMCSB DUNVIEW.C:6868 F0679 D2R2 route");
    expect_int("d2l2.route_count", d2l2 ? d2l2->route_count : -1, 1,
               "ReDMCSB DUNVIEW.C:6847 switch has a single D2L2 route fixture");
    expect_int("d2r2.route_count", d2r2 ? d2r2->route_count : -1, 1,
               "ReDMCSB DUNVIEW.C:6878 switch has a single D2R2 route fixture");
    expect_int("d2l2.f0115_call_count", d2l2 ? d2l2->f0115_call_count : -1, 0,
               "ReDMCSB DUNVIEW.C:6837-6865 has no F0115 call");
    expect_int("d2r2.f0115_call_count", d2r2 ? d2r2->f0115_call_count : -1, 0,
               "ReDMCSB DUNVIEW.C:6868-6896 has no F0115 call");
    expect_int("d2l2.f0172_count", d2l2 ? d2l2->f0172_square_aspect_count : 0, 1,
               "ReDMCSB DUNVIEW.C:6846 F0172 classification");
    expect_int("d2r2.f0172_count", d2r2 ? d2r2->f0172_square_aspect_count : 0, 1,
               "ReDMCSB DUNVIEW.C:6877 F0172 classification");
    expect_int("d2l2.wall_returns", d2l2 ? d2l2->wall_case_returns_before_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:6862 wall case return");
    expect_int("d2r2.wall_returns", d2r2 ? d2r2->wall_case_returns_before_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:6893 wall case return");
    expect_int("d2l2.teleporter_no_f0115", d2l2 ? d2l2->teleporter_case_has_no_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:6863-6865 teleporter field only");
    expect_int("d2r2.teleporter_no_f0115", d2r2 ? d2r2->teleporter_case_has_no_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:6894-6896 teleporter field only");
    expect_int("d2l2.corridor_absent", d2l2 ? d2l2->corridor_case_absent : 0, 1,
               "ReDMCSB DUNVIEW.C:6847-6865 has no corridor case");
    expect_int("d2r2.corridor_absent", d2r2 ? d2r2->corridor_case_absent : 0, 1,
               "ReDMCSB DUNVIEW.C:6878-6896 has no corridor case");
    expect_int("d2l2.door_absent", d2l2 ? d2l2->door_case_absent : 0, 1,
               "ReDMCSB DUNVIEW.C:6847-6865 has no door case");
    expect_int("d2r2.door_absent", d2r2 ? d2r2->door_case_absent : 0, 1,
               "ReDMCSB DUNVIEW.C:6878-6896 has no door case");
    expect_int("d2l2.no_f0108", d2l2 ? d2l2->no_f0108_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:6837-6865 has no floor ornament call");
    expect_int("d2r2.no_f0108", d2r2 ? d2r2->no_f0108_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:6868-6896 has no floor ornament call");
    expect_int("d2l2.no_f0111", d2l2 ? d2l2->no_f0111_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:6837-6865 has no door call");
    expect_int("d2r2.no_f0111", d2r2 ? d2r2->no_f0111_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:6868-6896 has no door call");
    expect_int("d2l2.no_f0115", d2l2 ? d2l2->no_f0115_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:6837-6865 F0115 NOT-route");
    expect_int("d2r2.no_f0115", d2r2 ? d2r2->no_f0115_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:6868-6896 F0115 NOT-route");
}

static void test_view_square_lane_and_f0128_order(void)
{
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2l2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2r2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d2l2.view_square", d2l2 ? d2l2->view_square_index : -1, 9,
               "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    expect_int("d2r2.view_square", d2r2 ? d2r2->view_square_index : -1, 10,
               "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    expect_int("d2l2.depth", d2l2 ? d2l2->view_depth : -1, 2,
               "ReDMCSB DUNVIEW.C:372 G2027[9]");
    expect_int("d2r2.depth", d2r2 ? d2r2->view_depth : -1, 2,
               "ReDMCSB DUNVIEW.C:372 G2027[10]");
    expect_int("d2l2.lane", d2l2 ? d2l2->view_lane : 99, -2,
               "ReDMCSB DUNVIEW.C:371 G2026[9] signed 254");
    expect_int("d2r2.lane", d2r2 ? d2r2->view_lane : 99, 2,
               "ReDMCSB DUNVIEW.C:371 G2026[10]");
    expect_int("d2l2.forward", d2l2 ? d2l2->relative_forward : -1, 2,
               "ReDMCSB DUNVIEW.C:8503 relative movement forward");
    expect_int("d2r2.forward", d2r2 ? d2r2->relative_forward : -1, 2,
               "ReDMCSB DUNVIEW.C:8507 relative movement forward");
    expect_int("d2l2.right", d2l2 ? d2l2->relative_right : 99, -2,
               "ReDMCSB DUNVIEW.C:8503 relative movement right");
    expect_int("d2r2.right", d2r2 ? d2r2->relative_right : 99, 2,
               "ReDMCSB DUNVIEW.C:8507 relative movement right");
    expect_int("d2l2.draw_order", d2l2 ? d2l2->f0128_draw_order_index : -1, 8,
               "ReDMCSB DUNVIEW.C:8503-8504 after D3C");
    expect_int("d2r2.draw_order", d2r2 ? d2r2->f0128_draw_order_index : -1, 9,
               "ReDMCSB DUNVIEW.C:8507-8508 before D2L");
    expect_int("draw_order.left_before_right",
               (d2l2 ? d2l2->f0128_draw_order_index : 99) <
               (d2r2 ? d2r2->f0128_draw_order_index : -1),
               1, "ReDMCSB DUNVIEW.C:8503-8508 D2L2 then D2R2");
    expect_int("d2l2.cell_order", d2l2 ? (int)d2l2->f0115_cell_order : -1, 0,
               "ReDMCSB DUNVIEW.C:6837-6865 no F0115 cell order");
    expect_int("d2r2.cell_order", d2r2 ? (int)d2r2->f0115_cell_order : -1, 0,
               "ReDMCSB DUNVIEW.C:6868-6896 no F0115 cell order");
    expect_int("d2l2.first_cell", d2l2 ? d2l2->f0115_first_cell : 0, -1,
               "ReDMCSB DUNVIEW.C:4561-4564 no nibbles consumed");
    expect_int("d2r2.first_cell", d2r2 ? d2r2->f0115_first_cell : 0, -1,
               "ReDMCSB DUNVIEW.C:4561-4564 no nibbles consumed");
    expect_int("d2l2.second_cell", d2l2 ? d2l2->f0115_second_cell : 0, -1,
               "ReDMCSB DUNVIEW.C:4561-4564 no nibbles consumed");
    expect_int("d2r2.second_cell", d2r2 ? d2r2->f0115_second_cell : 0, -1,
               "ReDMCSB DUNVIEW.C:4561-4564 no nibbles consumed");
    expect_int("d2l2.cell_count", d2l2 ? d2l2->f0115_cell_count : -1, 0,
               "ReDMCSB DUNVIEW.C:6837-6865 no cells drawn by F0115");
    expect_int("d2r2.cell_count", d2r2 ? d2r2->f0115_cell_count : -1, 0,
               "ReDMCSB DUNVIEW.C:6868-6896 no cells drawn by F0115");
}

static void test_disabled_thing_zone_bindings(void)
{
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2l2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2r2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d2l2.item_row.disabled", d2l2 ? d2l2->item_projectile_row : 0, -1,
               "ReDMCSB DUNVIEW.C:373 G2028[9]");
    expect_int("d2r2.item_row.disabled", d2r2 ? d2r2->item_projectile_row : 0, -1,
               "ReDMCSB DUNVIEW.C:373 G2028[10]");
    expect_int("d2l2.creature_row.disabled", d2l2 ? d2l2->creature_row : 0, -1,
               "ReDMCSB DUNVIEW.C:375 G2033[9]");
    expect_int("d2r2.creature_row.disabled", d2r2 ? d2r2->creature_row : 0, -1,
               "ReDMCSB DUNVIEW.C:375 G2033[10]");
    expect_int("d2l2.explosion_row.disabled", d2l2 ? d2l2->explosion_row : 0, -1,
               "ReDMCSB DUNVIEW.C:376 G2034[9]");
    expect_int("d2r2.explosion_row.disabled", d2r2 ? d2r2->explosion_row : 0, -1,
               "ReDMCSB DUNVIEW.C:376 G2034[10]");
    expect_int("d2l2.item.zone.disabled",
               dm1_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(d2l2, 0),
               -1, "ReDMCSB DUNVIEW.C:4923/5075 unreachable C2500 item zone");
    expect_int("d2r2.item.zone.disabled",
               dm1_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(d2r2, 3),
               -1, "ReDMCSB DUNVIEW.C:4923/5075 unreachable C2500 item zone");
    expect_int("d2l2.projectile.zone.disabled",
               dm1_v1_viewport_d2l2_d2r2_f0115_projectile_zone_pc34(d2l2, 0),
               -1, "ReDMCSB DUNVIEW.C:5668-5671 negative G2028 skips");
    expect_int("d2r2.projectile.zone.disabled",
               dm1_v1_viewport_d2l2_d2r2_f0115_projectile_zone_pc34(d2r2, 3),
               -1, "ReDMCSB DUNVIEW.C:5668-5671 negative G2028 skips");
    expect_int("d2l2.creature.zone.disabled",
               dm1_v1_viewport_d2l2_d2r2_f0115_creature_zone_pc34(d2l2, 0),
               -1, "ReDMCSB DUNVIEW.C:5211-5214 negative G2033 skips");
    expect_int("d2r2.creature.zone.disabled",
               dm1_v1_viewport_d2l2_d2r2_f0115_creature_zone_pc34(d2r2, 3),
               -1, "ReDMCSB DUNVIEW.C:5211-5214 negative G2033 skips");
    expect_int("d2l2.explosion.zone.disabled",
               dm1_v1_viewport_d2l2_d2r2_f0115_explosion_zone_pc34(d2l2),
               -1, "ReDMCSB DUNVIEW.C:5920 negative G2034");
    expect_int("d2r2.explosion.zone.disabled",
               dm1_v1_viewport_d2l2_d2r2_f0115_explosion_zone_pc34(d2r2),
               -1, "ReDMCSB DUNVIEW.C:5920 negative G2034");
    expect_int("null.item",
               dm1_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(NULL, 0),
               -1, "invalid input guard");
    expect_int("null.projectile",
               dm1_v1_viewport_d2l2_d2r2_f0115_projectile_zone_pc34(NULL, 0),
               -1, "invalid input guard");
    expect_int("null.creature",
               dm1_v1_viewport_d2l2_d2r2_f0115_creature_zone_pc34(NULL, 0),
               -1, "invalid input guard");
    expect_int("null.explosion",
               dm1_v1_viewport_d2l2_d2r2_f0115_explosion_zone_pc34(NULL),
               -1, "invalid input guard");
}

static void test_field_and_d2c_non_interference(void)
{
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2l2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2r2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d2l2.field_aspect", d2l2 ? d2l2->field_aspect_index : -1, 5,
               "ReDMCSB DUNVIEW.C:377 G2035[9]");
    expect_int("d2r2.field_aspect", d2r2 ? d2r2->field_aspect_index : -1, 6,
               "ReDMCSB DUNVIEW.C:377 G2035[10]");
    expect_int("d2l2.wall_zone", d2l2 ? d2l2->wall_zone : -1, 707,
               "ReDMCSB DUNVIEW.C:6858/6865 C707_ZONE_WALL_D2L2");
    expect_int("d2r2.wall_zone", d2r2 ? d2r2->wall_zone : -1, 708,
               "ReDMCSB DUNVIEW.C:6889/6896 C708_ZONE_WALL_D2R2");
    expect_int("wall_zone.adjacent",
               (d2l2 ? d2l2->wall_zone : 0) + 1 == (d2r2 ? d2r2->wall_zone : -1),
               1, "ReDMCSB DEFS.H:4047-4048 C707/C708");
    expect_int("d2l2.non_interference", d2l2 ? d2l2->d2c_f0115_non_interference : 0, 1,
               "ReDMCSB DUNVIEW.C:7244-7388 F0121 owns D2C F0115");
    expect_int("d2r2.non_interference", d2r2 ? d2r2->d2c_f0115_non_interference : 0, 1,
               "ReDMCSB DUNVIEW.C:7244-7388 F0121 owns D2C F0115");
    expect_int("d2l2.d2c_square", d2l2 ? d2l2->d2c_view_square_index : -1, 6,
               "ReDMCSB DEFS.H:2602 M603_VIEW_SQUARE_D2C");
    expect_int("d2r2.d2c_square", d2r2 ? d2r2->d2c_view_square_index : -1, 6,
               "ReDMCSB DEFS.H:2602 M603_VIEW_SQUARE_D2C");
    expect_int("d2l2.not_d2c_square",
               d2l2 ? (d2l2->view_square_index != d2l2->d2c_view_square_index) : 0,
               1, "ReDMCSB DEFS.H:2602/2605 distinct view squares");
    expect_int("d2r2.not_d2c_square",
               d2r2 ? (d2r2->view_square_index != d2r2->d2c_view_square_index) : 0,
               1, "ReDMCSB DEFS.H:2602/2606 distinct view squares");
    expect_int("d2l2.d2c_order", d2l2 ? d2l2->d2c_normal_cell_order : -1, 0x3421,
               "ReDMCSB DUNVIEW.C:7368 F0121 D2C normal F0115 order");
    expect_int("d2r2.d2c_order", d2r2 ? d2r2->d2c_normal_cell_order : -1, 0x3421,
               "ReDMCSB DUNVIEW.C:7368 F0121 D2C normal F0115 order");
    expect_int("d2l2.not_d2c_order",
               d2l2 ? ((int)d2l2->f0115_cell_order != d2l2->d2c_normal_cell_order) : 0,
               1, "ReDMCSB DUNVIEW.C:6837-6865 has no D2C 0x3421 order");
    expect_int("d2r2.not_d2c_order",
               d2r2 ? ((int)d2r2->f0115_cell_order != d2r2->d2c_normal_cell_order) : 0,
               1, "ReDMCSB DUNVIEW.C:6868-6896 has no D2C 0x3421 order");
    expect_int("d2l2.d2c_draw_order", d2l2 ? d2l2->d2c_f0128_draw_order_index : -1, 12,
               "ReDMCSB DUNVIEW.C:8520-8521 D2C draw order");
    expect_int("d2r2.d2c_draw_order", d2r2 ? d2r2->d2c_f0128_draw_order_index : -1, 12,
               "ReDMCSB DUNVIEW.C:8520-8521 D2C draw order");
    expect_int("d2l2.before_d2c",
               d2l2 ? (d2l2->f0128_draw_order_index < d2l2->d2c_f0128_draw_order_index) : 0,
               1, "ReDMCSB DUNVIEW.C:8503 before 8521");
    expect_int("d2r2.before_d2c",
               d2r2 ? (d2r2->f0128_draw_order_index < d2r2->d2c_f0128_draw_order_index) : 0,
               1, "ReDMCSB DUNVIEW.C:8507 before 8521");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_source_evidence_pc34();
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2l2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2r2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(2);

    expect_contains("evidence.contract_only", e, "source_locked_contract_only=1",
                    "source evidence");
    expect_contains("evidence.no_asset_parity", e, "no_real_asset_bitmap_parity=1",
                    "source evidence");
    expect_contains("evidence.no_game_data", e, "no_game_data_load=1",
                    "source evidence");
    expect_contains("evidence.f0115", e, "DUNVIEW.C:4547-4581 F0115",
                    "required ReDMCSB F0115 anchor");
    expect_contains("evidence.f0121_alias", e, "DUNVIEW.C:5180-5295",
                    "required task F0121/D2L2 box anchor");
    expect_contains("evidence.f0128_alias", e, "DUNVIEW.C:5668-5671",
                    "required task F0128/D2R2 projectile anchor");
    expect_contains("evidence.f0678", e, "DUNVIEW.C:6837-6865 F0678_DrawD2L2",
                    "actual D2L2 helper in this source snapshot");
    expect_contains("evidence.f0679", e, "DUNVIEW.C:6868-6896 F0679_DrawD2R2",
                    "actual D2R2 helper in this source snapshot");
    expect_contains("evidence.f0128.dispatch", e, "DUNVIEW.C:8503-8508 F0128",
                    "D2L2/D2R2 dispatcher");
    expect_contains("evidence.f0121.d2c", e, "DUNVIEW.C:7244-7388 F0121",
                    "D2C non-interference anchor");
    expect_contains("evidence.d2c.calls", e, "7315 and 7368",
                    "D2C F0115 call sites");
    expect_contains("evidence.view_maps", e, "DUNVIEW.C:371-377",
                    "view-square classification arrays");
    expect_contains("evidence.f0163", e, "DUNGEON.C:1769-1838 F0163",
                    "required DUNGEON.C F0163 anchor");
    expect_contains("evidence.f0164", e, "1840-1937 F0164",
                    "required DUNGEON.C F0164 anchor");
    expect_contains("evidence.f0172", e, "2466-2589 F0172",
                    "required DUNGEON.C F0172 anchor");
    expect_contains("evidence.defs.view", e, "DEFS.H:2602-2606",
                    "D2 view-square definitions");
    expect_contains("evidence.defs.cells", e, "DEFS.H:2642-2677",
                    "cell order definitions");
    expect_contains("evidence.defs.zones", e, "DEFS.H:4228-4236",
                    "F0115 zone definitions");
    expect_contains("d2l2.dispatch.anchor", d2l2 ? d2l2->redmcsb_dispatch_anchor : NULL,
                    "F0678_DrawD2L2", "fixture D2L2 dispatch anchor");
    expect_contains("d2r2.dispatch.anchor", d2r2 ? d2r2->redmcsb_dispatch_anchor : NULL,
                    "F0679_DrawD2R2", "fixture D2R2 dispatch anchor");
    expect_contains("d2l2.f0115.anchor", d2l2 ? d2l2->redmcsb_f0115_anchor : NULL,
                    "DUNVIEW.C:4547-4581", "fixture F0115 anchor");
    expect_contains("d2r2.f0115.anchor", d2r2 ? d2r2->redmcsb_f0115_anchor : NULL,
                    "disabled rows", "fixture F0115 disabled-row anchor");
    expect_contains("d2l2.dungeon.anchor", d2l2 ? d2l2->redmcsb_dungeon_anchor : NULL,
                    "F0163", "fixture DUNGEON.C anchor");
    expect_contains("d2r2.dungeon.anchor", d2r2 ? d2r2->redmcsb_dungeon_anchor : NULL,
                    "F0172", "fixture DUNGEON.C anchor");
    expect_int("d2l2.evidence.pointer", d2l2 ? (d2l2->source_lines == e) : 0,
               1, "source evidence pointer");
    expect_int("d2r2.evidence.pointer", d2r2 ? (d2r2->source_lines == e) : 0,
               1, "source evidence pointer");
}

int main(void)
{
    test_accessors_and_contract_markers();
    test_route_metadata_and_not_routes();
    test_view_square_lane_and_f0128_order();
    test_disabled_thing_zone_bindings();
    test_field_and_d2c_non_interference();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
