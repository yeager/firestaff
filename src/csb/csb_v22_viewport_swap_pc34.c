/*
 * csb_v22_viewport_swap_pc34.c
 *
 * Bounded CSB V2.2 per-cell modern-art swap. Pairs the per-cell V22
 * cache (raw_cell_type + direction) with the CSB 9-square viewport
 * state and exposes a single render-pass seam.
 *
 * V1 source ownership: this module never touches V1 state. When V22
 * is not the active presentation mode (modern pack missing, cache
 * not loaded, or presentation_mode != V22_MODERN), the render pass
 * is a no-op and the V1 draw pipeline stays in charge.
 *
 * The per-cell asset_id resolution goes through the new
 * csb_v22_inplace_get_bitmap_by_id(category, asset_id) helper
 * (sibling to dm2_v22_inplace_get_bitmap_by_id) so the per-cell
 * swap table can drive both the wall/floor/creature discriminator
 * AND the CSB-only narrative shapes (prison door, DSA scroll,
 * Lord Order statue, chaos rune marker) without touching the
 * sibling shape cache.
 *
 * Source-lock:
 *   CSBWin/Viewport.cpp:7290   (9-square viewport layout)
 *   ReDMCSB DUNVIEW.C F0128    (CSB viewport routing)
 *   ReDMCSB DUNGEON.C:35-44    (direction step tables)
 *   include/csb_v22_shapes.h   (parallel CSB_V22_ShapeType enum)
 *   include/csb_v22_modern_assets_pc34.h (asset pack paths + flags)
 *   include/csb_v22_inplace_draw_pc34.h (cache bitmap lookup)
 *   include/csb_v22_shape_cache_pc34.h (raw cell type store)
 *   csb_v22_inplace_draw_pc34.c (cache file format + bitmap blit)
 *   csb_v22_modern_assets_pc34.c (manifest path resolution)
 */

#include "csb_v22_viewport_swap_pc34.h"
#include "csb_v22_inplace_draw_pc34.h"
#include "csb_v22_shape_cache_pc34.h"
#include "csb_v22_modern_assets_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"

#include <string.h>

/* ── Per-cell asset_id + category table ──────────────────────────── */

/* Maps a CSB_V22_SwapShapeType to (manifest_category, asset_id).
 * The category strings match the manifest categories documented in
 * include/csb_v22_modern_assets_pc34.h. The asset_id strings are
 * the manifest entry keys consumed by the cache builder.
 *
 * CSB-only narrative shapes (PRISON_DOOR, DSA_SCROLL, LORD_ORDER,
 * CHAOS_RUNE) and CSB-only UI shapes (DSA_RUNE) are routed to
 * dedicated CSB-only categories ("chaos_runes", "dsa_scrolls",
 * "door_shapes", "champion_portraits") — the same categories the
 * csb_v22_modern_assets_pc34.{c,h} manifest documents. */
typedef struct {
    CSB_V22_SwapShapeType shape;
    const char*          category;
    const char*          asset_id;
} CSB_V22_SwapShapeMapping;

