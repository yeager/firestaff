#include "csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const char *A_F0676 =
    "ReDMCSB DUNVIEW.C:6226-6291 F0676_DrawD3L2";
static const char *A_F0677 =
    "ReDMCSB DUNVIEW.C:6293-6358 F0677_DrawD3R2";
static const char *A_F0116 =
    "ReDMCSB DUNVIEW.C:6361-6480 F0116_DUNGEONVIEW_DrawSquareD3L";
static const char *A_F0117 =
    "ReDMCSB DUNVIEW.C:6500-6622 F0117_DUNGEONVIEW_DrawSquareD3R";
static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C:4547-4581 F0115 thing-pass order";
static const char *A_PROJECTILE =
    "ReDMCSB DUNVIEW.C:5668-5671 projectile row guard; 5683 C2900 row math";
static const char *A_ITEM =
    "ReDMCSB DUNVIEW.C:4923/5075 C2500|MASK0x8000 item row math";
static const char *A_CREATURE =
    "ReDMCSB DUNVIEW.C:5201-5214/5615-5627 C3200|MASK0x8000 creature row math";
static const char *A_EXPLOSION =
    "ReDMCSB DUNVIEW.C:5915-5933/5998-6122 C3000/C3007/C3014/C3031 explosion rows";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C:8478-8508 F0128 D3L2 then D3R2 dispatch";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088,2610-2611,4042-4043";
static const char *A_COORD =
    "ReDMCSB COORD.C:1129-1193,1058-1123,1243-1252 parent records";
static const char *A_LINEAGE =
    "CSB Viewport.cpp D3L2/D3R2 path and 1903-1906 overlay binding";

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s=%d anchor=%s\n", id, got, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *spec_for(int side, int route)
{
    return csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(
        side == 0 ? 14 : 15,
        route);
}

static int expected_source_pixel(int index)
{
    return (index % 4) == 0 ? 10 : 20 + index;
}

static void seed_pixels(uint8_t *source, uint8_t *destination)
{
    for (int i = 0; i < 32; ++i) {
        source[i] = (uint8_t)expected_source_pixel(i);
        destination[i] = 77;
    }
}

