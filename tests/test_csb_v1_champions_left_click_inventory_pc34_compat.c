/*
 * test_csb_v1_champions_left_click_inventory_pc34_compat.c
 *
 * CSB V1 Champions GAP 4 (Left-Click Inventory Access, CHANGE7_28)
 * test gate.  Source-locked per ReDMCSB DEFS.H:327-330
 * (C125..C128 commands), COMMAND.C CHANGE7_28 (left-click on
 * champion icon opens inventory), DEFS.H:226
 * (C33_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION).
 *
 * In CSB PC 3.4 a left-click on a champion icon opens the
 * champion inventory; DM1 PC 3.4 requires right-click.  v1
 * implements a bounded version that maps a left-click on
 * champion icon slot N (0..3) to command C125+N when the
 * left-click inventory mode is enabled, or 0 (no dispatch)
 * when disabled.  The dispatch is plumbed through the
 * existing F0378 panel route in M11.
 */
#include "csb_v1_left_click_inventory_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    printf("=== CSB V1 Champions GAP 4: Left-Click Inventory (CHANGE7_28) ===\n");

    /* --- default behaviour: DM1 mode, left-click disabled --- */
    csb_v1_left_click_inventory_set(0);
    CHECK(csb_v1_left_click_inventory_get() == 0,
          "default mode is DM1 (left-click disabled)");

    /* All 4 slots return 0 (no dispatch) when disabled. */
    for (int slot = 0; slot < 4; ++slot) {
        int cmd = csb_v1_champion_icon_left_click_command(slot);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "disabled-mode slot %d returns 0 (got %d)", slot, cmd);
        CHECK(cmd == 0, buf);
    }

    /* --- enable CSB mode --- */
    csb_v1_left_click_inventory_set(1);
    CHECK(csb_v1_left_click_inventory_get() == 1,
          "set(1) flips to CSB mode (left-click enabled)");

    /* All 4 slots return C125..C128 (125..128) when enabled. */
    for (int slot = 0; slot < 4; ++slot) {
        int cmd = csb_v1_champion_icon_left_click_command(slot);
        int expected = 125 + slot;
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "enabled-mode slot %d returns C%d (got %d)", slot, expected, cmd);
        CHECK(cmd == expected, buf);
    }

    /* --- out-of-range slot returns 0 --- */
    CHECK(csb_v1_champion_icon_left_click_command(-1) == 0,
          "slot -1 returns 0");
    CHECK(csb_v1_champion_icon_left_click_command(4) == 0,
          "slot 4 returns 0");
    CHECK(csb_v1_champion_icon_left_click_command(99) == 0,
          "slot 99 returns 0");

    /* --- disable again: reverts to DM1 behaviour --- */
    csb_v1_left_click_inventory_set(0);
    CHECK(csb_v1_left_click_inventory_get() == 0,
          "set(0) reverts to DM1 mode");
    CHECK(csb_v1_champion_icon_left_click_command(2) == 0,
          "after disable, slot 2 returns 0 again");

    /* --- non-zero values for enabled flag are normalised to 1 --- */
    csb_v1_left_click_inventory_set(42);
    CHECK(csb_v1_left_click_inventory_get() == 1,
          "set(42) normalised to enabled");
    csb_v1_left_click_inventory_set(0);

    printf("=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
