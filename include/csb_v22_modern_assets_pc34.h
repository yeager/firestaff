/*
 * csb_v22_modern_assets_pc34.h — CSB V2.2 Modern Graphics Asset Pipeline
 *
 * CSB V2.2 "Modern Graphics" mode renders Chaos Strikes Back at 1920x1080
 * using generated/modern 3D-rendered 2D art as a drop-in replacement for
 * the V1 indexed surfaces.
 *
 * Parallel to dm1_v22_modern_assets_pc34.h; same shape source enum
 * semantics but CSB-specific paths:
 *   ~/.firestaff/assets/csb/modern/
 *
 * Source-lock anchors (CSB):
 * - ReDMCSB LIGHT.C F0212: CSB torchlight + lighting model
 * - ReDMCSB DUNVIEW.C F0128: CSB 9-square viewport (3x3 vs DM1's 4x3)
 * - ReDMCSB PANEL.C F0354: CSB champion panel refresh
 * - ReDMCSB COMMAND.C:108-113/254-291: command dispatch
 * - ReDMCSB CLIKMENU.C:142/180: input click routing
 * - CSBWin/Viewport.cpp:7290: 9-square grid mapping
 * - CSBWin/Chaos.cpp:60-69: DSA / chaos rune dispatch
 *
 * Asset dir: ~/.firestaff/assets/csb/modern/
 * Manifest:  modern_asset_manifest.json (format_version 1)
 * Categories: wall_shapes, floor_shapes, creature_shapes, ui_chrome,
 *             champion_portraits, door_shapes (optional),
 *             chaos_runes (CSB-only), dsa_scrolls (CSB-only)
 *
 * Required categories for a complete install:
 *   wall_shapes, floor_shapes, creature_shapes
 * Optional categories (graceful degradation if missing):
 *   ui_chrome, champion_portraits, door_shapes, chaos_runes, dsa_scrolls
 */

#ifndef FIRESTAFF_CSB_V22_MODERN_ASSETS_PC34_H
#define FIRESTAFF_CSB_V22_MODERN_ASSETS_PC34_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Shape source enum (CSB V2.2):
 *   V1 → V2.0 (filtered) → V2.1 (upscaled) → V2.2 (modern)
 * Reuses the DM1_V22_ShapeSource enum for cross-game compatibility;
 * the same fallback chain applies per game. */
typedef enum {
    CSB_V22_SHAPE_SOURCE_V1_ORIGINAL = 0,
    CSB_V22_SHAPE_SOURCE_V2_FILTERED = 1,
    CSB_V22_SHAPE_SOURCE_V2_UPSCALED = 2,
    CSB_V22_SHAPE_SOURCE_V2_MODERN   = 3,
    CSB_V22_SHAPE_SOURCE_COUNT
} CSB_V22_ShapeSource;

/* ── Public API ──────────────────────────────────────────────────── */

/* csb_v22_set_manifest_path — set path to the modern asset manifest.
 * dataDir is the CSB game data directory (e.g. ~/.firestaff/data/csb).
 * Walks up two levels to ~/.firestaff then appends assets/csb/modern/. */
void csb_v22_set_manifest_path(const char* dataDir);

/* csb_v22_validate_manifest — validates the JSON manifest.
 * Returns: 1=complete, 0=partial, -1=invalid/missing */
int csb_v22_validate_manifest(const char* manifest_path);

/* csb_v22_modern_assets_available — 1 if pack installed with critical
 * categories (wall_shapes, floor_shapes, creature_shapes), 0 otherwise. */
int csb_v22_modern_assets_available(void);

/* Installed flag — set by M12_AssetStatus_Scan at startup. */
void csb_v22_set_installed(int installed);
int  csb_v22_get_installed(void);

/* EPX cache warm flag (for V2.1 fallback). */
void csb_v22_set_epx_cache_warm(int warm);
int  csb_v22_get_epx_cache_warm(void);

/* csb_v22_best_available_shape_source — returns the best shape source
 * for the requested presentation mode index (0=V1, 1=V2.0, 2=V2.1, 3=V2.2)
 * and the current asset state.
 * Fallback chain: MODERN → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1). */
CSB_V22_ShapeSource csb_v22_best_available_shape_source(int presentation_mode_index);

/* csb_v22_get_missing_placeholder — 16x16 RGBA magenta checkerboard. */
const uint32_t* csb_v22_get_missing_placeholder(int* out_w, int* out_h);

/* csb_v22_get_shape_path — resolves (category, asset_id) from manifest
 * to a full filesystem path. Returns 1 on success, 0 if not found. */
int csb_v22_get_shape_path(const char* category, const char* asset_id,
                            char* out_path, size_t out_path_size);

/* csb_v22_shape_source_name — human-readable name for a shape source. */
const char* csb_v22_shape_source_name(CSB_V22_ShapeSource src);

/* csb_v22_source_evidence — citation string for source-lock tests. */
const char* csb_v22_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V22_MODERN_ASSETS_PC34_H */
