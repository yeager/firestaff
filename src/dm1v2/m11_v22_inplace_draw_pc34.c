/*
 * m11_v22_inplace_draw_pc34.c
 *
 * DM1 V2.2 GPU render path: V22 modern-art IN-PLACE bitmap cache.
 *
 * This is the foundation for switching the V22 render mode from
 * "overlay" (placeholder colored rectangle on top of V1) to
 * "in-place" (replace V1 sprite with V22 PBR PNG at the same cell).
 *
 * Build-time pipeline:
 *   1. .openclaw/tools/png_to_rgba.py reads modern_asset_manifest.json
 *      + every PNG referenced, scales to MAX_SIZE=256, packs into a
 *      single v22_inplace_cache.bin (header + per-asset entries + raw RGBA).
 *   2. Output: ~/.firestaff/assets/dm1/modern/v22_inplace_cache.bin
 *      (~6 MB for 29 assets at 256x256 max, lanczos-resampled)
 *
 * Runtime pipeline:
 *   m11_v22_inplace_draw_init() reads cache file, mmaps, builds in-memory
 *   hash table keyed by (category_hash, asset_id_hash).
 *   m11_v22_inplace_get_cell_bitmap(depth, lateral) looks up the V22 shape
 *   for the cell, maps variant -> asset_id, looks up bitmap in hash table,
 *   returns RGBA pointer + dimensions.
 *
 * Variant -> asset mapping (first cut, intentionally conservative):
 *   - Walls (any variant) -> wall_d3_carved_01
 *   - Floor PLAIN pattern  -> floor_plain_01
 *   - Floor CRACKED pattern -> floor_cracked_01
 *   - Floor MOSSY pattern  -> floor_plain_01 (no mossy variant in v1.4.0)
 *   - Creatures (any)      -> creature_demon_01
 *
 * Source-lock: m11_v22_shape_cache_pc34.h (the cache),
 * m11_v2_modern_assets_pc34.c (manifest path resolution),
 * m11_v22_render_overlay_pc34.c (sibling overlay path),
 * include/dm1_v2_shape_runtime_pc34.h (shape variant enum),
 * ReDMCSB DUNVIEW.C:6697-6816 (composition order).
 */

#include "m11_v22_inplace_draw_pc34.h"
#include "m11_v22_shape_cache_pc34.h"
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

/* For first cut, all walls use wall_d3_carved_01 (the most common
 * carved stone). All creatures use creature_demon_01. Floors use
 * the tile pattern. */
static const char* v22_wall_asset_id  = "wall_d3_carved_01";
static const char* v22_floor_plain_id = "floor_plain_01";
static const char* v22_floor_cracked_id = "floor_cracked_01";
static const char* v22_creature_asset_id = "creature_demon_01";

