/*
 * test_theron_v2_hud_overlay_pc34.c — Theron V2 Phase 3 HUD Overlay smoke test
 *
 * Phase 3 (initial seed): Tests that the HUD overlay module initialises,
 * sets parameters, renders into a 256x224 indexed PC Engine framebuffer,
 * and produces deterministic output without any game data or SDL.
 *
 * Source-lock anchors (citations only, no game data required):
 *   THQUEST.ASM T520  party placement / start position
 *   THQUEST.ASM T560  dungeon loading
 *   THQUEST.ASM T600  UI overlay zones
 *   THQUEST.ASM T700  timers / world tick
 *   THQUEST.ASM T800  champion persistence + inventory reset
 *   THQUEST.ASM T900  object database / rune magic
 *   HuC6260/HuC6270 datasheet (PC Engine VDC + VCE)
 *   dmweb Theron overview (7 dungeons + 7 relic goals + rune magic)
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 */

#include "theron_v2_hud_overlay_pc34.h"
#include "theron_v1_viewport.h"   /* TQR_FB_W, TQR_FB_H */
#include <stdio.h>
#include <string.h>

#define FB_W TQR_FB_W   /* 256 - PC Engine native width */
#define FB_H TQR_FB_H   /* 224 - PC Engine native height */

static int s_tests_passed = 0;
static int s_tests_failed = 0;

static void check(const char *name, int cond)
{
    if (cond) {
        printf("  PASS: %s\n", name);
        s_tests_passed++;
    } else {
        printf("  FAIL: %s\n", name);
        s_tests_failed++;
    }
}

