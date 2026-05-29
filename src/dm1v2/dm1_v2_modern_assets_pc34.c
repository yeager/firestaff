/*
 * dm1_v2_modern_assets_pc34.c — V2.2 Modern Graphics Fallback Pipeline
 *
 * V2.2 "Modern Graphics" is the third mode alongside V2.0 (Filtered) and
 * V2.1 (Upscaled). When V2.2 assets are unavailable or disabled, the
 * system gracefully falls back to the next-best available mode.
 *
 * Fallback chain:
 *   MODERN (V2.2) → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1)
 *
 * Modern assets are shipped as a separate asset pack installed to:
 *   ~/.firestaff/assets/dm1/modern/
 *
 * Each pack contains:
 *   modern_asset_manifest.json  — catalog of all modern asset files
 *   shapes/                      — wall, floor, creature, object shapes
 *   ui_chrome/                   — panel chrome and UI chrome
 *   champion_portraits/           — champion portrait textures
 *
 * Required categories for a complete install:
 *   wall_shapes, floor_shapes, creature_shapes, ui_chrome, champion_portraits
 *
 * Asset-not-found guard: if a specific modern asset referenced in the
 * manifest cannot be opened, the shape system returns
 * DM1_V22_SHAPE_MISSING_PLACEHOLDER rather than crashing.
 */

#include "dm1_v2_asset_pipeline_pc34.h"
#include "fs_portable_compat.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Module config ────────────────────────────────────────────────── */

/* V2.2 modern assets are disabled by default until detected at startup.
 * After M12_AssetStatus_Scan() runs, this flag reflects whether the
 * modern asset pack was found and validated. */
static int g_v22_modern_assets_installed = 0;

/* EPX cache state: tracks whether the V2.1 EPX upscale cache has been
 * populated for a given surface. When the cache is cold and the user
 * selected UPSCALED mode, we fall back to FILTERED (V2.0). */
static int g_epx_cache_warm = 0;

/* ── Manifest paths ───────────────────────────────────────────────── */

/* Path to the modern asset manifest, computed from the Firestaff data
 * directory at startup (via M12_AssetStatus_Scan) or from the default
 * ~/.firestaff/assets/dm1/modern/ location. */
static char g_v22_manifest_path[FSP_PATH_MAX];

/* ── Placeholder for missing assets ──────────────────────────────── */

/* DM1_V22_SHAPE_MISSING_PLACEHOLDER — returned by shape-fetching
 * functions when a modern asset referenced in the manifest cannot
 * be opened. This is a 16×16 RGBA magenta checkerboard pattern that
 * clearly signals "missing texture" in-game without crashing. */
static const uint32_t DM1_V22_SHAPE_MISSING_PLACEHOLDER_DATA[16 * 16] = {
    0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF,
    0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF,
    0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF,
    0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF,
    0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF,
    0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF,
    0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF,
    0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF,
    0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF,
    0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF,
    0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF,
    0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF,
    0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF,
    0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF,
    0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFF000000, 0xFFFF00FF,
    0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF, 0xFFFF00FF,
};

/* ── Shape source enum is defined in dm1_v2_asset_pipeline_pc34.h
 * (shared between the header API declarations and this implementation). */

/* ── JSON manifest parsing (minimal, no external library) ────────── */

/* Required categories in the manifest. All must be present for a
 * complete install; missing categories reduce completeness to "partial". */
static const char* const k_required_categories[] = {
    "wall_shapes",
    "floor_shapes",
    "creature_shapes",
    "ui_chrome",
    "champion_portraits",
    NULL
};

/* Required fields per manifest entry */
static const char* const k_required_fields[] = {
    "id",
    "source_file",
    "width",
    "height",
    NULL
};

static int m11_v22_file_exists(const char* path) {
    if (!path || path[0] == '\0') return 0;
    FILE* fp = fopen(path, "rb");
    if (fp) { fclose(fp); return 1; }
    return 0;
}

/* Trimming helpers */
static void m11_v22_trim(char* dst, const char* src, size_t dstSize) {
    if (!dst || dstSize == 0U) return;
    const char* start = src;
    if (!start) { dst[0] = '\0'; return; }
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') ++start;
    size_t len = strlen(start);
    const char* end = start + len;
    while (len > 0U && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) {
        --end; --len;
    }
    if (len >= dstSize) len = dstSize - 1U;
    memcpy(dst, start, len);
    dst[len] = '\0';
}

