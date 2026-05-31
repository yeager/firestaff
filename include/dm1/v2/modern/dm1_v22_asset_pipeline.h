/*
 * dm1_v22_asset_pipeline.h — DM1 V2.2 Modern Asset Pipeline
 *
 * Generated/modern art path for Dungeon Master I — V2.2 presentation.
 *
 * V2.2 ("Modern Graphics") replaces V1 indexed surfaces and V2.1 EPX
 * upscales with hand-crafted/generated RGBA textures at 1920×1080.
 * Modern assets ship as a drop-in optional asset pack:
 *   ~/.firestaff/assets/dm1/modern/
 *     modern_asset_manifest.json
 *     wall_shapes/
 *     floor_shapes/
 *     creature_shapes/
 *     object_shapes/
 *     ui_chrome/
 *     outdoor/
 *
 * Pipeline contract (V2.2 only — behind DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION):
 *   Modern RGBA texture (PNG/TGA, 1920×1080)
 *     → direct present surface blit
 *     → no EPX, no palette expansion, no indexed conversion
 *
 * Fallback chain:
 *   MODERN (V2.2) → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1)
 *
 * Source-lock anchors:
 *   ReDMCSB DUNGEON.C:2238-2246   — square type decode for shape dispatch
 *   ReDMCSB DEFS.H:922-941         — M034_SQUARE_TYPE enumeration
 *   ReDMCSB DUNVIEW.C:6697-6816    — wall/floor draw composition order
 *   ReDMCSB PANEL.C:418-428        — G0304_i_DungeonViewPaletteIndex (6 levels)
 *   ReDMCSB DRAWVIEW.C:1-200       — creature sprite framing (G0011_i_CreaturePosture)
 *   ReDMCSB PROJEXPL.C:43-165      — projectile/explosion routing
 *   Firestaff dm1_v22_shapes.c     — shape type → modern renderer bridge
 *
 * Phase: V2.2 modern assets are defined here; actual texture loading
 * and GPU rendering is handled by the asset pipeline agent.
 */

#ifndef FIRESTAFF_DM1_V22_ASSET_PIPELINE_H
#define FIRESTAFF_DM1_V22_ASSET_PIPELINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════════
 * Asset Provenance
 *
 * Every asset surface in the V2.2 pipeline carries a provenance tag
 * that records which generation path produced it. This is used for
 * diagnostics, visual verification, and the fallback chain logic.
 * ══════════════════════════════════════════════════════════════════════ */

/* Asset provenance — which generation path produced this asset.
 * Ordered from oldest (ORIGINAL) to newest (MODERN). */
typedef enum {
    DM1_V22_PROVENANCE_UNKNOWN = 0,   /* Uninitialised / not yet resolved */
    DM1_V22_PROVENANCE_ORIGINAL,       /* V1: raw GRAPHICS.DAT indexed, 320×200 */
    DM1_V22_PROVENANCE_FILTERED,       /* V2.0: V1 + scanline/palette filter */
    DM1_V22_PROVENANCE_UPSCALED,       /* V2.1: EPX 2x from V1 indexed */
    DM1_V22_PROVENANCE_MODERN,         /* V2.2: hand-crafted RGBA, 1920×1080 */
} DM1_V22_AssetProvenance;

/* Human-readable name for a provenance value. */
const char* dm1_v22_provenance_name(DM1_V22_AssetProvenance p);

/* ══════════════════════════════════════════════════════════════════════
 * Fallback Chain
 *
 * Describes the ordered list of provenance levels to try when loading
 * an asset. The chain is consulted when a higher-quality asset is
 * missing or invalid.
 * ══════════════════════════════════════════════════════════════════════ */

/* Maximum number of fallback levels in a chain. */
#define DM1_V22_FALLBACK_MAX 4

typedef struct {
    /* Ordered fallback levels, index 0 = preferred / highest quality.
     * Unused slots are set to DM1_V22_PROVENANCE_UNKNOWN. */
    DM1_V22_AssetProvenance levels[DM1_V22_FALLBACK_MAX];
    int count;  /* Number of valid levels, 1..DM1_V22_FALLBACK_MAX */
} DM1_V22_FallbackChain;

/* Predefined fallback chains for each presentation mode.
 * MODERN:  modern → upscaled → filtered → original
 * UPSCALED: upscaled → filtered → original
 * FILTERED: filtered → original
 * ORIGINAL: original only */
