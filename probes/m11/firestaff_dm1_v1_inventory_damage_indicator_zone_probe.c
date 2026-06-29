/*
 * DM1 V1 champion-panel inventory damage indicator zone probe.
 *
 * Data-free M11 helper gate for the ReDMCSB CHAMDRAW.C F0623:688-699
 * split: C016/C179..C182 for the open-inventory champion, C015/C167..C170
 * for the other champion status boxes. This does not load GRAPHICS.DAT
 * and does not claim original DOS pixel parity.
 */
#include "m11_game_view.h"

#include <stdio.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *label, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d\n", label, got, want);
    } else {
        printf("PASS %s == %d\n", label, want);
    }
}

static void expect_true(const char *label, int got)
{
    expect_int(label, got ? 1 : 0, 1);
}

static void expect_false(const char *label, int got)
{
    expect_int(label, got ? 1 : 0, 0);
}

int main(void)
{
    int x, y, w, h;

    expect_int("small.gfx", M11_GameView_GetV1ChampionSmallDamageGraphicId(), 15);
    expect_int("big.gfx", M11_GameView_GetV1ChampionBigDamageGraphicId(), 16);

    expect_int("small.zone_id.slot0", M11_GameView_GetV1DamageIndicatorZoneId(0), 167);
    expect_int("small.zone_id.slot3", M11_GameView_GetV1DamageIndicatorZoneId(3), 170);
    expect_int("big.zone_id.slot0", M11_GameView_GetV1InventoryDamageIndicatorZoneId(0), 179);
    expect_int("big.zone_id.slot3", M11_GameView_GetV1InventoryDamageIndicatorZoneId(3), 182);
    expect_int("small.zone_id.negative", M11_GameView_GetV1DamageIndicatorZoneId(-1), 0);
    expect_int("big.zone_id.high", M11_GameView_GetV1InventoryDamageIndicatorZoneId(4), 0);

    expect_true("small.zone.slot0",
                M11_GameView_GetV1DamageIndicatorZone(0, 45, 7, &x, &y, &w, &h));
    expect_int("small.slot0.x", x, 11);
    expect_int("small.slot0.y", y, 11);
    expect_int("small.slot0.w", w, 45);
    expect_int("small.slot0.h", h, 7);

    expect_true("small.zone.slot3",
                M11_GameView_GetV1DamageIndicatorZone(3, 45, 7, &x, &y, &w, &h));
    expect_int("small.slot3.x", x, 218);
    expect_int("small.slot3.y", y, 11);
    expect_int("small.slot3.w", w, 45);
    expect_int("small.slot3.h", h, 7);

    expect_true("big.zone.slot0",
                M11_GameView_GetV1InventoryDamageIndicatorZone(0, 32, 29, &x, &y, &w, &h));
    expect_int("big.slot0.x", x, 7);
    expect_int("big.slot0.y", y, 0);
    expect_int("big.slot0.w", w, 32);
    expect_int("big.slot0.h", h, 29);

    expect_true("big.zone.slot3",
                M11_GameView_GetV1InventoryDamageIndicatorZone(3, 32, 29, &x, &y, &w, &h));
    expect_int("big.slot3.x", x, 214);
    expect_int("big.slot3.y", y, 0);
    expect_int("big.slot3.w", w, 32);
    expect_int("big.slot3.h", h, 29);

    expect_false("big.zone.invalid_size",
                 M11_GameView_GetV1InventoryDamageIndicatorZone(0, 0, 29, &x, &y, &w, &h));
    expect_false("big.zone.invalid_slot",
                 M11_GameView_GetV1InventoryDamageIndicatorZone(-1, 32, 29, &x, &y, &w, &h));

    printf("dm1_v1_inventory_damage_indicator_zone_probe: assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
