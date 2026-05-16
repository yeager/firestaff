#include "dm1_v2_hud_interaction_pc34.h"
#include "dm1_v2_champion_select_pc34.h"
#include "dm1_v2_hud_overlay_pc34.h"
#include "action_area_routes_pc34_compat.h"
#include "champion_name_hand_routes_pc34_compat.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int expect_dispatch(int x,
                           int y,
                           unsigned int buttonMask,
                           M11_V2_HudTouchKind kind,
                           unsigned int commandId,
                           unsigned int zoneIndex,
                           unsigned int championIndex,
                           unsigned int slotIndex,
                           const char* groupName,
                           int expectedFocus) {
    M11_V2_HudTouchResult result;
    if (!v2_hud_interaction_dispatch_screen_click(x, y, buttonMask, &result)) return 0;
    if (!result.hit || result.kind != kind) return 0;
    if (result.commandId != commandId || result.zoneIndex != zoneIndex) return 0;
    if (result.championIndex != championIndex || result.slotIndex != slotIndex) return 0;
    if (!result.groupName || strcmp(result.groupName, groupName) != 0) return 0;
    if (!result.sourceEvidence || strstr(result.sourceEvidence, "COMMAND.C") == 0) return 0;
    if (expectedFocus >= 0 && v2_champion_select_current_index_pc34() != expectedFocus) return 0;
    return 1;
}

int main(void) {
    M11_V2_HudTouchResult result;
    uint8_t fb[320 * 200];
    int ok = 1;

    printf("probe=firestaff_dm1_v2_hud_interaction_pc34\n");
    printf("sourceEvidence=%s\n", v2_hud_interaction_get_source_evidence());
    printf("actionEvidence=%s\n", action_area_routes_GetEvidence());
    printf("championEvidence=%s\n", champion_name_hand_routes_GetEvidence());

    v2_champion_select_init();
    v2_hud_init();
    memset(fb, 0, sizeof(fb));
    v2_hud_set_direction(2);
    v2_hud_set_level(3, 10);
    v2_hud_render(fb, 320, 200);
    if (fb[8 * 320 + 8] == 0 || fb[184 * 320 + 8] == 0) ok = 0;

    if (!v2_hud_interaction_source_lock_ok()) ok = 0;
    if (!expect_dispatch(25, 9, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,
                         M11_V2_HUD_TOUCH_CHAMPION_FOCUS_PC34,
                         7u, 151u, 0u, 0u, "champion0.toggle_box", 0)) ok = 0;
    if (!expect_dispatch(139, 1, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         M11_V2_HUD_TOUCH_CHAMPION_FOCUS_PC34,
                         18u, 161u, 2u, 0u, "champion2.name", 2)) ok = 0;
    if (!expect_dispatch(232, 11, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         M11_V2_HUD_TOUCH_CHAMPION_HAND_PC34,
                         27u, 218u, 3u, 7u, "champion3.action_hand", 3)) ok = 0;
    if (!expect_dispatch(256, 87, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         M11_V2_HUD_TOUCH_ACTION_ICON_PC34,
                         117u, 90u, 1u, 0u, "action.icon1", 1)) ok = 0;
    if (!expect_dispatch(254, 99, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         M11_V2_HUD_TOUCH_ACTION_ROW_PC34,
                         114u, 83u, 0u, 0u, "action.row1", -1)) ok = 0;
    if (!expect_dispatch(234, 78, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         M11_V2_HUD_TOUCH_ACTION_PARENT_PC34,
                         111u, 11u, 0u, 0u, "action.parent", -1)) ok = 0;

    if (!v2_hud_interaction_dispatch_scaled_click(100, 39, 1280, 800,
                                                  TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &result) ||
        result.commandId != 7u || result.zoneIndex != 151u || result.championIndex != 0u) ok = 0;
    if (v2_hud_interaction_dispatch_screen_click(264, 126, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &result)) ok = 0;
    if (v2_hud_interaction_dispatch_screen_click(25, 9, 0u, &result)) ok = 0;

    printf("hudInteractionSourceLockOk=%u\n", v2_hud_interaction_source_lock_ok());
    printf("hudInteractionOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
