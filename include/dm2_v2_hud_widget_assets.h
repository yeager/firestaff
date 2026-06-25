/*
 * dm2_v2_hud_widget_assets.h — DM2 V2 HUD Widget Asset Manifest + Gate
 *
 * Phase 3: DM2 V2 HUD bitmap/widget asset discovery and classification gate.
 *
 * Scope (deliberately bounded, no finished-art claims):
 *   - Two widget slots explicitly named in docs/FIRESTAFF_GAP_LIST.md D2 V2
 *     Phase 3 (inventory quick-view, action prompt).
 *   - Five supporting chrome slots that the existing dm2_v2_hud_overlay
 *     pipeline already renders procedurally (compass rose, depth indicator,
 *     gold counter, champion-bar frame, action-strip frame). These keep
 *     the manifest aligned with the runtime, not the other way around.
 *   - A small manifest schema + classifier: PLACEHOLDER / PARTIAL / REAL /
 *     MISSING per slot.
 *   - An installed/partial gate mirroring the existing V2.2 modern-assets
 *     gate, accessible to M12 launcher status (M12_AssetStatus field).
 *
 * This module does NOT claim real PBR HUD widget art is shipped. It only:
 *   (a) catalogues what slots the Phase 3 overlay pipeline draws,
 *   (b) classifies whatever manifest is currently on disk,
 *   (c) makes the placeholder-vs-real distinction machine-checkable.
 *
 * Source:
 *   - SKULL.ASM T560 (DM2 HUD rendering pipeline)
 *   - skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - include/dm2_v22_modern_assets_pc34.h (sibling V2.2 manifest pattern)
 *   - docs/FIRESTAFF_GAP_LIST.md (D2 V2 Phase 3 row, widget bitmap row)
 */

#ifndef FIRESTAFF_DM2_V2_HUD_WIDGET_ASSETS_H
#define FIRESTAFF_DM2_V2_HUD_WIDGET_ASSETS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Widget slot enum ─────────────────────────────────────────────
 *
 * Two ordering groups:
 *   - DM2_V2_HUD_WIDGET_PHASE3_PRIMARY (the two slots explicitly named
 *     in docs/FIRESTAFF_GAP_LIST.md D2 V2 Phase 3 row).
 *   - DM2_V2_HUD_WIDGET_CHROME_SUPPORTING (slots the existing runtime
 *     already draws; classification parity for honest gap tracking).
 *
 * The enum is stable: ordinals double as table indices in
 * dm2_v2_hud_widget_assets_get_slot_classification(). Reordering or
 * inserting slots must keep DM2_V2_HUD_WIDGET_COUNT in sync and add a
 * `_SLOT_NAMES` entry in the .c file. */
typedef enum {
    DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW = 0,  /* Phase 3 primary */
    DM2_V2_HUD_WIDGET_ACTION_PROMPT        = 1,  /* Phase 3 primary */
    DM2_V2_HUD_WIDGET_COMPASS_ROSE         = 2,  /* chrome supporting */
    DM2_V2_HUD_WIDGET_DEPTH_INDICATOR      = 3,  /* chrome supporting */
    DM2_V2_HUD_WIDGET_GOLD_COUNTER         = 4,  /* chrome supporting */
    DM2_V2_HUD_WIDGET_CHAMPION_BAR_FRAME   = 5,  /* chrome supporting */
    DM2_V2_HUD_WIDGET_ACTION_STRIP_FRAME   = 6,  /* chrome supporting */
    DM2_V2_HUD_WIDGET_COUNT                = 7
} DM2_V2_HudWidgetSlot;

/* ── Classification ────────────────────────────────────────────── */
typedef enum {
    DM2_V2_HUD_WIDGET_CLASS_UNKNOWN    = 0, /* no manifest probed yet */
    DM2_V2_HUD_WIDGET_CLASS_MISSING    = 1, /* manifest valid but slot absent */
    DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER= 2, /* slot present, generator hint
                                              == "placeholder" (procedural) */
    DM2_V2_HUD_WIDGET_CLASS_PARTIAL    = 3, /* real asset metadata but some
                                              required fields missing */
    DM2_V2_HUD_WIDGET_CLASS_REAL       = 4  /* real PBR/PNG asset, all
                                              required fields present, source
                                              file resolves on disk */
} DM2_V2_HudWidgetClass;

/* ── Slot classification record (read-only view) ──────────────────
 *
 * Strings are owned by this struct (inline storage). Pass the address
 * to dm2_v2_hud_widget_assets_get_slot_info() and read the fields
 * afterwards — do not keep a pointer to them across other manifest
 * mutations. */
