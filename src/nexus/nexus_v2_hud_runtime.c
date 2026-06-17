/*
 * nexus_v2_hud_runtime.c — Nexus V2 Phase 3 HUD Runtime Integration
 *
 * V1→V2 HUD bridge for Dungeon Master Nexus (Saturn).
 * Mirrors dm2_v2_hud_runtime.c pattern (sibling CSB V2 wire-up).
 *
 * Source: Saturn NEXUS.BIN HUD surface data, DMDF/DGN level format,
 *         Saturn SDK VDP1/VDP2 layer documentation,
 *         ReDMCSB PANEL.C / DUNGEON.C (champion status refresh)
 */

#include "nexus_v2_hud_runtime.h"
#include "nexus_v2_hud_overlay.h"
#include "nexus_v2_phase_gate_pc34.h"
#include <string.h>

/* ── Module state ──────────────────────────────────────────────── */
static Nexus_V2_HudOverlay s_hud;
static int s_initialized = 0;
static const NEXUS_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;

static void ensure_init(void) {
    if (!s_initialized) {
        nexus_v2_hud_init(&s_hud);
        s_initialized = 1;
    }
}

/* ── Lifecycle ──────────────────────────────────────────────────── */
void nexus_v2_hud_runtime_init(void) {
    if (!s_initialized) {
        nexus_v2_hud_init(&s_hud);
        s_initialized = 1;
    }
    s_force_active = 0;
}

void nexus_v2_hud_runtime_shutdown(void) {
    if (s_initialized) {
        nexus_v2_hud_reset(&s_hud);
        s_initialized = 0;
    }
    s_gate_config = NULL;
    s_force_active = 0;
}

/* ── Configuration ──────────────────────────────────────────────── */
void nexus_v2_hud_runtime_set_gate_config(const NEXUS_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── State setters ──────────────────────────────────────────────── */
void nexus_v2_hud_runtime_set_party_gold(int gold_pieces) {
    ensure_init();
    nexus_v2_hud_set_gold(&s_hud, gold_pieces);
}

void nexus_v2_hud_runtime_set_direction(int dir) {
    ensure_init();
    nexus_v2_hud_set_direction(&s_hud, dir);
}

void nexus_v2_hud_runtime_set_level(int cur, int max) {
    ensure_init();
    nexus_v2_hud_set_level(&s_hud, cur, max);
}

void nexus_v2_hud_runtime_set_champion(int champ_idx, int hp_pct,
    int stamina_pct, int mana_pct, bool leader, bool spell_ready)
{
    ensure_init();
    nexus_v2_hud_set_champion_bar(&s_hud, champ_idx, hp_pct, stamina_pct,
        mana_pct, leader, spell_ready);
}

void nexus_v2_hud_runtime_set_action_active(Nexus_V2_ActionIcon icon) {
    ensure_init();
    nexus_v2_hud_set_action_active(&s_hud, icon);
}

void nexus_v2_hud_runtime_trigger_hit_flash(void) {
    ensure_init();
    nexus_v2_hud_trigger_hit_flash(&s_hud);
}

void nexus_v2_hud_runtime_set_opacity(uint8_t val) {
    ensure_init();
    nexus_v2_hud_set_opacity(&s_hud, val);
}

/* ── Gated render ──────────────────────────────────────────────── */
void nexus_v2_hud_runtime_render(uint8_t *fb, int w, int h_res) {
    if (!s_initialized) return;
    if (!s_force_active && !s_hud.visible) return;
    if (s_hud.opacity == 0) return;
    if (!s_force_active) {
        if (!s_gate_config) return;
        if (!s_gate_config->v2PresentationEnabled) return;
        if (!s_gate_config->v2ConfigPersistenceEnabled) return;
    }
    nexus_v2_hud_render(&s_hud, fb, w, h_res);
}

/* ── Status ────────────────────────────────────────────────────── */
int nexus_v2_hud_runtime_is_active(void) {
    if (!s_initialized) return 0;
    if (s_force_active) return 1;
    if (!s_hud.visible) return 0;
    if (!s_gate_config) return 0;
    if (!s_gate_config->v2PresentationEnabled) return 0;
    if (!s_gate_config->v2ConfigPersistenceEnabled) return 0;
    return 1;
}

/* ── V1 compatibility helper ──────────────────────────────────── */
void nexus_v2_hud_runtime_force_active_for_test(int active) {
    s_force_active = active ? 1 : 0;
}

const char *nexus_v2_hud_runtime_source_evidence(void) {
    return
        "Nexus V2 HUD Runtime — Phase 3 source-lock\n"
        "Source: Saturn NEXUS.BIN HUD surface data\n"
        "Source: DMDF/DGN level format (Saturn DMDF VDP1/VDP2 layers)\n"
        "Source: ReDMCSB PANEL.C / DUNGEON.C (champion status refresh)\n"
        "Source: csb_v2_hud_runtime.c (sibling CSB V2 wire-up pattern)\n"
        "Source: dm2_v2_hud_runtime.c (sibling DM2 V2 wire-up pattern)\n"
        "V1 invariant: V1 command routes, inventory, dungeon state NEVER bypassed\n"
        "V2 rule: HUD only active when v2PresentationEnabled AND v2ConfigPersistenceEnabled are both 1\n"
        "V2 rule: HUD render is no-op when V1 is active, no framebuffer pollution\n";
}