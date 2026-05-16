/* DM1 V2 HUD/touch interaction bridge — source-locked to the V1 PC34 route matrix.
 *
 * This file deliberately consumes the existing V1-compatible touch tables instead
 * of redefining coordinates.  ReDMCSB anchors:
 * - COMMAND.C:375-395 primary champion status-box scan.
 * - COMMAND.C:461-471 action-area parent, rows, pass and champion action icons.
 * - COMMAND.C:484-497 champion name/ready/action hand subroutes.
 * - CLIKCHAM.C:24-35 F0367 dispatches status-box clicks to set-leader/slot paths.
 */

#include "dm1_v2_hud_interaction_pc34.h"
#include "dm1_v2_champion_select_pc34.h"
#include "action_area_routes_pc34_compat.h"
#include "champion_name_hand_routes_pc34_compat.h"
#include <string.h>

static void v2_hud_touch_clear(M11_V2_HudTouchResult* outResult) {
    if (outResult) memset(outResult, 0, sizeof(*outResult));
}

static int v2_group_champion_index(const char* groupName, unsigned int* outChampionIndex) {
    if (!groupName || strncmp(groupName, "champion", 8) != 0) return 0;
    if (groupName[8] < '0' || groupName[8] > '3') return 0;
    if (outChampionIndex) *outChampionIndex = (unsigned int)(groupName[8] - '0');
    return 1;
}

static int v2_route_is_champion_hand(unsigned int commandId) {
    return commandId >= 20u && commandId <= 27u;
}

static int v2_route_is_champion_name_or_box(unsigned int commandId) {
    return (commandId >= 7u && commandId <= 10u) ||
           (commandId >= 16u && commandId <= 19u);
}

static int v2_route_is_action_row(unsigned int commandId) {
    return commandId >= 112u && commandId <= 115u;
}

static int v2_route_is_action_icon(unsigned int commandId) {
    return commandId >= 116u && commandId <= 119u;
}

static void v2_apply_touch_zone(const TouchClickZonePc34Compat* zone,
                                int screenX,
                                int screenY,
                                M11_V2_HudTouchResult* outResult) {
    unsigned int championIndex = 0u;
    if (!zone || !outResult) return;

    outResult->hit = 1;
    outResult->commandId = zone->commandId;
    outResult->zoneIndex = zone->zoneIndex;
    outResult->screenX = screenX;
    outResult->screenY = screenY;
    outResult->groupName = zone->groupName;
    outResult->sourceEvidence = zone->sourceEvidence;

    if (v2_route_is_action_icon(zone->commandId)) {
        championIndex = zone->commandId - 116u;
        outResult->kind = M11_V2_HUD_TOUCH_ACTION_ICON_PC34;
        outResult->championIndex = championIndex;
        v2_champion_select_focus_index_pc34(championIndex);
        return;
    }

    if (zone->commandId == 111u) {
        outResult->kind = M11_V2_HUD_TOUCH_ACTION_PARENT_PC34;
        return;
    }

    if (v2_route_is_action_row(zone->commandId)) {
        outResult->kind = M11_V2_HUD_TOUCH_ACTION_ROW_PC34;
        return;
    }

    if (!v2_group_champion_index(zone->groupName, &championIndex)) return;

    outResult->championIndex = championIndex;
    if (v2_route_is_champion_hand(zone->commandId)) {
        outResult->kind = M11_V2_HUD_TOUCH_CHAMPION_HAND_PC34;
        outResult->slotIndex = zone->commandId - 20u;
    } else if (v2_route_is_champion_name_or_box(zone->commandId)) {
        outResult->kind = M11_V2_HUD_TOUCH_CHAMPION_FOCUS_PC34;
    }
    v2_champion_select_focus_index_pc34(championIndex);
}

int v2_hud_interaction_dispatch_screen_click(int screenX,
                                             int screenY,
                                             unsigned int buttonMask,
                                             M11_V2_HudTouchResult* outResult) {
    TouchClickZonePc34Compat zone;
    v2_hud_touch_clear(outResult);
    if (!outResult || buttonMask == 0u) return 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(screenX, screenY, buttonMask, &zone)) return 0;
    v2_apply_touch_zone(&zone, screenX, screenY, outResult);
    return outResult->kind != M11_V2_HUD_TOUCH_NONE_PC34;
}

int v2_hud_interaction_dispatch_scaled_click(int physicalX,
                                             int physicalY,
                                             int surfaceW,
                                             int surfaceH,
                                             unsigned int buttonMask,
                                             M11_V2_HudTouchResult* outResult) {
    int screenX = 0;
    int screenY = 0;
    v2_hud_touch_clear(outResult);
    if (!TOUCHCLICK_Compat_NormalizeScaledScreenPoint(physicalX, physicalY, surfaceW, surfaceH, &screenX, &screenY)) return 0;
    return v2_hud_interaction_dispatch_screen_click(screenX, screenY, buttonMask, outResult);
}

unsigned int v2_hud_interaction_source_lock_ok(void) {
    ChampionStatusDispatchPc34Compat championDispatch;
    return action_area_routes_GetTouchMatrixInvariant() &&
           champion_name_hand_routes_GetInvariant() &&
           TOUCHCLICK_Compat_HitTestWithButton(25, 11, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0) &&
           champion_name_hand_routes_ResolveStatusBoxClick(0u, 25, 11, 0u, &championDispatch) &&
           championDispatch.secondaryCommandId == 21u &&
           championDispatch.secondaryZoneIndex == 212u;
}

const char* v2_hud_interaction_get_source_evidence(void) {
    return "V2 bridge consumes the V1 PC34 route matrix: touch_click_zone_matrix_pc34_compat.c:25-66 mirrors COMMAND.C champion/action subroutes; action_area_routes_pc34_compat.c:14-15 locks C111..C119 to COMMAND.C/CLIKMENU.C/MENU.C; champion_name_hand_routes_pc34_compat.c:10-22 and 73-74 lock C016..C027/C151..C154 to COMMAND.C/CLIKCHAM.C/CHAMPION.C.";
}
