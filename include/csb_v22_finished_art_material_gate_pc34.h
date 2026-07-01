/*
 * csb_v22_finished_art_material_gate_pc34.h
 *
 * CSB V2.2 finished-art / material screenshot pixel gate.
 *
 * The V2.2 modern-asset pack at ~/.firestaff/assets/csb/modern/ carries
 * per-shape material PNGs (wall, floor, creature, door, panel_lord_order,
 * champion_warrior_csb) plus CSB-only chaos_rune / dsa_scroll entries.
 * When the pack is shipped it is procedurally generated (placeholder
 * colors) - the honest baseline for this pass. When a reviewer has
 * signed off on real finished art, the manifest entries for those slots
 * carry:
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
 * The CSB V2.2 viewport is the 9-square (3 depth x 3 lateral) layout
 * defined in CSBWin/Viewport.cpp:7290. The slots below mirror the asset
 * ids dispatched by src/csb/csb_v22_inplace_draw_pc34.c so this gate
 * tracks the same manifest surface as the in-place modern-art swap.
 *
 * Source-lock:
 *   - ReDMCSB DUNVIEW.C F0128 (CSB viewport routing)
 *   - ReDMCSB PANEL.C F0354    (CSB champion panel refresh)
 *   - ReDMCSB COMMAND.C:108-113/254-291 (command dispatch)
 *   - CSBWin/Viewport.cpp:7290 (9-square viewport layout)
 *   - CSBWin/Chaos.cpp:60-69   (DSA / chaos rune dispatch)
 *   - include/csb_v22_inplace_draw_pc34.h (cell -> variant -> asset_id)
 *   - include/csb_v22_modern_assets_pc34.h (manifest path resolution)
 *   - sibling dm1_v22 / dm2_v22 FAMG modules (placeholder-vs-real pattern)
 *
 * Honest boundary: this gate reports the manifest state. It does NOT
 * claim any finished PBR art has been reviewed or shipped. The
 * FINISHED_REAL state is reachable only when an operator has dropped a
 * non-placeholder manifest with source_file paths that resolve on disk;
 * until then the gate stays in SYNTHETIC_PLACEHOLDER, which matches the
 * honest current default.
 */

#ifndef FIRESTAFF_CSB_V22_FINISHED_ART_MATERIAL_GATE_PC34_H
#define FIRESTAFF_CSB_V22_FINISHED_ART_MATERIAL_GATE_PC34_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Material slot enum ─────────────────────────────────────────
 *
 * Required CSB V2.2 in-place material bitmaps that the gate tracks.
 * The ordering is the stable slot table used by get_slot_info() -
 * ordinals double as table indices. Inserting/removing slots must
 * update CSB_V22_FAMG_MATERIAL_COUNT and k_slot_table in the .c
 * file.
 *
 * Selection rationale (mirrors src/csb/csb_v22_inplace_draw_pc34.c
 * variant -> asset_id mapping plus the CSBWin/Chaos.cpp:60-69
 * CSB-only chaos magic dispatch):
 *   - WALL_DUNGEON        : D1 center cell of the canonical 9-square
 *                            synthetic layout used by the in-place
 *                            render probe
 *   - FLOOR_PLAIN         : D1 mid cell, PLAIN pattern
 *   - FLOOR_CRACKED       : D1 mid cell, CRACKED pattern
 *   - CREATURE_CHAOS_FIEND: the "any creature" fallback in
 *                            csb_v22_inplace_draw_pc34.c
 *   - PANEL_LORD_ORDER    : CSB-specific UI chrome (ReDMCSB PANEL.C
 *                            F0354 champion panel refresh)
 *   - CHAMPION_WARRIOR_CSB: CSB-specific champion portrait slot
 *   - DOOR_PRISON         : CSB-only entrance-sequence shape
 *                            (ReDMCSB ENTRANCE.C prison door)
 *   - CHAOS_RUNE          : CSB-only chaos magic rune (Chaos.cpp:60-69)
 *
 * All slots map 1:1 to a fixture in
 * tests/test_csb_v22_finished_art_material_gate_pc34.c so the
 * synthetic-fallback tests and the real-asset tests cover the same
 * surface area. */
