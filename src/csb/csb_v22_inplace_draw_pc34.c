/*
 * csb_v22_inplace_draw_pc34.c
 *
 * CSB V2.2 GPU render path: V22 modern-art IN-PLACE bitmap cache.
 *
 * This is the guarded in-place path for a reviewed V2.2 artpack. It never
 * paints a generated overlay or cache fixture over source-owned V1 pixels.
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
#include "csb_v22_finished_art_material_gate_pc34.h"
#include "csb_v22_modern_assets_pc34.h"
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
/* The M11 CSB renderer publishes the active original PC3.4 palette before
 * F0128 runs.  Keep a private copy so this low-level cache never borrows a
 * transient renderer buffer. */
static uint8_t          g_v22_palette_rgb6[256][3];
static int              g_v22_palette_active = 0;

static void v22_discard_cache(void)
{
    if (g_v22_cache_buf && !g_v22_cache_mapped) {
        free(g_v22_cache_buf);
    }
    g_v22_cache_buf = NULL;
    g_v22_cache_size = 0u;
    g_v22_cache_mapped = 0;
    g_v22_bitmap_count = 0;
}

/* ── Hash helpers ──────────────────────────────────────────────── */

static uint32_t fnv1a_hash(const char* s) {
    uint32_t h = 2166136261u;
    while (*s) { h = (h ^ (uint8_t)*s++) * 16777619u; }
    return h;
}

/* FSV22C is written with Python's struct '<6I8x' by Artpack Studio. Decode
 * its wire integers explicitly so cache admission never depends on host byte
 * order. */
