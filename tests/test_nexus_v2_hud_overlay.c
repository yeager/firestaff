/* Nexus V2 Phase 3 HUD Overlay — smoke test
 * Tests that the HUD overlay module initialises, sets parameters,
 * renders into a framebuffer, and produces deterministic output
 * without any game data or SDL.
 */

#include "nexus_v2_hud_overlay.h"
#include <stdio.h>
#include <string.h>

#define FB_W 320
#define FB_H 200

static int s_tests_passed = 0;
static int s_tests_failed = 0;

static void check(const char *name, int cond) {
    if (cond) {
        printf("  PASS: %s\n", name);
        s_tests_passed++;
    } else {
        printf("  FAIL: %s\n", name);
        s_tests_failed++;
    }
}

int main(void) {
    printf("=== Nexus V2 Phase 3 HUD Overlay smoke test ===\n");

    /* ── Init/reset ─────────────────────────────────────────────── */
    Nexus_V2_HudOverlay h;
    nexus_v2_hud_init(&h);
    check("init: visible default", h.visible == true);
    check("init: opacity 255", h.opacity == 255);
    check("init: compass direction 0", h.compass.direction == 0);
    check("init: depth current 1", h.depth.current_level == 1);
    check("init: gold visible", h.gold.visible == true);
    check("init: gold party_gold 0", h.gold.party_gold == 0);
    check("init: action strip visible", h.action_strip.visible == true);
    check("init: party indicator true", h.panel.party_indicator == true);
    check("init: menu indicator false", h.panel.menu_indicator == false);
    check("init: map indicator false", h.panel.map_indicator == false);

    nexus_v2_hud_reset(&h);
    check("reset: same as init", h.opacity == 255 && h.visible == true);

    /* ── Parameter setters ───────────────────────────────────────── */
    nexus_v2_hud_set_direction(&h, 2);
    check("set_direction 2 → direction==2", h.compass.direction == 2);

    nexus_v2_hud_set_direction(&h, -1);
    check("set_direction -1 → clamped 0", h.compass.direction == 0);

    nexus_v2_hud_set_direction(&h, 99);
    check("set_direction 99 → clamped 3", h.compass.direction == 3);

    nexus_v2_hud_set_level(&h, 5, 8);
    check("set_level(5,8) → cur=5", h.depth.current_level == 5);
    check("set_level(5,8) → max=8", h.depth.max_level == 8);

    nexus_v2_hud_set_level(&h, -1, 0);
    check("set_level(-1,0) → cur=0", h.depth.current_level == 0);
    check("set_level(-1,0) → max=1", h.depth.max_level == 1);

    nexus_v2_hud_set_gold(&h, 1234);
    check("set_gold(1234)", h.gold.party_gold == 1234);

    nexus_v2_hud_set_champion_bar(&h, 2, 75, 50, 100, true, false);
    check("set_champion_bar idx=2", h.champion_bars[2].champion_index == 2);
    check("set_champion_bar hp=75", h.champion_bars[2].hp_pct == 75);
    check("set_champion_bar stamina=50", h.champion_bars[2].stamina_pct == 50);
    check("set_champion_bar mana=100", h.champion_bars[2].mana_pct == 100);
    check("set_champion_bar leader=true", h.champion_bars[2].leader == true);
    check("set_champion_bar spell_ready=false", h.champion_bars[2].spell_ready == false);

    nexus_v2_hud_set_champion_bar(&h, 0, 20, 30, 40, false, true);
    check("set_champion_bar idx=0", h.champion_bars[0].champion_index == 0);
    check("set_champion_bar hp=20", h.champion_bars[0].hp_pct == 20);

    /* Out-of-range champion index */
    nexus_v2_hud_set_champion_bar(&h, -1, 50, 50, 50, false, false);
    nexus_v2_hud_set_champion_bar(&h, 99, 50, 50, 50, false, false);
    check("set_champion_bar idx=-1: no crash", 1);
    check("set_champion_bar idx=99: no crash", 1);

    nexus_v2_hud_set_action_active(&h, NEXUS_V2_ACTION_CAST);
    check("set_action_active(CST) → CST active",
        h.action_strip.icons[NEXUS_V2_ACTION_CAST].active == true);
    check("set_action_active(CST) → others inactive",
        h.action_strip.icons[NEXUS_V2_ACTION_ATTACK].active == false &&
        h.action_strip.icons[NEXUS_V2_ACTION_MOVE].active == false);

    nexus_v2_hud_toggle(&h);
    check("toggle → visible=false", h.visible == false);
    nexus_v2_hud_toggle(&h);
    check("toggle → visible=true", h.visible == true);

    nexus_v2_hud_set_opacity(&h, 128);
    check("set_opacity 128", h.opacity == 128);

    nexus_v2_hud_trigger_hit_flash(&h);
    check("trigger_hit_flash → active+6", h.hit_flash_active == true && h.hit_flash_timer == 6);

    /* ── Render ──────────────────────────────────────────────────── */
    uint8_t fb[FB_W * FB_H];
    memset(fb, 0, sizeof(fb));

    nexus_v2_hud_render(&h, fb, FB_W, FB_H);
    check("render: no crash", 1);

    /* Check that some pixels were written (compass area: x=8..24, y=8..24) */
    int compass_pixels = 0;
    for (int y = 8; y < 24; y++) {
        for (int x = 8; x < 24; x++) {
            if (fb[y * FB_W + x] != 0) compass_pixels++;
        }
    }
    check("compass: some pixels written", compass_pixels > 0);

    /* Check gold counter area (bottom-right) was written */
    int gold_pixels = 0;
    for (int y = NEXUS_ACTION_STRIP_Y; y < 200; y++) {
        for (int x = FB_W - 60; x < FB_W; x++) {
            if (fb[y * FB_W + x] != 0) gold_pixels++;
        }
    }
    check("gold counter: some pixels written", gold_pixels > 0);

    /* Check depth indicator (top-right corner) was written */
    int depth_pixels = 0;
    for (int y = 4; y < 16; y++) {
        for (int x = FB_W - 60; x < FB_W; x++) {
            if (fb[y * FB_W + x] != 0) depth_pixels++;
        }
    }
    check("depth: some pixels written", depth_pixels > 0);

    /* Hidden render (opacity=0) → no pixels */
    memset(fb, 0, sizeof(fb));
    h.opacity = 0;
    nexus_v2_hud_render(&h, fb, FB_W, FB_H);
    int zero_pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) zero_pixels++;
    }
    check("hidden render (opacity=0): no pixels", zero_pixels == 0);

    /* Invisible render → no pixels */
    memset(fb, 0, sizeof(fb));
    h.visible = false;
    h.opacity = 255;
    nexus_v2_hud_render(&h, fb, FB_W, FB_H);
    zero_pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) zero_pixels++;
    }
    check("invisible render: no pixels", zero_pixels == 0);

    /* ── Hit flash decay ─────────────────────────────────────────── */
    h.visible = true;
    h.opacity = 255;
    h.hit_flash_active = true;
    h.hit_flash_timer = 3;
    nexus_v2_hud_render(&h, fb, FB_W, FB_H);
    check("hit_flash_timer decrements on render", h.hit_flash_timer == 2);
    nexus_v2_hud_render(&h, fb, FB_W, FB_H);
    check("hit_flash_timer 1 after 2nd render", h.hit_flash_timer == 1);
    nexus_v2_hud_render(&h, fb, FB_W, FB_H);
    check("hit_flash_timer 0 after 3rd render → inactive", h.hit_flash_timer == 0 && h.hit_flash_active == false);

    /* ── Null safety ─────────────────────────────────────────────── */
    nexus_v2_hud_set_direction(NULL, 1);
    nexus_v2_hud_set_level(NULL, 1, 10);
    nexus_v2_hud_set_gold(NULL, 0);
    nexus_v2_hud_set_champion_bar(NULL, 0, 50, 50, 50, false, false);
    nexus_v2_hud_set_action_active(NULL, NEXUS_V2_ACTION_ATTACK);
    nexus_v2_hud_trigger_hit_flash(NULL);
    nexus_v2_hud_toggle(NULL);
    nexus_v2_hud_set_opacity(NULL, 255);
    nexus_v2_hud_render(NULL, fb, FB_W, FB_H);
    check("null pointers: no crash", 1);

    /* ── Source evidence ─────────────────────────────────────────── */
    const char *ev = nexus_v2_hud_source_evidence();
    check("hud source_evidence not NULL", ev != NULL && strlen(ev) > 10);

    /* ── Result ─────────────────────────────────────────────────── */
    printf("\n=== Results: %d passed, %d failed ===\n", s_tests_passed, s_tests_failed);
    return s_tests_failed > 0 ? 1 : 0;
}
