/*
 * test_csb_v1_graphics_extras_pc34_compat.c
 *
 * CSB V1 Graphics GAP 2 (engine version display CHANGE7_36,
 * CHANGE8_13) + Champions GAP 4 (left-click inventory
 * CHANGE7_28) + Dungeon GAP 6 (teleporter access + Grey
 * Lord).  Bounded-fix regression gate.
 */
#include "csb_v1_engine_version_display_pc34_compat.h"
#include "csb_v1_left_click_inventory_pc34_compat.h"
#include "csb_v1_teleporter_access_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    printf("=== CSB V1 Graphics GAP 2 + Champions GAP 4 + Dungeon GAP 6 ===\n");

    /* ── GAP 2: engine version display ── */
    csb_v1_engine_version_display_set_csb(0);
    CHECK(csb_v1_engine_version_display_is_csb() == 0,
          "engine version display: default DM1");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "DM1 engine version = v2.0");
    csb_v1_engine_version_display_set_csb(1);
    CHECK(csb_v1_engine_version_display_is_csb() == 1,
          "engine version display: can be set to CSB");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.1") == 0,
          "CSB engine version = v2.1 (CHANGE8_13 hardcoded)");
    csb_v1_engine_version_display_set_csb(0);
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "engine version display: can be reset to DM1");

    /* ── GAP 4: left-click inventory (CHANGE7_28) ── */
    csb_v1_left_click_inventory_set(0);
    CHECK(csb_v1_left_click_inventory_get() == 0,
          "left-click inventory: default disabled (DM1)");
    CHECK(csb_v1_champion_icon_left_click_command(0) == 0,
          "left-click inventory: slot 0 -> 0 when disabled");
    CHECK(csb_v1_champion_icon_left_click_command(3) == 0,
          "left-click inventory: slot 3 -> 0 when disabled");
    csb_v1_left_click_inventory_set(1);
    CHECK(csb_v1_left_click_inventory_get() == 1,
          "left-click inventory: can be enabled");
    CHECK(csb_v1_champion_icon_left_click_command(0) == 125,
          "left-click inventory: slot 0 -> C125 (top-left)");
    CHECK(csb_v1_champion_icon_left_click_command(1) == 126,
          "left-click inventory: slot 1 -> C126 (top-right)");
    CHECK(csb_v1_champion_icon_left_click_command(2) == 127,
          "left-click inventory: slot 2 -> C127 (bottom-right)");
    CHECK(csb_v1_champion_icon_left_click_command(3) == 128,
          "left-click inventory: slot 3 -> C128 (bottom-left)");
    CHECK(csb_v1_champion_icon_left_click_command(-1) == 0,
          "left-click inventory: slot -1 -> 0 (out of range)");
    CHECK(csb_v1_champion_icon_left_click_command(4) == 0,
          "left-click inventory: slot 4 -> 0 (out of range)");
    csb_v1_left_click_inventory_set(0);
    CHECK(csb_v1_champion_icon_left_click_command(0) == 0,
          "left-click inventory: can be reset to disabled");

    /* ── GAP 6: teleporter access + Grey Lord ── */
    csb_v1_teleporter_access_set(0);
    CHECK(csb_v1_teleporter_access_get() == 0,
          "teleporter access: default disabled (DM1)");
    CHECK(csb_v1_can_creature_use_teleporter(22) == 1,
          "teleporter: Lord Chaos (22) -> can teleport (DM1+CSB)");
    CHECK(csb_v1_can_creature_use_teleporter(24) == 1,
          "teleporter: Lord Order (24) -> can teleport (DM1+CSB)");
    CHECK(csb_v1_can_creature_use_teleporter(26) == 0,
          "teleporter: Grey Lord (26) -> cannot (DM1 baseline)");
    CHECK(csb_v1_can_creature_use_teleporter(27) == 0,
          "teleporter: Materializer (27) -> cannot (DM1 baseline)");
    CHECK(csb_v1_can_creature_use_teleporter(0) == 0,
          "teleporter: Screamer (0) -> cannot");
    csb_v1_teleporter_access_set(1);
    CHECK(csb_v1_can_creature_use_teleporter(26) == 1,
          "teleporter: Grey Lord (26) -> can teleport (CSB)");
    CHECK(csb_v1_can_creature_use_teleporter(27) == 1,
          "teleporter: Materializer (27) -> can teleport (CSB)");
    CHECK(csb_v1_can_creature_use_teleporter(22) == 1,
          "teleporter: Lord Chaos (22) -> can teleport (CSB unchanged)");
    csb_v1_teleporter_access_set(0);
    CHECK(csb_v1_can_creature_use_teleporter(26) == 0,
          "teleporter: reset to DM1, Grey Lord -> cannot");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
