/*
 * dm1_v22_finished_art_material_gate_pc34.h
 *
 * DM1 V2.2 finished-art / material screenshot pixel gate.
 *
 * The V2.2 modern-asset pack at ~/.firestaff/assets/dm1/modern/ carries
 * per-shape material PNGs (wall/floor/creature/door/champion). When the
 * pack is shipped it is procedurally generated (placeholder colors) — the
 * honest baseline for this pass. When a reviewer has signed off on real
 * finished art, the manifest entries for those slots carry:
 *
 *   generator    != "placeholder"  (e.g. "pbr_hero" | "ai_upscale")
 *   source_file  resolves on disk under the modern asset root
 *   width,height match the on-disk PNG header
 *
 * This module is the CI-runnable distinction. It reads the manifest,
 * classifies each material slot into one of:
 *
 *   NOT_PROBED         — gate never ran yet
 *   NO_MANIFEST        — manifest missing / unreadable
 *   SYNTHETIC_PLACEHOLDER — manifest valid, all declared slots use
 *                          generator == "placeholder" (the CI default)
 *   PARTIAL            — at least one slot is REAL, at least one is
 *                          PLACEHOLDER / MISSING / UNKNOWN
 *   FINISHED_REAL      — every required material slot is REAL with
 *                          generator != "placeholder" and source_file
 *                          resolving on disk
 *
 * Companion to the existing real-asset SKIP-only gate
 * (test_dm1_v22_real_asset_material_gate_pc34). The sibling SKIP gate
 * only runs when an operator has dropped the full hero manifest on
 * disk; this module runs in CI by default and exercises the synthetic
 * baselines that match the actual current runtime state.
 *
 * Source-lock:
 *   - ReDMCSB DUNVIEW.C:6697-6816 (DM1 viewport composition order)
 *   - ReDMCSB DUNGEON.C:2238-2246 (square-type decode feeding
 *     m11_v22_shape_for_cell())
 *   - include/dm1_v2_asset_pipeline_pc34.h (modern asset manifest
 *     path resolution)
 *   - include/m11_v22_inplace_draw_pc34.h (cell -> variant -> asset_id)
 *   - sibling dm2_v2_hud_widget_assets.h (placeholder-vs-real pattern)
 *
 * Honest boundary: this gate reports the manifest state. It does NOT
 * claim any finished PBR art has been reviewed or shipped. The
 * FINISHED_REAL state is reachable only when an operator has dropped a
 * non-placeholder manifest with source_file paths that resolve on disk;
 * until then the gate stays in SYNTHETIC_PLACEHOLDER, which matches the
 * honest current default.
 */

#ifndef FIRESTAFF_DM1_V22_FINISHED_ART_MATERIAL_GATE_PC34_H
#define FIRESTAFF_DM1_V22_FINISHED_ART_MATERIAL_GATE_PC34_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Material slot enum ─────────────────────────────────────────
 *
 * Required DM1 V2.2 in-place material bitmaps that the gate tracks.
 * The ordering is the stable slot table used by get_slot_info() —
 * ordinals double as table indices. Inserting/removing slots must
 * update DM1_V22_FAMG_MATERIAL_COUNT and k_slot_table in the .c
 * file.
 *
 * Selection rationale (mirrors dm1_v22_inplace_render_probe):
 *   - WALL_D3_CARVED  : D1 center cell of the canonical synthetic
 *                       3x3 layout used by the in-place render probe
 *   - FLOOR_PLAIN     : D1 mid cell
 *   - FLOOR_PIT       : D1 right cell
 *   - CREATURE_DEMON  : the "any creature" fallback in
 *                       m11_v22_inplace_draw_pc34.c
 *   - CHAMPION_WARRIOR: the first champion portrait slot
 *   - DOOR_FRONT      : the door shape used by DUNGEON.C:2238-2246
 *
 * All slots map 1:1 to a fixture in tests/test_dm1_v22_real_asset_
 * material_gate_pc34.c so the synthetic-fallback tests and the real-
 * asset tests cover the same surface area. */
