/*
 * dm1_v22_finished_art_material_gate_pc34.c
 *
 * DM1 V2.2 finished-art / material screenshot pixel gate.
 *
 * Companion to include/dm1_v22_finished_art_material_gate_pc34.h.
 *
 * This module is the CI-runnable distinction between:
 *   - SYNTHETIC_PLACEHOLDER (procedural / "placeholder" generator,
 *     the current honest runtime default)
 *   - FINISHED_REAL        (operator-installed hero PNGs with
 *     generator != "placeholder" and source_file resolving on disk)
 *
 * It runs in CI without requiring real PBR art on disk by:
 *   1. Reading the existing modern_asset_manifest.json (when present)
 *      and classifying every tracked material slot.
 *   2. Exercising the gate state machine via synthetic manifest
 *      fixtures authored by the test/probe under a probe-only HOME.
 *
 * The manifest schema reuses the modern_asset_manifest.json format
 * defined for include/dm1_v2_asset_pipeline_pc34.h, with the
 * addition of an optional `generator` field. When `generator` ==
 * "placeholder", the slot is the procedural fallback. Any other
 * value (e.g. "pbr_hero", "ai_upscale", "reviewed") is a non-
 * placeholder marker that, combined with a disk-resolvable PNG whose
 * IHDR dimensions match the manifest, promotes the slot to REAL.
 *
 * Source-lock:
 *   - ReDMCSB DUNVIEW.C:6697-6816 (DM1 viewport composition order)
 *   - ReDMCSB DUNGEON.C:2238-2246 (square-type decode feeding
 *     m11_v22_shape_for_cell())
 *   - include/dm1_v2_asset_pipeline_pc34.h (modern asset manifest
 *     path resolution)
 *   - include/m11_v22_inplace_draw_pc34.h (cell -> variant -> asset_id)
 *   - sibling dm2_v2_hud_widget_assets.c (placeholder-vs-real pattern)
 *
 * Honest boundary: this gate tracks manifest classification only.
 * It does NOT claim finished PBR art has been reviewed or shipped.
 * FINISHED_REAL is reachable only when an operator has dropped a
 * non-placeholder manifest with source_file paths that resolve on
 * disk; until then the gate stays in SYNTHETIC_PLACEHOLDER, which
 * matches the honest current default.
 */

#include "dm1_v22_finished_art_material_gate_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Slot table (stable, ordered) ────────────────────────────────
 *
 * Each slot pairs:
 *   - id           : manifest entry id (stable, matches the
 *                    hero_01 ids in
 *                    tests/test_dm1_v22_real_asset_material_gate_pc34.c
 *                    so this gate and the SKIP-only sibling gate
 *                    cover the same manifest surface).
 *   - category     : modern_asset_manifest.json top-level category
 *                    (drives source_file path resolution).
 *   - asset_id_v22 : the synthetic / m11_v22_inplace asset_id used
 *                    by the in-place cache when no real hero pack is
 *                    installed. Reported in the slot info record for
 *                    honest gap tracking.
 *
 * Reordering / inserting slots must keep DM1_V22_FAMG_MATERIAL_COUNT
 * in sync and add an entry here. */
typedef struct {
    DM1_V22_FamgSlot slot;
    const char*      id;            /* modern_asset_manifest.json id */
    const char*      category;      /* wall_shapes / creature_shapes / ... */
    const char*      asset_id_v22;  /* m11_v22_inplace asset_id when
                                       procedural fallback is used */
} DM1_V22_FamgSlotDesc;