static const CSB_V22_SwapShapeMapping kCSBV22SwapMappingTable[] = {
    /* Walls (CSB 9-square) */
    { CSB_V22_SWAP_SHAPE_WALL_STRAIGHT,     "wall_shapes",  "wall_dungeon_01" },
    { CSB_V22_SWAP_SHAPE_WALL_CORNER_INNER, "wall_shapes",  "wall_dungeon_01" },
    { CSB_V22_SWAP_SHAPE_WALL_CORNER_OUTER, "wall_shapes",  "wall_dungeon_01" },
    { CSB_V22_SWAP_SHAPE_WALL_DOORWAY,      "wall_shapes",  "wall_dungeon_doorway_01" },
    { CSB_V22_SWAP_SHAPE_WALL_ALCOVE,       "wall_shapes",  "wall_dungeon_alcove_01" },
    { CSB_V22_SWAP_SHAPE_WALL_INSCRIPTION,  "wall_shapes",  "wall_dungeon_inscription_01" },

    /* Floors */
    { CSB_V22_SWAP_SHAPE_FLOOR_PLAIN,       "floor_shapes", "floor_plain_01" },
    { CSB_V22_SWAP_SHAPE_FLOOR_CRACKED,     "floor_shapes", "floor_cracked_01" },
    { CSB_V22_SWAP_SHAPE_FLOOR_MOSSY,       "floor_shapes", "floor_cracked_01" }, /* no mossy variant in v1.4.0 */
    { CSB_V22_SWAP_SHAPE_FLOOR_PIT,         "floor_shapes", "floor_pit_01" },
    { CSB_V22_SWAP_SHAPE_FLOOR_STAIRS_UP,   "floor_shapes", "floor_stairs_01" },
    { CSB_V22_SWAP_SHAPE_FLOOR_STAIRS_DOWN, "floor_shapes", "floor_stairs_01" },
    { CSB_V22_SWAP_SHAPE_FLOOR_DOOR,        "door_shapes",  "door_iron_portcullis_01" },

    /* Ceilings */
    { CSB_V22_SWAP_SHAPE_CEILING_PLAIN,     "floor_shapes", "ceiling_plain_01" },
    { CSB_V22_SWAP_SHAPE_CEILING_VAULTED,   "floor_shapes", "ceiling_plain_01" }, /* placeholder */

    /* Creatures / items */
    { CSB_V22_SWAP_SHAPE_CREATURE,             "creature_shapes", "creature_chaos_fiend_01" },
    { CSB_V22_SWAP_SHAPE_CREATURE_PROJECTILE,  "creature_shapes", "creature_chaos_fiend_01" },
    { CSB_V22_SWAP_SHAPE_ITEM,                 "creature_shapes", "creature_chaos_fiend_01" }, /* placeholder */
    { CSB_V22_SWAP_SHAPE_ITEM_FLOOR,           "creature_shapes", "creature_chaos_fiend_01" }, /* placeholder */
    { CSB_V22_SWAP_SHAPE_ITEM_PROJECTILE,      "creature_shapes", "creature_chaos_fiend_01" }, /* placeholder */

    /* Fields (CSB-only routing) */
    { CSB_V22_SWAP_SHAPE_FIELD_TELEPORTER,     "floor_shapes", "field_teleporter_01" },
    { CSB_V22_SWAP_SHAPE_FIELD_FLUXCAGE,       "floor_shapes", "field_chaos_rift_01" }, /* placeholder */
    { CSB_V22_SWAP_SHAPE_FIELD_EXPLOSION,      "floor_shapes", "field_explosion_01" },
    { CSB_V22_SWAP_SHAPE_FIELD_CHAOS_RIFT,     "floor_shapes", "field_chaos_rift_01" },

    /* UI chrome */
    { CSB_V22_SWAP_SHAPE_UI_CHROME,            "ui_chrome",         "ui_panel_01" },
    { CSB_V22_SWAP_SHAPE_UI_PORTRAIT,          "champion_portraits","champion_warrior_csb" },
    { CSB_V22_SWAP_SHAPE_UI_MESSAGE_LOG,       "ui_chrome",         "ui_message_log_01" },
    { CSB_V22_SWAP_SHAPE_UI_INVENTORY_GRID,    "ui_chrome",         "ui_inventory_01" },
    { CSB_V22_SWAP_SHAPE_UI_DSA_RUNE,          "chaos_runes",       "chaos_rune_01" }, /* CSB-only */

    /* Narrative (CSB-only) */
    { CSB_V22_SWAP_SHAPE_PRISON_DOOR,          "door_shapes",       "door_prison_01" },
    { CSB_V22_SWAP_SHAPE_DSA_SCROLL,           "dsa_scrolls",       "dsa_scroll_01" },
    { CSB_V22_SWAP_SHAPE_LORD_ORDER,           "champion_portraits","statue_lord_order_01" },
    { CSB_V22_SWAP_SHAPE_CHAOS_RUNE,           "chaos_runes",       "chaos_rune_marker_01" },

    /* Sentinel — must remain last. */
    { CSB_V22_SWAP_SHAPE_NONE,                 NULL,               NULL }
};

