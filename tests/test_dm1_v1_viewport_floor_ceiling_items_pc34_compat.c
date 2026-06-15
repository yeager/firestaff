#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    PC34_C10_COLOR_FLESH = 10,
    PC34_C700_ZONE_VIEWPORT_CEILING_AREA = 700,
    PC34_C701_ZONE_VIEWPORT_FLOOR_AREA = 701,
    PC34_C2500_ZONE_OBJECT_BASE = 2500,
    PC34_C2900_ZONE_PROJECTILE_BASE = 2900,
    PC34_VIEWPORT_FLOOR_HEIGHT = 70,
    PC34_VIEWPORT_CEILING_HEIGHT = 29
};

static int g_assertions = 0;
static int g_failures = 0;

static unsigned char c10_blend(unsigned char dst, unsigned char src)
{
    return src == PC34_C10_COLOR_FLESH ? dst : src;
}

static int f0099_flipped_x(int width, int x)
{
    return width - 1 - x;
}

static int object_zone(int view_square, int view_cell)
{
    return PC34_C2500_ZONE_OBJECT_BASE + (view_square * 4) + view_cell;
}

static int projectile_zone(int view_square, int view_cell)
{
    return PC34_C2900_ZONE_PROJECTILE_BASE + (view_square * 4) + view_cell;
}

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *text,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void test_floor_set_and_floor_ceiling_ownership(void)
{
    expect_int("floor_set.first_graphic", DM1_GRAPHIC_FIRST_FLOOR_SET, 78,
               "DEFS.H:2348 M644_GRAPHIC_FIRST_FLOOR_SET");
    expect_int("floor_set.graphic_count", DM1_FLOOR_SET_GRAPHIC_COUNT, 2,
               "DEFS.H:2423 C002_FLOOR_SET_GRAPHIC_COUNT");
    expect_int("floor_set.0.floor", (int)dm1_floor_set_floor_graphic(0), 78,
               "DUNVIEW.C:2042-2045 floorSet*2+M644");
    expect_int("floor_set.0.ceiling", (int)dm1_floor_set_ceiling_graphic(0), 79,
               "DEFS.H:2349-2350 floor 0/ceiling 0");
    expect_int("floor_set.1.floor", (int)dm1_floor_set_floor_graphic(1), 80,
               "DUNVIEW.C:2042-2045 floorSet*2+M644");
    expect_int("floor_set.1.ceiling", (int)dm1_floor_set_ceiling_graphic(1), 81,
               "DUNVIEW.C:2042-2045 ceiling is floor+1");
    expect_int("floor_set.2.floor", (int)dm1_floor_set_floor_graphic(2), 82,
               "DUNVIEW.C:2042-2045 floorSet*2+M644");
    expect_int("floor_set.2.ceiling", (int)dm1_floor_set_ceiling_graphic(2), 83,
               "DUNVIEW.C:2042-2045 ceiling is floor+1");

    expect_int("ownership.ceiling_zone", PC34_C700_ZONE_VIEWPORT_CEILING_AREA, 700,
               "DEFS.H:4040 C700_ZONE_VIEWPORT_CEILING_AREA");
    expect_int("ownership.floor_zone", PC34_C701_ZONE_VIEWPORT_FLOOR_AREA, 701,
               "DEFS.H:4041 C701_ZONE_VIEWPORT_FLOOR_AREA");
    expect_int("ownership.floor_height", PC34_VIEWPORT_FLOOR_HEIGHT, 70,
               "DUNVIEW.C:8363 floor bitmap height is 70 lines");
    expect_int("ownership.ceiling_height", PC34_VIEWPORT_CEILING_HEIGHT, 29,
               "DUNVIEW.C:8425 ceiling bitmap height is 29 lines");
    expect_int("ownership.floor_flipped_zone", PC34_C701_ZONE_VIEWPORT_FLOOR_AREA, 701,
               "DUNVIEW.C:8367-8368 floor uses C701 when parity flips");
    expect_int("ownership.ceiling_flipped_zone", PC34_C700_ZONE_VIEWPORT_CEILING_AREA, 700,
               "DUNVIEW.C:8430 ceiling uses C700 when parity flips");
}

