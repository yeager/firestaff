#include "main_loop_m11.h"
#include "dm1_v2_hud_interaction_pc34.h"
#include "touch_click_zone_matrix_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static int presented_center_for_source_axis(int sourceCoord,
                                            int sourceExtent,
                                            int presentedExtent) {
    int lo;
    int hi;
    if (sourceCoord < 0) sourceCoord = 0;
    if (sourceCoord >= sourceExtent) sourceCoord = sourceExtent - 1;
    lo = (sourceCoord * presentedExtent + sourceExtent - 1) / sourceExtent;
    hi = (((sourceCoord + 1) * presentedExtent) - 1) / sourceExtent;
    return (lo + hi) / 2;
}

static void presented_point_for_source(int sourceX,
                                       int sourceY,
                                       int* outX,
                                       int* outY) {
    *outX = presented_center_for_source_axis(sourceX, 320, 3840);
    *outY = presented_center_for_source_axis(sourceY, 200, 2160);
}

static void expect_source_mapping(int presentationMode,
                                  int sourceX,
                                  int sourceY) {
    int presentedX;
    int presentedY;
    int mappedX;
    int mappedY;
    presented_point_for_source(sourceX, sourceY, &presentedX, &presentedY);
    mappedX = presentedX;
    mappedY = presentedY;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              presentationMode, 3840, 2160, &mappedX, &mappedY) == 1);
    CHECK(mappedX == sourceX);
    CHECK(mappedY == sourceY);
}

static void expect_primary_or_secondary_zone(int presentationMode,
                                             int sourceX,
                                             int sourceY,
                                             unsigned int buttonMask,
                                             unsigned int commandId,
                                             unsigned int zoneIndex,
                                             const char* groupName) {
    int presentedX;
    int presentedY;
    int mappedX;
    int mappedY;
    TouchClickZonePc34Compat zone;
    presented_point_for_source(sourceX, sourceY, &presentedX, &presentedY);
    mappedX = presentedX;
    mappedY = presentedY;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              presentationMode, 3840, 2160, &mappedX, &mappedY) == 1);
    CHECK(mappedX == sourceX);
    CHECK(mappedY == sourceY);
    CHECK(TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(
              mappedX, mappedY, buttonMask, &zone) == 1);
    CHECK(zone.commandId == commandId);
    CHECK(zone.zoneIndex == zoneIndex);
    CHECK(strcmp(zone.groupName, groupName) == 0);
}

static void expect_v2_hud_zone(int presentationMode,
                               int sourceX,
                               int sourceY,
                               M11_V2_HudTouchKind kind,
                               unsigned int commandId,
                               unsigned int zoneIndex,
                               unsigned int championIndex,
                               const char* groupName) {
    int presentedX;
    int presentedY;
    int mappedX;
    int mappedY;
    M11_V2_HudTouchResult result;
    presented_point_for_source(sourceX, sourceY, &presentedX, &presentedY);
    mappedX = presentedX;
    mappedY = presentedY;
    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              presentationMode, 3840, 2160, &mappedX, &mappedY) == 1);
    CHECK(mappedX == sourceX);
    CHECK(mappedY == sourceY);
    CHECK(v2_hud_interaction_dispatch_screen_click(
              mappedX, mappedY, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &result) == 1);
    CHECK(result.hit == 1);
    CHECK(result.kind == kind);
    CHECK(result.commandId == commandId);
    CHECK(result.zoneIndex == zoneIndex);
    CHECK(result.championIndex == championIndex);
    CHECK(strcmp(result.groupName, groupName) == 0);
}

static void run_mode(int presentationMode) {
    int x = 3839;
    int y = 2159;

    CHECK(presentationMode == M12_PRESENTATION_V21_UPSCALED ||
          presentationMode == M12_PRESENTATION_V22_MODERN);
    expect_source_mapping(presentationMode, 0, 0);
    expect_source_mapping(presentationMode, 319, 199);

    CHECK(M11_MapPresentedGamePointToSourceForPresentation(
              presentationMode, 3840, 2160, &x, &y) == 1);
    CHECK(x == 319);
    CHECK(y == 199);

    /* ReDMCSB COMMAND.C:396-405 secondary movement table, C003/C070. */
    expect_primary_or_secondary_zone(presentationMode, 264, 126,
                                     TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                     3u, 70u, "movement.forward");

    /* ReDMCSB COMMAND.C:403 primary/secondary viewport route, C080/C007. */
    expect_primary_or_secondary_zone(presentationMode, 112, 101,
                                     TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                     80u, 7u, "viewport.dungeon");

    /* ReDMCSB COMMAND.C:394 freeze-game corner route remains reachable. */
    expect_primary_or_secondary_zone(presentationMode, 1, 199,
                                     TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                     147u, 0u, "system.freeze_game");

    /* ReDMCSB COMMAND.C:490 maps champion0 action hand to C021/C212. */
    expect_v2_hud_zone(presentationMode, 25, 11,
                       M11_V2_HUD_TOUCH_CHAMPION_HAND_PC34,
                       21u, 212u, 0u, "champion0.action_hand");

    /* ReDMCSB COMMAND.C:469 maps champion1 action icon to C117/C090. */
    expect_v2_hud_zone(presentationMode, 260, 100,
                       M11_V2_HUD_TOUCH_ACTION_ICON_PC34,
                       117u, 90u, 1u, "action.icon1");
}

int main(void) {
    CHECK(v2_hud_interaction_source_lock_ok() == 1u);

    run_mode(M12_PRESENTATION_V21_UPSCALED);
    run_mode(M12_PRESENTATION_V22_MODERN);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_4k_input_zone_mapping_pc34: ok");
    return 0;
}
