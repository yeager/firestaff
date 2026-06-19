/*
 * dm2_v22_modern_assets_pc34.h — DM2 V2.2 Modern Graphics Asset Pipeline
 *
 * DM2 V2.2 "Modern Graphics" mode renders DM2's Quest at 1920x1080
 * using generated/modern 3D-rendered 2D art as a drop-in replacement for
 * the V1 indexed surfaces (320x200 DM2 PC base + outdoor T600 from HuC6260/HuC6270 VDC).
 *
 * Parallel to dm1_v22_modern_assets_pc34.h;
 * same shape source enum semantics but DM2-specific paths:
 *   ~/.firestaff/assets/dm2/modern/
 *
 * Source-lock anchors (DM2):
 * - SKULL.ASM T400/T520/T600: DM2 dungeon viewport + champion panel + outdoor viewport
 * - Saturn VDP1 + VDP2: Saturn VDP1/VDP2 graphics (256x224 NTSC, 16-color palette)
 * - ReDMCSB COMMAND.C:108-113/254-291 (parity contract): command dispatch
 *
 * Asset dir: ~/.firestaff/assets/dm2/modern/
 * Manifest:  modern_asset_manifest.json (format_version 1)
 * Categories: wall_shapes, floor_shapes, creature_shapes, ui_chrome,
 *             champion_portraits, door_shapes (optional)
 *
 * Required categories for a complete install:
 *   wall_shapes, floor_shapes, creature_shapes
 * Optional categories (graceful degradation if missing):
 *   ui_chrome, champion_portraits, door_shapes
 */

#ifndef FIRESTAFF_DM2_V22_MODERN_ASSETS_PC34_H
#define FIRESTAFF_DM2_V22_MODERN_ASSETS_PC34_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Shape source enum (DM2 V2.2):
 *   V1 → V2.0 (filtered) → V2.1 (upscaled) → V2.2 (modern)
 * Reuses the DM1_V22_ShapeSource enum for cross-game compatibility;
 * the same fallback chain applies per game. */
typedef enum {
    DM2_V22_SHAPE_SOURCE_V1_ORIGINAL = 0,
    DM2_V22_SHAPE_SOURCE_V2_FILTERED = 1,
    DM2_V22_SHAPE_SOURCE_V2_UPSCALED = 2,
    DM2_V22_SHAPE_SOURCE_V2_MODERN   = 3,
    DM2_V22_SHAPE_SOURCE_COUNT
} DM2_V22_ShapeSource;

/* ── Public API ──────────────────────────────────────────────────── */

/* dm2_v22_set_manifest_path — set path to the modern asset manifest.
 * dataDir is the DM2 game data directory (e.g. ~/.firestaff/data/dm2).
 * Walks up two levels to ~/.firestaff then appends assets/dm2/modern/. */
void dm2_v22_set_manifest_path(const char* dataDir);

/* dm2_v22_validate_manifest — validates the JSON manifest.
 * Returns: 1=complete, 0=partial, -1=invalid/missing */
int dm2_v22_validate_manifest(const char* manifest_path);

/* dm2_v22_modern_assets_available — 1 if pack installed with critical
 * categories (wall_shapes, floor_shapes, creature_shapes), 0 otherwise. */
int dm2_v22_modern_assets_available(void);

/* Installed flag — set by M12_AssetStatus_Scan at startup. */
void dm2_v22_set_installed(int installed);
int  dm2_v22_get_installed(void);

/* EPX cache warm flag (for V2.1 fallback). */
void dm2_v22_set_epx_cache_warm(int warm);
int  dm2_v22_get_epx_cache_warm(void);

/* dm2_v22_best_available_shape_source — returns the best shape source
 * for the requested presentation mode index (0=V1, 1=V2.0, 2=V2.1, 3=V2.2)
 * and the current asset state.
 * Fallback chain: MODERN → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1). */
DM2_V22_ShapeSource dm2_v22_best_available_shape_source(int presentation_mode_index);

/* dm2_v22_get_missing_placeholder — 16x16 RGBA magenta checkerboard. */
const uint32_t* dm2_v22_get_missing_placeholder(int* out_w, int* out_h);

/* dm2_v22_get_shape_path — resolves (category, asset_id) from manifest
 * to a full filesystem path. Returns 1 on success, 0 if not found. */
int dm2_v22_get_shape_path(const char* category, const char* asset_id,
                            char* out_path, size_t out_path_size);

/* dm2_v22_shape_source_name — human-readable name for a shape source. */
const char* dm2_v22_shape_source_name(DM2_V22_ShapeSource src);

/* dm2_v22_source_evidence — citation string for source-lock tests. */
const char* dm2_v22_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V22_MODERN_ASSETS_PC34_H */
