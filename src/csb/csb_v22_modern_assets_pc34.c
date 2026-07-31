/*
 * csb_v22_modern_assets_pc34.c — CSB V2.2 Modern Graphics Fallback Pipeline
 *
 * V2.2 "Modern Graphics" is the third mode alongside V2.0 (Filtered) and
 * V2.1 (Upscaled). When a complete real V2.2 pack is unavailable, the
 * system gracefully falls back to the next-best available mode.
 *
 * Fallback chain:
 *   MODERN (V2.2) → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1)
 *
 * Modern assets are shipped as a separate asset pack installed to:
 *   ~/.firestaff/assets/csb/modern/
 *
 * Each pack contains:
 *   modern_asset_manifest.json  — catalog of all modern asset files
 *   shapes/                      — wall, floor, creature, object shapes
 *   ui_chrome/                   — panel chrome and UI chrome
 *   champion_portraits/           — champion portrait textures
 *
 * Catalog validation checks wall/floor/creature/UI/portrait categories.
 * Final runtime admission is delegated to the finished-art gate, which
 * verifies every pair emitted by the active per-cell router.
 *
 * Asset-not-found guard: if a specific modern asset referenced in the
 * manifest cannot be opened, the shape system leaves the source-owned V1
 * command intact rather than substituting generated pixels.
 */

#include "csb_v22_modern_assets_pc34.h"
#include "csb_v22_finished_art_material_gate_pc34.h"
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
 * ~/.firestaff/assets/csb/modern/ location. */
static char g_v22_manifest_path[FSP_PATH_MAX];

/* ── Shape source enum is defined in csb_v22_modern_assets_pc34.h
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
static const char* const __attribute__((unused)) k_required_fields [] = {
    "id",
    "source_file",
    "width",
    "height",
    NULL
};

static int csb_v22_file_exists(const char* path) {
    if (!path || path[0] == '\0') return 0;
    FILE* fp = fopen(path, "rb");
    if (fp) { fclose(fp); return 1; }
    return 0;
}

/* Trimming helpers */
static void csb_v22_trim(char* dst, const char* src, size_t dstSize) {
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
static int csb_v22_read_line(FILE* fp, char* out, size_t outSize) {
    if (!fp || !out || outSize == 0U) return 0;
    out[0] = '\0';
    if (!fgets(out, (int)outSize, fp)) return 0;
    csb_v22_trim(out, out, outSize);
    return 1;
}

/* Extract a string value for a key from a JSON line of the form: "key": "value"
 * Returns 1 on success, 0 if not found. */
static int csb_v22_extract_string(const char* line, const char* key, char* out, size_t outSize) {
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
static int csb_v22_extract_int(const char* line, const char* key, int* out) {
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

/* Read a two-value JSON array both from Artpack Studio's compact output
 * (`"sourceDimensions": [64, 61]`) and its pretty-printed output. */
static int csb_v22_extract_int_pair(const char* line, const char* key,
                                    int* first, int* second) {
    char pattern[64];
    const char* p;
    char* end = NULL;
    long a;
    long b;

    if (!line || !key || !first || !second) return 0;
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(line, pattern);
    if (!p) return 0;
    p = strchr(p + strlen(pattern), '[');
    if (!p) return 0;
    a = strtol(p + 1, &end, 10);
    if (end == p + 1) return 0;
    p = strchr(end, ',');
    if (!p) return 0;
    b = strtol(p + 1, &end, 10);
    if (end == p + 1 || a <= 0 || b <= 0) return 0;
    *first = (int)a;
    *second = (int)b;
    return 1;
}

/* Artpacks are user-imported archives.  Their manifest must not be able to
 * escape the selected pack through a category or file path. */
static int csb_v22_safe_path_component(const char* value) {
    const unsigned char* p = (const unsigned char*)value;
    if (!p || !p[0] || strcmp(value, ".") == 0 || strcmp(value, "..") == 0) {
        return 0;
    }
    while (*p) {
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') || *p == '_' || *p == '-' ||
              *p == '.')) {
            return 0;
        }
        ++p;
    }
    return 1;
}

static int csb_v22_sha256_hex(const char* value) {
    size_t i;
    if (!value || strlen(value) != 64u) return 0;
    for (i = 0u; i < 64u; ++i) {
        unsigned char c = (unsigned char)value[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
              (c >= 'A' && c <= 'F'))) {
            return 0;
        }
    }
    return 1;
}