static void test_anchor_table_and_evidence(void)
{
    const char *e =
        csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_source_evidence_pc34();
    const CSB_V1_D3L2D3R2F0115ThingPassEvidencePc34 *ev =
        csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_evidence_pc34();

    expect_contains("evidence.contract", e, "CSB-only source-locked", A_F0115);
    expect_contains("evidence.no_asset", e, "no real-asset bitmap parity", A_F0115);
    expect_contains("evidence.no_game_data", e, "no CSB game-data load", A_F0115);
    expect_contains("evidence.f0676", e, "DUNVIEW.C:6226-6291", A_F0676);
    expect_contains("evidence.f0677", e, "DUNVIEW.C:6293-6358", A_F0677);
    expect_contains("evidence.f0116", e, "DUNVIEW.C:6361-6480", A_F0116);
    expect_contains("evidence.f0117", e, "DUNVIEW.C:6500-6622", A_F0117);
    expect_contains("evidence.f0115", e, "DUNVIEW.C:4547-4581", A_F0115);
    expect_contains("evidence.projectile_guard", e, "5668-5671", A_PROJECTILE);
    expect_contains("evidence.f0128", e, "DUNVIEW.C:8478-8508", A_F0128);
    expect_contains("evidence.c10", e, "DEFS.H:2088", A_DEFS);
    expect_contains("evidence.c702", e, "DEFS.H:4042", A_DEFS);
    expect_contains("evidence.c703", e, "DEFS.H:4043", A_DEFS);
    expect_contains("evidence.c14_square", e, "DEFS.H:2610", A_DEFS);
    expect_contains("evidence.c15_square", e, "DEFS.H:2611", A_DEFS);
    expect_contains("evidence.coord_item", e, "COORD.C:1129-1193", A_COORD);
    expect_contains("evidence.coord_explosion", e, "1058-1123", A_COORD);
    expect_contains("evidence.coord_creature", e, "1243-1252", A_COORD);
    expect_contains("evidence.lineage_d3", e, "D3L2/D3R2 thing-pass path", A_LINEAGE);
    expect_contains("evidence.overlay", e, "CSB Viewport.cpp:1903-1906", A_LINEAGE);
    expect_contains("evidence.no_door_panel", e, "NO-DOOR-PANEL", A_F0115);
    expect_contains("struct.f0676", ev ? ev->f0676_d3l2_lines : 0,
                    "F0676_DrawD3L2", A_F0676);
    expect_contains("struct.f0677", ev ? ev->f0677_d3r2_lines : 0,
                    "F0677_DrawD3R2", A_F0677);
    expect_contains("struct.f0116", ev ? ev->f0116_d3l_lines : 0,
                    "F0116_DUNGEONVIEW_DrawSquareD3L", A_F0116);
    expect_contains("struct.f0117", ev ? ev->f0117_d3r_lines : 0,
                    "F0117_DUNGEONVIEW_DrawSquareD3R", A_F0117);
    expect_contains("struct.f0115", ev ? ev->f0115_order_lines : 0,
                    "4547-4581", A_F0115);
    expect_contains("struct.projectile_guard", ev ? ev->f0115_projectile_guard_lines : 0,
                    "5668-5671", A_PROJECTILE);
    expect_contains("struct.f0128", ev ? ev->f0128_dispatch_lines : 0,
                    "D3L2 before D3R2", A_F0128);
    expect_contains("struct.defs", ev ? ev->defs_lines : 0,
                    "2611 C15", A_DEFS);
    expect_contains("struct.coord", ev ? ev->coord_parent_record_lines : 0,
                    "parent records", A_COORD);
    expect_contains("struct.lineage", ev ? ev->csb_lineage_thing_pass_lines : 0,
                    "D3L2/D3R2", A_LINEAGE);
    expect_contains("struct.overlay", ev ? ev->csb_lineage_overlay_lines : 0,
                    "1903-1906", A_LINEAGE);
}