typedef enum {
    CSB_V22_FAMG_WALL_DUNGEON       = 0,  /* wall_dungeon_01 */
    CSB_V22_FAMG_FLOOR_PLAIN        = 1,  /* floor_plain_01 */
    CSB_V22_FAMG_FLOOR_CRACKED      = 2,  /* floor_cracked_01 */
    CSB_V22_FAMG_CREATURE_CHAOS_FIEND = 3, /* creature_chaos_fiend_01 */
    CSB_V22_FAMG_PANEL_LORD_ORDER   = 4,  /* panel_lord_order_01 */
    CSB_V22_FAMG_CHAMPION_WARRIOR_CSB = 5,/* champion_warrior_csb_01 */
    CSB_V22_FAMG_DOOR_PRISON        = 6,  /* door_prison_01 (CSB-only) */
    CSB_V22_FAMG_CHAOS_RUNE         = 7,  /* chaos_rune_01 (CSB-only) */
    CSB_V22_FAMG_MATERIAL_COUNT     = 8
} CSB_V22_FamgSlot;

/* ── Per-slot classification ──────────────────────────────────── */
typedef enum {
    CSB_V22_FAMG_CLASS_UNKNOWN     = 0, /* no manifest probed yet */
    CSB_V22_FAMG_CLASS_MISSING     = 1, /* manifest valid but slot absent */
    CSB_V22_FAMG_CLASS_PLACEHOLDER = 2, /* slot present, generator ==
                                          "placeholder" (procedural) */
    CSB_V22_FAMG_CLASS_PARTIAL     = 3, /* real asset metadata but some
                                          required fields missing */
    CSB_V22_FAMG_CLASS_REAL        = 4  /* real PBR/PNG asset, all
                                          required fields present, source
                                          file resolves on disk */
} CSB_V22_FamgClass;

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
 * full hero manifest under ~/.firestaff/assets/csb/modern/ with
 * source_file paths that resolve on disk. Until that happens the
 * gate stays in SYNTHETIC_PLACEHOLDER, which is the honest current
 * default and the value CI should expect. */
typedef enum {
    CSB_V22_FAMG_GATE_NOT_PROBED            = 0,
    CSB_V22_FAMG_GATE_NO_MANIFEST           = 1,
    CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER = 2,
    CSB_V22_FAMG_GATE_PARTIAL               = 3,
    CSB_V22_FAMG_GATE_FINISHED_REAL         = 4
} CSB_V22_FamgGate;

/* ── Slot record (read-only view) ────────────────────────────────
 *
 * Strings are stored inline. The struct lifetime is the caller's; do
 * not retain pointers across other gate() invocations because the
 * stored manifest is module-level state. */
typedef struct {
    CSB_V22_FamgSlot slot;
    char            id[64];            /* manifest id (stable) */
    char            category[32];      /* "wall_shapes" / "creature_shapes" /
                                         "champion_portraits" / "door_shapes" /
                                         "ui_chrome" / "chaos_runes" */
    char            generator[32];     /* "placeholder" / "pbr_hero" / "" */
    char            source_file[256];  /* manifest source_file or "" */
    char            resolved_path[1024];/* full path or "" */
    int             width;
    int             height;
    int             file_exists;       /* 1 if resolved_path exists */
    CSB_V22_FamgClass classification;
} CSB_V22_FamgSlotInfo;

/* ── Public API ─────────────────────────────────────────────────── */

/* Set the manifest root directory. Mirrors csb_v22_set_manifest_path:
 *   dataDir is the CSB game data dir (e.g. ~/.firestaff/data/csb);
 *   the manifest is read from ~/.firestaff/assets/csb/modern/ by
 *   walking up two parents and appending assets/csb/modern/modern_
 *   asset_manifest.json.
 *
 * Passing NULL or "" clears the stored manifest path. */
void csb_v22_famg_set_manifest_path(const char* dataDir);

/* Returns the resolved manifest path (read-only). May be empty if the
 * path has not been set. */
const char* csb_v22_famg_get_manifest_path(void);

/* Validate the manifest at the given path (or the stored path if NULL).
 *
 * Returns:
 *   -1 — manifest missing, unreadable, or fundamentally invalid JSON
 *    0 — manifest readable but no slot entries recognised
 *    1 — manifest valid, every declared slot's required fields are
 *        populated (id + generator + source_file + width + height)
 */
int csb_v22_famg_validate_manifest(const char* manifest_path);

/* Classify the named slot using the stored manifest. Returns the
 * classification. If the manifest is missing or the slot is not
 * present, returns CSB_V22_FAMG_CLASS_MISSING. */
CSB_V22_FamgClass csb_v22_famg_classify_slot(CSB_V22_FamgSlot slot);

/* Fill out the slot info record for the given slot from the stored
 * manifest. Returns 1 on success, 0 if the slot is not present in the
 * manifest or the manifest is invalid. */
int csb_v22_famg_get_slot_info(CSB_V22_FamgSlot slot,
                                CSB_V22_FamgSlotInfo* out);

/* Compute the overall manifest gate. Reads the stored manifest and
 * classifies every required slot. */