int csb_v22_get_route_provenance(const char* category, const char* asset_id,
                                 CSB_V22_RouteProvenancePc34* out_provenance)
{
    FILE* fp;
    char line[512];
    int in_routes = 0;
    int in_entry = 0;
    int dimensions_seen = 0;
    int output_dimensions_seen = 0;
    CSB_V22_RouteProvenancePc34 current;

    if (out_provenance) memset(out_provenance, 0, sizeof(*out_provenance));
    if (!category || !category[0] || !asset_id || !asset_id[0] ||
        !out_provenance || !g_v22_manifest_path[0]) {
        return 0;
    }
    fp = fopen(g_v22_manifest_path, "rb");
    if (!fp) return 0;
    memset(&current, 0, sizeof(current));
    current.source_graphic_index = -1;

    while (fgets(line, sizeof(line), fp)) {
        if (!in_routes) {
            const char *route_key = strstr(line, "\"routeProvenance\"");
            if (!route_key) {
                continue;
            }
            in_routes = 1;
            /* Artpack Studio may compact the array opener and the first
             * provenance object onto one line.  Only defer processing when
             * that line contains no object after the routeProvenance key;
             * its leading root-object brace must never become an entry. */
            if (strchr(route_key + strlen("\"routeProvenance\""), '{') == NULL) {
                continue;
            }
        }
        if (!in_entry && strchr(line, '{') != NULL) {
            memset(&current, 0, sizeof(current));
            current.source_graphic_index = -1;
            dimensions_seen = 0;
            output_dimensions_seen = 0;
            in_entry = 1;
        }
        if (!in_entry) continue;
        (void)csb_v22_extract_string(line, "id", current.id,
                                     sizeof(current.id));
        (void)csb_v22_extract_string(line, "category", current.category,
                                     sizeof(current.category));
        (void)csb_v22_extract_int(line, "sourceGraphicIndex",
                                  &current.source_graphic_index);
        (void)csb_v22_extract_string(line, "sourceRecordSha256",
                                     current.source_record_sha256,
                                     sizeof(current.source_record_sha256));
        if (csb_v22_extract_int_pair(line, "sourceDimensions",
                                     &current.source_width,
                                     &current.source_height)) {
            dimensions_seen = 0;
        } else if (strstr(line, "\"sourceDimensions\"") != NULL) {
            dimensions_seen = 1;
            continue;
        }
        if (csb_v22_extract_int_pair(line, "outputDimensions",
                                     &current.output_width,
                                     &current.output_height)) {
            output_dimensions_seen = 0;
        } else if (strstr(line, "\"outputDimensions\"") != NULL) {
            output_dimensions_seen = 1;
            continue;
        }
        if (dimensions_seen && current.source_width == 0) {
            current.source_width = (int)strtol(line, NULL, 10);
            continue;
        }
        if (dimensions_seen && current.source_height == 0) {
            current.source_height = (int)strtol(line, NULL, 10);
            dimensions_seen = 0;
            continue;
        }
        if (output_dimensions_seen && current.output_width == 0) {
            current.output_width = (int)strtol(line, NULL, 10);
            continue;
        }
        if (output_dimensions_seen && current.output_height == 0) {
            current.output_height = (int)strtol(line, NULL, 10);
            output_dimensions_seen = 0;
        }
        if (strchr(line, '}') != NULL) {
            if (current.id[0] && current.category[0] &&
                current.source_graphic_index >= 0 && current.source_width > 0 &&
                current.source_height > 0 && current.output_width > 0 &&
                current.output_height > 0 &&
                csb_v22_sha256_hex(current.source_record_sha256) &&
                strcmp(current.id, asset_id) == 0 &&
                strcmp(current.category, category) == 0) {
                current.valid = 1;
                *out_provenance = current;
                fclose(fp);
                return 1;
            }
            in_entry = 0;
        }
    }
    fclose(fp);
    return 0;
}