/* Read a line from a FILE*, stripping trailing whitespace */
static int m11_v22_read_line(FILE* fp, char* out, size_t outSize) {
    if (!fp || !out || outSize == 0U) return 0;
    out[0] = '\0';
    if (!fgets(out, (int)outSize, fp)) return 0;
    m11_v22_trim(out, out, outSize);
    return 1;
}

/* Extract a string value for a key from a JSON line of the form: "key": "value"
 * Returns 1 on success, 0 if not found. */
static int m11_v22_extract_string(const char* line, const char* key, char* out, size_t outSize) {
    if (!line || !key || !out || outSize == 0U) return 0;
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char* p = strstr(line, pattern);
    if (!p) return 0;
    p += strlen(pattern);
    while (*p == ' ' || *p == ':' || *p == '\t') ++p;
    if (*p != '"') return 0;
    ++p;
    size_t dst = 0U;
    while (p[0] != '\0' && dst < outSize - 1U) {
        if (p[0] == '\\' && p[1] != '\0') ++p;
        if (p[0] == '"') break;
        out[dst++] = p[0];
        ++p;
    }
    out[dst] = '\0';
    return dst > 0U ? 1 : 0;
}

/* Extract an integer value for a key from a JSON line of the form: "key": 123
 * Returns 1 on success, 0 if not found. */
static int m11_v22_extract_int(const char* line, const char* key, int* out) {
    if (!line || !key || !out) return 0;
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char* p = strstr(line, pattern);
    if (!p) return 0;
    p += strlen(pattern);
    while (*p == ' ' || *p == ':' || *p == '\t') ++p;
    char* end = NULL;
    long val = strtol(p, &end, 10);
    if (end == p || val == 0) return 0;
    *out = (int)val;
    return 1;
}

/* Skip to the start of the next JSON object/array in the file */
static void m11_v22_skip_to_next_object(FILE* fp) {
    int depth = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        for (char* c = line; *c; ++c) {
            if (*c == '{') { depth++; break; }
            if (*c == '[') { depth++; break; }
        }
        if (depth == 0) {
            /* Rewind to start of this line for next pass */
            long pos = ftell(fp);
            fgets(line, sizeof(line), fp); /* consume current line */
            (void)pos;
            break;
        }
        depth--;
    }
}

/* ── Public API ──────────────────────────────────────────────────── */

/* m11_v22_set_manifest_path — set the path to the modern asset manifest.
 * Called by M12_AssetStatus_Scan() after resolving the data directory.
 * The manifest path is derived as: <dataDir>/../assets/dm1/modern/
 * modern_asset_manifest.json */
void m11_v22_set_manifest_path(const char* dataDir) {
    if (!dataDir || dataDir[0] == '\0') {
        g_v22_manifest_path[0] = '\0';
        return;
    }
    /* Build: <dataDir>/../assets/dm1/modern/modern_asset_manifest.json */
    char assetsDir[FSP_PATH_MAX];
    FSP_JoinPath(assetsDir, sizeof(assetsDir), dataDir, "..");
    FSP_JoinPath(assetsDir, sizeof(assetsDir), assetsDir, "assets");
    FSP_JoinPath(assetsDir, sizeof(assetsDir), assetsDir, "dm1");
    FSP_JoinPath(assetsDir, sizeof(assetsDir), assetsDir, "modern");
    FSP_JoinPath(g_v22_manifest_path, sizeof(g_v22_manifest_path),
                  assetsDir, "modern_asset_manifest.json");
}

/* m11_v22_validate_manifest — validates the JSON manifest.
 *
 * Checks:
 *   - File exists and is readable
 *   - All required categories are present (wall_shapes, floor_shapes,
 *     creature_shapes, ui_chrome, champion_portraits)
 *   - Each entry has required fields (id, source_file, width, height)
 *
 * Returns:
 *   -1  — error (missing manifest, unreadable, or fundamentally invalid)
 *    0  — partial (manifest readable but some categories/entries incomplete)
 *    1  — complete (all required categories present with valid entries)
 *
 * Note: This performs a quick validation pass only (first entry of
 * each category). A full validation counts every entry but is too
 * slow for startup; it is deferred to a background thread if needed. */