static void test_spec_table_contract(void)
{
    static const int want_square[2] = { 14, 15 };
    static const int want_lane[2] = { 254, 2 };
    static const int want_lateral[2] = { -2, 2 };
    static const int want_wall[2] = { 702, 703 };
    static const int want_field[2] = { 0, 1 };
    static const int want_g2028[2] = { 3, 4 };
    static const int want_g2033[2] = { 3, 4 };
    static const int want_g2034[2] = { 6, 7 };
    static const int want_open[2] = { 0x3421, 0x4312 };
    static const int want_rear[2] = { 0x0218, 0x0128 };
    static const int want_front[2] = { 0x0349, 0x0439 };
    static const int want_lineage_cell[2] = { 2, 10 };
    static const int want_contents[2] = { 60117, 60121 };
    static const int want_xy[2] = { 60100, 60104 };

    expect_int("init", csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_init_pc34(),
               1, A_F0128);
    expect_int("spec.count",
               (int)csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34(),
               8, A_F0115);
    expect_int("at.out_of_range",
               csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_at_pc34(8) == NULL,
               1, A_F0115);
    expect_int("unknown.route",
               csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(14, 99) == NULL,
               1, A_F0115);
    expect_int("unknown.square",
               csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_route_pc34(12, 1) == NULL,
               1, A_F0116);

    for (int side = 0; side < 2; ++side) {
        for (int route = 1; route <= 4; ++route) {
            const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *s =
                spec_for(side, route);
            char label[96];
            const char *anchor = side == 0 ? A_F0676 : A_F0677;

            snprintf(label, sizeof(label), "side%d.route%d.present", side, route);
            expect_int(label, s != NULL, 1, anchor);
            snprintf(label, sizeof(label), "side%d.route%d.contract", side, route);
            expect_int(label, s ? s->source_locked_contract_only : 0, 1, A_F0115);
            snprintf(label, sizeof(label), "side%d.route%d.no_asset", side, route);
            expect_int(label, s ? s->no_real_asset_bitmap_parity : 0, 1, A_F0115);
            snprintf(label, sizeof(label), "side%d.route%d.no_game_data", side, route);
            expect_int(label, s ? s->no_game_data_load : 0, 1, A_F0115);
            snprintf(label, sizeof(label), "side%d.route%d.view_square", side, route);
            expect_int(label, s ? s->view_square : -1, want_square[side], A_DEFS);
            snprintf(label, sizeof(label), "side%d.route%d.route_id", side, route);
            expect_int(label, s ? s->route : -1, route, A_F0115);
            snprintf(label, sizeof(label), "side%d.route%d.dispatch_order", side, route);
            expect_int(label, s ? s->f0128_dispatch_order : -1, side, A_F0128);
            snprintf(label, sizeof(label), "side%d.route%d.depth", side, route);
            expect_int(label, s ? s->view_depth : -1, 3, A_F0128);
            snprintf(label, sizeof(label), "side%d.route%d.lateral", side, route);
            expect_int(label, s ? s->f0128_relative_lateral : 0, want_lateral[side],
                       A_F0128);
            snprintf(label, sizeof(label), "side%d.route%d.lane", side, route);
            expect_int(label, s ? s->view_lane : -1, want_lane[side], A_F0115);
            snprintf(label, sizeof(label), "side%d.route%d.wall_zone", side, route);
            expect_int(label, s ? s->wall_zone : -1, want_wall[side], A_DEFS);
            snprintf(label, sizeof(label), "side%d.route%d.field_aspect", side, route);
            expect_int(label, s ? s->field_aspect_index : -1, want_field[side],
                       A_F0115);
            snprintf(label, sizeof(label), "side%d.route%d.g2028", side, route);
            expect_int(label, s ? s->g2028_row : -1, want_g2028[side], A_PROJECTILE);
            snprintf(label, sizeof(label), "side%d.route%d.g2033", side, route);
            expect_int(label, s ? s->g2033_row : -1, want_g2033[side], A_CREATURE);
            snprintf(label, sizeof(label), "side%d.route%d.g2034", side, route);
            expect_int(label, s ? s->g2034_row : -1, want_g2034[side], A_EXPLOSION);
            snprintf(label, sizeof(label), "side%d.route%d.open_order", side, route);
            expect_int(label, s ? (int)s->open_cell_order : -1, want_open[side], anchor);
            snprintf(label, sizeof(label), "side%d.route%d.door_rear_order", side, route);
            expect_int(label, s ? (int)s->door_front_rear_f0115_order : -1,
                       want_rear[side], anchor);
            snprintf(label, sizeof(label), "side%d.route%d.door_front_order", side, route);
            expect_int(label, s ? (int)s->door_front_front_f0115_order : -1,
                       want_front[side], anchor);
            snprintf(label, sizeof(label), "side%d.route%d.f0111_present_but_excluded",
                     side, route);
            expect_int(label, s ? s->door_front_f0111_order : 0, 1, anchor);
            snprintf(label, sizeof(label), "side%d.route%d.no_door_panel", side, route);
            expect_int(label, s ? s->no_door_panel_marker : 0, 1, A_F0115);
            snprintf(label, sizeof(label), "side%d.route%d.f0111_excluded", side, route);
            expect_int(label, s ? s->f0111_excluded_from_thing_pass : 0, 1, A_F0115);
            snprintf(label, sizeof(label), "side%d.route%d.inner_non_owner", side, route);
            expect_int(label, s ? s->f0116_f0117_inner_square_non_owner : 0, 1,
                       side == 0 ? A_F0116 : A_F0117);
            snprintf(label, sizeof(label), "side%d.route%d.lineage_cell", side, route);
            expect_int(label, s ? s->csb_lineage_relative_cell : -1,
                       want_lineage_cell[side], A_LINEAGE);
            snprintf(label, sizeof(label), "side%d.route%d.lineage_contents", side, route);
            expect_int(label, s ? s->csb_lineage_contents_opcode : -1,
                       want_contents[side], A_LINEAGE);
            snprintf(label, sizeof(label), "side%d.route%d.lineage_xy", side, route);
            expect_int(label, s ? s->csb_lineage_xy_opcode : -1, want_xy[side],
                       A_LINEAGE);
            snprintf(label, sizeof(label), "side%d.route%d.room_objects_opcode",
                     side, route);
            expect_int(label, s ? s->csb_lineage_std_draw_room_objects_opcode : -1,
                       60006, A_LINEAGE);
        }
    }
}

