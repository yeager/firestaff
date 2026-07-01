/*
 * csb_v22_inplace_draw_pc34.c
 *
 * CSB V2.2 GPU render path: V22 modern-art IN-PLACE bitmap cache.
 *
 * This is the foundation for switching the V22 render mode from
 * "overlay" (placeholder colored rectangle on top of V1) to
 * "in-place" (replace V1 sprite with V22 PBR PNG at the same cell).
 *
 * Build-time pipeline:
 *   1. .openclaw/tools/png_to_rgba.py reads modern_asset_manifest.json
 *      + every PNG referenced, scales to MAX_SIZE=256, packs into a
 *      single v22_inplace_cache.bin (header + per-asset entries + raw RGBA).
 *   2. Output: ~/.firestaff/assets/csb/modern/v22_inplace_cache.bin
 *      (~6 MB for 29 assets at 256x256 max, lanczos-resampled)
 *
 * Runtime pipeline:
 *   csb_v22_inplace_draw_init() reads cache file, mmaps, builds in-memory
 *   hash table keyed by (category_hash, asset_id_hash).
 *   csb_v22_inplace_get_cell_bitmap(depth, lateral) looks up the V22 shape
 *   for the cell, maps variant -> asset_id, looks up bitmap in hash table,
 *   returns RGBA pointer + dimensions.
 *
 * Variant -> asset mapping (first cut, intentionally conservative):
 *   - Walls (any variant) -> wall_dungeon_01
 *   - Floor PLAIN pattern  -> floor_plain_01
 *   - Floor CRACKED pattern -> floor_cracked_01
 *   - Floor MOSSY pattern  -> floor_plain_01 (no mossy variant in v1.4.0)
 *   - Creatures (any)      -> creature_demon_01
 *
 * Source-lock: csb_v22_shape_cache_pc34.h (the cache),
 * csb_v22_modern_assets_pc34.c (manifest path resolution),
 * csb_v22_shape_cache_pc34.c (sibling cache + CSB_V22_CellRect coords),
 * include/dm1_v2_shape_runtime_pc34.h (shape variant enum),
 * ReDMCSB DUNVIEW.C:6697-6816 (composition order).
 */

#include "csb_v22_inplace_draw_pc34.h"
#include "csb_v22_inplace_route_pc34.h"
#include "csb_v22_shape_cache_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
#include "dm1_v2_asset_pipeline_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ── Cache file format ──────────────────────────────────────────── */

#define FSV22C_MAGIC     "FSV22C\0\0"
#define FSV22C_VERSION   1U
#define FSV22C_HDR_SIZE  32
#define FSV22C_ENT_SIZE  32
#define FSV22C_MAX_ENT   256

typedef struct {
    uint32_t category_hash;
    uint32_t asset_id_hash;
    uint32_t width;
    uint32_t height;
    uint32_t rgba_size;
    uint32_t rgba_offset;
} FsV22CacheEntry;

typedef struct {
    FsV22CacheEntry entry;
    const uint8_t* rgba;     /* pointer into mmap or malloc'd buffer */
} FsV22CachedBitmap;

/* ── Module state ───────────────────────────────────────────────── */

static int              g_v22_inplace_active = 0;
static unsigned char*   g_v22_cache_buf = NULL;     /* mmap or malloc'd cache file */
static size_t           g_v22_cache_size = 0;
static int              g_v22_cache_mapped = 0;    /* 1 if mmap, 0 if malloc */
static FsV22CachedBitmap g_v22_bitmaps[FSV22C_MAX_ENT];
static int              g_v22_bitmap_count = 0;

/* ── Variant -> asset_id mapping ───────────────────────────────── */