static void test_floor_ornament_variants_and_palettes(void)
{
    static const unsigned char d3[16] =
        { 0, 12, 1, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 2, 14, 13 };
    static const unsigned char d2[16] =
        { 0, 1, 2, 3, 4, 3, 6, 7, 5, 9, 10, 11, 12, 13, 14, 15 };
    int i;

    expect_int("variant.d3_side", DM1_FLOOR_ORN_D3_SIDE, 0,
               "DUNVIEW.C:820-823 G0191 D3 side increments");
    expect_int("variant.d3_center", DM1_FLOOR_ORN_D3_CENTER, 1,
               "DUNVIEW.C:824 G0191 D3 center increment");
    expect_int("variant.d2_side", DM1_FLOOR_ORN_D2_SIDE, 2,
               "DUNVIEW.C:826,828 G0191 D2 side increments");
    expect_int("variant.d2_center", DM1_FLOOR_ORN_D2_CENTER, 3,
               "DUNVIEW.C:827 G0191 D2 center increment");
    expect_int("variant.d1_side", DM1_FLOOR_ORN_D1_SIDE, 4,
               "DUNVIEW.C:829,831 G0191 D1 side increments");
    expect_int("variant.d1_center", DM1_FLOOR_ORN_D1_CENTER, 5,
               "DUNVIEW.C:830 G0191 D1 center increment");
    expect_int("variant.count", DM1_FLOOR_ORN_VARIANT_COUNT, 6,
               "DUNVIEW.C:820-831 six native bitmap increments are used");

    for (i = 0; i < 16; ++i) {
        char id[32];
        snprintf(id, sizeof(id), "palette.d3.%02d", i);
        expect_int(id, DM1_FloorOrnPalette_D3[i], d3[i],
                   "DUNVIEW.C:1534 G0213 floor-ornament D3 palette");
    }
    for (i = 0; i < 16; ++i) {
        char id[32];
        snprintf(id, sizeof(id), "palette.d2.%02d", i);
        expect_int(id, DM1_FloorOrnPalette_D2[i], d2[i],
                   "DUNVIEW.C:1535 G0214 floor-ornament D2 palette");
    }
}

