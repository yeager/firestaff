/*
 * dm2_v22_finished_art_material_gate_pc34.h
 *
 * DM2 V2.2 finished-art / material screenshot pixel gate.
 *
 * The V2.2 modern-asset pack at ~/.firestaff/assets/dm2/modern/ carries
 * per-shape material PNGs (indoor walls, outdoor walls, floor variants,
 * creatures, sky/ground bands, trees, and doors). When the
 * pack is shipped it is procedurally generated (placeholder colors) - the
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
 *   NOT_PROBED         - gate never ran yet
 *   NO_MANIFEST        - manifest missing / unreadable
 *   SYNTHETIC_PLACEHOLDER - manifest valid, all declared slots use
 *                          generator == "placeholder" (the CI default)
 *   PARTIAL            - at least one slot is REAL, at least one is
 *                          PLACEHOLDER / MISSING / UNKNOWN
 *   FINISHED_REAL      - every required material slot is REAL with
 *                          generator != "placeholder" and source_file
 *                          resolving on disk
 *
 * Source-lock:
 *   - SKULL.ASM T520/T560/T600 (DM2 viewport ticks)
 *   - ReDMCSB DUNVIEW.C:2962-3070 (DM2 outdoor sky/ground composition)
 *   - include/dm2_v22_modern_assets_pc34.h (modern asset manifest
 *     path resolution)
 *   - include/dm2_v22_inplace_draw_pc34.h (cell -> variant -> asset_id)
 *   - include/dm2_v22_viewport_swap_pc34.h (T560/T600 shape -> asset_id)
 *   - sibling dm2_v2_hud_widget_assets.h (placeholder-vs-real pattern)
 *
 * Honest boundary: this gate reports the manifest state. It does NOT
 * claim any finished PBR art has been reviewed or shipped. The
 * FINISHED_REAL state is reachable only when an operator has dropped a
 * non-placeholder manifest with source_file paths that resolve on disk;
 * until then the gate stays in SYNTHETIC_PLACEHOLDER, which matches the
 * honest current default.
 */

#ifndef FIRESTAFF_DM2_V22_FINISHED_ART_MATERIAL_GATE_PC34_H
#define FIRESTAFF_DM2_V22_FINISHED_ART_MATERIAL_GATE_PC34_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Material slot enum -----------------------------------------
 *
 * Required DM2 V2.2 in-place material bitmaps that the gate tracks.
 * The ordering is the stable slot table used by get_slot_info() -
 * ordinals double as table indices. Inserting/removing slots must
 * update DM2_V22_FAMG_MATERIAL_COUNT and k_slot_table in the .c
 * file.
 *
 * Selection rationale (mirrors dm2_v22_viewport_swap_pc34.c):
 *   - WALL_DUNGEON    : indoor T560 wall fallback
 *   - WALL_OUTDOOR    : outdoor T600 building/wall fallback
 *   - FLOOR_PLAIN     : plain/cracked/mossy indoor floor fallback
 *   - FLOOR_PIT       : pit floor fallback
 *   - FLOOR_STAIRS    : stairs up/down fallback
 *   - CREATURE_BRIGAND: any creature/projectile fallback
 *   - SKY/GROUND/HORIZON/TREE: outdoor T600 material bands
 *   - DOOR_WOOD       : DM2 door fallback
 *
 * Manifest IDs intentionally match dm2_v22_asset_id_for_shape() outputs
 * so this gate tracks the same asset surface as the runtime swap. */