typedef struct {
    DM2_V2_HudWidgetSlot slot;
    char                 id[64];           /* manifest id (stable) */
    char                 category[32];     /* "hud_widgets" or sub-category */
    char                 generator[32];   /* "placeholder" / "pbr_hero" / "" */
    char                 source_file[256];/* manifest source_file or "" */
    char                 resolved_path[1024]; /* full path or "" */
    int                  width;
    int                  height;
    int                  file_exists;     /* 1 if resolved_path exists */
    DM2_V2_HudWidgetClass classification;
} DM2_V2_HudWidgetSlotInfo;

/* ── Manifest gate (overall) ────────────────────────────────────── */
typedef enum {
    DM2_V2_HUD_WIDGET_GATE_NOT_PROBED   = 0,
    DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST  = 1, /* manifest missing/unreadable */
    DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER  = 2, /* manifest valid, all slots
                                                PLACEHOLDER (procedural HUD
                                                rendering — current default) */
    DM2_V2_HUD_WIDGET_GATE_PARTIAL      = 3, /* some slots REAL, some
                                                PLACEHOLDER */
    DM2_V2_HUD_WIDGET_GATE_COMPLETE     = 4  /* all slots REAL */
} DM2_V2_HudWidgetGate;

/* ── Public API ─────────────────────────────────────────────────── */

/* Set the manifest root directory. Mirrors dm2_v22_set_manifest_path:
 *   dataDir is the DM2 game data dir (e.g. ~/.firestaff/data/dm2);
 *   the manifest is read from ~/.firestaff/assets/dm2/hud/ by walking
 *   up two parents and appending assets/dm2/hud/hud_widget_manifest.json.
 *
 * Passing NULL or "" clears the stored manifest path. */
void dm2_v2_hud_widget_assets_set_manifest_path(const char* dataDir);

/* Returns the resolved manifest path (read-only). May be empty if the
 * path has not been set. */
const char* dm2_v2_hud_widget_assets_get_manifest_path(void);

/* Validate the manifest at the given path (or the stored path if NULL).
 *
 * Returns:
 *   -1 — manifest missing, unreadable, or fundamentally invalid JSON
 *    0 — manifest readable but no slot entries recognised (still v22-shape
 *        fallback: not installed)
 *    1 — manifest valid, every declared slot's required fields are
 *        populated (id + generator + source_file + width + height)
 */
int dm2_v2_hud_widget_assets_validate_manifest(const char* manifest_path);

/* Classify the named slot using the stored manifest. Returns the
 * classification. If the manifest is missing or the slot is not present,
 * returns DM2_V2_HUD_WIDGET_CLASS_MISSING. */
DM2_V2_HudWidgetClass dm2_v2_hud_widget_assets_classify_slot(
    DM2_V2_HudWidgetSlot slot);

/* Fill out the slot info record for the given slot from the stored
 * manifest. Returns 1 on success, 0 if the slot is not present in the
 * manifest or the manifest is invalid. */
int dm2_v2_hud_widget_assets_get_slot_info(DM2_V2_HudWidgetSlot slot,
                                            DM2_V2_HudWidgetSlotInfo* out);

/* Compute the overall manifest gate. Reads the stored manifest and
 * classifies every declared slot. */
DM2_V2_HudWidgetGate dm2_v2_hud_widget_assets_gate(void);

/* Returns the human-readable name of a slot. Stable; useful for
 * diagnostics, manifest IDs, and probe output. */
const char* dm2_v2_hud_widget_assets_slot_name(DM2_V2_HudWidgetSlot slot);

/* Returns the human-readable name of a classification. */
const char* dm2_v2_hud_widget_assets_class_name(DM2_V2_HudWidgetClass cls);

/* Returns the human-readable name of a gate state. */
const char* dm2_v2_hud_widget_assets_gate_name(DM2_V2_HudWidgetGate gate);

/* Counts slots classified as REAL across the stored manifest. Returns
 * the count and (optionally) the total declared slot count via out_total.
 * If the manifest is missing or invalid, returns 0 and sets *out_total
 * to 0. */
int dm2_v2_hud_widget_assets_real_count(int* out_total);

/* Installed flag mirror — for M12_AssetStatus and Phase 7 verification
 * suite integration. dm2_v2_hud_widget_assets_set_installed() is the
 * last value seen from dm2_v2_hud_widget_assets_gate(); the helper
 * computes "PARTIAL or COMPLETE" → 1, everything else → 0. */
void dm2_v2_hud_widget_assets_set_installed(int installed);
int  dm2_v2_hud_widget_assets_get_installed(void);

/* Convenience: should the runtime use placeholder rendering for the
 * requested slot? Returns 1 if the slot classification is
 * DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER, MISSING, UNKNOWN, or PARTIAL,
 * 0 only if the slot is fully REAL. */
int dm2_v2_hud_widget_assets_uses_placeholder(DM2_V2_HudWidgetSlot slot);

/* Source evidence citation for source-lock tests. */
const char* dm2_v2_hud_widget_assets_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_HUD_WIDGET_ASSETS_H */
