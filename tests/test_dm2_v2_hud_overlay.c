/* DM2 V2 Phase 3 HUD Overlay — smoke test
 * Tests that the HUD overlay module initialises, sets parameters,
 * renders into a framebuffer, and produces deterministic output
 * without any game data or SDL.
 */

#include "dm2_v2_hud_overlay.h"
#include "dm2_v2_interaction_feedback.h"
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
    printf("=== DM2 V2 Phase 3 HUD Overlay smoke test ===\n");

    /* ── Init/reset ─────────────────────────────────────────────── */
    DM2_V2_HudOverlay h;
    dm2_v2_hud_init(&h);
    check("init: visible default", h.visible == true);
    check("init: opacity 255", h.opacity == 255);
    check("init: compass direction 0", h.compass.direction == 0);
    check("init: depth current 1", h.depth.current_level == 1);
    check("init: gold visible", h.gold.visible == true);
    check("init: gold party_gold 0", h.gold.party_gold == 0);
    check("init: action strip visible", h.action_strip.visible == true);

    dm2_v2_hud_reset(&h);
    check("reset: same as init", h.opacity == 255 && h.visible == true);

    /* ── Parameter setters ───────────────────────────────────────── */
    dm2_v2_hud_set_direction(&h, 2);
    check("set_direction 2 → direction==2", h.compass.direction == 2);

    dm2_v2_hud_set_direction(&h, -1);
    check("set_direction -1 → clamped 0", h.compass.direction == 0);

    dm2_v2_hud_set_direction(&h, 99);
    check("set_direction 99 → clamped 3", h.compass.direction == 3);

    dm2_v2_hud_set_level(&h, 5, 8);
    check("set_level(5,8) → cur=5", h.depth.current_level == 5);
    check("set_level(5,8) → max=8", h.depth.max_level == 8);

    dm2_v2_hud_set_level(&h, -1, 0);
    check("set_level(-1,0) → cur=0", h.depth.current_level == 0);
    check("set_level(-1,0) → max=1", h.depth.max_level == 1);

    dm2_v2_hud_set_gold(&h, 1234);
    check("set_gold(1234)", h.gold.party_gold == 1234);

    dm2_v2_hud_set_champion_bar(&h, 2, 75, 50, 100, true, false);
    check("set_champion_bar idx=2", h.champion_bars[2].champion_index == 2);
    check("set_champion_bar hp=75", h.champion_bars[2].hp_pct == 75);
    check("set_champion_bar stamina=50", h.champion_bars[2].stamina_pct == 50);
    check("set_champion_bar mana=100", h.champion_bars[2].mana_pct == 100);
    check("set_champion_bar leader=true", h.champion_bars[2].leader == true);
    check("set_champion_bar spell_ready=false", h.champion_bars[2].spell_ready == false);

    dm2_v2_hud_set_champion_bar(&h, 0, 20, 30, 40, false, true);
    check("set_champion_bar idx=0", h.champion_bars[0].champion_index == 0);
    check("set_champion_bar hp=20", h.champion_bars[0].hp_pct == 20);

    /* Out-of-range champion index */
    dm2_v2_hud_set_champion_bar(&h, -1, 50, 50, 50, false, false);
    dm2_v2_hud_set_champion_bar(&h, 99, 50, 50, 50, false, false);
    check("set_champion_bar idx=-1: no crash", 1);
    check("set_champion_bar idx=99: no crash", 1);

    dm2_v2_hud_set_action_active(&h, DM2_V2_ACTION_CAST);
    check("set_action_active(CST) → CST active",
        h.action_strip.icons[DM2_V2_ACTION_CAST].active == true);
    check("set_action_active(CST) → others inactive",
        h.action_strip.icons[DM2_V2_ACTION_ATTACK].active == false &&
        h.action_strip.icons[DM2_V2_ACTION_MOVE].active == false);

    dm2_v2_hud_toggle(&h);
    check("toggle → visible=false", h.visible == false);
    dm2_v2_hud_toggle(&h);
    check("toggle → visible=true", h.visible == true);

    dm2_v2_hud_set_opacity(&h, 128);
    check("set_opacity 128", h.opacity == 128);

    dm2_v2_hud_trigger_hit_flash(&h);
    check("trigger_hit_flash → active+6", h.hit_flash_active == true && h.hit_flash_timer == 6);

    /* ── Render ──────────────────────────────────────────────────── */
    uint8_t fb[FB_W * FB_H];
    memset(fb, 0, sizeof(fb));

    dm2_v2_hud_render(&h, fb, FB_W, FB_H);
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
    for (int y = DM2_ACTION_STRIP_Y; y < 200; y++) {
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
    dm2_v2_hud_render(&h, fb, FB_W, FB_H);
    int zero_pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) zero_pixels++;
    }
    check("hidden render (opacity=0): no pixels", zero_pixels == 0);

    /* Invisible render → no pixels */
    memset(fb, 0, sizeof(fb));
    h.visible = false;
    h.opacity = 255;
    dm2_v2_hud_render(&h, fb, FB_W, FB_H);
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
    dm2_v2_hud_render(&h, fb, FB_W, FB_H);
    check("hit_flash_timer decrements on render", h.hit_flash_timer == 2);
    dm2_v2_hud_render(&h, fb, FB_W, FB_H);
    check("hit_flash_timer 1 after 2nd render", h.hit_flash_timer == 1);
    dm2_v2_hud_render(&h, fb, FB_W, FB_H);
    check("hit_flash_timer 0 after 3rd render → inactive", h.hit_flash_timer == 0 && h.hit_flash_active == false);

    /* ── Interaction feedback ────────────────────────────────────── */
    dm2_v2_interaction_set_hud_instance(&h);

    DM2_V2_InteractionResult r;
    r = dm2_v2_feedback_action_icon(1);
    check("feedback_action_icon(1): hit=1", r.hit == 1);
    check("feedback_action_icon(1): zone=ACTION_ICON", r.zone_id == DM2_V2_ZONE_ACTION_ICON);
    check("feedback_action_icon(1): sub_zone=1", r.sub_zone == 1);

    r = dm2_v2_feedback_champion_click(3);
    check("feedback_champion_click(3): hit=1", r.hit == 1);
    check("feedback_champion_click(3): zone=CHAMPION", r.zone_id == DM2_V2_ZONE_CHAMPION);
    check("feedback_champion_click(3): champ[3] leader", h.champion_bars[3].leader == true);
    check("feedback_champion_click(3): others not leader",
        h.champion_bars[0].leader == false &&
        h.champion_bars[1].leader == false &&
        h.champion_bars[2].leader == false);

    r = dm2_v2_feedback_gold_change();
    check("feedback_gold_change: hit=1", r.hit == 1);
    check("feedback_gold_change: zone=GOLD_COUNTER", r.zone_id == DM2_V2_ZONE_GOLD_COUNTER);

    r = dm2_v2_feedback_level_change(7, 12);
    check("feedback_level_change: hit=1", r.hit == 1);
    check("feedback_level_change: cur=7", h.depth.current_level == 7);
    check("feedback_level_change: max=12", h.depth.max_level == 12);

    /* ── Hit test ─────────────────────────────────────────────────── */
    r = dm2_v2_hit_test(16, 16, 0);
    check("hit_test(16,16): COMPASS", r.zone_id == DM2_V2_ZONE_COMPASS && r.hit == 1);

    r = dm2_v2_hit_test(4, 4, 0);
    check("hit_test(4,4): CHAMPION", r.zone_id == DM2_V2_ZONE_CHAMPION && r.hit == 1);

    r = dm2_v2_hit_test(48, 180, 0);
    check("hit_test(48,180): ACTION_ICON[1]", r.zone_id == DM2_V2_ZONE_ACTION_ICON && r.hit == 1);
    check("hit_test(48,180): sub_zone=1", r.sub_zone == 1);

    r = dm2_v2_hit_test(80, 180, 0);
    check("hit_test(80,180): ACTION_ICON[2]", r.zone_id == DM2_V2_ZONE_ACTION_ICON && r.hit == 1);

    r = dm2_v2_hit_test(144, 180, 0);
    check("hit_test(144,180): ACTION_ICON[4] (last)", r.zone_id == DM2_V2_ZONE_ACTION_ICON && r.hit == 1);

    r = dm2_v2_hit_test(176, 180, 0);
    check("hit_test(176,180): x>=240, slot falls through → ACTION_ROW",
        r.zone_id == DM2_V2_ZONE_ACTION_ROW && r.hit == 1);

    /* Portrait panel (x=240..319, y=28..171 = indoor dungeon area only) */
    r = dm2_v2_hit_test(280, 100, 0);
    check("hit_test(280,100,indoor): PORTRAIT_PANEL (dungeon area)",
        r.zone_id == DM2_V2_ZONE_PORTRAIT_PANEL && r.hit == 1);

    r = dm2_v2_hit_test(280, 100, 1);
    check("hit_test(280,100,outdoor): no portrait = MISS",
        r.zone_id == DM2_V2_ZONE_MISS && r.hit == 0);

    r = dm2_v2_hit_test(280, 185, 0);
    check("hit_test(280,185,indoor): action strip overlap → ACTION_ROW",
        r.zone_id == DM2_V2_ZONE_ACTION_ROW && r.hit == 1);

    r = dm2_v2_hit_test(280, 185, 1);
    check("hit_test(280,185,outdoor): action strip overlap → ACTION_ROW",
        r.zone_id == DM2_V2_ZONE_ACTION_ROW && r.hit == 1);

    r = dm2_v2_hit_test(100, 100, 0);
    check("hit_test(100,100): dungeon area = MISS", r.zone_id == DM2_V2_ZONE_MISS && r.hit == 0);

    r = dm2_v2_hit_test(-1, 10, 0);
    check("hit_test(-1,10): out of range = MISS", r.zone_id == DM2_V2_ZONE_MISS && r.hit == 0);

    r = dm2_v2_feedback_combat_hit(1);
    check("feedback_combat_hit(1): hit=1", r.hit == 1);
    check("feedback_combat_hit(1): hp reduced", h.champion_bars[1].hp_pct < 75);

    r = dm2_v2_feedback_spell_cast(2);
    check("feedback_spell_cast(2): hit=1", r.hit == 1);

    /* ── Source evidence ─────────────────────────────────────────── */
    const char *ev = dm2_v2_hud_source_evidence();
    check("hud source_evidence not NULL", ev != NULL && strlen(ev) > 10);
    ev = dm2_v2_interaction_source_evidence();
    check("interaction source_evidence not NULL", ev != NULL && strlen(ev) > 10);

    /* ── Out-of-range icon index ─────────────────────────────────── */
    r = dm2_v2_feedback_action_icon(-1);
    check("feedback_action_icon(-1): MISS", r.zone_id == DM2_V2_ZONE_MISS && r.hit == 0);

    r = dm2_v2_feedback_action_icon(99);
    check("feedback_action_icon(99): MISS", r.zone_id == DM2_V2_ZONE_MISS && r.hit == 0);

    r = dm2_v2_feedback_champion_click(-1);
    check("feedback_champion_click(-1): MISS", r.zone_id == DM2_V2_ZONE_MISS);

    r = dm2_v2_feedback_champion_click(99);
    check("feedback_champion_click(99): MISS", r.zone_id == DM2_V2_ZONE_MISS);

    /* ── Null safety ─────────────────────────────────────────────── */
    dm2_v2_hud_set_direction(NULL, 1);
    dm2_v2_hud_set_level(NULL, 1, 10);
    dm2_v2_hud_set_gold(NULL, 0);
    dm2_v2_hud_set_champion_bar(NULL, 0, 50, 50, 50, false, false);
    dm2_v2_hud_set_action_active(NULL, DM2_V2_ACTION_ATTACK);
    dm2_v2_hud_trigger_hit_flash(NULL);
    dm2_v2_hud_toggle(NULL);
    dm2_v2_hud_set_opacity(NULL, 255);
    dm2_v2_hud_render(NULL, fb, FB_W, FB_H);
    check("null pointers: no crash", 1);

    /* ── Result ─────────────────────────────────────────────────── */
    printf("\n=== Results: %d passed, %d failed ===\n", s_tests_passed, s_tests_failed);
    return s_tests_failed > 0 ? 1 : 0;
}