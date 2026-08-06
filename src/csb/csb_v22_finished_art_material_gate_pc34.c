/*
 * csb_v22_finished_art_material_gate_pc34.c
 *
 * CSB V2.2 finished-art / material screenshot pixel gate.
 *
 * Companion to include/csb_v22_finished_art_material_gate_pc34.h.
 *
 * This module is the CI-runnable distinction between:
 *   - SYNTHETIC_PLACEHOLDER (procedural / "placeholder" generator,
 *     the current honest runtime default)
 *   - FINISHED_REAL        (the documented PC3.4 GRAPHICS.DAT export with
 *     generator == "original_csb_pc34_graphics_dat")
 *
 * It runs in CI without requiring real PBR art on disk by:
 *   1. Reading the existing modern_asset_manifest.json (when present)
 *      and classifying every tracked material slot.
 *   2. Exercising the gate state machine via synthetic manifest
 *      fixtures authored by the test/probe under a probe-only HOME.
 *
 * The manifest schema reuses the modern_asset_manifest.json format
 * parsed by src/csb/csb_v22_modern_assets_pc34.c, with the
 * addition of an optional `generator` field. Only the exact generator emitted
 * by scripts/build_csb_v22_source_fsart.py can promote a slot. A label such
 * as "pbr_hero", "ai_upscale" or "reviewed" is not evidence that pixels
 * came from the authenticated original archive.
 *
 * Source-lock:
 *   - ReDMCSB DUNVIEW.C F0128 (CSB viewport routing)
 *   - ReDMCSB PANEL.C F0354    (CSB champion panel refresh)
 *   - ReDMCSB COMMAND.C:108-113/254-291 (command dispatch)
 *   - CSBWin/Viewport.cpp:7290 (CSB 9-square viewport layout)
 *   - CSBWin/Chaos.cpp:60-69   (DSA / chaos rune dispatch)
 *   - include/csb_v22_inplace_draw_pc34.h (cell -> variant -> asset_id)
 *   - include/csb_v22_modern_assets_pc34.h (manifest path resolution)
 *   - sibling dm1_v22 / dm2_v22 FAMG modules (placeholder-vs-real pattern)
 *
 * Honest boundary: this gate tracks manifest classification only.
 * It does NOT claim finished PBR art has been reviewed or shipped.
 * FINISHED_REAL is reachable only when an operator has dropped a
 * non-placeholder manifest with source_file paths that resolve on
 * disk; until then the gate stays in SYNTHETIC_PLACEHOLDER, which
 * matches the honest current default.
 */

#include "csb_v22_finished_art_material_gate_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Slot table (stable, ordered) ────────────────────────────────
 *
 * Each slot pairs:
 *   - id           : manifest entry id. These are deliberately the
 *                    asset_id strings dispatched by
 *                    csb_v22_inplace_get_cell_asset_id() in
 *                    src/csb/csb_v22_inplace_draw_pc34.c so this
 *                    gate tracks the same surface as the runtime
 *                    per-cell modern-art swap.
 *   - category     : modern_asset_manifest.json top-level category
 *                    (drives source_file path resolution).
 *   - asset_id_v22 : the synthetic / csb_v22_inplace asset_id used
 *                    by the in-place cache when no real hero pack is
 *                    installed. Reported in the slot info record for
 *                    honest gap tracking.
 *
 * Reordering / inserting slots must keep CSB_V22_FAMG_MATERIAL_COUNT
 * in sync and add an entry here. */
typedef struct {
    CSB_V22_FamgSlot slot;
    const char*      id;            /* modern_asset_manifest.json id */
    const char*      category;      /* wall_shapes / creature_shapes / ... */
    const char*      asset_id_v22;  /* csb_v22_inplace asset_id when
                                       procedural fallback is used */
} CSB_V22_FamgSlotDesc;

