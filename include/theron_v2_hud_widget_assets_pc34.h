/*
 * theron_v2_hud_widget_assets_pc34.h — Theron V2 HUD Widget Asset Manifest + Gate
 *
 * Phase 3: Theron V2 HUD bitmap/widget asset discovery and
 * classification gate.  Mirrors the DM2 widget-asset gate
 * (include/dm2_v2_hud_widget_assets.h) with Theron-specific slots.
 *
 * Scope (deliberately bounded, no finished-art claims):
 *   - Seven widget slots aligned with the runtime surface defined by
 *     src/theron/theron_v2_hud_overlay_pc34.c:
 *       * compass_rose         (top-bar, 4-way cardinal)
 *       * quest_items          (top-bar, "Qx/Qy" counter)
 *       * dungeon_progress     (top-bar, "Dn/7" indicator)
 *       * relic_counter        (top-bar, "Rr/7" indicator)
 *       * rune_indicator       (top-bar, 4-slot spell-rune ready)
 *       * champion_bars        (bottom-panel, 4-champion HP/Stam/Mana)
 *       * action_strip         (bottom strip, ATK/CST/USE/DRP/MOV)
 *   - A small manifest schema + classifier:
 *     MISSING / PLACEHOLDER / PARTIAL / REAL per slot.
 *   - An installed/partial gate mirroring the existing V2.2 modern-
 *     assets gate (theron_v22_modern_assets_pc34) and the DM2 V2 HUD
 *     widget gate.
 *
 * This module does NOT claim real PBR HUD widget art is shipped.
 * It only:
 *   (a) catalogues what slots the Phase 3 overlay pipeline draws,
 *   (b) classifies whatever manifest is currently on disk,
 *   (c) makes the placeholder-vs-real distinction machine-checkable
 *       so M12 launcher status, the Phase 3 gap row, and the
 *       CI verification suite can promote the gate as real assets
 *       are added.
 *
 * Source:
 *   - THQUEST.ASM T520  (party placement / start position)
 *   - THQUEST.ASM T560  (dungeon loading)
 *   - THQUEST.ASM T600  (UI overlay zones: top-bar / right / bottom)
 *   - THQUEST.ASM T700  (timers / world tick)
 *   - THQUEST.ASM T800  (champion persistence + inventory reset)
 *   - THQUEST.ASM T900  (object database / rune magic)
 *   - HuC6260 / HuC6270 datasheet (PC Engine VDC + VCE)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
 *   - dmweb Theron overview (7 dungeons + 7 relic goals + rune magic)
 *   - docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 *   - src/theron/theron_v2_hud_overlay_pc34.c (procedural fallback)
 *   - include/theron_v22_modern_assets_pc34.h (sibling V2.2 manifest)
 *   - include/dm2_v2_hud_widget_assets.h (sibling Phase 3 gate pattern)
 *   - docs/FIRESTAFF_GAP_LIST.md Theron V2 Phase 3 row
 */

#ifndef FIRESTAFF_THERON_V2_HUD_WIDGET_ASSETS_PC34_H
#define FIRESTAFF_THERON_V2_HUD_WIDGET_ASSETS_PC34_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Widget slot enum ─────────────────────────────────────────────
 *
 * Stable ordering: ordinals double as table indices in
 * theron_v2_hud_widget_assets_get_slot_classification().
 *
 * THERON_V2_HUD_WIDGET_COUNT must remain last and equal the table
 * length in src/theron/theron_v2_hud_widget_assets_pc34.c.
 *
 * THERON_V2_HUD_WIDGET_PHASE3_PRIMARY  — slots added by Phase 3
 *                                          (top-bar / right / bottom
 *                                          enhancements not present in
 *                                          the V1 UI chrome).
 * THERON_V2_HUD_WIDGET_CHROME_SUPPORTING — slots the V1 chrome already
 *                                          draws; classification parity
 *                                          for honest gap tracking so a
 *                                          finished PBR sprite pack
 *                                          can replace the procedural
 *                                          rectangles. */
typedef enum {
    THERON_V2_HUD_WIDGET_COMPASS_ROSE        = 0,  /* Phase 3 primary */
    THERON_V2_HUD_WIDGET_QUEST_ITEMS         = 1,  /* Phase 3 primary */
    THERON_V2_HUD_WIDGET_DUNGEON_PROGRESS    = 2,  /* Phase 3 primary */
    THERON_V2_HUD_WIDGET_RELIC_COUNTER       = 3,  /* Phase 3 primary */
    THERON_V2_HUD_WIDGET_RUNE_INDICATOR      = 4,  /* Phase 3 primary */
    THERON_V2_HUD_WIDGET_CHAMPION_BARS       = 5,  /* chrome supporting */
    THERON_V2_HUD_WIDGET_ACTION_STRIP        = 6,  /* chrome supporting */
    THERON_V2_HUD_WIDGET_COUNT               = 7
} Theron_V2_HudWidgetSlot;

/* ── Classification ────────────────────────────────────────────── */
typedef enum {
    THERON_V2_HUD_WIDGET_CLASS_UNKNOWN    = 0, /* no manifest probed yet */
    THERON_V2_HUD_WIDGET_CLASS_MISSING    = 1, /* manifest valid but slot absent */
    THERON_V2_HUD_WIDGET_CLASS_PLACEHOLDER= 2, /* slot present, generator hint
                                                  == "placeholder" (procedural) */
    THERON_V2_HUD_WIDGET_CLASS_PARTIAL    = 3, /* real asset metadata but some
                                                  required fields missing */
    THERON_V2_HUD_WIDGET_CLASS_REAL       = 4  /* real PBR/PNG asset, all
                                                  required fields present,
                                                  source_file resolves on disk */
} Theron_V2_HudWidgetClass;

