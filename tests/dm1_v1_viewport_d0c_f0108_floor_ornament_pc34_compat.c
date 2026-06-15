/*
 * ReDMCSB anchors: DUNVIEW.C F0108:3940-4011, F0107:3502-3938,
 * F0098:2962-3002, F0115:4547-4581/5180-5188/5211-5214/5668-5671;
 * DEFS.H:2088,2596-2611,2668-2677,2698-2702,4045-4046.
 */
#include "firestaff/dm1/v1/viewport/d0c_f0108_floor_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int test_failures;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++test_failures;
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n", id,
               needle ? needle : "(null)", anchor);
        ++test_failures;
    }
}

static void test_d0c_f0108_floor_ornament_kappetaal_aspect_variant(void)
{
    const DM1_V1_D0CF0108FloorOrnamentKappetaalVariantPc34 *k =
        dm1_v1_viewport_d0c_f0108_floor_ornament_kappetaal_variant_pc34_compat();

    expect_int("kappetaal.present", k != NULL, 1,
               "contract object must be present");
    if (!k) return;

    /* ReDMCSB DUNGEON.C F0172:2628-2678 can leave a sensor-provided
     * M558 floor-ornament ordinal on an open pit. DUNVIEW.C F0127:8274-8296
     * routes D0C open pits to the pit edge graphics and F0115, with no
     * F0108 call and no G0206/G0207/G0208 ornament/button route. */
    expect_int("kappetaal.contract_only", k->source_locked_contract_only, 1,
               "contract-only gate");
    expect_int("kappetaal.no_dos_parity_claim", k->no_original_dos_parity_claim, 1,
               "do not claim original DOS parity");
    expect_int("kappetaal.square_type", k->square_element_pit, 2,
               "DEFS.H:1009 C02_ELEMENT_PIT");
    expect_int("kappetaal.open_mask", k->pit_open_mask, 0x08,
               "DEFS.H:1027 MASK0x0008_PIT_OPEN");
    expect_int("kappetaal.invisible_mask", k->pit_invisible_mask, 0x04,
               "DEFS.H:1026 MASK0x0004_PIT_INVISIBLE");
    expect_int("kappetaal.visible_slot", k->pit_visible_aspect_slot_pc34, 3,
               "DEFS.H:2554 M554_PIT_OR_TELEPORTER_VISIBLE");
    expect_int("kappetaal.floor_orn_slot", k->floor_ornament_aspect_slot_pc34, 5,
               "DEFS.H:2558 M558_FLOOR_ORNAMENT_ORDINAL");
    expect_int("kappetaal.sensor_ordinal", k->sensor_floor_ornament_ordinal, 4,
               "DUNGEON.C F0172:2673-2678 sensor Remote.OrnamentOrdinal");
    expect_int("kappetaal.is_kappetaal", k->is_kappetaal_pit_boundary_variant, 1,
               "D0C front-edge pit boundary variant");
    expect_int("kappetaal.not_regular_floor", k->is_regular_floor_ornament_variant, 0,
               "not regular F0108 floor-ornament blit");
    expect_int("kappetaal.no_f0108", k->f0108_floor_ornament_calls, 0,
               "DUNVIEW.C F0127:8274-8296 no F0108 call");
    expect_int("kappetaal.no_f0108_zone", k->f0108_floor_ornament_zone, -1,
               "C1500 floor-ornament zone unused");
    expect_int("kappetaal.no_g0206", k->g0206_floor_ornament_coordinate_route, 0,
               "DUNVIEW.C G0206:1167-1194 not used by D0C pit");
    expect_int("kappetaal.no_g0207", k->g0207_door_ornament_route, 0,
               "DUNVIEW.C G0207:1196-1208 not used by pit edge");
    expect_int("kappetaal.no_g0208", k->g0208_door_button_route, 0,
               "DUNVIEW.C G0208:1210-1216 not used by pit edge");
    expect_int("kappetaal.view_square", k->d0c_view_square, 0,
               "DEFS.H:2596 M609_VIEW_SQUARE_D0C");
    expect_int("kappetaal.cell_order", k->d0c_cell_order, 0x0021,
               "DEFS.H:2662 C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT");
    expect_int("kappetaal.floor_graphic", k->floor_pit_graphic_open_pc34, 57,
               "DEFS.H:2340 M761_GRAPHIC_FLOOR_PIT_D0C");
    expect_int("kappetaal.floor_invisible", k->floor_pit_graphic_invisible_pc34, 63,
               "DEFS.H:2346 M767_GRAPHIC_FLOOR_PIT_INVISIBLE_D0C");
    expect_int("kappetaal.floor_zone", k->floor_pit_zone_pc34, 862,
               "DEFS.H:4209 C862_ZONE_FLOORPIT_D0C");
    expect_int("kappetaal.ceiling_graphic", k->ceiling_pit_graphic_pc34, 69,
               "DEFS.H:2253 C069_GRAPHIC_CEILING_PIT_D0C");
    expect_int("kappetaal.ceiling_zone", k->ceiling_pit_zone_pc34, 871,
               "DEFS.H:4218 C871_ZONE_CEILING_PIT_D0C");
    expect_int("kappetaal.viewport_w", k->viewport_width, 224,
               "224x136 viewport inside 320x200 screen");
    expect_int("kappetaal.viewport_h", k->viewport_height, 136,
               "224x136 viewport inside 320x200 screen");
    expect_int("kappetaal.screen_w", k->screen_width, 320,
               "DM1 V1 screen width");
    expect_int("kappetaal.screen_h", k->screen_height, 200,
               "DM1 V1 screen height");
    expect_int("kappetaal.dst_x", k->open_pit_dst_x, 27,
               "GRAPHICS.DAT D0C pit front edge");
    expect_int("kappetaal.dst_y", k->open_pit_dst_y, 127,
               "GRAPHICS.DAT D0C pit front edge");
    expect_int("kappetaal.dst_w", k->open_pit_dst_w, 170,
               "GRAPHICS.DAT D0C pit front edge");
    expect_int("kappetaal.dst_h", k->open_pit_dst_h, 9,
               "GRAPHICS.DAT D0C pit front edge");
    expect_int("kappetaal.invisible_dst_x", k->invisible_pit_dst_x, 25,
               "GRAPHICS.DAT invisible D0C pit edge");
    expect_int("kappetaal.invisible_dst_y", k->invisible_pit_dst_y, 127,
               "GRAPHICS.DAT invisible D0C pit edge");
    expect_int("kappetaal.invisible_dst_w", k->invisible_pit_dst_w, 174,
               "GRAPHICS.DAT invisible D0C pit edge");
    expect_int("kappetaal.invisible_dst_h", k->invisible_pit_dst_h, 9,
               "GRAPHICS.DAT invisible D0C pit edge");
    expect_int("kappetaal.inside_viewport", k->geometry_inside_viewport, 1,
               "27+170<=224 and 127+9<=136");
    expect_int("kappetaal.asset_route", k->graphics_dat_asset_route, 1,
               "real GRAPHICS.DAT floor-pit asset route");
    expect_contains("kappetaal.anchor.f0108", k->redmcsb_f0108_anchor,
                    "F0108", "DUNVIEW.C F0108:3940-4011");
    expect_contains("kappetaal.anchor.f0172", k->redmcsb_f0172_anchor,
                    "F0172:2628-2678", "DUNGEON.C open-pit aspect");
    expect_contains("kappetaal.anchor.f0127", k->redmcsb_f0127_anchor,
                    "F0127:8274-8296", "D0C pit route");
    expect_contains("kappetaal.anchor.coords", k->redmcsb_coordinate_anchor,
                    "G0206/G0207/G0208", "coordinate-route negative guard");
    expect_contains("kappetaal.anchor.asset", k->firestaff_graphics_dat_anchor,
                    "m11_game_view.c:12829-12830", "GRAPHICS.DAT geometry anchor");
}

