/*
 * dm2_v2_hud_runtime.c — DM2 V2 Phase 3 HUD Runtime Integration
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
 * Source: SKULL.ASM T560 (DM2 HUD rendering)
 *         skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *         ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *         ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
 *         ReDMCSB COMMAND.C action feedback gates
 *         ReDMCSB DISPLAY.C pulse animation timing (2 Hz)
 *         csb_v2_hud_runtime.c (sibling CSB V2 wire-up pattern)
 */

#include "dm2_v2_hud_runtime.h"
#include <string.h>

/* ── Module state ──────────────────────────────────────────────── */
static DM2_V2_HudOverlay s_hud;
static int s_initialized = 0;
static const DM2_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;  /* 0 = phase-gated, 1 = always on (test only) */

static void ensure_init(void) {
    if (!s_initialized) {
        dm2_v2_hud_init(&s_hud);
        s_initialized = 1;
    }
}

/* ── Lifecycle ──────────────────────────────────────────────────── */
void dm2_v2_hud_runtime_init(void) {
    if (!s_initialized) {
        dm2_v2_hud_init(&s_hud);
        s_initialized = 1;
    }
    s_force_active = 0;
}

void dm2_v2_hud_runtime_shutdown(void) {
    if (s_initialized) {
        dm2_v2_hud_reset(&s_hud);
        s_initialized = 0;
    }
    s_gate_config = NULL;
    s_force_active = 0;
}

/* ── Configuration ──────────────────────────────────────────────── */
void dm2_v2_hud_runtime_set_gate_config(const DM2_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── State setters (V1 → V2 HUD bridge) ────────────────────────── */
void dm2_v2_hud_runtime_set_party_gold(int gold_pieces) {
    ensure_init();
    dm2_v2_hud_set_gold(&s_hud, gold_pieces);
}

void dm2_v2_hud_runtime_set_direction(int dir) {
    ensure_init();
    dm2_v2_hud_set_direction(&s_hud, dir);
}

void dm2_v2_hud_runtime_set_level(int cur, int max) {
    ensure_init();
    dm2_v2_hud_set_level(&s_hud, cur, max);
}

void dm2_v2_hud_runtime_set_champion(int champ_idx, int hp_pct,
    int stamina_pct, int mana_pct, bool leader, bool spell_ready)
{
    ensure_init();
    dm2_v2_hud_set_champion_bar(&s_hud, champ_idx, hp_pct, stamina_pct,
        mana_pct, leader, spell_ready);
}

void dm2_v2_hud_runtime_set_action_active(DM2_V2_ActionIcon icon) {
    ensure_init();
    dm2_v2_hud_set_action_active(&s_hud, icon);
}

void dm2_v2_hud_runtime_trigger_hit_flash(void) {
    ensure_init();
    dm2_v2_hud_trigger_hit_flash(&s_hud);
}

void dm2_v2_hud_runtime_set_opacity(uint8_t val) {
    ensure_init();
    dm2_v2_hud_set_opacity(&s_hud, val);
}

/* ── Gated render ──────────────────────────────────────────────── */
void dm2_v2_hud_runtime_render(uint8_t *fb, int w, int h_res) {
    if (!s_initialized) return;
    if (!s_force_active && !s_hud.visible) return;
    if (s_hud.opacity == 0) return;
    if (!s_force_active) {
        /* Phase gate: HUD is presentation-only, requires V2 launch+profile
         * both enabled.  When V1 is active, the HUD is hidden (V1 chrome
         * owns the framebuffer). */
        if (!s_gate_config) return;
        if (!s_gate_config->v2LaunchEnabled) return;
        if (!s_gate_config->v2ProfileEnabled) return;
    }
    dm2_v2_hud_render(&s_hud, fb, w, h_res);
}

/* ── Status ────────────────────────────────────────────────────── */
int dm2_v2_hud_runtime_is_active(void) {
    if (!s_initialized) return 0;
    if (s_force_active) return 1;
    if (!s_hud.visible) return 0;
    if (!s_gate_config) return 0;
    if (!s_gate_config->v2LaunchEnabled) return 0;
    if (!s_gate_config->v2ProfileEnabled) return 0;
    return 1;
}

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
void dm2_v2_hud_runtime_force_active_for_test(int active) {
    s_force_active = active ? 1 : 0;
}

const char *dm2_v2_hud_runtime_source_evidence(void) {
    return
        "DM2 V2 HUD Runtime — Phase 3 source-lock\n"
        "ReDMCSB SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: SKULL.ASM T560              (DM2 HUD rendering pipeline)\n"
        "Source: SKULL.ASM T520              (party/movement tick → HUD state source)\n"
        "Source: SKULL.ASM T048              (input dispatch → action strip source)\n"
        "Source: skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout, sibling reimpl)\n"
        "Source: ReDMCSB PANEL.C F0354       (champion status-box drawing)\n"
        "Source: ReDMCSB DUNGEON.C F0260     (stat-bar refresh timing)\n"
        "Source: ReDMCSB COMMAND.C           (action feedback gates)\n"
        "Source: ReDMCSB DISPLAY.C           (pulse animation timing 2 Hz)\n"
        "Source: dm2_v2_phase_gate.h         (DM2_V2_PHASE_DOMAIN_HUD gate)\n"
        "Source: csb_v2_hud_runtime.c        (sibling CSB V2 wire-up pattern)\n"
        "V1 invariant: V1 command routes, inventory, dungeon state NEVER bypassed\n"
        "V2 rule: HUD only active when v2LaunchEnabled AND v2ProfileEnabled are both 1\n"
        "V2 rule: HUD render is no-op when V1 is active, no framebuffer pollution\n";
}