static const DM1_V22_FamgSlotDesc k_slot_table[DM1_V22_FAMG_MATERIAL_COUNT] = {
    {
        DM1_V22_FAMG_WALL_D3_CARVED,
        "wall_d3_carved_hero_01",
        "wall_shapes",
        "wall_d3_carved_01"
    },
    {
        DM1_V22_FAMG_FLOOR_PLAIN,
        "floor_plain_hero_01",
        "floor_shapes",
        "floor_plain_01"
    },
    {
        DM1_V22_FAMG_FLOOR_PIT,
        "floor_pit_hero_01",
        "floor_shapes",
        "floor_pit_01"
    },
    {
        DM1_V22_FAMG_CREATURE_DEMON,
        "creature_demon_hero_01",
        "creature_shapes",
        "creature_demon_01"
    },
    {
        DM1_V22_FAMG_CHAMPION_WARRIOR,
        "champion_warrior_hero_01",
        "champion_portraits",
        NULL /* champion portrait falls back to the runtime champion-stat
                renderer; there is no procedural champion portrait asset
                in the in-place cache. */
    },
    {
        DM1_V22_FAMG_DOOR_FRONT,
        "door_hero_01",
        "door_shapes",
        NULL /* doors are drawn by m11_draw_dm1_door_pc34 directly; no
                in-place asset_id exists in the synthetic cache. */
    },
    {
        DM1_V22_FAMG_TELEPORTER_FIELD,
        "field_teleporter_hero_01",
        "field_shapes",
        "field_teleporter_01"
    }
};

static const char* k_receipt_id =
    "dm1_v22_real_screenshot_material_receipt_01";

/* ── Module state ──────────────────────────────────────────────── */
static char               g_manifest_path[FSP_PATH_MAX] = {0};
static int                g_installed = 0;     /* last gate: 1 if PARTIAL/FINISHED_REAL */
static DM1_V22_FamgGate   g_last_gate = DM1_V22_FAMG_GATE_NOT_PROBED;

