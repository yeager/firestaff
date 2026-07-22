/* test_dm2_v1_runtime_shop_pc34_compat.c
 *
 * DM2 V1 runtime shop gold-transaction writeback test.
 *
 * Verifies the Lane B (DM2 V1 mechanics parity) fix for shop/NPC gold
 * transactions:
 *   - dm2_v1_runtime_enter_shop() syncs gs->gold into the shop module
 *   - dm2_v1_runtime_buy_from_shop() commits the deducted gold back to
 *     DM2_V1_GameState on success
 *   - dm2_v1_runtime_sell_to_shop() commits the added gold back to
 *     DM2_V1_GameState on success
 *   - dm2_v1_runtime_leave_shop() writes the final shop gold to gs->gold
 *
 * The test uses a synthetic verified boot profile, so it runs without real
 * DM2 assets.
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_game.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_shop.h"
#include "dm2_v1_tech_magic.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else      { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static void make_synthetic_verified_profile(DM2_V1_BootProfile *profile)
{
    dm2_v1_boot_profile_init(profile);
    profile->assets_verified = 1;
    snprintf(profile->asset_root, sizeof(profile->asset_root),
             "synthetic-dm2-v1-runtime-shop");
}

int main(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_GameState *state;

    printf("DM2 V1 runtime shop gold writeback tests\n\n");

    make_synthetic_verified_profile(&profile);
    CHECK(dm2_v1_boot_enter_game(&profile) == 0,
          "synthetic verified profile enters DM2 V1 game state");
    CHECK(profile.dm2_state != NULL,
          "boot handoff populates dm2_state");
    CHECK(dm2_v1_runtime_bind_boot_profile(&profile) == 1,
          "runtime binds to synthetic boot profile");

    state = (DM2_V1_GameState *)profile.dm2_state;

    /* General Store is at map 0, (10,5).  Need outdoor=1 for enter_shop. */
    state->gold = 240;
    dm2_v1_shop_reset_state();
    dm2_v1_runtime_set_position(0, 10, 6, 0);
    dm2_v1_runtime_set_outdoor(1);

    CHECK(dm2_v1_runtime_enter_shop(0, 10, 5) == 0,
          "runtime enters General Store by map position");
    CHECK(dm2_v1_shop_is_active() == 1 &&
          dm2_v1_shop_get_active_shop() == DM2_SHOP_ID_GENERAL,
          "General Store becomes active shop");
    CHECK(dm2_v1_shop_get_party_gold() == 240u,
          "enter_shop syncs gs->gold into shop state");
    CHECK(state->gold == 240,
          "gs->gold is unchanged by enter alone");

    /* General Store stock 0 = Lantern, base 30; at 50% skill price is 15. */
    CHECK(dm2_v1_runtime_buy_from_shop(0) == 1,
          "runtime buys stock item 0 from active shop");
    CHECK(state->gold == 225,
          "gs->gold is committed after a successful buy");
    CHECK(dm2_v1_shop_get_party_gold() == 225u,
          "shop-local gold matches committed game-state gold");

    /* Sell a Heal Potion back: base 25 -> sell price 12.
     * Slot 0 is the Lantern bought above; the potion is slot 1. */
    dm2_v1_shop_add_inventory(DM2_ITEM_HEAL_POTION, 1);
    CHECK(dm2_v1_runtime_sell_to_shop(1) == 1,
          "runtime sells inventory item 1 to active shop");
    CHECK(state->gold == 237,
          "gs->gold is committed after a successful sell");

    /* Leave persists the final gold amount. */
    CHECK(dm2_v1_runtime_leave_shop() == 0,
          "runtime leaves active shop");
    CHECK(dm2_v1_shop_is_active() == 0,
          "shop is inactive after leave");
    CHECK(state->gold == 237,
          "gs->gold remains after leave");

    /* Failure path: buy with no active shop returns the source error. */
    CHECK(dm2_v1_runtime_buy_from_shop(0) ==
              (int)DM2_SHOP_RESULT_NO_ACTIVE_SHOP,
          "buy without active shop returns NO_ACTIVE_SHOP");

    /* Failure path: insufficient gold does not mutate gs->gold. */
    state->gold = 5;
    dm2_v1_runtime_enter_shop(0, 10, 5);
    CHECK(dm2_v1_runtime_buy_from_shop(0) ==
              (int)DM2_SHOP_RESULT_INSUFFICIENT_GOLD,
          "buy with insufficient gold fails");
    CHECK(state->gold == 5,
          "gs->gold unchanged on failed buy");

    printf("\n%d/%d checks passed\n", passed, passed + failed);
    return failed ? 1 : 0;
}
