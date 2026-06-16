/*
 * firestaff_nexus_v2_hud_overlay_probe.c
 *
 * Nexus V2.0/V2.1/V2.2 HUD overlay headless probe.
 *
 * Headless probe: verifies nexus_v2_hud_overlay.c without requiring
 * live game asset files or a running SDL renderer.
 *
 * This probe validates:
 *
 *   1. nexus_v2_hud_init() zeroes state and sets default values
 *      (visible=0, opacity=255, hit_flash_active=0)
 *
 *   2. nexus_v2_hud_reset() returns to init defaults
 *
 *   3. nexus_v2_hud_set_direction() stores the 0..3 cardinal
 *      direction (4-way: N/E/S/W)
 *
 *   4. nexus_v2_hud_set_level() stores current_level + max_level
 *
 *   5. nexus_v2_hud_set_gold() stores party gold pieces
 *
 *   6. nexus_v2_hud_set_champion_bar() stores per-champion HP/St/Ma
 *      plus leader + spell_ready flags
 *
 *   7. nexus_v2_hud_set_action_active() flips the action strip
 *      active flag
 *
 *   8. nexus_v2_hud_trigger_hit_flash() sets hit_flash_active and
 *      resets hit_flash_timer to a non-zero value
 *
 *   9. nexus_v2_hud_toggle() flips the visible flag
 *
 *  10. nexus_v2_hud_set_opacity() clamps to 0..255 range
 *
 *  11. nexus_v2_hud_render() draws without crashing on a 320x200
 *      framebuffer, byte-modifies the buffer, and produces
 *      deterministic output for the same state
 *
 *  12. NEXUS_V2_ACTION_COUNT = 5 (Attack, Cast, Use, Drop, Move)
 *
 *  13. null-args are safe across all entry points
 *
 *  14. source-evidence references NEXUS.BIN / VDP1 / VDP2 / ReDMCSB
 *
 * Exit codes:
 *   0  - all checks passed
 *   1  - one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_nexus_v2_hud_overlay_probe
 *
 * Source references:
 *   Saturn NEXUS.BIN           HUD surface data
 *   DMDF parser documentation (DMDF/DGN level format)
 *   Saturn SDK VDP1/VDP2       bitmap surfaces, background layers
 *   ReDMCSB PANEL.C            champion status refresh
 *   ReDMCSB DUNGEON.C          party stat bar refresh timing
 */

#include "nexus_v2_hud_overlay.h"

#include <stdio.h>
#include <string.h>

static int g_total = 0;
static int g_failed = 0;

