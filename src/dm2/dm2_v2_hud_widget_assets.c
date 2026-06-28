/*
 * dm2_v2_hud_widget_assets.c — DM2 V2 HUD Widget Asset Manifest + Gate
 *
 * Implementation. Companion to include/dm2_v2_hud_widget_assets.h.
 *
 * The manifest schema is intentionally minimal and aligned with the
 * existing dm2_v22_modern_assets_pc34 shape (id + source_file + width
 * + height), with one extra field:
 *
 *   "generator": "placeholder" | "pbr_hero" | "ai_upscale" | ...
 *
 * When generator == "placeholder" the slot is treated as the procedural
 * rectangle/letter fallback already drawn by dm2_v2_hud_overlay.c — this
 * is the honest current default for every widget. Real PNG/PBR slots
 * must declare generator != "placeholder" and a source_file that
 * resolves on disk via FSP_JoinPath against the manifest's directory.
 *
 * The module is fully data-free: no game assets are required for the
 * gate to be useful. When the manifest is absent, the gate reports
 * NO_MANIFEST, every slot classifies as MISSING, and
 * uses_placeholder() returns 1 for every slot — exactly matching the
 * runtime's current behaviour.
 *
 * Source:
 *   - SKULL.ASM T560 (DM2 HUD rendering pipeline)
 *   - skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *   - ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *   - include/dm2_v22_modern_assets_pc34.h (sibling V2.2 manifest pattern)
 *   - src/dm2/dm2_v2_hud_overlay.c (runtime drawing fallback paths)
 */

#include "dm2_v2_hud_widget_assets.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Slot table (stable, ordered) ──────────────────────────────── */
typedef struct {
    DM2_V2_HudWidgetSlot slot;
    const char          *id;
    const char          *category;
} DM2_V2_HudWidgetSlotDesc;

static const DM2_V2_HudWidgetSlotDesc k_slot_table[DM2_V2_HUD_WIDGET_COUNT] = {
    { DM2_V2_HUD_WIDGET_INVENTORY_QUICK_VIEW, "inventory_quick_view", "hud_widgets" },
    { DM2_V2_HUD_WIDGET_ACTION_PROMPT,        "action_prompt",        "hud_widgets" },
    { DM2_V2_HUD_WIDGET_COMPASS_ROSE,         "compass_rose",         "hud_chrome"  },
    { DM2_V2_HUD_WIDGET_DEPTH_INDICATOR,      "depth_indicator",      "hud_chrome"  },
    { DM2_V2_HUD_WIDGET_GOLD_COUNTER,         "gold_counter",         "hud_chrome"  },
    { DM2_V2_HUD_WIDGET_CHAMPION_BAR_FRAME,   "champion_bar_frame",   "hud_chrome"  },
    { DM2_V2_HUD_WIDGET_ACTION_STRIP_FRAME,   "action_strip_frame",   "hud_chrome"  }
};

/* ── Module state ──────────────────────────────────────────────── */
static char     g_manifest_path[FSP_PATH_MAX] = {0};
static int      g_installed = 0;  /* last gate classification: 1 if PARTIAL/COMPLETE */
static DM2_V2_HudWidgetGate g_last_gate = DM2_V2_HUD_WIDGET_GATE_NOT_PROBED;

/* ── Trimming / JSON helpers (parallels dm2_v22_modern_assets_pc34) ── */
static void dm2_v2_hwa_trim(char* dst, const char* src, size_t dstSize) {
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

static int dm2_v2_hwa_file_exists(const char* path) {
    if (!path || path[0] == '\0') return 0;
    FILE* fp = fopen(path, "rb");
    if (fp) { fclose(fp); return 1; }
    return 0;
}

/* Forward decl: raw slot record defined further below. */
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
} DM2_V2_HwaSlotRaw;

/* ── Entry-content accumulator ────────────────────────────────────
 *
 * We accumulate the entry's raw JSON text as we walk braces, then
 * extract fields from the buffer when the entry closes. This handles
 * three tricky layouts the line-naive approach misses:
 *   1. Single-line JSON (entry spans one line).
 *   2. Fields on the same line as the closing `}` (so post-brace-walk
 *      field extraction would see in_entry=0 and skip).
 *   3. Pretty-printed multi-line entries whose `{` and `}` live on
 *      lines different from the field lines. */