static void test_route_zone_math(void)
{
    static const int want_row[2] = { 3, 4 };
    static const int want_creature_row[2] = { 3, 4 };
    static const int want_explosion_row[2] = { 6, 7 };

    for (int side = 0; side < 2; ++side) {
        const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *projectile =
            spec_for(side, CSB_V1_D3L2_D3R2_F0115_ROUTE_PROJECTILE_PC34);
        const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *creature =
            spec_for(side, CSB_V1_D3L2_D3R2_F0115_ROUTE_CREATURE_PC34);
        const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *item =
            spec_for(side, CSB_V1_D3L2_D3R2_F0115_ROUTE_ITEM_PC34);
        const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *explosion =
            spec_for(side, CSB_V1_D3L2_D3R2_F0115_ROUTE_EXPLOSION_PC34);
        char label[96];

        snprintf(label, sizeof(label), "side%d.projectile.zone_base", side);
        expect_int(label, projectile ? projectile->projectile_zone_base : -1,
                   2900, A_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.projectile.row_selection", side);
        expect_int(label, projectile ? projectile->projectile_row_selection_uses_g2028 : 0,
                   1, A_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.projectile.row_guard", side);
        expect_int(label, projectile ? projectile->projectile_row_guard_5668_5671 : 0,
                   1, A_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.projectile.front0_suppressed", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(projectile, 0),
                   -1, A_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.projectile.front1_suppressed", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(projectile, 1),
                   -1, A_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.projectile.cell2", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(projectile, 2),
                   2900 + want_row[side] * 4 + 2, A_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.projectile.cell3", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(projectile, 3),
                   2900 + want_row[side] * 4 + 3, A_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.projectile.bad_cell", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(projectile, 4),
                   -1, A_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.projectile.requires_c14", side);
        expect_int(label, projectile ? projectile->projectile_requires_type_c14 : 0,
                   1, A_PROJECTILE);
        snprintf(label, sizeof(label), "side%d.projectile.requires_cell", side);
        expect_int(label, projectile ? projectile->projectile_requires_cell_match : 0,
                   1, A_PROJECTILE);

        snprintf(label, sizeof(label), "side%d.item.zone_base", side);
        expect_int(label, item ? item->item_zone_base : -1, 2500, A_ITEM);
        snprintf(label, sizeof(label), "side%d.item.front0_suppressed", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(item, 0),
                   -1, A_ITEM);
        snprintf(label, sizeof(label), "side%d.item.cell2_layout", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_item_layout_zone_pc34(item, 2),
                   2500 + want_row[side] * 4 + 2, A_ITEM);
        snprintf(label, sizeof(label), "side%d.item.cell3_shifted", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(item, 3),
                   0x8000 | (2500 + want_row[side] * 4 + 3), A_ITEM);
        snprintf(label, sizeof(label), "side%d.item.weapon_to_junk", side);
        expect_int(label, item ? item->item_requires_weapon_to_junk : 0, 1, A_ITEM);
        snprintf(label, sizeof(label), "side%d.item.pile_shift", side);
        expect_int(label, item ? item->item_pile_shift_advances : 0, 1, A_ITEM);

        snprintf(label, sizeof(label), "side%d.creature.zone_base", side);
        expect_int(label, creature ? creature->creature_zone_base : -1, 3200,
                   A_CREATURE);
        snprintf(label, sizeof(label), "side%d.creature.group_marker", side);
        expect_int(label, creature ? creature->creature_requires_group_marker : 0,
                   1, A_CREATURE);
        snprintf(label, sizeof(label), "side%d.creature.front1_suppressed", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(creature, 1, 1),
                   -1, A_CREATURE);
        snprintf(label, sizeof(label), "side%d.creature.coord0_cell2", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(creature, 0, 2),
                   0x8000 | (3200 + want_creature_row[side] * 5 + 2),
                   A_CREATURE);
        snprintf(label, sizeof(label), "side%d.creature.coord1_cell3", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(creature, 1, 3),
                   0x8000 | (3200 + 65 + want_creature_row[side] * 5 + 3),
                   A_CREATURE);

        snprintf(label, sizeof(label), "side%d.explosion.restart", side);
        expect_int(label, explosion ? explosion->explosion_restarts_thing_list_after_cells : 0,
                   1, A_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.explosion.g2034_g2035", side);
        expect_int(label, explosion ? explosion->explosion_restart_uses_g2034_g2035 : 0,
                   1, A_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.explosion.step1", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_explosion_rebirth_step1_zone_pc34(
                       explosion),
                   3000 + want_explosion_row[side], A_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.explosion.step2", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_explosion_rebirth_step2_zone_pc34(
                       explosion),
                   3007 + want_explosion_row[side], A_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.explosion.center", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_explosion_centered_zone_pc34(
                       explosion),
                   3014 + want_explosion_row[side], A_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.explosion.side0", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_explosion_side_zone_pc34(
                       explosion, 0),
                   3031 + want_explosion_row[side] * 2, A_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.explosion.side1", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_explosion_side_zone_pc34(
                       explosion, 1),
                   3031 + want_explosion_row[side] * 2 + 1, A_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.explosion.bad_side_cell", side);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_explosion_side_zone_pc34(
                       explosion, 2),
                   -1, A_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.fluxcage_defers", side);
        expect_int(label, explosion ? explosion->fluxcage_defers_to_f0113 : 0,
                   1, A_EXPLOSION);
        snprintf(label, sizeof(label), "side%d.fluxcage_zone", side);
        expect_int(label, explosion ? explosion->fluxcage_field_zone : -1,
                   side == 0 ? 702 : 703, A_DEFS);
    }

    expect_int("null.projectile",
               csb_v1_viewport_d3l2_d3r2_f0115_projectile_zone_pc34(NULL, 2),
               -1, "invalid input guard");
    expect_int("null.item",
               csb_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(NULL, 2),
               -1, "invalid input guard");
    expect_int("null.creature",
               csb_v1_viewport_d3l2_d3r2_f0115_creature_zone_pc34(NULL, 0, 2),
               -1, "invalid input guard");
    expect_int("null.explosion",
               csb_v1_viewport_d3l2_d3r2_f0115_explosion_centered_zone_pc34(NULL),
               -1, "invalid input guard");
}