const DM1_V22_FallbackChain* dm1_v22_fallback_for_mode(DM1_V22_AssetProvenance mode);

/* Get the next fallback level after the given provenance.
 * Returns DM1_V22_PROVENANCE_UNKNOWN if already at the bottom of the chain. */
DM1_V22_AssetProvenance dm1_v22_fallback_next(DM1_V22_AssetProvenance current);

/* ══════════════════════════════════════════════════════════════════════
 * Asset Descriptor
 *
 * The canonical descriptor for a single modern-asset surface.
 * Carries provenance, filesystem path, dimensions, and validation state.
 * ══════════════════════════════════════════════════════════════════════ */

/* Asset format for modern textures. */
typedef enum {
    DM1_V22_FORMAT_UNKNOWN = 0,
    DM1_V22_FORMAT_INDEXED,   /* V1: 4-bit indexed, 320×200 */
    DM1_V22_FORMAT_PALETTED,   /* V2.0: 8-bit paletted, 320×200 */
    DM1_V22_FORMAT_RGBA,       /* V2.1: 32-bit RGBA, EPX ×2 → 640×400 */
    DM1_V22_FORMAT_PNG,        /* V2.2: RGBA PNG, 1920×1080 */
    DM1_V22_FORMAT_TGA,        /* V2.2: RGBA TGA, 1920×1080 */
} DM1_V22_AssetFormat;

/* Asset category — mirrors DM1_V2_SurfaceCategory but scoped to V2.2. */
typedef enum {
    DM1_V22_CATEGORY_UNKNOWN = 0,
    DM1_V22_CATEGORY_WALL,
    DM1_V22_CATEGORY_FLOOR,
    DM1_V22_CATEGORY_CEILING,
    DM1_V22_CATEGORY_DOOR,
    DM1_V22_CATEGORY_CREATURE,
    DM1_V22_CATEGORY_OBJECT,
    DM1_V22_CATEGORY_PROJECTILE,
    DM1_V22_CATEGORY_EXPLOSION,
    DM1_V22_CATEGORY_FLUXCAGE,
    DM1_V22_CATEGORY_FONT,
    DM1_V22_CATEGORY_UI_CHROME,
    DM1_V22_CATEGORY_PANEL,
    DM1_V22_CATEGORY_TITLE,
    DM1_V22_CATEGORY_ENTRANCE,
    DM1_V22_CATEGORY_COUNT
} DM1_V22_AssetCategory;

/* Maximum path length for an asset file on disk. */
#define DM1_V22_ASSET_PATH_MAX 512

/* Missing-asset placeholder dimensions.
 * The renderer uses a 16×16 magenta checkerboard when the entire
 * fallback chain is exhausted and no asset can be found. */
#define DM1_V22_MISSING_W 16
#define DM1_V22_MISSING_H 16

typedef struct {
    /* Provenance of this specific asset file. */
    DM1_V22_AssetProvenance provenance;

    /* Category and sub-type identifiers. */
    DM1_V22_AssetCategory  category;
    const char*            asset_id;       /* e.g. "wall_d3_left_01" — owned by manifest */
    const char*            source_anchor;  /* e.g. "DUNVIEW.C:4547" */

    /* Filesystem path (absolute or relative to modern asset root). */
    char                   file_path[DM1_V22_ASSET_PATH_MAX];

    /* Pixel dimensions. V1/V2.0/V2.1 assets are always 320×200.
     * V2.2 modern assets are 1920×1080. */
    int                    width;
    int                    height;

    /* Asset format. */
    DM1_V22_AssetFormat    format;

    /* Loaded pixel data. NULL until dm1_v22_asset_load() is called.
     * For RGBA formats: uint32_t pixels (ARGB or RGBA byte order).
     * For indexed formats: uint8_t indexed pixels. */
    void*                  pixels;
    size_t                 pixels_size;  /* bytes */

    /* Validation state. */
    int                    is_valid;      /* 0=not loaded/invalid, 1=loaded+checked */
    int                    load_attempted; /* 1=load was called */
} DM1_V22_AssetDescriptor;

/* ══════════════════════════════════════════════════════════════════════
 * Modern Asset Pack Discovery
 * ══════════════════════════════════════════════════════════════════════ */

/* Set the modern asset manifest path. Called by M12_AssetStatus_Scan()
 * after resolving the Firestaff data directory.
 * Path: <dataDir>/../assets/dm1/modern/modern_asset_manifest.json */
void dm1_v22_set_manifest_path(const char* manifest_path);