typedef enum {
    DM2_V22_FAMG_WALL_DUNGEON     = 0,  /* wall_dm2_temple_01 */
    DM2_V22_FAMG_WALL_OUTDOOR     = 1,  /* wall_dm2_outdoor_01 */
    DM2_V22_FAMG_FLOOR_PLAIN      = 2,  /* floor_dm2_outdoor_01 */
    DM2_V22_FAMG_FLOOR_PIT        = 3,  /* floor_dm2_pit_01 */
    DM2_V22_FAMG_FLOOR_STAIRS     = 4,  /* floor_dm2_stairs_01 */
    DM2_V22_FAMG_CREATURE_BRIGAND = 5,  /* creature_dm2_brigand_01 */
    DM2_V22_FAMG_SKY              = 6,  /* sky_dm2_outdoor_01 */
    DM2_V22_FAMG_GROUND           = 7,  /* ground_dm2_outdoor_01 */
    DM2_V22_FAMG_GROUND_HORIZON   = 8,  /* ground_dm2_horizon_01 */
    DM2_V22_FAMG_TREE             = 9,  /* tree_dm2_outdoor_01 */
    DM2_V22_FAMG_DOOR_WOOD        = 10, /* door_dm2_wood_01 */
    DM2_V22_FAMG_MATERIAL_COUNT   = 11
} DM2_V22_FamgSlot;

/* -- Per-slot classification ------------------------------------ */
typedef enum {
    DM2_V22_FAMG_CLASS_UNKNOWN     = 0, /* no manifest probed yet */
    DM2_V22_FAMG_CLASS_MISSING     = 1, /* manifest valid but slot absent */
    DM2_V22_FAMG_CLASS_PLACEHOLDER = 2, /* slot present, generator ==
                                          "placeholder" (procedural) */
    DM2_V22_FAMG_CLASS_PARTIAL     = 3, /* real asset metadata but some
                                          required fields missing */
    DM2_V22_FAMG_CLASS_REAL        = 4  /* real PBR/PNG asset, all
                                          required fields present, source
                                          file resolves on disk */
} DM2_V22_FamgClass;

/* -- Overall manifest gate ---------------------------------------
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
 * full hero manifest under ~/.firestaff/assets/dm2/modern/ with
 * source_file paths that resolve on disk. Until that happens the
 * gate stays in SYNTHETIC_PLACEHOLDER, which is the honest current
 * default and the value CI should expect. */
typedef enum {
    DM2_V22_FAMG_GATE_NOT_PROBED            = 0,
    DM2_V22_FAMG_GATE_NO_MANIFEST           = 1,
    DM2_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER = 2,
    DM2_V22_FAMG_GATE_PARTIAL               = 3,
    DM2_V22_FAMG_GATE_FINISHED_REAL         = 4
} DM2_V22_FamgGate;

/* -- Slot record (read-only view) --------------------------------
 *
 * Strings are stored inline. The struct lifetime is the caller's; do
 * not retain pointers across other gate() invocations because the
 * stored manifest is module-level state. */
typedef struct {
    DM2_V22_FamgSlot slot;
    char            id[64];            /* manifest id (stable) */
    char            category[32];      /* "wall_shapes" / "floor_shapes" /
                                         "creature_shapes" / "door_shapes" */
    char            generator[32];     /* "placeholder" / "pbr_hero" / "" */
    char            source_file[256];  /* manifest source_file or "" */
    char            resolved_path[1024];/* full path or "" */
    int             width;
    int             height;
    int             file_exists;       /* 1 if resolved_path exists */
    DM2_V22_FamgClass classification;
} DM2_V22_FamgSlotInfo;

/* -- Public API --------------------------------------------------- */

/* Set the manifest root directory. Mirrors dm2_v22_set_manifest_path:
 *   dataDir is the DM2 game data dir (e.g. ~/.firestaff/data/dm2);
 *   the manifest is read from ~/.firestaff/assets/dm2/modern/ by
 *   walking up two parents and appending assets/dm2/modern/modern_
 *   asset_manifest.json.
 *
 * Passing NULL or "" clears the stored manifest path. */
void dm2_v22_famg_set_manifest_path(const char* dataDir);

/* Returns the resolved manifest path (read-only). May be empty if the
 * path has not been set. */
const char* dm2_v22_famg_get_manifest_path(void);