int main(void)
{
    printf("=== Theron V2 Phase 3 HUD Overlay smoke test ===\n\n");

    /* ── Init/reset ────────────────────────────────────────────────── */
    Theron_V2_HudOverlay h;
    theron_v2_hud_init(&h);
    check("init: visible default", h.visible == true);
    check("init: opacity 255", h.opacity == 255);
    check("init: compass direction 0 (N)", h.compass.direction == 0);
    check("init: dungeon progress 1/7 (Theron fixed per DMWeb)",
          h.dungeon_progress.current_dungeon == 1 &&
          h.dungeon_progress.total_dungeons == 7);
    check("init: quest items visible", h.quest_items.visible == true);
    check("init: relic counter 0/7", h.relic_counter.relics_found == 0 &&
          h.relic_counter.relics_required == 7);
    check("init: rune indicator visible", h.rune_indicator.visible == true);
    check("init: action strip visible", h.action_strip.visible == true);
    check("init: stats_bar_visible true", h.stats_bar_visible == true);
    check("init: top_bar_visible true", h.top_bar_visible == true);
    check("init: slot 0 leader (Theron)", h.champion_bars[0].leader == true);
    check("init: slot 1..3 not leader",
          !h.champion_bars[1].leader && !h.champion_bars[2].leader &&
          !h.champion_bars[3].leader);

    theron_v2_hud_reset(&h);
    check("reset: same as init", h.opacity == 255 && h.visible == true);

    /* ── Parameter setters ──────────────────────────────────────────── */
    theron_v2_hud_set_direction(&h, 2);
    check("set_direction 2 (S) → direction==2", h.compass.direction == 2);

    theron_v2_hud_set_direction(&h, -1);
    check("set_direction -1 → clamped 0", h.compass.direction == 0);

    theron_v2_hud_set_direction(&h, 99);
    check("set_direction 99 → clamped 3", h.compass.direction == 3);

    theron_v2_hud_set_quest_items(&h, 3, 12);
    check("set_quest_items(3,12)", h.quest_items.collected == 3 &&
          h.quest_items.total == 12);

    theron_v2_hud_set_dungeon_progress(&h, 5, 7);
    check("set_dungeon_progress(5,7)", h.dungeon_progress.current_dungeon == 5 &&
          h.dungeon_progress.total_dungeons == 7);

    theron_v2_hud_set_dungeon_progress(&h, -1, 0);
    check("set_dungeon_progress(-1,0) → cur=0,max=1",
          h.dungeon_progress.current_dungeon == 0 &&
          h.dungeon_progress.total_dungeons == 1);

    theron_v2_hud_set_relics(&h, 4, 7);
    check("set_relics(4,7)", h.relic_counter.relics_found == 4 &&
          h.relic_counter.relics_required == 7);

    theron_v2_hud_set_rune_indicator(&h, true, true, 2);
    check("set_rune_indicator(ready,charging,idx=2)",
          h.rune_indicator.rune_ready == true &&
          h.rune_indicator.spell_charging == true &&
          h.rune_indicator.rune_index == 2);

    theron_v2_hud_set_champion_bar(&h, 2, 75, 50, 100, true, false);
    check("set_champion_bar idx=2", h.champion_bars[2].champion_index == 2);
    check("set_champion_bar hp=75", h.champion_bars[2].hp_pct == 75);
    check("set_champion_bar stamina=50", h.champion_bars[2].stamina_pct == 50);
    check("set_champion_bar mana=100", h.champion_bars[2].mana_pct == 100);
    check("set_champion_bar leader=true", h.champion_bars[2].leader == true);
    check("set_champion_bar spell_ready=false", h.champion_bars[2].spell_ready == false);

    theron_v2_hud_set_champion_bar(&h, 0, 20, 30, 40, false, true);
    check("set_champion_bar idx=0", h.champion_bars[0].champion_index == 0);
    check("set_champion_bar hp=20", h.champion_bars[0].hp_pct == 20);

    /* Out-of-range champion index */
    theron_v2_hud_set_champion_bar(&h, -1, 50, 50, 50, false, false);
    theron_v2_hud_set_champion_bar(&h, 99, 50, 50, 50, false, false);
    check("set_champion_bar idx=-1: no crash", 1);
    check("set_champion_bar idx=99: no crash", 1);

    theron_v2_hud_set_action_active(&h, THERON_V2_ACTION_CAST);
    check("set_action_active(CST) → CST active",
          h.action_strip.icons[THERON_V2_ACTION_CAST].active == true);
    check("set_action_active(CST) → others inactive",
          h.action_strip.icons[THERON_V2_ACTION_ATTACK].active == false &&
          h.action_strip.icons[THERON_V2_ACTION_MOVE].active == false);

    theron_v2_hud_toggle(&h);
    check("toggle → visible=false", h.visible == false);
    theron_v2_hud_toggle(&h);
    check("toggle → visible=true", h.visible == true);

    theron_v2_hud_set_opacity(&h, 128);
    check("set_opacity 128", h.opacity == 128);

    theron_v2_hud_trigger_hit_flash(&h);
    check("trigger_hit_flash → active+6", h.hit_flash_active == true &&
          h.hit_flash_timer == 6);

    /* ── Render into 256x224 fb (PC Engine native, no SDL) ──────────── */
    uint8_t fb[FB_W * FB_H];
    memset(fb, 0, sizeof(fb));

    /* Set all four champion bars to reasonable values so the
     * stats_bar_visible branch paints at least one set of bars. */
    theron_v2_hud_set_champion_bar(&h, 0, 80, 60, 90, true, true);
    theron_v2_hud_set_champion_bar(&h, 1, 60, 50, 50, false, false);
    theron_v2_hud_set_champion_bar(&h, 2, 40, 30, 20, false, false);
    theron_v2_hud_set_champion_bar(&h, 3, 100, 100, 100, false, false);

    theron_v2_hud_set_opacity(&h, 255);
    theron_v2_hud_render(&h, fb, FB_W, FB_H);
    check("render: no crash", 1);

    /* ── Top-bar (y=0..23): compass at (16,12) ─────────────────────── */
    int compass_pixels = 0;
    for (int y = 4; y <= 22; y++) {
        for (int x = 8; x <= 24; x++) {
            if (fb[y * FB_W + x] != 0) compass_pixels++;
        }
    }
    check("compass area: some pixels written", compass_pixels > 0);

    /* ── Top-bar: quest item region (x≈64..96) ─────────────────────── */
    int quest_pixels = 0;
    for (int y = 2; y <= 14; y++) {
        for (int x = 60; x <= 100; x++) {
            if (fb[y * FB_W + x] != 0) quest_pixels++;
        }
    }
    check("quest items area: some pixels written", quest_pixels > 0);

    /* ── Top-bar: dungeon progress region (x≈160..200) ─────────────── */
    int dungeon_pixels = 0;
    for (int y = 2; y <= 14; y++) {
        for (int x = 156; x <= 200; x++) {
            if (fb[y * FB_W + x] != 0) dungeon_pixels++;
        }
    }
    check("dungeon progress area: some pixels written", dungeon_pixels > 0);

    /* ── Top-bar: relic counter region (x≈220..252) ─────────────────── */
    int relic_pixels = 0;
    for (int y = 2; y <= 14; y++) {
        for (int x = 216; x <= FB_W; x++) {
            if (fb[y * FB_W + x] != 0) relic_pixels++;
        }
    }
    check("relic counter area: some pixels written", relic_pixels > 0);

    /* ── Bottom panel: champion mini-bars (y=184..191) ──────────────── */
    int bar_pixels = 0;
    for (int y = THERON_V2_CHAMP_BAR_Y; y < THERON_V2_CHAMP_BAR_Y + THERON_V2_CHAMP_BAR_H; y++) {
        for (int x = 0; x < FB_W; x++) {
            if (fb[y * FB_W + x] != 0) bar_pixels++;
        }
    }
    check("champion bars: some pixels written", bar_pixels > 0);

    /* ── Bottom action strip (y=208..221) ───────────────────────────── */
    int strip_pixels = 0;
    for (int y = THERON_V2_ACTION_STRIP_Y; y < THERON_V2_ACTION_STRIP_Y + THERON_V2_ACTION_STRIP_H; y++) {
        for (int x = 0; x < FB_W; x++) {
            if (fb[y * FB_W + x] != 0) strip_pixels++;
        }
    }
    check("action strip: some pixels written", strip_pixels > 0);

    /* ── Hidden render (opacity=0) → no pixels ──────────────────────── */
    memset(fb, 0, sizeof(fb));
    h.opacity = 0;
    theron_v2_hud_render(&h, fb, FB_W, FB_H);
    int zero_pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) zero_pixels++;
    }
    check("hidden render (opacity=0): no pixels", zero_pixels == 0);

    /* ── Invisible render → no pixels ───────────────────────────────── */
    memset(fb, 0, sizeof(fb));
    h.visible = false;
    h.opacity = 255;
    theron_v2_hud_render(&h, fb, FB_W, FB_H);
    zero_pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) zero_pixels++;
    }
    check("invisible render: no pixels", zero_pixels == 0);

    /* ── Top-bar visibility toggle ──────────────────────────────────── */
    h.visible = true;
    h.opacity = 255;
    h.top_bar_visible = false;
    h.stats_bar_visible = true;
    memset(fb, 0, sizeof(fb));
    theron_v2_hud_render(&h, fb, FB_W, FB_H);
    int topbar_pixels_after_hide = 0;
    for (int y = 0; y < THERON_V2_HUD_TOPBAR_H; y++) {
        for (int x = 0; x < FB_W; x++) {
            if (fb[y * FB_W + x] != 0) topbar_pixels_after_hide++;
        }
    }
    check("top_bar disabled: no pixels in y=0..23",
          topbar_pixels_after_hide == 0);

    /* ── Hit flash decay (V2.2 interaction feedback) ────────────────── */
    h.visible = true;
    h.opacity = 255;
    h.top_bar_visible = true;
    h.stats_bar_visible = true;
    h.action_strip.visible = true;
    h.hit_flash_active = true;
    h.hit_flash_timer = 3;
    theron_v2_hud_render(&h, fb, FB_W, FB_H);
    check("hit_flash_timer decrements on render", h.hit_flash_timer == 2);
    theron_v2_hud_render(&h, fb, FB_W, FB_H);
    check("hit_flash_timer 1 after 2nd render", h.hit_flash_timer == 1);
    theron_v2_hud_render(&h, fb, FB_W, FB_H);
    check("hit_flash_timer 0 after 3rd render → inactive",
          h.hit_flash_timer == 0 && h.hit_flash_active == false);

    /* ── Low-HP pulse triggers on hp_pct < 25 (no crash) ─────────────── */
    theron_v2_hud_set_champion_bar(&h, 0, 10, 30, 30, true, false);
    memset(fb, 0, sizeof(fb));
    theron_v2_hud_render(&h, fb, FB_W, FB_H);
    check("low-HP pulse: render with hp<25% no crash", 1);

    /* ── Out-of-range action icon (defensive cast) ──────────────────── */
    theron_v2_hud_set_action_active(&h, (Theron_V2_ActionIcon)99);
    check("set_action_active(invalid): no crash", 1);

    /* ── Source evidence ────────────────────────────────────────────── */
    const char *ev = theron_v2_hud_source_evidence();
    check("hud source_evidence not NULL", ev != NULL && strlen(ev) > 10);
    check("source_evidence: mentions THQUEST.ASM T600",
          strstr(ev, "THQUEST.ASM T600") != NULL);
    check("source_evidence: mentions THQUEST.ASM T900 (rune magic)",
          strstr(ev, "THQUEST.ASM T900") != NULL);
    check("source_evidence: mentions HuC6260/HuC6270 (PC Engine)",
          strstr(ev, "HuC6260") != NULL || strstr(ev, "HuC6270") != NULL);
    check("source_evidence: mentions dmweb Theron overview",
          strstr(ev, "dmweb") != NULL || strstr(ev, "DMWeb") != NULL);

    /* ── Null safety ────────────────────────────────────────────────── */
    theron_v2_hud_set_direction(NULL, 1);
    theron_v2_hud_set_quest_items(NULL, 0, 0);
    theron_v2_hud_set_dungeon_progress(NULL, 1, 7);
    theron_v2_hud_set_relics(NULL, 0, 7);
    theron_v2_hud_set_rune_indicator(NULL, false, false, -1);
    theron_v2_hud_set_champion_bar(NULL, 0, 50, 50, 50, false, false);
    theron_v2_hud_set_action_active(NULL, THERON_V2_ACTION_ATTACK);
    theron_v2_hud_trigger_hit_flash(NULL);
    theron_v2_hud_toggle(NULL);
    theron_v2_hud_set_opacity(NULL, 255);
    theron_v2_hud_render(NULL, fb, FB_W, FB_H);
    theron_v2_hud_init(NULL);
    theron_v2_hud_reset(NULL);
    check("null pointers: no crash", 1);

    /* ── Result ─────────────────────────────────────────────────────── */
    printf("\n=== Results: %d passed, %d failed ===\n",
           s_tests_passed, s_tests_failed);
    return s_tests_failed > 0 ? 1 : 0;
}
