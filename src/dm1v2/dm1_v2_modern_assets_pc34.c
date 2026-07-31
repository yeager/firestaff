/*
 * dm1_v2_modern_assets_pc34.c — V2.2 Modern Graphics Finished-Pack Pipeline
 *
 * V2.2 "Modern Graphics" is the third mode alongside V2.0 (Filtered) and
 * V2.1 (Upscaled). V2.2 is admitted only with a complete reviewed pack.
 * Otherwise rendering falls back to the source-backed V2.1/V2.0/V1 modes.
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
 * Missing modern assets fail closed for V2.2 and use the source-backed
 * presentation chain. No generated replacement surface is retained here.
 */

#include "dm1_v2_asset_pipeline_pc34.h"
#include "dm1_v22_finished_art_material_gate_pc34.h"
#include "dm1_v22_finished_pack_receipt_pc34.h"
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
static const char* const __attribute__((unused)) k_required_fields [] = {
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

static char* m11_v22_read_file(const char* path, size_t* out_len) {
    FILE* fp;
    long size;
    char* text;
    if (out_len) *out_len = 0U;
    if (!path || path[0] == '\0') return NULL;
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return NULL; }
    size = ftell(fp);
    if (size <= 0 || size > 1024L * 1024L) { fclose(fp); return NULL; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return NULL; }
    text = (char*)malloc((size_t)size + 1U);
    if (!text) { fclose(fp); return NULL; }
    if (fread(text, 1, (size_t)size, fp) != (size_t)size) {
        free(text);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    text[size] = '\0';
    if (out_len) *out_len = (size_t)size;
    return text;
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
static void __attribute__((unused)) m11_v22_skip_to_next_object (FILE* fp) {
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

/* The asset manifest is external input.  The field scanner below is enough
 * to classify a well-formed pack, but it must never make malformed JSON look
 * like a merely incomplete pack.  Keep a small complete JSON grammar here
 * rather than accepting arbitrary text with a few matching substrings. */
static void m11_v22_json_skip_ws(const char** cursor) {
    while (cursor && *cursor && isspace((unsigned char)**cursor)) {
        ++*cursor;
    }
}

static int m11_v22_json_parse_value(const char** cursor, int depth);

static int m11_v22_json_parse_string(const char** cursor) {
    const char* p;
    if (!cursor || !(p = *cursor) || *p != '"') return 0;
    ++p;
    while (*p && *p != '"') {
        unsigned char ch = (unsigned char)*p++;
        if (ch < 0x20u) return 0;
        if (ch == '\\') {
            if (*p == '\0') return 0;
            if (*p == 'u') {
                int i;
                ++p;
                for (i = 0; i < 4; ++i, ++p) {
                    if (!isxdigit((unsigned char)*p)) return 0;
                }
            } else if (!strchr("\\\"/bfnrt", *p)) {
                return 0;
            } else {
                ++p;
            }
        }
    }
    if (*p != '"') return 0;
    *cursor = p + 1;
    return 1;
}

static int m11_v22_json_parse_object(const char** cursor, int depth) {
    const char* p;
    if (!cursor || !(p = *cursor) || *p != '{' || depth > 64) return 0;
    ++p;
    m11_v22_json_skip_ws(&p);
    if (*p == '}') {
        *cursor = p + 1;
        return 1;
    }
    for (;;) {
        if (!m11_v22_json_parse_string(&p)) return 0;
        m11_v22_json_skip_ws(&p);
        if (*p++ != ':') return 0;
        m11_v22_json_skip_ws(&p);
        if (!m11_v22_json_parse_value(&p, depth + 1)) return 0;
        m11_v22_json_skip_ws(&p);
        if (*p == '}') {
            *cursor = p + 1;
            return 1;
        }
        if (*p++ != ',') return 0;
        m11_v22_json_skip_ws(&p);
    }
}

static int m11_v22_json_parse_array(const char** cursor, int depth) {
    const char* p;
    if (!cursor || !(p = *cursor) || *p != '[' || depth > 64) return 0;
    ++p;
    m11_v22_json_skip_ws(&p);
    if (*p == ']') {
        *cursor = p + 1;
        return 1;
    }
    for (;;) {
        if (!m11_v22_json_parse_value(&p, depth + 1)) return 0;
        m11_v22_json_skip_ws(&p);
        if (*p == ']') {
            *cursor = p + 1;
            return 1;
        }
        if (*p++ != ',') return 0;
        m11_v22_json_skip_ws(&p);
    }
}

static int m11_v22_json_parse_value(const char** cursor, int depth) {
    const char* p;
    char* end;
    if (!cursor || !(p = *cursor) || depth > 64) return 0;
    m11_v22_json_skip_ws(&p);
    if (*p == '{') {
        if (!m11_v22_json_parse_object(&p, depth)) return 0;
        *cursor = p;
        return 1;
    }
    if (*p == '[') {
        if (!m11_v22_json_parse_array(&p, depth)) return 0;
        *cursor = p;
        return 1;
    }
    if (*p == '"') {
        if (!m11_v22_json_parse_string(&p)) return 0;
    } else if (strncmp(p, "true", 4) == 0 || strncmp(p, "null", 4) == 0) {
        p += 4;
    } else if (strncmp(p, "false", 5) == 0) {
        p += 5;
    } else {
        (void)strtod(p, &end);
        if (end == p) return 0;
        p = end;
    }
    *cursor = p;
    return 1;
}

static int m11_v22_json_document_is_object(const char* text) {
    const char* p = text;
    if (!p) return 0;
    m11_v22_json_skip_ws(&p);
    if (*p != '{' || !m11_v22_json_parse_object(&p, 0)) return 0;
    m11_v22_json_skip_ws(&p);
    return *p == '\0';
}

/* ── Public API ──────────────────────────────────────────────────── */

/* m11_v22_set_manifest_path — set the path to the modern asset manifest.
 * Called by M12_AssetStatus_Scan() after resolving the data directory.
 * The manifest path is derived as: ~/.firestaff/assets/dm1/modern/
 * modern_asset_manifest.json (per docs/v2_2_asset_manifest.md and
 * docs/v2_2_asset_provenance_schema.md).
 *
 * dataDir is the game data directory path (e.g. ~/.firestaff/data/dm1).
 * We walk up two levels to reach ~/.firestaff, then append
 * assets/dm1/modern/modern_asset_manifest.json.
 *
 * CRITICAL: We must use FSP_ParentDir to strip the last path segment,
 * NOT FSP_JoinPath with "..". The ".." join preserves the directory
 * name in the path string (e.g. "dm1/../" doesn't simplify to just the
 * parent), so using it would produce a manifest path that contains the
 * dataDir name (e.g. ".../dm1/../assets/...") which never resolves to
 * the actual file. FSP_ParentDir strips the last segment cleanly.
 *
 * Example: dataDir="/home/user/.firestaff/data/dm1"
 *   ParentDir → "/home/user/.firestaff/data"
 *   ParentDir → "/home/user/.firestaff"
 *   assets/dm1/modern → correct modern assets directory.
 *
 * Source: FSP_ParentDir finds the last separator and truncates there. */
void m11_v22_set_manifest_path(const char* dataDir) {
    /* The V2.2 admission path has two sibling checks: the manifest/material
     * gate and the hash-bound review receipt. Keep their roots in lockstep
     * whenever the host selects its DM1 data directory. */
    dm1_v22_fpr_set_receipt_path(dataDir);
    if (!dataDir || dataDir[0] == '\0') {
        g_v22_manifest_path[0] = '\0';
        return;
    }
    /* Build: ~/.firestaff/assets/dm1/modern/modern_asset_manifest.json
     * Walk up two levels from dataDir (e.g. data/dm1 -> data -> ~/.firestaff)
     * then append assets/dm1/modern/. */
    char parent1[FSP_PATH_MAX];
    char parent2[FSP_PATH_MAX];
    char assetsRoot[FSP_PATH_MAX];
    char modernDir[FSP_PATH_MAX];
    if (!FSP_ParentDir(parent1, sizeof(parent1), dataDir) ||
        !FSP_ParentDir(parent2, sizeof(parent2), parent1)) {
        /* Fallback: try treating dataDir as already being ~/.firestaff
         * (single-component dataDir from a custom setup). */
        FSP_JoinPath(assetsRoot, sizeof(assetsRoot), dataDir, "assets");
    } else {
        FSP_JoinPath(assetsRoot, sizeof(assetsRoot), parent2, "assets");
    }
    FSP_JoinPath(modernDir, sizeof(modernDir), assetsRoot, "dm1");
    FSP_JoinPath(modernDir, sizeof(modernDir), modernDir, "modern");
    FSP_JoinPath(g_v22_manifest_path, sizeof(g_v22_manifest_path),
                 modernDir, "modern_asset_manifest.json");
}

const char* m11_v22_get_modern_asset_root(void) {
    static char modern_dir[FSP_PATH_MAX];
    char* last_slash;
    if (g_v22_manifest_path[0] == '\0') return "";
    snprintf(modern_dir, sizeof(modern_dir), "%s", g_v22_manifest_path);
    last_slash = strrchr(modern_dir, '/');
    if (!last_slash) return "";
    *last_slash = '\0';
    return modern_dir;
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
    char* text = NULL;
    long file_size;
    int result = -1;
    size_t ci;

    if (!manifest_path || manifest_path[0] == '\0') return -1;

    fp = fopen(manifest_path, "rb");
    if (!fp) return -1;
    if (fseek(fp, 0L, SEEK_END) != 0 ||
        (file_size = ftell(fp)) <= 0L || file_size > 1024L * 1024L ||
        fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }
    text = (char*)malloc((size_t)file_size + 1U);
    if (!text || fread(text, 1U, (size_t)file_size, fp) != (size_t)file_size) {
        free(text);
        fclose(fp);
        return -1;
    }
    fclose(fp);
    text[file_size] = '\0';

    if (!m11_v22_json_document_is_object(text)) {
        free(text);
        return -1;
    }
    {
        char version[64] = {0};
        char pack_id[128] = {0};
        if (!m11_v22_extract_string(text, "manifestVersion", version,
                                    sizeof(version)) ||
            !m11_v22_extract_string(text, "packId", pack_id,
                                    sizeof(pack_id)) ||
            version[0] == '\0' || pack_id[0] == '\0') {
            free(text);
            return -1;
        }
    }

    /* Manifests are deliberately pretty-printed by Art Studio. Validate the
     * first object in each required array as one object, not one line. */
    result = 1;
    for (ci = 0U; k_required_categories[ci] != NULL; ++ci) {
        char category_pattern[64];
        const char* category;
        const char* object_start;
        const char* object_end;
        char object[1024];
        char id_val[128] = {0};
        char file_val[256] = {0};
        int width_val = 0;
        int height_val = 0;
        size_t object_len;

        snprintf(category_pattern, sizeof(category_pattern), "\"%s\":",
                 k_required_categories[ci]);
        category = strstr(text, category_pattern);
        object_start = category ? strchr(category, '{') : NULL;
        object_end = object_start ? strchr(object_start, '}') : NULL;
        if (!category || !object_start || !object_end) {
            result = 0;
            break;
        }
        object_len = (size_t)(object_end - object_start + 1);
        if (object_len >= sizeof(object)) {
            result = 0;
            break;
        }
        memcpy(object, object_start, object_len);
        object[object_len] = '\0';
        if (!m11_v22_extract_string(object, "id", id_val, sizeof(id_val)) ||
            !m11_v22_extract_string(object, "source_file", file_val,
                                    sizeof(file_val)) ||
            !m11_v22_extract_int(object, "width", &width_val) ||
            !m11_v22_extract_int(object, "height", &height_val) ||
            id_val[0] == '\0' || file_val[0] == '\0' ||
            width_val <= 0 || height_val <= 0) {
            result = 0;
            break;
        }
    }
    free(text);
    return result;
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
    /* Critical-category presence is not enough for DM1 V2.2 runtime.
     * The modern path may draw only a complete non-placeholder pack
     * whose finish receipt matches the current manifest. */
    return dm1_v22_famg_is_finished_real() && dm1_v22_fpr_is_promoted();
}

static int __attribute__((unused)) m11_v22_modern_assets_manifest_has_critical_categories(void) {
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
    while (m11_v22_read_line(fp, line, sizeof(line))) {
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
            if (m11_v22_extract_string(line, "id", id_val, sizeof(id_val))) {
                found_critical[ci] = 1;
                current_cat = -1;
            } else {
                /* id not on this line — read subsequent lines for this
                 * category's entry. */
                int depth = 0;
                int in_entry = 0;
                while (m11_v22_read_line(fp, line, sizeof(line))) {
                    for (char* c = line; *c; ++c) {
                        if (*c == '{') { depth++; in_entry = 1; }
                        if (*c == '}') { depth--; }
                        if (*c == '[') depth++;
                        if (*c == ']') depth--;
                    }
                    if (depth < 0) break;
                    if (in_entry) {
                        if (m11_v22_extract_string(line, "id", id_val, sizeof(id_val))) {
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
            if (g_v22_modern_assets_installed &&
                m11_v22_modern_assets_available()) {
                return DM1_V22_SHAPE_SOURCE_V2_MODERN;
            }
            fprintf(stderr, "[V2.2] reviewed V2.2 modern pack not available, "
                            "falling back to source-backed V2.1 upscaled\n");
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

/* m11_v22_get_shape_path — given a category and asset id from the
 * manifest, returns the full filesystem path to the asset file.
 * Returns 1 on success, 0 if not found or manifest not available. */
int m11_v22_get_shape_path(const char* category, const char* asset_id,
                            char* out_path, size_t out_path_size) {
    if (!category || !asset_id || !out_path || out_path_size == 0U) return 0;
    out_path[0] = '\0';
    if (g_v22_manifest_path[0] == '\0') return 0;

    size_t manifest_len = 0U;
    char* manifest = m11_v22_read_file(g_v22_manifest_path, &manifest_len);
    if (!manifest) return 0;

    char cat_pattern[128];
    snprintf(cat_pattern, sizeof(cat_pattern), "\"%s\"", category);
    char* cat = strstr(manifest, cat_pattern);
    if (!cat) { free(manifest); return 0; }
    char* array = strchr(cat, '[');
    if (!array) { free(manifest); return 0; }
    char* array_end = strchr(array, ']');
    if (!array_end) { free(manifest); return 0; }

    char id_pattern[160];
    snprintf(id_pattern, sizeof(id_pattern), "\"id\":\"%s\"", asset_id);
    char* entry = strstr(array, id_pattern);
    if (!entry || entry > array_end) {
        snprintf(id_pattern, sizeof(id_pattern), "\"id\": \"%s\"", asset_id);
        entry = strstr(array, id_pattern);
    }
    if (!entry || entry > array_end) { free(manifest); return 0; }

    char* obj_start = entry;
    while (obj_start > array && *obj_start != '{') --obj_start;
    char* obj_end = strchr(entry, '}');
    if (*obj_start != '{' || !obj_end || obj_end > array_end) {
        free(manifest);
        return 0;
    }

    char saved = obj_end[1];
    obj_end[1] = '\0';
    char resolved_file[256] = {0};
    int has_file = m11_v22_extract_string(obj_start, "source_file",
                                          resolved_file, sizeof(resolved_file));
    obj_end[1] = saved;
    if (!has_file || resolved_file[0] == '\0') {
        free(manifest);
        return 0;
    }

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
    int exists = m11_v22_file_exists(out_path) ? 1 : 0;
    if (!exists) out_path[0] = '\0';
    free(manifest);
    return exists;
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