/* ── Trimming / JSON helpers ───────────────────────────────────── */
static void dm1_v22_famg_trim(char* dst, const char* src, size_t dstSize) {
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

static int dm1_v22_famg_file_exists(const char* path) {
    if (!path || path[0] == '\0') return 0;
    FILE* fp = fopen(path, "rb");
    if (fp) { fclose(fp); return 1; }
    return 0;
}

static uint32_t dm1_v22_famg_read_be32(const unsigned char* p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |
           (uint32_t)p[3];
}

static int dm1_v22_famg_png_header(const char* path,
                                   int* out_width,
                                   int* out_height) {
    static const unsigned char k_png_sig[8] = {
        0x89u, 0x50u, 0x4eu, 0x47u, 0x0du, 0x0au, 0x1au, 0x0au
    };
    unsigned char header[24];
    FILE* fp;
    size_t got;
    if (out_width) *out_width = 0;
    if (out_height) *out_height = 0;
    if (!path || path[0] == '\0') return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    got = fread(header, 1, sizeof(header), fp);
    fclose(fp);
    if (got != sizeof(header)) return 0;
    if (memcmp(header, k_png_sig, sizeof(k_png_sig)) != 0) return 0;
    if (header[8] != 0x00u || header[9] != 0x00u ||
        header[10] != 0x00u || header[11] != 0x0du) {
        return 0;
    }
    if (memcmp(header + 12, "IHDR", 4) != 0) return 0;
    uint32_t w = dm1_v22_famg_read_be32(header + 16);
    uint32_t h = dm1_v22_famg_read_be32(header + 20);
    if (w == 0U || h == 0U || w > 32768U || h > 32768U) return 0;
    if (out_width) *out_width = (int)w;
    if (out_height) *out_height = (int)h;
    return 1;
}

static int dm1_v22_famg_png_header_matches(const char* path,
                                           int expected_width,
                                           int expected_height,
                                           int* out_width,
                                           int* out_height) {
    int actual_width = 0;
    int actual_height = 0;
    if (!dm1_v22_famg_png_header(path, &actual_width, &actual_height)) {
        if (out_width) *out_width = actual_width;
        if (out_height) *out_height = actual_height;
        return 0;
    }
    if (out_width) *out_width = actual_width;
    if (out_height) *out_height = actual_height;
    return actual_width == expected_width && actual_height == expected_height;
}

static int dm1_v22_famg_file_starts_with_object(const char* path) {
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
    char frame_hash[128];
    char material_gate[64];
    int  width;
    int  height;
    int  has_id;
    int  has_generator;
    int  has_source_file;
    int  has_frame_hash;
    int  has_material_gate;
    int  has_width;
    int  has_height;
} DM1_V22_FamgSlotRaw;

static void dm1_v22_famg_slot_raw_init(DM1_V22_FamgSlotRaw* r) {
    memset(r, 0, sizeof(*r));
}

/* ── Entry-content accumulator ──────────────────────────────────
 *
 * The manifest can be single-line or pretty-printed. We accumulate
 * each entry's raw JSON text as we walk braces/brackets, then run
 * field extractors against the buffer when the entry closes. This
 * handles all three layouts the dm2 sibling module supports:
 *   1. Single-line JSON (entry spans one line).
 *   2. Fields on the same line as the closing `}`.
 *   3. Pretty-printed multi-line entries whose `{` and `}` are on
 *      separate lines from the field lines. */
static char g_entry_buf[16384];
static int  g_entry_buf_len = 0;

static void dm1_v22_famg_buf_reset(void) {
    g_entry_buf_len = 0;
    if (g_entry_buf_len < (int)sizeof(g_entry_buf)) {
        g_entry_buf[g_entry_buf_len] = '\0';
    }
}

static void dm1_v22_famg_buf_append_char(char c) {
    if (g_entry_buf_len + 1 < (int)sizeof(g_entry_buf)) {
        g_entry_buf[g_entry_buf_len++] = c;
        g_entry_buf[g_entry_buf_len] = '\0';
    }
}

static int dm1_v22_famg_extract_string(const char* line, const char* key,
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

static int dm1_v22_famg_extract_int(const char* line, const char* key,
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

static void dm1_v22_famg_extract_fields_from_buf(DM1_V22_FamgSlotRaw* out) {
    char val[256];
    if (dm1_v22_famg_extract_string(g_entry_buf, "id", val, sizeof(val))) {
        dm1_v22_famg_trim(out->id, val, sizeof(out->id));
        out->has_id = 1;
    }
    if (dm1_v22_famg_extract_string(g_entry_buf, "generator", val, sizeof(val))) {
        dm1_v22_famg_trim(out->generator, val, sizeof(out->generator));
        out->has_generator = 1;
    }
    if (dm1_v22_famg_extract_string(g_entry_buf, "source_file", val, sizeof(val))) {
        dm1_v22_famg_trim(out->source_file, val, sizeof(out->source_file));
        out->has_source_file = 1;
    }
    if (dm1_v22_famg_extract_string(g_entry_buf, "frame_hash", val, sizeof(val))) {
        dm1_v22_famg_trim(out->frame_hash, val, sizeof(out->frame_hash));
        out->has_frame_hash = 1;
    }
    if (dm1_v22_famg_extract_string(g_entry_buf, "material_gate", val, sizeof(val))) {
        dm1_v22_famg_trim(out->material_gate, val, sizeof(out->material_gate));
        out->has_material_gate = 1;
    }
    int w = 0, h = 0;
    if (dm1_v22_famg_extract_int(g_entry_buf, "width", &w)) {
        out->width = w;
        out->has_width = 1;
    }
    if (dm1_v22_famg_extract_int(g_entry_buf, "height", &h)) {
        out->height = h;
        out->has_height = 1;
    }
}

/* ── Path resolution ───────────────────────────────────────────── */
void dm1_v22_famg_set_manifest_path(const char* dataDir) {
    if (!dataDir || dataDir[0] == '\0') {
        g_manifest_path[0] = '\0';
        return;
    }
    /* ~/.firestaff/data/dm1 -> ~/.firestaff -> assets/dm1/modern/modern_asset_manifest.json
     * Walks two parents up from dataDir, same pattern as
     * m11_v22_set_manifest_path in dm1_v2_asset_pipeline_pc34.c. */
    char parent1[FSP_PATH_MAX];
    char parent2[FSP_PATH_MAX];
    char assets_root[FSP_PATH_MAX];
    char dm1_modern_dir[FSP_PATH_MAX];
    if (!FSP_ParentDir(parent1, sizeof(parent1), dataDir) ||
        !FSP_ParentDir(parent2, sizeof(parent2), parent1)) {
        FSP_JoinPath(assets_root, sizeof(assets_root), dataDir, "assets");
    } else {
        FSP_JoinPath(assets_root, sizeof(assets_root), parent2, "assets");
    }
    FSP_JoinPath(dm1_modern_dir, sizeof(dm1_modern_dir), assets_root, "dm1");
    FSP_JoinPath(dm1_modern_dir, sizeof(dm1_modern_dir), dm1_modern_dir, "modern");
    FSP_JoinPath(g_manifest_path, sizeof(g_manifest_path),
                 dm1_modern_dir, "modern_asset_manifest.json");
}

const char* dm1_v22_famg_get_manifest_path(void) {
    return g_manifest_path;
}

/* ── Internal manifest scan ────────────────────────────────────── */
static int dm1_v22_famg_find_slot_in_manifest(const char* manifest_path,
                                              const char* slot_id,
                                              DM1_V22_FamgSlotRaw* out) {
    if (!manifest_path || manifest_path[0] == '\0' || !slot_id || !out) return 0;
    dm1_v22_famg_slot_raw_init(out);

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

    /* Object-based pass: modern_asset_manifest.json fixtures are often
     * compact one-line JSON with multiple top-level categories. Rather
     * than latching one category per physical line, scan every JSON
     * object and extract its fields. The outer manifest object is also
     * scanned but will not match later slot ids because extract_string()
     * returns only the first id inside that outer object. */
    const char* scan = text;
    while ((scan = strchr(scan, '{')) != NULL) {
        const char* end = scan;
        int depth = 0;
        while (*end) {
            if (*end == '{') {
                depth++;
            } else if (*end == '}') {
                depth--;
                if (depth == 0) {
                    break;
                }
            }
            ++end;
        }
        if (*end != '}') break;

        dm1_v22_famg_buf_reset();
        {
            const char* c = scan;
            while (c <= end) {
                dm1_v22_famg_buf_append_char(*c);
                ++c;
            }
        }

        DM1_V22_FamgSlotRaw raw;
        dm1_v22_famg_slot_raw_init(&raw);
        dm1_v22_famg_extract_fields_from_buf(&raw);
        if (raw.has_id && strcmp(raw.id, slot_id) == 0) {
            *out = raw;
            free(text);
            return 1;
        }

        scan++;
    }

    free(text);
    return 0;
}

/* Resolve a manifest source_file (relative) against the manifest dir.
 * Returns 1 if the resolved path exists on disk, 0 otherwise.
 *
 * For DM1 V2.2 materials the source_file is expected to live under
 * <modern-dir>/<category>/<source_file>, e.g.:
 *   ~/.firestaff/assets/dm1/modern/wall_shapes/wall_d3_carved_hero_01.png
 */
static int dm1_v22_famg_resolve_source_file(const char* manifest_path,
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
    return dm1_v22_famg_file_exists(joined);
}

static int dm1_v22_famg_resolve_receipt_file(const char* manifest_path,
                                             const char* source_file,
                                             char* out, size_t outSize) {
    return dm1_v22_famg_resolve_source_file(manifest_path,
                                            "receipts",
                                            source_file,
                                            out,
                                            outSize);
}

static int dm1_v22_famg_generator_is_synthetic(const char* generator) {
    if (!generator || generator[0] == '\0') return 0;
    return (strcmp(generator, "placeholder") == 0 ||
            strcmp(generator, "synthetic") == 0 ||
            strcmp(generator, "synthetic_test") == 0) ? 1 : 0;
}

/* ── Validation ────────────────────────────────────────────────── */
int dm1_v22_famg_validate_manifest(const char* manifest_path) {
    const char* p = manifest_path ? manifest_path : g_manifest_path;
    if (!p || p[0] == '\0') return -1;
    if (!dm1_v22_famg_file_exists(p)) return -1;
    if (!dm1_v22_famg_file_starts_with_object(p)) return -1;

    /* Probe the first required slot. If it can be located AND has
     * all required fields, the manifest is considered valid; a partial
     * manifest (some required fields missing) returns 0. */
    const char* first_id = k_slot_table[0].id;
    DM1_V22_FamgSlotRaw raw;
    if (!dm1_v22_famg_find_slot_in_manifest(p, first_id, &raw)) {
        return 0; /* manifest readable but no slot declared */
    }
    int complete = raw.has_id && raw.has_generator &&
                   raw.has_source_file && raw.has_width &&
                   raw.has_height && raw.width > 0 && raw.height > 0;
    return complete ? 1 : 0;
}

/* ── Slot classification ───────────────────────────────────────── */
DM1_V22_FamgClass dm1_v22_famg_classify_slot(DM1_V22_FamgSlot slot) {
    if ((unsigned)slot >= (unsigned)DM1_V22_FAMG_MATERIAL_COUNT) {
        return DM1_V22_FAMG_CLASS_UNKNOWN;
    }
    if (g_manifest_path[0] == '\0' ||
        !dm1_v22_famg_file_exists(g_manifest_path)) {
        return DM1_V22_FAMG_CLASS_MISSING;
    }

    DM1_V22_FamgSlotRaw raw;
    if (!dm1_v22_famg_find_slot_in_manifest(g_manifest_path,
                                            k_slot_table[slot].id, &raw)) {
        return DM1_V22_FAMG_CLASS_MISSING;
    }

    int has_required = raw.has_id && raw.has_generator &&
                       raw.has_source_file && raw.has_width &&
                       raw.has_height && raw.width > 0 && raw.height > 0;
    if (!has_required) {
        return DM1_V22_FAMG_CLASS_PARTIAL;
    }

    /* generator == "placeholder" is the explicit fallback marker */
    if (strcmp(raw.generator, "placeholder") == 0) {
        return DM1_V22_FAMG_CLASS_PLACEHOLDER;
    }

    /* Real asset: required fields + non-placeholder generator +
     * source_file resolves on disk + PNG IHDR dimensions match the
     * manifest. This keeps text files renamed to .png from promoting
     * the finished-art gate. */
    char resolved_path[FSP_PATH_MAX];
    int exists = dm1_v22_famg_resolve_source_file(g_manifest_path,
                                                  k_slot_table[slot].category,
                                                  raw.source_file,
                                                  resolved_path,
                                                  sizeof(resolved_path));
    if (!exists) {
        return DM1_V22_FAMG_CLASS_PARTIAL;
    }
    return dm1_v22_famg_png_header_matches(resolved_path,
                                           raw.width,
                                           raw.height,
                                           NULL,
                                           NULL)
               ? DM1_V22_FAMG_CLASS_REAL
               : DM1_V22_FAMG_CLASS_PARTIAL;
}

int dm1_v22_famg_get_slot_info(DM1_V22_FamgSlot slot,
                                DM1_V22_FamgSlotInfo* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if ((unsigned)slot >= (unsigned)DM1_V22_FAMG_MATERIAL_COUNT) return 0;

    out->slot = slot;
    snprintf(out->id,       sizeof(out->id),       "%s", k_slot_table[slot].id);
    snprintf(out->category, sizeof(out->category), "%s", k_slot_table[slot].category);
    out->classification = dm1_v22_famg_classify_slot(slot);

    if (g_manifest_path[0] == '\0' ||
        !dm1_v22_famg_file_exists(g_manifest_path)) {
        return 0;
    }
    DM1_V22_FamgSlotRaw raw;
    if (!dm1_v22_famg_find_slot_in_manifest(g_manifest_path,
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
        int exists = dm1_v22_famg_resolve_source_file(g_manifest_path,
                                                      k_slot_table[slot].category,
                                                      raw.source_file,
                                                      out->resolved_path,
                                                      sizeof(out->resolved_path));
        out->file_exists = exists ? 1 : 0;
        if (exists) {
            out->png_header_valid =
                dm1_v22_famg_png_header_matches(out->resolved_path,
                                                out->width,
                                                out->height,
                                                &out->png_width,
                                                &out->png_height);
        }
    }
    return 1;
}

/* ── Aggregate counts / gate ───────────────────────────────────── */
int dm1_v22_famg_real_count(int* out_total) {
    int real = 0;
    int total = 0;
    if (g_manifest_path[0] != '\0' &&
        dm1_v22_famg_file_exists(g_manifest_path)) {
        for (size_t i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
            DM1_V22_FamgClass cls =
                dm1_v22_famg_classify_slot((DM1_V22_FamgSlot)i);
            if (cls != DM1_V22_FAMG_CLASS_MISSING) {
                ++total;
                if (cls == DM1_V22_FAMG_CLASS_REAL) ++real;
            }
        }
    }
    if (out_total) *out_total = total;
    return real;
}

DM1_V22_FamgGate dm1_v22_famg_gate(void) {
    int total = 0;
    int real  = dm1_v22_famg_real_count(&total);

    if (g_manifest_path[0] == '\0' ||
        !dm1_v22_famg_file_exists(g_manifest_path)) {
        g_last_gate = DM1_V22_FAMG_GATE_NO_MANIFEST;
        g_installed = 0;
        return g_last_gate;
    }

    int manifest_valid = dm1_v22_famg_validate_manifest(NULL);
    if (manifest_valid < 0) {
        g_last_gate = DM1_V22_FAMG_GATE_NO_MANIFEST;
        g_installed = 0;
        return g_last_gate;
    }

    if (total == 0) {
        /* Manifest present but no slot entries declared — still
         * synthetic/placeholder-mode default. */
        g_last_gate = DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER;
        g_installed = 0;
        return g_last_gate;
    }

    if (real == (int)DM1_V22_FAMG_MATERIAL_COUNT) {
        g_last_gate = DM1_V22_FAMG_GATE_FINISHED_REAL;
        g_installed = 1;
    } else if (real > 0) {
        g_last_gate = DM1_V22_FAMG_GATE_PARTIAL;
        g_installed = 1;
    } else {
        g_last_gate = DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER;
        g_installed = 0;
    }
    return g_last_gate;
}

/* ── Names ─────────────────────────────────────────────────────── */
const char* dm1_v22_famg_slot_name(DM1_V22_FamgSlot slot) {
    if ((unsigned)slot >= (unsigned)DM1_V22_FAMG_MATERIAL_COUNT) return "UNKNOWN";
    return k_slot_table[slot].id;
}

const char* dm1_v22_famg_slot_category(DM1_V22_FamgSlot slot) {
    if ((unsigned)slot >= (unsigned)DM1_V22_FAMG_MATERIAL_COUNT) return "";
    return k_slot_table[slot].category;
}

const char* dm1_v22_famg_slot_manifest_id(DM1_V22_FamgSlot slot) {
    if ((unsigned)slot >= (unsigned)DM1_V22_FAMG_MATERIAL_COUNT) return "";
    return k_slot_table[slot].id;
}

const char* dm1_v22_famg_class_name(DM1_V22_FamgClass cls) {
    switch (cls) {
        case DM1_V22_FAMG_CLASS_UNKNOWN:     return "UNKNOWN";
        case DM1_V22_FAMG_CLASS_MISSING:     return "MISSING";
        case DM1_V22_FAMG_CLASS_PLACEHOLDER: return "PLACEHOLDER";
        case DM1_V22_FAMG_CLASS_PARTIAL:     return "PARTIAL";
        case DM1_V22_FAMG_CLASS_REAL:        return "REAL";
        default: return "INVALID";
    }
}

const char* dm1_v22_famg_gate_name(DM1_V22_FamgGate gate) {
    switch (gate) {
        case DM1_V22_FAMG_GATE_NOT_PROBED:            return "NOT_PROBED";
        case DM1_V22_FAMG_GATE_NO_MANIFEST:           return "NO_MANIFEST";
        case DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER: return "SYNTHETIC_PLACEHOLDER";
        case DM1_V22_FAMG_GATE_PARTIAL:               return "PARTIAL";
        case DM1_V22_FAMG_GATE_FINISHED_REAL:         return "FINISHED_REAL";
        default: return "INVALID";
    }
}

/* ── Installed flag / convenience queries ──────────────────────── */
void dm1_v22_famg_set_installed(int installed) {
    g_installed = installed ? 1 : 0;
}

int dm1_v22_famg_get_installed(void) {
    return g_installed;
}

int dm1_v22_famg_uses_placeholder(DM1_V22_FamgSlot slot) {
    DM1_V22_FamgClass cls = dm1_v22_famg_classify_slot(slot);
    switch (cls) {
        case DM1_V22_FAMG_CLASS_REAL:
            return 0;
        case DM1_V22_FAMG_CLASS_PLACEHOLDER:
        case DM1_V22_FAMG_CLASS_PARTIAL:
        case DM1_V22_FAMG_CLASS_MISSING:
        case DM1_V22_FAMG_CLASS_UNKNOWN:
        default:
            return 1;
    }
}

int dm1_v22_famg_is_finished_real(void) {
    return dm1_v22_famg_gate() == DM1_V22_FAMG_GATE_FINISHED_REAL ? 1 : 0;
}

int dm1_v22_famg_is_synthetic_or_partial(void) {
    DM1_V22_FamgGate g = dm1_v22_famg_gate();
    return (g == DM1_V22_FAMG_GATE_SYNTHETIC_PLACEHOLDER ||
            g == DM1_V22_FAMG_GATE_PARTIAL) ? 1 : 0;
}

const char* dm1_v22_famg_receipt_manifest_id(void) {
    return k_receipt_id;
}

DM1_V22_FamgReceiptGate dm1_v22_famg_receipt_gate(void) {
    if (g_manifest_path[0] == '\0' ||
        !dm1_v22_famg_file_exists(g_manifest_path)) {
        return DM1_V22_FAMG_RECEIPT_NO_RECEIPT;
    }

    DM1_V22_FamgSlotRaw raw;
    if (!dm1_v22_famg_find_slot_in_manifest(g_manifest_path,
                                            k_receipt_id,
                                            &raw)) {
        return DM1_V22_FAMG_RECEIPT_NO_RECEIPT;
    }

    if (!raw.has_id || !raw.has_generator ||
        !raw.has_source_file || !raw.has_width || !raw.has_height ||
        !raw.has_frame_hash || !raw.has_material_gate ||
        raw.width <= 0 || raw.height <= 0) {
        return DM1_V22_FAMG_RECEIPT_PARTIAL;
    }

    if (dm1_v22_famg_generator_is_synthetic(raw.generator)) {
        return DM1_V22_FAMG_RECEIPT_SYNTHETIC_PLACEHOLDER;
    }

    if (strcmp(raw.material_gate, "FINISHED_REAL") != 0 ||
        dm1_v22_famg_gate() != DM1_V22_FAMG_GATE_FINISHED_REAL) {
        return DM1_V22_FAMG_RECEIPT_PARTIAL;
    }

    char resolved_path[FSP_PATH_MAX];
    return dm1_v22_famg_resolve_receipt_file(g_manifest_path,
                                             raw.source_file,
                                             resolved_path,
                                             sizeof(resolved_path))
        ? DM1_V22_FAMG_RECEIPT_FINISHED_REAL
        : DM1_V22_FAMG_RECEIPT_PARTIAL;
}

const char* dm1_v22_famg_receipt_gate_name(DM1_V22_FamgReceiptGate gate) {
    switch (gate) {
        case DM1_V22_FAMG_RECEIPT_NOT_PROBED:            return "NOT_PROBED";
        case DM1_V22_FAMG_RECEIPT_NO_RECEIPT:            return "NO_RECEIPT";
        case DM1_V22_FAMG_RECEIPT_SYNTHETIC_PLACEHOLDER: return "SYNTHETIC_PLACEHOLDER";
        case DM1_V22_FAMG_RECEIPT_PARTIAL:               return "PARTIAL";
        case DM1_V22_FAMG_RECEIPT_FINISHED_REAL:         return "FINISHED_REAL";
        default: return "INVALID";
    }
}

int dm1_v22_famg_get_receipt_info(DM1_V22_FamgReceiptInfo* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    snprintf(out->id, sizeof(out->id), "%s", k_receipt_id);
    out->gate = dm1_v22_famg_receipt_gate();

    if (g_manifest_path[0] == '\0' ||
        !dm1_v22_famg_file_exists(g_manifest_path)) {
        return 0;
    }

    DM1_V22_FamgSlotRaw raw;
    if (!dm1_v22_famg_find_slot_in_manifest(g_manifest_path,
                                            k_receipt_id,
                                            &raw)) {
        return 0;
    }

    if (raw.has_generator) {
        snprintf(out->generator, sizeof(out->generator), "%s", raw.generator);
    }
    if (raw.has_source_file) {
        snprintf(out->source_file, sizeof(out->source_file), "%s", raw.source_file);
    }
    if (raw.has_frame_hash) {
        snprintf(out->frame_hash, sizeof(out->frame_hash), "%s", raw.frame_hash);
    }
    if (raw.has_material_gate) {
        snprintf(out->material_gate, sizeof(out->material_gate), "%s", raw.material_gate);
    }
    out->width = raw.has_width ? raw.width : 0;
    out->height = raw.has_height ? raw.height : 0;
    if (raw.has_source_file && raw.source_file[0] != '\0') {
        out->file_exists = dm1_v22_famg_resolve_receipt_file(g_manifest_path,
                                                             raw.source_file,
                                                             out->resolved_path,
                                                             sizeof(out->resolved_path))
            ? 1 : 0;
    }
    return 1;
}

int dm1_v22_famg_has_finished_real_receipt(void) {
    return dm1_v22_famg_receipt_gate() ==
        DM1_V22_FAMG_RECEIPT_FINISHED_REAL ? 1 : 0;
}

int dm1_v22_famg_has_synthetic_receipt(void) {
    return dm1_v22_famg_receipt_gate() ==
        DM1_V22_FAMG_RECEIPT_SYNTHETIC_PLACEHOLDER ? 1 : 0;
}

const char* dm1_v22_famg_source_evidence(void) {
    return
        "DM1 V2.2 finished-art material gate — placeholder-vs-real classifier\n"
        "Source: ReDMCSB DUNVIEW.C:6697-6816 (DM1 viewport composition order)\n"
        "Source: ReDMCSB DUNGEON.C:2238-2246 (square-type decode feeding m11_v22_shape_for_cell)\n"
        "Source: ReDMCSB PANEL.C F0354       (champion status-box drawing)\n"
        "Source: include/dm1_v2_asset_pipeline_pc34.h (modern asset manifest path)\n"
        "Source: include/m11_v22_inplace_draw_pc34.h (cell -> variant -> asset_id)\n"
        "Source: include/m11_v22_shape_cache_pc34.h (per-frame V22 shape cache)\n"
        "Source: src/dm1v2/dm1_v22_shapes.c  (DM1 V2.2 shape classification)\n"
        "Source: src/dm1v2/dm1_v2_modern_assets_pc34.c (missing-asset placeholder 16x16 magenta)\n"
        "Source: sibling dm2_v2_hud_widget_assets.c (placeholder-vs-real pattern)\n"
        "Source: docs/FIRESTAFF_GAP_LIST.md B3 V2 per-mode material row\n"
        "Manifest path: ~/.firestaff/assets/dm1/modern/modern_asset_manifest.json\n"
        "Schema: { id, generator, source_file, width, height } per slot entry\n"
        "Generator 'placeholder' is the procedural fallback marker (synthetic)\n"
        "Non-placeholder generator + source_file resolves on disk + PNG IHDR dimensions match = REAL\n"
        "Gate states: NOT_PROBED / NO_MANIFEST / SYNTHETIC_PLACEHOLDER / PARTIAL / FINISHED_REAL\n"
        "FINISHED_REAL requires every tracked slot to be REAL with non-placeholder generator\n"
        "Receipt id: dm1_v22_real_screenshot_material_receipt_01\n"
        "Receipt schema: { id, generator, source_file, width, height, frame_hash, material_gate }\n"
        "Receipt source_file resolves under ~/.firestaff/assets/dm1/modern/receipts/\n"
        "Receipt FINISHED_REAL requires material gate FINISHED_REAL and non-synthetic generator\n"
        "V1 invariant: V1 command routes, dungeon state, save/restore NEVER bypassed\n"
        "V2 rule: finished-art material only activates when V2 launch+profile enabled\n"
        "Honest boundary: this gate tracks manifest classification only.\n"
        "It does NOT claim finished PBR art has been reviewed or shipped.\n"
        "FINISHED_REAL promotion requires operator-installed hero PNGs at\n"
        "~/.firestaff/assets/dm1/modern/<category>/<source_file> with\n"
        "generator != 'placeholder', PNG signature + IHDR dimensions matching\n"
        "the manifest width/height, plus a\n"
        "sibling gap-list update to mark the real-asset promotion gate green.\n";
}