static char g_entry_buf[16384];
static int  g_entry_buf_len = 0;

static void dm2_v2_hwa_buf_reset(void) {
    g_entry_buf_len = 0;
    if (g_entry_buf_len < (int)sizeof(g_entry_buf)) {
        g_entry_buf[g_entry_buf_len] = '\0';
    }
}

static void dm2_v2_hwa_buf_append_char(char c) {
    if (g_entry_buf_len + 1 < (int)sizeof(g_entry_buf)) {
        g_entry_buf[g_entry_buf_len++] = c;
        g_entry_buf[g_entry_buf_len] = '\0';
    }
}

/* Forward decls for the field extractors defined further below. */
static int dm2_v2_hwa_extract_string(const char* line, const char* key,
                                      char* out, size_t outSize);
static int dm2_v2_hwa_extract_int(const char* line, const char* key,
                                   int* out);

static void dm2_v2_hwa_extract_fields_from_buf(DM2_V2_HwaSlotRaw* out) {
    char val[256];
    if (dm2_v2_hwa_extract_string(g_entry_buf, "id", val, sizeof(val))) {
        dm2_v2_hwa_trim(out->id, val, sizeof(out->id));
        out->has_id = 1;
    }
    if (dm2_v2_hwa_extract_string(g_entry_buf, "generator", val, sizeof(val))) {
        dm2_v2_hwa_trim(out->generator, val, sizeof(out->generator));
        out->has_generator = 1;
    }
    if (dm2_v2_hwa_extract_string(g_entry_buf, "source_file", val, sizeof(val))) {
        dm2_v2_hwa_trim(out->source_file, val, sizeof(out->source_file));
        out->has_source_file = 1;
    }
    int w = 0, h = 0;
    if (dm2_v2_hwa_extract_int(g_entry_buf, "width", &w)) {
        out->width = w;
        out->has_width = 1;
    }
    if (dm2_v2_hwa_extract_int(g_entry_buf, "height", &h)) {
        out->height = h;
        out->has_height = 1;
    }
}

/* Extract a JSON string value for a key from a single-line chunk.
 * Format:  "key": "value"
 * Returns 1 on success, 0 if not found. */