/* ── Module state ────────────────────────────────────────────────── */

/* Records the per-cell ShapeType so the asset_id lookup can stay
 * a tight O(N) table scan instead of recomputing the discriminator
 * every render pass. Repopulated by csb_v22_viewport_swap_update(). */
typedef struct {
    CSB_V22_SwapShapeType shapes[3][3];   /* D0..D2 x L/C/R */
    int                   direction;
    int                   populated;
} CSB_V22_SwapCellCache;

static CSB_V22_SwapCellCache g_csb_swap_cache;

/* Counter for tests/probes — accumulated by the render pass and
 * reset on every update(). */
static int g_csb_cells_painted = 0;

/* ── Discriminator ───────────────────────────────────────────────── */

/* csb_v22_swap_shape_for_cell — bounded CSB V1 square-type -> V22
 * swap-shape discriminator.
 *
 * The CSB V1 square type is an 8-bit field whose high bits carry
 * creature-vs-floor-vs-wall flags and low bits carry variant data.
 * The mapping below mirrors the CSB V1 square type documentation
 * (ReDMCSB DUNGEON.C:35-44 direction step tables + the CSBWin
 * Viewport.cpp:7290 grid mapping) and stays conservative in this
 * first cut. Sub-cell variant decoding (e.g. corner vs straight
 * wall) is a follow-up once real CSB V22 shape book art lands.
 *
 * Direction parameter is preserved in the API even though the first
 * cut does not consume it (per-direction shape variants are a
 * follow-up). */
CSB_V22_SwapShapeType csb_v22_swap_shape_for_cell(uint8_t raw_cell_type,
                                                    uint8_t direction) {
    (void)direction;

    /* High-bit discriminator: top bit set -> creature */
    if (raw_cell_type & 0x80) {
        /* Projectile creatures use a separate tag (top bit + 0x40). */
        if (raw_cell_type & 0x40) return CSB_V22_SWAP_SHAPE_CREATURE_PROJECTILE;
        return CSB_V22_SWAP_SHAPE_CREATURE;
    }

    /* Items: 0x40 + low nibble 0x01..0x0F (CSB V1 item flag) */
    if (raw_cell_type & 0x40) {
        uint8_t low = raw_cell_type & 0x0F;
        if (low == 0x00) return CSB_V22_SWAP_SHAPE_ITEM_FLOOR;
        if (low <= 0x04) return CSB_V22_SWAP_SHAPE_ITEM;
        return CSB_V22_SWAP_SHAPE_ITEM_PROJECTILE;
    }

    /* Door (0x20 bit + 0x10 bit off) */
    if (raw_cell_type & 0x20) return CSB_V22_SWAP_SHAPE_FLOOR_DOOR;

    /* Pit cells (0x10 bit + low nibble 0x00) */
    if ((raw_cell_type & 0x10) && (raw_cell_type & 0x0F) == 0x00) {
        return CSB_V22_SWAP_SHAPE_FLOOR_PIT;
    }

    /* Stairs: 0x10 + bit 0 (up vs down) */
    if (raw_cell_type & 0x10) {
        return (raw_cell_type & 0x01) ? CSB_V22_SWAP_SHAPE_FLOOR_STAIRS_DOWN
                                     : CSB_V22_SWAP_SHAPE_FLOOR_STAIRS_UP;
    }

    /* Otherwise: walls, plain/cracked/mossy floors, fields.
     * Distinguish by the low nibble (0..15). The first cut maps:
     *   0..3   -> wall variants
     *   4      -> floor plain
     *   5      -> floor cracked
     *   6      -> floor mossy
     *   7..11  -> walls (corner/alcove/inscription)
     *   12..14 -> fields / ceiling
     *   15     -> wall (default)
     */
    switch (raw_cell_type & 0x0F) {
        case 0:  return CSB_V22_SWAP_SHAPE_WALL_STRAIGHT;
        case 1:  return CSB_V22_SWAP_SHAPE_WALL_CORNER_INNER;
        case 2:  return CSB_V22_SWAP_SHAPE_WALL_CORNER_OUTER;
        case 3:  return CSB_V22_SWAP_SHAPE_WALL_DOORWAY;
        case 4:  return CSB_V22_SWAP_SHAPE_FLOOR_PLAIN;
        case 5:  return CSB_V22_SWAP_SHAPE_FLOOR_CRACKED;
        case 6:  return CSB_V22_SWAP_SHAPE_FLOOR_MOSSY;
        case 7:  return CSB_V22_SWAP_SHAPE_WALL_ALCOVE;
        case 8:  return CSB_V22_SWAP_SHAPE_WALL_INSCRIPTION;
        case 9:  return CSB_V22_SWAP_SHAPE_WALL_STRAIGHT;
        case 10: return CSB_V22_SWAP_SHAPE_WALL_STRAIGHT;
        case 11: return CSB_V22_SWAP_SHAPE_WALL_STRAIGHT;
        case 12: return CSB_V22_SWAP_SHAPE_FIELD_TELEPORTER;
        case 13: return CSB_V22_SWAP_SHAPE_FIELD_FLUXCAGE;
        case 14: return CSB_V22_SWAP_SHAPE_CEILING_PLAIN;
        case 15: return CSB_V22_SWAP_SHAPE_WALL_STRAIGHT;
        default: return CSB_V22_SWAP_SHAPE_FLOOR_PLAIN;        /* safe default */
    }
}

