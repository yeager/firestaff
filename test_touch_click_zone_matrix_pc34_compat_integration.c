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
    int normX = -1;
    int normY = -1;
    int ok = 1;
    printf("probe=firestaff_touch_click_zone_matrix\n");
    printf("sourceEvidence=%s\n", TOUCHCLICK_Compat_GetSourceEvidence());
    printf("zoneCount=%u\n", TOUCHCLICK_Compat_GetZoneCount());
    if (TOUCHCLICK_Compat_GetZoneCount() != 88u) ok = 0;

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
    if (!expect_zone(78u, 58u, 537u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 37, 7, 16, 16, "inventory.chest_1")) ok = 0;
    if (!expect_zone(87u, 71u, 546u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 12, 13, 16, 16, "inventory.eye")) ok = 0;

    if (!action_area_routes_GetTouchMatrixInvariant()) ok = 0;
    if (!champion_name_hand_routes_GetInvariant()) ok = 0;
    if (!spell_area_routes_GetInvariant()) ok = 0;
    if (!spell_area_symbol_routes_GetInvariant()) ok = 0;
    if (!has_zone(119u, 92u) || !has_zone(107u, 254u) || !has_zone(27u, 218u) || !has_zone(141u, 568u) || !has_zone(65u, 544u)) ok = 0;

    if (!TOUCHCLICK_Compat_HitTest(264, 126, &hit) || hit.zoneIndex != 70u) ok = 0;
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
    if (!TOUCHCLICK_Compat_HitTestInCoordMode(116, 53, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.zoneIndex != 544u) ok = 0;
    if (TOUCHCLICK_Compat_HitTestInCoordMode(56, 13, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &hit) && hit.zoneIndex == 545u) ok = 0;

    if (!TOUCHCLICK_Compat_NormalizeScaledScreenPoint(352, 180, 1280, 720, &normX, &normY) || normX != 80 || normY != 50) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestScaledScreenPoint(1056, 450, 1280, 720, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.zoneIndex != 70u) ok = 0;
    if (TOUCHCLICK_Compat_HitTestScaledScreenPoint(20, 20, 1280, 720, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit)) ok = 0;
    if (!TOUCHCLICK_Compat_NormalizeScaledScreenPoint(526, 290, 640, 480, &normX, &normY) || normX != 263 || normY != 125) ok = 0;
    if (!TOUCHCLICK_Compat_HitTestScaledScreenPoint(526, 290, 640, 480, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &hit) || hit.zoneIndex != 70u) ok = 0;

    printf("normalizedScaledPoint=%d,%d\n", normX, normY);
    printf("actionAreaTouchMatrixInvariantOk=%u\n", action_area_routes_GetTouchMatrixInvariant());
    printf("touchClickZoneMatrixInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
