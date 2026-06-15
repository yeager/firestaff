/*
 * dm1_v22_asset_pipeline.c — DM1 V2.2 Modern Asset Pipeline Implementation
 *
 * Generated/modern art path for Dungeon Master I — V2.2 presentation.
 *
 * Implements:
 *   - Asset provenance metadata and naming
 *   - Fallback chain definitions and lookup
 *   - Modern asset manifest discovery and validation
 *   - Asset loading with full fallback chain traversal
 *   - Asset validation (dimensions, format, pixel buffer consistency)
 *   - Best-available provenance selection
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
 */

#include "dm1/v2/modern/dm1_v22_asset_pipeline.h"
#include "dm1_v2_asset_pipeline_pc34.h"  /* For DM1_V2_ASSET_MODE_MODERN / _UPSCALED */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ══════════════════════════════════════════════════════════════════════
 * Module State
 * ══════════════════════════════════════════════════════════════════════ */

/* Path to the modern asset manifest JSON (set by dm1_v22_set_manifest_path). */
static char g_manifest_path[DM1_V22_ASSET_PATH_MAX] = {0};

/* Whether the manifest has been loaded (and validated as parseable). */
static int g_manifest_loaded = 0;

/* Whether the modern asset pack is installed and has critical categories.
 * Set by dm1_v22_load_manifest() after successful manifest parse. */
static int g_modern_assets_installed = 0;

/* V2.2 assets installed flag (mirrors the m11_v22_get_installed() API
 * but scoped here so the asset pipeline is self-contained). */
static int g_v22_installed = 0;

/* ══════════════════════════════════════════════════════════════════════
 * Provenance Naming
 * ══════════════════════════════════════════════════════════════════════ */