/* The asset_id for a cell is decided by csb_v22_inplace_route_cell,
 * the per-cell material-routing gate. The gate is the single source
 * of truth for which (category, asset_id) the in-place draw should
 * consult for a given (depth, lateral, cell_type). Per-cell routing
 * gives the 9-square CSB viewport distinct per-depth wall art,
 * distinct per-depth floor art, distinct pit/stairs art, and a
 * lateral-driven chaos-rune index. Fields (teleporter / fluxcage /
 * explosion / chaos_rift) deliberately return no asset so they
 * fall back to V1 instead of being painted with the wrong wall art.
 *
 * The route gate returns the asset_id by value (a copy of a static
 * literal into the caller's CSB_V22_AssetRouteDecision). This module
 * keeps a static mirror of the last routed asset_id so its
 * csb_v22_inplace_get_cell_asset_id contract can keep returning
 * a pointer that lives for the program lifetime (callers, including
 * the bitmap cache lookup in csb_v22_inplace_get_cell_bitmap, treat
 * the returned pointer as a key, not as a temporary). The mirror is
 * process-singleton; the in-place draw is single-threaded. */

/* Lookup table from (depth 0..2, lateral -1..+1) to the
 * per-cell asset_id the route gate picked last. Used only to
 * publish a stable pointer for csb_v22_inplace_get_cell_asset_id.
 * The cell-bitmap lookup consults the route gate directly and
 * does not read this table. */
static char g_csb_v22_inplace_asset_mirror[3][3][CSB_V22_ASSET_ID_MAX];
static int  g_csb_v22_inplace_asset_mirror_valid[3][3];

static const char* v22_inplace_get_cell_asset_id(int depth, int lateral) {
    if (!csb_v22_shape_cache_active(depth, lateral)) return NULL;
    const CSB_V22_ShapeRuntimeResult* r = csb_v22_shape_cache_get(depth, lateral);
    if (!r || !r->active) return NULL;

    /* Drive the per-cell route gate. The gate consumes the cached
     * raw M034 cell type (stored by csb_v22_shape_cache_update) so
     * callers do not have to consult the shape book first. */
    int raw = csb_v22_shape_cache_get_raw_cell(depth, lateral);
    if (raw < 0) return NULL;
    CSB_V22_AssetRouteDecision d;
    csb_v22_inplace_route_cell(depth, lateral, raw,
                                csb_v2_presentation_mode_is_v22(), &d);
    if (!d.use_v22 || d.asset_id[0] == '\0') return NULL;
    if (depth < 0 || depth > 2) return NULL;
    if (lateral < -1 || lateral > 1) return NULL;
    {
        int li = lateral + 1;
        size_t n = strlen(d.asset_id);
        if (n + 1U > CSB_V22_ASSET_ID_MAX) n = CSB_V22_ASSET_ID_MAX - 1U;
        memcpy(g_csb_v22_inplace_asset_mirror[depth][li],
               d.asset_id, n);
        g_csb_v22_inplace_asset_mirror[depth][li][n] = '\0';
        g_csb_v22_inplace_asset_mirror_valid[depth][li] = 1;
        return g_csb_v22_inplace_asset_mirror[depth][li];
    }
}

/* ── Hash helpers ──────────────────────────────────────────────── */

static uint32_t fnv1a_hash(const char* s) {
    uint32_t h = 2166136261u;
    while (*s) { h = (h ^ (uint8_t)*s++) * 16777619u; }
    return h;
}

static int v22_find_bitmap(uint32_t category_hash, uint32_t asset_id_hash) {
    for (int i = 0; i < g_v22_bitmap_count; ++i) {
        if (g_v22_bitmaps[i].entry.category_hash == category_hash &&
            g_v22_bitmaps[i].entry.asset_id_hash == asset_id_hash) {
            return i;
        }
    }
    return -1;
}

/* ── Cache load ─────────────────────────────────────────────────── */