/* Skip to the start of the next JSON object/array in the file */
static void __attribute__((unused)) csb_v22_skip_to_next_object (FILE* fp) {
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

/* csb_v22_set_manifest_path — set the path to the modern asset manifest.
 * Called by M12_AssetStatus_Scan() after resolving the data directory.
 * The manifest path is derived as: ~/.firestaff/assets/csb/modern/
 * modern_asset_manifest.json (per docs/v2_2_asset_manifest.md and
 * docs/v2_2_asset_provenance_schema.md).
 *
 * dataDir is the CSB game data directory path (e.g. ~/.firestaff/data/csb).
 * We walk up two levels to reach ~/.firestaff, then append
 * assets/csb/modern/modern_asset_manifest.json.
 *
 * CRITICAL: We must use FSP_ParentDir to strip the last path segment,
 * NOT FSP_JoinPath with "..". The ".." join preserves the directory
 * name in the path string (e.g. "csb/../" doesn't simplify to just the
 * parent), so using it would produce a manifest path that contains the
 * dataDir name (e.g. ".../csb/../assets/...") which never resolves to
 * the actual file. FSP_ParentDir strips the last segment cleanly.
 *
 * Example: dataDir="/home/user/.firestaff/data/csb"
 *   ParentDir → "/home/user/.firestaff/data"
 *   ParentDir → "/home/user/.firestaff"
 *   assets/csb/modern → correct modern assets directory.
 *
 * Source: FSP_ParentDir finds the last separator and truncates there. */
void csb_v22_set_manifest_path(const char* dataDir) {
    char resolved_data_dir[FSP_PATH_MAX];
    const char *candidates[2];
    int candidate_count = 1;
    int candidate_index;
    if (!dataDir || dataDir[0] == '\0') {
        g_v22_manifest_path[0] = '\0';
        csb_v22_famg_set_manifest_path(NULL);
        return;
    }
    candidates[0] = dataDir;
    if (FSP_ResolvePhysicalPath(resolved_data_dir,
                                sizeof(resolved_data_dir), dataDir) &&
        strcmp(resolved_data_dir, dataDir) != 0) {
        candidates[candidate_count++] = resolved_data_dir;
    }
    for (candidate_index = 0; candidate_index < candidate_count;
         ++candidate_index) {
        char parent1[FSP_PATH_MAX];
        char parent2[FSP_PATH_MAX];
        char assetsRoot[FSP_PATH_MAX];
        char modernDir[FSP_PATH_MAX];
        char manifest[FSP_PATH_MAX];
        const char *asset_data_dir = candidates[candidate_index];
        if (!FSP_ParentDir(parent1, sizeof(parent1), asset_data_dir) ||
            !FSP_ParentDir(parent2, sizeof(parent2), parent1)) {
            FSP_JoinPath(assetsRoot, sizeof(assetsRoot), asset_data_dir,
                         "assets");
        } else {
            FSP_JoinPath(assetsRoot, sizeof(assetsRoot), parent2, "assets");
        }
        FSP_JoinPath(modernDir, sizeof(modernDir), assetsRoot, "csb");
        FSP_JoinPath(modernDir, sizeof(modernDir), modernDir, "modern");
        FSP_JoinPath(manifest, sizeof(manifest), modernDir,
                     "modern_asset_manifest.json");
        if (candidate_index + 1 == candidate_count || FSP_FileExists(manifest)) {
            snprintf(g_v22_manifest_path, sizeof(g_v22_manifest_path), "%s",
                     manifest);
            break;
        }
    }
    /* Keep the public catalog API and the runtime admission gate on the
     * same manifest root. A partial catalog must never expose V2.2. */
    csb_v22_famg_set_manifest_path(dataDir);
}

void csb_v22_set_manifest_file_path(const char* manifest_path) {
    if (!manifest_path || manifest_path[0] == '\0') {
        g_v22_manifest_path[0] = '\0';
        csb_v22_famg_set_manifest_file_path(NULL);
        return;
    }
    snprintf(g_v22_manifest_path, sizeof(g_v22_manifest_path), "%s",
             manifest_path);
    csb_v22_famg_set_manifest_file_path(g_v22_manifest_path);
}

const char* csb_v22_get_manifest_path(void) {
    return g_v22_manifest_path;
}

/* csb_v22_validate_manifest — validates the JSON manifest.
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
int csb_v22_validate_manifest(const char* manifest_path) {
    FILE* fp;
    char line[256];
    int found_categories = 0;
    int total_required = 0;  /* aggregate count reserved for future reporting */
    int categories_with_entries = 0;
    int current_category = -1;
    int entries_in_current_category = 0;
    int entry_has_all_fields = 1;
    (void)total_required;
    const size_t k_num_required_cats =
        sizeof(k_required_categories) / sizeof(k_required_categories[0]) - 1U;

    if (!manifest_path || manifest_path[0] == '\0') return -1;

    fp = fopen(manifest_path, "rb");
    if (!fp) return -1;

    /* Scan the file for categories and validate first entry per category */
    while (csb_v22_read_line(fp, line, sizeof(line))) {
        /* Detect category line: "category_name": [ or "category_name": { */
        int is_object = 0;
        for (size_t ci = 0U; k_required_categories[ci] != NULL; ++ci) {
            char cat_pattern[64];
            snprintf(cat_pattern, sizeof(cat_pattern), "\"%s\":", k_required_categories[ci]);
            /* Use strstr so we find the category anywhere in the line,
             * not just at position 0 (the entire manifest is one line). */
            if (strstr(line, cat_pattern) != NULL) {
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
            if (!csb_v22_read_line(fp, line, sizeof(line))) break;
        }

        /* Scan entries within this category until we hit the closing bracket/brace */
        int depth = 0;
        for (;;) {
            if (!csb_v22_read_line(fp, line, sizeof(line))) break;
            for (char* c = line; *c; ++c) {
                if (*c == '{') depth++;
                if (*c == '}') { depth--; if (depth < 0) depth = 0; }
                if (*c == '[') depth++;
                if (*c == ']') { depth--; if (depth < 0) { depth = 0; break; } }
            }
            if (depth <= 0) {
                /* At depth 0 or below, check if this line closes the array
                 * (']') or if we're negative (mismatched brackets). Do NOT
                 * exit on a '}' that belongs to an entry object nested in
                 * the array — those are consumed inside the array before
                 * the ']' line. */
                if (strchr(line, ']') != NULL) break;
                if (depth < 0) break;
            }
            /* This is an entry line — check required fields */
            if (strchr(line, '{') != NULL || strchr(line, '"') != NULL) {
                char id_val[128] = {0};
                char file_val[256] = {0};
                int width_val = 0, height_val = 0;
                int has_id = csb_v22_extract_string(line, "id", id_val, sizeof(id_val));
                int has_file = csb_v22_extract_string(line, "source_file", file_val, sizeof(file_val));
                int has_width = csb_v22_extract_int(line, "width", &width_val);
                int has_height = csb_v22_extract_int(line, "height", &height_val);
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

/* csb_v22_modern_assets_available — checks if the modern asset pack
 * is installed and at least critical shape categories are present.
 *
 * Checks:
 *   - ~/.firestaff/assets/csb/modern/modern_asset_manifest.json exists
 *   - All critical categories have at least one entry (wall_shapes,
 *     floor_shapes, creature_shapes — UI chrome is optional for playability)
 *
 * Returns: 1 if available, 0 if not installed or partial */
int csb_v22_modern_assets_available(void) {
    if (g_v22_manifest_path[0] == '\0') return 0;
    if (!csb_v22_file_exists(g_v22_manifest_path)) return 0;
    /* The legacy category-only scan below remains useful to validate an
     * imported catalog. Runtime admission, however, must cover every pair
     * the active per-cell router can emit and require real source files. */
    if (!csb_v22_famg_is_finished_real()) return 0;

    /* FAMG validates every active route pair and its resolved source file.
     * Do not downgrade a valid pretty-printed FSART through the legacy
     * fixed-line category scan below. */
    return 1;

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
    (void)current_cat;  /* set on category match; reserved for cross-line state */

    /* Each iteration reads one logical line (the entire manifest, since
     * it has no newlines). On the first read, the entire JSON content is
     * read as one line. The outer loop detects ALL three critical
     * categories on this single line and extracts their ids.
     *
     * We detect all categories present on THIS line BEFORE processing any
     * single category's id. This avoids the bug where finding one
     * category's id caused current_cat to be set to -1, preventing
     * detection of other categories that also have their ids on the
     * same line. */
    char id_val[64];
    while (csb_v22_read_line(fp, line, sizeof(line))) {
        /* First pass: detect any categories whose pattern appears on
         * this line. We detect ALL such categories before any id
         * extraction, so that all three categories are found even when
         * all ids are on the same physical line. */
        int cats_on_this_line[3] = {0, 0, 0};
        for (int ci = 0; critical_cats[ci] != NULL; ++ci) {
            char pattern[64];
            snprintf(pattern, sizeof(pattern), "\"%s\":", critical_cats[ci]);
            if (strstr(line, pattern) != NULL) {
                cats_on_this_line[ci] = 1;
            }
        }

        /* For each category detected on this line, try to extract the id.
         * The id may appear on this same line (if the entire entry is
         * inline) or on a subsequent line (multi-line entry format). */
        for (int ci = 0; ci < 3; ++ci) {
            if (!cats_on_this_line[ci]) continue;
            if (found_critical[ci]) continue; /* already found */
            current_cat = ci;

            /* Try to extract id on this same line first. */
            if (csb_v22_extract_string(line, "id", id_val, sizeof(id_val))) {
                found_critical[ci] = 1;
                current_cat = -1;
            } else {
                /* id not on this line — read subsequent lines for this
                 * category's entry. */
                int depth = 0;
                int in_entry = 0;
                while (csb_v22_read_line(fp, line, sizeof(line))) {
                    for (char* c = line; *c; ++c) {
                        if (*c == '{') { depth++; in_entry = 1; }
                        if (*c == '}') { depth--; }
                        if (*c == '[') depth++;
                        if (*c == ']') depth--;
                    }
                    if (depth < 0) break;
                    if (in_entry) {
                        if (csb_v22_extract_string(line, "id", id_val, sizeof(id_val))) {
                            found_critical[ci] = 1;
                            current_cat = -1;
                            in_entry = 0;
                            break;
                        }
                    }
                    if (depth == 0 && strchr(line, ']') != NULL) {
                        current_cat = -1;
                        break;
                    }
                }
            }
        }

        /* If all three categories were found on the first line, break
         * immediately — no need to read further lines. */
        if (found_critical[0] && found_critical[1] && found_critical[2]) {
            break;
        }
    }
    fclose(fp);

    return (found_critical[0] && found_critical[1] && found_critical[2]) ? 1 : 0;
}

/* csb_v22_set_installed — called at startup by M12_AssetStatus_Scan()
 * after detecting whether the modern asset pack is present. */
void csb_v22_set_installed(int installed) {
    g_v22_modern_assets_installed = installed ? 1 : 0;
}

/* csb_v22_get_installed — returns whether V2.2 modern assets are installed */
int csb_v22_get_installed(void) {
    return g_v22_modern_assets_installed;
}

/* csb_v22_set_epx_cache_warm / csb_v22_get_epx_cache_warm — EPX cache
 * state for V2.1 upscale. When UPSCALED mode is selected but the EPX
 * cache is cold, the system falls back to FILTERED (V2.0). */
void csb_v22_set_epx_cache_warm(int warm) {
    g_epx_cache_warm = warm ? 1 : 0;
}

int csb_v22_get_epx_cache_warm(void) {
    return g_epx_cache_warm;
}

/* csb_v22_best_available_shape_source — returns the best available
 * shape source for the current config and asset state.
 *
 * Fallback chain: MODERN → UPSCALED (V2.1) → FILTERED (V2.0) → ORIGINAL (V1)
 *
 * Logic:
 *   - If V2.2 (MODERN) selected AND every routed asset has passed the
 *     finished-art/source-provenance gate → V2_MODERN
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
CSB_V22_ShapeSource csb_v22_best_available_shape_source(int presentation_mode_index) {
    switch (presentation_mode_index) {
        case 3: /* M12_PRESENTATION_V22_MODERN */
            /* `set_installed()` is launcher state, not evidence. A caller
             * can set it before the manifest is scanned, so it must not
             * admit an unreviewed or generated pack into a draw route. */
            if (g_v22_modern_assets_installed &&
                csb_v22_modern_assets_available()) {
                return CSB_V22_SHAPE_SOURCE_V2_MODERN;
            }
            /* Fall through: no modern assets, fall back to V2.1 */
            fprintf(stderr, "[V2.2] V2.2 modern assets not available, "
                            "falling back to V2.1 upscaled\n");
            /* fall through */
        case 2: /* M12_PRESENTATION_V21_UPSCALED */
            if (g_epx_cache_warm) {
                return CSB_V22_SHAPE_SOURCE_V2_UPSCALED;
            }
            /* EPX cache cold — fall back to V2.0 filtered */
            fprintf(stderr, "[V2.2] EPX cache cold, falling back to V2.0 filtered\n");
            return CSB_V22_SHAPE_SOURCE_V2_FILTERED;

        case 1: /* M12_PRESENTATION_V20_FILTERED */
            return CSB_V22_SHAPE_SOURCE_V2_FILTERED;

        case 0:
        default:
            return CSB_V22_SHAPE_SOURCE_V1_ORIGINAL;
    }
}

/* csb_v22_get_shape_path — given a category and asset id from the
 * manifest, returns the full filesystem path to the asset file.
 * Returns 1 on success, 0 if not found or manifest not available. */
int csb_v22_get_shape_path(const char* category, const char* asset_id,
                            char* out_path, size_t out_path_size) {
    if (!category || !asset_id || !out_path || out_path_size == 0U) return 0;
    out_path[0] = '\0';
    if (!csb_v22_safe_path_component(category) ||
        !csb_v22_safe_path_component(asset_id)) return 0;
    if (g_v22_manifest_path[0] == '\0') return 0;

    FILE* fp = fopen(g_v22_manifest_path, "rb");
    if (!fp) return 0;

    int found_entry = 0;
    char line[256];
    int in_target_category = 0;
    int in_target_entry = 0;
    char resolved_id[64] = {0};
    char resolved_file[256] = {0};

    while (csb_v22_read_line(fp, line, sizeof(line))) {
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

        /* Extract before checking the closing brace. Artpack Studio also
         * writes compact entries whose id, source_file and `}` share one
         * line. */
        char val[256];
        if (csb_v22_extract_string(line, "id", val, sizeof(val))) {
            csb_v22_trim(resolved_id, val, sizeof(resolved_id));
        }
        if (csb_v22_extract_string(line, "source_file", val, sizeof(val))) {
            csb_v22_trim(resolved_file, val, sizeof(resolved_file));
        }
        if (strchr(line, '}') != NULL) {
            /* End of entry */
            in_target_entry = 0;
            if (resolved_id[0] != '\0' && strcmp(resolved_id, asset_id) == 0) {
                found_entry = 1;
                break;
            }
        }
    }

    fclose(fp);

    if (!found_entry || !csb_v22_safe_path_component(resolved_file)) return 0;

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
    return csb_v22_file_exists(out_path) ? 1 : 0;
}

/* csb_v22_shape_source_name — human-readable name for a shape source */
const char* csb_v22_shape_source_name(CSB_V22_ShapeSource src) {
    switch (src) {
        case CSB_V22_SHAPE_SOURCE_V1_ORIGINAL: return "V1_ORIGINAL";
        case CSB_V22_SHAPE_SOURCE_V2_FILTERED: return "V2_FILTERED";
        case CSB_V22_SHAPE_SOURCE_V2_UPSCALED: return "V2_UPSCALED";
        case CSB_V22_SHAPE_SOURCE_V2_MODERN:   return "V2_MODERN";
        default: return "UNKNOWN";
    }
}

/* csb_v22_source_evidence — citation string for source-lock tests. */
const char* csb_v22_source_evidence(void) {
    return "ReDMCSB DUNVIEW.C F0128 (CSB 9-square viewport); "
           "ReDMCSB LIGHT.C F0212 (CSB torchlight); "
           "ReDMCSB PANEL.C F0354 (CSB champion panel refresh); "
           "ReDMCSB COMMAND.C:108-113/254-291; "
           "CSBWin/Viewport.cpp:7290 (9-square grid mapping); "
           "CSBWin/Chaos.cpp:60-69 (DSA / chaos rune dispatch); "
           "include/dm1_v2_asset_pipeline_pc34.h (V2.2 modern-asset API contract, parallel module pattern).";
}