/* csb_v22_swap_asset_id_for_shape — single-row lookup over the
 * mapping table. Returns NULL when the shape has no mapping. The
 * returned string is owned by the static mapping table. */
const char* csb_v22_swap_asset_id_for_shape(CSB_V22_SwapShapeType shape) {
    int i;
    if (shape == CSB_V22_SWAP_SHAPE_NONE) return NULL;
    for (i = 0; kCSBV22SwapMappingTable[i].asset_id != NULL; ++i) {
        if (kCSBV22SwapMappingTable[i].shape == shape) {
            return kCSBV22SwapMappingTable[i].asset_id;
        }
    }
    return NULL;
}

/* csb_v22_swap_category_for_shape — single-row lookup over the
 * mapping table. Returns NULL when the shape has no mapping. The
 * returned string is owned by the static mapping table. */
const char* csb_v22_swap_category_for_shape(CSB_V22_SwapShapeType shape) {
    int i;
    if (shape == CSB_V22_SWAP_SHAPE_NONE) return NULL;
    for (i = 0; kCSBV22SwapMappingTable[i].category != NULL; ++i) {
        if (kCSBV22SwapMappingTable[i].shape == shape) {
            return kCSBV22SwapMappingTable[i].category;
        }
    }
    return NULL;
}

/* Internal helper: resolve (category, asset_id) for a shape. */
static int csb_v22_resolve_shape_mapping(CSB_V22_SwapShapeType shape,
                                          const char** out_category,
                                          const char** out_asset_id) {
    int i;
    if (out_category) *out_category = NULL;
    if (out_asset_id)  *out_asset_id  = NULL;
    if (shape == CSB_V22_SWAP_SHAPE_NONE) return 0;
    for (i = 0; kCSBV22SwapMappingTable[i].asset_id != NULL; ++i) {
        if (kCSBV22SwapMappingTable[i].shape == shape) {
            if (out_category) *out_category = kCSBV22SwapMappingTable[i].category;
            if (out_asset_id)  *out_asset_id  = kCSBV22SwapMappingTable[i].asset_id;
            return 1;
        }
    }
    return 0;
}

/* ── Update + activation ─────────────────────────────────────────── */

