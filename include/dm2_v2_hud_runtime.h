#ifndef FIRESTAFF_DM2_V2_HUD_RUNTIME_H
#define FIRESTAFF_DM2_V2_HUD_RUNTIME_H
#include "dm2_v2_hud_overlay.h"
#include "dm2_v2_phase_gate.h"
#include "dm2_v2_hud_widget_assets.h"
#include <stdbool.h>
#include <stdint.h>

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

typedef int (*DM2_V2_HudGdatFetch)(void *user,
    int key,
    const uint8_t **out_pixels,
    int *out_width,
    int *out_height,
    int *out_stride);
typedef int (*DM2_V2_HudGdatPaletteFetch)(void *user,
    int key,
    uint8_t out_palette16[16],
    uint32_t *out_palette_hash);
void dm2_v2_hud_runtime_set_gdat_source(DM2_V2_HudGdatFetch fetch,
    DM2_V2_HudGdatPaletteFetch palette_fetch,
    void *user,
    int mounted);

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

/* ── Asset-aware render (Phase 3 widget bitmap hook) ───────────── */
/* Per-slot path mode recorded by dm2_v2_hud_runtime_render_with_assets().
 * Mirrors the asset gate classification but uses a render-perspective
 * vocabulary. It is diagnostic-only: widget manifests do not own runtime
 * pixels. The runtime always draws authenticated original GDAT or no-draw.
 *
 * Every manifest classification maps to DM2_V2_HUD_RUNTIME_PATH_NO_DRAW.
 * A manifest's REAL label is not original-GDAT provenance and therefore
 * never authorizes a bitmap render path.
 *
 * A manifest classification is retained for tooling only; it cannot promote
 * generated, operator-provided, or placeholder pixels into the game. */
typedef enum {
    DM2_V2_HUD_RUNTIME_PATH_NO_DRAW             = 0,
    /* Compatibility spelling; no procedural renderer exists. */
    DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK =
        DM2_V2_HUD_RUNTIME_PATH_NO_DRAW,
    DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP         = 1
} DM2_V2_HudRuntimePathMode;

/* Asset-aware variant of dm2_v2_hud_runtime_render(). Walks the seven
 * Phase 3 / chrome-supporting widget slots (see dm2_v2_hud_widget_assets),
 * records the manifest classification per slot, then renders the HUD through
 * dm2_v2_hud_runtime_render()'s original-GDAT-only route.
 *
 * Phase-gated like the regular render: no-op when V2 launch/profile are
 * both off, when the HUD is hidden, or when opacity is 0.
 *
 * The recorded path-mode for slot S is also exposed by
 * dm2_v2_hud_runtime_last_path_mode(S) and the aggregate counts by
 * dm2_v2_hud_runtime_last_path_counts(&real,&fallback).
 *
 * Source-lock: SKULL.ASM T560 (DM2 HUD rendering pipeline),
 *              skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout),
 *              ReDMCSB PANEL.C F0354 (champion status-box drawing),
 *              include/dm2_v2_hud_widget_assets.h (slot gate). */
void dm2_v2_hud_runtime_render_with_assets(uint8_t *fb, int w, int h_res);

/* Returns the path-mode the runtime recorded for slot S during the most
 * recent dm2_v2_hud_runtime_render_with_assets() call. Returns
 * DM2_V2_HUD_RUNTIME_PATH_NO_DRAW if no render has run yet,
 * or if S is out of range. */
DM2_V2_HudRuntimePathMode dm2_v2_hud_runtime_last_path_mode(
    DM2_V2_HudWidgetSlot slot);

/* Aggregate path counts from the most recent render_with_assets() call.
 * Either out pointer may be NULL if the caller only needs the other count.
 * Returns the total slots classified (real + fallback); if no render has
 * run yet, both are 0 and the return is 0. */
int dm2_v2_hud_runtime_last_path_counts(int* out_real, int* out_fallback);

/* Returns the manifest classification the runtime observed for slot S during
 * the most recent render_with_assets() call. It is diagnostic only and does
 * not authorize a bitmap path.
 * Returns DM2_V2_HUD_WIDGET_CLASS_UNKNOWN if no render has run yet or
 * S is out of range. */
DM2_V2_HudWidgetClass dm2_v2_hud_runtime_last_slot_class(
    DM2_V2_HudWidgetSlot slot);

/* ── Status ────────────────────────────────────────────────────── */
/* Returns 1 if the HUD runtime is currently active (V2 enabled and
 * HUD toggled on).  Returns 0 if V1 is the active presentation, or
 * if the HUD is hidden. */
int dm2_v2_hud_runtime_is_active(void);

/* ── HUD fixture helper (for tests + wire-up probes) ───────────── */
/* This setter is compiled only for diagnostic targets. Production never
 * bypasses the V2 phase gate. */
#if defined(FIRESTAFF_DM2_V2_TESTING)
void dm2_v2_hud_runtime_force_active_for_test(int active);
#endif

/* Source evidence citation */
const char *dm2_v2_hud_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_HUD_RUNTIME_H */