static void test_pixel_run_contract(void)
{
    for (size_t i = 0;
         i < csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34();
         ++i) {
        const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *s =
            csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_at_pc34(i);
        uint8_t source[32];
        uint8_t destination[32];
        char label[96];

        seed_pixels(source, destination);
        snprintf(label, sizeof(label), "pixel%zu.transparent_color", i);
        expect_int(label, s ? s->transparent_color : -1, 10, A_DEFS);
        snprintf(label, sizeof(label), "pixel%zu.c10_skip_flag", i);
        expect_int(label, s ? s->c10_transparency_skip : 0, 1, A_DEFS);
        snprintf(label, sizeof(label), "pixel%zu.derived_none", i);
        expect_int(label, s ? s->derived_bitmap_none : 0, -1, A_PROJECTILE);
        snprintf(label, sizeof(label), "pixel%zu.scaled_path_none", i);
        expect_int(label, s ? s->scaled_path_uses_cm1_derived_bitmap_none : 0,
                   1, A_PROJECTILE);
        snprintf(label, sizeof(label), "pixel%zu.blit_copied", i);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_apply_c10_blit_pc34(
                       s, source, 8, destination, 8, 8, 4,
                       s ? s->dynamic_horizontal_flip_flag : 0,
                       s ? s->dynamic_vertical_flip_flag : 0),
                   24, "synthetic 8-wide x 4-tall F0791 C10 pixel run");

        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 8; ++x) {
                const int sx = (s && s->dynamic_horizontal_flip_flag) ? 7 - x : x;
                const int sy = (s && s->dynamic_vertical_flip_flag) ? 3 - y : y;
                const int src_index = sy * 8 + sx;
                const int dst_index = y * 8 + x;
                const int source_pixel = expected_source_pixel(src_index);
                if (x == 0 && y < 2) {
                    snprintf(label, sizeof(label), "pixel%zu.sample%d%d", i, y, x);
                    expect_int(label, destination[dst_index],
                               source_pixel == 10 ? 77 : source_pixel,
                               "dynamic horizontal/vertical flip plus C10 skip");
                }
            }
        }

        snprintf(label, sizeof(label), "pixel%zu.reject_null", i);
        expect_int(label,
                   csb_v1_viewport_d3l2_d3r2_f0115_apply_c10_blit_pc34(
                       NULL, source, 8, destination, 8, 8, 4, 0, 0),
                   -1, "invalid input guard");
    }
}

