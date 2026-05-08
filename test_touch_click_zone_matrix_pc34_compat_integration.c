#include <stdio.h>
#include <string.h>
#include "touch_click_zone_matrix_pc34_compat.h"
#include "action_area_routes_pc34_compat.h"
#include "champion_name_hand_routes_pc34_compat.h"
#include "spell_area_routes_pc34_compat.h"
#include "spell_area_symbol_routes_pc34_compat.h"

static int expect_zone(unsigned int ordinal, unsigned int commandId, unsigned int zoneIndex,
                       TouchClickCoordModePc34Compat coordMode, unsigned int buttonMask,
                       int x, int y, int w, int h, const char* groupName) {
    TouchClickZonePc34Compat zone;
    if (!TOUCHCLICK_Compat_GetZone(ordinal, &zone)) return 0;
    return zone.commandId == commandId && zone.zoneIndex == zoneIndex &&
           zone.coordMode == coordMode && zone.buttonMask == buttonMask &&
           zone.x == x && zone.y == y && zone.w == w && zone.h == h &&
           strcmp(zone.groupName, groupName) == 0;
}

static int has_zone(unsigned int commandId, unsigned int zoneIndex) {
    unsigned int i;
    for (i = 0; i < TOUCHCLICK_Compat_GetZoneCount(); ++i) {
        TouchClickZonePc34Compat zone;
        if (!TOUCHCLICK_Compat_GetZone(i, &zone)) return 0;
        if (zone.commandId == commandId && zone.zoneIndex == zoneIndex) return 1;
    }
    return 0;
}