CSB_V22_FamgGate csb_v22_famg_gate(void);

/* Returns the human-readable name of a slot. Stable; useful for
 * diagnostics, manifest IDs, and probe output. */
const char* csb_v22_famg_slot_name(CSB_V22_FamgSlot slot);

/* Returns the category name used in modern_asset_manifest.json for the
 * given slot (e.g. "wall_shapes", "creature_shapes"). */
const char* csb_v22_famg_slot_category(CSB_V22_FamgSlot slot);

/* Returns the manifest id used in modern_asset_manifest.json for the
 * given slot (e.g. "wall_dungeon_01", "creature_chaos_fiend_01").
 * These ids match the asset_id_for_shape() outputs in
 * src/csb/csb_v22_inplace_draw_pc34.c so the gate and the in-place
 * swap cover the same manifest surface. */
const char* csb_v22_famg_slot_manifest_id(CSB_V22_FamgSlot slot);

/* Returns the human-readable name of a classification. */
const char* csb_v22_famg_class_name(CSB_V22_FamgClass cls);

/* Returns the human-readable name of a gate state. */
const char* csb_v22_famg_gate_name(CSB_V22_FamgGate gate);

/* Counts slots classified as REAL across the stored manifest. Returns
 * the count and (optionally) the total declared slot count via out_total.
 * If the manifest is missing or invalid, returns 0 and sets *out_total
 * to 0. */
int csb_v22_famg_real_count(int* out_total);

/* Installed flag mirror — for M12_AssetStatus and Phase 7 verification
 * suite integration. csb_v22_famg_set_installed() is the last value seen
 * from csb_v22_famg_gate(); the helper computes "PARTIAL or
 * FINISHED_REAL" -> 1, everything else -> 0. */
void csb_v22_famg_set_installed(int installed);
int  csb_v22_famg_get_installed(void);

/* Convenience: should the runtime use placeholder rendering for the
 * requested slot? Returns 1 if the slot classification is
 * PLACEHOLDER, MISSING, UNKNOWN, or PARTIAL, 0 only if the slot is
 * fully REAL. Mirrors the existing dm1/dm2 FAMG modules'
 * uses_placeholder() contract. */
int csb_v22_famg_uses_placeholder(CSB_V22_FamgSlot slot);

/* Returns 1 if the gate is FINISHED_REAL — i.e. every required slot
 * is REAL with a non-placeholder generator and a source_file path that
 * resolves on disk. This is the "real reviewed finished-art pack"
 * state the gap-list row asks the gate to distinguish. */
int csb_v22_famg_is_finished_real(void);

/* Returns 1 if the gate is in SYNTHETIC_PLACEHOLDER or PARTIAL —
 * i.e. at least one slot is declared but not fully REAL. This is the
 * "placeholder/synthetic art" state. */
int csb_v22_famg_is_synthetic_or_partial(void);

/* ── Per-cell routing helpers ─────────────────────────────────────
 *
 * The CSB V2.2 viewport is a 3 depth x 3 lateral grid
 * (CSBWin/Viewport.cpp:7290). These helpers map a (depth, lateral)
 * cell coordinate to the FAMG slot the gate tracks for that cell.
 * The mapping mirrors csb_v22_inplace_get_cell_asset_id() in
 * src/csb/csb_v22_inplace_draw_pc34.c so the gate and the runtime
 * per-cell modern-art swap route to the same material slot:
 *
 *   depth 0 (D0, closest) lateral -1/0/+1 = WALL_DUNGEON
 *   depth 1 (D1, middle)  lateral -1/0/+1 = WALL_DUNGEON
 *   depth 2 (D2, back)    lateral -1/0/+1 = WALL_DUNGEON
 *
 * The 9-square viewport in this gate covers the wall/floor fallback
 * set. Creatures and panels route through the runtime swap module
 * (CREATURE_CHAOS_FIEND / PANEL_LORD_ORDER) and are looked up
 * directly via classify_slot() by callers that need them.
 *
 * Returns CSB_V22_FAMG_MATERIAL_COUNT when the cell coordinates are
 * out of range (depth not 0..2 or lateral not -1..+1). */
CSB_V22_FamgSlot csb_v22_famg_slot_for_cell(int depth, int lateral);

/* Convenience: classify the FAMG slot for a given CSB (depth, lateral)
 * viewport cell in one call. Returns CSB_V22_FAMG_CLASS_UNKNOWN for
 * out-of-range cells. */
CSB_V22_FamgClass csb_v22_famg_classify_cell(int depth, int lateral);

/* Source evidence citation for source-lock tests. */
const char* csb_v22_famg_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V22_FINISHED_ART_MATERIAL_GATE_PC34_H */