void csb_v22_viewport_swap_update(int direction,
                                    const unsigned char raw_cells[3][3]) {
    int d, l;
    g_csb_swap_cache.direction = direction;

    if (raw_cells) {
        /* Per-cell discriminator. */
        for (d = 0; d < 3; ++d) {
            for (l = 0; l < 3; ++l) {
                g_csb_swap_cache.shapes[d][l] =
                    csb_v22_swap_shape_for_cell(raw_cells[d][l], (uint8_t)direction);
            }
        }
    } else {
        /* No raw_cells: blank cache. */
        for (d = 0; d < 3; ++d) {
            for (l = 0; l < 3; ++l) {
                g_csb_swap_cache.shapes[d][l] = CSB_V22_SWAP_SHAPE_NONE;
            }
        }
    }

    g_csb_swap_cache.populated = 1;
    g_csb_cells_painted = 0;

    /* Forward the cells to the sibling shape cache so any other V22
     * consumer (overlay pass, debug inspector) sees the same
     * raw_cell_type + direction that the swap uses. The sibling
     * shape cache populates its own internal state from raw_cells +
     * direction, but only when V22 presentation mode is active —
     * see csb_v22_shape_cache_update() in csb_v22_shape_cache_pc34.c.
     *
     * We deliberately call the sibling only when our own
     * presentation_mode is V22 so the swap module remains the
     * source of truth for the per-cell discriminator and the
     * sibling cache is treated as a parallel viewer. */
    if (raw_cells && csb_v2_presentation_mode_is_v22()) {
        csb_v22_shape_cache_update(direction, raw_cells);
    }
}

int csb_v22_viewport_swap_populated(void) {
    return g_csb_swap_cache.populated;
}

int csb_v22_viewport_swap_active(void) {
    if (!g_csb_swap_cache.populated) return 0;
    if (!csb_v22_inplace_draw_active()) return 0;
    if (!csb_v22_get_installed()) return 0;
    if (csb_v22_best_available_shape_source(3) != CSB_V22_SHAPE_SOURCE_V2_MODERN) {
        return 0;
    }
    return 1;
}

int csb_v22_viewport_swap_cells_painted(void) {
    return g_csb_cells_painted;
}

/* ── Render ──────────────────────────────────────────────────────── */

/* Clamp helper. */
static int csb_v22_clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Paint a single RGBA bitmap into the framebuffer using nearest-
 * neighbor scaling. The bitmap is keyed by (category, asset_id)
 * via csb_v22_inplace_get_bitmap_by_id, which gives the per-cell
 * swap a clean direct lookup independent of the sibling shape
 * cache. Returns 1 on success, 0 on missing bitmap. */
static int csb_v22_paint_cell_for_shape(unsigned char* framebuffer, int fbW, int fbH,
                                          int dst_x, int dst_y, int dst_w, int dst_h,
                                          CSB_V22_SwapShapeType shape) {
    const char* category = NULL;
    const char* asset_id = NULL;
    int w = 0, h = 0;
    const uint32_t* rgba;
    int x, y;

    if (!csb_v22_resolve_shape_mapping(shape, &category, &asset_id)) return 0;
    if (!category || !asset_id) return 0;

    rgba = csb_v22_inplace_get_bitmap_by_id(category, asset_id, &w, &h);
    if (!rgba || w <= 0 || h <= 0) return 0;

    if (dst_w <= 0 || dst_h <= 0) return 0;
    for (y = 0; y < dst_h; ++y) {
        int sy = (y * h) / dst_h;
        if (sy >= h) sy = h - 1;
        int py = dst_y + y;
        if (py < 0 || py >= fbH) continue;
        for (x = 0; x < dst_w; ++x) {
            int sx = (x * w) / dst_w;
            if (sx >= w) sx = w - 1;
            uint32_t px = rgba[sy * w + sx];
            unsigned char r = (unsigned char)((px >> 16) & 0xFFu);
            unsigned char g = (unsigned char)((px >>  8) & 0xFFu);
            unsigned char b = (unsigned char)((px      ) & 0xFFu);
            /* Same 2-bit-per-channel quantizer as
             * csb_v22_inplace_render_pass (the existing V22 pass) and
             * dm2_v22_viewport_swap_render (the sibling module). */
            int ri = (r * 3 + 127) / 255;
            int gi = (g * 3 + 127) / 255;
            int bi = (b * 3 + 127) / 255;
            unsigned char idx = (unsigned char)((ri << 4) | (gi << 2) | bi);
            int px_x = dst_x + x;
            if (px_x < 0 || px_x >= fbW) continue;
            framebuffer[py * fbW + px_x] = idx;
        }
    }
    return 1;
}