typedef enum {
    DM1_V22_FAMG_WALL_D3_CARVED  = 0,  /* material: walls */
    DM1_V22_FAMG_FLOOR_PLAIN     = 1,  /* material: floors */
    DM1_V22_FAMG_FLOOR_PIT       = 2,  /* material: floor pits */
    DM1_V22_FAMG_CREATURE_DEMON  = 3,  /* material: creatures */
    DM1_V22_FAMG_CHAMPION_WARRIOR= 4,  /* material: champion portraits */
    DM1_V22_FAMG_DOOR_FRONT      = 5,  /* material: doors */
    DM1_V22_FAMG_MATERIAL_COUNT  = 6
} DM1_V22_FamgSlot;

/* ── Per-slot classification ──────────────────────────────────── */
typedef enum {
    DM1_V22_FAMG_CLASS_UNKNOWN     = 0, /* no manifest probed yet */
    DM1_V22_FAMG_CLASS_MISSING     = 1, /* manifest valid but slot absent */
    DM1_V22_FAMG_CLASS_PLACEHOLDER = 2, /* slot present, generator ==
                                          "placeholder" (procedural) */
    DM1_V22_FAMG_CLASS_PARTIAL     = 3, /* real asset metadata but some
                                          required fields missing */
    DM1_V22_FAMG_CLASS_REAL        = 4  /* real PBR/PNG asset, all
                                          required fields present, source
                                          file resolves on disk */
} DM1_V22_FamgClass;

/* ── Overall manifest gate ───────────────────────────────────────
 *
 * State machine (driven by classify_slot() over all required slots):
 *
 *   NOT_PROBED            -> the gate has never been evaluated
 *   NO_MANIFEST           -> manifest missing / unreadable
 *   SYNTHETIC_PLACEHOLDER -> every declared slot is PLACEHOLDER
 *                            (matches current runtime default)
 *   PARTIAL               -> at least one REAL, at least one non-REAL
 *   FINISHED_REAL         -> every required slot is REAL
 *
 * FINISHED_REAL is reachable only when an operator has dropped the
 * full hero manifest under ~/.firestaff/assets/dm1/modern/ with
 * source_file paths that resolve on disk. Until that happens the
 * gate stays in SYNTHETIC_PLACEHOLDER, which is the honest current
 * default and the value CI should expect. */
typedef enum {
    DM1_V22_FAMG_GATE_NOT_PROBED            = 0,
    DM1_V22_FAMG_GATE_NO_MANIFEST           = 1,
    DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER = 2,
    DM1_V22_FAMG_GATE_PARTIAL               = 3,
    DM1_V22_FAMG_GATE_FINISHED_REAL         = 4
} DM1_V22_FamgGate;

/* ── Slot record (read-only view) ────────────────────────────────
 *
 * Strings are stored inline. The struct lifetime is the caller's; do
 * not retain pointers across other gate() invocations because the
 * stored manifest is module-level state. */
typedef struct {
    DM1_V22_FamgSlot slot;
    char            id[64];            /* manifest id (stable) */
    char            category[32];      /* "wall_shapes" / "creature_shapes" /
                                         "champion_portraits" / "door_shapes" */
    char            generator[32];     /* "placeholder" / "pbr_hero" / "" */
    char            source_file[256];  /* manifest source_file or "" */
    char            resolved_path[1024];/* full path or "" */
    int             width;
    int             height;
    int             file_exists;       /* 1 if resolved_path exists */
    DM1_V22_FamgClass classification;
} DM1_V22_FamgSlotInfo;

/* ── Public API ─────────────────────────────────────────────────── */

/* Set the manifest root directory. Mirrors m11_v22_set_manifest_path:
 *   dataDir is the DM1 game data dir (e.g. ~/.firestaff/data/dm1);
 *   the manifest is read from ~/.firestaff/assets/dm1/modern/ by
 *   walking up two parents and appending assets/dm1/modern/modern_
 *   asset_manifest.json.
 *
 * Passing NULL or "" clears the stored manifest path. */
void dm1_v22_famg_set_manifest_path(const char* dataDir);

/* Returns the resolved manifest path (read-only). May be empty if the
 * path has not been set. */
const char* dm1_v22_famg_get_manifest_path(void);