static void test_source_lines_and_manifest_markers(void)
{
    for (size_t i = 0;
         i < csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34();
         ++i) {
        const CSB_V1_D3L2D3R2F0115ThingPassSpecPc34 *s =
            csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_at_pc34(i);
        char label[96];

        snprintf(label, sizeof(label), "source%zu.redmcsb_function", i);
        expect_contains(label, s ? s->redmcsb_function : 0, "F0115",
                        A_F0115);
        snprintf(label, sizeof(label), "source%zu.lines", i);
        expect_contains(label, s ? s->source_lines : 0, "ReDMCSB DUNVIEW.C",
                        A_F0115);
        snprintf(label, sizeof(label), "source%zu.coord_item_parent", i);
        expect_int(label, s ? s->coord_item_parent_record : -1, 4, A_COORD);
        snprintf(label, sizeof(label), "source%zu.coord_explosion_parent", i);
        expect_int(label, s ? s->coord_explosion_parent_record : -1, 4, A_COORD);
        snprintf(label, sizeof(label), "source%zu.coord_creature_parent", i);
        expect_int(label, s ? s->coord_creature_parent_record : -1, 4, A_COORD);
        snprintf(label, sizeof(label), "source%zu.left_then_right", i);
        expect_int(label, s ? s->f0128_left_then_right_dispatch : 0, 1,
                   A_F0128);
        snprintf(label, sizeof(label), "source%zu.route_enabled", i);
        expect_int(label, s ? s->thing_pass_route_enabled : 0, 1, A_F0115);
    }
}

int main(void)
{
    printf("probe=csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d3l2_d3r2_f0115_thing_pass_source_evidence_pc34());

    test_anchor_table_and_evidence();
    test_spec_table_contract();
    test_route_zone_math();
    test_pixel_run_contract();
    test_source_lines_and_manifest_markers();

    expect_int("assertion_count_at_least_120", g_assertions >= 120, 1, A_F0115);
    printf("assertionCount=%d failures=%d\n", g_assertions, g_failures);
    if (g_failures == 0) {
        printf("PASS final_status=1 anchor=%s assertionCount=%d\n",
               A_F0115, g_assertions);
    }
    return g_failures == 0 ? 0 : 1;
}