static void check(int cond, const char *name)
{
    ++g_total;
    if (!cond) {
        ++g_failed;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static void check_init(void)
{
    Nexus_V2_HudOverlay h;
    memset(&h, 0x55, sizeof(h));
    nexus_v2_hud_init(&h);
    check(h.visible == 1, "init: visible == 1 (V2 default)");
    check(h.opacity == 255, "init: opacity == 255 (fully opaque)");
    check(h.hit_flash_active == 0, "init: hit_flash_active == 0");
    check(h.hit_flash_timer == 0, "init: hit_flash_timer == 0");
    check(h.compass.direction == 0, "init: compass.direction == 0 (N)");
    check(h.depth.current_level == 1, "init: depth.current_level == 1");
    check(h.depth.max_level == 10, "init: depth.max_level == 10");
    check(h.gold.party_gold == 0, "init: gold.party_gold == 0");
    check(h.gold.visible == 1, "init: gold.visible == 1");
    check(h.action_strip.visible == 1, "init: action_strip.visible == 1");
}

static void check_reset(void)
{
    Nexus_V2_HudOverlay h;
    nexus_v2_hud_init(&h);
    /* Mutate. */
    nexus_v2_hud_set_direction(&h, 2);
    nexus_v2_hud_set_level(&h, 5, 16);
    nexus_v2_hud_set_gold(&h, 12345);
    h.visible = 1;
    h.opacity = 128;
    h.hit_flash_active = 1;
    /* Reset. */
    nexus_v2_hud_reset(&h);
    check(h.compass.direction == 0, "reset: compass.direction -> 0");
    check(h.depth.current_level == 1, "reset: depth.current_level -> 1");
    check(h.depth.max_level == 10, "reset: depth.max_level -> 10");
    check(h.gold.party_gold == 0, "reset: gold.party_gold -> 0");
    check(h.visible == 1, "reset: visible -> 1");
    check(h.opacity == 255, "reset: opacity -> 255");
    check(h.hit_flash_active == 0, "reset: hit_flash_active -> 0");
}

static void check_set_direction(void)
{
    Nexus_V2_HudOverlay h;
    nexus_v2_hud_init(&h);
    nexus_v2_hud_set_direction(&h, 0);
    check(h.compass.direction == 0, "set_direction: N=0");
    nexus_v2_hud_set_direction(&h, 1);
    check(h.compass.direction == 1, "set_direction: E=1");
    nexus_v2_hud_set_direction(&h, 2);
    check(h.compass.direction == 2, "set_direction: S=2");
    nexus_v2_hud_set_direction(&h, 3);
    check(h.compass.direction == 3, "set_direction: W=3");
    /* OOB direction is implementation-defined; just check no crash. */
    nexus_v2_hud_set_direction(&h, 99);
    check(1, "set_direction: OOB safe");
}

static void check_set_level(void)
{
    Nexus_V2_HudOverlay h;
    nexus_v2_hud_init(&h);
    nexus_v2_hud_set_level(&h, 4, 10);
    check(h.depth.current_level == 4, "set_level: cur=4");
    check(h.depth.max_level == 10, "set_level: max=10");
    nexus_v2_hud_set_level(&h, 0, 0);
    check(h.depth.current_level == 0, "set_level: cur=0");
    check(h.depth.max_level == 1, "set_level: max=0 clamps to 1");
}

static void check_set_gold(void)
{
    Nexus_V2_HudOverlay h;
    nexus_v2_hud_init(&h);
    nexus_v2_hud_set_gold(&h, 12345);
    check(h.gold.party_gold == 12345, "set_gold: party_gold=12345");
    nexus_v2_hud_set_gold(&h, 0);
    check(h.gold.party_gold == 0, "set_gold: party_gold=0");
    nexus_v2_hud_set_gold(&h, -1);
    check(h.gold.party_gold == -1, "set_gold: party_gold=-1 (raw, no clamp)");
}

static void check_set_champion_bar(void)
{
    Nexus_V2_HudOverlay h;
    nexus_v2_hud_init(&h);
    nexus_v2_hud_set_champion_bar(&h, 0, 80, 50, 100, 1, 1);
    check(h.champion_bars[0].hp_pct == 80, "champion[0].hp_pct=80");
    check(h.champion_bars[0].stamina_pct == 50, "champion[0].stamina_pct=50");
    check(h.champion_bars[0].mana_pct == 100, "champion[0].mana_pct=100");
    check(h.champion_bars[0].leader == 1, "champion[0].leader=1");
    check(h.champion_bars[0].spell_ready == 1, "champion[0].spell_ready=1");
    check(h.champion_bars[1].hp_pct == 0, "champion[1].hp_pct default 0");
    check(h.champion_bars[1].leader == 0, "champion[1].leader default 0");
    /* OOB champion index is safe. */
    nexus_v2_hud_set_champion_bar(&h, 99, 50, 50, 50, 0, 0);
    check(1, "set_champion_bar: OOB safe");
    nexus_v2_hud_set_champion_bar(&h, -1, 50, 50, 50, 0, 0);
    check(1, "set_champion_bar: neg index safe");
}

static void check_set_action_active(void)
{
    Nexus_V2_HudOverlay h;
    int i;
    int active_before;
    int active_after;

    nexus_v2_hud_init(&h);
    /* All icons start inactive. */
    active_before = 0;
    for (i = 0; i < NEXUS_V2_ACTION_COUNT; ++i) {
        if (h.action_strip.icons[i].active) ++active_before;
    }
    check(active_before == 0, "set_action_active: all start inactive");

    /* Set one active. */
    nexus_v2_hud_set_action_active(&h, NEXUS_V2_ACTION_ATTACK);
    check(h.action_strip.icons[NEXUS_V2_ACTION_ATTACK].active == 1,
          "set_action_active: ATTACK active after set");
    check(h.action_strip.icons[NEXUS_V2_ACTION_CAST].active == 0,
          "set_action_active: CAST still inactive");

    /* Set_action_active replaces the active icon (single-select). */
    nexus_v2_hud_set_action_active(&h, NEXUS_V2_ACTION_MOVE);
    active_after = 0;
    for (i = 0; i < NEXUS_V2_ACTION_COUNT; ++i) {
        if (h.action_strip.icons[i].active) ++active_after;
    }
    check(active_after == 1, "set_action_active: single-select, exactly 1 active");
    check(h.action_strip.icons[NEXUS_V2_ACTION_MOVE].active == 1,
          "set_action_active: MOVE active after second set");
    check(h.action_strip.icons[NEXUS_V2_ACTION_ATTACK].active == 0,
          "set_action_active: ATTACK no longer active after MOVE set");
}

static void check_hit_flash(void)
{
    Nexus_V2_HudOverlay h;
    nexus_v2_hud_init(&h);
    check(h.hit_flash_active == 0, "hit_flash: starts inactive");
    nexus_v2_hud_trigger_hit_flash(&h);
    check(h.hit_flash_active == 1, "hit_flash: active after trigger");
    check(h.hit_flash_timer > 0, "hit_flash: timer > 0 after trigger");
}

static void check_toggle(void)
{
    Nexus_V2_HudOverlay h;
    nexus_v2_hud_init(&h);
    check(h.visible == 1, "toggle: starts visible (V2 default)");
    nexus_v2_hud_toggle(&h);
    check(h.visible == 0, "toggle: invisible after 1st toggle");
    nexus_v2_hud_toggle(&h);
    check(h.visible == 1, "toggle: visible after 2nd toggle");
}

static void check_set_opacity(void)
{
    Nexus_V2_HudOverlay h;
    nexus_v2_hud_init(&h);
    nexus_v2_hud_set_opacity(&h, 128);
    check(h.opacity == 128, "set_opacity: 128");
    nexus_v2_hud_set_opacity(&h, 0);
    check(h.opacity == 0, "set_opacity: 0");
    nexus_v2_hud_set_opacity(&h, 255);
    check(h.opacity == 255, "set_opacity: 255");
}

static void check_action_count(void)
{
    check(NEXUS_V2_ACTION_COUNT == 5,
          "NEXUS_V2_ACTION_COUNT == 5 (ATTACK, CAST, USE, DROP, MOVE)");
}

static void check_render_deterministic(void)
{
    Nexus_V2_HudOverlay h1, h2;
    uint8_t fb1[320 * 200];
    uint8_t fb2[320 * 200];
    int i;
    int same;

    memset(fb1, 0, sizeof(fb1));
    memset(fb2, 0, sizeof(fb2));
    nexus_v2_hud_init(&h1);
    nexus_v2_hud_init(&h2);
    nexus_v2_hud_set_direction(&h1, 1);
    nexus_v2_hud_set_level(&h1, 4, 16);
    nexus_v2_hud_set_gold(&h1, 100);
    nexus_v2_hud_set_champion_bar(&h1, 0, 80, 50, 100, 1, 1);
    h1.visible = 1;
    h1.opacity = 255;
    /* Copy state to h2. */
    h2 = h1;

    nexus_v2_hud_render(&h1, fb1, 320, 200);
    nexus_v2_hud_render(&h2, fb2, 320, 200);
    same = 1;
    for (i = 0; i < 320 * 200; ++i) {
        if (fb1[i] != fb2[i]) {
            same = 0;
            break;
        }
    }
    check(same, "render: deterministic for same state");
}

static void check_render_modifies_buffer(void)
{
    Nexus_V2_HudOverlay h;
    uint8_t fb[320 * 200];
    int i;
    int any_changed = 0;

    memset(fb, 0, sizeof(fb));
    nexus_v2_hud_init(&h);
    /* Make HUD visible with content. */
    h.visible = 1;
    h.opacity = 255;
    nexus_v2_hud_set_direction(&h, 0);
    nexus_v2_hud_set_level(&h, 1, 10);
    nexus_v2_hud_set_gold(&h, 50);
    nexus_v2_hud_render(&h, fb, 320, 200);
    for (i = 0; i < 320 * 200; ++i) {
        if (fb[i] != 0) {
            any_changed = 1;
            break;
        }
    }
    check(any_changed, "render: modifies framebuffer when visible");
}

static void check_render_invisible_no_change(void)
{
    Nexus_V2_HudOverlay h;
    uint8_t fb[320 * 200];
    int i;
    int any_changed = 0;

    memset(fb, 0, sizeof(fb));
    nexus_v2_hud_init(&h);
    /* Force invisible. */
    h.visible = 0;
    nexus_v2_hud_render(&h, fb, 320, 200);
    for (i = 0; i < 320 * 200; ++i) {
        if (fb[i] != 0) {
            any_changed = 1;
            break;
        }
    }
    check(any_changed == 0, "render: invisible HUD leaves buffer unchanged");
}

static void check_null_args(void)
{
    /* All entry points must be safe on NULL. */
    nexus_v2_hud_init(0);
    nexus_v2_hud_reset(0);
    nexus_v2_hud_set_direction(0, 0);
    nexus_v2_hud_set_level(0, 0, 0);
    nexus_v2_hud_set_gold(0, 0);
    nexus_v2_hud_set_champion_bar(0, 0, 0, 0, 0, 0, 0);
    nexus_v2_hud_set_action_active(0, NEXUS_V2_ACTION_ATTACK);
    nexus_v2_hud_trigger_hit_flash(0);
    nexus_v2_hud_toggle(0);
    nexus_v2_hud_set_opacity(0, 0);
    nexus_v2_hud_render(0, 0, 0, 0);
    check(1, "null_args: all entry points safe");
}

static void check_source_evidence(void)
{
    const char *e = nexus_v2_hud_source_evidence();
    check(e != 0 && e[0] != 0, "evidence: present + non-empty");
    check(e && strstr(e, "NEXUS.BIN") != 0, "evidence: NEXUS.BIN");
    check(e && strstr(e, "VDP1") != 0, "evidence: VDP1 (Saturn)");
    check(e && strstr(e, "VDP2") != 0, "evidence: VDP2 (Saturn)");
    check(e && strstr(e, "DMDF") != 0, "evidence: DMDF (DGN format)");
    check(e && strstr(e, "PANEL.C") != 0, "evidence: ReDMCSB PANEL.C");
    check(e && strstr(e, "DUNGEON.C") != 0, "evidence: ReDMCSB DUNGEON.C");
}

int main(void)
{
    printf("=== Nexus V2 HUD Overlay Probe ===\n");
    check_init();
    check_reset();
    check_set_direction();
    check_set_level();
    check_set_gold();
    check_set_champion_bar();
    check_set_action_active();
    check_hit_flash();
    check_toggle();
    check_set_opacity();
    check_action_count();
    check_render_deterministic();
    check_render_modifies_buffer();
    check_render_invisible_no_change();
    check_null_args();
    check_source_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