/* Validate the manifest at the given path (or the stored path if NULL).
 *
 * Returns:
 *   -1 — manifest missing, unreadable, or fundamentally invalid JSON
 *    0 — manifest readable but no slot entries recognised
 *    1 — manifest valid, every declared slot's required fields are
 *        populated (id + generator + source_file + width + height)
 */
int dm1_v22_famg_validate_manifest(const char* manifest_path);

/* Classify the named slot using the stored manifest. Returns the
 * classification. If the manifest is missing or the slot is not
 * present, returns DM1_V22_FAMG_CLASS_MISSING. */
DM1_V22_FamgClass dm1_v22_famg_classify_slot(DM1_V22_FamgSlot slot);

/* Fill out the slot info record for the given slot from the stored
 * manifest. Returns 1 on success, 0 if the slot is not present in the
 * manifest or the manifest is invalid. */
int dm1_v22_famg_get_slot_info(DM1_V22_FamgSlot slot,
                                DM1_V22_FamgSlotInfo* out);

/* Compute the overall manifest gate. Reads the stored manifest and
 * classifies every required slot. */
DM1_V22_FamgGate dm1_v22_famg_gate(void);

/* Returns the human-readable name of a slot. Stable; useful for
 * diagnostics, manifest IDs, and probe output. */
const char* dm1_v22_famg_slot_name(DM1_V22_FamgSlot slot);

/* Returns the category name used in modern_asset_manifest.json for the
 * given slot (e.g. "wall_shapes", "creature_shapes"). */
const char* dm1_v22_famg_slot_category(DM1_V22_FamgSlot slot);

/* Returns the manifest id used in modern_asset_manifest.json for the
 * given slot (e.g. "wall_d3_carved_hero_01", "creature_demon_hero_01").
 * These ids match the expected hero_01 ids in the existing
 * test_dm1_v22_real_asset_material_gate_pc34.c fixture so the two
 * gates cover the same manifest surface. */
const char* dm1_v22_famg_slot_manifest_id(DM1_V22_FamgSlot slot);

/* Returns the human-readable name of a classification. */
const char* dm1_v22_famg_class_name(DM1_V22_FamgClass cls);

/* Returns the human-readable name of a gate state. */
const char* dm1_v22_famg_gate_name(DM1_V22_FamgGate gate);

/* Counts slots classified as REAL across the stored manifest. Returns
 * the count and (optionally) the total declared slot count via out_total.
 * If the manifest is missing or invalid, returns 0 and sets *out_total
 * to 0. */
int dm1_v22_famg_real_count(int* out_total);

/* Installed flag mirror — for M12_AssetStatus and Phase 7 verification
 * suite integration. dm1_v22_famg_set_installed() is the last value seen
 * from dm1_v22_famg_gate(); the helper computes "PARTIAL or
 * FINISHED_REAL" -> 1, everything else -> 0. */
void dm1_v22_famg_set_installed(int installed);
int  dm1_v22_famg_get_installed(void);

/* Convenience: should the runtime use placeholder rendering for the
 * requested slot? Returns 1 if the slot classification is
 * PLACEHOLDER, MISSING, UNKNOWN, or PARTIAL, 0 only if the slot is
 * fully REAL. Mirrors the existing dm2_v2_hud_widget_assets
 * uses_placeholder() contract. */
int dm1_v22_famg_uses_placeholder(DM1_V22_FamgSlot slot);

/* Returns 1 if the gate is FINISHED_REAL — i.e. every required slot
 * is REAL with a non-placeholder generator and a source_file path that
 * resolves on disk. This is the "real reviewed finished-art pack"
 * state the gap-list row asks the gate to distinguish. */
int dm1_v22_famg_is_finished_real(void);

/* Returns 1 if the gate is in SYNTHETIC_PLACEHOLDER or PARTIAL —
 * i.e. at least one slot is declared but not fully REAL. This is the
 * "placeholder/synthetic art" state. */
int dm1_v22_famg_is_synthetic_or_partial(void);

/* Source evidence citation for source-lock tests. */
const char* dm1_v22_famg_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V22_FINISHED_ART_MATERIAL_GATE_PC34_H */