/* Validate the manifest at the given path (or the stored path if NULL).
 *
 * Returns:
 *   -1 - manifest missing, unreadable, or fundamentally invalid JSON
 *    0 - manifest readable but no slot entries recognised
 *    1 - manifest valid, every declared slot's required fields are
 *        populated (id + generator + source_file + width + height)
 */
int dm2_v22_famg_validate_manifest(const char* manifest_path);

/* Classify the named slot using the stored manifest. Returns the
 * classification. If the manifest is missing or the slot is not
 * present, returns DM2_V22_FAMG_CLASS_MISSING. */
DM2_V22_FamgClass dm2_v22_famg_classify_slot(DM2_V22_FamgSlot slot);

/* Fill out the slot info record for the given slot from the stored
 * manifest. Returns 1 on success, 0 if the slot is not present in the
 * manifest or the manifest is invalid. */
int dm2_v22_famg_get_slot_info(DM2_V22_FamgSlot slot,
                                DM2_V22_FamgSlotInfo* out);

/* Compute the overall manifest gate. Reads the stored manifest and
 * classifies every required slot. */
DM2_V22_FamgGate dm2_v22_famg_gate(void);

/* Returns the human-readable name of a slot. Stable; useful for
 * diagnostics, manifest IDs, and probe output. */
const char* dm2_v22_famg_slot_name(DM2_V22_FamgSlot slot);

/* Returns the category name used in modern_asset_manifest.json for the
 * given slot (e.g. "wall_shapes", "creature_shapes"). */
const char* dm2_v22_famg_slot_category(DM2_V22_FamgSlot slot);

/* Returns the manifest id used in modern_asset_manifest.json for the
 * given slot (e.g. "wall_dm2_temple_01", "creature_dm2_brigand_01").
 * These ids match dm2_v22_asset_id_for_shape() outputs so the gate
 * covers the same manifest surface as the T560/T600 runtime swap. */
const char* dm2_v22_famg_slot_manifest_id(DM2_V22_FamgSlot slot);

/* Returns the human-readable name of a classification. */
const char* dm2_v22_famg_class_name(DM2_V22_FamgClass cls);

/* Returns the human-readable name of a gate state. */
const char* dm2_v22_famg_gate_name(DM2_V22_FamgGate gate);

/* Counts slots classified as REAL across the stored manifest. Returns
 * the count and (optionally) the total declared slot count via out_total.
 * If the manifest is missing or invalid, returns 0 and sets *out_total
 * to 0. */
int dm2_v22_famg_real_count(int* out_total);

/* Installed flag mirror - for M12_AssetStatus and Phase 7 verification
 * suite integration. dm2_v22_famg_set_installed() is the last value seen
 * from dm2_v22_famg_gate(); the helper computes "PARTIAL or
 * FINISHED_REAL" -> 1, everything else -> 0. */
void dm2_v22_famg_set_installed(int installed);
int  dm2_v22_famg_get_installed(void);

/* Convenience: should the runtime use placeholder rendering for the
 * requested slot? Returns 1 if the slot classification is
 * PLACEHOLDER, MISSING, UNKNOWN, or PARTIAL, 0 only if the slot is
 * fully REAL. Mirrors the existing dm2_v2_hud_widget_assets
 * uses_placeholder() contract. */
int dm2_v22_famg_uses_placeholder(DM2_V22_FamgSlot slot);

/* Returns 1 if the gate is FINISHED_REAL - i.e. every required slot
 * is REAL with a non-placeholder generator and a source_file path that
 * resolves on disk. This is the "real reviewed finished-art pack"
 * state the gap-list row asks the gate to distinguish. */
int dm2_v22_famg_is_finished_real(void);

/* Returns 1 if the gate is in SYNTHETIC_PLACEHOLDER or PARTIAL -
 * i.e. at least one slot is declared but not fully REAL. This is the
 * "placeholder/synthetic art" state. */
int dm2_v22_famg_is_synthetic_or_partial(void);

/* Source evidence citation for source-lock tests. */
const char* dm2_v22_famg_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V22_FINISHED_ART_MATERIAL_GATE_PC34_H */