/* ── Slot classification record (read-only view) ──────────────────
 *
 * Strings are owned by this struct (inline storage). Pass the address
 * to theron_v2_hud_widget_assets_get_slot_info() and read the fields
 * afterwards — do not keep a pointer to them across other manifest
 * mutations. */
typedef struct {
    Theron_V2_HudWidgetSlot slot;
    char                    id[64];           /* manifest id (stable) */
    char                    category[32];     /* "hud_widgets" or sub-category */
    char                    generator[32];    /* "placeholder" / "pbr_hero" / "" */
    char                    source_file[256]; /* manifest source_file or "" */
    char                    resolved_path[1024]; /* full path or "" */
    int                     width;
    int                     height;
    int                     file_exists;      /* 1 if resolved_path exists */
    Theron_V2_HudWidgetClass classification;
} Theron_V2_HudWidgetSlotInfo;

/* ── Manifest gate (overall) ────────────────────────────────────── */
typedef enum {
    THERON_V2_HUD_WIDGET_GATE_NOT_PROBED   = 0,
    THERON_V2_HUD_WIDGET_GATE_NO_MANIFEST  = 1, /* manifest missing/unreadable */
    THERON_V2_HUD_WIDGET_GATE_PLACEHOLDER  = 2, /* manifest valid, all slots
                                                    PLACEHOLDER (procedural HUD
                                                    rendering — current default) */
    THERON_V2_HUD_WIDGET_GATE_PARTIAL      = 3, /* some slots REAL, some
                                                    PLACEHOLDER */
    THERON_V2_HUD_WIDGET_GATE_COMPLETE     = 4  /* all slots REAL */
} Theron_V2_HudWidgetGate;

/* ── Public API ─────────────────────────────────────────────────── */

/* Set the manifest root directory. Mirrors
 * theron_v22_set_manifest_path:
 *   dataDir is the Theron game data dir (e.g. ~/.firestaff/data/theron);
 *   the manifest is read from
 *     ~/.firestaff/assets/theron/hud/hud_widget_manifest.json
 *   by walking up two parents and appending
 *   assets/theron/hud/hud_widget_manifest.json.
 *
 * Passing NULL or "" clears the stored manifest path. */
void theron_v2_hud_widget_assets_set_manifest_path(const char* dataDir);

/* Returns the resolved manifest path (read-only). May be empty if the
 * path has not been set. */
const char* theron_v2_hud_widget_assets_get_manifest_path(void);

/* Validate the manifest at the given path (or the stored path if NULL).
 *
 * Returns:
 *   -1 — manifest missing, unreadable, or fundamentally invalid JSON
 *    0 — manifest readable but no slot entries recognised (still v22-shape
 *        fallback: not installed)
 *    1 — manifest valid, every declared slot's required fields are
 *        populated (id + generator + source_file + width + height) */
int theron_v2_hud_widget_assets_validate_manifest(const char* manifest_path);

/* Classify the named slot using the stored manifest. Returns the
 * classification. If the manifest is missing or the slot is not present,
 * returns THERON_V2_HUD_WIDGET_CLASS_MISSING. */
Theron_V2_HudWidgetClass theron_v2_hud_widget_assets_classify_slot(
    Theron_V2_HudWidgetSlot slot);

/* Fill out the slot info record for the given slot from the stored
 * manifest. Returns 1 on success, 0 if the slot is not present in the
 * manifest or the manifest is invalid. */
int theron_v2_hud_widget_assets_get_slot_info(Theron_V2_HudWidgetSlot slot,
                                              Theron_V2_HudWidgetSlotInfo* out);

/* Compute the overall manifest gate. Reads the stored manifest and
 * classifies every declared slot. */
Theron_V2_HudWidgetGate theron_v2_hud_widget_assets_gate(void);

/* Returns the human-readable name of a slot. Stable; useful for
 * diagnostics, manifest IDs, and probe output. */
const char* theron_v2_hud_widget_assets_slot_name(Theron_V2_HudWidgetSlot slot);

/* Returns the human-readable name of a classification. */
const char* theron_v2_hud_widget_assets_class_name(Theron_V2_HudWidgetClass cls);

/* Returns the human-readable name of a gate state. */
const char* theron_v2_hud_widget_assets_gate_name(Theron_V2_HudWidgetGate gate);

/* Counts slots classified as REAL across the stored manifest. Returns
 * the count and (optionally) the total declared slot count via
 * out_total. If the manifest is missing or invalid, returns 0 and sets
 * *out_total to 0. */
int theron_v2_hud_widget_assets_real_count(int* out_total);

/* Installed flag mirror — for M12_AssetStatus and Phase 7 verification
 * suite integration. theron_v2_hud_widget_assets_set_installed() is
 * the last value seen from theron_v2_hud_widget_assets_gate(); the
 * helper computes "PARTIAL or COMPLETE" → 1, everything else → 0. */
void theron_v2_hud_widget_assets_set_installed(int installed);
int  theron_v2_hud_widget_assets_get_installed(void);

/* Convenience: should the runtime use placeholder rendering for the
 * requested slot? Returns 1 if the slot classification is
 * THERON_V2_HUD_WIDGET_CLASS_PLACEHOLDER, MISSING, UNKNOWN, or
 * PARTIAL, 0 only if the slot is fully REAL. */
int theron_v2_hud_widget_assets_uses_placeholder(Theron_V2_HudWidgetSlot slot);

/* Source evidence citation for source-lock tests. */
const char* theron_v2_hud_widget_assets_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_HUD_WIDGET_ASSETS_PC34_H */
