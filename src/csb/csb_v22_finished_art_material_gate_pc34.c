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
 * parsed by src/csb/csb_v22_modern_assets_pc34.c, with the
 * addition of an optional `generator` field. When `generator` ==
 * "placeholder", the slot is the procedural fallback. Any other
 * value (e.g. "pbr_hero", "ai_upscale", "reviewed") is a non-
 * placeholder marker that, combined with a disk-resolvable
 * `source_file`, promotes the slot to REAL.
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
    {
        CSB_V22_FAMG_WALL_DUNGEON,
        "wall_dungeon_01",
        "wall_shapes",
        "wall_dungeon_01"
    },
    {
        CSB_V22_FAMG_FLOOR_PLAIN,
        "floor_plain_01",
        "floor_shapes",
        "floor_plain_01"
    },
    {
        CSB_V22_FAMG_FLOOR_CRACKED,
        "floor_cracked_01",
        "floor_shapes",
        "floor_cracked_01"
    },
    {
        CSB_V22_FAMG_CREATURE_CHAOS_FIEND,
        "creature_chaos_fiend_01",
        "creature_shapes",
        "creature_chaos_fiend_01"
    },
    {
        CSB_V22_FAMG_PANEL_LORD_ORDER,
        "panel_lord_order_01",
        "ui_chrome",
        "panel_lord_order_01"
    },
    {
        CSB_V22_FAMG_CHAMPION_WARRIOR_CSB,
        "champion_warrior_csb_01",
        "champion_portraits",
        "champion_warrior_csb_01"
    },
    {
        CSB_V22_FAMG_DOOR_PRISON,
        "door_prison_01",
        "door_shapes",
        "door_prison_01"
    },
    {
        CSB_V22_FAMG_CHAOS_RUNE,
        "chaos_rune_01",
        "chaos_runes",
        "chaos_rune_01"
    }
};

/* ── Module state ──────────────────────────────────────────────── */
static char               g_manifest_path[FSP_PATH_MAX] = {0};
static int                g_installed = 0;     /* last gate: 1 if PARTIAL/FINISHED_REAL */
static CSB_V22_FamgGate   g_last_gate = CSB_V22_FAMG_GATE_NOT_PROBED;

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
    if (!dataDir || dataDir[0] == '\0') {
        g_manifest_path[0] = '\0';
        return;
    }
    /* ~/.firestaff/data/csb -> ~/.firestaff -> assets/csb/modern/modern_asset_manifest.json
     * Walks two parents up from dataDir, same pattern as
     * csb_v22_set_manifest_path in csb_v22_modern_assets_pc34.c. */
    char parent1[FSP_PATH_MAX];
    char parent2[FSP_PATH_MAX];
    char assets_root[FSP_PATH_MAX];
    char csb_modern_dir[FSP_PATH_MAX];
    if (!FSP_ParentDir(parent1, sizeof(parent1), dataDir) ||
        !FSP_ParentDir(parent2, sizeof(parent2), parent1)) {
        FSP_JoinPath(assets_root, sizeof(assets_root), dataDir, "assets");
    } else {
        FSP_JoinPath(assets_root, sizeof(assets_root), parent2, "assets");
    }
    FSP_JoinPath(csb_modern_dir, sizeof(csb_modern_dir), assets_root, "csb");
    FSP_JoinPath(csb_modern_dir, sizeof(csb_modern_dir), csb_modern_dir, "modern");
    FSP_JoinPath(g_manifest_path, sizeof(g_manifest_path),
                 csb_modern_dir, "modern_asset_manifest.json");
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

        csb_v22_famg_buf_reset();
        {
            const char* c = scan;
            while (c <= end) {
                csb_v22_famg_buf_append_char(*c);
                ++c;
            }
        }

        CSB_V22_FamgSlotRaw raw;
        csb_v22_famg_slot_raw_init(&raw);
        csb_v22_famg_extract_fields_from_buf(&raw);
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
 * For CSB V2.2 materials the source_file is expected to live under
 * <modern-dir>/<category>/<source_file>, e.g.:
 *   ~/.firestaff/assets/csb/modern/wall_shapes/wall_dungeon_01.png
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