static const char* v22_inplace_get_cell_asset_id(int depth, int lateral) {
    if (!m11_v22_shape_cache_active(depth, lateral)) return NULL;
    const DM1_V2_ShapeRuntimeResult* r = m11_v22_shape_cache_get(depth, lateral);
    if (!r || !r->active) return NULL;

    /* Use M11_V22_ShapeParams.type to pick asset. The shape type enum
     * (M11_V22_SHAPE_FLOOR_PLAIN/CRACKED etc., M11_V22_SHAPE_WALL_*,
     * M11_V22_SHAPE_CREATURE_*) is the authoritative discriminator. */
    M11_V22_ShapeType t = r->params.type;
    switch (t) {
        case M11_V22_SHAPE_FLOOR_PLAIN:
        case M11_V22_SHAPE_CEILING_PLAIN:
            return v22_floor_plain_id;
        case M11_V22_SHAPE_FLOOR_CRACKED:
        case M11_V22_SHAPE_FLOOR_MOSSY:
            return v22_floor_cracked_id;
        case M11_V22_SHAPE_CREATURE:
        case M11_V22_SHAPE_CREATURE_PROJECTILE:
            return v22_creature_asset_id;
        case M11_V22_SHAPE_ITEM:
        case M11_V22_SHAPE_ITEM_FLOOR:
        case M11_V22_SHAPE_ITEM_PROJECTILE:
            /* Items use creature sprite as placeholder (no items asset yet) */
            return v22_creature_asset_id;
        default:
            /* Walls (1-9), door (10-12), fields (50-52), stairs (14-15), pit (13) */
            return v22_wall_asset_id;
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

int m11_v22_inplace_draw_init(void) {
    if (g_v22_inplace_active) return 1;

    /* Resolve cache path from manifest path */
    char cache_path[FSP_PATH_MAX];
    {
        /* Re-use m11_v22_get_shape_path to find the modern dir.
         * The cache file lives next to modern_asset_manifest.json. */
        char manifest_path[FSP_PATH_MAX];
        /* m11_v22 doesn't expose its manifest path; reconstruct from data dir
         * using the same logic as m11_v22_set_manifest_path. For first cut,
         * hardcode the conventional path ~/.firestaff/assets/dm1/modern/. */
        const char* home = getenv("HOME");
        if (!home) home = ".";
        snprintf(manifest_path, sizeof(manifest_path),
                 "%s/.firestaff/assets/dm1/modern/modern_asset_manifest.json", home);
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

void m11_v22_inplace_draw_shutdown(void) {
    if (g_v22_cache_buf && !g_v22_cache_mapped) free(g_v22_cache_buf);
    g_v22_cache_buf = NULL;
    g_v22_cache_size = 0;
    g_v22_cache_mapped = 0;
    g_v22_bitmap_count = 0;
    g_v22_inplace_active = 0;
}

int m11_v22_inplace_draw_active(void) {
    return g_v22_inplace_active;
}

const uint32_t* m11_v22_inplace_get_cell_bitmap(int depth, int lateral,
                                                 int* out_w, int* out_h) {
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (!g_v22_inplace_active) return NULL;
    const char* asset_id = v22_inplace_get_cell_asset_id(depth, lateral);
    if (!asset_id) return NULL;
    const char* category = "wall_shapes";  /* default; refine per cell */
    /* Decide category from asset_id */
    if (strncmp(asset_id, "floor_", 6) == 0) category = "floor_shapes";
    else if (strncmp(asset_id, "creature_", 9) == 0) category = "creature_shapes";
    else if (strncmp(asset_id, "ui_", 3) == 0) category = "ui_chrome";
    else if (strncmp(asset_id, "champion_", 9) == 0) category = "champion_portraits";
    else if (strncmp(asset_id, "door_", 5) == 0) category = "door_shapes";

    uint32_t cat_hash = fnv1a_hash(category);
    uint32_t aid_hash = fnv1a_hash(asset_id);
    int idx = v22_find_bitmap(cat_hash, aid_hash);
    if (idx < 0) return NULL;
    if (out_w) *out_w = (int)g_v22_bitmaps[idx].entry.width;
    if (out_h) *out_h = (int)g_v22_bitmaps[idx].entry.height;
    return (const uint32_t*)g_v22_bitmaps[idx].rgba;
}

const char* m11_v22_inplace_get_cell_asset_id(int depth, int lateral) {
    return v22_inplace_get_cell_asset_id(depth, lateral);
}

const char* m11_v22_inplace_draw_source_evidence(void) {
    return "m11_v22_shape_cache_pc34.c (per-cell V22 shape cache); "
           "dm1_v2_modern_assets_pc34.c (manifest path resolution); "
           "m11_v22_render_overlay_pc34.c (sibling overlay path, placeholder); "
           "include/dm1_v2_shape_runtime_pc34.h (shape variant enum); "
           "ReDMCSB DUNVIEW.C:6697-6816 (DM1 4x3 composition order); "
           "v22_inplace_cache.bin (build-time RGBA pack from PNG via PIL).";
}