static int v22_load_cache_file(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    if (sz <= 0 || sz > 64L * 1024L * 1024L) {  /* hard cap 64 MB */
        fclose(fp); return 0;
    }
    fseek(fp, 0, SEEK_SET);
    g_v22_cache_size = (size_t)sz;
    g_v22_cache_buf = (unsigned char*)malloc(g_v22_cache_size);
    if (!g_v22_cache_buf) { fclose(fp); return 0; }
    if (fread(g_v22_cache_buf, 1, g_v22_cache_size, fp) != g_v22_cache_size) {
        free(g_v22_cache_buf); g_v22_cache_buf = NULL; g_v22_cache_size = 0;
        fclose(fp); return 0;
    }
    fclose(fp);
    g_v22_cache_mapped = 0;

    /* Parse header */
    if (g_v22_cache_size < FSV22C_HDR_SIZE) return 0;
    if (memcmp(g_v22_cache_buf, FSV22C_MAGIC, 8) != 0) return 0;
    uint32_t version = 0, count = 0;
    memcpy(&version, g_v22_cache_buf + 8, 4);
    memcpy(&count, g_v22_cache_buf + 12, 4);
    if (version != FSV22C_VERSION) return 0;
    if (count > FSV22C_MAX_ENT) return 0;

    /* Parse entries + index RGBA pointers */
    size_t entries_off = FSV22C_HDR_SIZE;
    size_t data_off    = FSV22C_HDR_SIZE + (size_t)count * FSV22C_ENT_SIZE;
    if (data_off >= g_v22_cache_size) return 0;

    g_v22_bitmap_count = 0;
    for (uint32_t i = 0; i < count; ++i) {
        const unsigned char* ep = g_v22_cache_buf + entries_off + i * FSV22C_ENT_SIZE;
        FsV22CacheEntry e;
        memcpy(&e.category_hash, ep + 0,  4);
        memcpy(&e.asset_id_hash, ep + 4,  4);
        memcpy(&e.width,         ep + 8,  4);
        memcpy(&e.height,        ep + 12, 4);
        memcpy(&e.rgba_size,      ep + 16, 4);
        memcpy(&e.rgba_offset,    ep + 20, 4);
        if (e.rgba_offset + e.rgba_size > g_v22_cache_size) continue;
        g_v22_bitmaps[g_v22_bitmap_count].entry = e;
        g_v22_bitmaps[g_v22_bitmap_count].rgba = g_v22_cache_buf + e.rgba_offset;
        g_v22_bitmap_count++;
    }
    return g_v22_bitmap_count > 0 ? 1 : 0;
}

int csb_v22_inplace_draw_init(void) {
    if (g_v22_inplace_active) return 1;

    /* Resolve cache path from manifest path */
    char cache_path[FSP_PATH_MAX];
    {
        /* Re-use csb_v22_get_shape_path to find the modern dir.
         * The cache file lives next to modern_asset_manifest.json. */
        char manifest_path[FSP_PATH_MAX];
        /* csb_v22 doesn't expose its manifest path; reconstruct from data dir
         * using the same logic as csb_v22_set_manifest_path. For first cut,
         * hardcode the conventional path ~/.firestaff/assets/csb/modern/. */
        const char* home = getenv("HOME");
        if (!home) home = ".";
        snprintf(manifest_path, sizeof(manifest_path),
                 "%s/.firestaff/assets/csb/modern/modern_asset_manifest.json", home);
        char* last_slash = strrchr(manifest_path, '/');
        if (last_slash) *last_slash = '\0';
        snprintf(cache_path, sizeof(cache_path), "%s/v22_inplace_cache.bin", manifest_path);
    }

    if (!v22_load_cache_file(cache_path)) {
        return 0;
    }
    g_v22_inplace_active = 1;
    return 1;
}

void csb_v22_inplace_draw_shutdown(void) {
    if (g_v22_cache_buf && !g_v22_cache_mapped) free(g_v22_cache_buf);
    g_v22_cache_buf = NULL;
    g_v22_cache_size = 0;
    g_v22_cache_mapped = 0;
    g_v22_bitmap_count = 0;
    g_v22_inplace_active = 0;
    memset(g_csb_v22_inplace_asset_mirror_valid, 0,
           sizeof(g_csb_v22_inplace_asset_mirror_valid));
}

int csb_v22_inplace_draw_active(void) {
    return g_v22_inplace_active;
}