int csb_v22_viewport_swap_render(unsigned char* framebuffer,
                                   int fbW, int fbH) {
    int cells_painted = 0;
    int depth, lateral;

    if (!framebuffer || fbW <= 0 || fbH <= 0) return 0;
    if (!csb_v22_viewport_swap_active()) return 0;
    if (!csb_v22_viewport_swap_populated()) return 0;

    /* CSB 9-square viewport: paint the 9 cells (D0..D2 x L/C/R).
     * The cell rects are sourced from the sibling shape cache so the
     * swap module shares the same coords with the (future) overlay
     * pass and the existing in-place render pass. */
    for (depth = 0; depth < 3; ++depth) {
        for (lateral = -1; lateral <= 1; ++lateral) {
            const CSB_V22_CellRect* rect =
                &csb_v22_kCellRects[depth][lateral + 1];
            CSB_V22_SwapShapeType shape =
                g_csb_swap_cache.shapes[depth][lateral + 1];
            int dx, dy, dw, dh;

            if (shape == CSB_V22_SWAP_SHAPE_NONE) continue;

            dx = csb_v22_clampi(rect->x,         0, fbW);
            dy = csb_v22_clampi(rect->y,         0, fbH);
            dw = csb_v22_clampi(rect->x + rect->w, 0, fbW) - dx;
            dh = csb_v22_clampi(rect->y + rect->h, 0, fbH) - dy;
            if (dw <= 0 || dh <= 0) continue;

            if (csb_v22_paint_cell_for_shape(framebuffer, fbW, fbH,
                                              dx, dy, dw, dh,
                                              shape)) {
                cells_painted++;
            }
        }
    }

    g_csb_cells_painted += cells_painted;
    return cells_painted;
}

/* ── Source evidence ─────────────────────────────────────────────── */

const char* csb_v22_viewport_swap_source_evidence(void) {
    return "CSBWin/Viewport.cpp:7290 (9-square viewport layout); "
           "ReDMCSB DUNVIEW.C F0128 (CSB viewport routing); "
           "ReDMCSB DUNGEON.C:35-44 (direction step tables N/E/S/W); "
           "ReDMCSB LIGHT.C F0212 (CSB torchlight + lighting model); "
           "ReDMCSB PANEL.C F0354 (CSB champion panel refresh); "
           "ReDMCSB COMMAND.C:108-113/254-291 (command dispatch); "
           "ReDMCSB CLIKMENU.C:142/180 (input click routing); "
           "include/csb_v22_shapes.h (parallel CSB_V22_ShapeType enum); "
           "include/csb_v22_modern_assets_pc34.h (asset pack paths + flags); "
           "include/csb_v22_inplace_draw_pc34.h (cache bitmap lookup); "
           "include/csb_v22_shape_cache_pc34.h (raw cell type store); "
           "csb_v22_inplace_draw_pc34.c (cache file format + bitmap blit); "
           "csb_v22_modern_assets_pc34.c (manifest path resolution); "
           "include/dm2_v22_viewport_swap_pc34.h (parallel DM2 swap module — same per-cell contract); "
           "v22_inplace_cache.bin (build-time RGBA pack from PNG via PIL).";
}
