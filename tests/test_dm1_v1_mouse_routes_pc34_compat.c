#include "dm1_v1_mouse_routes_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass;
static int g_fail;

#define ASSERT_TRUE(expr, msg) do { \
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

static int test_zone_rect(int zoneId,
                          void* user,
                          int* outX,
                          int* outY,
                          int* outW,
                          int* outH) {
    (void)user;
    if (zoneId == 2) {
        *outX = 0; *outY = 0; *outW = 320; *outH = 200;
        return 1;
    }
    if (zoneId == 68) {
        *outX = 250; *outY = 121; *outW = 14; *outH = 13;
        return 1;
    }
    if (zoneId >= 211 && zoneId <= 218) {
        *outX = 10 + (zoneId - 211) * 10; *outY = 5; *outW = 8; *outH = 8;
        return 1;
    }
    if (zoneId == 507) {
        *outX = 6; *outY = 53; *outW = 16; *outH = 16;
        return 1;
    }
    if (zoneId == 537) {
        *outX = 117; *outY = 59; *outW = 16; *outH = 16;
        return 1;
    }
    if (zoneId == 545) {
        *outX = 56; *outY = 13; *outW = 16; *outH = 16;
        return 1;
    }
    if (zoneId == 101) {
        *outX = 110; *outY = 50; *outW = 90; *outH = 80;
        return 1;
    }
    return 0;
}

static void assert_route(int list,
                         int index,
                         int command,
                         int space,
                         int zone,
                         int mask,
                         const char* label) {
    DM1_V1_MouseRoutePc34Compat route;
    ASSERT_TRUE(DM1_V1_MouseRoutes_GetRoutePc34Compat(list, index, &route), label);
    ASSERT_EQ(route.command, command, label);
    ASSERT_EQ(route.coordinateSpace, space, label);
    ASSERT_EQ(route.zoneId, zone, label);
    ASSERT_EQ(route.buttonMask, mask, label);
}