/* Get the currently configured manifest path. Returns "" if not set. */
const char* dm1_v22_get_manifest_path(void);

/* Load the modern asset manifest from disk.
 * Parses JSON, populates the internal asset registry.
 * Returns: -1 on error (missing/invalid JSON), 0 if not found,
 *          1 if fully loaded.
 * After calling this, dm1_v22_get_asset() can locate assets by ID. */
int dm1_v22_load_manifest(const char* manifest_path);

/* Validate the modern asset manifest JSON.
 * Checks: required top-level fields present, required categories have
 * at least one entry, first entry of each category has id, source_file,
 * width, height, format fields.
 * Returns: -1 on error, 0 if partial, 1 if complete. */
int dm1_v22_validate_manifest(const char* manifest_path);

/* Check if the modern asset pack is installed (manifest found and valid).
 * Returns 1 if available, 0 otherwise. */
int dm1_v22_modern_assets_available(void);

/* ── V2.2 Installed State ──────────────────────────────────────────── */

/* Set the V2.2 modern asset pack installed flag.
 * Called by the launcher after detecting whether the pack is present. */
void dm1_v22_set_installed(int installed);

/* Get the current V2.2 modern asset pack installed flag. */
int dm1_v22_get_installed(void);

/* ══════════════════════════════════════════════════════════════════════
 * Asset Loading and Fallback
 * ══════════════════════════════════════════════════════════════════════ */

/* Attempt to load an asset using the full fallback chain.
 *
 * Given a desired provenance (e.g. DM1_V22_PROVENANCE_MODERN), this
 * function walks the fallback chain in order, trying to open each
 * file variant until one succeeds.
 *
 * out_descriptor: filled with the loaded asset's metadata and pixel data.
 *                 The descriptor's provenance field reflects which level
 *                 was actually loaded (may be lower than requested).
 *
 * Returns: 1 if any asset was loaded, 0 if none could be loaded.
 * On failure, *out_descriptor has is_valid=0 and provenance=UNKNOWN.
 *
 * The loaded pixels are dynamically allocated; call
 * dm1_v22_asset_free() to release them. */
int dm1_v22_asset_load(const char* category, const char* asset_id,
                       DM1_V22_AssetProvenance desired_provenance,
                       DM1_V22_AssetDescriptor* out_descriptor);

/* Free pixel data in a descriptor. Safe to call on a zero-initialised
 * descriptor or one with NULL pixels. Resets is_valid=0. */
void dm1_v22_asset_free(DM1_V22_AssetDescriptor* desc);

/* Validate a loaded asset descriptor.
 * Checks: dimensions non-zero, format consistent with dimensions,
 * pixel buffer non-NULL when is_valid=1, size matches w×h×bpp.
 * Returns: 1 if valid, 0 if any check fails.
 * Logs specific failure reason on validation error. */
int dm1_v22_asset_validate(const DM1_V22_AssetDescriptor* desc);

/* Returns a static missing-asset descriptor (16×16 magenta checkerboard).
 * out_w/out_h are always set to 16. Never returns NULL.
 * This is used by the renderer when the full fallback chain is exhausted
 * and no asset can be found. */
const DM1_V22_AssetDescriptor* dm1_v22_get_missing_descriptor(int* out_w, int* out_h);

/* ══════════════════════════════════════════════════════════════════════
 * Best-Available Source Selection
 *
 * Given a desired provenance and current asset state, returns the best
 * available provenance level. Applies the fallback chain automatically.
 * ══════════════════════════════════════════════════════════════════════ */

/* Get the best available provenance for a category/asset.
 * desired: the preferred provenance (e.g. DM1_V22_PROVENANCE_MODERN).
 * If the modern asset is not installed, walks down to UPSCALED, FILTERED,
 * or ORIGINAL as available.
 * Returns: the best available provenance, never UNKNOWN (ORIGINAL is
 * always available as the final fallback). */
DM1_V22_AssetProvenance dm1_v22_best_available_provenance(
    const char* category, const char* asset_id,
    DM1_V22_AssetProvenance desired);

/* Human-readable name for an asset category enum value. */
const char* dm1_v22_category_name(DM1_V22_AssetCategory cat);

/* ══════════════════════════════════════════════════════════════════════
 * Source Evidence
 * ══════════════════════════════════════════════════════════════════════ */

const char* dm1_v22_asset_pipeline_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V22_ASSET_PIPELINE_H */