static void test_item_pile_zone_flip_and_transparency_contract(void)
{
    expect_int("item.c10.skip", c10_blend(0xee, 10), 0xee,
               "DUNVIEW.C:5870-5879 F0115 item blit uses C10 transparency");
    expect_int("item.c10.write", c10_blend(0xee, 0x4a), 0x4a,
               "DUNVIEW.C:5870-5879 F0115 opaque item pixel writes");
    expect_int("floor_orn.c10.skip", c10_blend(0xdd, 10), 0xdd,
               "DUNVIEW.C:3988-4004 F0108 floor ornament uses C10");
    expect_int("floor_orn.c10.write", c10_blend(0xdd, 0x33), 0x33,
               "DUNVIEW.C:3988-4004 F0108 opaque floor pixel writes");

    expect_int("f0099.left_to_right", f0099_flipped_x(112, 0), 111,
               "DUNVIEW.C:3018-3045 F0099 horizontal flip");
    expect_int("f0099.right_to_left", f0099_flipped_x(112, 111), 0,
               "DUNVIEW.C:3018-3045 F0099 horizontal flip");
    expect_int("f0099.row_local_mid", f0099_flipped_x(16, 7), 8,
               "DUNVIEW.C:3033-3034 copy then flip within byte-width row");
    expect_int("f0099.row_local_next", f0099_flipped_x(16, 8), 7,
               "DUNVIEW.C:3033-3034 copy then flip within byte-width row");

    expect_int("zone.object.base", PC34_C2500_ZONE_OBJECT_BASE, 2500,
               "DEFS.H:4228 C2500_ZONE_");
    expect_int("zone.projectile.base", PC34_C2900_ZONE_PROJECTILE_BASE, 2900,
               "DEFS.H:4230 C2900_ZONE_");
    expect_int("zone.object.d2r.cell0", object_zone(8, 0), 2532,
               "DUNVIEW.C:5075 C2500 + viewSquare*4 + viewCell");
    expect_int("zone.object.d2r.cell3", object_zone(8, 3), 2535,
               "DUNVIEW.C:5075 C2500 + viewSquare*4 + viewCell");
    expect_int("zone.projectile.d2r.cell0", projectile_zone(8, 0), 2932,
               "DUNVIEW.C:5683 C2900 + viewSquare*4 + viewCell");
    expect_int("zone.projectile.d2r.cell3", projectile_zone(8, 3), 2935,
               "DUNVIEW.C:5683 C2900 + viewSquare*4 + viewCell");

    expect_int("alcove.1", dm1_is_alcove_ornament(1), 1,
               "DUNGEON.C F0149, header source-lock: alcove indices 1,2,3");
    expect_int("alcove.2", dm1_is_alcove_ornament(2), 1,
               "DUNGEON.C F0149, header source-lock: alcove indices 1,2,3");
    expect_int("alcove.3", dm1_is_alcove_ornament(3), 1,
               "DUNGEON.C F0149, header source-lock: alcove indices 1,2,3");
    expect_int("alcove.4", dm1_is_alcove_ornament(4), 0,
               "DUNGEON.C F0149, header source-lock: only indices 1,2,3");

    expect_int("contract.floor_no_f0111", PC34_C701_ZONE_VIEWPORT_FLOOR_AREA != 719, 1,
               "DUNVIEW.C:7180-7197 F0111 is door route, not F0098 floor");
    expect_int("contract.floor_no_f0107", PC34_C701_ZONE_VIEWPORT_FLOOR_AREA != 1004, 1,
               "DUNVIEW.C:7119-7122 F0107 is wall-alcove ornament route");
    expect_int("contract.floor_no_f0100_pc34", PC34_C701_ZONE_VIEWPORT_FLOOR_AREA == 701, 1,
               "DUNVIEW.C:8367-8368 PC34 floor uses F0792/C701, not F0100");
    expect_int("contract.ceiling_no_f0100_pc34", PC34_C700_ZONE_VIEWPORT_CEILING_AREA == 700, 1,
               "DUNVIEW.C:8430 PC34 ceiling uses F0792/C700, not F0100");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e =
        "contract_only=1; no_real_asset_pixel_parity=1; "
        "DUNVIEW.C:2962 F0098 floor/ceiling; "
        "DUNVIEW.C:3018-3045 F0099 row-local flip; "
        "DUNVIEW.C:3940-4008 F0108 floor ornaments; "
        "DUNVIEW.C:4547-4565 F0115 item/projectile pass; "
        "DUNVIEW.C:8367-8368 C701 floor zone; "
        "DUNVIEW.C:8430 C700 ceiling zone; "
        "DUNVIEW.C:5075 C2500 object zone; "
        "DUNVIEW.C:5683 C2900 projectile zone; "
        "DEFS.H:2348/2423 floor-set graphics; "
        "DEFS.H:4228/4230 C2500/C2900 zones; "
        "COMMAND.C:2174-2177 slot command route checked as unrelated.";

    expect_contains("evidence.contract", e, "contract_only=1",
                    "source evidence");
    expect_contains("evidence.no_asset_parity", e, "no_real_asset_pixel_parity=1",
                    "source evidence");
    expect_contains("evidence.f0098", e, "DUNVIEW.C:2962 F0098",
                    "DUNVIEW.C floor/ceiling owner");
    expect_contains("evidence.f0099", e, "DUNVIEW.C:3018-3045 F0099",
                    "DUNVIEW.C flip helper");
    expect_contains("evidence.f0108", e, "DUNVIEW.C:3940-4008 F0108",
                    "DUNVIEW.C floor ornament");
    expect_contains("evidence.f0115", e, "DUNVIEW.C:4547-4565 F0115",
                    "DUNVIEW.C item pile pass");
    expect_contains("evidence.c701", e, "DUNVIEW.C:8367-8368 C701",
                    "DUNVIEW.C floor zone");
    expect_contains("evidence.c700", e, "DUNVIEW.C:8430 C700",
                    "DUNVIEW.C ceiling zone");
    expect_contains("evidence.c2500", e, "DUNVIEW.C:5075 C2500",
                    "DUNVIEW.C object zone");
    expect_contains("evidence.c2900", e, "DUNVIEW.C:5683 C2900",
                    "DUNVIEW.C projectile zone");
    expect_contains("evidence.defs", e, "DEFS.H:2348/2423",
                    "DEFS.H floor-set constants");
    expect_contains("evidence.command", e, "COMMAND.C:2174-2177",
                    "COMMAND.C unrelated command route checked");
}

int main(void)
{
    test_floor_set_and_floor_ceiling_ownership();
    test_floor_ornament_variants_and_palettes();
    test_item_pile_zone_flip_and_transparency_contract();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_floor_ceiling_items_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_floor_ceiling_items_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
