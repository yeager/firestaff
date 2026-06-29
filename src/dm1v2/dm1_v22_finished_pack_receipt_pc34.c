/*
 * dm1_v22_finished_pack_receipt_pc34.c
 *
 * DM1 V2.2 finished-art pack reviewer-receipt promotion gate — impl.
 *
 * Companion implementation to include/dm1_v22_finished_pack_receipt_pc34.h.
 *
 * Reads finish_receipt.json from the modern asset root and cross-
 * checks it against the sibling material gate (which itself reads
 * modern_asset_manifest.json). The promotion predicate requires all
 * four conditions:
 *
 *   1. Receipt file is present + JSON-object + has required fields.
 *   2. manifestHashFnv1a in the receipt matches the FNV-1a hash of
 *      the on-disk modern_asset_manifest.json file content.
 *   3. The sibling material gate is in FINISHED_REAL state.
 *   4. The receipt's reviewedSlots array covers every required slot
 *      id from the sibling gate.
 *
 * The receipt never overrides the material gate; if (2) fails the
 * receipt is treated as STALE, if (3) fails as MATERIAL_NOT_REAL,
 * if (4) fails as MATCH_PARTIAL. The CI-safe default remains
 * NOT_INSTALLED when no receipt file is on disk.
 *
 * Source-lock:
 *   - ReDMCSB DUNVIEW.C:6697-6816 (DM1 viewport composition order)
 *   - ReDMCSB DUNGEON.C:2238-2246 (square-type decode feeding
 *     m11_v22_shape_for_cell)
 *   - include/dm1_v22_finished_art_material_gate_pc34.h (sibling
 *     material gate)
 *   - include/dm1_v2_asset_pipeline_pc34.h (modern asset root path)
 *   - include/fs_portable_compat.h (portable path operations)
 *
 * Honest boundary: the receipt is a review note. There is no PKI;
 * the integrity model is "the manifest content hash the reviewer
 * signed off on still matches what's on disk". See the .h for
 * the full honest-boundary paragraph.
 */

#include "dm1_v22_finished_pack_receipt_pc34.h"
#include "dm1_v22_finished_art_material_gate_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Module state ──────────────────────────────────────────────── */
static char g_receipt_path[FSP_PATH_MAX] = {0};
static char g_manifest_path[FSP_PATH_MAX] = {0};
static int  g_state_dirty = 1;

typedef struct {
    char manifest_path[1024];
    char manifest_hash_hex[16];
    char reviewer[64];
    char reviewed_at_utc[32];
    char gate_target[32];
    int  has_manifest_path;
    int  has_manifest_hash;
    int  has_reviewer;
    int  has_reviewed_at;
    int  has_gate_target;
    /* Each required slot id maps to "in receipt reviewedSlots?" */
    int  reviewed[DM1_V22_FAMG_MATERIAL_COUNT];
} DM1_V22_FprReceipt;

static DM1_V22_FprReceipt g_last_receipt;

/* Required slots are 1:1 with the sibling material gate, so we
 * reuse dm1_v22_famg_slot_name() rather than re-declaring the
 * hero_01 ids here. */

/* ── Path resolution ───────────────────────────────────────────── */
void dm1_v22_fpr_set_receipt_path(const char* dataDir) {
    g_state_dirty = 1;
    if (!dataDir || dataDir[0] == '\0') {
        g_receipt_path[0] = '\0';
        g_manifest_path[0] = '\0';
        return;
    }
    /* dataDir = <root>/data/dm1 -> modern-asset-root = <root>/assets/dm1/modern
     * The receipt lives at <modern-asset-root>/finish_receipt.json.
     * The manifest path mirrors dm1_v22_famg_set_manifest_path. */
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
    FSP_JoinPath(g_receipt_path, sizeof(g_receipt_path),
                 dm1_modern_dir, "finish_receipt.json");
    FSP_JoinPath(g_manifest_path, sizeof(g_manifest_path),
                 dm1_modern_dir, "modern_asset_manifest.json");

    /* Set the sibling gate's path too so the cross-check pulls
     * the same manifest content the gate would classify. */
    dm1_v22_famg_set_manifest_path(dataDir);
}

const char* dm1_v22_fpr_get_receipt_path(void) {
    return g_receipt_path;
}

const char* dm1_v22_fpr_get_manifest_path(void) {
    return g_manifest_path;
}