int main(void) {
    int space = -1;
    int zone = -1;
    int command;

    ASSERT_TRUE(strstr(DM1_V1_MouseRoutes_SourceEvidencePc34Compat(),
                       "COMMAND.C G0447") != NULL,
                "source evidence names ReDMCSB route tables");
    ASSERT_EQ(DM1_V1_MouseRoutes_GetRouteCountPc34Compat(DM1_V1_MOUSE_LIST_INTERFACE_PC34),
              26,
              "interface route count");
    ASSERT_EQ(DM1_V1_MouseRoutes_GetRouteCountPc34Compat(DM1_V1_MOUSE_LIST_MOVEMENT_PC34),
              8,
              "movement route count");
    ASSERT_EQ(DM1_V1_MouseRoutes_GetRouteCountPc34Compat(DM1_V1_MOUSE_LIST_INVENTORY_PC34),
              46,
              "inventory route count");
    ASSERT_EQ(DM1_V1_MouseRoutes_GetRouteCountPc34Compat(
                  DM1_V1_MOUSE_LIST_PANEL_CHEST_PC34),
              8,
              "panel chest route count");
    ASSERT_EQ(DM1_V1_MouseRoutes_GetRouteCountPc34Compat(
                  DM1_V1_MOUSE_LIST_PARTY_RESTING_PC34),
              2,
              "party resting wake route count");

    assert_route(DM1_V1_MOUSE_LIST_INTERFACE_PC34, 0, 20,
                 DM1_V1_MOUSE_SPACE_SCREEN_PC34, 211,
                 DM1_V1_MOUSE_MASK_LEFT_PC34, "interface hand route");
    assert_route(DM1_V1_MOUSE_LIST_MOVEMENT_PC34, 0, 1,
                 DM1_V1_MOUSE_SPACE_SCREEN_PC34, 68,
                 DM1_V1_MOUSE_MASK_LEFT_PC34, "movement forward-left route");
    assert_route(DM1_V1_MOUSE_LIST_INVENTORY_PC34, 9, 28,
                 DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 507,
                 DM1_V1_MOUSE_MASK_LEFT_PC34, "inventory ready hand route");
    assert_route(DM1_V1_MOUSE_LIST_INVENTORY_PC34, 16, 70,
                 DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 545,
                 DM1_V1_MOUSE_MASK_LEFT_PC34, "inventory mouth route");
    assert_route(DM1_V1_MOUSE_LIST_PANEL_CHEST_PC34, 0, 58,
                 DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 537,
                 DM1_V1_MOUSE_MASK_LEFT_PC34, "panel chest route");
    /* C140/C145/C011/C141 are narrow C101-overlapping controls and must
     * precede the broad parent route. */
    assert_route(DM1_V1_MOUSE_LIST_INVENTORY_PC34, 45, 81,
                 DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, 101,
                 DM1_V1_MOUSE_MASK_LEFT_PC34, "inventory panel fallback route");
    assert_route(DM1_V1_MOUSE_LIST_PARTY_RESTING_PC34, 0, 146,
                 DM1_V1_MOUSE_SPACE_SCREEN_PC34, 7,
                 DM1_V1_MOUSE_MASK_LEFT_PC34, "party resting viewport wake route");
    assert_route(DM1_V1_MOUSE_LIST_PARTY_RESTING_PC34, 1, 146,
                 DM1_V1_MOUSE_SPACE_SCREEN_PC34, 2,
                 DM1_V1_MOUSE_MASK_RIGHT_PC34, "party resting screen wake route");

    command = DM1_V1_MouseRoutes_CommandForPointPc34Compat(
        DM1_V1_MOUSE_LIST_INVENTORY_PC34, 8 + 6, 33 + 53,
        DM1_V1_MOUSE_MASK_LEFT_PC34, 8, 33, test_zone_rect, NULL, &space, &zone);
    ASSERT_EQ(command, 28, "viewport source slot hit command");
    ASSERT_EQ(space, DM1_V1_MOUSE_SPACE_VIEWPORT_PC34, "viewport source slot space");
    ASSERT_EQ(zone, 507, "viewport source slot zone");

    command = DM1_V1_MouseRoutes_CommandForPointPc34Compat(
        DM1_V1_MOUSE_LIST_INVENTORY_PC34, 8 + 56, 33 + 13,
        DM1_V1_MOUSE_MASK_LEFT_PC34, 8, 33, test_zone_rect, NULL, &space, &zone);
    ASSERT_EQ(command, 70, "mouth route command");
    ASSERT_EQ(zone, 545, "mouth route zone");

    command = DM1_V1_MouseRoutes_CommandForPointPc34Compat(
        DM1_V1_MOUSE_LIST_INVENTORY_PC34, 8 + 117, 33 + 59,
        DM1_V1_MOUSE_MASK_LEFT_PC34, 8, 33, test_zone_rect, NULL, &space, &zone);
    ASSERT_EQ(command, 81, "inventory route enters the C081 panel parent");
    ASSERT_EQ(zone, 101, "inventory panel parent zone");

    command = DM1_V1_MouseRoutes_CommandForPointPc34Compat(
        DM1_V1_MOUSE_LIST_PANEL_CHEST_PC34, 8 + 117, 33 + 59,
        DM1_V1_MOUSE_MASK_LEFT_PC34, 8, 33, test_zone_rect, NULL, &space, &zone);
    ASSERT_EQ(command, 58, "panel chest child route resolves C058");
    ASSERT_EQ(zone, 537, "panel chest child zone");

    command = DM1_V1_MouseRoutes_CommandForPointPc34Compat(
        DM1_V1_MOUSE_LIST_INVENTORY_PC34, 5, 5,
        DM1_V1_MOUSE_MASK_RIGHT_PC34, 8, 33, test_zone_rect, NULL, &space, &zone);
    ASSERT_EQ(command, 11, "right button inventory close route");
    ASSERT_EQ(space, DM1_V1_MOUSE_SPACE_SCREEN_PC34, "right close screen space");
    ASSERT_EQ(zone, 2, "right close screen zone");

    command = DM1_V1_MouseRoutes_CommandForPointPc34Compat(
        DM1_V1_MOUSE_LIST_INVENTORY_PC34, 8 + 6, 33 + 53,
        0, 8, 33, test_zone_rect, NULL, &space, &zone);
    ASSERT_EQ(command, 0, "empty button mask misses viewport slot");
    ASSERT_EQ(space, DM1_V1_MOUSE_SPACE_NONE_PC34, "miss resets space");
    ASSERT_EQ(zone, 0, "miss resets zone");

    command = DM1_V1_MouseRoutes_CommandForScreenPointPc34Compat(
        DM1_V1_MOUSE_LIST_PARTY_RESTING_PC34, 0, 33,
        DM1_V1_MOUSE_MASK_LEFT_PC34, &space, &zone);
    ASSERT_EQ(command, 146, "party resting viewport left click wakes");
    ASSERT_EQ(space, DM1_V1_MOUSE_SPACE_SCREEN_PC34,
              "party resting left wake preserves C007 screen coordinates");
    ASSERT_EQ(zone, 7, "party resting left wake uses source viewport zone");

    command = DM1_V1_MouseRoutes_CommandForScreenPointPc34Compat(
        DM1_V1_MOUSE_LIST_PARTY_RESTING_PC34, 319, 199,
        DM1_V1_MOUSE_MASK_RIGHT_PC34, &space, &zone);
    ASSERT_EQ(command, 146, "party resting screen right click wakes at inclusive edge");
    ASSERT_EQ(space, DM1_V1_MOUSE_SPACE_SCREEN_PC34,
              "party resting right wake uses source screen space");
    ASSERT_EQ(zone, 2, "party resting right wake uses source screen zone");

    command = DM1_V1_MouseRoutes_CommandForScreenPointPc34Compat(
        DM1_V1_MOUSE_LIST_PARTY_RESTING_PC34, 0, 32,
        DM1_V1_MOUSE_MASK_LEFT_PC34, &space, &zone);
    ASSERT_EQ(command, 0, "party resting left click outside viewport misses");
    ASSERT_EQ(space, DM1_V1_MOUSE_SPACE_NONE_PC34,
              "party resting outside viewport clears coordinate space");

    if (g_fail) {
        fprintf(stderr, "dm1_v1_mouse_routes_pc34_compat: %d failed, %d passed\n",
                g_fail, g_pass);
        return 1;
    }
    printf("dm1_v1_mouse_routes_pc34_compat: %d passed\n", g_pass);
    return 0;
}
