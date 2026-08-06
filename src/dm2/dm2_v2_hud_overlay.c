#include "dm2_v2_hud_overlay.h"
#include "dm1_v2_anim_timing.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ══════════════════════════════════════════════════════════════════════
 * DM2 V2 HUD Overlay — presentation-only enhanced UI chrome
 *
 * Phase 3: DM2 V2 enhanced in-game overlay presentation, UI chrome,
 * and interaction feedback.
 *
 * V2.0/V2.1 overlay:
 *   - Compass rose (4-way cardinal indicator, top-left)
 *   - Dungeon depth counter "N/M" (top-right)
 *   - Gold piece counter (bottom-right strip, DM2-specific)
 *   - Champion mini-bars in top status bar
 *   - Action strip icons (bottom 28px)
 *
 * V2.2 interaction feedback:
 *   - Hit flash (action icon pulse for one frame)
 *   - Low-health pulse (champion HP bar ~2 Hz)
 *   - Smooth compass needle interpolation
 *
 * Source: SKULL.ASM T560 (DM2 HUD rendering)
 *         SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *         ReDMCSB PANEL.C F0354 champion status box drawing
 *         ReDMCSB DUNGEON.C stat-bar refresh timing
 *
 * This module is deliberately presentation-only: it draws optional
 * overlay elements into the supplied framebuffer and does NOT mutate
 * dungeon, champion, companion, or command runtime state.
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Lifecycle ──────────────────────────────────────────────────── */
void dm2_v2_hud_init(DM2_V2_HudOverlay *h) {
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
}

void dm2_v2_hud_reset(DM2_V2_HudOverlay *h) {
    dm2_v2_hud_init(h);
}

/* ── Parameter setters ──────────────────────────────────────────── */
void dm2_v2_hud_set_direction(DM2_V2_HudOverlay *h, int dir) {
    if (!h) return;
    if (dir < 0) dir = 0;
    if (dir > 3) dir = 3;
    h->compass.direction = dir;
    h->compass.needle_angle = (float)dir * 90.0f;
}

void dm2_v2_hud_set_level(DM2_V2_HudOverlay *h, int cur, int max) {
    if (!h) return;
    if (cur < 0) cur = 0;
    if (max <= 0) max = 1;
    h->depth.current_level = cur;
    h->depth.max_level = max;
}

void dm2_v2_hud_set_gold(DM2_V2_HudOverlay *h, int gold_pieces) {
    if (!h) return;
    h->gold.party_gold = gold_pieces;
    h->gold.visible = true;
}

void dm2_v2_hud_set_champion_bar(DM2_V2_HudOverlay *h, int champ_idx,
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

void dm2_v2_hud_set_action_active(DM2_V2_HudOverlay *h, DM2_V2_ActionIcon icon) {
    if (!h) return;
    for (int i = 0; i < DM2_V2_ACTION_COUNT; i++) {
        h->action_strip.icons[i].active = (i == (int)icon);
    }
}

void dm2_v2_hud_trigger_hit_flash(DM2_V2_HudOverlay *h) {
    if (!h) return;
    h->hit_flash_active = true;
    h->hit_flash_timer = 6;
}

void dm2_v2_hud_toggle(DM2_V2_HudOverlay *h) {
    if (!h) return;
    h->visible = !h->visible;
}

void dm2_v2_hud_set_opacity(DM2_V2_HudOverlay *h, uint8_t val) {
    if (!h) return;
    h->opacity = val;
}

/* ── Main render entry ──────────────────────────────────────────── */
void dm2_v2_hud_render(DM2_V2_HudOverlay *h, uint8_t *fb, int stride, int h_res) {
    /* No generated HUD pixels.  This former direct overlay has no original
     * GDAT loader/context argument, therefore it must remain no-draw.  The
     * production route is dm2_v2_hud_runtime_render(), which requires the
     * mounted, hash-verified INTERFACE_GENERAL/CHAMPIONS data source. */
    (void)h;
    (void)fb;
    (void)stride;
    (void)h_res;
}

const char *dm2_v2_hud_source_evidence(void) {
    return
        "DM2 V2 direct overlay: retired; no generated pixels are admitted\n"
        "  Source: SKULL.ASM T560 (DM2 HUD rendering)\n"
        "  Source: SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout)\n"
        "  Source: ReDMCSB PANEL.C F0354 (champion status-box rendering)\n"
        "  Source: ReDMCSB DUNGEON.C (stat-bar refresh, F0260)\n"
        "DM2 V2.2: hit flash, low-HP pulse, smooth compass interpolation\n"
        "  Source: ReDMCSB COMMAND.C action feedback gates\n"
        "  Source: ReDMCSB DISPLAY.C pulse animation timing (2 Hz)\n";
}