static const CSB_V22_FamgSlotDesc k_slot_table[CSB_V22_FAMG_MATERIAL_COUNT] = {
#define FAMG_SLOT(symbol, id, category) { symbol, id, category, id }
    FAMG_SLOT(CSB_V22_FAMG_WALL_DUNGEON_D0, "wall_dungeon_d0_01", "wall_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_WALL_DUNGEON_D1, "wall_dungeon_d1_01", "wall_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_WALL_DUNGEON_D2, "wall_dungeon_d2_01", "wall_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_DOOR_D0, "door_d0_01", "door_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_DOOR_D1, "door_d1_01", "door_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_DOOR_D2, "door_d2_01", "door_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_PLAIN_D0, "floor_plain_d0_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_PLAIN_D1, "floor_plain_d1_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_PLAIN_D2, "floor_plain_d2_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_CRACKED_D0, "floor_cracked_d0_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_CRACKED_D1, "floor_cracked_d1_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_CRACKED_D2, "floor_cracked_d2_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_MOSSY_D0, "floor_mossy_d0_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_MOSSY_D1, "floor_mossy_d1_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_MOSSY_D2, "floor_mossy_d2_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_PIT, "floor_pit_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_STAIRS_UP, "floor_stairs_up_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_FLOOR_STAIRS_DOWN, "floor_stairs_down_01", "floor_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_CEILING, "ceiling_01", "wall_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_CREATURE_DEMON_D0, "creature_demon_d0_01", "creature_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_CREATURE_DEMON_D1, "creature_demon_d1_01", "creature_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_CREATURE_DEMON_D2, "creature_demon_d2_01", "creature_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_PRISON_DOOR, "prison_door_01", "wall_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_LORD_ORDER, "lord_order_01", "wall_shapes"),
    FAMG_SLOT(CSB_V22_FAMG_CHAOS_RUNE_0, "chaos_rune_0_01", "chaos_runes"),
    FAMG_SLOT(CSB_V22_FAMG_CHAOS_RUNE_1, "chaos_rune_1_01", "chaos_runes"),
    FAMG_SLOT(CSB_V22_FAMG_CHAOS_RUNE_2, "chaos_rune_2_01", "chaos_runes"),
    FAMG_SLOT(CSB_V22_FAMG_CHAOS_RUNE_3, "chaos_rune_3_01", "chaos_runes"),
    FAMG_SLOT(CSB_V22_FAMG_DSA_SCROLL, "dsa_scroll_01", "dsa_scrolls")
#undef FAMG_SLOT
};

/* ── Module state ──────────────────────────────────────────────── */
static char               g_manifest_path[FSP_PATH_MAX] = {0};
static int                g_installed = 0;     /* last gate: 1 if PARTIAL/FINISHED_REAL */
static CSB_V22_FamgGate   g_last_gate = CSB_V22_FAMG_GATE_NOT_PROBED;

#define CSB_V22_FAMG_SOURCE_EXPORT_GENERATOR "original_csb_pc34_graphics_dat"

/* ── Trimming / JSON helpers ───────────────────────────────────── */
static void csb_v22_famg_trim(char* dst, const char* src, size_t dstSize) {
    if (!dst || dstSize == 0U) return;
    const char* start = src ? src : "";
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') ++start;
    size_t len = strlen(start);
    const char* end = start + len;
    while (len > 0U && (end[-1] == ' ' || end[-1] == '\t' ||
                        end[-1] == '\r' || end[-1] == '\n')) {
        --end; --len;
    }
    if (len >= dstSize) len = dstSize - 1U;
    memcpy(dst, start, len);
    dst[len] = '\0';
}

static int csb_v22_famg_file_exists(const char* path) {
    if (!path || path[0] == '\0') return 0;
    FILE* fp = fopen(path, "rb");
    if (fp) { fclose(fp); return 1; }
    return 0;
}

static int csb_v22_famg_file_starts_with_object(const char* path) {
    FILE* fp;
    int ch;
    if (!path || path[0] == '\0') return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    do {
        ch = fgetc(fp);
    } while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
    fclose(fp);
    return ch == '{';
}

/* Raw slot record populated by the manifest scan. */
typedef struct {
    char id[64];
    char generator[64];
    char source_file[256];
    int  width;
    int  height;
    int  has_id;
    int  has_generator;
    int  has_source_file;
    int  has_width;
    int  has_height;
} CSB_V22_FamgSlotRaw;

static void csb_v22_famg_slot_raw_init(CSB_V22_FamgSlotRaw* r) {
    memset(r, 0, sizeof(*r));
}

/* ── Entry-content accumulator ──────────────────────────────────
 *
 * The manifest can be single-line or pretty-printed. We accumulate
 * each entry's raw JSON text as we walk braces/brackets, then run
 * field extractors against the buffer when the entry closes. This
 * handles all three layouts the dm1/dm2 sibling modules support:
 *   1. Single-line JSON (entry spans one line).
 *   2. Fields on the same line as the closing `}`.
 *   3. Pretty-printed multi-line entries whose `{` and `}` are on
 *      separate lines from the field lines. */
static char g_entry_buf[16384];
static int  g_entry_buf_len = 0;

static void csb_v22_famg_buf_reset(void) {
    g_entry_buf_len = 0;
    if (g_entry_buf_len < (int)sizeof(g_entry_buf)) {
        g_entry_buf[g_entry_buf_len] = '\0';
    }
}

static void csb_v22_famg_buf_append_char(char c) {
    if (g_entry_buf_len + 1 < (int)sizeof(g_entry_buf)) {
        g_entry_buf[g_entry_buf_len++] = c;
        g_entry_buf[g_entry_buf_len] = '\0';
    }
}

static int csb_v22_famg_extract_string(const char* line, const char* key,
                                       char* out, size_t outSize) {
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

static int csb_v22_famg_extract_int(const char* line, const char* key,
                                    int* out) {
    if (!line || !key || !out) return 0;
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char* p = strstr(line, pattern);
    if (!p) return 0;
    p += strlen(pattern);
    while (*p == ' ' || *p == ':' || *p == '\t') ++p;
    char* end = NULL;
    long val = strtol(p, &end, 10);
    if (end == p) return 0;
    if (val < 0) val = 0;
    *out = (int)val;
    return 1;
}

static void csb_v22_famg_extract_fields_from_buf(CSB_V22_FamgSlotRaw* out) {
    char val[256];
    if (csb_v22_famg_extract_string(g_entry_buf, "id", val, sizeof(val))) {
        csb_v22_famg_trim(out->id, val, sizeof(out->id));
        out->has_id = 1;
    }
    if (csb_v22_famg_extract_string(g_entry_buf, "generator", val, sizeof(val))) {
        csb_v22_famg_trim(out->generator, val, sizeof(out->generator));
        out->has_generator = 1;
    }
    if (csb_v22_famg_extract_string(g_entry_buf, "source_file", val, sizeof(val))) {
        csb_v22_famg_trim(out->source_file, val, sizeof(out->source_file));
        out->has_source_file = 1;
    }
    int w = 0, h = 0;
    if (csb_v22_famg_extract_int(g_entry_buf, "width", &w)) {
        out->width = w;
        out->has_width = 1;
    }
    if (csb_v22_famg_extract_int(g_entry_buf, "height", &h)) {
        out->height = h;
        out->has_height = 1;
    }
}

/* ── Path resolution ───────────────────────────────────────────── */
void csb_v22_famg_set_manifest_path(const char* dataDir) {
    char resolved_data_dir[FSP_PATH_MAX];
    const char *candidates[2];
    int candidate_count = 1;
    int candidate_index;
    if (!dataDir || dataDir[0] == '\0') {
        g_manifest_path[0] = '\0';
        return;
    }
    candidates[0] = dataDir;
    if (FSP_ResolvePhysicalPath(resolved_data_dir,
                                sizeof(resolved_data_dir), dataDir) &&
        strcmp(resolved_data_dir, dataDir) != 0) {
        candidates[candidate_count++] = resolved_data_dir;
    }
    /* Prefer the logical ~/.firestaff/data/csb ancestry.  It may be a
     * symlink to a game-data volume while the user's reviewed .fsart remains
     * under ~/.firestaff/assets.  Fall back to the physical volume only when
     * the logical sibling manifest is absent. */
    for (candidate_index = 0; candidate_index < candidate_count;
         ++candidate_index) {
        char parent1[FSP_PATH_MAX];
        char parent2[FSP_PATH_MAX];
        char assets_root[FSP_PATH_MAX];
        char csb_modern_dir[FSP_PATH_MAX];
        char manifest[FSP_PATH_MAX];
        const char *asset_data_dir = candidates[candidate_index];
        if (!FSP_ParentDir(parent1, sizeof(parent1), asset_data_dir) ||
            !FSP_ParentDir(parent2, sizeof(parent2), parent1)) {
            FSP_JoinPath(assets_root, sizeof(assets_root), asset_data_dir,
                         "assets");
        } else {
            FSP_JoinPath(assets_root, sizeof(assets_root), parent2, "assets");
        }
        FSP_JoinPath(csb_modern_dir, sizeof(csb_modern_dir), assets_root,
                     "csb");
        FSP_JoinPath(csb_modern_dir, sizeof(csb_modern_dir), csb_modern_dir,
                     "modern");
        FSP_JoinPath(manifest, sizeof(manifest), csb_modern_dir,
                     "modern_asset_manifest.json");
        if (candidate_index + 1 == candidate_count || FSP_FileExists(manifest)) {
            snprintf(g_manifest_path, sizeof(g_manifest_path), "%s", manifest);
            return;
        }
    }
}

void csb_v22_famg_set_manifest_file_path(const char* manifest_path) {
    if (!manifest_path || manifest_path[0] == '\0') {
        g_manifest_path[0] = '\0';
        return;
    }
    snprintf(g_manifest_path, sizeof(g_manifest_path), "%s", manifest_path);
}

const char* csb_v22_famg_get_manifest_path(void) {
    return g_manifest_path;
}

/* ── Internal manifest scan ────────────────────────────────────── */
static int csb_v22_famg_find_slot_in_manifest(const char* manifest_path,
                                              const char* slot_id,
                                              CSB_V22_FamgSlotRaw* out) {
    if (!manifest_path || manifest_path[0] == '\0' || !slot_id || !out) return 0;
    csb_v22_famg_slot_raw_init(out);

    FILE* fp = fopen(manifest_path, "rb");
    if (!fp) return 0;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    long size = ftell(fp);
    if (size < 0 || size > 1024L * 1024L) {
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    char* text = (char*)malloc((size_t)size + 1U);
    if (!text) {
        fclose(fp);
        return 0;
    }
    if (fread(text, 1, (size_t)size, fp) != (size_t)size) {
        free(text);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    text[size] = '\0';

    /* Locate the exact id first, then isolate its immediate JSON object.
     * The old brace walker began with the outer manifest object and its
     * field extractor always saw the first entry.  A finished pretty-printed
     * 29-slot CSB pack was consequently classified as one real asset plus
     * 28 partial assets.  Source_file names are required to be local and the
     * manifest grammar does not permit an id string inside another entry, so
     * the nearest enclosing braces are the selected asset object. */
    {
        const char* match;
        const char* begin;
        const char* end;
        size_t slot_len = strlen(slot_id);
        match = text;
        for (;;) {
            const char* value;
            match = strstr(match, "\"id\"");
            if (!match) break;
            value = match + 4;
            while (*value == ' ' || *value == '\t' || *value == '\r' ||
                   *value == '\n') ++value;
            if (*value != ':') {
                match += 4;
                continue;
            }
            ++value;
            while (*value == ' ' || *value == '\t' || *value == '\r' ||
                   *value == '\n') ++value;
            if (*value == '"' &&
                strncmp(value + 1, slot_id, slot_len) == 0 &&
                value[slot_len + 1] == '"') {
                break;
            }
            match += 4;
        }
        while (match) {
            begin = match;
            while (begin > text && *begin != '{') --begin;
            end = match;
            while (*end && *end != '}') ++end;
            if (*begin != '{' || *end != '}') break;
            csb_v22_famg_buf_reset();
            {
                const char* cursor = begin;
                while (cursor <= end) {
                    csb_v22_famg_buf_append_char(*cursor);
                    ++cursor;
                }
            }
            {
                CSB_V22_FamgSlotRaw raw;
                csb_v22_famg_slot_raw_init(&raw);
                csb_v22_famg_extract_fields_from_buf(&raw);
                if (raw.has_id && strcmp(raw.id, slot_id) == 0 &&
                    raw.has_source_file) {
                    *out = raw;
                    free(text);
                    return 1;
                }
            }
            /* Provenance metadata may repeat an asset id. Only the actual
             * manifest entry has source_file; continue past metadata. */
            match = end + 1;
            for (;;) {
                const char* value;
                match = strstr(match, "\"id\"");
                if (!match) break;
                value = match + 4;
                while (*value == ' ' || *value == '\t' || *value == '\r' ||
                       *value == '\n') ++value;
                if (*value == ':') {
                    ++value;
                    while (*value == ' ' || *value == '\t' || *value == '\r' ||
                           *value == '\n') ++value;
                    if (*value == '"' &&
                        strncmp(value + 1, slot_id, slot_len) == 0 &&
                        value[slot_len + 1] == '"') break;
                }
                match += 4;
            }
        }
    }

    free(text);
    return 0;
}

/* A source-derived bitmap alone is not enough to replace a V1 F0128 draw.
 * The artpack records its source-geometry result separately in
 * routeProvenance.  Keep the finished-art gate tied to that explicit result:
 * an "unbound" route may be useful to Artpack Studio, but it must remain on
 * the V1 path at runtime. */
static int csb_v22_famg_route_provenance_admits_slot(
    const char* manifest_path, const char* category, const char* slot_id) {
    FILE* fp;
    char* text;
    long size;
    const char* route;
    const char* match;
    const size_t slot_len = slot_id ? strlen(slot_id) : 0U;

    if (!manifest_path || !category || !slot_id || slot_len == 0U) return 0;
    fp = fopen(manifest_path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0 || (size = ftell(fp)) < 0 ||
        size > 1024L * 1024L || fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    text = (char*)malloc((size_t)size + 1U);
    if (!text || fread(text, 1, (size_t)size, fp) != (size_t)size) {
        free(text);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    text[size] = '\0';
    route = strstr(text, "\"routeProvenance\"");
    if (!route) {
        free(text);
        return 0;
    }

    match = route;
    while ((match = strstr(match, "\"id\"")) != NULL) {
        const char* value = match + 4;
        const char* begin;
        const char* end;
        char entry_category[64];
        char projection_status[64];

        while (*value == ' ' || *value == '\t' || *value == '\r' ||
               *value == '\n') ++value;
        if (*value != ':') {
            match += 4;
            continue;
        }
        ++value;
        while (*value == ' ' || *value == '\t' || *value == '\r' ||
               *value == '\n') ++value;
        if (*value != '"' || strncmp(value + 1, slot_id, slot_len) != 0 ||
            value[slot_len + 1] != '"') {
            match += 4;
            continue;
        }
        begin = match;
        while (begin > route && *begin != '{') --begin;
        end = match;
        while (*end && *end != '}') ++end;
        if (*begin != '{' || *end != '}') break;
        csb_v22_famg_buf_reset();
        while (begin <= end) csb_v22_famg_buf_append_char(*begin++);
        entry_category[0] = '\0';
        projection_status[0] = '\0';
        if (csb_v22_famg_extract_string(g_entry_buf, "category",
                                         entry_category,
                                         sizeof(entry_category)) &&
            csb_v22_famg_extract_string(g_entry_buf,
                                         "f0128ProjectionStatus",
                                         projection_status,
                                         sizeof(projection_status)) &&
            strcmp(entry_category, category) == 0 &&
            strncmp(projection_status, "admitted_", 9) == 0) {
            free(text);
            return 1;
        }
        match = end + 1;
    }
    free(text);
    return 0;
}

/* Resolve a manifest source_file (relative) against the manifest dir.
 * Returns 1 if the resolved path exists on disk, 0 otherwise.
 *
 * For CSB V2.2 materials the source_file is expected to live under
 * <modern-dir>/<category>/<source_file>, e.g.:
 *   ~/.firestaff/assets/csb/modern/wall_shapes/wall_dungeon_d0_01.png
 */
static int csb_v22_famg_resolve_source_file(const char* manifest_path,
                                            const char* category,
                                            const char* source_file,
                                            char* out, size_t outSize) {
    if (!manifest_path || !category || !source_file ||
        source_file[0] == '\0' || !out || outSize == 0U) {
        if (out && outSize > 0U) out[0] = '\0';
        return 0;
    }

    char manifest_dir[FSP_PATH_MAX];
    const char* last_slash = strrchr(manifest_path, '/');
    if (last_slash) {
        size_t dir_len = (size_t)(last_slash - manifest_path);
        if (dir_len >= sizeof(manifest_dir)) dir_len = sizeof(manifest_dir) - 1U;
        memcpy(manifest_dir, manifest_path, dir_len);
        manifest_dir[dir_len] = '\0';
    } else {
        manifest_dir[0] = '.';
        manifest_dir[1] = '\0';
    }

    char joined[FSP_PATH_MAX];
    FSP_JoinPath(joined, sizeof(joined), manifest_dir, category);
    FSP_JoinPath(joined, sizeof(joined), joined, source_file);
    if (strlen(joined) >= outSize) {
        out[0] = '\0';
        return 0;
    }
    snprintf(out, outSize, "%s", joined);
    return csb_v22_famg_file_exists(joined);
}

/* ── Validation ────────────────────────────────────────────────── */
int csb_v22_famg_validate_manifest(const char* manifest_path) {
    const char* p = manifest_path ? manifest_path : g_manifest_path;
    if (!p || p[0] == '\0') return -1;
    if (!csb_v22_famg_file_exists(p)) return -1;
    if (!csb_v22_famg_file_starts_with_object(p)) return -1;

    /* Probe the first required slot. If it can be located AND has
     * all required fields, the manifest is considered valid; a partial
     * manifest (some required fields missing) returns 0. */
    const char* first_id = k_slot_table[0].id;
    CSB_V22_FamgSlotRaw raw;
    if (!csb_v22_famg_find_slot_in_manifest(p, first_id, &raw)) {
        return 0; /* manifest readable but no slot declared */
    }
    int complete = raw.has_id && raw.has_generator &&
                   raw.has_source_file && raw.has_width &&
                   raw.has_height && raw.width > 0 && raw.height > 0;
    return complete ? 1 : 0;
}

/* A partial source pack is useful only where one concrete original raster has
 * a concrete renderer receipt.  Do not require unrelated wall/floor/creature
 * slots to be bound before F0111 can use an independently admitted door.
 * ReDMCSB DUNVIEW.C F0111 supplies the source door record; F0128 retains the
 * caller's checked clip and composition order. */
int csb_v22_famg_admits_material(const char* category, const char* asset_id) {
    CSB_V22_FamgSlotRaw raw;
    char resolved_path[FSP_PATH_MAX];

    if (!category || !asset_id || category[0] == '\0' || asset_id[0] == '\0') {
        return 0;
    }
    if (g_manifest_path[0] == '\0' ||
        !csb_v22_famg_file_exists(g_manifest_path)) {
        return 0;
    }
    if (!csb_v22_famg_find_slot_in_manifest(g_manifest_path, asset_id, &raw)) {
        return 0;
    }
    if (!raw.has_id || !raw.has_generator || !raw.has_source_file ||
        !raw.has_width || !raw.has_height || raw.width <= 0 || raw.height <= 0 ||
        strcmp(raw.generator, CSB_V22_FAMG_SOURCE_EXPORT_GENERATOR) != 0) {
        return 0;
    }
    return csb_v22_famg_resolve_source_file(g_manifest_path, category,
                                            raw.source_file, resolved_path,
                                            sizeof(resolved_path)) &&
           csb_v22_famg_route_provenance_admits_slot(g_manifest_path, category,
                                                      asset_id);
}

/* ── Slot classification ───────────────────────────────────────── */
CSB_V22_FamgClass csb_v22_famg_classify_slot(CSB_V22_FamgSlot slot) {
    CSB_V22_FamgSlotRaw raw;
    int has_required;
    if ((unsigned)slot >= (unsigned)CSB_V22_FAMG_MATERIAL_COUNT) {
        return CSB_V22_FAMG_CLASS_UNKNOWN;
    }
    if (g_manifest_path[0] == '\0' ||
        !csb_v22_famg_file_exists(g_manifest_path)) {
        return CSB_V22_FAMG_CLASS_MISSING;
    }
    if (!csb_v22_famg_find_slot_in_manifest(g_manifest_path,
                                            k_slot_table[slot].id, &raw)) {
        return CSB_V22_FAMG_CLASS_MISSING;
    }
    has_required = raw.has_id && raw.has_generator && raw.has_source_file &&
                   raw.has_width && raw.has_height && raw.width > 0 && raw.height > 0;
    if (!has_required) return CSB_V22_FAMG_CLASS_PARTIAL;
    if (strcmp(raw.generator, "placeholder") == 0) return CSB_V22_FAMG_CLASS_PLACEHOLDER;
    return csb_v22_famg_admits_material(k_slot_table[slot].category,
                                        k_slot_table[slot].id)
        ? CSB_V22_FAMG_CLASS_REAL : CSB_V22_FAMG_CLASS_PARTIAL;
}

int csb_v22_famg_get_slot_info(CSB_V22_FamgSlot slot,
                                CSB_V22_FamgSlotInfo* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if ((unsigned)slot >= (unsigned)CSB_V22_FAMG_MATERIAL_COUNT) return 0;

    out->slot = slot;
    snprintf(out->id,       sizeof(out->id),       "%s", k_slot_table[slot].id);
    snprintf(out->category, sizeof(out->category), "%s", k_slot_table[slot].category);
    out->classification = csb_v22_famg_classify_slot(slot);

    if (g_manifest_path[0] == '\0' ||
        !csb_v22_famg_file_exists(g_manifest_path)) {
        return 0;
    }
    CSB_V22_FamgSlotRaw raw;
    if (!csb_v22_famg_find_slot_in_manifest(g_manifest_path,
                                            k_slot_table[slot].id, &raw)) {
        return 0;
    }
    if (raw.has_generator) {
        snprintf(out->generator, sizeof(out->generator), "%s", raw.generator);
    }
    if (raw.has_source_file) {
        snprintf(out->source_file, sizeof(out->source_file), "%s", raw.source_file);
    }
    out->width  = raw.has_width  ? raw.width  : 0;
    out->height = raw.has_height ? raw.height : 0;

    if (raw.has_source_file && raw.source_file[0] != '\0') {
        int exists = csb_v22_famg_resolve_source_file(g_manifest_path,
                                                      k_slot_table[slot].category,
                                                      raw.source_file,
                                                      out->resolved_path,
                                                      sizeof(out->resolved_path));
        out->file_exists = exists ? 1 : 0;
    }
    return 1;
}

/* ── Aggregate counts / gate ───────────────────────────────────── */
int csb_v22_famg_real_count(int* out_total) {
    int real = 0;
    int total = 0;
    if (g_manifest_path[0] != '\0' &&
        csb_v22_famg_file_exists(g_manifest_path)) {
        for (size_t i = 0; i < CSB_V22_FAMG_MATERIAL_COUNT; ++i) {
            CSB_V22_FamgClass cls =
                csb_v22_famg_classify_slot((CSB_V22_FamgSlot)i);
            if (cls != CSB_V22_FAMG_CLASS_MISSING) {
                ++total;
                if (cls == CSB_V22_FAMG_CLASS_REAL) ++real;
            }
        }
    }
    if (out_total) *out_total = total;
    return real;
}

CSB_V22_FamgGate csb_v22_famg_gate(void) {
    int total = 0;
    int real  = csb_v22_famg_real_count(&total);

    if (g_manifest_path[0] == '\0' ||
        !csb_v22_famg_file_exists(g_manifest_path)) {
        g_last_gate = CSB_V22_FAMG_GATE_NO_MANIFEST;
        g_installed = 0;
        return g_last_gate;
    }

    int manifest_valid = csb_v22_famg_validate_manifest(NULL);
    if (manifest_valid < 0) {
        g_last_gate = CSB_V22_FAMG_GATE_NO_MANIFEST;
        g_installed = 0;
        return g_last_gate;
    }

    if (total == 0) {
        /* Manifest present but no slot entries declared — still
         * synthetic/placeholder-mode default. */
        g_last_gate = CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER;
        g_installed = 0;
        return g_last_gate;
    }

    /* Missing slots are not optional. A short manifest with only one real
     * entry must remain partial rather than being promoted to a finished
     * pack simply because all of its declared entries happen to be real. */
    if (total == (int)CSB_V22_FAMG_MATERIAL_COUNT &&
        real == (int)CSB_V22_FAMG_MATERIAL_COUNT) {
        g_last_gate = CSB_V22_FAMG_GATE_FINISHED_REAL;
        g_installed = 1;
    } else if (real > 0) {
        g_last_gate = CSB_V22_FAMG_GATE_PARTIAL;
        g_installed = 1;
    } else {
        g_last_gate = CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER;
        g_installed = 0;
    }
    return g_last_gate;
}

/* ── Names ─────────────────────────────────────────────────────── */
const char* csb_v22_famg_slot_name(CSB_V22_FamgSlot slot) {
    if ((unsigned)slot >= (unsigned)CSB_V22_FAMG_MATERIAL_COUNT) return "UNKNOWN";
    return k_slot_table[slot].id;
}

const char* csb_v22_famg_slot_category(CSB_V22_FamgSlot slot) {
    if ((unsigned)slot >= (unsigned)CSB_V22_FAMG_MATERIAL_COUNT) return "";
    return k_slot_table[slot].category;
}

const char* csb_v22_famg_slot_manifest_id(CSB_V22_FamgSlot slot) {
    if ((unsigned)slot >= (unsigned)CSB_V22_FAMG_MATERIAL_COUNT) return "";
    return k_slot_table[slot].id;
}

const char* csb_v22_famg_class_name(CSB_V22_FamgClass cls) {
    switch (cls) {
        case CSB_V22_FAMG_CLASS_UNKNOWN:     return "UNKNOWN";
        case CSB_V22_FAMG_CLASS_MISSING:     return "MISSING";
        case CSB_V22_FAMG_CLASS_PLACEHOLDER: return "PLACEHOLDER";
        case CSB_V22_FAMG_CLASS_PARTIAL:     return "PARTIAL";
        case CSB_V22_FAMG_CLASS_REAL:        return "REAL";
        default: return "INVALID";
    }
}

const char* csb_v22_famg_gate_name(CSB_V22_FamgGate gate) {
    switch (gate) {
        case CSB_V22_FAMG_GATE_NOT_PROBED:            return "NOT_PROBED";
        case CSB_V22_FAMG_GATE_NO_MANIFEST:           return "NO_MANIFEST";
        case CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER: return "SYNTHETIC_PLACEHOLDER";
        case CSB_V22_FAMG_GATE_PARTIAL:               return "PARTIAL";
        case CSB_V22_FAMG_GATE_FINISHED_REAL:         return "FINISHED_REAL";
        default: return "INVALID";
    }
}

/* ── Installed flag / convenience queries ──────────────────────── */
void csb_v22_famg_set_installed(int installed) {
    g_installed = installed ? 1 : 0;
}

int csb_v22_famg_get_installed(void) {
    return g_installed;
}

int csb_v22_famg_uses_placeholder(CSB_V22_FamgSlot slot) {
    CSB_V22_FamgClass cls = csb_v22_famg_classify_slot(slot);
    switch (cls) {
        case CSB_V22_FAMG_CLASS_REAL:
            return 0;
        case CSB_V22_FAMG_CLASS_PLACEHOLDER:
        case CSB_V22_FAMG_CLASS_PARTIAL:
        case CSB_V22_FAMG_CLASS_MISSING:
        case CSB_V22_FAMG_CLASS_UNKNOWN:
        default:
            return 1;
    }
}

int csb_v22_famg_is_finished_real(void) {
    return csb_v22_famg_gate() == CSB_V22_FAMG_GATE_FINISHED_REAL ? 1 : 0;
}

int csb_v22_famg_is_synthetic_or_partial(void) {
    CSB_V22_FamgGate g = csb_v22_famg_gate();
    return (g == CSB_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER ||
            g == CSB_V22_FAMG_GATE_PARTIAL) ? 1 : 0;
}

/* ── Per-cell routing ─────────────────────────────────────────────
 *
 * Map (depth, lateral) to the FAMG slot the runtime swap would route
 * the cell's depth-specific wall fallback to. Exact material selection
 * is owned by csb_v22_inplace_route_pc34.c, which also emits floors,
 * doors, creatures, runes and special shapes.
 *
 * Floor variants route through FLOOR_PLAIN and FLOOR_CRACKED via the
 * caller; cells that have already been classified by csb_v22_inplace
 * can be reclassified via classify_slot() directly.
 *
 * Returns CSB_V22_FAMG_MATERIAL_COUNT (= sentinel "out of range")
 * when depth or lateral is outside the 9-square viewport bounds. */
CSB_V22_FamgSlot csb_v22_famg_slot_for_cell(int depth, int lateral) {
    /* This diagnostic helper tracks the same depth convention as the
     * active route. Shape-specific selection remains in the route module. */
    if (depth < 0 || depth > 2) return CSB_V22_FAMG_MATERIAL_COUNT;
    if (lateral < -1 || lateral > 1) return CSB_V22_FAMG_MATERIAL_COUNT;
    return (CSB_V22_FamgSlot)(CSB_V22_FAMG_WALL_DUNGEON_D0 + depth);
}

CSB_V22_FamgClass csb_v22_famg_classify_cell(int depth, int lateral) {
    CSB_V22_FamgSlot slot = csb_v22_famg_slot_for_cell(depth, lateral);
    if ((unsigned)slot >= (unsigned)CSB_V22_FAMG_MATERIAL_COUNT) {
        return CSB_V22_FAMG_CLASS_UNKNOWN;
    }
    return csb_v22_famg_classify_slot(slot);
}

const char* csb_v22_famg_source_evidence(void) {
    return
        "CSB V2.2 finished-art material gate - placeholder-vs-real classifier\n"
        "Source: ReDMCSB DUNVIEW.C F0128       (CSB 9-square viewport routing)\n"
        "Source: ReDMCSB PANEL.C F0354        (CSB champion panel refresh)\n"
        "Source: ReDMCSB COMMAND.C:108-113    (command dispatch open quote)\n"
        "Source: ReDMCSB COMMAND.C:254-291    (command dispatch body)\n"
        "Source: ReDMCSB ENTRANCE.C           (CSB prison door + intro)\n"
        "Source: ReDMCSB LIGHT.C F0212        (CSB torchlight + lighting)\n"
        "Source: CSBWin/Viewport.cpp:7290     (CSB 9-square viewport layout)\n"
        "Source: CSBWin/Chaos.cpp:60-69       (DSA / chaos rune dispatch)\n"
        "Source: include/csb_v22_inplace_draw_pc34.h (cell -> variant -> asset_id)\n"
        "Source: include/csb_v22_modern_assets_pc34.h (modern asset manifest path)\n"
        "Source: include/csb_v22_shapes.h     (CSB V2.2 shape classification)\n"
        "Source: scripts/build_csb_v22_source_fsart.py (PC3.4 source export)\n"
        "Source: sibling dm1_v22 / dm2_v22 FAMG modules (placeholder-vs-real pattern)\n"
        "Source: docs/FIRESTAFF_GAP_LIST.md B3 V2 per-mode material row\n"
        "Manifest path: ~/.firestaff/assets/csb/modern/modern_asset_manifest.json\n"
        "Schema: { id, generator, source_file, width, height } per slot entry\n"
        "Each REAL slot also needs routeProvenance.f0128ProjectionStatus=admitted_*\n"
        "Generator 'placeholder' is the procedural fallback marker (synthetic)\n"
        "Only generator 'original_csb_pc34_graphics_dat' + source_file + admitted projection = REAL\n"
        "Gate states: NOT_PROBED / NO_MANIFEST / SYNTHETIC_PLACEHOLDER / PARTIAL / FINISHED_REAL\n"
        "FINISHED_REAL requires every tracked slot to be a PC3.4 source export\n"
        "V1 invariant: V1 command routes, dungeon state, save/restore NEVER bypassed\n"
        "V2 rule: finished-art material only activates when V2 launch+profile enabled\n"
        "Honest boundary: this gate tracks manifest classification only.\n"
        "It does NOT claim finished PBR art has been reviewed or shipped.\n"
        "FINISHED_REAL promotion requires operator-installed PC3.4 source exports at\n"
        "~/.firestaff/assets/csb/modern/<category>/<source_file> with\n"
        "generator == 'original_csb_pc34_graphics_dat', non-zero width/height, and an\n"
        "explicit admitted F0128 projection, plus a\n"
        "sibling gap-list update to mark the real-asset promotion gate green.\n";
}