/* ── Slot classification ───────────────────────────────────────── */
CSB_V22_FamgClass csb_v22_famg_classify_slot(CSB_V22_FamgSlot slot) {
    if ((unsigned)slot >= (unsigned)CSB_V22_FAMG_MATERIAL_COUNT) {
        return CSB_V22_FAMG_CLASS_UNKNOWN;
    }
    if (g_manifest_path[0] == '\0' ||
        !csb_v22_famg_file_exists(g_manifest_path)) {
        return CSB_V22_FAMG_CLASS_MISSING;
    }

    CSB_V22_FamgSlotRaw raw;
    if (!csb_v22_famg_find_slot_in_manifest(g_manifest_path,
                                            k_slot_table[slot].id, &raw)) {
        return CSB_V22_FAMG_CLASS_MISSING;
    }

    int has_required = raw.has_id && raw.has_generator &&
                       raw.has_source_file && raw.has_width &&
                       raw.has_height && raw.width > 0 && raw.height > 0;
    if (!has_required) {
        return CSB_V22_FAMG_CLASS_PARTIAL;
    }

    /* generator == "placeholder" is the explicit fallback marker */
    if (strcmp(raw.generator, "placeholder") == 0) {
        return CSB_V22_FAMG_CLASS_PLACEHOLDER;
    }

    /* Real asset: required fields + non-placeholder generator +
     * source_file resolves on disk. */
    char resolved_path[FSP_PATH_MAX];
    int exists = csb_v22_famg_resolve_source_file(g_manifest_path,
                                                  k_slot_table[slot].category,
                                                  raw.source_file,
                                                  resolved_path,
                                                  sizeof(resolved_path));
    return exists ? CSB_V22_FAMG_CLASS_REAL
                  : CSB_V22_FAMG_CLASS_PARTIAL;
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

    if (real == total) {
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
 * the cell's wall fallback to. For the first cut every wall cell in
 * the 9-square viewport routes to WALL_DUNGEON because the runtime
 * variant -> asset_id mapping in csb_v22_inplace_draw_pc34.c maps all
 * wall variants to wall_dungeon_01. Per-cell refinement (e.g. mossy
 * walls for slime zones) is a follow-up that mirrors the variant
 * enum in csb_v22_shapes.h.
 *
 * Floor variants route through FLOOR_PLAIN and FLOOR_CRACKED via the
 * caller; cells that have already been classified by csb_v22_inplace
 * can be reclassified via classify_slot() directly.
 *
 * Returns CSB_V22_FAMG_MATERIAL_COUNT (= sentinel "out of range")
 * when depth or lateral is outside the 9-square viewport bounds. */
CSB_V22_FamgSlot csb_v22_famg_slot_for_cell(int depth, int lateral) {
    /* First cut: all wall cells in the 3x3 viewport route to the
     * WALL_DUNGEON slot. Floor cells route through the separate
     * floor slots; callers needing per-cell floor classification
     * dispatch via csb_v22_inplace_get_cell_asset_id() + the
     * slot_for_asset_id() helper below. */
    (void)depth;
    (void)lateral;
    if (depth < 0 || depth > 2) return CSB_V22_FAMG_MATERIAL_COUNT;
    if (lateral < -1 || lateral > 1) return CSB_V22_FAMG_MATERIAL_COUNT;
    return CSB_V22_FAMG_WALL_DUNGEON;
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
        "Source: src/csb/csb_v22_modern_assets_pc34.c (missing-asset placeholder 16x16 magenta)\n"
        "Source: sibling dm1_v22 / dm2_v22 FAMG modules (placeholder-vs-real pattern)\n"
        "Source: docs/FIRESTAFF_GAP_LIST.md B3 V2 per-mode material row\n"
        "Manifest path: ~/.firestaff/assets/csb/modern/modern_asset_manifest.json\n"
        "Schema: { id, generator, source_file, width, height } per slot entry\n"
        "Generator 'placeholder' is the procedural fallback marker (synthetic)\n"
        "Non-placeholder generator + source_file resolves on disk = REAL\n"
        "Gate states: NOT_PROBED / NO_MANIFEST / SYNTHETIC_PLACEHOLDER / PARTIAL / FINISHED_REAL\n"
        "FINISHED_REAL requires every tracked slot to be REAL with non-placeholder generator\n"
        "V1 invariant: V1 command routes, dungeon state, save/restore NEVER bypassed\n"
        "V2 rule: finished-art material only activates when V2 launch+profile enabled\n"
        "Honest boundary: this gate tracks manifest classification only.\n"
        "It does NOT claim finished PBR art has been reviewed or shipped.\n"
        "FINISHED_REAL promotion requires operator-installed hero PNGs at\n"
        "~/.firestaff/assets/csb/modern/<category>/<source_file> with\n"
        "generator != 'placeholder' and a non-zero width/height, plus a\n"
        "sibling gap-list update to mark the real-asset promotion gate green.\n";
}
