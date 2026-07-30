#include "csb_v2_hud_overlay_pc34.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#define FB_W 320
#define FB_H 200

static uint8_t g_fb[FB_W * FB_H];

static void clear_fb(void) {
    memset(g_fb, 0, sizeof(g_fb));
}

static __attribute__((unused)) int fb_has_nonzero(void) {
    for (int i = 0; i < FB_W * FB_H; i++) {
        if (g_fb[i] != 0) return 1;
    }
    return 0;
}

static __attribute__((unused)) int fb_pixel(int x, int y) {
    if (x < 0 || x >= FB_W || y < 0 || y >= FB_H) return -1;
    return g_fb[y * FB_W + x];
}

static void test_init_defaults(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    assert(h.visible == true);
    assert(h.opacity == 255);
    assert(h.compass.direction == 0);
    assert(h.compass.animated == true);
    assert(h.depth.current_level == 1);
    assert(h.depth.max_level == 10);
    assert(h.gold.party_gold == 0);
    assert(h.gold.visible == true);
    assert(h.stats_bar_visible == true);
    assert(h.action_strip.visible == true);
    assert(h.hit_flash_active == false);
    assert(h.hit_flash_timer == 0);
    assert(h.chaos.chaos_active == false);
    assert(h.chaos.power_rune_count == 0);
    assert(h.chaos.visible == true);
    printf("  init_defaults OK\n");
}

static void test_reset_clears(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_direction(&h, 2);
    csb_v2_hud_set_gold(&h, 500);
    csb_v2_hud_set_chaos_active(&h, true, 3);
    csb_v2_hud_reset(&h);
    assert(h.compass.direction == 0);
    assert(h.gold.party_gold == 0);
    assert(h.chaos.chaos_active == false);
    printf("  reset_clears OK\n");
}

static void test_set_direction(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    for (int d = 0; d < 4; d++) {
        csb_v2_hud_set_direction(&h, d);
        assert(h.compass.direction == d);
    }
    csb_v2_hud_set_direction(&h, -1);
    assert(h.compass.direction == 0);
    csb_v2_hud_set_direction(&h, 5);
    assert(h.compass.direction == 3);
    printf("  set_direction OK\n");
}

static void test_set_level(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_level(&h, 4, 10);
    assert(h.depth.current_level == 4);
    assert(h.depth.max_level == 10);
    csb_v2_hud_set_level(&h, -1, 0);
    assert(h.depth.current_level == 0);
    assert(h.depth.max_level == 1);
    printf("  set_level OK\n");
}

static void test_set_gold(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_gold(&h, 1234);
    assert(h.gold.party_gold == 1234);
    assert(h.gold.visible == true);
    printf("  set_gold OK\n");
}

static void test_set_champion_bar(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_champion_bar(&h, 0, 100, 80, 60, true, false);
    assert(h.champion_bars[0].hp_pct == 100);
    assert(h.champion_bars[0].stamina_pct == 80);
    assert(h.champion_bars[0].mana_pct == 60);
    assert(h.champion_bars[0].leader == true);
    assert(h.champion_bars[0].spell_ready == false);
    csb_v2_hud_set_champion_bar(&h, 3, 10, 20, 30, false, true);
    assert(h.champion_bars[3].hp_pct == 10);
    assert(h.champion_bars[3].spell_ready == true);
    csb_v2_hud_set_champion_bar(&h, -1, 50, 50, 50, false, false);
    csb_v2_hud_set_champion_bar(&h, 4, 50, 50, 50, false, false);
    printf("  set_champion_bar OK\n");
}

static void test_set_action_active(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_action_active(&h, CSB_V2_ACTION_CAST);
    assert(h.action_strip.icons[CSB_V2_ACTION_ATTACK].active == false);
    assert(h.action_strip.icons[CSB_V2_ACTION_CAST].active == true);
    assert(h.action_strip.icons[CSB_V2_ACTION_USE].active == false);
    printf("  set_action_active OK\n");
}

static void test_trigger_hit_flash(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_trigger_hit_flash(&h);
    assert(h.hit_flash_active == true);
    assert(h.hit_flash_timer == 6);
    printf("  trigger_hit_flash OK\n");
}

static void test_toggle(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    assert(h.visible == true);
    csb_v2_hud_toggle(&h);
    assert(h.visible == false);
    csb_v2_hud_toggle(&h);
    assert(h.visible == true);
    printf("  toggle OK\n");
}

static void test_set_opacity(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_opacity(&h, 128);
    assert(h.opacity == 128);
    csb_v2_hud_set_opacity(&h, 0);
    assert(h.opacity == 0);
    printf("  set_opacity OK\n");
}

static void test_set_chaos_active(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_chaos_active(&h, true, 2);
    assert(h.chaos.chaos_active == true);
    assert(h.chaos.power_rune_count == 2);
    assert(h.chaos.visible == true);
    csb_v2_hud_set_chaos_active(&h, false, -1);
    assert(h.chaos.chaos_active == false);
    assert(h.chaos.power_rune_count == 0);
    csb_v2_hud_set_chaos_active(&h, true, 5);
    assert(h.chaos.power_rune_count == 3);
    printf("  set_chaos_active OK\n");
}

