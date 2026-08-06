#include "nexus_v2_hud_overlay.h"
#include <string.h>
#include <stdio.h>

/* ══════════════════════════════════════════════════════════════════════
 * Nexus V2 HUD Overlay — presentation-only enhanced UI chrome
 *
 * Phase 3: Nexus V2 enhanced in-game overlay presentation, UI chrome,
 * and interaction feedback.
 *
 * V2.0/V2.1 overlay:
 *   - Compass rose (4-way cardinal indicator, top-left)
 *   - Dungeon depth counter "N/M" (top-right)
 *   - Gold piece counter (bottom-right strip)
 *   - Champion mini-bars in top status bar
 *   - Action strip icons (bottom 28px)
 *   - Saturn panel indicators (menu/map/party)
 *
 * V2.2 interaction feedback:
 *   - Hit flash (action icon pulse for one frame)
 *   - Low-health pulse (champion HP bar ~2 Hz)
 *   - Smooth compass needle interpolation
 *
 * This file is a procedural diagnostic/test overlay only.  The supplied
 * European retail corpus has no authenticated NEXUS.BIN HUD surface or
 * Saturn widget-placement receipt, so this module is never a production
 * pixel owner.  Production M11 keeps it gated until such evidence exists.
 * ReDMCSB PANEL.C/DUNGEON.C remains a behavioural reference for isolated
 * state tests, not provenance for these pixels.
 *
 * This module is deliberately presentation-only: it draws optional
 * overlay elements into the supplied framebuffer and does NOT mutate
 * dungeon, champion, or command runtime state.
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Lifecycle ──────────────────────────────────────────────────── */
void nexus_v2_hud_init(Nexus_V2_HudOverlay *h) {
    if (!h) return;
    memset(h, 0, sizeof(*h));
    h->compass.direction = 0;
    h->compass.needle_angle = 0.0f;
    h->compass.animated = true;
    h->depth.current_level = 1;
    h->depth.max_level = 10;
    h->gold.party_gold = 0;
    h->gold.visible = true;
    h->visible = true;
    h->opacity = 255;
    h->stats_bar_visible = true;
    h->action_strip.visible = true;
    h->hit_flash_active = false;
    h->hit_flash_timer = 0;
    h->panel.menu_indicator = false;
    h->panel.map_indicator = false;
    h->panel.party_indicator = true;
}

void nexus_v2_hud_reset(Nexus_V2_HudOverlay *h) {
    nexus_v2_hud_init(h);
}

/* ── Parameter setters ──────────────────────────────────────────── */
void nexus_v2_hud_set_direction(Nexus_V2_HudOverlay *h, int dir) {
    if (!h) return;
    if (dir < 0) dir = 0;
    if (dir > 3) dir = 3;
    h->compass.direction = dir;
    h->compass.needle_angle = (float)dir * 90.0f;
}

void nexus_v2_hud_set_level(Nexus_V2_HudOverlay *h, int cur, int max) {
    if (!h) return;
    if (cur < 0) cur = 0;
    if (max <= 0) max = 1;
    h->depth.current_level = cur;
    h->depth.max_level = max;
}

void nexus_v2_hud_set_gold(Nexus_V2_HudOverlay *h, int gold_pieces) {
    if (!h) return;
    h->gold.party_gold = gold_pieces;
    h->gold.visible = true;
}

void nexus_v2_hud_set_champion_bar(Nexus_V2_HudOverlay *h, int champ_idx,
    int hp_pct, int stamina_pct, int mana_pct, bool leader, bool spell_ready)
{
    if (!h) return;
    if (champ_idx < 0 || champ_idx >= 4) return;
    h->champion_bars[champ_idx].champion_index = champ_idx;
    h->champion_bars[champ_idx].hp_pct = hp_pct;
    h->champion_bars[champ_idx].stamina_pct = stamina_pct;
    h->champion_bars[champ_idx].mana_pct = mana_pct;
    h->champion_bars[champ_idx].leader = leader;
    h->champion_bars[champ_idx].spell_ready = spell_ready;
}

void nexus_v2_hud_set_action_active(Nexus_V2_HudOverlay *h, Nexus_V2_ActionIcon icon) {
    if (!h) return;
    for (int i = 0; i < NEXUS_V2_ACTION_COUNT; i++) {
        h->action_strip.icons[i].active = (i == (int)icon);
    }
}

void nexus_v2_hud_trigger_hit_flash(Nexus_V2_HudOverlay *h) {
    if (!h) return;
    h->hit_flash_active = true;
    h->hit_flash_timer = 6;
}

void nexus_v2_hud_toggle(Nexus_V2_HudOverlay *h) {
    if (!h) return;
    h->visible = !h->visible;
}

void nexus_v2_hud_set_opacity(Nexus_V2_HudOverlay *h, uint8_t val) {
    if (!h) return;
    h->opacity = val;
}

/* ── Main render entry ──────────────────────────────────────────── */
void nexus_v2_hud_render(Nexus_V2_HudOverlay *h, uint8_t *fb, int stride, int h_res) {
    (void)h;
    (void)fb;
    (void)stride;
    (void)h_res;

    /* State is retained for diagnostics, but no procedural compass, glyph,
     * bar, icon, palette, or placement may enter the retail framebuffer.
     * Restore drawing only after the original Saturn HUD owner and VDP1/VDP2
     * placement are captured from the supplied game. */
}

const char *nexus_v2_hud_source_evidence(void) {
    return
        "Nexus V2 HUD: procedural diagnostic/test overlay only\n"
        "  No authenticated retail NEXUS.BIN/widget surface in supplied corpus\n"
        "  DMDF/DGN describes dungeon source formats, not HUD ownership\n"
        "  No Saturn VDP1/VDP2 placement receipt; production pixels forbidden\n"
        "  ReDMCSB PANEL.C/DUNGEON.C are behavioural test references only\n";
}