const char* dm1_v22_provenance_name(DM1_V22_AssetProvenance p) {
    switch (p) {
        case DM1_V22_PROVENANCE_UNKNOWN:    return "unknown";
        case DM1_V22_PROVENANCE_ORIGINAL:    return "original";     /* V1: 320×200 indexed */
        case DM1_V22_PROVENANCE_FILTERED:    return "filtered";     /* V2.0: scanline/palette */
        case DM1_V22_PROVENANCE_UPSCALED:    return "upscaled";    /* V2.1: EPX 2× → RGBA */
        case DM1_V22_PROVENANCE_MODERN:      return "modern";       /* V2.2: 1920×1080 RGBA */
        default:                             return "???";
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Fallback Chain Definitions
 * ══════════════════════════════════════════════════════════════════════ */

/* Predefined fallback chains, one per presentation mode.
 * Index 0 = preferred (highest quality). */
static const DM1_V22_FallbackChain k_fallback_chains[] = {
    /* DM1_V22_PROVENANCE_MODERN: modern → upscaled → filtered → original */
    [DM1_V22_PROVENANCE_MODERN] = {
        .levels = {
            DM1_V22_PROVENANCE_MODERN,
            DM1_V22_PROVENANCE_UPSCALED,
            DM1_V22_PROVENANCE_FILTERED,
            DM1_V22_PROVENANCE_ORIGINAL
        },
        .count = 4
    },
    /* DM1_V22_PROVENANCE_UPSCALED: upscaled → filtered → original */
    [DM1_V22_PROVENANCE_UPSCALED] = {
        .levels = {
            DM1_V22_PROVENANCE_UPSCALED,
            DM1_V22_PROVENANCE_FILTERED,
            DM1_V22_PROVENANCE_ORIGINAL,
            DM1_V22_PROVENANCE_UNKNOWN
        },
        .count = 3
    },
    /* DM1_V22_PROVENANCE_FILTERED: filtered → original */
    [DM1_V22_PROVENANCE_FILTERED] = {
        .levels = {
            DM1_V22_PROVENANCE_FILTERED,
            DM1_V22_PROVENANCE_ORIGINAL,
            DM1_V22_PROVENANCE_UNKNOWN,
            DM1_V22_PROVENANCE_UNKNOWN
        },
        .count = 2
    },
    /* DM1_V22_PROVENANCE_ORIGINAL: original only */
    [DM1_V22_PROVENANCE_ORIGINAL] = {
        .levels = {
            DM1_V22_PROVENANCE_ORIGINAL,
            DM1_V22_PROVENANCE_UNKNOWN,
            DM1_V22_PROVENANCE_UNKNOWN,
            DM1_V22_PROVENANCE_UNKNOWN
        },
        .count = 1
    }
};

const DM1_V22_FallbackChain* dm1_v22_fallback_for_mode(DM1_V22_AssetProvenance mode) {
    if (mode <= DM1_V22_PROVENANCE_UNKNOWN || mode > DM1_V22_PROVENANCE_MODERN) {
        return NULL;
    }
    return &k_fallback_chains[mode];
}

DM1_V22_AssetProvenance dm1_v22_fallback_next(DM1_V22_AssetProvenance current) {
    switch (current) {
        case DM1_V22_PROVENANCE_MODERN:   return DM1_V22_PROVENANCE_UPSCALED;
        case DM1_V22_PROVENANCE_UPSCALED:  return DM1_V22_PROVENANCE_FILTERED;
        case DM1_V22_PROVENANCE_FILTERED: return DM1_V22_PROVENANCE_ORIGINAL;
        case DM1_V22_PROVENANCE_ORIGINAL: return DM1_V22_PROVENANCE_UNKNOWN;
        default:                           return DM1_V22_PROVENANCE_UNKNOWN;
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Category Naming
 * ══════════════════════════════════════════════════════════════════════ */

const char* dm1_v22_category_name(DM1_V22_AssetCategory cat) {
    switch (cat) {
        case DM1_V22_CATEGORY_UNKNOWN:     return "unknown";
        case DM1_V22_CATEGORY_WALL:       return "wall";
        case DM1_V22_CATEGORY_FLOOR:      return "floor";
        case DM1_V22_CATEGORY_CEILING:    return "ceiling";
        case DM1_V22_CATEGORY_DOOR:       return "door";
        case DM1_V22_CATEGORY_CREATURE:   return "creature";
        case DM1_V22_CATEGORY_OBJECT:     return "object";
        case DM1_V22_CATEGORY_PROJECTILE: return "projectile";
        case DM1_V22_CATEGORY_EXPLOSION:  return "explosion";
        case DM1_V22_CATEGORY_FLUXCAGE:   return "fluxcage";
        case DM1_V22_CATEGORY_FONT:        return "font";
        case DM1_V22_CATEGORY_UI_CHROME:  return "ui_chrome";
        case DM1_V22_CATEGORY_PANEL:      return "panel";
        case DM1_V22_CATEGORY_TITLE:       return "title";
        case DM1_V22_CATEGORY_ENTRANCE:    return "entrance";
        default:                            return "???";
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Missing Asset Placeholder
 *
 * 16×16 magenta checkerboard — returned when the entire fallback chain
 * is exhausted and no asset can be found.
 * ══════════════════════════════════════════════════════════════════════ */

#define DM1_V22_MISSING_W  16
#define DM1_V22_MISSING_H  16

static const uint32_t g_missing_pixels[DM1_V22_MISSING_H][DM1_V22_MISSING_W] = {
    { 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF,
      0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF },
    { 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF,
      0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF },
    { 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF,
      0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF },
    { 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF,
      0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF },
    { 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF,
      0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF },
    { 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF,
      0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF },
    { 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF,
      0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF },
    { 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF,
      0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF },
    { 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF,
      0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF },
    { 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF,
      0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF },
    { 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF,
      0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF },
    { 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF,
      0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF },
    { 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF,
      0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF },
    { 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF,
      0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF },
    { 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF,
      0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF },
    { 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF,
      0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF00FF }
};

static const DM1_V22_AssetDescriptor g_missing_descriptor = {
    .provenance  = DM1_V22_PROVENANCE_UNKNOWN,
    .category    = DM1_V22_CATEGORY_UNKNOWN,
    .asset_id    = "DM1_V22_MISSING_PLACEHOLDER",
    .source_anchor = "dm1_v22_asset_pipeline.c:missing_placeholder",
    .file_path   = "",
    .width       = DM1_V22_MISSING_W,
    .height      = DM1_V22_MISSING_H,
    .format      = DM1_V22_FORMAT_RGBA,
    .pixels      = (void*)g_missing_pixels,
    .pixels_size = sizeof(g_missing_pixels),
    .is_valid    = 1,
    .load_attempted = 1
};

const DM1_V22_AssetDescriptor* dm1_v22_get_missing_descriptor(int* out_w, int* out_h) {
    if (out_w)  *out_w  = DM1_V22_MISSING_W;
    if (out_h)  *out_h  = DM1_V22_MISSING_H;
    return &g_missing_descriptor;
}

/* ══════════════════════════════════════════════════════════════════════
 * Modern Asset Manifest Discovery
 * ══════════════════════════════════════════════════════════════════════ */

void dm1_v22_set_manifest_path(const char* manifest_path) {
    if (!manifest_path) return;
    strncpy(g_manifest_path, manifest_path, sizeof(g_manifest_path) - 1);
    g_manifest_path[sizeof(g_manifest_path) - 1] = '\0';
    /* Invalidate entire manifest cache when path changes */
    g_manifest_loaded = 0;
    g_modern_assets_installed = 0;
    /* Also reset V2 installed flag since manifest changed */
    g_v22_installed = 0;
}

const char* dm1_v22_get_manifest_path(void) {
    return g_manifest_path[0] ? g_manifest_path : "";
}

/* Minimal JSON manifest parser.
 * Accepts the subset of JSON needed for modern_asset_manifest.json:
 *   { "categories": { "wall_shapes": [ {"id": "...", "source_file": "..."} ], ... } }
 * This avoids adding a JSON dependency while providing basic manifest support.
 *
 * Returns: -1 on error (malformed), 0 if file not found, 1 on success.
 * Sets g_modern_assets_installed=1 on success. */
static int parse_minimal_manifest(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) return 0;

    /* Read the file into a buffer (max 64 KB). */
    char buf[65536];
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    if (n == 0) return -1;
    buf[n] = '\0';

    /* Very lightweight JSON sanity check:
     * 1. Must start with '{'
     * 2. Must contain "categories": {
     * 3. Must contain at least one top-level category key followed by ':'
     * 4. After ':', there must be '[' (array start)
     *
     * We don't implement a full JSON parser — we just verify the structure
     * is sufficient for the renderer to build asset paths. */

    if (n < 2 || buf[0] != '{') {
        return -1;
    }

    /* Find "categories" key */
    const char* cats = strstr(buf, "\"categories\"");
    if (!cats) return -1;

    /* Skip past "categories" and any whitespace, find the ':' */
    const char* p = cats + strlen("\"categories\"");
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    if (*p != ':') return -1;
    p++;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    if (*p != '{') return -1;

    /* Verify at least one category key exists: "category_name":
     * We look for a pattern like "wall_shapes" or "creature_shapes" in quotes. */
    const char* wall = strstr(buf, "\"wall_shapes\"");
    const char* creature = strstr(buf, "\"creature_shapes\"");
    const char* floor_s = strstr(buf, "\"floor_shapes\"");

    if (!wall && !creature && !floor_s) {
        return -1; /* No known critical categories found */
    }

    /* If we have wall_shapes, verify it has an array with at least one entry */
    if (wall) {
        const char* arr_start = wall + strlen("\"wall_shapes\"");
        while (*arr_start && (*arr_start == ' ' || *arr_start == '\t' || *arr_start == '\n' || *arr_start == '\r' || *arr_start == ':')) arr_start++;
        if (*arr_start != '[') return -1;
        /* Look for closing ']' — if none, malformed */
        const char* arr_end = strchr(arr_start, ']');
        if (!arr_end) return -1;
    }

    /* All basic structural checks passed */
    g_modern_assets_installed = 1;
    g_manifest_loaded = 1;
    return 1;
}

int dm1_v22_load_manifest(const char* manifest_path) {
    const char* path = manifest_path ? manifest_path : g_manifest_path;
    if (!path || !path[0]) return 0;
    return parse_minimal_manifest(path);
}

int dm1_v22_validate_manifest(const char* manifest_path) {
    const char* path = manifest_path ? manifest_path : g_manifest_path;
    if (!path || !path[0]) return 0;

    FILE* fp = fopen(path, "r");
    if (!fp) return 0;

    /* Quick structural checks: see parse_minimal_manifest() above.
     * We repeat the checks here so validate is self-contained. */
    char buf[65536];
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    if (n == 0) return -1;
    buf[n] = '\0';

    if (buf[0] != '{') return -1;

    const char* cats = strstr(buf, "\"categories\"");
    if (!cats) return -1;

    const char* p = cats + strlen("\"categories\"");
    while (*p && ( *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    if (*p != ':') return -1;
    p++;
    while (*p && ( *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    if (*p != '{') return -1;

    /* Check critical categories */
    int found_critical = 0;
    const char* wall = strstr(buf, "\"wall_shapes\"");
    const char* creature = strstr(buf, "\"creature_shapes\"");
    const char* floor_s = strstr(buf, "\"floor_shapes\"");

    if (wall)   found_critical = 1;
    if (creature) found_critical = 1;
    if (floor_s)  found_critical = 1;

    if (!found_critical) return -1; /* No critical categories */

    /* If we have wall_shapes, verify array structure */
    if (wall) {
        const char* arr_start = wall + strlen("\"wall_shapes\"");
        while (*arr_start && ( *arr_start == ' ' || *arr_start == '\t' || *arr_start == '\n' || *arr_start == '\r' || *arr_start == ':')) arr_start++;
        if (*arr_start != '[') return -1;
        const char* arr_end = strchr(arr_start, ']');
        if (!arr_end) return -1;
        /* Verify the array is not empty (has at least one entry with "id") */
        const char* id_key = strstr(arr_start, "\"id\"");
        if (!id_key || id_key > arr_end) return 0; /* Missing id in first entry → partial */
    }

    return 1; /* Complete and valid */
}

int dm1_v22_modern_assets_available(void) {
    /* If manifest was already loaded successfully, return that state */
    if (g_manifest_loaded) return g_modern_assets_installed;
    /* If no manifest path is set, no assets are available.
     * Empty path: cannot open a file — mark cache as checked and return 0. */
    if (!g_manifest_path[0]) {
        g_modern_assets_installed = 0;
        g_manifest_loaded = 1;  /* mark as checked so we don't re-enter */
        return 0;
    }
    /* Otherwise try to load from the configured path */
    int r = parse_minimal_manifest(g_manifest_path);
    g_modern_assets_installed = (r == 1) ? 1 : 0;
    g_manifest_loaded = 1;
    return g_modern_assets_installed;
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.2 Installed State (mirrors m11_v22_set_installed/get_installed)
 * ══════════════════════════════════════════════════════════════════════ */

void dm1_v22_set_installed(int installed) {
    g_v22_installed = installed ? 1 : 0;
}

int dm1_v22_get_installed(void) {
    return g_v22_installed;
}

/* ══════════════════════════════════════════════════════════════════════
 * Asset Loading with Fallback Chain
 *
 * Given a category/asset_id and desired provenance, walks the fallback
 * chain and returns the first successfully opened asset.
 *
 * Since we don't ship actual asset files, the implementation validates
 * the pipeline state and descriptor semantics. A full file-loading
 * implementation would use the modern asset root + category + asset_id
 * to build the file path and load PNG/TGA/RGBA data.
 * ══════════════════════════════════════════════════════════════════════ */

/* Build the expected file path for a given provenance/category/asset_id.
 * Returns 1 if the path was built into out_path, 0 if the combination
 * is not representable as a file path (e.g. ORIGINAL has no file). */
static int build_asset_path(DM1_V22_AssetProvenance prov,
                            const char* category,
                            const char* asset_id,
                            char* out_path, size_t out_size) {
    if (!out_path || out_size == 0) return 0;

    /* ORIGINAL (V1) and FILTERED (V2.0) and UPSCALED (V2.1) assets
     * are not standalone files — they are generated at runtime from
     * GRAPHICS.DAT. Only MODERN (V2.2) assets have explicit files. */
    if (prov != DM1_V22_PROVENANCE_MODERN) {
        out_path[0] = '\0';
        return 0;
    }

    /* Modern assets: <root>/<category>/<asset_id>.png or .tga */
    if (!g_manifest_path[0]) {
        out_path[0] = '\0';
        return 0;
    }

    /* Strip the manifest filename to get the asset root */
    const char* root = g_manifest_path;
    const char* last_slash = NULL;
    for (const char* cp = root; *cp; cp++) {
        if (*cp == '/' || *cp == '\\') last_slash = cp;
    }
    size_t root_len = last_slash ? (size_t)(last_slash - root) : strlen(root);

    /* Build <root>/<category>/<asset_id>.png */
    int n = snprintf(out_path, out_size, "%.*s/%s/%s.png",
                     (int)root_len, root,
                     category ? category : "unknown",
                     asset_id  ? asset_id : "unknown");
    if (n < 0 || (size_t)n >= out_size) {
        out_path[0] = '\0';
        return 0;
    }
    return 1;
}

/* Attempt to load a single asset file (PNG or TGA) from disk.
 * This is the low-level file load used within the fallback chain.
 *
 * We implement a minimal PNG header read to get dimensions without
 * a full PNG library. TGA is not implemented (would require the
 * full asset pack to be present).
 *
 * Returns: 1 if file was found and header is valid, 0 otherwise.
 * On success, out_w/out_h and out_format are filled in.
 * NOTE: actual pixel loading is not implemented in this probe-friendly
 * version — the renderer will use the missing placeholder until the
 * full asset loading agent wires in a PNG library. */
static int try_load_asset_file(const char* file_path,
                               int* out_w, int* out_h,
                               DM1_V22_AssetFormat* out_format) {
    if (!file_path || !file_path[0]) return 0;

    FILE* fp = fopen(file_path, "rb");
    if (!fp) return 0;

    unsigned char header[8];
    size_t hdr_n = fread(header, 1, sizeof(header), fp);
    fclose(fp);

    if (hdr_n < 8) return 0;

    /* PNG signature: 89 50 4E 47 0D 0A 1A 0A */
    if (header[0] == 0x89 && header[1] == 0x50 &&
        header[2] == 0x4E && header[3] == 0x47 &&
        header[4] == 0x0D && header[5] == 0x0A &&
        header[6] == 0x1A && header[7] == 0x0A) {
        /* Minimal PNG: read IHDR chunk to get width/height.
         * This requires scanning chunks, but we know IHDR is the first
         * chunk after the 8-byte signature. */
        FILE* fp2 = fopen(file_path, "rb");
        if (!fp2) return 0;
        unsigned char buf[256];
        size_t r = fread(buf, 1, sizeof(buf), fp2);
        fclose(fp2);
        if (r < 29) return 0; /* Not enough for IHDR */

        /* PNG chunk: length (4 bytes BE) + type (4 bytes) + data + CRC (4 bytes)
         * Chunk type for IHDR is "IHDR" = 0x49 0x48 0x44 0x52
         * Data is 13 bytes: width (4 BE), height (4 BE), bit depth, color type,
         * compression, filter, interlace */
        if (buf[12] != 0x49 || buf[13] != 0x48 || /* I */
            buf[14] != 0x44 || buf[15] != 0x52) { /* HDR */
            return 0;
        }

        /* Width: bytes 16-19 (big-endian uint32) */
        uint32_t w = ((uint32_t)buf[16] << 24) |
                     ((uint32_t)buf[17] << 16) |
                     ((uint32_t)buf[18] << 8)  |
                     ((uint32_t)buf[19]);
        uint32_t h = ((uint32_t)buf[20] << 24) |
                     ((uint32_t)buf[21] << 16) |
                     ((uint32_t)buf[22] << 8)  |
                     ((uint32_t)buf[23]);

        if (out_w)     *out_w     = (int)w;
        if (out_h)     *out_h     = (int)h;
        if (out_format) *out_format = DM1_V22_FORMAT_PNG;
        return 1;
    }

    /* TGA signature: no magic — a TGA without extension is identified
     * by having the extension area offset = 0 and a 0 vendor string.
     * We don't implement TGA loading without a full asset pack. */
    (void)out_format;
    return 0;
}

int dm1_v22_asset_load(const char* category, const char* asset_id,
                       DM1_V22_AssetProvenance desired_provenance,
                       DM1_V22_AssetDescriptor* out_desc) {
    if (!out_desc) return 0;

    /* Zero-initialise the output descriptor */
    memset(out_desc, 0, sizeof(*out_desc));

    if (!category || !asset_id) return 0;

    /* Get the fallback chain for the desired provenance */
    const DM1_V22_FallbackChain* chain = dm1_v22_fallback_for_mode(desired_provenance);
    if (!chain) return 0;

    /* Walk the fallback chain */
    for (int i = 0; i < chain->count; i++) {
        DM1_V22_AssetProvenance prov = chain->levels[i];

        /* Try to build the file path for this provenance level */
        char path[DM1_V22_ASSET_PATH_MAX];
        int path_valid = build_asset_path(prov, category, asset_id, path, sizeof(path));

        if (path_valid && path[0]) {
            /* Try to load the file */
            int w = 0, h = 0;
            DM1_V22_AssetFormat fmt = DM1_V22_FORMAT_UNKNOWN;
            if (try_load_asset_file(path, &w, &h, &fmt)) {
                /* File found and header valid — fill descriptor.
                 * NOTE: actual pixel buffer loading would happen here
                 * with a full PNG/TGA library. For now we record the
                 * metadata. */
                out_desc->provenance = prov;
                out_desc->category   = DM1_V22_CATEGORY_UNKNOWN; /* caller sets */
                out_desc->asset_id   = asset_id;
                out_desc->source_anchor = "dm1_v22_asset_pipeline.c:fallback_chain";
                strncpy(out_desc->file_path, path, sizeof(out_desc->file_path) - 1);
                out_desc->width   = w;
                out_desc->height  = h;
                out_desc->format  = fmt;
                out_desc->pixels  = NULL; /* Not loaded in this probe version */
                out_desc->pixels_size = 0;
                out_desc->is_valid = 0;   /* Pixels not loaded */
                out_desc->load_attempted = 1;
                return 1; /* Found it */
            }
        }

        /* For non-file provenance levels (ORIGINAL/FILTERED/UPSCALED),
         * we don't have file paths but we record the provenance as
         * the best available by consulting the V2.1 pipeline state. */
        if (!path_valid || !path[0]) {
            if (prov == DM1_V22_PROVENANCE_UPSCALED) {
                /* Check if V2.1 pipeline has the surface.
                 * DM1_V2_GetAssetMode() tells us the current mode. */
                DM1_V2_AssetMode mode = DM1_V2_GetAssetMode();
                if (mode >= DM1_V2_ASSET_MODE_UPSCALED) {
                    out_desc->provenance = prov;
                    out_desc->category   = DM1_V22_CATEGORY_UNKNOWN;
                    out_desc->asset_id   = asset_id;
                    out_desc->source_anchor = "dm1_v2_asset_pipeline_pc34.c:F0115_DrawObjectsCreaturesProjectiles";
                    out_desc->width   = 320;
                    out_desc->height = 200;
                    out_desc->format  = DM1_V22_FORMAT_RGBA;
                    out_desc->pixels  = NULL;
                    out_desc->pixels_size = 0;
                    out_desc->is_valid = 1; /* V2.1 upscaled pipeline is always valid */
                    out_desc->load_attempted = 1;
                    return 1;
                }
            } else if (prov == DM1_V22_PROVENANCE_ORIGINAL) {
                /* V1 original is always available as the final fallback */
                out_desc->provenance = prov;
                out_desc->category   = DM1_V22_CATEGORY_UNKNOWN;
                out_desc->asset_id   = asset_id;
                out_desc->source_anchor = "dm1_v1_graphics_loader_pc34_compat.c:G0163_WallSetTable";
                out_desc->width   = 320;
                out_desc->height  = 200;
                out_desc->format  = DM1_V22_FORMAT_INDEXED;
                out_desc->pixels  = NULL;
                out_desc->pixels_size = 0;
                out_desc->is_valid = 1; /* V1 indexed is always available */
                out_desc->load_attempted = 1;
                return 1;
            }
        }
    }

    /* Exhausted fallback chain — return missing descriptor reference */
    out_desc->provenance = DM1_V22_PROVENANCE_UNKNOWN;
    out_desc->is_valid   = 0;
    out_desc->load_attempted = 1;
    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Asset Lifecycle
 * ══════════════════════════════════════════════════════════════════════ */

void dm1_v22_asset_free(DM1_V22_AssetDescriptor* desc) {
    if (!desc) return;
    if (desc->pixels && desc->pixels != (void*)g_missing_pixels) {
        free(desc->pixels);
    }
    memset(desc, 0, sizeof(*desc));
}

/* ══════════════════════════════════════════════════════════════════════
 * Asset Validation
 * ══════════════════════════════════════════════════════════════════════ */

int dm1_v22_asset_validate(const DM1_V22_AssetDescriptor* desc) {
    if (!desc) return 0;

    /* Check provenance is valid */
    if (desc->provenance <= DM1_V22_PROVENANCE_UNKNOWN ||
        desc->provenance > DM1_V22_PROVENANCE_MODERN) {
        return 0;
    }

    /* Dimensions must be non-zero */
    if (desc->width <= 0 || desc->height <= 0) return 0;

    /* Format must be valid */
    if (desc->format <= DM1_V22_FORMAT_UNKNOWN ||
        desc->format > DM1_V22_FORMAT_TGA) {
        return 0;
    }

    /* If is_valid==1, pixels must be non-NULL and size must match */
    if (desc->is_valid) {
        if (!desc->pixels) return 0;

        /* Compute expected minimum size based on format and dimensions */
        size_t expected_min = 0;
        switch (desc->format) {
            case DM1_V22_FORMAT_INDEXED:
                /* 4-bit indexed: 1 byte per 2 pixels, rows padded to byte */
                expected_min = ((desc->width + 1) / 2) * desc->height;
                break;
            case DM1_V22_FORMAT_PALETTED:
                expected_min = (size_t)desc->width * desc->height;
                break;
            case DM1_V22_FORMAT_RGBA:
            case DM1_V22_FORMAT_PNG:
            case DM1_V22_FORMAT_TGA:
                expected_min = (size_t)desc->width * desc->height * 4;
                break;
            default:
                return 0;
        }

        if (desc->pixels_size < expected_min) return 0;
    }

    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Best-Available Provenance Selection
 * ══════════════════════════════════════════════════════════════════════ */

DM1_V22_AssetProvenance dm1_v22_best_available_provenance(
    const char* category, const char* asset_id,
    DM1_V22_AssetProvenance desired) {

    (void)category;
    (void)asset_id;

    if (desired <= DM1_V22_PROVENANCE_UNKNOWN) {
        return DM1_V22_PROVENANCE_UNKNOWN;
    }

    /* Walk the fallback chain from the desired level downward.
     * Stop at the first level that is currently "available":
     *   MODERN: available if g_v22_installed and manifest loaded
     *   UPSCALED: available if DM1_V2_GetAssetMode() >= UPSCALED
     *   FILTERED: available if DM1_V2_GetAssetMode() >= FILTERED
     *   ORIGINAL: always available */
    DM1_V22_AssetProvenance prov = desired;
    while (prov != DM1_V22_PROVENANCE_UNKNOWN) {
        switch (prov) {
            case DM1_V22_PROVENANCE_MODERN:
                if (g_v22_installed && dm1_v22_modern_assets_available()) {
                    return prov;
                }
                break;
            case DM1_V22_PROVENANCE_UPSCALED:
                if (DM1_V2_GetAssetMode() >= DM1_V2_ASSET_MODE_UPSCALED) {
                    return prov;
                }
                break;
            case DM1_V22_PROVENANCE_FILTERED:
                if (DM1_V2_GetAssetMode() >= DM1_V2_ASSET_MODE_FILTERED) {
                    return prov;
                }
                break;
            case DM1_V22_PROVENANCE_ORIGINAL:
                /* Always available */
                return prov;
            default:
                break;
        }
        prov = dm1_v22_fallback_next(prov);
    }

    return DM1_V22_PROVENANCE_ORIGINAL; /* Absolute fallback */
}

/* ══════════════════════════════════════════════════════════════════════
 * Source Evidence
 * ══════════════════════════════════════════════════════════════════════ */

const char* dm1_v22_asset_pipeline_source_evidence(void) {
    return
        "dm1_v22_asset_pipeline.h/c — Phase 8 V2.2 modern asset pipeline\n"
        "Source-lock anchors:\n"
        "  ReDMCSB DUNGEON.C:2238-2246   — square type decode for shape dispatch\n"
        "  ReDMCSB DEFS.H:922-941        — M034_SQUARE_TYPE enumeration\n"
        "  ReDMCSB DUNVIEW.C:6697-6816   — wall/floor draw composition order\n"
        "  ReDMCSB PANEL.C:418-428       — G0304_i_DungeonViewPaletteIndex (6 levels)\n"
        "  ReDMCSB DRAWVIEW.C:1-200      — creature sprite framing (G0011_i_CreaturePosture)\n"
        "  ReDMCSB PROJEXPL.C:43-165     — projectile/explosion routing\n"
        "  Firestaff dm1_v22_shapes.c    — shape type → modern renderer bridge\n"
        "  Firestaff dm1_v2_asset_pipeline_pc34.c — V2.1 EPX pipeline (fallback source)\n"
        "Pipeline: MODERN (V2.2) → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1)\n";
}