int m11_v22_validate_manifest(const char* manifest_path) {
    FILE* fp;
    char line[256];
    int found_categories = 0;
    int total_required = 0;
    int categories_with_entries = 0;
    int current_category = -1;
    int entries_in_current_category = 0;
    int entry_has_all_fields = 1;
    const size_t k_num_required_cats =
        sizeof(k_required_categories) / sizeof(k_required_categories[0]) - 1U;

    if (!manifest_path || manifest_path[0] == '\0') return -1;

    fp = fopen(manifest_path, "rb");
    if (!fp) return -1;

    /* Scan the file for categories and validate first entry per category */
    while (m11_v22_read_line(fp, line, sizeof(line))) {
        /* Detect category line: "category_name": [ or "category_name": { */
        int is_object = 0;
        for (size_t ci = 0U; k_required_categories[ci] != NULL; ++ci) {
            char cat_pattern[64];
            snprintf(cat_pattern, sizeof(cat_pattern), "\"%s\":", k_required_categories[ci]);
            if (strncmp(line, cat_pattern, strlen(cat_pattern)) == 0) {
                /* Finish validating previous category */
                if (current_category >= 0 && entry_has_all_fields) {
                    categories_with_entries++;
                }
                /* Save previous count toward total */
                total_required += (current_category >= 0 && entries_in_current_category > 0) ? 1 : 0;

                current_category = (int)ci;
                found_categories++;
                entries_in_current_category = 0;
                entry_has_all_fields = 1;
                is_object = 1;
                break;
            }
        }

        if (!is_object) continue;

        /* Consume the opening brace or bracket for this category */
        if (strchr(line, '[') == NULL && strchr(line, '{') == NULL) {
            /* Read next line to find opening */
            if (!m11_v22_read_line(fp, line, sizeof(line))) break;
        }

        /* Scan entries within this category until we hit the closing bracket/brace */
        int depth = 0;
        for (;;) {
            if (!m11_v22_read_line(fp, line, sizeof(line))) break;
            for (char* c = line; *c; ++c) {
                if (*c == '{') depth++;
                if (*c == '}') { depth--; if (depth < 0) depth = 0; }
                if (*c == '[') depth++;
                if (*c == ']') { depth--; if (depth < 0) { depth = 0; break; } }
            }
            if (depth == 0 && strchr(line, '}') == NULL && strchr(line, ']') == NULL) {
                /* End of category */
                break;
            }
            /* This is an entry line — check required fields */
            if (strchr(line, '{') != NULL || strchr(line, '"') != NULL) {
                char id_val[128] = {0};
                char file_val[256] = {0};
                int width_val = 0, height_val = 0;
                int has_id = m11_v22_extract_string(line, "id", id_val, sizeof(id_val));
                int has_file = m11_v22_extract_string(line, "source_file", file_val, sizeof(file_val));
                int has_width = m11_v22_extract_int(line, "width", &width_val);
                int has_height = m11_v22_extract_int(line, "height", &height_val);
                if (!has_id || !has_file || !has_width || !has_height || width_val <= 0 || height_val <= 0) {
                    entry_has_all_fields = 0;
                }
                entries_in_current_category++;
                /* We only validate the first entry; full validation is too slow for startup */
                break;
            }
        }
    }

    /* Close out the last category */
    if (current_category >= 0) {
        if (entry_has_all_fields) categories_with_entries++;
        total_required += entries_in_current_category > 0 ? 1 : 0;
    }

    fclose(fp);

    if (found_categories == 0) return -1;
    if (categories_with_entries < (int)k_num_required_cats) return 0;
    return 1;
}

/* m11_v22_modern_assets_available — checks if the modern asset pack
 * is installed and at least critical shape categories are present.
 *
 * Checks:
 *   - ~/.firestaff/assets/dm1/modern/modern_asset_manifest.json exists
 *   - All critical categories have at least one entry (wall_shapes,
 *     floor_shapes, creature_shapes — UI chrome is optional for playability)
 *
 * Returns: 1 if available, 0 if not installed or partial */
