/* test_dm2_v1_runtime_shop_pc34_compat.c
 *
 * DM2 V1 runtime shop provenance gate.
 *
 * A data-free profile must not turn fixed coordinates into a catalog shop.
 * Source: SKWINSPX/src/v4/skguidrw.cpp::_32cb_0f82_SHOP_GLASS.
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_game.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_shop.h"
#include "dm2_v1_tech_magic.h"

#include <stdint.h>

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

    printf("DM2 V1 runtime shop provenance tests\n\n");

    make_synthetic_verified_profile(&profile);
    CHECK(dm2_v1_boot_enter_game(&profile) == 0,
          "synthetic verified profile enters DM2 V1 game state");
    CHECK(profile.dm2_state != NULL,
          "boot handoff populates dm2_state");
    CHECK(dm2_v1_runtime_bind_boot_profile(&profile) == 1,
          "runtime binds to synthetic boot profile");

    state = (DM2_V1_GameState *)profile.dm2_state;

    state->gold = 240;
    dm2_v1_shop_reset_state();
    dm2_v1_runtime_set_position(0, 10, 6, 0);
    dm2_v1_runtime_set_outdoor(1);

    CHECK(dm2_v1_runtime_enter_shop(0, 10, 5) < 0,
          "coordinate-only shop entry is rejected");
    CHECK(dm2_v1_shop_is_active() == 0 && state->gold == 240,
          "rejection preserves catalog inactivity and source game state");
    CHECK(dm2_v1_runtime_buy_from_shop(0) ==
              (int)DM2_SHOP_RESULT_NO_ACTIVE_SHOP,
          "buy has no active catalog after rejection");
    CHECK(dm2_v1_runtime_sell_to_shop(0) ==
              (int)DM2_SHOP_RESULT_NO_ACTIVE_SHOP &&
              dm2_v1_runtime_leave_shop() < 0,
          "sell and leave also reject without source shop ownership");
    CHECK(dm2_v1_runtime_npc_interact(0, 10, 5) < 0 &&
              state->reputation == 0 &&
              dm2_v1_runtime_get_last_npc_id() == DM2_NPC_NONE &&
              dm2_v1_runtime_get_last_npc_dialog_line() < 0,
          "coordinate-only NPC interaction cannot invent merchant identity, dialog or reputation");
    CHECK(dm2_v1_runtime_invoke_actuator(
              0, 10, 5, DM2_ACTUATOR_PUSH_BUTTON_WALL_SWITCH, 0u) < 0 &&
              dm2_v1_runtime_get_actuator_count() == 0,
          "generic wall-switch input cannot invent a DB3 actuator transition");
    CHECK(dm2_v1_runtime_invoke_actuator(
              0, 10, 5, DM2_ACTUATOR_CREATURE_GENERATOR,
              DM2_AI_DRAGOTH_MINION) < 0 &&
              dm2_v1_runtime_get_spawn_count() == 0,
          "unowned creature-generator flag cannot create a creature");
    CHECK(dm2_v1_runtime_invoke_actuator(
              0, 10, 5, DM2_ACTUATOR_ITEM_GENERATOR, 0x1234u) < 0 &&
              dm2_v1_runtime_get_last_generated_object() == 0u,
          "unowned item-generator flag cannot create an object");

    printf("\n%d/%d checks passed\n", passed, passed + failed);
    return failed ? 1 : 0;
}