static uint32_t v22_read_u32le(const unsigned char *bytes)
{
    return (uint32_t)bytes[0] |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

static uint32_t fnv1a_bytes(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t index;

    if (!bytes || size == 0u) {
        return 0u;
    }
    for (index = 0u; index < size; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
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
    v22_discard_cache();
    if (!fp) return 0;
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    if (sz <= 0 || sz > 64L * 1024L * 1024L) {  /* hard cap 64 MB */
        fclose(fp); return 0;
    }
    fseek(fp, 0, SEEK_SET);
    g_v22_cache_size = (size_t)sz;
    g_v22_cache_buf = (unsigned char*)malloc(g_v22_cache_size);
    if (!g_v22_cache_buf) { fclose(fp); g_v22_cache_size = 0u; return 0; }
    if (fread(g_v22_cache_buf, 1, g_v22_cache_size, fp) != g_v22_cache_size) {
        free(g_v22_cache_buf); g_v22_cache_buf = NULL; g_v22_cache_size = 0;
        fclose(fp); return 0;
    }
    fclose(fp);
    g_v22_cache_mapped = 0;

    /* Parse header */
    if (g_v22_cache_size < FSV22C_HDR_SIZE) goto reject;
    if (memcmp(g_v22_cache_buf, FSV22C_MAGIC, 8) != 0) goto reject;
    uint32_t version = v22_read_u32le(g_v22_cache_buf + 8);
    uint32_t count = v22_read_u32le(g_v22_cache_buf + 12);
    if (version != FSV22C_VERSION || count == 0u || count > FSV22C_MAX_ENT) goto reject;

    /* Parse entries + index RGBA pointers */
    size_t entries_off = FSV22C_HDR_SIZE;
    size_t data_off    = FSV22C_HDR_SIZE + (size_t)count * FSV22C_ENT_SIZE;
    if (data_off > g_v22_cache_size) goto reject;

    g_v22_bitmap_count = 0;
    for (uint32_t i = 0; i < count; ++i) {
        const unsigned char* ep = g_v22_cache_buf + entries_off + i * FSV22C_ENT_SIZE;
        FsV22CacheEntry e;
        e.category_hash = v22_read_u32le(ep + 0);
        e.asset_id_hash = v22_read_u32le(ep + 4);
        e.width = v22_read_u32le(ep + 8);
        e.height = v22_read_u32le(ep + 12);
        e.rgba_size = v22_read_u32le(ep + 16);
        e.rgba_offset = v22_read_u32le(ep + 20);
        size_t expected_rgba_size;
        if (e.width == 0u || e.height == 0u ||
            (size_t)e.width > SIZE_MAX / (size_t)e.height ||
            (size_t)e.width * (size_t)e.height > SIZE_MAX / 4u) goto reject;
        expected_rgba_size = (size_t)e.width * (size_t)e.height * 4u;
        if ((size_t)e.rgba_size != expected_rgba_size ||
            (size_t)e.rgba_offset < data_off ||
            (size_t)e.rgba_offset > g_v22_cache_size ||
            (size_t)e.rgba_size > g_v22_cache_size - (size_t)e.rgba_offset) goto reject;
        for (int previous = 0; previous < g_v22_bitmap_count; ++previous) {
            const FsV22CacheEntry *old = &g_v22_bitmaps[previous].entry;
            const size_t old_offset = (size_t)old->rgba_offset;
            const size_t old_size = (size_t)old->rgba_size;
            const size_t new_offset = (size_t)e.rgba_offset;
            const size_t new_size = (size_t)e.rgba_size;

            /* The source cache writer emits one distinct, contiguous RGBA
             * span per manifest asset.  A duplicate key would make lookup
             * order decide pixels; an overlap would let one asset alias
             * another's material.  Neither has original-pack provenance. */
            if ((old->category_hash == e.category_hash &&
                 old->asset_id_hash == e.asset_id_hash) ||
                (new_offset < old_offset + old_size &&
                 old_offset < new_offset + new_size)) goto reject;
        }
        g_v22_bitmaps[g_v22_bitmap_count].entry = e;
        g_v22_bitmaps[g_v22_bitmap_count].rgba = g_v22_cache_buf + e.rgba_offset;
        g_v22_bitmap_count++;
    }
    return g_v22_bitmap_count == (int)count ? 1 : 0;
reject:
    v22_discard_cache();
    return 0;
}

int csb_v22_inplace_draw_init(void) {
    if (g_v22_inplace_active) return 1;

    /* Resolve cache path from manifest path */
    char cache_path[FSP_PATH_MAX];
    {
        /* The cache is part of the chosen pack. Do not reconstruct a HOME
         * path here: launcher-selected artpack roots can be elsewhere. */
        char manifest_path[FSP_PATH_MAX];
        const char* configured_manifest = csb_v22_get_manifest_path();
        if (!configured_manifest || configured_manifest[0] == '\0') return 0;
        snprintf(manifest_path, sizeof(manifest_path), "%s", configured_manifest);
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
    v22_discard_cache();
    g_v22_inplace_active = 0;
    csb_v22_inplace_draw_clear_indexed_palette();
}

int csb_v22_inplace_draw_active(void) {
    return g_v22_inplace_active;
}

int csb_v22_inplace_draw_set_indexed_palette_rgb6(
    const uint8_t rgb6[256][3])
{
    if (!rgb6) {
        return 0;
    }
    memcpy(g_v22_palette_rgb6, rgb6, sizeof(g_v22_palette_rgb6));
    g_v22_palette_active = 1;
    return 1;
}

void csb_v22_inplace_draw_clear_indexed_palette(void)
{
    g_v22_palette_active = 0;
    memset(g_v22_palette_rgb6, 0, sizeof(g_v22_palette_rgb6));
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

/* RGB art is projected into the same current CSB palette that the original
 * indexed F0128 passes use.  The native PC3.4 page intentionally duplicates
 * its 16 source colours over the 256 byte index range; choosing the first
 * nearest entry gives deterministic original indices and avoids the old
 * unrelated EGA-cube colours. */
static unsigned char rgb_to_source_palette_index(unsigned char r,
                                                 unsigned char g,
                                                 unsigned char b)
{
    unsigned int best_distance = UINT32_MAX;
    unsigned char best_index = 0;
    int index;

    for (index = 0; index < 256; ++index) {
        int pr = (int)((g_v22_palette_rgb6[index][0] << 2) |
                       (g_v22_palette_rgb6[index][0] >> 4));
        int pg = (int)((g_v22_palette_rgb6[index][1] << 2) |
                       (g_v22_palette_rgb6[index][1] >> 4));
        int pb = (int)((g_v22_palette_rgb6[index][2] << 2) |
                       (g_v22_palette_rgb6[index][2] >> 4));
        int dr = (int)r - pr;
        int dg = (int)g - pg;
        int db = (int)b - pb;
        unsigned int distance = (unsigned int)(dr * dr + dg * dg + db * db);
        if (distance < best_distance) {
            best_distance = distance;
            best_index = (unsigned char)index;
            if (distance == 0u) {
                break;
            }
        }
    }
    return best_index;
}

/* Nearest-neighbor blit of RGBA bitmap into framebuffer[y*fbW+x]
 * sized src_w x src_h -> dst_w x dst_h. Fully transparent source pixels
 * leave the source-owned F0128 framebuffer intact; this is required for
 * C10_COLOR_FLESH door holes. Opaque pixels are mapped to a single byte via
 * the current CSB source palette. Without that source-owned palette the
 * command is rejected; a generated EGA fallback would alter F0128 output. */
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
            unsigned char a = (unsigned char)((px >> 24) & 0xFFu);
            if (a == 0u) continue;
            unsigned char r = (unsigned char)((px >> 16) & 0xFFu);
            unsigned char g = (unsigned char)((px >>  8) & 0xFFu);
            unsigned char b = (unsigned char)((px      ) & 0xFFu);
            unsigned char idx = rgb_to_source_palette_index(r, g, b);
            int px_x = dst_x + x;
            if (px_x < 0 || px_x >= fbW) continue;
            framebuffer[py * fbW + px_x] = idx;
        }
    }
}

int csb_v22_inplace_render_f0128_command(
    const CSB_V1_ViewportRuntimeDrawCommandPc34* source_command,
    unsigned char* framebuffer, int fbW, int fbH)
{
    char dynamic_asset_id[CSB_V22_ASSET_ID_MAX];
    const char* asset_id = NULL;
    CSB_V22_RouteProvenancePc34 provenance;
    CSB_V22_F0128ProjectionCommandPc34 projection;
    const uint32_t* rgba;
    int width = 0;
    int height = 0;

    if (!source_command || !framebuffer || fbW <= 0 || fbH <= 0 ||
        !csb_v22_inplace_draw_active() || !g_v22_palette_active ||
        /* A readable cache is not proof that its RGBA pixels are authentic.
         * The full pack must still satisfy every F0128 route/material gate. */
        !csb_v22_famg_is_finished_real()) {
        return 0;
    }

    /* ReDMCSB's known PC3.4 front-door commands have distinct source record
     * identities. No other command can enter this path until it has the same
     * F0128 placement/provenance receipt. */
    switch (source_command->route) {
    case CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D1_F0111_DOOR_PC34:
        asset_id = "door_d0_01";
        break;
    case CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D2_F0111_DOOR_PC34:
        asset_id = "door_d1_01";
        break;
    case CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3L2_F0111_DOOR_PC34:
    case CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3R2_F0111_DOOR_PC34:
        asset_id = "door_d2_01";
        break;
    default:
        return 0;
    }

    dynamic_asset_id[0] = '\0';
    if (source_command->source_graphics_item_index > 0 &&
        (source_command->route == CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D1_F0111_DOOR_PC34 ||
         source_command->route == CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D2_F0111_DOOR_PC34 ||
         source_command->route == CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3L2_F0111_DOOR_PC34 ||
         source_command->route == CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3R2_F0111_DOOR_PC34)) {
        int offset = source_command->route ==
                     CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D1_F0111_DOOR_PC34 ? 2 :
                     source_command->route ==
                     CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D2_F0111_DOOR_PC34 ? 1 : 0;
        int set = (source_command->source_graphics_item_index - 246 - offset) / 3;
        if (set < 0 || set > 3 ||
            source_command->source_graphics_item_index != 246 + set * 3 + offset) {
            return 0;
        }
        if (set != 0) {
            (void)snprintf(dynamic_asset_id, sizeof(dynamic_asset_id),
                           "door_set_%d_d%d", set,
                           offset == 0 ? 3 : offset == 1 ? 2 : 1);
            asset_id = dynamic_asset_id;
        }
    }

    memset(&provenance, 0, sizeof(provenance));
    memset(&projection, 0, sizeof(projection));
    if (!csb_v22_get_route_provenance("door_shapes", asset_id, &provenance) ||
        !csb_v22_admit_f0128_door_projection_pc34(source_command,
                                                    &provenance,
                                                    &projection) ||
        !projection.valid || projection.clip_x < 0 || projection.clip_y < 0 ||
        projection.clip_w <= 0 || projection.clip_h <= 0 ||
        projection.clip_x + projection.clip_w > fbW ||
        projection.clip_y + projection.clip_h > fbH) {
        return 0;
    }

    /* The V2.2 pack may replace a door only after the V1 command carries
     * the decoded original raster selected by ReDMCSB F0111.  A route/index
     * tuple and a manifest hash alone can be fabricated by a caller; require
     * the exact source span that the checked F0128 byte handoff validated. */
    if (!source_command->decoded_pixels || source_command->decoded_width <= 0 ||
        source_command->decoded_height <= 0 ||
        source_command->decoded_width != projection.source_width ||
        source_command->decoded_height != projection.source_height ||
        source_command->decoded_size !=
            (size_t)source_command->decoded_width *
                (size_t)source_command->decoded_height ||
        source_command->material_hash == 0u ||
        source_command->material_hash != fnv1a_bytes(
            source_command->decoded_pixels, source_command->decoded_size)) {
        return 0;
    }

    rgba = csb_v22_inplace_get_bitmap_by_id(projection.category,
                                              projection.asset_id,
                                              &width, &height);
    if (!rgba || width <= 0 || height <= 0) {
        return 0;
    }
    if (source_command->route ==
            CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3L2_F0111_DOOR_PC34 ||
        source_command->route ==
            CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3R2_F0111_DOOR_PC34) {
        /* F0791 uses the source bitmap dimensions.  The layout clip is 48x40
         * but PC/I34's real G0693 surface is 44x38, so scaling it to the
         * whole clip would invent pixels and overwrite source background. */
        if (width != projection.source_width || height != projection.source_height ||
            width > projection.clip_w || height > projection.clip_h) return 0;
        blit_bitmap_to_cell(rgba, width, height, framebuffer, fbW, fbH,
                            projection.clip_x, projection.clip_y, width, height);
    } else {
        blit_bitmap_to_cell(rgba, width, height, framebuffer, fbW, fbH,
                            projection.clip_x, projection.clip_y,
                            projection.clip_w, projection.clip_h);
    }
    return 1;
}

const char* csb_v22_inplace_draw_source_evidence(void) {
    return "csb_v22_shape_cache_pc34.c (per-cell V22 shape cache); "
           "dm1_v2_modern_assets_pc34.c (manifest path resolution); "
           "csb_v22_shape_cache_pc34.c (sibling cache + CSB_V22_CellRect coords); "
           "csb_v22_modern_assets_pc34.c (manifest path resolution); "
           "csb_v22_finished_art_material_gate_pc34.c (route admission); "
           "ReDMCSB DUNVIEW.C F0128 (CSB viewport routing); "
           "CSBWin/Viewport.cpp:7290 (9-square layout); "
           "include/dm1_v2_shape_runtime_pc34.h (shape variant enum pattern); "
           "v22_inplace_cache.bin (admitted artpack cache only).";
}
