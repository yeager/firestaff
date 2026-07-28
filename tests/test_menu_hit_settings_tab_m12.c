/*
 * test_menu_hit_settings_tab_m12.c
 *
 * M12 settings tab strip mouse hit-test gate.  The settings
 * view in m12_draw_tabbed_settings_view draws a tab strip at
 * y=52 with M12_SETTINGS_TAB_COUNT equally-sized tabs.
 * Before the fix in d2c5e7f8 clicking the tab strip fell
 * through to M12_HIT_NONE; this test verifies the new
 * M12_HIT_SETTINGS_TAB kind is produced by
 * M12_ModernMenu_HitTest for clicks in the tab-strip rect and
 * that M12_ModernMenu_ApplyHit routes to a tab switch.
 */
#include "menu_hit_m12.h"
#include "menu_startup_m12.h"
#include "config_m12.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    M12_StartupMenuState state;
    M12_StartupTextInputHostReceipt textInputReceipt;
    M12_MouseHit hit;
    int tabW = (1920 - 2 * (1920 / 30)) / M12_SETTINGS_TAB_COUNT;
    const int settingsRowXRight = 96 + 36 + (1920 - 2 * 96 - 2 * 36) - 20;
    const int settingsGameLeftColumnXRight = 96 + 36 +
        ((1920 - 2 * 96 - 2 * 36 - 24) / 2) - 20;
    const int settingsRowY0 = 260 + 36;
    const int settingsRowYOffset[] = { 0, 70, 140, 210, 280 };

    printf("=== M12 settings tab hit-test (v2.7.15) ===\n");

    /* The tab strip is at y=52, height 22.  Margin is fw/30 = 64. */
    memset(&state, 0, sizeof(state));
    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsTabIndex = 0;

    /* Click each tab 0..COUNT-1 in the middle of its rect. */
    for (int i = 0; i < M12_SETTINGS_TAB_COUNT; ++i) {
        int cx = 64 + i * tabW + (tabW - 2) / 2;
        int cy = 52 + 11;
        hit = M12_ModernMenu_HitTest(&state, cx, cy);
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "click on tab %d at (%d, %d) -> M12_HIT_SETTINGS_TAB",
                 i, cx, cy);
        CHECK(hit.kind == M12_HIT_SETTINGS_TAB, msg);
        snprintf(msg, sizeof(msg),
                 "click on tab %d -> hit.index == %d", i, i);
        CHECK(hit.index == i, msg);
    }

    /* Click above the tab strip: should be NONE (no row up there). */
    hit = M12_ModernMenu_HitTest(&state, 500, 10);
    CHECK(hit.kind == M12_HIT_NONE, "click above tab strip -> M12_HIT_NONE");

    /* Click below the tab strip on the content panel: M12_HIT_SETTINGS_ROW. */
    hit = M12_ModernMenu_HitTest(&state, 500, 320);
    /* The content panel may also produce M12_HIT_NONE for the empty
     * area above the first row.  We just verify it is not SETTINGS_TAB. */
    CHECK(hit.kind != M12_HIT_SETTINGS_TAB,
          "click on content panel -> not SETTINGS_TAB");

    /* Every row published by the shared settings model must have a matching
     * clickable modern-menu rectangle. This keeps persistence, rendering and
     * mouse navigation on one row catalogue. */
    for (int tab = 0; tab < M12_SETTINGS_TAB_COUNT; ++tab) {
        const int *rows;
        int rowCount = 0;
        state.settingsTabIndex = tab;
        rows = M12_StartupMenu_GetSettingsRowsForTab(tab, &rowCount);
        CHECK(rows != NULL && rowCount > 0 && rowCount <= 11,
              "settings tab publishes a bounded clickable row catalogue");
        for (int row = 0; rows && row < rowCount; ++row) {
            const int useTwoColumns = rowCount > 8;
            const int contentW = 1920 - 2 * 96 - 2 * 36;
            const int columnW = useTwoColumns ? (contentW - 24) / 2 : contentW;
            const int rowsPerColumn = useTwoColumns ? (rowCount + 1) / 2 : rowCount;
            const int column = useTwoColumns ? row / rowsPerColumn : 0;
            const int rowInColumn = useTwoColumns ? row % rowsPerColumn : row;
            const int settingsRowXRight = 96 + 36 + column * (columnW + 24) +
                columnW - 20;
            hit = M12_ModernMenu_HitTest(
                &state, settingsRowXRight,
                settingsRowY0 + rowInColumn * 70 + 25);
            CHECK((hit.kind == M12_HIT_SETTINGS_ROW ||
                   hit.kind == M12_HIT_SETTINGS_CYCLE) &&
                      hit.index == rows[row],
                  "published settings row resolves to its mouse hit target");
        }
    }

    /* M12_ModernMenu_ApplyHit routes M12_HIT_SETTINGS_TAB to a
     * tab switch.  Initial settingsTabIndex=0; clicking tab 2
     * (M12_SETTINGS_TAB_CONTROLS) should set it to 2. */
    state.settingsTabIndex = 0;
    (void)M12_ModernMenu_ApplyHit(&state,
        (M12_MouseHit){ M12_HIT_SETTINGS_TAB, 2, 0 });
    CHECK(state.settingsTabIndex == 2,
          "M12_HIT_SETTINGS_TAB(2) routes to settingsTabIndex=2");

    /* Clicking the current tab is a no-op (no change). */
    (void)M12_ModernMenu_ApplyHit(&state,
        (M12_MouseHit){ M12_HIT_SETTINGS_TAB, 2, 0 });
    CHECK(state.settingsTabIndex == 2,
          "clicking current tab keeps settingsTabIndex=2");

    /* Clicking tab 0 from tab 2 routes back to 0. */
    (void)M12_ModernMenu_ApplyHit(&state,
        (M12_MouseHit){ M12_HIT_SETTINGS_TAB, 0, 0 });
    CHECK(state.settingsTabIndex == 0,
          "M12_HIT_SETTINGS_TAB(0) routes to settingsTabIndex=0");

    /* Clicking accessibility and online tabs should route to their
     * public enum values. */
    (void)M12_ModernMenu_ApplyHit(&state,
        (M12_MouseHit){ M12_HIT_SETTINGS_TAB,
                        M12_SETTINGS_TAB_ACCESSIBILITY, 0 });
    CHECK(state.settingsTabIndex == M12_SETTINGS_TAB_ACCESSIBILITY,
          "M12_HIT_SETTINGS_TAB(ACCESSIBILITY) sets accessibility tab");

    (void)M12_ModernMenu_ApplyHit(&state,
        (M12_MouseHit){ M12_HIT_SETTINGS_TAB,
                        M12_SETTINGS_TAB_ONLINE, 0 });
    CHECK(state.settingsTabIndex == M12_SETTINGS_TAB_ONLINE,
          "M12_HIT_SETTINGS_TAB(ONLINE) sets RetroAchievements settings tab");

    memset(&state, 0, sizeof(state));
    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsTabIndex = M12_SETTINGS_TAB_ONLINE;

    hit = M12_ModernMenu_HitTest(&state,
                                 settingsRowXRight,
                                 settingsRowY0 + settingsRowYOffset[0] + 25);
    CHECK(hit.kind == M12_HIT_SETTINGS_CYCLE,
          "RetroAchievements visible row -> cycle hit");
    CHECK(hit.index == M12_STARTUP_SETTINGS_ROW_RETROACHIEVEMENTS,
          "RetroAchievements visible row maps to M12_SETTINGS_ROW_RETROACHIEVEMENTS");
    (void)M12_ModernMenu_ApplyHit(&state, hit);
    CHECK(state.settings.retroAchievementsEnabled == 1,
          "RetroAchievements click toggles enabled setting");

    state.view = M12_MENU_VIEW_SETTINGS;
    state.settingsTabIndex = M12_SETTINGS_TAB_ONLINE;
    state.settingsSelectedIndex = M12_STARTUP_SETTINGS_ROW_DATA_STATUS;
    state.settings.retroAchievementsEnabled = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    CHECK(state.settingsSelectedIndex == M12_STARTUP_SETTINGS_ROW_RETROACHIEVEMENTS,
          "ONLINE settings keyboard DOWN reaches RetroAchievements");
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    CHECK(state.settings.retroAchievementsEnabled == 1,
          "settings keyboard ACCEPT toggles RetroAchievements");

    hit = M12_ModernMenu_HitTest(&state,
                                 settingsRowXRight,
                                 settingsRowY0 + settingsRowYOffset[1] + 25);
    CHECK(hit.kind == M12_HIT_SETTINGS_CYCLE,
          "RA Hardcore visible row -> cycle hit");
    CHECK(hit.index == M12_STARTUP_SETTINGS_ROW_RA_HARDCORE,
          "RA Hardcore visible row maps to M12_SETTINGS_ROW_RA_HARDCORE");
    state.settings.retroAchievementsHardcore = 0;
    (void)M12_ModernMenu_ApplyHit(&state, hit);
    CHECK(state.settings.retroAchievementsHardcore == 1,
          "RA Hardcore click toggles hardcore setting");

    hit = M12_ModernMenu_HitTest(&state,
                                 settingsRowXRight,
                                 settingsRowY0 + settingsRowYOffset[2] + 25);
    CHECK(hit.kind == M12_HIT_SETTINGS_CYCLE,
          "RA Username visible row -> cycle hit");
    CHECK(hit.index == M12_STARTUP_SETTINGS_ROW_RA_USERNAME,
          "RA Username visible row maps to M12_SETTINGS_ROW_RA_USERNAME");
    (void)M12_ModernMenu_ApplyHit(&state, hit);
    CHECK(M12_StartupMenu_TextEditActive(&state),
          "RA Username click starts text edit");
    CHECK(M12_StartupMenu_TextInputHostReceipt(&state, 0, &textInputReceipt) == 1 &&
          textInputReceipt.startTextInput &&
          textInputReceipt.physicalKeyboardValid &&
          textInputReceipt.onscreenKeyboardValid,
          "RA Username requests host text input for keyboard and screen keyboard");
    CHECK(M12_StartupMenu_ConsumeTextInput(&state, "yeager") == 1,
          "RA Username accepts text input");
    CHECK(M12_StartupMenu_TextEditCommit(&state) == 1,
          "RA Username commit succeeds");
    CHECK(M12_StartupMenu_TextInputHostReceipt(&state, 1, &textInputReceipt) == 1 &&
          textInputReceipt.stopTextInput,
          "RA Username commit requests host text input stop");
    CHECK(strcmp(state.settings.retroAchievementsUsername, "yeager") == 0,
          "RA Username stored in menu settings");

    hit = M12_ModernMenu_HitTest(&state,
                                 settingsRowXRight,
                                 settingsRowY0 + settingsRowYOffset[3] + 25);
    CHECK(hit.kind == M12_HIT_SETTINGS_CYCLE,
          "RA Token visible row -> cycle hit");
    CHECK(hit.index == M12_STARTUP_SETTINGS_ROW_RA_TOKEN,
          "RA Token visible row maps to M12_SETTINGS_ROW_RA_TOKEN");
    (void)M12_ModernMenu_ApplyHit(&state, hit);
    CHECK(M12_StartupMenu_TextEditActive(&state),
          "RA Token click starts text edit");
    CHECK(M12_StartupMenu_TextInputHostReceipt(&state, 0, &textInputReceipt) == 1 &&
          textInputReceipt.startTextInput &&
          textInputReceipt.physicalKeyboardValid &&
          textInputReceipt.onscreenKeyboardValid,
          "RA Token requests host text input for keyboard and screen keyboard");
    CHECK(M12_StartupMenu_ConsumeTextInput(&state, "secret-API_123") == 1,
          "RA Token accepts keyboard token characters");
    CHECK(strcmp(M12_StartupMenu_GetSettingsValue(&state,
                                                  M12_STARTUP_SETTINGS_ROW_RA_TOKEN),
                 "EDITING (14)") == 0,
          "RA Token edit gives masked live input feedback");
    CHECK(M12_StartupMenu_TextEditBackspace(&state) == 1,
          "RA Token supports backspace");
    CHECK(M12_StartupMenu_TextEditCommit(&state) == 1,
          "RA Token commit succeeds");
    CHECK(strcmp(state.settings.retroAchievementsToken, "secret-API_12") == 0,
          "RA Token stored without displaying raw token");
    CHECK(!M12_StartupMenu_TextEditActive(&state),
          "RA Token commit exits text edit");

    (void)M12_ModernMenu_ApplyHit(&state, hit);
    CHECK(M12_StartupMenu_TextEditActive(&state),
          "RA Token re-click starts second text edit");
    CHECK(M12_StartupMenu_ConsumeTextInput(&state,
                                           "0123456789abcdef0123456789abcdef"
                                           "0123456789abcdef0123456789abcdef"
                                           "0123456789abcdef0123456789abcdef"
                                           "0123456789abcdef0123456789abcdef") == 1,
          "RA Token input accepts long paste up to row capacity");
    CHECK(M12_StartupMenu_TextEditCommit(&state) == 1,
          "RA Token long paste commit succeeds");
    CHECK(strlen(state.settings.retroAchievementsToken) == 127,
          "RA Token edit clamps to persisted token capacity");

    (void)M12_ModernMenu_ApplyHit(&state, hit);
    CHECK(M12_StartupMenu_TextEditActive(&state),
          "RA Token third click starts cancelable edit");
    CHECK(M12_StartupMenu_ConsumeTextInput(&state, "SHOULD_NOT_PERSIST") == 1,
          "RA Token cancel edit accepts temporary text");
    CHECK(M12_StartupMenu_TextEditCancel(&state) == 1,
          "RA Token cancel succeeds");
    CHECK(strlen(state.settings.retroAchievementsToken) == 127,
          "RA Token cancel leaves previous token unchanged");

    state.settings.retroAchievementsEndpoint[0] = '\0';
    hit = M12_ModernMenu_HitTest(&state,
                                 settingsRowXRight,
                                 settingsRowY0 + settingsRowYOffset[4] + 25);
    CHECK(hit.kind == M12_HIT_SETTINGS_CYCLE,
          "RA Endpoint visible row -> cycle hit");
    CHECK(hit.index == M12_STARTUP_SETTINGS_ROW_RA_ENDPOINT,
          "RA Endpoint visible row maps to M12_SETTINGS_ROW_RA_ENDPOINT");
    (void)M12_ModernMenu_ApplyHit(&state, hit);
    CHECK(M12_StartupMenu_TextEditActive(&state),
          "RA Endpoint click starts text edit");
    CHECK(M12_StartupMenu_ConsumeTextInput(&state, "https://retroachievements.org") == 1,
          "RA Endpoint accepts URL input");
    CHECK(M12_StartupMenu_TextEditCommit(&state) == 1,
          "RA Endpoint commit succeeds");
    CHECK(strcmp(state.settings.retroAchievementsEndpoint,
                 "https://retroachievements.org") == 0,
          "RA Endpoint stored in menu settings");

    state.settingsTabIndex = M12_SETTINGS_TAB_GAME;
    hit = M12_ModernMenu_HitTest(&state,
                                 settingsGameLeftColumnXRight,
                                 settingsRowY0 + settingsRowYOffset[3] + 25);
    CHECK(hit.kind == M12_HIT_SETTINGS_CYCLE,
          "Session Timer visible row -> cycle hit");
    CHECK(hit.index == M12_STARTUP_SETTINGS_ROW_SESSION_TIMER,
          "Session Timer visible row maps to M12_SETTINGS_ROW_SESSION_TIMER");
    (void)M12_ModernMenu_ApplyHit(&state, hit);
    CHECK(state.settings.sessionTimerIndex == 1,
          "Session Timer click still cycles timer setting");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