int m11_v22_modern_assets_available(void) {
    if (g_v22_manifest_path[0] == '\0') return 0;
    if (!m11_v22_file_exists(g_v22_manifest_path)) return 0;

    /* Quick check: open the manifest and look for at least the three
     * critical categories (wall_shapes, floor_shapes, creature_shapes)
     * with at least one entry each. */
    FILE* fp = fopen(g_v22_manifest_path, "rb");
    if (!fp) return 0;

    static const char* const critical_cats[] = {
        "wall_shapes", "floor_shapes", "creature_shapes", NULL
    };
    int found_critical[3] = {0, 0, 0};
    char line[256];
    int current_cat = -1;

    while (m11_v22_read_line(fp, line, sizeof(line))) {
        for (int ci = 0; critical_cats[ci] != NULL; ++ci) {
            char pattern[64];
            snprintf(pattern, sizeof(pattern), "\"%s\":", critical_cats[ci]);
            if (strncmp(line, pattern, strlen(pattern)) == 0) {
                current_cat = ci;
                break;
            }
        }
        if (current_cat >= 0) {
            /* Scan for first entry with a valid id */
            while (m11_v22_read_line(fp, line, sizeof(line))) {
                if (strchr(line, '}') != NULL || strchr(line, ']') != NULL) {
                    current_cat = -1;
                    break;
                }
                char id_val[64];
                if (m11_v22_extract_string(line, "id", id_val, sizeof(id_val))) {
                    found_critical[current_cat] = 1;
                    current_cat = -1;
                    break;
                }
            }
        }
    }
    fclose(fp);

    return (found_critical[0] && found_critical[1] && found_critical[2]) ? 1 : 0;
}

/* m11_v22_set_installed — called at startup by M12_AssetStatus_Scan()
 * after detecting whether the modern asset pack is present. */
void m11_v22_set_installed(int installed) {
    g_v22_modern_assets_installed = installed ? 1 : 0;
}

/* m11_v22_get_installed — returns whether V2.2 modern assets are installed */
int m11_v22_get_installed(void) {
    return g_v22_modern_assets_installed;
}

/* m11_v22_set_epx_cache_warm / m11_v22_get_epx_cache_warm — EPX cache
 * state for V2.1 upscale. When UPSCALED mode is selected but the EPX
 * cache is cold, the system falls back to FILTERED (V2.0). */
void m11_v22_set_epx_cache_warm(int warm) {
    g_epx_cache_warm = warm ? 1 : 0;
}

int m11_v22_get_epx_cache_warm(void) {
    return g_epx_cache_warm;
}

/* m11_v22_best_available_shape_source — returns the best available
 * shape source for the current config and asset state.
 *
 * Fallback chain: MODERN → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1)
 *
 * Logic:
 *   - If V2.2 (MODERN) selected AND modern assets installed → V2_MODERN
 *   - If V2.2 selected BUT modern assets NOT installed → V2_UPSCALED (warn)
 *   - If V2.1 (UPSCALED) selected AND EPX cache warm → V2_UPSCALED
 *   - If V2.1 selected BUT EPX cache cold → V2_FILTERED (fallback)
 *   - If V2.0 (FILTERED) selected → V2_FILTERED
 *   - Otherwise → V1_ORIGINAL
 *
 * The presentation_mode index (0=V1, 1=V2.0, 2=V2.1, 3=V2.2) maps as:
 *   0 → V1_ORIGINAL
 *   1 → V2_FILTERED
 *   2 → V2_UPSCALED
 *   3 → V2_MODERN (or fallback)
 *
 * Ref: config_m12.h M12_PresentationMode — presentation mode index */
DM1_V22_ShapeSource m11_v22_best_available_shape_source(int presentation_mode_index) {
    switch (presentation_mode_index) {
        case 3: /* M12_PRESENTATION_V22_MODERN */
            if (g_v22_modern_assets_installed) {
                return DM1_V22_SHAPE_SOURCE_V2_MODERN;
            }
            /* Fall through: no modern assets, fall back to V2.1 */
            fprintf(stderr, "[V2.2] V2.2 modern assets not available, "
                            "falling back to V2.1 upscaled\n");
            /* fall through */
        case 2: /* M12_PRESENTATION_V21_UPSCALED */
            if (g_epx_cache_warm) {
                return DM1_V22_SHAPE_SOURCE_V2_UPSCALED;
            }
            /* EPX cache cold — fall back to V2.0 filtered */
            fprintf(stderr, "[V2.2] EPX cache cold, falling back to V2.0 filtered\n");
            return DM1_V22_SHAPE_SOURCE_V2_FILTERED;

        case 1: /* M12_PRESENTATION_V20_FILTERED */
            return DM1_V22_SHAPE_SOURCE_V2_FILTERED;

        case 0:
        default:
            return DM1_V22_SHAPE_SOURCE_V1_ORIGINAL;
    }
}

