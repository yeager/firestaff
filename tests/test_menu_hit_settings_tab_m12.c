/*
 * test_menu_hit_settings_tab_m12.c
 *
 * M12 settings tab strip mouse hit-test gate.  The settings
 * view in m12_draw_tabbed_settings_view draws a tab strip at
 * y=52 with M12_SETTINGS_TAB_COUNT (5) equally-sized tabs.
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
    M12_MouseHit hit;
    int tabW = (1920 - 2 * (1920 / 30)) / M12_SETTINGS_TAB_COUNT;

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

    /* Clicking tab 4 (last tab, M12_SETTINGS_TAB_ACCESSIBILITY)
     * sets settingsTabIndex=4. */
    (void)M12_ModernMenu_ApplyHit(&state,
        (M12_MouseHit){ M12_HIT_SETTINGS_TAB,
                        M12_SETTINGS_TAB_ACCESSIBILITY, 0 });
    CHECK(state.settingsTabIndex == M12_SETTINGS_TAB_ACCESSIBILITY,
          "M12_HIT_SETTINGS_TAB(ACCESSIBILITY) sets tab to 4");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