const uint32_t* csb_v22_inplace_get_cell_bitmap(int depth, int lateral,
                                                 int* out_w, int* out_h) {
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (!g_v22_inplace_active) return NULL;
    if (!csb_v22_shape_cache_active(depth, lateral)) return NULL;
    {
        const CSB_V22_ShapeRuntimeResult* r =
            csb_v22_shape_cache_get(depth, lateral);
        if (!r || !r->active) return NULL;
    }
    /* Drive the per-cell route gate directly. The category and
     * asset_id are both taken from the gate, so per-cell routing
     * (e.g. chaos_runes vs wall_shapes) is honored by the bitmap
     * lookup. The category string is only consumed for hashing
     * (uint32_t), so a stack-local copy is fine here. */
    int raw = csb_v22_shape_cache_get_raw_cell(depth, lateral);
    if (raw < 0) return NULL;
    CSB_V22_AssetRouteDecision d;
    csb_v22_inplace_route_cell(depth, lateral, raw,
                                csb_v2_presentation_mode_is_v22(), &d);
    if (!d.use_v22 || d.asset_id[0] == '\0' || d.category[0] == '\0') {
        return NULL;
    }

    uint32_t cat_hash = fnv1a_hash(d.category);
    uint32_t aid_hash = fnv1a_hash(d.asset_id);
    int idx = v22_find_bitmap(cat_hash, aid_hash);
    if (idx < 0) return NULL;
    if (out_w) *out_w = (int)g_v22_bitmaps[idx].entry.width;
    if (out_h) *out_h = (int)g_v22_bitmaps[idx].entry.height;
    return (const uint32_t*)g_v22_bitmaps[idx].rgba;
}

const char* csb_v22_inplace_get_cell_asset_id(int depth, int lateral) {
    return v22_inplace_get_cell_asset_id(depth, lateral);
}

const uint32_t* csb_v22_inplace_get_bitmap_by_id(const char* category,
                                                  const char* asset_id,
                                                  int* out_w, int* out_h) {
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (!g_v22_inplace_active) return NULL;
    if (!category || !asset_id || !asset_id[0]) return NULL;
    {
        uint32_t cat_hash = fnv1a_hash(category);
        uint32_t aid_hash = fnv1a_hash(asset_id);
        int idx = v22_find_bitmap(cat_hash, aid_hash);
        if (idx < 0) return NULL;
        if (out_w) *out_w = (int)g_v22_bitmaps[idx].entry.width;
        if (out_h) *out_h = (int)g_v22_bitmaps[idx].entry.height;
        return (const uint32_t*)g_v22_bitmaps[idx].rgba;
    }
}

/* ── In-place bitmap blit ──────────────────────────────────────── */

/* CSB 3x3 cell rect coordinates (depth x lateral). Must match
 * csb_v22_kCellRects in csb_v22_shape_cache_pc34.c (exposed as
 * CSB_V22_CellRect in csb_v22_shape_cache_pc34.h). */
static const CSB_V22_CellRect kV22CellRects[3][3] = {
    /* depth 0 = D1 (closest) */ {
        {  8, 103, 69, 30 },
        { 78, 103, 61, 30 },
        {139, 103, 69, 30 }
    },
    /* depth 1 = D2 (middle) */ {
        {  8,  72, 69, 30 },
        { 78,  72, 61, 30 },
        {139,  72, 69, 30 }
    },
    /* depth 2 = D3 (back) */ {
        {  8,  41, 69, 30 },
        { 78,  41, 61, 30 },
        {139,  41, 69, 30 }
    }
};

/* Clamp helper */
static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Map an RGB color (0..255 per channel) to the nearest EGA/VGA
 * 6-bit-cube palette index. Equivalent to the standard VGA 0x3F
 * (bright=0, palette bits 5..0 = R*2, G*2, B*2 mapping). */
static unsigned char rgb_to_ega_index(unsigned char r,
                                      unsigned char g,
                                      unsigned char b) {
    /* Quantize each channel to 2 bits (0/85/170/255) and combine. */
    int ri = (r * 3 + 127) / 255;
    int gi = (g * 3 + 127) / 255;
    int bi = (b * 3 + 127) / 255;
    return (unsigned char)((ri << 4) | (gi << 2) | bi);
}