static void test_render_fails_closed(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_champion_bar(&h, 0, 100, 80, 60, true, false);
    csb_v2_hud_set_champion_bar(&h, 1, 50, 50, 50, false, false);
    csb_v2_hud_set_gold(&h, 42);
    csb_v2_hud_set_action_active(&h, CSB_V2_ACTION_ATTACK);
    clear_fb();
    csb_v2_hud_render(&h, g_fb, FB_W, FB_H);
    assert(!fb_has_nonzero());
    printf("  render_fails_closed OK\n");
}

static void test_render_invisible_noop(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_toggle(&h);
    clear_fb();
    csb_v2_hud_render(&h, g_fb, FB_W, FB_H);
    assert(!fb_has_nonzero());
    printf("  render_invisible_noop OK\n");
}

static void test_render_zero_opacity_noop(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_opacity(&h, 0);
    clear_fb();
    csb_v2_hud_render(&h, g_fb, FB_W, FB_H);
    assert(!fb_has_nonzero());
    printf("  render_zero_opacity_noop OK\n");
}

static void test_render_compass_region(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_direction(&h, 0);
    clear_fb();
    csb_v2_hud_render(&h, g_fb, FB_W, FB_H);
    int compass_pixels = 0;
    (void)compass_pixels;
    for (int y = 0; y < 30; y++)
        for (int x = 0; x < 30; x++)
            if (fb_pixel(x, y) != 0) compass_pixels++;
    assert(compass_pixels == 0);
    printf("  render_compass_region_no_draw OK\n");
}

static void test_render_action_strip_region(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_action_active(&h, CSB_V2_ACTION_USE);
    clear_fb();
    csb_v2_hud_render(&h, g_fb, FB_W, FB_H);
    int action_pixels = 0;
    (void)action_pixels;
    for (int y = CSB_ACTION_STRIP_Y; y < CSB_ACTION_STRIP_Y + 28; y++)
        for (int x = CSB_ACTION_ICONS_X_START;
             x < CSB_ACTION_ICONS_X_START + CSB_V2_ACTION_COUNT * (CSB_ACTION_ICON_W + 4);
             x++)
            if (fb_pixel(x, y) != 0) action_pixels++;
    assert(action_pixels == 0);
    printf("  render_action_strip_region_no_draw OK\n");
}

static void test_render_hit_flash_state_preserved_no_draw(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_set_action_active(&h, CSB_V2_ACTION_ATTACK);
    csb_v2_hud_trigger_hit_flash(&h);
    assert(h.hit_flash_timer == 6);
    clear_fb();
    csb_v2_hud_render(&h, g_fb, FB_W, FB_H);
    assert(h.hit_flash_timer == 6);
    assert(h.hit_flash_active == true);
    for (int i = 0; i < 5; i++) {
        clear_fb();
        csb_v2_hud_render(&h, g_fb, FB_W, FB_H);
    }
    assert(h.hit_flash_timer == 6);
    assert(h.hit_flash_active == true);
    printf("  render_hit_flash_state_preserved_no_draw OK\n");
}

static void test_render_null_safety(void) {
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    csb_v2_hud_render(NULL, g_fb, FB_W, FB_H);
    csb_v2_hud_render(&h, NULL, FB_W, FB_H);
    csb_v2_hud_render(&h, g_fb, 0, FB_H);
    csb_v2_hud_render(&h, g_fb, FB_W, 0);
    csb_v2_hud_init(NULL);
    csb_v2_hud_reset(NULL);
    csb_v2_hud_set_direction(NULL, 0);
    csb_v2_hud_set_level(NULL, 1, 1);
    csb_v2_hud_set_gold(NULL, 0);
    csb_v2_hud_set_champion_bar(NULL, 0, 0, 0, 0, false, false);
    csb_v2_hud_set_action_active(NULL, CSB_V2_ACTION_ATTACK);
    csb_v2_hud_trigger_hit_flash(NULL);
    csb_v2_hud_toggle(NULL);
    csb_v2_hud_set_opacity(NULL, 0);
    csb_v2_hud_set_chaos_active(NULL, false, 0);
    printf("  render_null_safety OK\n");
}

static void test_source_evidence(void) {
    const char *ev = csb_v2_hud_source_evidence();
    (void)ev;
    assert(ev != NULL);
    assert(strstr(ev, "CSBWin") != NULL);
    assert(strstr(ev, "ReDMCSB") != NULL);
    assert(strstr(ev, "F0354") != NULL);
    assert(strstr(ev, "chaos") != NULL);
    printf("  source_evidence OK\n");
}

int main(void) {
    printf("test_csb_v2_hud_overlay_pc34:\n");
    test_init_defaults();
    test_reset_clears();
    test_set_direction();
    test_set_level();
    test_set_gold();
    test_set_champion_bar();
    test_set_action_active();
    test_trigger_hit_flash();
    test_toggle();
    test_set_opacity();
    test_set_chaos_active();
    test_render_fails_closed();
    test_render_invisible_noop();
    test_render_zero_opacity_noop();
    test_render_compass_region();
    test_render_action_strip_region();
    test_render_hit_flash_state_preserved_no_draw();
    test_render_null_safety();
    test_source_evidence();
    printf("  ALL PASSED\n");
    return 0;
}
