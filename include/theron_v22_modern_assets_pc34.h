/*
 * theron_v22_modern_assets_pc34.h — Theron V2.2 Modern Graphics Asset Pipeline
 *
 * Theron V2.2 "Modern Graphics" mode renders Theron's Quest at 1920x1080
 * using generated/modern 3D-rendered 2D art as a drop-in replacement for
 * the V1 indexed surfaces (256x224 NTSC base from HuC6260/HuC6270 VDC).
 *
 * Parallel to dm1_v22_modern_assets_pc34.h;
 * same shape source enum semantics but Theron-specific paths:
 *   ~/.firestaff/assets/theron/modern/
 *
 * Source-lock anchors (Theron):
 * - THQUEST.ASM T400/T520/T600: Theron dungeon viewport + champion panel + outdoor viewport
 * - HuC6260/HuC6270 VDC + HuC6270 VCE: PCE graphics hardware (256x224 NTSC, 16-color palette)
 * - ReDMCSB COMMAND.C:108-113/254-291 (parity contract): command dispatch
 *
 * Asset dir: ~/.firestaff/assets/theron/modern/
 * Manifest:  modern_asset_manifest.json (format_version 1)
 * Categories: wall_shapes, floor_shapes, creature_shapes, ui_chrome,
 *             champion_portraits, door_shapes (optional)
 *
 * Required categories for a complete install:
 *   wall_shapes, floor_shapes, creature_shapes
 * Optional categories (graceful degradation if missing):
 *   ui_chrome, champion_portraits, door_shapes
 */

#ifndef FIRESTAFF_THERON_V22_MODERN_ASSETS_PC34_H
#define FIRESTAFF_THERON_V22_MODERN_ASSETS_PC34_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Shape source enum (Theron V2.2):
 *   V1 → V2.0 (filtered) → V2.1 (upscaled) → V2.2 (modern)
 * Reuses the DM1_V22_ShapeSource enum for cross-game compatibility;
 * the same fallback chain applies per game. */
typedef enum {
    THERON_V22_SHAPE_SOURCE_V1_ORIGINAL = 0,
    THERON_V22_SHAPE_SOURCE_V2_FILTERED = 1,
    THERON_V22_SHAPE_SOURCE_V2_UPSCALED = 2,
    THERON_V22_SHAPE_SOURCE_V2_MODERN   = 3,
    THERON_V22_SHAPE_SOURCE_COUNT
} THERON_V22_ShapeSource;

/* ── Public API ──────────────────────────────────────────────────── */

/* theron_v22_set_manifest_path — set path to the modern asset manifest.
 * dataDir is the Theron game data directory (e.g. ~/.firestaff/data/theron).
 * Walks up two levels to ~/.firestaff then appends assets/theron/modern/. */
void theron_v22_set_manifest_path(const char* dataDir);

/* theron_v22_validate_manifest — validates the JSON manifest.
 * Returns: 1=complete, 0=partial, -1=invalid/missing */
int theron_v22_validate_manifest(const char* manifest_path);

/* theron_v22_modern_assets_available — 1 if pack installed with critical
 * categories (wall_shapes, floor_shapes, creature_shapes), 0 otherwise. */
int theron_v22_modern_assets_available(void);

/* Installed flag — set by M12_AssetStatus_Scan at startup. */
void theron_v22_set_installed(int installed);
int  theron_v22_get_installed(void);

/* EPX cache warm flag (for V2.1 fallback). */
void theron_v22_set_epx_cache_warm(int warm);
int  theron_v22_get_epx_cache_warm(void);

/* theron_v22_best_available_shape_source — returns the best shape source
 * for the requested presentation mode index (0=V1, 1=V2.0, 2=V2.1, 3=V2.2)
 * and the current asset state.
 * Fallback chain: MODERN → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1). */
THERON_V22_ShapeSource theron_v22_best_available_shape_source(int presentation_mode_index);

/* theron_v22_get_missing_placeholder — 16x16 RGBA magenta checkerboard. */
const uint32_t* theron_v22_get_missing_placeholder(int* out_w, int* out_h);

/* theron_v22_get_shape_path — resolves (category, asset_id) from manifest
 * to a full filesystem path. Returns 1 on success, 0 if not found. */
int theron_v22_get_shape_path(const char* category, const char* asset_id,
                            char* out_path, size_t out_path_size);

/* theron_v22_shape_source_name — human-readable name for a shape source. */
const char* theron_v22_shape_source_name(THERON_V22_ShapeSource src);

/* theron_v22_source_evidence — citation string for source-lock tests. */
const char* theron_v22_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V22_MODERN_ASSETS_PC34_H */