/* Nearest-neighbor blit of RGBA bitmap into framebuffer[y*fbW+x]
 * sized src_w x src_h -> dst_w x dst_h. The RGBA pixels are mapped
 * to a single byte via rgb_to_ega_index (good enough for indexed
 * framebuffer V1 mode; full color-blend is a follow-up). */
static void blit_bitmap_to_cell(const uint32_t* rgba, int src_w, int src_h,
                                  unsigned char* framebuffer, int fbW, int fbH,
                                  int dst_x, int dst_y, int dst_w, int dst_h) {
    int x, y;
    if (dst_w <= 0 || dst_h <= 0 || src_w <= 0 || src_h <= 0) return;
    for (y = 0; y < dst_h; ++y) {
        int sy = (y * src_h) / dst_h;
        if (sy >= src_h) sy = src_h - 1;
        int py = dst_y + y;
        if (py < 0 || py >= fbH) continue;
        for (x = 0; x < dst_w; ++x) {
            int sx = (x * src_w) / dst_w;
            if (sx >= src_w) sx = src_w - 1;
            uint32_t px = rgba[sy * src_w + sx];
            /* Extract RGB from RGBA. Alpha is ignored for now
             * (opaque-only assumption; alpha-blend is a follow-up). */
            unsigned char r = (unsigned char)((px >> 16) & 0xFFu);
            unsigned char g = (unsigned char)((px >>  8) & 0xFFu);
            unsigned char b = (unsigned char)((px      ) & 0xFFu);
            unsigned char idx = rgb_to_ega_index(r, g, b);
            int px_x = dst_x + x;
            if (px_x < 0 || px_x >= fbW) continue;
            framebuffer[py * fbW + px_x] = idx;
        }
    }
}

int csb_v22_inplace_render_pass(unsigned char* framebuffer, int fbW, int fbH) {
    int depth, lateral;
    int cells_painted = 0;
    if (!framebuffer || fbW <= 0 || fbH <= 0) return 0;
    if (!csb_v22_inplace_draw_active()) return 0;
    if (!csb_v22_shape_cache_populated()) return 0;
    for (depth = 0; depth < 3; ++depth) {
        for (lateral = -1; lateral <= 1; ++lateral) {
            int w = 0, h = 0;
            const uint32_t* rgba =
                csb_v22_inplace_get_cell_bitmap(depth, lateral, &w, &h);
            if (!rgba || w <= 0 || h <= 0) continue;
            const CSB_V22_CellRect* rect = &kV22CellRects[depth][lateral + 1];
            /* Clamp cell rect to framebuffer bounds */
            int dx = clampi(rect->x, 0, fbW);
            int dy = clampi(rect->y, 0, fbH);
            int dw = clampi(rect->x + rect->w, 0, fbW) - dx;
            int dh = clampi(rect->y + rect->h, 0, fbH) - dy;
            if (dw <= 0 || dh <= 0) continue;
            blit_bitmap_to_cell(rgba, w, h,
                                 framebuffer, fbW, fbH,
                                 dx, dy, dw, dh);
            cells_painted++;
        }
    }
    return cells_painted;
}

const char* csb_v22_inplace_draw_source_evidence(void) {
    return "csb_v22_shape_cache_pc34.c (per-cell V22 shape cache); "
           "dm1_v2_modern_assets_pc34.c (manifest path resolution); "
           "csb_v22_shape_cache_pc34.c (sibling cache + CSB_V22_CellRect coords); "
           "csb_v22_modern_assets_pc34.c (manifest path resolution); "
           "csb_v22_shapes.c (CSB V22 shape book); "
           "ReDMCSB DUNVIEW.C F0128 (CSB viewport routing); "
           "CSBWin/Viewport.cpp:7290 (9-square layout); "
           "include/dm1_v2_shape_runtime_pc34.h (shape variant enum pattern); "
           "v22_inplace_cache.bin (build-time RGBA pack from PNG via PIL).";
}
