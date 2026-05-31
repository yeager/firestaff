#ifndef FIRESTAFF_CSB_V2_HUD_RUNTIME_H
#define FIRESTAFF_CSB_V2_HUD_RUNTIME_H
#include "csb_v2_hud_overlay_pc34.h"
#include "csb_v2_phase_gate_pc34.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * csb_v2_hud_runtime.h — CSB V2 Phase 3 HUD Runtime Integration
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
 * activates only when v2PresentationEnabled is true. The overlay
 * renders into the supplied framebuffer without altering V1 state.
 *
 * Usage:
 *   csb_v2_hud_runtime_init();
 *   csb_v2_hud_runtime_set_party_gold(gold);
 *   csb_v2_hud_runtime_set_direction(dir);
 *   csb_v2_hud_runtime_set_level(cur, max);
 *   csb_v2_hud_runtime_set_champion(idx, hp, stamina, mana, leader, spell_ready);
 *   csb_v2_hud_runtime_set_action_active(icon);
 *   csb_v2_hud_runtime_trigger_hit_flash();
 *   csb_v2_hud_runtime_set_chaos_active(active, rune_count);
 *   csb_v2_hud_render(fb, w, h_res);   // uses current phase gate config
 *
 * Source: CSBWin/Viewport.cpp (CSB HUD layout, 7290 lines)
 *         CSBWin/Graphics.cpp (CSB graphics, 3186 lines)
 *         ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *         ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
 *         ReDMCSB COMMAND.C action feedback gates
 *         ReDMCSB DISPLAY.C pulse animation timing (2 Hz)
 *         DM2 DM2_V2_PHASE_DOMAIN_HUD pattern (Phase 3 HUD gate)
 * ================================================================ */

/* ── Lifecycle ─────────────────────────────────────────────────── */
void csb_v2_hud_runtime_init(void);
void csb_v2_hud_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
/* Set the phase gate config used by the HUD runtime.
 * Call this before csb_v2_hud_runtime_render() to ensure correct gating. */
void csb_v2_hud_runtime_set_gate_config(const CSB_V2_PhaseGateConfig *config);

/* ── State setters (presentation-only) ─────────────────────────── */
/* Party gold */
void csb_v2_hud_runtime_set_party_gold(int gold);

/* Compass direction (0=N, 1=E, 2=S, 3=W) */
void csb_v2_hud_runtime_set_direction(int dir);

/* Dungeon depth level */
void csb_v2_hud_runtime_set_level(int current, int max_level);

/* Champion bar — called for each of 4 champions */
void csb_v2_hud_runtime_set_champion(int champ_idx,
    int hp_pct, int stamina_pct, int mana_pct,
    bool leader, bool spell_ready);

/* Action strip — highlight one action icon */
void csb_v2_hud_runtime_set_action_active(CSB_V2_ActionIcon icon);
void csb_v2_hud_runtime_clear_action(void);

/* Hit flash — one-shot trigger for action feedback */
void csb_v2_hud_runtime_trigger_hit_flash(void);

/* Chaos magic indicator (CSB-specific DSA active + power runes) */
void csb_v2_hud_runtime_set_chaos_active(bool active, int power_rune_count);

/* ── HUD toggle ────────────────────────────────────────────────── */
void csb_v2_hud_runtime_toggle(void);
void csb_v2_hud_runtime_set_opacity(uint8_t val);

/* ── Render ─────────────────────────────────────────────────────── */
/* Render the HUD overlay into the supplied 320×200 framebuffer.
 * Respects the phase gate: when v2PresentationEnabled is false,
 * this is a no-op. Does NOT mutate any V1 state. */
void csb_v2_hud_runtime_render(uint8_t *fb, int stride, int h_res);

/* ── Direct HUD access ─────────────────────────────────────────── */
/* Get the raw HUD state for advanced callers.
 * Returns NULL if HUD is not initialised. */
CSB_V2_HudOverlay *csb_v2_hud_runtime_get_hud(void);

/* ── V1 compatibility seam ─────────────────────────────────────── */
const char *csb_v2_hud_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_HUD_RUNTIME_H */