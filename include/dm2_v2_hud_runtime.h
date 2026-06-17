#ifndef FIRESTAFF_DM2_V2_HUD_RUNTIME_H
#define FIRESTAFF_DM2_V2_HUD_RUNTIME_H
#include "dm2_v2_hud_overlay.h"
#include "dm2_v2_phase_gate.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * dm2_v2_hud_runtime.h — DM2 V2 Phase 3 HUD Runtime Integration
 *
 * Provides the integration layer between the DM2 V1 command dispatch
 * (SKULL.ASM T520/T048) and the DM2 V2 HUD presentation layer.
 *
 * This module is PRESENTATION-ONLY: it reads V1 game state to populate
 * the HUD overlay, but does NOT write to any V1 data structures.
 * V1 command routes, inventory transactions, and dungeon state are
 * NEVER bypassed or altered by this module.
 *
 * Phase 3 rule: HUD overlay is gated on DM2_V2_PHASE_DOMAIN_HUD and
 * activates only when both v2LaunchEnabled and v2ProfileEnabled are
 * true. The overlay renders into the supplied framebuffer without
 * altering V1 state.
 *
 * Architecture (mirrors csb_v2_hud_runtime):
 *   dm2_v2_hud_runtime_init()           — init module + HUD state
 *   dm2_v2_hud_runtime_shutdown()       — reset HUD state
 *   dm2_v2_hud_runtime_set_gate_config() — bind phase gate
 *   dm2_v2_hud_runtime_set_party_gold() — set party gold pieces
 *   dm2_v2_hud_runtime_set_direction()  — set party direction 0..3
 *   dm2_v2_hud_runtime_set_level()      — set current/max level
 *   dm2_v2_hud_runtime_set_champion()   — set champion bar stats
 *   dm2_v2_hud_runtime_set_action_active() — highlight action icon
 *   dm2_v2_hud_runtime_trigger_hit_flash()
 *   dm2_v2_hud_runtime_render()         — gated render into framebuffer
 *   dm2_v2_hud_runtime_is_active()      — true if presentation is enabled
 *
 * Source: SKULL.ASM T560 (DM2 HUD rendering)
 *         skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *         ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *         ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
 *         ReDMCSB COMMAND.C action feedback gates
 *         ReDMCSB DISPLAY.C pulse animation timing (2 Hz)
 *         csb_v2_hud_runtime.c (sibling CSB V2 wire-up pattern)
 * ================================================================ */

/* ── Lifecycle ─────────────────────────────────────────────────── */
void dm2_v2_hud_runtime_init(void);
void dm2_v2_hud_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
void dm2_v2_hud_runtime_set_gate_config(const DM2_V2_PhaseGateConfig *config);

/* ── State setters (V1 → V2 HUD bridge) ────────────────────────── */
void dm2_v2_hud_runtime_set_party_gold(int gold_pieces);
void dm2_v2_hud_runtime_set_direction(int dir);
void dm2_v2_hud_runtime_set_level(int cur, int max);
void dm2_v2_hud_runtime_set_champion(int champ_idx, int hp_pct,
    int stamina_pct, int mana_pct, bool leader, bool spell_ready);
void dm2_v2_hud_runtime_set_action_active(DM2_V2_ActionIcon icon);
void dm2_v2_hud_runtime_trigger_hit_flash(void);
void dm2_v2_hud_runtime_set_opacity(uint8_t val);

/* ── Gated render ──────────────────────────────────────────────── */
/* Renders the HUD overlay into the supplied 320×200 framebuffer.
 * If v2PresentationEnabled is false, this is a no-op (V1 untouched).
 * If the HUD is not visible, this is a no-op.
 * Source-lock: SKULL.ASM T560 (DM2 HUD rendering). */
void dm2_v2_hud_runtime_render(uint8_t *fb, int w, int h_res);

/* ── Status ────────────────────────────────────────────────────── */
/* Returns 1 if the HUD runtime is currently active (V2 enabled and
 * HUD toggled on).  Returns 0 if V1 is the active presentation, or
 * if the HUD is hidden. */
int dm2_v2_hud_runtime_is_active(void);

/* ── HUD V1 compatibility helper (for tests + wire-up probes) ──── */
/* Force-activates the HUD regardless of phase gate (used by the
 * wire-up probe to verify the HUD data flow).  Not called by
 * production code. */
void dm2_v2_hud_runtime_force_active_for_test(int active);

/* Source evidence citation */
const char *dm2_v2_hud_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_HUD_RUNTIME_H */