/* m11_v22_get_missing_placeholder — returns the missing-asset
 * placeholder surface (16×16 RGBA). Never returns NULL. */
const uint32_t* m11_v22_get_missing_placeholder(int* out_w, int* out_h) {
    if (out_w) *out_w = 16;
    if (out_h) *out_h = 16;
    return DM1_V22_SHAPE_MISSING_PLACEHOLDER_DATA;
}

/* m11_v22_get_shape_path — given a category and asset id from the
 * manifest, returns the full filesystem path to the asset file.
 * Returns 1 on success, 0 if not found or manifest not available. */
int m11_v22_get_shape_path(const char* category, const char* asset_id,
                            char* out_path, size_t out_path_size) {
    if (!category || !asset_id || !out_path || out_path_size == 0U) return 0;
    if (g_v22_manifest_path[0] == '\0') return 0;

    FILE* fp = fopen(g_v22_manifest_path, "rb");
    if (!fp) return 0;

    int found_entry = 0;
    char line[256];
    int in_target_category = 0;
    int in_target_entry = 0;
    char resolved_id[64] = {0};
    char resolved_file[256] = {0};

    while (m11_v22_read_line(fp, line, sizeof(line))) {
        /* Category detection */
        char cat_pattern[64];
        snprintf(cat_pattern, sizeof(cat_pattern), "\"%s\":", category);
        if (strncmp(line, cat_pattern, strlen(cat_pattern)) == 0) {
            in_target_category = 1;
            found_entry = 0;
            continue;
        }

        if (!in_target_category) continue;

        /* Entry start */
        if (strchr(line, '{') != NULL && !in_target_entry) {
            in_target_entry = 1;
            resolved_id[0] = '\0';
            resolved_file[0] = '\0';
        }

        if (!in_target_entry) continue;

        /* Field extraction */
        if (strchr(line, '}') != NULL) {
            /* End of entry */
            in_target_entry = 0;
            if (resolved_id[0] != '\0' && strcmp(resolved_id, asset_id) == 0) {
                found_entry = 1;
                break;
            }
            continue;
        }

        /* Extract id and source_file fields */
        char val[256];
        if (m11_v22_extract_string(line, "id", val, sizeof(val))) {
            m11_v22_trim(resolved_id, val, sizeof(resolved_id));
        }
        if (m11_v22_extract_string(line, "source_file", val, sizeof(val))) {
            m11_v22_trim(resolved_file, val, sizeof(resolved_file));
        }
    }

    fclose(fp);

    if (!found_entry || resolved_file[0] == '\0') return 0;

    /* Resolve the file relative to the manifest's directory:
     * <modern_dir>/<category>/<source_file> */
    char modern_dir[FSP_PATH_MAX];
    FSP_JoinPath(modern_dir, sizeof(modern_dir),
                 g_v22_manifest_path, "..");
    /* g_v22_manifest_path is the manifest itself; go up from modern/ */
    char* last_slash = strrchr(g_v22_manifest_path, '/');
    if (last_slash) {
        size_t dir_len = (size_t)(last_slash - g_v22_manifest_path);
        if (dir_len < sizeof(modern_dir)) {
            memcpy(modern_dir, g_v22_manifest_path, dir_len);
            modern_dir[dir_len] = '\0';
        }
    } else {
        modern_dir[0] = '.';
        modern_dir[1] = '\0';
    }

    /* Build: <modern_dir>/<category>/<source_file>
     * Note: we use the manifest's category path directly */
    FSP_JoinPath(out_path, out_path_size, modern_dir, category);
    FSP_JoinPath(out_path, out_path_size, out_path, resolved_file);
    return m11_v22_file_exists(out_path) ? 1 : 0;
}

/* m11_v22_shape_source_name — human-readable name for a shape source */
const char* m11_v22_shape_source_name(DM1_V22_ShapeSource src) {
    switch (src) {
        case DM1_V22_SHAPE_SOURCE_V1_ORIGINAL: return "V1_ORIGINAL";
        case DM1_V22_SHAPE_SOURCE_V2_FILTERED: return "V2_FILTERED";
        case DM1_V22_SHAPE_SOURCE_V2_UPSCALED: return "V2_UPSCALED";
        case DM1_V22_SHAPE_SOURCE_V2_MODERN:   return "V2_MODERN";
        default: return "UNKNOWN";
    }
}