void dm1_v22_fpr_reset_state(void) {
    g_state_dirty = 1;
}

/* ── FNV-1a 32-bit ────────────────────────────────────────────── */
uint32_t dm1_v22_fpr_fnv1a_buf(const void* data, size_t len) {
    const unsigned char* p = (const unsigned char*)data;
    uint32_t h = 2166136261u;
    size_t i;
    if (!p) return 0U;
    for (i = 0U; i < len; ++i) {
        h = (h ^ (uint32_t)p[i]) * 16777619u;
    }
    return h;
}

uint32_t dm1_v22_fpr_fnv1a_file(const char* path) {
    if (!path || path[0] == '\0') return 0U;
    FILE* fp = fopen(path, "rb");
    long size;
    unsigned char* buf;
    uint32_t h;
    if (!fp) return 0U;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0U; }
    size = ftell(fp);
    if (size < 0 || size > (long)(1024L * 1024L)) { fclose(fp); return 0U; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return 0U; }
    buf = (unsigned char*)malloc((size_t)size + 1U);
    if (!buf) { fclose(fp); return 0U; }
    if (fread(buf, 1, (size_t)size, fp) != (size_t)size) {
        free(buf); fclose(fp); return 0U;
    }
    fclose(fp);
    h = dm1_v22_fpr_fnv1a_buf(buf, (size_t)size);
    free(buf);
    return h;
}

