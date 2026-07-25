#include "csb_v2_hud_runtime.h"

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

static void test_init_shutdown(void) {
    csb_v2_hud_runtime_init();
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud != NULL);
    assert(hud->visible == true);
    assert(hud->opacity == 255);
    csb_v2_hud_runtime_shutdown();
    printf("  init_shutdown OK\n");
}

static void test_double_init(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_init();
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud != NULL);
    csb_v2_hud_runtime_shutdown();
    printf("  double_init OK\n");
}

static void test_lazy_init(void) {
    csb_v2_hud_runtime_shutdown();
    csb_v2_hud_runtime_set_direction(2);
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud != NULL);
    assert(hud->compass.direction == 2);
    csb_v2_hud_runtime_shutdown();
    printf("  lazy_init OK\n");
}

static void test_set_party_gold(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_set_party_gold(999);
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud->gold.party_gold == 999);
    csb_v2_hud_runtime_shutdown();
    printf("  set_party_gold OK\n");
}

static void test_set_direction(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_set_direction(3);
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud->compass.direction == 3);
    csb_v2_hud_runtime_shutdown();
    printf("  set_direction OK\n");
}

static void test_set_level(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_set_level(5, 12);
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud->depth.current_level == 5);
    assert(hud->depth.max_level == 12);
    csb_v2_hud_runtime_shutdown();
    printf("  set_level OK\n");
}

static void test_set_champion(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_set_champion(1, 80, 60, 40, true, true);
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud->champion_bars[1].hp_pct == 80);
    assert(hud->champion_bars[1].stamina_pct == 60);
    assert(hud->champion_bars[1].mana_pct == 40);
    assert(hud->champion_bars[1].leader == true);
    assert(hud->champion_bars[1].spell_ready == true);
    csb_v2_hud_runtime_shutdown();
    printf("  set_champion OK\n");
}

static void test_action_and_clear(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_set_action_active(CSB_V2_ACTION_CAST);
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud->action_strip.icons[CSB_V2_ACTION_CAST].active == true);
    assert(hud->action_strip.icons[CSB_V2_ACTION_ATTACK].active == false);
    csb_v2_hud_runtime_clear_action();
    assert(hud->action_strip.icons[CSB_V2_ACTION_CAST].active == false);
    csb_v2_hud_runtime_shutdown();
    printf("  action_and_clear OK\n");
}

static void test_hit_flash(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_trigger_hit_flash();
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud->hit_flash_active == true);
    assert(hud->hit_flash_timer == 6);
    csb_v2_hud_runtime_shutdown();
    printf("  hit_flash OK\n");
}

static void test_chaos(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_set_chaos_active(true, 2);
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud->chaos.chaos_active == true);
    assert(hud->chaos.power_rune_count == 2);
    csb_v2_hud_runtime_shutdown();
    printf("  chaos OK\n");
}

static void test_toggle_and_opacity(void) {
    csb_v2_hud_runtime_init();
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud->visible == true);
    csb_v2_hud_runtime_toggle();
    assert(hud->visible == false);
    csb_v2_hud_runtime_toggle();
    assert(hud->visible == true);
    csb_v2_hud_runtime_set_opacity(128);
    assert(hud->opacity == 128);
    csb_v2_hud_runtime_shutdown();
    printf("  toggle_and_opacity OK\n");
}

