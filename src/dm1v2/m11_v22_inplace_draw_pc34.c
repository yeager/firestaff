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
 *   - Teleporter fields    -> field_teleporter_01
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
#include "m11_v22_render_overlay_pc34.h"   /* M11_V22_CellRect shared coord */
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
static const char* v22_floor_pit_id = "floor_pit_01";
static const char* v22_floor_stairs_down_id = "floor_stairs_down_01";
static const char* v22_field_teleporter_id = "field_teleporter_01";
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
        case M11_V22_SHAPE_FLOOR_PIT:
            return v22_floor_pit_id;
        case M11_V22_SHAPE_FLOOR_STAIRS_UP:
        case M11_V22_SHAPE_FLOOR_STAIRS_DOWN:
            return v22_floor_stairs_down_id;
        case M11_V22_SHAPE_FIELD_TELEPORTER:
            /* ReDMCSB DUNVIEW.C F0113 draws the teleporter/fluxcage
             * field as its own C10-transparent surface after the floor/
             * wall tail, not as wall art. V2.2 keeps that distinction by
             * routing to a field asset when the modern pack provides one. */
            return v22_field_teleporter_id;
        case M11_V22_SHAPE_FIELD_FLUXCAGE:
        case M11_V22_SHAPE_FIELD_EXPLOSION:
            return NULL;
        case M11_V22_SHAPE_CREATURE:
        case M11_V22_SHAPE_CREATURE_PROJECTILE:
            return v22_creature_asset_id;
        case M11_V22_SHAPE_ITEM:
        case M11_V22_SHAPE_ITEM_FLOOR:
        case M11_V22_SHAPE_ITEM_PROJECTILE:
            /* Items use creature sprite as placeholder (no items asset yet) */
            return v22_creature_asset_id;
        default:
            /* Walls and doors use the first-cut carved-stone asset. */
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

typedef struct {
    uint32_t category_hash;
    uint32_t asset_id_hash;
} FsV22ManifestKey;

static int v22_extract_string_field(const char* line,
                                    const char* key,
                                    char* out,
                                    size_t out_size) {
    char pattern[64];
    const char* p;
    size_t dst = 0U;
    if (!line || !key || !out || out_size == 0U) return 0;
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(line, pattern);
    if (!p) return 0;
    p += strlen(pattern);
    while (*p == ' ' || *p == ':' || *p == '\t') ++p;
    if (*p != '"') return 0;
    ++p;
    while (*p != '\0' && dst + 1U < out_size) {
        if (*p == '"') break;
        out[dst++] = *p++;
    }
    out[dst] = '\0';
    return dst > 0U ? 1 : 0;
}

static int v22_manifest_category_from_line(const char* line,
                                           char* out,
                                           size_t out_size) {
    static const char* const categories[] = {
        "wall_shapes",
        "floor_shapes",
        "creature_shapes",
        "field_shapes",
        "ui_chrome",
        "champion_portraits",
        "door_shapes",
        NULL
    };
    int i;
    if (!line || !out || out_size == 0U) return 0;
    for (i = 0; categories[i] != NULL; ++i) {
        char pattern[64];
        snprintf(pattern, sizeof(pattern), "\"%s\"", categories[i]);
        if (strstr(line, pattern) != NULL && strchr(line, '[') != NULL) {
            snprintf(out, out_size, "%s", categories[i]);
            return 1;
        }
    }
    return 0;
}