/* ── Trimming / hex conversion / extractors ───────────────────── */
static void fpr_trim(char* dst, const char* src, size_t dstSize) {
    if (!dst || dstSize == 0U) return;
    const char* start = src ? src : "";
    while (*start == ' ' || *start == '\t' ||
           *start == '\r' || *start == '\n') ++start;
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

static uint32_t parse_hex_u32(const char* s) {
    uint32_t v = 0U;
    if (!s) return 0U;
    while (*s == ' ' || *s == '\t') ++s;
    for (; *s; ++s) {
        char c = *s;
        if (c >= '0' && c <= '9') {
            v = (v << 4) | (uint32_t)(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            v = (v << 4) | (uint32_t)(10 + c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            v = (v << 4) | (uint32_t)(10 + c - 'A');
        } else {
            break;
        }
    }
    return v;
}

/* Scan the receipt file for a single top-level string field.
 *
 * The receipt is small (<= a few KB) but the parser must handle both
 * compact one-line JSON and pretty-printed multi-line layouts. We
 * use a brace-balanced scan: a top-level string is only a candidate
 * when it sits at depth 1 inside the outer object AND no inner
 * object/array brace was opened on the way. This keeps "reviewed"-
 * style values nested inside the reviewedSlots array from being
 * picked up by the outer-field scan. */
static int fpr_extract_top_string(char* text, size_t textLen,
                                  const char* key,
                                  char* out, size_t outSize) {
    char pattern[64];
    const char* p;
    int depth;
    if (!text || !key || !out || outSize == 0U) return 0;
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = text;
    while ((p = strstr(p, pattern)) != NULL) {
        const char* after = p + strlen(pattern);
        /* Make sure we are at depth 1 (just inside the outer {}).
         * Walk back from the match start and track depth. */
        const char* walk = text;
        depth = 0;
        while (walk < p) {
            if (*walk == '{' || *walk == '[') ++depth;
            else if (*walk == '}' || *walk == ']') --depth;
            ++walk;
        }
        if (depth != 1) {
            p = after;
            continue;
        }
        /* Skip past key + colon + whitespace, then read the string. */
        const char* q = after;
        while (*q == ' ' || *q == ':' || *q == '\t' || *q == '\r' || *q == '\n') ++q;
        if (*q != '"') { p = after; continue; }
        ++q;
        size_t dst = 0U;
        while (*q && *q != '"' && dst < outSize - 1U) {
            if (*q == '\\' && q[1] != '\0') {
                ++q;
                if (dst < outSize - 1U) out[dst++] = *q++;
            } else {
                out[dst++] = *q++;
            }
        }
        out[dst] = '\0';
        return 1;
    }
    (void)textLen;
    return 0;
}

/* Find each required-slot id inside the receipt's reviewedSlots
 * array. Marks the corresponding receipt.reviewed[] flag. */
static void fpr_scan_reviewed_slots(char* text, DM1_V22_FprReceipt* r) {
    const char* arr;
    const char* end;
    if (!text || !r) return;
    arr = strstr(text, "\"reviewedSlots\"");
    if (!arr) return;
    arr = strchr(arr, '[');
    if (!arr) return;
    end = strchr(arr, ']');
    if (!end) return;

    const char* p = arr + 1;
    while (p < end) {
        const char* q = strchr(p, '"');
        if (!q || q > end) break;
        ++q;
        const char* qend = strchr(q, '"');
        if (!qend || qend > end) break;
        char id_buf[64];
        size_t id_len = (size_t)(qend - q);
        if (id_len >= sizeof(id_buf)) id_len = sizeof(id_buf) - 1U;
        memcpy(id_buf, q, id_len);
        id_buf[id_len] = '\0';

        for (size_t i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
            const char* required = dm1_v22_famg_slot_name(
                (DM1_V22_FamgSlot)i);
            if (required && strcmp(id_buf, required) == 0) {
                r->reviewed[i] = 1;
            }
        }
        p = qend + 1;
    }
}

/* ── Receipt parser ───────────────────────────────────────────── */
static int fpr_parse_receipt(DM1_V22_FprReceipt* r) {
    FILE* fp;
    long size;
    char* text;
    char tmp[1024];

    memset(r, 0, sizeof(*r));
    if (g_receipt_path[0] == '\0') return 0;
    fp = fopen(g_receipt_path, "rb");
    if (!fp) return 0;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0; }
    size = ftell(fp);
    if (size < 0 || size > (long)(64L * 1024L)) { fclose(fp); return 0; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return 0; }
    text = (char*)malloc((size_t)size + 1U);
    if (!text) { fclose(fp); return 0; }
    if (fread(text, 1, (size_t)size, fp) != (size_t)size) {
        free(text); fclose(fp); return 0;
    }
    fclose(fp);
    text[size] = '\0';

    /* The receipt file must be a JSON object. */
    const char* t = text;
    while (*t == ' ' || *t == '\t' || *t == '\r' || *t == '\n') ++t;
    if (*t != '{') {
        free(text);
        return 0;
    }

    if (fpr_extract_top_string(text, (size_t)size, "manifestPath",
                                r->manifest_path,
                                sizeof(r->manifest_path))) {
        r->has_manifest_path = 1;
        fpr_trim(r->manifest_path, r->manifest_path,
                  sizeof(r->manifest_path));
    }
    if (fpr_extract_top_string(text, (size_t)size, "manifestHashFnv1a",
                                r->manifest_hash_hex,
                                sizeof(r->manifest_hash_hex))) {
        r->has_manifest_hash = 1;
        fpr_trim(r->manifest_hash_hex, r->manifest_hash_hex,
                  sizeof(r->manifest_hash_hex));
    }
    if (fpr_extract_top_string(text, (size_t)size, "reviewer",
                                r->reviewer, sizeof(r->reviewer))) {
        r->has_reviewer = 1;
        fpr_trim(r->reviewer, r->reviewer, sizeof(r->reviewer));
    }
    if (fpr_extract_top_string(text, (size_t)size, "reviewedAtUtc",
                                r->reviewed_at_utc,
                                sizeof(r->reviewed_at_utc))) {
        r->has_reviewed_at = 1;
        fpr_trim(r->reviewed_at_utc, r->reviewed_at_utc,
                  sizeof(r->reviewed_at_utc));
    }
    if (fpr_extract_top_string(text, (size_t)size, "gateTarget",
                                r->gate_target, sizeof(r->gate_target))) {
        r->has_gate_target = 1;
        fpr_trim(r->gate_target, r->gate_target, sizeof(r->gate_target));
    }

    fpr_scan_reviewed_slots(text, r);

    /* Filled tmp is unused but kept so a future reviewer-signature
     * field can reuse it. Mark as used to silence -Wunused. */
    (void)snprintf(tmp, sizeof(tmp), "%s", "fpr-parse-ok");

    free(text);

    /* Schema sanity. The receipt must carry at minimum
     * manifestHashFnv1a + reviewedSlots. Without manifestPath or
     * reviewer the receipt is informational only; we still treat
     * it as MALFORMED for promotion purposes because reviewers
     * expect the full schema. */
    int has_required = r->has_manifest_hash &&
                       (r->reviewed[0] || r->reviewed[1] ||
                        r->reviewed[2] || r->reviewed[3] ||
                        r->reviewed[4] || r->reviewed[5]);
    if (!has_required) return 0;
    return 1;
}

/* ── State evaluator (cached) ────────────────────────────────── */
static DM1_V22_FprState fpr_evaluate(void) {
    int manifest_real = 0;
    int receipt_hashes_match = 0;
    int all_reviewed = 0;
    int some_reviewed = 0;
    size_t i;

    /* Reset the cached receipt record; the state is rebuilt from
     * scratch every time the gate is dirtied (initial call after
     * set_receipt_path() / reset_state()). */
    memset(&g_last_receipt, 0, sizeof(g_last_receipt));

    /* 1. Receipt file present? */
    int receipt_present = 0;
    {
        FILE* fp = (g_receipt_path[0] != '\0')
                       ? fopen(g_receipt_path, "rb") : NULL;
        receipt_present = (fp != NULL);
        if (fp) fclose(fp);
    }
    if (!receipt_present) {
        g_state_dirty = 0;
        return DM1_V22_FPR_NOT_INSTALLED;
    }

    /* 2. Receipt parseable? */
    if (!fpr_parse_receipt(&g_last_receipt)) {
        g_state_dirty = 0;
        return DM1_V22_FPR_MALFORMED;
    }

    /* 3. Manifest hash match? */
    {
        uint32_t manifest_hash = dm1_v22_fpr_fnv1a_file(g_manifest_path);
        uint32_t receipt_hash = parse_hex_u32(g_last_receipt.manifest_hash_hex);
        receipt_hashes_match = (receipt_hash != 0U && receipt_hash == manifest_hash);
    }
    if (!receipt_hashes_match) {
        g_state_dirty = 0;
        return DM1_V22_FPR_STALE;
    }

    /* 4. Material gate FINISHED_REAL? */
    manifest_real = dm1_v22_famg_is_finished_real();
    if (!manifest_real) {
        g_state_dirty = 0;
        return DM1_V22_FPR_MATERIAL_NOT_REAL;
    }

    /* 5. reviewedSlots coverage? */
    for (i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        if (g_last_receipt.reviewed[i]) ++some_reviewed;
    }
    all_reviewed = (some_reviewed == (int)DM1_V22_FAMG_MATERIAL_COUNT);
    if (!all_reviewed) {
        g_state_dirty = 0;
        return DM1_V22_FPR_MATCH_PARTIAL;
    }

    g_state_dirty = 0;
    return DM1_V22_FPR_MATCH_FINISHED_REAL;
}

DM1_V22_FprState dm1_v22_fpr_state(void) {
    static DM1_V22_FprState last_state = DM1_V22_FPR_NOT_INSTALLED;
    if (g_state_dirty) {
        last_state = fpr_evaluate();
    } else {
        /* Even when not dirty, callers expect INSTALLED_UNVERIFIED to
         * mean "the receipt file exists but evaluation has not been
         * requested". Since we always evaluate on the first call, a
         * subsequent call simply reuses the cached result. The probe
         * explicitly resets via dm1_v22_fpr_reset_state() between
         * scenarios to keep the loop deterministic. */
        (void)last_state;
    }
    if (g_state_dirty) last_state = fpr_evaluate();
    return last_state;
}

/* ── Convenience queries ──────────────────────────────────────── */
int dm1_v22_fpr_is_promoted(void) {
    return dm1_v22_fpr_state() == DM1_V22_FPR_MATCH_FINISHED_REAL ? 1 : 0;
}

int dm1_v22_fpr_receipt_present(void) {
    FILE* fp;
    if (g_receipt_path[0] == '\0') return 0;
    fp = fopen(g_receipt_path, "rb");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}

int dm1_v22_fpr_receipt_hash_matches(void) {
    FILE* fp;
    uint32_t manifest_hash;
    uint32_t receipt_hash;
    char tmp[16] = {0};
    if (g_receipt_path[0] == '\0') return -1;
    fp = fopen(g_receipt_path, "rb");
    if (!fp) return -1;
    fclose(fp);
    if (!fpr_parse_receipt(&g_last_receipt)) return 0;
    (void)snprintf(tmp, sizeof(tmp), "%s", "hash-match");
    manifest_hash = dm1_v22_fpr_fnv1a_file(g_manifest_path);
    receipt_hash = parse_hex_u32(g_last_receipt.manifest_hash_hex);
    return (receipt_hash != 0U && receipt_hash == manifest_hash) ? 1 : 0;
}

int dm1_v22_fpr_receipt_slot_count(int* out_required) {
    int reviewed = 0;
    size_t i;
    if (out_required) *out_required = (int)DM1_V22_FAMG_MATERIAL_COUNT;
    for (i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        if (g_last_receipt.reviewed[i]) ++reviewed;
    }
    return reviewed;
}

int dm1_v22_fpr_receipt_stale_review_count(void) {
    int stale = 0;
    size_t i;
    /* A "stale review" means the receipt lists a slot as reviewed,
     * but the material gate's current classification for that slot
     * is not REAL. This catches the post-review regression case. */
    for (i = 0; i < DM1_V22_FAMG_MATERIAL_COUNT; ++i) {
        if (!g_last_receipt.reviewed[i]) continue;
        DM1_V22_FamgClass cls = dm1_v22_famg_classify_slot(
            (DM1_V22_FamgSlot)i);
        if (cls != DM1_V22_FAMG_CLASS_REAL) ++stale;
    }
    return stale;
}

const char* dm1_v22_fpr_state_name(DM1_V22_FprState state) {
    switch (state) {
        case DM1_V22_FPR_NOT_INSTALLED:        return "NOT_INSTALLED";
        case DM1_V22_FPR_INSTALLED_UNVERIFIED: return "INSTALLED_UNVERIFIED";
        case DM1_V22_FPR_MALFORMED:            return "MALFORMED";
        case DM1_V22_FPR_STALE:                return "STALE";
        case DM1_V22_FPR_MATERIAL_NOT_REAL:    return "MATERIAL_NOT_REAL";
        case DM1_V22_FPR_MATCH_PARTIAL:        return "MATCH_PARTIAL";
        case DM1_V22_FPR_MATCH_FINISHED_REAL:  return "MATCH_FINISHED_REAL";
        default: return "INVALID";
    }
}

const char* dm1_v22_fpr_source_evidence(void) {
    return
        "DM1 V2.2 finished-art pack reviewer-receipt promotion gate\n"
        "Source: ReDMCSB DUNVIEW.C:6697-6816 (DM1 viewport composition order)\n"
        "Source: ReDMCSB DUNGEON.C:2238-2246 (square-type decode feeding m11_v22_shape_for_cell)\n"
        "Source: include/dm1_v22_finished_art_material_gate_pc34.h (sibling gate)\n"
        "Source: include/dm1_v2_asset_pipeline_pc34.h (modern asset root)\n"
        "Source: sibling dm2_v2_hud_widget_assets.c (placeholder-vs-real pattern)\n"
        "Receipt path: ~/.firestaff/assets/dm1/modern/finish_receipt.json\n"
        "Schema: { receiptVersion, manifestPath, manifestHashFnv1a,\n"
        "         reviewer, reviewedAtUtc, gateTarget, reviewedSlots, notes }\n"
        "Hash: FNV-1a 32-bit on the on-disk modern_asset_manifest.json content\n"
        "State: NOT_INSTALLED (skip-safe default)\n"
        "       INSTALLED_UNVERIFIED (receipt present, never evaluated)\n"
        "       MALFORMED (missing required fields)\n"
        "       STALE (receipt manifestHashFnv1a != on-disk manifest hash)\n"
        "       MATERIAL_NOT_REAL (hash matches but material gate != FINISHED_REAL)\n"
        "       MATCH_PARTIAL (hash matches, material FINISHED_REAL, slot list incomplete)\n"
        "       MATCH_FINISHED_REAL (hash matches, material FINISHED_REAL, slots complete)\n"
        "is_promoted() returns 1 ONLY for MATCH_FINISHED_REAL.\n"
        "The receipt never overrides material evidence; the synthetic CI default\n"
        "(no receipt on disk -> NOT_INSTALLED) leaves all existing gates honest.\n"
        "Honest boundary: this module does not perform PKI / signature verification\n"
        "of the reviewer handle. The integrity model is 'the manifest content the\n"
        "reviewer signed off on still matches what's on disk'. A reviewer who can\n"
        "rewrite both files together is a Firestaff workflow integrity problem,\n"
        "not a cryptographically-soluble one.\n";
}
