#include "dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat.h"

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
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0l2;
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0r2;

    dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_init_pc34();
    d0l2 = dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(1);
    d0r2 = dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("count", (int)dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:8005/8115 D0L/D0R near side thing routes");
    expect_int("spec_count", (int)dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:8005/8115 D0L/D0R near side thing routes");
    expect_int("d0l2.present", d0l2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L");
    expect_int("d0r2.present", d0r2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R");
    expect_int("unknown.null",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(9) == NULL,
               1, "ReDMCSB DEFS.H:2596-2606 only D0 side lanes use this gate");
    expect_int("at0.d0l2",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(0) == d0l2,
               1, "ReDMCSB DUNVIEW.C:8005 left fixture first");
    expect_int("spec_at0.d0l2",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_at_pc34(0) == d0l2,
               1, "ReDMCSB DUNVIEW.C:8005 left fixture first");
    expect_int("at1.d0r2",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(1) == d0r2,
               1, "ReDMCSB DUNVIEW.C:8115 right fixture second");
    expect_int("spec_at1.d0r2",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_at_pc34(1) == d0r2,
               1, "ReDMCSB DUNVIEW.C:8115 right fixture second");
    expect_int("past.end.null",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(2) == NULL,
               1, "contract-only accessor bounds");
    expect_int("spec_past.end.null",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_at_pc34(2) == NULL,
               1, "contract-only accessor bounds");
    expect_int("spec_for_side.d0l2",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_for_side_pc34(1) == d0l2,
               1, "ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L");
    expect_int("spec_for_side.d0r2",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_for_side_pc34(2) == d0r2,
               1, "ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R");
    expect_int("spec_for_side.unknown.null",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_for_side_pc34(0) == NULL,
               1, "contract-only accessor bounds");
    expect_int("d0l2.contract_only", d0l2 ? d0l2->source_locked_contract_only : 0,
               1, "ReDMCSB DUNVIEW.C:4547 F0115 contract-only marker");
    expect_int("d0r2.contract_only", d0r2 ? d0r2->source_locked_contract_only : 0,
               1, "ReDMCSB DUNVIEW.C:4547 F0115 contract-only marker");
    expect_int("d0l2.no_asset_parity", d0l2 ? d0l2->no_real_asset_bitmap_parity : 0,
               1, "contract-only no real-asset bitmap parity");
    expect_int("d0r2.no_asset_parity", d0r2 ? d0r2->no_real_asset_bitmap_parity : 0,
               1, "contract-only no real-asset bitmap parity");
    expect_int("d0l2.no_game_data", d0l2 ? d0l2->no_game_data_load : 0,
               1, "contract-only synthetic fixture");
    expect_int("d0r2.no_game_data", d0r2 ? d0r2->no_game_data_load : 0,
               1, "contract-only synthetic fixture");
}

static void test_cell_order_decode_and_pass703_shape(void)
{
    expect_int("decode.d0l2.backright",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_decode_cell_order_pc34(
                   0x0002u, 0),
               1, "ReDMCSB DUNVIEW.C:4547-4581; DEFS.H:2660");
    expect_int("decode.d0r2.backleft",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_decode_cell_order_pc34(
                   0x0001u, 0),
               0, "ReDMCSB DUNVIEW.C:4547-4581; DEFS.H:2659");
    expect_int("decode.zero.terminator",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_decode_cell_order_pc34(
                   0x0002u, 1),
               -1, "ReDMCSB DUNVIEW.C:4561-4564 zero terminates cells");
    expect_int("decode.door.pass.marker8",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_decode_cell_order_pc34(
                   0x0018u, 0),
               -1, "ReDMCSB DUNVIEW.C:4563 door-pass marker");
    expect_int("decode.door.pass.marker9",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_decode_cell_order_pc34(
                   0x0039u, 0),
               -1, "ReDMCSB DUNVIEW.C:4563 door-pass marker");
    expect_int("decode.bad.ordinal.high",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_decode_cell_order_pc34(
                   0x0001u, 4),
               -1, "decode guard");
    expect_int("decode.bad.ordinal.negative",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_decode_cell_order_pc34(
                   0x0001u, -1),
               -1, "decode guard");
}

static void test_route_metadata(void)
{
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0l2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0r2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d0l2.side", d0l2 ? d0l2->side : -1, 1,
               "ReDMCSB DUNVIEW.C:8005 F0125 D0L route");
    expect_int("d0r2.side", d0r2 ? d0r2->side : -1, 2,
               "ReDMCSB DUNVIEW.C:8115 F0126 D0R route");
    expect_int("d0l2.route_count", d0l2 ? d0l2->route_count : -1, 1,
               "ReDMCSB DUNVIEW.C:7999-8005 single open-floor F0115 route");
    expect_int("d0r2.route_count", d0r2 ? d0r2->route_count : -1, 1,
               "ReDMCSB DUNVIEW.C:8103-8115 single open-floor F0115 route");
    expect_int("d0l2.f0115_call_count", d0l2 ? d0l2->f0115_call_count : -1, 1,
               "ReDMCSB DUNVIEW.C:8005 one F0115 call");
    expect_int("d0r2.f0115_call_count", d0r2 ? d0r2->f0115_call_count : -1, 1,
               "ReDMCSB DUNVIEW.C:8115 one F0115 call");
    expect_int("d0l2.f0108_stage_before_f0115",
               d0l2 ? d0l2->f0108_floor_stage_before_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:3940-4008 C10 floor stage before F0115 composition");
    expect_int("d0r2.f0108_stage_before_f0115",
               d0r2 ? d0r2->f0108_floor_stage_before_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:3940-4008 C10 floor stage before F0115 composition");
    expect_int("d0l2.f0108_direct_call_count",
               d0l2 ? d0l2->f0108_direct_call_count : -1, 0,
               "ReDMCSB DUNVIEW.C:7977-8005 D0L has no direct F0108");
    expect_int("d0r2.f0108_direct_call_count",
               d0r2 ? d0r2->f0108_direct_call_count : -1, 0,
               "ReDMCSB DUNVIEW.C:8081-8115 D0R has no direct F0108");
    expect_int("d0l2.no_f0107", d0l2 ? d0l2->no_f0107_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:8007-8038 wall branch returns separately");
    expect_int("d0r2.no_f0107", d0r2 ? d0r2->no_f0107_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:8117-8144 wall branch returns separately");
    expect_int("d0l2.no_f0111", d0l2 ? d0l2->no_f0111_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:8005 route has no door-front draw");
    expect_int("d0r2.no_f0111", d0r2 ? d0r2->no_f0111_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:8115 route has no door-front draw");
    expect_int("d0l2.f0112_before", d0l2 ? d0l2->f0112_before_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:8003 before 8005");
    expect_int("d0r2.f0112_before", d0r2 ? d0r2->f0112_before_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:8113 before 8115");
    expect_int("d0l2.f0172_source", d0l2 ? d0l2->f0172_square_aspect_source : 0,
               1, "ReDMCSB DUNGEON.C:2466-2523 F0172");
    expect_int("d0r2.f0172_source", d0r2 ? d0r2->f0172_square_aspect_source : 0,
               1, "ReDMCSB DUNGEON.C:2466-2523 F0172");
}

static void test_view_square_lane_order_metadata(void)
{
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0l2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0r2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d0l2.view_square", d0l2 ? d0l2->view_square_index : -1, 1,
               "ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L");
    expect_int("d0r2.view_square", d0r2 ? d0r2->view_square_index : -1, 2,
               "ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R");
    expect_int("d0l2.depth", d0l2 ? d0l2->view_depth : -1, 0,
               "ReDMCSB DUNVIEW.C:372 G2027[1]");
    expect_int("d0r2.depth", d0r2 ? d0r2->view_depth : -1, 0,
               "ReDMCSB DUNVIEW.C:372 G2027[2]");
    expect_int("d0l2.lane", d0l2 ? d0l2->view_lane : 99, -1,
               "ReDMCSB DUNVIEW.C:371 G2026[1]");
    expect_int("d0r2.lane", d0r2 ? d0r2->view_lane : 99, 1,
               "ReDMCSB DUNVIEW.C:371 G2026[2]");
    expect_int("d0l2.order", d0l2 ? (int)d0l2->f0115_cell_order : -1, 0x0002,
               "ReDMCSB DUNVIEW.C:8005; DEFS.H:2660 C0x0002");
    expect_int("d0r2.order", d0r2 ? (int)d0r2->f0115_cell_order : -1, 0x0001,
               "ReDMCSB DUNVIEW.C:8115; DEFS.H:2659 C0x0001");
    expect_int("d0l2.first_cell", d0l2 ? d0l2->f0115_first_cell : -1, 2,
               "ReDMCSB DEFS.H:2644 C02_VIEW_CELL_BACK_RIGHT");
    expect_int("d0r2.first_cell", d0r2 ? d0r2->f0115_first_cell : -1, 3,
               "ReDMCSB DEFS.H:2645 C03_VIEW_CELL_BACK_LEFT");
    expect_int("d0l2.cell_count", d0l2 ? d0l2->f0115_cell_count : -1, 1,
               "ReDMCSB DUNVIEW.C:4561-4564 one nonzero nibble");
    expect_int("d0r2.cell_count", d0r2 ? d0r2->f0115_cell_count : -1, 1,
               "ReDMCSB DUNVIEW.C:4561-4564 one nonzero nibble");
    expect_int("draw_order.d0l_before_d0r",
               (d0l2 ? d0l2->view_square_index : 9) < (d0r2 ? d0r2->view_square_index : -1),
               1, "ReDMCSB DUNVIEW.C:8536-8541 F0128 D0L then D0R");
}

static void test_zone_bindings_and_disabled_item_projectile_rows(void)
{
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0l2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0r2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d0l2.item_row.disabled", d0l2 ? d0l2->item_projectile_row : 0, -1,
               "ReDMCSB DUNVIEW.C:373 G2028[1]");
    expect_int("d0r2.item_row.disabled", d0r2 ? d0r2->item_projectile_row : 0, -1,
               "ReDMCSB DUNVIEW.C:373 G2028[2]");
    expect_int("d0l2.item.zone.disabled",
               dm1_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(d0l2, 2),
               -1, "ReDMCSB DUNVIEW.C:4923 L2476_i_ >= 0 gate");
    expect_int("d0r2.item.zone.disabled",
               dm1_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(d0r2, 3),
               -1, "ReDMCSB DUNVIEW.C:4923 L2476_i_ >= 0 gate");
    expect_int("d0l2.projectile.zone.disabled",
               dm1_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(d0l2, 2),
               -1, "ReDMCSB DUNVIEW.C:5668-5671 negative G2028 skips");
    expect_int("d0r2.projectile.zone.disabled",
               dm1_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(d0r2, 3),
               -1, "ReDMCSB DUNVIEW.C:5668-5671 negative G2028 skips");
    expect_int("d0l2.creature_row", d0l2 ? d0l2->creature_row : -1, 11,
               "ReDMCSB DUNVIEW.C:375 G2033[1]");
    expect_int("d0r2.creature_row", d0r2 ? d0r2->creature_row : -1, 12,
               "ReDMCSB DUNVIEW.C:375 G2033[2]");
    expect_int("d0l2.creature.zone.backright",
               dm1_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(d0l2, 2),
               (0x8000 | (3200 + 11 * 5 + 2)),
               "ReDMCSB DUNVIEW.C:5295/5615-5617 D0L BACKRIGHT");
    expect_int("d0r2.creature.zone.backleft",
               dm1_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(d0r2, 3),
               (0x8000 | (3200 + 12 * 5 + 3)),
               "ReDMCSB DUNVIEW.C:5295/5615-5617 D0R BACKLEFT");
    expect_int("d0l2.creature.bad_cell",
               dm1_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(d0l2, 3),
               -1, "ReDMCSB DUNVIEW.C:5295 rejects D0L non-BACKRIGHT");
    expect_int("d0r2.creature.bad_cell",
               dm1_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(d0r2, 2),
               -1, "ReDMCSB DUNVIEW.C:5295 rejects D0R non-BACKLEFT");
    expect_int("null.item",
               dm1_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(NULL, 0),
               -1, "invalid input guard");
    expect_int("null.creature",
               dm1_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(NULL, 2),
               -1, "invalid input guard");
}

static void test_explosion_and_field_metadata(void)
{
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0l2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0r2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d0l2.explosion_row", d0l2 ? d0l2->explosion_row : -1, 15,
               "ReDMCSB DUNVIEW.C:376 G2034[1]");
    expect_int("d0r2.explosion_row", d0r2 ? d0r2->explosion_row : -1, 16,
               "ReDMCSB DUNVIEW.C:376 G2034[2]");
    expect_int("d0l2.centered_explosion.zone",
               dm1_v1_viewport_d0l2_d0r2_f0115_centered_explosion_zone_pc34(d0l2),
               3014 + 15, "ReDMCSB DUNVIEW.C:6107 C3014 + G2034 row");
    expect_int("d0r2.centered_explosion.zone",
               dm1_v1_viewport_d0l2_d0r2_f0115_centered_explosion_zone_pc34(d0r2),
               3014 + 16, "ReDMCSB DUNVIEW.C:6107 C3014 + G2034 row");
    expect_int("d0l2.side_explosion.front_left",
               dm1_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(d0l2, 0),
               3031 + 15 * 2, "ReDMCSB DUNVIEW.C:6110-6122 C3031 + row*2 + cell");
    expect_int("d0r2.side_explosion.front_right",
               dm1_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(d0r2, 1),
               3031 + 16 * 2 + 1, "ReDMCSB DUNVIEW.C:6110-6122 C3031 + row*2 + cell");
    expect_int("d0l2.side_explosion.bad_cell",
               dm1_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(d0l2, 2),
               -1, "ReDMCSB DUNVIEW.C:6110-6114 chooses only front explosion cells");
    expect_int("null.centered_explosion",
               dm1_v1_viewport_d0l2_d0r2_f0115_centered_explosion_zone_pc34(NULL),
               -1, "invalid input guard");
    expect_int("d0l2.field_aspect", d0l2 ? d0l2->field_aspect_index : -1, 14,
               "ReDMCSB DUNVIEW.C:377 G2035[1]");
    expect_int("d0r2.field_aspect", d0r2 ? d0r2->field_aspect_index : -1, 15,
               "ReDMCSB DUNVIEW.C:377 G2035[2]");
    expect_int("d0l2.fluxcage.zone.metadata", d0l2 ? d0l2->fluxcage_field_zone : -1,
               716, "ReDMCSB DUNVIEW.C:6199-6219 C702 + G2035[1]");
    expect_int("d0r2.fluxcage.zone.metadata", d0r2 ? d0r2->fluxcage_field_zone : -1,
               717, "ReDMCSB DUNVIEW.C:6199-6219 C702 + G2035[2]");
    expect_int("d0l2.fluxcage.after_explosions",
               d0l2 ? d0l2->fluxcage_field_after_explosions : 0,
               1, "ReDMCSB DUNVIEW.C:6199-6219 after explosion loop");
    expect_int("d0r2.fluxcage.after_explosions",
               d0r2 ? d0r2->fluxcage_field_after_explosions : 0,
               1, "ReDMCSB DUNVIEW.C:6199-6219 after explosion loop");
    expect_int("d0l2.fluxcage.door_pass1_guard",
               d0l2 ? d0l2->fluxcage_suppressed_on_door_pass1 : 0,
               1, "ReDMCSB DUNVIEW.C:6199-6203 door pass 1 guard");
    expect_int("d0r2.fluxcage.endgame_guard",
               d0r2 ? d0r2->fluxcage_suppressed_during_endgame : 0,
               1, "ReDMCSB DUNVIEW.C:6199-6203 endgame guard");
    expect_int("d0l2.fluxcage.zone",
               dm1_v1_viewport_d0l2_d0r2_f0115_fluxcage_field_zone_pc34(d0l2, 0, 0),
               716, "ReDMCSB DUNVIEW.C:6219 C702 + fieldAspect 14");
    expect_int("d0r2.fluxcage.zone",
               dm1_v1_viewport_d0l2_d0r2_f0115_fluxcage_field_zone_pc34(d0r2, 0, 0),
               717, "ReDMCSB DUNVIEW.C:6219 C702 + fieldAspect 15");
    expect_int("d0l2.fluxcage.door_pass1.suppressed",
               dm1_v1_viewport_d0l2_d0r2_f0115_fluxcage_field_zone_pc34(d0l2, 1, 0),
               -1, "ReDMCSB DUNVIEW.C:6199-6203 L0175 pass 1 guard");
    expect_int("d0r2.fluxcage.endgame.suppressed",
               dm1_v1_viewport_d0l2_d0r2_f0115_fluxcage_field_zone_pc34(d0r2, 0, 1),
               -1, "ReDMCSB DUNVIEW.C:6199-6203 endgame guard");
    expect_int("null.fluxcage",
               dm1_v1_viewport_d0l2_d0r2_f0115_fluxcage_field_zone_pc34(NULL, 0, 0),
               -1, "invalid input guard");
    expect_int("d0l2.wall_zone", d0l2 ? d0l2->wall_zone : -1, 716,
               "ReDMCSB DUNVIEW.C:8059 C716_ZONE_WALL_D0L");
    expect_int("d0r2.wall_zone", d0r2 ? d0r2->wall_zone : -1, 717,
               "ReDMCSB DUNVIEW.C:8159 C717_ZONE_WALL_D0R");
    expect_int("d0l2.no_door_zone", d0l2 ? d0l2->door_zone : 0, -1,
               "ReDMCSB DEFS.H:4250-4260 has no D0 door-front zone");
    expect_int("d0r2.no_door_zone", d0r2 ? d0r2->door_zone : 0, -1,
               "ReDMCSB DEFS.H:4250-4260 has no D0 door-front zone");
    expect_int("d0l2.ceiling_zone", d0l2 ? d0l2->ceiling_pit_zone : -1, 870,
               "ReDMCSB DUNVIEW.C:8003 C870_ZONE_CEILING_PIT_D0L");
    expect_int("d0r2.ceiling_zone", d0r2 ? d0r2->ceiling_pit_zone : -1, 872,
               "ReDMCSB DUNVIEW.C:8113 C872_ZONE_CEILING_PIT_D0R");
    expect_int("d0l2.field_after_f0115", d0l2 ? d0l2->field_draw_after_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:8050-8059 after 8005");
    expect_int("d0r2.field_after_f0115", d0r2 ? d0r2->field_draw_after_f0115 : 0, 1,
               "ReDMCSB DUNVIEW.C:8150-8159 after 8115");
}

static void test_f0108_f0115_composition_and_mutation_guard(void)
{
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0l2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_for_side_pc34(1);
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0r2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_spec_for_side_pc34(2);
    DM1_V1_D0L2D0R2F0115ThingPassTracePc34 trace;

    expect_int("blend.transparent",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_blend_pixel_pc34(77, 10),
               77, "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("blend.opaque",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_blend_pixel_pc34(77, 42),
               42, "ReDMCSB DUNVIEW.C:3988-4004 C10 transparent blit");

    expect_int("compose.d0l2.ok",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_compose_pixel_pc34(
                   d0l2, 3, 4, 5, &trace),
               0, "ReDMCSB DUNVIEW.C:3940-4008 F0108 then 4547-4581 F0115");
    expect_int("compose.d0l2.trace.ok", trace.ok, 1,
               "ReDMCSB DUNVIEW.C:4547-4581 F0115 composition");
    expect_int("compose.d0l2.f0108_calls", trace.f0108_calls, 1,
               "ReDMCSB DUNVIEW.C:3940-4008 synthetic floor stage");
    expect_int("compose.d0l2.f0115_calls", trace.f0115_calls, 1,
               "ReDMCSB DUNVIEW.C:8005 one F0115 call");
    expect_int("compose.d0l2.after_f0108", trace.after_f0108, 4,
               "ReDMCSB DUNVIEW.C:3988-4004 C10 transparent floor blit");
    expect_int("compose.d0l2.after_f0115", trace.after_f0115, 5,
               "ReDMCSB DUNVIEW.C:4547-4581 thing pass overlays after floor");
    expect_int("compose.d0l2.floor_transparent", trace.f0108_transparent, 0,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("compose.d0l2.thing_transparent", trace.f0115_transparent, 0,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");

    expect_int("compose.transparent.floor.ok",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_compose_pixel_pc34(
                   d0r2, 9, 10, 6, &trace),
               0, "ReDMCSB DUNVIEW.C:3940-4008 F0108 C10 transparency");
    expect_int("compose.transparent.floor.after_f0108", trace.after_f0108, 9,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("compose.transparent.floor.after_f0115", trace.after_f0115, 6,
               "ReDMCSB DUNVIEW.C:4547-4581 F0115 follows floor stage");
    expect_int("compose.transparent.floor.flag", trace.f0108_transparent, 1,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");

    expect_int("compose.transparent.thing.ok",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_compose_pixel_pc34(
                   d0r2, 9, 6, 10, &trace),
               0, "ReDMCSB DUNVIEW.C:4547-4581 F0115 C10 transparency");
    expect_int("compose.transparent.thing.after_f0115", trace.after_f0115, 6,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("compose.transparent.thing.flag", trace.f0115_transparent, 1,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");

    expect_int("compose.null.spec",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_compose_pixel_pc34(
                   NULL, 0, 1, 2, &trace),
               -1, "invalid input guard");
    expect_int("compose.null.trace",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_compose_pixel_pc34(
                   d0l2, 0, 1, 2, NULL),
               -1, "invalid input guard");
    expect_int("mutation.d0l2",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_is_draw_mutating_pc34(d0l2),
               0, "ReDMCSB DUNGEON.C:1769-1838 F0163 not called by draw");
    expect_int("mutation.d0r2",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_is_draw_mutating_pc34(d0r2),
               0, "ReDMCSB DUNGEON.C:1840-1905 F0164 not called by draw");
    expect_int("mutation.null",
               dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_is_draw_mutating_pc34(NULL),
               -1, "invalid input guard");
    expect_int("d0l2.f0163_guard", d0l2 ? d0l2->f0163_not_called_by_draw : 0, 1,
               "ReDMCSB DUNGEON.C:1769-1838 F0163");
    expect_int("d0r2.f0164_guard", d0r2 ? d0r2->f0164_not_called_by_draw : 0, 1,
               "ReDMCSB DUNGEON.C:1840-1905 F0164");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_source_evidence_pc34();
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0l2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D0L2D0R2F0115ThingPassPc34 *d0r2 =
        dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(2);

    expect_contains("evidence.contract_only", e, "source_locked_contract_only=1",
                    "source evidence");
    expect_contains("evidence.no_asset_parity", e, "no_real_asset_bitmap_parity=1",
                    "source evidence");
    expect_contains("evidence.f0125", e, "DUNVIEW.C:7960-8062 F0125",
                    "DUNVIEW.C D0L function");
    expect_contains("evidence.f0126", e, "DUNVIEW.C:8064-8162 F0126",
                    "DUNVIEW.C D0R function");
    expect_contains("evidence.f0128", e, "DUNVIEW.C:8536-8541 F0128",
                    "DUNVIEW.C D0L/D0R caller");
    expect_contains("evidence.f0108", e, "DUNVIEW.C:3940-4008 F0108",
                    "DUNVIEW.C floor-ornament C10 transparent stage");
    expect_contains("evidence.f0115", e, "DUNVIEW.C:4547-4581 F0115",
                    "DUNVIEW.C thing pass");
    expect_contains("evidence.items_disabled", e, "4923",
                    "DUNVIEW.C item visibility gate");
    expect_contains("evidence.projectiles_disabled", e, "5668-5671",
                    "DUNVIEW.C projectile visibility gate");
    expect_contains("evidence.creature_gate", e, "5295",
                    "DUNVIEW.C quarter creature gate");
    expect_contains("evidence.creature_zone", e, "5615-5617",
                    "DUNVIEW.C creature zone");
    expect_contains("evidence.explosion_zone", e, "6107/6122",
                    "DUNVIEW.C explosion zones");
    expect_contains("evidence.fluxcage_defer", e, "6006-6015",
                    "DUNVIEW.C fluxcage defer anchor");
    expect_contains("evidence.fluxcage_field", e, "6199-6219",
                    "DUNVIEW.C fluxcage field anchor");
    expect_contains("evidence.fluxcage_zone", e, "C702 + G2035",
                    "DUNVIEW.C fluxcage field-zone mapping");
    expect_contains("evidence.defs", e, "DEFS.H:2642-2660",
                    "DEFS.H cell order anchors");
    expect_contains("evidence.defs_c10", e, "DEFS.H:2088 C10_COLOR_FLESH",
                    "DEFS.H C10 transparency anchor");
    expect_contains("evidence.no_d0_door_zone", e, "DEFS.H:4250-4260",
                    "DEFS.H door-zone absence anchor");
    expect_contains("evidence.f0163", e, "DUNGEON.C:1769-1838 F0163",
                    "DUNGEON.C mutating link anchor");
    expect_contains("evidence.f0164", e, "1840-1905 F0164",
                    "DUNGEON.C mutating unlink anchor");
    expect_contains("evidence.f0172", e, "DUNGEON.C:2466-2523 F0172",
                    "DUNGEON.C square-aspect anchor");
    expect_contains("d0l2.anchor", d0l2 ? d0l2->redmcsb_dispatch_anchor : NULL,
                    "8003/8005/8059", "fixture source anchor");
    expect_contains("d0r2.anchor", d0r2 ? d0r2->redmcsb_dispatch_anchor : NULL,
                    "8113/8115/8159", "fixture source anchor");
    expect_contains("d0l2.f0115.anchor", d0l2 ? d0l2->redmcsb_f0115_anchor : NULL,
                    "6199-6219", "fixture F0115 anchor");
    expect_contains("d0r2.f0115.anchor", d0r2 ? d0r2->redmcsb_f0115_anchor : NULL,
                    "5668-5671", "fixture F0115 anchor");
    expect_contains("d0l2.dungeon.anchor", d0l2 ? d0l2->redmcsb_dungeon_anchor : NULL,
                    "F0172", "fixture DUNGEON.C anchor");
    expect_contains("d0r2.defs.anchor", d0r2 ? d0r2->redmcsb_defs_anchor : NULL,
                    "4250-4260", "fixture DEFS.H anchor");
}

int main(void)
{
    test_accessors_and_contract_markers();
    test_cell_order_decode_and_pass703_shape();
    test_route_metadata();
    test_view_square_lane_order_metadata();
    test_zone_bindings_and_disabled_item_projectile_rows();
    test_explosion_and_field_metadata();
    test_f0108_f0115_composition_and_mutation_guard();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