int main(void) {
    TouchClickZonePc34Compat hit;
    TouchClickDispatchPc34Compat dispatch;
    ChampionStatusDispatchPc34Compat championDispatch;
    ChampionNameHandRoutePc34Compat championRoute;
    int normX = -1;
    int normY = -1;
    int ok = 1;
    printf("probe=firestaff_touch_click_zone_matrix\n");
    printf("sourceEvidence=%s\n", TOUCHCLICK_Compat_GetSourceEvidence());
    printf("zoneCount=%u\n", TOUCHCLICK_Compat_GetZoneCount());
    printf("championNameHandEvidence=%s\n", champion_name_hand_routes_GetEvidence());
    printf("championNameHandRouteCount=%u\n", champion_name_hand_routes_GetRouteCount());
    if (TOUCHCLICK_Compat_GetZoneCount() != 103u) ok = 0;

    if (!expect_zone(0u, 1u, 68u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 234, 125, 28, 21, "movement.turn_left")) ok = 0;
    if (!expect_zone(1u, 3u, 70u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 263, 125, 27, 21, "movement.forward")) ok = 0;
    if (!expect_zone(13u, 111u, 11u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 233, 77, 87, 45, "action.parent")) ok = 0;
    if (!expect_zone(15u, 113u, 82u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 234, 86, 85, 11, "action.row0")) ok = 0;
    if (!expect_zone(19u, 117u, 90u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 255, 86, 20, 35, "action.icon1")) ok = 0;
    if (!expect_zone(23u, 101u, 245u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 235, 51, 13, 11, "spell.symbol1")) ok = 0;
    if (!expect_zone(29u, 108u, 252u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 234, 63, 70, 11, "spell.cast")) ok = 0;
    if (!expect_zone(36u, 21u, 212u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 24, 10, 16, 16, "champion0.action_hand")) ok = 0;
    if (!expect_zone(48u, 28u, 507u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 6, 53, 16, 16, "inventory.ready_hand")) ok = 0;
    if (!expect_zone(49u, 29u, 508u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 62, 53, 16, 16, "inventory.action_hand")) ok = 0;
    if (!expect_zone(50u, 30u, 509u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 34, 26, 16, 16, "inventory.head")) ok = 0;
    if (!expect_zone(77u, 57u, 536u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 202, 33, 16, 16, "inventory.backpack_line1_9")) ok = 0;
    if (!expect_zone(78u, 58u, 537u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 117, 59, 16, 16, "inventory.chest_1")) ok = 0;
    if (!expect_zone(87u, 71u, 546u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 12, 13, 16, 16, "inventory.eye")) ok = 0;

    if (!action_area_routes_GetTouchMatrixInvariant()) ok = 0;
    if (!champion_name_hand_routes_GetInvariant()) ok = 0;
    if (champion_name_hand_routes_GetRouteCount() != 12u) ok = 0;
    if (!champion_name_hand_routes_GetRoute(6u, &championRoute) ||
        championRoute.commandId != 21u || championRoute.zoneIndex != 212u || championRoute.slotIndex != 1u ||
        strcmp(championRoute.name, "champion0.action_hand") != 0) ok = 0;
    if (!champion_name_hand_routes_ResolveStatusBoxClick(0u, 25, 11, 0u, &championDispatch) ||
        championDispatch.primaryCommandId != 12u || championDispatch.primaryZoneIndex != 151u ||
        championDispatch.secondaryCommandId != 21u || championDispatch.secondaryZoneIndex != 212u ||
        championDispatch.slotIndex != 1u || championDispatch.kind != CHAMPION_STATUS_DISPATCH_SLOT_BOX_PC34_COMPAT) ok = 0;
    if (!champion_name_hand_routes_ResolveStatusBoxClick(3u, 232, 11, 0u, &championDispatch) ||
        championDispatch.primaryCommandId != 15u || championDispatch.primaryZoneIndex != 154u ||
        championDispatch.secondaryCommandId != 27u || championDispatch.secondaryZoneIndex != 218u ||
        championDispatch.slotIndex != 7u || championDispatch.kind != CHAMPION_STATUS_DISPATCH_SLOT_BOX_PC34_COMPAT) ok = 0;
    if (!champion_name_hand_routes_ResolveStatusBoxClick(1u, 70, 1, 0u, &championDispatch) ||
        championDispatch.primaryCommandId != 13u || championDispatch.primaryZoneIndex != 152u ||
        championDispatch.secondaryCommandId != 17u || championDispatch.secondaryZoneIndex != 160u ||
        championDispatch.kind != CHAMPION_STATUS_DISPATCH_SET_LEADER_PC34_COMPAT) ok = 0;
    if (!champion_name_hand_routes_ResolveStatusBoxClick(2u, 170, 20, 3u, &championDispatch) ||
        championDispatch.primaryCommandId != 14u || championDispatch.primaryZoneIndex != 153u ||
        championDispatch.secondaryCommandId != 14u || championDispatch.secondaryZoneIndex != 153u ||
        championDispatch.kind != CHAMPION_STATUS_DISPATCH_SET_LEADER_PC34_COMPAT) ok = 0;
    if (champion_name_hand_routes_ResolveStatusBoxClick(0u, 50, 20, 0u, &championDispatch)) ok = 0;
    if (!spell_area_routes_GetInvariant()) ok = 0;
    if (!spell_area_symbol_routes_GetInvariant()) ok = 0;
    if (!has_zone(119u, 92u) || !has_zone(107u, 254u) || !has_zone(27u, 218u) || !has_zone(141u, 568u) || !has_zone(65u, 544u) || !has_zone(160u, 570u) || !has_zone(161u, 571u) || !has_zone(162u, 573u) || !has_zone(7u, 187u) || !has_zone(12u, 151u) || !has_zone(125u, 113u) || !has_zone(128u, 116u)) ok = 0;

    if (!TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(25, 11, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) ||
        hit.commandId != 12u || hit.zoneIndex != 151u || strcmp(hit.groupName, "champion0.status_box") != 0) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(25, 11, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &hit) ||
        hit.commandId != 7u || hit.zoneIndex != 151u || strcmp(hit.groupName, "champion0.toggle_box") != 0) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(264, 126, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) ||
        hit.commandId != 3u || hit.zoneIndex != 70u || strcmp(hit.groupName, "movement.forward") != 0) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(111, 82, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) ||
        hit.commandId != 80u || hit.zoneIndex != 7u || strcmp(hit.groupName, "viewport.dungeon") != 0) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(264, 126, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &hit) ||
        hit.commandId != 83u || hit.zoneIndex != 2u || strcmp(hit.groupName, "inventory.toggle_leader") != 0) ok = 0;

    if (!TOUCHCLICK_Compat_HitTest(264, 126, &hit) || hit.zoneIndex != 70u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(234, 125, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 1u || hit.zoneIndex != 68u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(261, 145, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 1u || hit.zoneIndex != 68u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(263, 125, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 3u || hit.zoneIndex != 70u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(289, 145, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 3u || hit.zoneIndex != 70u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(291, 125, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 2u || hit.zoneIndex != 69u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(318, 145, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 2u || hit.zoneIndex != 69u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(234, 147, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 6u || hit.zoneIndex != 73u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(261, 167, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 6u || hit.zoneIndex != 73u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(263, 147, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 5u || hit.zoneIndex != 72u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(289, 167, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 5u || hit.zoneIndex != 72u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(291, 147, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 4u || hit.zoneIndex != 71u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(318, 167, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 4u || hit.zoneIndex != 71u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(0, 33, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 80u || hit.zoneIndex != 7u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(223, 168, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 80u || hit.zoneIndex != 7u) ok = 0;
    if (TOUCHCLICK_Compat_HitTestWithButton(224, 168, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) && hit.zoneIndex == 7u) ok = 0;
    if (TOUCHCLICK_Compat_HitTestWithButton(223, 169, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) && hit.zoneIndex == 7u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTest(286, 78, &hit) || hit.zoneIndex != 98u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTest(236, 52, &hit) || hit.zoneIndex != 245u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTest(25, 11, &hit) || hit.zoneIndex != 212u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(25, 11, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.zoneIndex != 212u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(25, 11, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &hit) || hit.zoneIndex != 151u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestWithButton(264, 126, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.zoneIndex != 70u) ok = 0;
    if (TOUCHCLICK_Compat_HitTestWithButton(264, 126, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &hit) && hit.zoneIndex == 70u) ok = 0;
    if (TOUCHCLICK_Compat_HitTest(56, 13, &hit) && hit.zoneIndex == 545u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestInCoordMode(56, 13, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.zoneIndex != 545u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestInCoordMode(202, 33, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.zoneIndex != 536u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestInCoordMode(196, 105, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.zoneIndex != 544u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestInCoordMode(104, 53, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 160u || hit.zoneIndex != 570u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestInCoordMode(163, 53, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 161u || hit.zoneIndex != 571u) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestInCoordMode(104, 113, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.commandId != 162u || hit.zoneIndex != 573u) ok = 0;
    if (TOUCHCLICK_Compat_HitTestInCoordMode(56, 13, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &hit) && hit.zoneIndex == 545u) ok = 0;
    if (!TOUCHCLICK_Compat_MapViewportLocalPointToDispatch(56, 13, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &dispatch) ||
        dispatch.screenX != 56 || dispatch.screenY != 46 ||
        dispatch.commandId != 70u || dispatch.zoneIndex != 545u ||
        strcmp(dispatch.groupName, "inventory.mouth") != 0) ok = 0;
    if (!TOUCHCLICK_Compat_MapViewportLocalPointToDispatch(196, 105, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &dispatch) ||
        dispatch.screenX != 196 || dispatch.screenY != 138 ||
        dispatch.commandId != 65u || dispatch.zoneIndex != 544u ||
        strcmp(dispatch.groupName, "inventory.chest_8") != 0) ok = 0;
    if (!TOUCHCLICK_Compat_MapViewportLocalPointToDispatch(104, 113, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &dispatch) ||
        dispatch.screenX != 104 || dispatch.screenY != 146 ||
        dispatch.commandId != 162u || dispatch.zoneIndex != 573u ||
        strcmp(dispatch.groupName, "panel.cancel") != 0) ok = 0;
    if (TOUCHCLICK_Compat_MapViewportLocalPointToDispatch(56, 13, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &dispatch)) ok = 0;

    if (!TOUCHCLICK_Compat_NormalizeScaledScreenPoint(352, 180, 1280, 720, &normX, &normY) || normX != 80 || normY != 50) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestScaledScreenPoint(1056, 450, 1280, 720, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.zoneIndex != 70u) ok = 0;
    if (TOUCHCLICK_Compat_HitTestScaledScreenPoint(20, 20, 1280, 720, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit)) ok = 0;
    if (!TOUCHCLICK_Compat_NormalizeScaledScreenPoint(526, 290, 640, 480, &normX, &normY) || normX != 263 || normY != 125) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestScaledScreenPoint(526, 290, 640, 480, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.zoneIndex != 70u) ok = 0;


    if (!TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(1056, 450, 1280, 720, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &dispatch) ||
        dispatch.screenX != 275 || dispatch.screenY != 125 ||
        dispatch.buttonStatus != TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT ||
        dispatch.commandId != 3u || dispatch.zoneIndex != 70u ||
        strcmp(dispatch.groupName, "movement.forward") != 0) ok = 0;
    if (!TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(100, 39, 1280, 800, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &dispatch) ||
        dispatch.screenX != 25 || dispatch.screenY != 9 ||
        dispatch.buttonStatus != TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT ||
        dispatch.commandId != 7u || dispatch.zoneIndex != 151u ||
        strcmp(dispatch.groupName, "champion0.toggle_box") != 0) ok = 0;
    if (TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(20, 20, 1280, 720, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &dispatch)) ok = 0;
    if (TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(1056, 450, 1280, 720, 0u, &dispatch)) ok = 0;

    printf("normalizedScaledPoint=%d,%d\n", normX, normY);
    printf("actionAreaTouchMatrixInvariantOk=%u\n", action_area_routes_GetTouchMatrixInvariant());
    printf("championNameHandRoutesInvariantOk=%u\n", champion_name_hand_routes_GetInvariant());
    printf("touchClickZoneMatrixInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
