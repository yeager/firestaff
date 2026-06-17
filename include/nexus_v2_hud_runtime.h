#ifndef FIRESTAFF_NEXUS_V2_HUD_RUNTIME_H
#define FIRESTAFF_NEXUS_V2_HUD_RUNTIME_H
#include "nexus_v2_hud_overlay.h"
#include "nexus_v2_phase_gate_pc34.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * nexus_v2_hud_runtime.h — Nexus V2 Phase 3 HUD Runtime Integration
 *
 * V1→V2 HUD bridge for Dungeon Master Nexus (Saturn).
 * Mirrors dm2_v2_hud_runtime.c pattern.
 *
 * This module is PRESENTATION-ONLY: it reads V1 game state to populate
 * the HUD overlay, but does NOT write to any V1 data structures.
 * V1 command routes, inventory transactions, and dungeon state are
 * NEVER bypassed or altered by this module.
 *
 * Phase 3 rule: HUD overlay is gated on the Nexus V2 phase gate and
 * activates only when both v2LaunchEnabled and v2ProfileEnabled are
 * true. The overlay renders into the supplied framebuffer without
 * altering V1 state.
 *
 * Source: Saturn NEXUS.BIN HUD surface data, DMDF/DGN level format,
 *         Saturn SDK VDP1/VDP2 layer documentation,
 *         ReDMCSB PANEL.C / DUNGEON.C (champion status refresh)
 * ================================================================ */

/* ── Lifecycle ─────────────────────────────────────────────────── */
void nexus_v2_hud_runtime_init(void);
void nexus_v2_hud_runtime_shutdown(void);

/* ── Configuration ─────────────────────────────────────────────── */
void nexus_v2_hud_runtime_set_gate_config(const NEXUS_V2_PhaseGateConfig *config);

/* ── State setters (V1 → V2 HUD bridge) ────────────────────────── */
void nexus_v2_hud_runtime_set_party_gold(int gold_pieces);
void nexus_v2_hud_runtime_set_direction(int dir);
void nexus_v2_hud_runtime_set_level(int cur, int max);
void nexus_v2_hud_runtime_set_champion(int champ_idx, int hp_pct,
    int stamina_pct, int mana_pct, bool leader, bool spell_ready);
void nexus_v2_hud_runtime_set_action_active(Nexus_V2_ActionIcon icon);
void nexus_v2_hud_runtime_trigger_hit_flash(void);
void nexus_v2_hud_runtime_set_opacity(uint8_t val);

/* ── Gated render ──────────────────────────────────────────────── */
void nexus_v2_hud_runtime_render(uint8_t *fb, int w, int h_res);

/* ── Status ────────────────────────────────────────────────────── */
int nexus_v2_hud_runtime_is_active(void);

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
void nexus_v2_hud_runtime_force_active_for_test(int active);

/* Source evidence citation */
const char *nexus_v2_hud_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V2_HUD_RUNTIME_H */