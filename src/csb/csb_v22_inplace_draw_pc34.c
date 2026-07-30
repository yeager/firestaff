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
#include "csb_v22_modern_assets_pc34.h"
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
/* The M11 CSB renderer publishes the active original PC3.4 palette before
 * F0128 runs.  Keep a private copy so this low-level cache never borrows a
 * transient renderer buffer. */
static uint8_t          g_v22_palette_rgb6[256][3];
static int              g_v22_palette_active = 0;

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
    if (g_v22_cache_buf && !g_v22_cache_mapped) free(g_v22_cache_buf);
    g_v22_cache_buf = NULL;
    g_v22_cache_size = 0;
    g_v22_cache_mapped = 0;
    g_v22_bitmap_count = 0;
    g_v22_inplace_active = 0;
    memset(g_csb_v22_inplace_asset_mirror_valid, 0,
           sizeof(g_csb_v22_inplace_asset_mirror_valid));
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

int csb_v22_inplace_render_pass(unsigned char* framebuffer, int fbW, int fbH) {
    /* This old grid painter has no ReDMCSB F0128 command, clipping or
     * draw-order receipt.  It used to replace arbitrary 3x3 cells with a
     * stretched pack bitmap, which can hide source walls, ornaments and
     * Things.  Keep the ABI while failing closed: the only admitted V2.2
     * compositor is csb_v22_inplace_render_f0128_command(), called directly
     * from the authenticated source command stream. */
    (void)framebuffer;
    (void)fbW;
    (void)fbH;
    return 0;
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
        !csb_v22_inplace_draw_active() || !g_v22_palette_active) {
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
           "csb_v22_shapes.c (CSB V22 shape book); "
           "ReDMCSB DUNVIEW.C F0128 (CSB viewport routing); "
           "CSBWin/Viewport.cpp:7290 (9-square layout); "
           "include/dm1_v2_shape_runtime_pc34.h (shape variant enum pattern); "
           "v22_inplace_cache.bin (build-time RGBA pack from PNG via PIL).";
}