static int v22_build_manifest_keys_for_cache(const char* cache_path,
                                             FsV22ManifestKey* keys,
                                             size_t max_keys) {
    char manifest_path[FSP_PATH_MAX];
    char current_category[64];
    char line[512];
    char* slash;
    FILE* fp;
    size_t count = 0U;
    if (!cache_path || !keys || max_keys == 0U) return 0;
    snprintf(manifest_path, sizeof(manifest_path), "%s", cache_path);
    slash = strrchr(manifest_path, '/');
    if (!slash) return 0;
    slash[1] = '\0';
    strncat(manifest_path, "modern_asset_manifest.json",
            sizeof(manifest_path) - strlen(manifest_path) - 1U);

    fp = fopen(manifest_path, "rb");
    if (!fp) return 0;
    current_category[0] = '\0';
    while (fgets(line, sizeof(line), fp) != NULL) {
        char id[128];
        if (v22_manifest_category_from_line(line,
                                            current_category,
                                            sizeof(current_category))) {
            continue;
        }
        if (current_category[0] == '\0') continue;
        if (!v22_extract_string_field(line, "id", id, sizeof(id))) continue;
        if (count >= max_keys) break;
        keys[count].category_hash = fnv1a_hash(current_category);
        keys[count].asset_id_hash = fnv1a_hash(id);
        count++;
    }
    fclose(fp);
    return (int)count;
}

/* ── Cache load ─────────────────────────────────────────────────── */

static int v22_parse_cache_entries(uint32_t count,
                                   size_t entry_size,
                                   size_t field_offset,
                                   size_t data_off,
                                   int offset_is_relative,
                                   const FsV22ManifestKey* remap_keys,
                                   int remap_key_count) {
    uint32_t i;
    size_t entries_off = FSV22C_HDR_SIZE;
    if (count > FSV22C_MAX_ENT) return 0;
    if (entries_off + (size_t)count * entry_size > g_v22_cache_size) return 0;
    if (data_off >= g_v22_cache_size) return 0;
    if (remap_keys && remap_key_count <= 0) return 0;

    g_v22_bitmap_count = 0;
    for (i = 0; i < count; ++i) {
        const unsigned char* ep =
            g_v22_cache_buf + entries_off + (size_t)i * entry_size + field_offset;
        FsV22CacheEntry e;
        size_t rgba_offset;
        memcpy(&e.category_hash, ep + 0,  4);
        memcpy(&e.asset_id_hash, ep + 4,  4);
        memcpy(&e.width,         ep + 8,  4);
        memcpy(&e.height,        ep + 12, 4);
        memcpy(&e.rgba_size,      ep + 16, 4);
        memcpy(&e.rgba_offset,    ep + 20, 4);
        if (remap_keys && (int)i < remap_key_count) {
            e.category_hash = remap_keys[i].category_hash;
            e.asset_id_hash = remap_keys[i].asset_id_hash;
        }
        if (e.category_hash == 0U || e.asset_id_hash == 0U) continue;
        if (e.width == 0 || e.height == 0 || e.rgba_size == 0) continue;
        if (e.width > 4096U || e.height > 4096U) continue;
        rgba_offset = offset_is_relative
            ? data_off + (size_t)e.rgba_offset
            : (size_t)e.rgba_offset;
        if (rgba_offset > g_v22_cache_size) continue;
        if ((size_t)e.rgba_size > g_v22_cache_size - rgba_offset) continue;
        e.rgba_offset = (uint32_t)rgba_offset;
        g_v22_bitmaps[g_v22_bitmap_count].entry = e;
        g_v22_bitmaps[g_v22_bitmap_count].rgba = g_v22_cache_buf + rgba_offset;
        g_v22_bitmap_count++;
    }
    return g_v22_bitmap_count > 0 ? 1 : 0;
}

static int v22_load_cache_file(const char* path) {
    FILE* fp = fopen(path, "rb");
    FsV22ManifestKey manifest_keys[FSV22C_MAX_ENT];
    int manifest_key_count = 0;
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

    /* The installed DM1 v1.4 material cache was generated by the PNG
     * packer with a 4-byte reserved prefix, 40-byte entries, and RGBA
     * offsets relative to the post-entry data block. Try that first:
     * parsing it as the older 32-byte probe format can accidentally accept
     * misaligned rows. The probe writer's original 32-byte absolute-offset
     * format remains as a fallback for data-free tests. */
    manifest_key_count =
        v22_build_manifest_keys_for_cache(path, manifest_keys, FSV22C_MAX_ENT);
    if (v22_parse_cache_entries(count,
                                40U,
                                4U,
                                FSV22C_HDR_SIZE + (size_t)count * 40U,
                                1,
                                manifest_keys,
                                manifest_key_count)) {
        return 1;
    }
    if (v22_parse_cache_entries(count,
                                FSV22C_ENT_SIZE,
                                0U,
                                FSV22C_HDR_SIZE + (size_t)count * FSV22C_ENT_SIZE,
                                0,
                                NULL,
                                0)) {
        return 1;
    }
    return 0;
}