static void test_apply_frame(void) {
    csb_v2_hud_runtime_init();
    CSB_V2_HudRuntimeFrame frame;
    memset(&frame, 0, sizeof(frame));
    frame.direction = 2;
    frame.current_level = 3;
    frame.max_level = 8;
    frame.party_gold = 500;
    frame.champion_count = 4;
    frame.leader_index = 1;
    frame.hp_pct[0] = 100; frame.stamina_pct[0] = 90; frame.mana_pct[0] = 80;
    frame.hp_pct[1] = 70;  frame.stamina_pct[1] = 60; frame.mana_pct[1] = 50;
    frame.hp_pct[2] = 40;  frame.stamina_pct[2] = 30; frame.mana_pct[2] = 20;
    frame.hp_pct[3] = 10;  frame.stamina_pct[3] = 5;  frame.mana_pct[3] = 0;
    frame.chaos_active = 1;
    frame.power_rune_count = 3;

    csb_v2_hud_runtime_apply_frame(&frame);
    CSB_V2_HudOverlay *hud = csb_v2_hud_runtime_get_hud();
    (void)hud;
    assert(hud->compass.direction == 2);
    assert(hud->depth.current_level == 3);
    assert(hud->depth.max_level == 8);
    assert(hud->gold.party_gold == 500);
    assert(hud->champion_bars[0].hp_pct == 100);
    assert(hud->champion_bars[1].leader == true);
    assert(hud->champion_bars[3].mana_pct == 0);
    assert(hud->chaos.chaos_active == true);
    assert(hud->chaos.power_rune_count == 3);
    csb_v2_hud_runtime_shutdown();
    printf("  apply_frame OK\n");
}

static void test_apply_frame_null(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_apply_frame(NULL);
    csb_v2_hud_runtime_shutdown();
    printf("  apply_frame_null OK\n");
}

static void test_render_no_gate(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_set_champion(0, 100, 100, 100, true, false);
    csb_v2_hud_runtime_set_party_gold(42);
    clear_fb();
    csb_v2_hud_runtime_render(g_fb, FB_W, FB_H);
    assert(fb_has_nonzero());
    csb_v2_hud_runtime_shutdown();
    printf("  render_no_gate OK\n");
}

static void test_render_with_gate_allowed(void) {
    csb_v2_hud_runtime_init();
    CSB_V2_PhaseGateConfig config;
    memset(&config, 0, sizeof(config));
    config.v2PresentationEnabled = 1;
    csb_v2_hud_runtime_set_gate_config(&config);
    csb_v2_hud_runtime_set_champion(0, 100, 100, 100, true, false);
    clear_fb();
    csb_v2_hud_runtime_render(g_fb, FB_W, FB_H);
    assert(fb_has_nonzero());
    csb_v2_hud_runtime_set_gate_config(NULL);
    csb_v2_hud_runtime_shutdown();
    printf("  render_with_gate_allowed OK\n");
}

static void test_render_with_gate_blocked(void) {
    csb_v2_hud_runtime_init();
    CSB_V2_PhaseGateConfig config;
    memset(&config, 0, sizeof(config));
    config.v2PresentationEnabled = 0;
    csb_v2_hud_runtime_set_gate_config(&config);
    csb_v2_hud_runtime_set_champion(0, 100, 100, 100, true, false);
    clear_fb();
    csb_v2_hud_runtime_render(g_fb, FB_W, FB_H);
    assert(!fb_has_nonzero());
    csb_v2_hud_runtime_set_gate_config(NULL);
    csb_v2_hud_runtime_shutdown();
    printf("  render_with_gate_blocked OK\n");
}

static void test_render_null_safety(void) {
    csb_v2_hud_runtime_init();
    csb_v2_hud_runtime_render(NULL, FB_W, FB_H);
    csb_v2_hud_runtime_render(g_fb, 0, FB_H);
    csb_v2_hud_runtime_render(g_fb, FB_W, 0);
    csb_v2_hud_runtime_shutdown();
    printf("  render_null_safety OK\n");
}

static void test_source_evidence(void) {
    const char *ev = csb_v2_hud_runtime_source_evidence();
    (void)ev;
    assert(ev != NULL);
    assert(strstr(ev, "CSB V2") != NULL);
    assert(strstr(ev, "F0354") != NULL);
    assert(strstr(ev, "presentation") != NULL);
    printf("  source_evidence OK\n");
}

int main(void) {
    printf("test_csb_v2_hud_runtime:\n");
    test_init_shutdown();
    test_double_init();
    test_lazy_init();
    test_set_party_gold();
    test_set_direction();
    test_set_level();
    test_set_champion();
    test_action_and_clear();
    test_hit_flash();
    test_chaos();
    test_toggle_and_opacity();
    test_apply_frame();
    test_apply_frame_null();
    test_render_no_gate();
    test_render_with_gate_allowed();
    test_render_with_gate_blocked();
    test_render_null_safety();
    test_source_evidence();
    printf("  ALL PASSED\n");
    return 0;
}
