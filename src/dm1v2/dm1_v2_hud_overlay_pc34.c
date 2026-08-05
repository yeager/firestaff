/*
 * dm1_v2_hud_overlay_pc34.c
 *
 * DM1 V2 compatibility state boundary.
 *
 * This module used to paint a synthetic HUD made from hard-coded rectangles,
 * invented champion names and an inline 5x5 font. That is not an acceptable
 * source for DM1 pixels. The authenticated PC34 M653/C009/C010/C011 routes in
 * M11 own the real HUD surfaces; until a V2 caller supplies those decoded
 * surfaces, this compatibility API records presentation state but draws
 * nothing.
 */
#include "dm1_v2_anim_timing.h"
#include "dm1_v2_hud_overlay_pc34.h"

#include <string.h>

static M11_V2_HudOverlay g_v2_hud_state;
static V2_Anim g_health_pulse;

static int v2_hud_clamp_pct(int pct) {
    if (pct < 0) return 0;
    if (pct > 100) return 100;
    return pct;
}

static int v2_hud_clamp_index_or_none(int idx, int count) {
    if (idx < 0 || idx >= count) return -1;
    return idx;
}

void v2_hud_init(void) {
    memset(&g_v2_hud_state, 0, sizeof(g_v2_hud_state));
    g_v2_hud_state.compass.direction = 0;
    g_v2_hud_state.depth.current_level = 1;
    g_v2_hud_state.depth.max_level = 10;
    g_v2_hud_state.visible = true;
    g_v2_hud_state.opacity = 255;
    g_v2_hud_state.stats_bar_visible = true;
    v2_hud_clear_presentation_state();
}

void v2_hud_set_direction(int dir) {
    if (dir < 0) dir = 0;
    if (dir > 3) dir = 3;
    g_v2_hud_state.compass.direction = dir;
    g_v2_hud_state.compass.needle_angle = (float)dir * 90.0f;
}

void v2_hud_set_level(int cur, int max) {
    if (cur < 0) cur = 0;
    if (max <= 0) max = 1;
    g_v2_hud_state.depth.current_level = cur;
    g_v2_hud_state.depth.max_level = max;
}

void v2_hud_render(uint8_t* fb, int w, int h) {
    (void)fb;
    (void)w;
    (void)h;
    /* Strict no-draw: no decoded source surface is owned by this API. */
}

void v2_hud_toggle(void) {
    g_v2_hud_state.visible = !g_v2_hud_state.visible;
}

void v2_hud_set_opacity(uint8_t val) {
    g_v2_hud_state.opacity = val;
}

void v2_hud_clear_presentation_state(void) {
    memset(g_v2_hud_state.champions, 0, sizeof(g_v2_hud_state.champions));
    memset(&g_v2_hud_state.action, 0, sizeof(g_v2_hud_state.action));
    memset(&g_v2_hud_state.runes, 0, sizeof(g_v2_hud_state.runes));
    g_v2_hud_state.action.active_champion = -1;
    g_v2_hud_state.action.highlighted_icon = -1;
    g_v2_hud_state.runes.active_rune = -1;
}

void v2_hud_set_champion_overlay_state(int champion_idx,
                                       int hp_pct,
                                       int stamina_pct,
                                       int mana_pct,
                                       bool active_leader,
                                       bool spell_ready) {
    if (champion_idx < 0 || champion_idx >= M11_V2_HUD_CHAMPION_COUNT_PC34) return;
    M11_V2_HudChampionOverlayPc34* c = &g_v2_hud_state.champions[champion_idx];
    c->hp_pct = v2_hud_clamp_pct(hp_pct);
    c->stamina_pct = v2_hud_clamp_pct(stamina_pct);
    c->mana_pct = v2_hud_clamp_pct(mana_pct);
    c->active_leader = active_leader;
    c->spell_ready = spell_ready;
    c->visible = true;
}

void v2_hud_set_action_overlay_state(int active_champion,
                                     int highlighted_icon,
                                     uint8_t flash_ticks) {
    g_v2_hud_state.action.visible = true;
    g_v2_hud_state.action.active_champion =
        v2_hud_clamp_index_or_none(active_champion,
                                   M11_V2_HUD_ACTION_ICON_COUNT_PC34);
    g_v2_hud_state.action.highlighted_icon =
        v2_hud_clamp_index_or_none(highlighted_icon,
                                   M11_V2_HUD_ACTION_ICON_COUNT_PC34);
    g_v2_hud_state.action.flash_ticks = flash_ticks;
}

void v2_hud_set_rune_overlay_state(uint8_t selected_rune_mask,
                                   int active_rune,
                                   bool cast_enabled,
                                   bool recant_enabled,
                                   bool caster_ready) {
    g_v2_hud_state.runes.visible = true;
    g_v2_hud_state.runes.selected_rune_mask =
        (uint8_t)(selected_rune_mask &
                  ((1u << M11_V2_HUD_RUNE_COUNT_PC34) - 1u));
    g_v2_hud_state.runes.active_rune =
        v2_hud_clamp_index_or_none(active_rune, M11_V2_HUD_RUNE_COUNT_PC34);
    g_v2_hud_state.runes.cast_enabled = cast_enabled;
    g_v2_hud_state.runes.recant_enabled = recant_enabled;
    g_v2_hud_state.runes.caster_ready = caster_ready;
}

void v2_hud_tick_presentation_state(void) {
    if (g_v2_hud_state.action.flash_ticks > 0u) {
        g_v2_hud_state.action.flash_ticks--;
    }
}

const char* v21_hud_panel_source_evidence(void) {
    return
        "DM1 V2 compatibility state only: strict no-draw without decoded "
        "PC34 M653/C009/C010/C011 surfaces. Source pixel owners: "
        "ReDMCSB PANEL.C F0354, STATS.C F0090-F0092, CHAMDRAW.C, "
        "GRAPHICS.DAT M653/C009/C010/C011.\n";
}

void v22_hud_pulse_v1_tick(void) {
    V2_Anim a = g_health_pulse;
    v2_anim_update(&a, (float)V1_TICK_MS);
    g_health_pulse = a;
}

void v22_hud_notify_move_complete(void) {
    g_v2_hud_state.moveCompletePending = 1;
}

void v22_hud_clear_move_complete(void) {
    g_v2_hud_state.moveCompletePending = 0;
}

int v22_hud_is_move_complete_pending(void) {
    return g_v2_hud_state.moveCompletePending;
}

void v22_hud_notify_turn_complete(void) {
    g_v2_hud_state.turnCompletePending = 1;
}

void v22_hud_clear_turn_complete(void) {
    g_v2_hud_state.turnCompletePending = 0;
}

int v22_hud_is_turn_complete_pending(void) {
    return g_v2_hud_state.turnCompletePending;
}

void v22_hud_start_health_pulse(void) {
    v2_anim_start(&g_health_pulse, 0.6f, 1.0f,
                  9 * V1_TICK_MS, V2_EASE_IN_OUT_QUAD);
    g_health_pulse.loops = -1;
}

float v22_hud_health_pulse_alpha(void) {
    return v2_anim_value(&g_health_pulse);
}

void v22_hud_render_champion_panel(uint8_t* fb, int w, int h,
                                   int champion_hp_pct[4],
                                   int champion_stam_pct[4],
                                   int champion_mana_pct[4]) {
    (void)fb;
    (void)w;
    (void)h;
    (void)champion_hp_pct;
    (void)champion_stam_pct;
    (void)champion_mana_pct;
    /* No invented names, glyphs, frames, bars, or hand icons. */
}