static int dm2_v2_hwa_extract_string(const char* line, const char* key,
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

/* Extract an integer value for a key. Returns 1 on success, 0 if not
 * found. Matches "key": 1234 (positive int). */
static int dm2_v2_hwa_extract_int(const char* line, const char* key,
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

/* ── Path resolution ───────────────────────────────────────────── */
void dm2_v2_hud_widget_assets_set_manifest_path(const char* dataDir) {
    if (!dataDir || dataDir[0] == '\0') {
        g_manifest_path[0] = '\0';
        return;
    }
    /* ~/.firestaff/data/dm2 -> ~/.firestaff -> assets/dm2/hud/hud_widget_manifest.json
     * Walks two parents up from dataDir, same pattern as
     * dm2_v22_set_manifest_path. */
    char parent1[FSP_PATH_MAX];
    char parent2[FSP_PATH_MAX];
    char assets_root[FSP_PATH_MAX];
    char dm2_hud_dir[FSP_PATH_MAX];
    if (!FSP_ParentDir(parent1, sizeof(parent1), dataDir) ||
        !FSP_ParentDir(parent2, sizeof(parent2), parent1)) {
        FSP_JoinPath(assets_root, sizeof(assets_root), dataDir, "assets");
    } else {
        FSP_JoinPath(assets_root, sizeof(assets_root), parent2, "assets");
    }
    FSP_JoinPath(dm2_hud_dir, sizeof(dm2_hud_dir), assets_root, "dm2");
    FSP_JoinPath(dm2_hud_dir, sizeof(dm2_hud_dir), dm2_hud_dir, "hud");
    FSP_JoinPath(g_manifest_path, sizeof(g_manifest_path),
                 dm2_hud_dir, "hud_widget_manifest.json");
}

const char* dm2_v2_hud_widget_assets_get_manifest_path(void) {
    return g_manifest_path;
}

/* ── Internal manifest scan ────────────────────────────────────── */

/* Internal: parse the manifest and populate `out` for the slot whose
 * id matches k_slot_table[slot].id. Returns 1 if a matching entry was
 * found and its fields extracted, 0 if absent or malformed.
 *
 * The parser is intentionally line-naive: the manifest can either be a
 * single physical line (all entries inline) or a multi-line indented
 * JSON object per entry. We scan for any "id" field value matching
 * the slot's id, then read the surrounding entry's required fields.
 *
 * Required fields for a slot to be non-PARTIAL:
 *   id            (string)
 *   generator     (string)
 *   source_file   (string)
 *   width         (int > 0)
 *   height        (int > 0)
 *
 * If any are missing, classification is PARTIAL (real metadata but
 * incomplete). */
/* DM2_V2_HwaSlotRaw defined earlier in the file (forward-declared so
 * the buffer accumulator helpers can use it). The actual definition
 * lives next to the parser so we can colocate related code. */
static void dm2_v2_hwa_slot_raw_init(DM2_V2_HwaSlotRaw* r) {
    memset(r, 0, sizeof(*r));
}

static int dm2_v2_hwa_find_slot_in_manifest(const char* manifest_path,
                                             const char* slot_id,
                                             DM2_V2_HwaSlotRaw* out) {
    if (!manifest_path || manifest_path[0] == '\0' || !slot_id || !out) return 0;
    dm2_v2_hwa_slot_raw_init(out);

    FILE* fp = fopen(manifest_path, "rb");
    if (!fp) return 0;

    char line[4096];
    int  in_array = 0;   /* set once we've seen the hud_widgets [ */
    int  brace_depth = 0;/* nested-object depth inside the array  */
    int  in_entry = 0;   /* true between { and matching } in array */

    /* Single-pass scanner. We track when we're inside the array and
     * use brace_depth to find entry boundaries. Entry text is
     * accumulated into g_entry_buf char-by-char so layout (single-line
     * vs pretty-printed, fields-on-close-line) is irrelevant.
     *
     * Important: we do NOT treat the outer manifest `{` as an entry.
     * The outer `{` is before in_array is set, so the entry-open
     * condition (in_array && brace_depth==0) cannot match it. Only
     * `{` characters that appear AFTER the hud_widgets `[` are
     * detected as entry openings. */
    while (fgets(line, sizeof(line), fp)) {
        for (const char* c = line; *c; ++c) {
            if (*c == '[') {
                if (!in_array) {
                    in_array = 1;
                    brace_depth = 0;
                }
                if (in_entry) {
                    dm2_v2_hwa_buf_append_char('[');
                }
            } else if (*c == ']') {
                if (in_entry) {
                    dm2_v2_hwa_buf_append_char(']');
                }
                if (brace_depth == 0 && in_array) {
                    in_array = 0;
                }
            } else if (*c == '{') {
                if (in_array && brace_depth == 0) {
                    /* Open new widget entry */
                    dm2_v2_hwa_buf_reset();
                    in_entry = 1;
                    brace_depth = 1;
                    dm2_v2_hwa_buf_append_char('{');
                } else {
                    /* Either pre-array outer brace, or a nested object
                     * inside the current entry. Don't open a new entry,
                     * just track depth and append to current buffer. */
                    brace_depth++;
                    if (in_entry) {
                        dm2_v2_hwa_buf_append_char('{');
                    }
                }
            } else if (*c == '}') {
                if (brace_depth > 0) brace_depth--;
                if (in_entry) {
                    dm2_v2_hwa_buf_append_char('}');
                    if (brace_depth == 0) {
                        /* End of entry */
                        in_entry = 0;
                        DM2_V2_HwaSlotRaw raw;
                        dm2_v2_hwa_slot_raw_init(&raw);
                        dm2_v2_hwa_extract_fields_from_buf(&raw);
                        if (raw.has_id && strcmp(raw.id, slot_id) == 0) {
                            *out = raw;
                            fclose(fp);
                            return 1;
                        }
                    }
                }
            } else {
                if (in_entry) {
                    dm2_v2_hwa_buf_append_char(*c);
                }
            }
        }
    }

    fclose(fp);
    return 0;
}

/* Resolve a manifest source_file (relative) against the manifest dir.
 * Returns 1 if the resolved path exists on disk, 0 otherwise.
 *
 * For widgets the source_file is expected to live under
 * <hud-dir>/<category>/<source_file>, e.g.:
 *   ~/.firestaff/assets/dm2/hud/hud_widgets/inventory_quick_view.png
 */
static int dm2_v2_hwa_resolve_source_file(const char* manifest_path,
                                           const char* category,
                                           const char* source_file,
                                           char* out, size_t outSize) {
    if (!manifest_path || !category || !source_file ||
        source_file[0] == '\0' || !out || outSize == 0U) {
        if (out && outSize > 0U) out[0] = '\0';
        return 0;
    }

    /* Get the manifest's parent directory */
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
    return dm2_v2_hwa_file_exists(joined);
}

/* ── Validation ────────────────────────────────────────────────── */
int dm2_v2_hud_widget_assets_validate_manifest(const char* manifest_path) {
    const char* p = manifest_path ? manifest_path : g_manifest_path;
    if (p[0] == '\0') return -1;
    if (!dm2_v2_hwa_file_exists(p)) return -1;

    /* Validate at least one declared slot has the required fields.
     * We probe the first slot in the table to keep the scan O(1). */
    const char* first_id = k_slot_table[0].id;
    DM2_V2_HwaSlotRaw raw;
    if (!dm2_v2_hwa_find_slot_in_manifest(p, first_id, &raw)) {
        return 0; /* manifest readable but no slot declared */
    }
    int complete = raw.has_id && raw.has_generator && raw.has_source_file &&
                   raw.has_width && raw.has_height &&
                   raw.width > 0 && raw.height > 0;
    return complete ? 1 : 0;
}

/* ── Slot classification ───────────────────────────────────────── */
DM2_V2_HudWidgetClass dm2_v2_hud_widget_assets_classify_slot(
    DM2_V2_HudWidgetSlot slot) {

    if ((unsigned)slot >= (unsigned)DM2_V2_HUD_WIDGET_COUNT) {
        return DM2_V2_HUD_WIDGET_CLASS_UNKNOWN;
    }
    if (g_manifest_path[0] == '\0' ||
        !dm2_v2_hwa_file_exists(g_manifest_path)) {
        return DM2_V2_HUD_WIDGET_CLASS_MISSING;
    }

    DM2_V2_HwaSlotRaw raw;
    if (!dm2_v2_hwa_find_slot_in_manifest(g_manifest_path,
                                            k_slot_table[slot].id, &raw)) {
        return DM2_V2_HUD_WIDGET_CLASS_MISSING;
    }

    /* All required fields present? */
    int has_required = raw.has_id && raw.has_generator &&
                       raw.has_source_file && raw.has_width &&
                       raw.has_height && raw.width > 0 && raw.height > 0;
    if (!has_required) {
        return DM2_V2_HUD_WIDGET_CLASS_PARTIAL;
    }

    /* Placeholder generator is an explicit fallback marker */
    if (strcmp(raw.generator, "placeholder") == 0) {
        return DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER;
    }

    /* Real asset: required fields + non-placeholder generator */
    char resolved_path[FSP_PATH_MAX];
    int exists = dm2_v2_hwa_resolve_source_file(g_manifest_path,
                                                  k_slot_table[slot].category,
                                                  raw.source_file,
                                                  resolved_path,
                                                  sizeof(resolved_path));
    return exists ? DM2_V2_HUD_WIDGET_CLASS_REAL
                  : DM2_V2_HUD_WIDGET_CLASS_PARTIAL;
}

int dm2_v2_hud_widget_assets_get_slot_info(DM2_V2_HudWidgetSlot slot,
                                            DM2_V2_HudWidgetSlotInfo* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if ((unsigned)slot >= (unsigned)DM2_V2_HUD_WIDGET_COUNT) return 0;

    out->slot = slot;
    snprintf(out->id,       sizeof(out->id),       "%s", k_slot_table[slot].id);
    snprintf(out->category, sizeof(out->category), "%s", k_slot_table[slot].category);
    out->classification = dm2_v2_hud_widget_assets_classify_slot(slot);

    if (g_manifest_path[0] == '\0' ||
        !dm2_v2_hwa_file_exists(g_manifest_path)) {
        return 0;
    }
    DM2_V2_HwaSlotRaw raw;
    if (!dm2_v2_hwa_find_slot_in_manifest(g_manifest_path,
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

    /* Resolve source_file against the manifest's directory when possible.
     * If the resolution fails (manifest-declared but file missing),
     * file_exists stays 0 and the classification above already records
     * PARTIAL. */
    if (raw.has_source_file && raw.source_file[0] != '\0') {
        int exists = dm2_v2_hwa_resolve_source_file(g_manifest_path,
                                                      k_slot_table[slot].category,
                                                      raw.source_file,
                                                      out->resolved_path,
                                                      sizeof(out->resolved_path));
        out->file_exists = exists ? 1 : 0;
    }
    return 1;
}

int dm2_v2_hud_widget_assets_real_count(int* out_total) {
    int real = 0;
    int total = 0;
    if (g_manifest_path[0] != '\0' &&
        dm2_v2_hwa_file_exists(g_manifest_path)) {
        for (size_t i = 0; i < DM2_V2_HUD_WIDGET_COUNT; ++i) {
            DM2_V2_HudWidgetClass cls =
                dm2_v2_hud_widget_assets_classify_slot(
                    (DM2_V2_HudWidgetSlot)i);
            if (cls != DM2_V2_HUD_WIDGET_CLASS_MISSING) {
                ++total;
                if (cls == DM2_V2_HUD_WIDGET_CLASS_REAL) ++real;
            }
        }
    }
    if (out_total) *out_total = total;
    return real;
}

DM2_V2_HudWidgetGate dm2_v2_hud_widget_assets_gate(void) {
    int total = 0;
    int real  = dm2_v2_hud_widget_assets_real_count(&total);

    if (g_manifest_path[0] == '\0' ||
        !dm2_v2_hwa_file_exists(g_manifest_path)) {
        g_last_gate = DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST;
        g_installed = 0;
        return g_last_gate;
    }

    int manifest_valid = dm2_v2_hud_widget_assets_validate_manifest(NULL);
    if (manifest_valid < 0) {
        g_last_gate = DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST;
        g_installed = 0;
        return g_last_gate;
    }

    if (total == 0) {
        /* Manifest present but no slot entries declared — still
         * placeholder-mode default; record as PLACEHOLDER gate rather
         * than NO_MANIFEST (operator can still write a manifest and
         * promote the gate). */
        g_last_gate = DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER;
        g_installed = 0;
        return g_last_gate;
    }

    if (real == total) {
        g_last_gate = DM2_V2_HUD_WIDGET_GATE_COMPLETE;
        g_installed = 1;
    } else if (real > 0) {
        g_last_gate = DM2_V2_HUD_WIDGET_GATE_PARTIAL;
        g_installed = 1;
    } else {
        g_last_gate = DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER;
        g_installed = 0;
    }
    return g_last_gate;
}

/* ── Names ─────────────────────────────────────────────────────── */
const char* dm2_v2_hud_widget_assets_slot_name(DM2_V2_HudWidgetSlot slot) {
    if ((unsigned)slot >= (unsigned)DM2_V2_HUD_WIDGET_COUNT) return "UNKNOWN";
    return k_slot_table[slot].id;
}

const char* dm2_v2_hud_widget_assets_class_name(DM2_V2_HudWidgetClass cls) {
    switch (cls) {
        case DM2_V2_HUD_WIDGET_CLASS_UNKNOWN:     return "UNKNOWN";
        case DM2_V2_HUD_WIDGET_CLASS_MISSING:     return "MISSING";
        case DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER: return "PLACEHOLDER";
        case DM2_V2_HUD_WIDGET_CLASS_PARTIAL:     return "PARTIAL";
        case DM2_V2_HUD_WIDGET_CLASS_REAL:        return "REAL";
        default: return "INVALID";
    }
}

const char* dm2_v2_hud_widget_assets_gate_name(DM2_V2_HudWidgetGate gate) {
    switch (gate) {
        case DM2_V2_HUD_WIDGET_GATE_NOT_PROBED:  return "NOT_PROBED";
        case DM2_V2_HUD_WIDGET_GATE_NO_MANIFEST: return "NO_MANIFEST";
        case DM2_V2_HUD_WIDGET_GATE_PLACEHOLDER: return "PLACEHOLDER";
        case DM2_V2_HUD_WIDGET_GATE_PARTIAL:     return "PARTIAL";
        case DM2_V2_HUD_WIDGET_GATE_COMPLETE:    return "COMPLETE";
        default: return "INVALID";
    }
}

/* ── Installed flag ────────────────────────────────────────────── */
void dm2_v2_hud_widget_assets_set_installed(int installed) {
    g_installed = installed ? 1 : 0;
}

int dm2_v2_hud_widget_assets_get_installed(void) {
    return g_installed;
}

int dm2_v2_hud_widget_assets_uses_placeholder(DM2_V2_HudWidgetSlot slot) {
    DM2_V2_HudWidgetClass cls = dm2_v2_hud_widget_assets_classify_slot(slot);
    switch (cls) {
        case DM2_V2_HUD_WIDGET_CLASS_REAL:
            return 0;
        case DM2_V2_HUD_WIDGET_CLASS_PLACEHOLDER:
        case DM2_V2_HUD_WIDGET_CLASS_PARTIAL:
        case DM2_V2_HUD_WIDGET_CLASS_MISSING:
        case DM2_V2_HUD_WIDGET_CLASS_UNKNOWN:
        default:
            return 1;
    }
}

const char* dm2_v2_hud_widget_assets_source_evidence(void) {
    return
        "DM2 V2 HUD Widget Asset Manifest — Phase 3 placeholder-vs-real gate\n"
        "Source: SKULL.ASM T560              (DM2 HUD rendering pipeline)\n"
        "Source: skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)\n"
        "Source: ReDMCSB PANEL.C F0354       (champion status-box drawing)\n"
        "Source: ReDMCSB DUNGEON.C F0260     (stat-bar refresh timing)\n"
        "Source: src/dm2/dm2_v2_hud_overlay.c (procedural rendering fallback)\n"
        "Source: include/dm2_v22_modern_assets_pc34.h (sibling V2.2 manifest pattern)\n"
        "Source: docs/FIRESTAFF_GAP_LIST.md D2 V2 Phase 3 row (widget bitmap gap)\n"
        "Source: examples/dm2_hud_widget_synthetic/README.md (synthetic-test fixture)\n"
        "Manifest path: ~/.firestaff/assets/dm2/hud/hud_widget_manifest.json\n"
        "Schema: { id, generator, source_file, width, height } per slot entry\n"
        "Generator 'placeholder' is the procedural fallback marker\n"
        "Non-placeholder generator + source_file resolves on disk = REAL\n"
        "Synthetic-test example: examples/dm2_hud_widget_synthetic/ uses\n"
        "generator 'synthetic_test' + 1x1 procedural PNG fixtures only to\n"
        "exercise PARTIAL/COMPLETE gates in scratch directories; it is NOT\n"
        "operator-installable finished art and must not be used for public\n"
        "visual claims.\n"
        "V1 invariant: V1 command routes, inventory, dungeon state NEVER bypassed\n"
        "V2 rule: HUD widget assets only activate when V2 launch+profile enabled\n"
        "Honest boundary: this gate tracks asset classification; it does NOT\n"
        "claim finished PBR widget art. Real-art promotion requires\n"
        "operator-installed source_file entries with generator != 'placeholder'\n"
        "and disk-resolvable paths, and a sibling gap-list update.\n";
}