int m11_v22_inplace_draw_init(void) {
    if (g_v22_inplace_active) return 1;

    char cache_path[FSP_PATH_MAX];
    const char* modern_root = m11_v22_get_modern_asset_root();
    if (modern_root && modern_root[0] != '\0') {
        FSP_JoinPath(cache_path, sizeof(cache_path),
                     modern_root, "v22_inplace_cache.bin");
    } else {
        const char* home = getenv("HOME");
        if (!home) home = ".";
        snprintf(cache_path, sizeof(cache_path),
                 "%s/.firestaff/assets/dm1/modern/v22_inplace_cache.bin",
                 home);
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
    else if (strncmp(asset_id, "field_", 6) == 0) category = "field_shapes";
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

/* ── In-place bitmap blit ──────────────────────────────────────── */

/* DM1 4x3 cell rect coordinates (depth x lateral). Must match
 * kV22CellRects in m11_v22_render_overlay_pc34.c (exposed as
 * M11_V22_CellRect in m11_v22_render_overlay_pc34.h). */
static const M11_V22_CellRect kV22CellRects[3][3] = {
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
            int px_x = dst_x + x;
            if (px == 0U) continue;
            if (px_x < 0 || px_x >= fbW) continue;
            /* Extract RGB from the cache's legacy 0x00RRGGBB pixels.
             * A fully zero pixel is transparent/no-write so modern art
             * cutouts do not erase the source V1 dungeon underneath. */
            unsigned char r = (unsigned char)((px >> 16) & 0xFFu);
            unsigned char g = (unsigned char)((px >>  8) & 0xFFu);
            unsigned char b = (unsigned char)((px      ) & 0xFFu);
            unsigned char idx = rgb_to_ega_index(r, g, b);
            framebuffer[py * fbW + px_x] = idx;
        }
    }
}

int m11_v22_inplace_render_pass(unsigned char* framebuffer, int fbW, int fbH) {
    int depth, lateral;
    int cells_painted = 0;
    if (!framebuffer || fbW <= 0 || fbH <= 0) return 0;
    if (!m11_v22_inplace_draw_active()) return 0;
    if (!m11_v22_shape_cache_populated()) return 0;
    for (depth = 0; depth < 3; ++depth) {
        for (lateral = -1; lateral <= 1; ++lateral) {
            int w = 0, h = 0;
            const uint32_t* rgba =
                m11_v22_inplace_get_cell_bitmap(depth + 1, lateral, &w, &h);
            if (!rgba || w <= 0 || h <= 0) continue;
            const M11_V22_CellRect* rect = &kV22CellRects[depth][lateral + 1];
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

const char* m11_v22_inplace_draw_source_evidence(void) {
    return "m11_v22_shape_cache_pc34.c (per-cell V22 shape cache); "
           "dm1_v2_modern_assets_pc34.c (manifest path resolution); "
           "m11_v22_render_overlay_pc34.c (sibling overlay path, placeholder); "
           "pit/stairs/teleporter-field material routing and no wrong-wall fallback; "
           "include/dm1_v2_shape_runtime_pc34.h (shape variant enum); "
           "ReDMCSB DUNVIEW.C:6697-6816 (DM1 4x3 composition order); "
           "v22_inplace_cache.bin (build-time RGBA pack from PNG via PIL).";
}
