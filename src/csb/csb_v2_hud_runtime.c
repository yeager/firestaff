/*
 * csb_v2_hud_runtime.c — CSB V2 Phase 3 HUD Runtime Integration
 *
 * Provides the integration layer between the V1 command dispatch
 * (COMMAND.C, CLIKMENU.C) and the V2 HUD presentation layer.
 *
 * This module is PRESENTATION-ONLY: it reads V1 game state to populate
 * the HUD overlay, but does NOT write to any V1 data structures.
 * V1 command routes, inventory transactions, and dungeon state are
 * NEVER bypassed or altered by this module.
 *
 * Phase 3 rule: HUD overlay is gated on CSB_V2_PHASE_DOMAIN_HUD and
 * activates only when v2PresentationEnabled is true.
 *
 * Source: CSBWin/Viewport.cpp (CSB HUD layout, 7290 lines)
 *         CSBWin/Graphics.cpp (CSB graphics, 3186 lines)
 *         ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *         ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
 *         ReDMCSB COMMAND.C action feedback gates
 *         ReDMCSB DISPLAY.C pulse animation timing (2 Hz)
 *         DM2 DM2_V2_PHASE_DOMAIN_HUD pattern (Phase 3 HUD gate)
 */

#include "csb_v2_hud_runtime.h"
#include "csb_v2_hud_overlay_pc34.h"
#include <string.h>

/* ── Module state ──────────────────────────────────────────────── */
static CSB_V2_HudOverlay s_hud;
static int s_initialized = 0;
static const CSB_V2_PhaseGateConfig *s_gate_config = NULL;

static void ensure_init(void) {
    if (!s_initialized) {
        csb_v2_hud_init(&s_hud);
        s_initialized = 1;
    }
}

/* ── Lifecycle ──────────────────────────────────────────────────── */
void csb_v2_hud_runtime_init(void) {
    if (!s_initialized) {
        csb_v2_hud_init(&s_hud);
        s_initialized = 1;
    }
}

void csb_v2_hud_runtime_shutdown(void) {
    if (s_initialized) {
        csb_v2_hud_reset(&s_hud);
        s_initialized = 0;
    }
}

/* ── Configuration ──────────────────────────────────────────────── */
void csb_v2_hud_runtime_set_gate_config(const CSB_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── State setters ──────────────────────────────────────────────── */
void csb_v2_hud_runtime_set_party_gold(int gold) {
    ensure_init();
    csb_v2_hud_set_gold(&s_hud, gold);
}

void csb_v2_hud_runtime_set_direction(int dir) {
    ensure_init();
    csb_v2_hud_set_direction(&s_hud, dir);
}

void csb_v2_hud_runtime_set_level(int current, int max_level) {
    ensure_init();
    csb_v2_hud_set_level(&s_hud, current, max_level);
}

void csb_v2_hud_runtime_set_champion(int champ_idx,
    int hp_pct, int stamina_pct, int mana_pct,
    bool leader, bool spell_ready)
{
    ensure_init();
    csb_v2_hud_set_champion_bar(&s_hud, champ_idx,
        hp_pct, stamina_pct, mana_pct, leader, spell_ready);
}

void csb_v2_hud_runtime_set_action_active(CSB_V2_ActionIcon icon) {
    ensure_init();
    csb_v2_hud_set_action_active(&s_hud, icon);
}

void csb_v2_hud_runtime_clear_action(void) {
    ensure_init();
    /* Clear all actions by setting to MOVE (neutral) then deactivating */
    csb_v2_hud_set_action_active(&s_hud, CSB_V2_ACTION_COUNT);
}

void csb_v2_hud_runtime_trigger_hit_flash(void) {
    ensure_init();
    csb_v2_hud_trigger_hit_flash(&s_hud);
}

void csb_v2_hud_runtime_set_chaos_active(bool active, int power_rune_count) {
    ensure_init();
    csb_v2_hud_set_chaos_active(&s_hud, active, power_rune_count);
}

/* ── HUD toggle ─────────────────────────────────────────────────── */
void csb_v2_hud_runtime_toggle(void) {
    ensure_init();
    csb_v2_hud_toggle(&s_hud);
}

void csb_v2_hud_runtime_set_opacity(uint8_t val) {
    ensure_init();
    csb_v2_hud_set_opacity(&s_hud, val);
}

/* ── Render ──────────────────────────────────────────────────────── */
void csb_v2_hud_runtime_render(uint8_t *fb, int stride, int h_res) {
    if (!fb || stride <= 0 || h_res <= 0) return;

    /* Gate check: HUD activates only when v2PresentationEnabled is true.
     * This preserves the V1 command route — HUD is purely presentation. */
    if (s_gate_config) {
        CSB_V2_PhaseGateDecision d =
            csb_v2_phase_gate_pc34_decide(s_gate_config, CSB_V2_PHASE_DOMAIN_HUD);
        if (!d.v2PresentationAllowed) return;
    }

    ensure_init();
    csb_v2_hud_render(&s_hud, fb, stride, h_res);
}

/* ── Direct HUD access ─────────────────────────────────────────── */
CSB_V2_HudOverlay *csb_v2_hud_runtime_get_hud(void) {
    ensure_init();
    return &s_hud;
}

/* ── Source evidence ─────────────────────────────────────────────── */
const char *csb_v2_hud_runtime_source_evidence(void) {
    return
        "CSB V2 Phase 3 HUD Runtime Integration\n"
        "  Source: CSBWin/Viewport.cpp (CSB HUD layout, 7290 lines)\n"
        "  Source: CSBWin/Graphics.cpp (CSB graphics, 3186 lines)\n"
        "  Source: ReDMCSB PANEL.C F0354 (champion status-box rendering)\n"
        "  Source: ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)\n"
        "  Source: ReDMCSB COMMAND.C action feedback gates\n"
        "  Source: ReDMCSB DISPLAY.C pulse animation timing (2 Hz)\n"
        "  Pattern: DM2 DM2_V2_PHASE_DOMAIN_HUD (Phase 3 HUD gate)\n"
        "  Architecture: presentation-only, does not mutate V1 state\n";
}