int main(void)
{
    DM1_V1_D0CF0108FloorOrnamentSelfTestResultPc34 result;
    const int ok =
        run_dm1_v1_viewport_d0c_f0108_floor_ornament_self_test_pc34_compat(&result);

    test_d0c_f0108_floor_ornament_kappetaal_aspect_variant();

    if (!ok || result.failures != 0 ||
        result.deterministic_hash != DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34 ||
        test_failures != 0) {
        printf("FAIL test_dm1_v1_viewport_d0c_f0108_floor_ornament_pc34_compat "
               "assertions=%d failures=%d kappetaal_failures=%d floor_writes=%d thing_passes=%d "
               "keepouts=%d mutation_rejections=%d hash=0x%08x expected=0x%08x\n",
               result.assertions,
               result.failures + (result.deterministic_hash !=
                   DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34 ? 1 : 0),
               test_failures,
               result.floor_writes,
               result.thing_pass_calls,
               result.keepout_preservations,
               result.mutation_rejections,
               result.deterministic_hash,
               (uint32_t)DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d0c_f0108_floor_ornament_pc34_compat "
           "assertions=%d failures=0 kappetaal_failures=0 floor_writes=%d thing_passes=%d "
           "keepouts=%d mutation_rejections=%d hash=0x%08x\n",
           result.assertions,
           result.floor_writes,
           result.thing_pass_calls,
           result.keepout_preservations,
           result.mutation_rejections,
           result.deterministic_hash);
    return 0;
}
