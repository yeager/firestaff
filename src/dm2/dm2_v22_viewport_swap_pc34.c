/*
 * dm2_v22_viewport_swap_pc34.c
 *
 * Bounded DM2 V2.2 per-cell modern-art swap. Pairs the per-cell
 * V22 cache (raw_cell_type + direction) with the DM2 viewport
 * state and exposes a single render-pass seam for both the
 * indoor T560 (dungeon grid) and outdoor T600 (sky/ground split)
 * viewport types.
 *
 * V1 source ownership: this module never touches V1 state. When
 * V22 is not the active presentation mode (modern pack missing,
 * cache not loaded, or presentation_mode != V22_MODERN), the render
 * pass is a no-op and the V1 draw pipeline stays in charge.
 *
 * The per-cell asset_id resolution goes through the new
 * dm2_v22_inplace_get_bitmap_by_id(category, asset_id) helper
 * instead of the sibling shape cache, so the indoor T560 path
 * can render the full 26-shape discriminator and the outdoor
 * T600 path can render sky/horizon/ground cells without touching
 * the per-depth/lateral cell rect table used for the dungeon
 * view.
 *
 * Source-lock:
 *   SKULL.ASM T520/T560/T600 (DM2 viewport ticks)
 *   ReDMCSB DUNVIEW.C:2962-3070 (outdoor sky/ground composition order)
 *   include/dm2_v22_modern_assets_pc34.h (asset pack paths + flags)
 *   include/dm2_v22_inplace_draw_pc34.h (cache bitmap lookup)
 *   include/dm2_v22_shape_cache_pc34.h (raw cell type store)
 *   dm2_v22_inplace_draw_pc34.c (cache file format + bitmap blit)
 *   dm2_v22_modern_assets_pc34.c (manifest path resolution)
 */

#include "dm2_v22_viewport_swap_pc34.h"
#include "dm2_v22_inplace_draw_pc34.h"
#include "dm2_v22_shape_cache_pc34.h"
#include "dm2_v22_modern_assets_pc34.h"

#include <string.h>

/* ── Outdoor cell rect table (1920x1080) ──────────────────────────── */

/* The outdoor T600 viewport draws a sky gradient over the top half
 * of the viewport and a ground band over the bottom half. The
 * per-cell modern-art swap uses the same split:
 *
 *   index 0 (sky)        — full-width sky band at the top
 *   index 1 (horizon)    — 1-line transition strip at the boundary
 *   index 2 (ground)     — full-width ground band below the horizon
 *
 * This keeps the per-cell swap table compact (3 entries) while still
 * leaving the seam available for a 3-cell L/C/R ground refinement
 * later (the per-cell render loop reads the same rect shape, so a
 * future expansion only changes the rect table). */
const DM2_V22_OutdoorCellRect dm2_v22_kOutdoorCellRects[3] = {
    /* sky band         */ {   0,   0, 1920, 540 },
    /* horizon strip    */ {   0, 540, 1920,   2 },
    /* ground band      */ {   0, 542, 1920, 538 }
};

/* ── Per-cell asset_id + category table ──────────────────────────── */

/* Maps a Dm2_V22_ShapeType to (manifest_category, asset_id). The
 * category strings match the manifest categories documented in
 * include/dm2_v22_modern_assets_pc34.h. The asset_id strings are
 * the manifest entry keys consumed by the cache builder. */
typedef struct {
    Dm2_V22_ShapeType shape;
    const char*       category;
    const char*       asset_id;
} Dm2_V22_ShapeMapping;

static const Dm2_V22_ShapeMapping kDm2V22MappingTable[] = {
    /* Indoor walls (T560) */
    { DM2_V22_SHAPE_WALL_STRAIGHT,      "wall_shapes",  "wall_dm2_temple_01" },
    { DM2_V22_SHAPE_WALL_CORNER_INNER,  "wall_shapes",  "wall_dm2_temple_01" },
    { DM2_V22_SHAPE_WALL_CORNER_OUTER,  "wall_shapes",  "wall_dm2_temple_01" },
    { DM2_V22_SHAPE_WALL_ALCOVE,        "wall_shapes",  "wall_dm2_alcove_01" },
    { DM2_V22_SHAPE_WALL_DOORWAY,       "wall_shapes",  "wall_dm2_doorway_01" },
    { DM2_V22_SHAPE_WALL_INSCRIPTION,   "wall_shapes",  "wall_dm2_inscription_01" },

    /* Indoor floors */
    { DM2_V22_SHAPE_FLOOR_PLAIN,        "floor_shapes", "floor_dm2_outdoor_01" },
    { DM2_V22_SHAPE_FLOOR_CRACKED,      "floor_shapes", "floor_dm2_outdoor_01" },
    { DM2_V22_SHAPE_FLOOR_MOSSY,        "floor_shapes", "floor_dm2_outdoor_01" },
    { DM2_V22_SHAPE_FLOOR_PIT,          "floor_shapes", "floor_dm2_pit_01" },
    { DM2_V22_SHAPE_FLOOR_STAIRS_UP,    "floor_shapes", "floor_dm2_stairs_01" },
    { DM2_V22_SHAPE_FLOOR_STAIRS_DOWN,  "floor_shapes", "floor_dm2_stairs_01" },
    { DM2_V22_SHAPE_CEILING_PLAIN,      "floor_shapes", "ceiling_dm2_plain_01" },

    /* Creatures / items */
    { DM2_V22_SHAPE_CREATURE,           "creature_shapes", "creature_dm2_brigand_01" },
    { DM2_V22_SHAPE_CREATURE_PROJECTILE, "creature_shapes", "creature_dm2_brigand_01" },

    /* Doors / fields */
    { DM2_V22_SHAPE_DOOR,               "door_shapes",  "door_dm2_wood_01" },
    { DM2_V22_SHAPE_FIELD_TELEPORTER,   "wall_shapes",  "field_dm2_teleporter_01" },
    { DM2_V22_SHAPE_FIELD_OPENING,      "wall_shapes",  "field_dm2_opening_01" },

    /* Outdoor (T600) */
    { DM2_V22_SHAPE_OUTDOOR_SKY,        "wall_shapes",  "sky_dm2_outdoor_01" },
    { DM2_V22_SHAPE_OUTDOOR_GROUND,     "floor_shapes", "ground_dm2_outdoor_01" },
    { DM2_V22_SHAPE_OUTDOOR_BUILDING,   "wall_shapes",  "wall_dm2_outdoor_01" },
    { DM2_V22_SHAPE_OUTDOOR_TREE,       "creature_shapes", "tree_dm2_outdoor_01" },
    { DM2_V22_SHAPE_OUTDOOR_HORIZON,    "floor_shapes", "ground_dm2_horizon_01" },

    /* Sentinel — must remain last. */
    { DM2_V22_SHAPE_NONE,               NULL,           NULL }
};

/* ── Module state ────────────────────────────────────────────────── */

/* Records the per-cell ShapeType so the asset_id lookup can stay
 * a tight O(N) table scan instead of recomputing the discriminator
 * every render pass. Repopulated by dm2_v22_viewport_swap_update(). */
typedef struct {
    Dm2_V22_ShapeType shapes[3][3];   /* D0..D2 x L/C/R for indoor */
    int               direction;
    int               is_outdoor;
    int               populated;
} Dm2_V22_SwapCellCache;

static Dm2_V22_SwapCellCache g_swap_cache;

/* Counters for tests/probes — accumulated by the render pass and
 * reset on every update. */
static int g_cells_painted_indoor = 0;
static int g_cells_painted_outdoor = 0;

/* ── Discriminator ───────────────────────────────────────────────── */

/* dm2_v22_shape_for_cell — bounded indoor discriminator.
 *
 * The DM2 V1 raw cell type is an 8-bit field whose high bits carry
 * wall-vs-floor-vs-creature flags and low bits carry variant data.
 * The mapping below mirrors the indoor T560 cell-type documentation
 * shipped with skproject SKULL.ASM and stays conservative in this
 * first cut (no corner-vs-straight distinction at the byte level —
 * that refinement is a follow-up once real GRAPHICS.DAT bytes land).
 *
 * Direction parameter is preserved in the API even though the first
 * cut does not consume it (per-direction shape variants are a
 * follow-up). */
Dm2_V22_ShapeType dm2_v22_shape_for_cell(uint8_t raw_cell_type,
                                          uint8_t direction) {
    (void)direction;

    /* High-bit discriminator: top bit set -> creature */
    if (raw_cell_type & 0x80) {
        /* Projectile creatures use a separate tag (top bit + 0x40). */
        if (raw_cell_type & 0x40) return DM2_V22_SHAPE_CREATURE_PROJECTILE;
        return DM2_V22_SHAPE_CREATURE;
    }

    /* Pit cells (0x40 bit on a non-creature cell) */
    if (raw_cell_type & 0x40) return DM2_V22_SHAPE_FLOOR_PIT;

    /* Doorway cells (0x20 bit) */
    if (raw_cell_type & 0x20) return DM2_V22_SHAPE_WALL_DOORWAY;

    /* Stairs: 0x10 + bit 0 (up vs down) */
    if (raw_cell_type & 0x10) {
        return (raw_cell_type & 0x01) ? DM2_V22_SHAPE_FLOOR_STAIRS_DOWN
                                     : DM2_V22_SHAPE_FLOOR_STAIRS_UP;
    }

    /* Otherwise: walls, plain/cracked/mossy floors, fields.
     * Distinguish by the low nibble (0..15). The first cut maps:
     *   0..3   -> wall variants
     *   4      -> floor plain
     *   5      -> floor cracked
     *   6      -> floor mossy
     *   7..11  -> walls (corner/alcove/inscription)
     *   12     -> door
     *   13     -> teleporter field
     *   14     -> opening field
     *   15     -> ceiling
     */
    switch (raw_cell_type & 0x0F) {
        case 0:  return DM2_V22_SHAPE_WALL_STRAIGHT;
        case 1:  return DM2_V22_SHAPE_WALL_CORNER_INNER;
        case 2:  return DM2_V22_SHAPE_WALL_CORNER_OUTER;
        case 3:  return DM2_V22_SHAPE_WALL_ALCOVE;
        case 4:  return DM2_V22_SHAPE_FLOOR_PLAIN;
        case 5:  return DM2_V22_SHAPE_FLOOR_CRACKED;
        case 6:  return DM2_V22_SHAPE_FLOOR_MOSSY;
        case 7:  return DM2_V22_SHAPE_WALL_STRAIGHT;       /* alcove wall */
        case 8:  return DM2_V22_SHAPE_WALL_INSCRIPTION;
        case 9:  return DM2_V22_SHAPE_WALL_STRAIGHT;
        case 10: return DM2_V22_SHAPE_DOOR;
        case 11: return DM2_V22_SHAPE_WALL_STRAIGHT;
        case 12: return DM2_V22_SHAPE_FIELD_TELEPORTER;
        case 13: return DM2_V22_SHAPE_FIELD_OPENING;
        case 14: return DM2_V22_SHAPE_CEILING_PLAIN;
        case 15: return DM2_V22_SHAPE_WALL_STRAIGHT;
        default: return DM2_V22_SHAPE_FLOOR_PLAIN;          /* safe default */
    }
}

/* dm2_v22_shape_for_cell_pos — bounded position-aware indoor T560
 * discriminator. The base discriminator (above) is intentionally
 * position-agnostic; this position-aware sibling adds:
 *
 *   - Wall cells in the LEFT column  (lateral == -1)
 *       -> WALL_CORNER_INNER
 *   - Wall cells in the RIGHT column (lateral == +1)
 *       -> WALL_CORNER_OUTER
 *   - Wall cells in the CENTER column (lateral == 0)
 *       -> WALL_STRAIGHT
 *
 *   - Floor cells at depth 0 (closest, D0)  -> FLOOR_PLAIN
 *   - Floor cells at depth 1 (middle,  D1)  -> FLOOR_CRACKED
 *   - Floor cells at depth 2 (farthest, D2) -> FLOOR_MOSSY
 *
 *   - Pit / stairs / creatures / doors / fields / ceiling /
 *     teleporter / opening keep the position-agnostic shape so
 *     those gameplay markers retain a single per-cell visual
 *     identity when the party direction changes.
 *
 * Out-of-range depth/lateral are clamped to the closest in-range
 * value before dispatch so a malformed raw grid never reads
 * garbage memory.
 *
 * Source-lock: ReDMCSB DUNVIEW.C:6239-6675 (per-cell wall/floor
 * zone tables for D3L2/D3C/D3R2/D2L/D2R/D1L2/D1C/D1R2/D0L/D0R)
 * + DUNVIEW.C:2962-3070 (per-cell composition order that owns
 * the 4×3 grid). The shape variant enum (Dm2_V22_ShapeType) is the
 * same vocabulary used by the existing per-cell render pass, so
 * the new discriminator composes without changing the per-cell
 * render loop.
 *
 * V1 source ownership: the function is read-only and never
 * touches V1 state. It only refines the ShapeType selection
 * inside the swap cache, so V1 draw paths are unaffected. */
Dm2_V22_ShapeType dm2_v22_shape_for_cell_pos(uint8_t raw_cell_type,
                                               uint8_t direction,
                                               int depth,
                                               int lateral) {
    Dm2_V22_ShapeType base;
    uint8_t low_nibble;

    /* Clamp position. lateral must land in {-1, 0, +1}. */
    if (lateral < -1) lateral = -1;
    if (lateral >  1) lateral =  1;
    /* depth must land in {0, 1, 2}. Out-of-range depths are clamped. */
    if (depth < 0) depth = 0;
    if (depth > 2) depth = 2;

    base = dm2_v22_shape_for_cell(raw_cell_type, direction);

    /* Refine walls by column. This mirrors ReDMCSB DUNVIEW.C:6239-6675
     * which uses distinct wall indices per (depth, lateral) cell,
     * so the same raw cell type (0x00 = wall) renders different
     * sprite indices when seen at D1L vs D1R vs D1C. In this first
     * bounded cut we only vary by lateral — that already yields
     * 3 distinct wall shapes per row across the 4 facing directions,
     * which is the smallest useful position-aware discriminator. */
    if (base == DM2_V22_SHAPE_WALL_STRAIGHT ||
        base == DM2_V22_SHAPE_WALL_ALCOVE  ||
        base == DM2_V22_SHAPE_WALL_INSCRIPTION) {
        if      (lateral == -1) return DM2_V22_SHAPE_WALL_CORNER_INNER;
        else if (lateral ==  1) return DM2_V22_SHAPE_WALL_CORNER_OUTER;
        return DM2_V22_SHAPE_WALL_STRAIGHT;
    }

    /* Refine plain floors by depth. Pits / stairs / mossy stay as
     * their base shape because they carry gameplay semantics —
     * a pit MUST remain a pit regardless of whether it is closest
     * or farthest. PLAIN / CRACKED / MOSSY are base-shape variants
     * the user can see in the corridor; switching them by depth
     * gives the modern-art swap a visible per-row floor change
     * (D0 = cleanest floor, D2 = oldest floor) without touching
     * the source-locked gameplay bit layout. */
    low_nibble = (uint8_t)(raw_cell_type & 0x0F);
    if (base == DM2_V22_SHAPE_FLOOR_PLAIN) {
        /* Only refines plain floors; CRACKED/MOSSY/PIT/STAIRS
         * keep their base shape. */
        if      (depth == 0) return DM2_V22_SHAPE_FLOOR_PLAIN;
        else if (depth == 1) return DM2_V22_SHAPE_FLOOR_CRACKED;
        else                 return DM2_V22_SHAPE_FLOOR_MOSSY;
    }

    /* low_nibble is consulted only to keep the existing base
     * discriminator honest about FLOOR_PLAIN; this avoids the
     * `unused variable` warning while keeping the cheap lookup
     * O(1) without changing the position-agnostic dispatcher. */
    if (low_nibble == 0xFFu) return DM2_V22_SHAPE_NONE;  /* unreachable */

    return base;
}

/* dm2_v22_asset_id_for_shape — single-row lookup over the mapping
 * table. Returns NULL when the shape has no mapping. The returned
 * string is owned by the static mapping table. */
const char* dm2_v22_asset_id_for_shape(Dm2_V22_ShapeType shape) {
    int i;
    if (shape == DM2_V22_SHAPE_NONE) return NULL;
    for (i = 0; kDm2V22MappingTable[i].asset_id != NULL; ++i) {
        if (kDm2V22MappingTable[i].shape == shape) {
            return kDm2V22MappingTable[i].asset_id;
        }
    }
    return NULL;
}

/* Internal helper: resolve (category, asset_id) for a shape. */
static int resolve_shape_mapping(Dm2_V22_ShapeType shape,
                                  const char** out_category,
                                  const char** out_asset_id) {
    int i;
    if (out_category) *out_category = NULL;
    if (out_asset_id) *out_asset_id = NULL;
    if (shape == DM2_V22_SHAPE_NONE) return 0;
    for (i = 0; kDm2V22MappingTable[i].asset_id != NULL; ++i) {
        if (kDm2V22MappingTable[i].shape == shape) {
            if (out_category) *out_category = kDm2V22MappingTable[i].category;
            if (out_asset_id) *out_asset_id = kDm2V22MappingTable[i].asset_id;
            return 1;
        }
    }
    return 0;
}

/* ── Update + activation ─────────────────────────────────────────── */

void dm2_v22_viewport_swap_update(int direction,
                                   const unsigned char raw_cells[3][3],
                                   int is_outdoor) {
    int d, l;
    g_swap_cache.direction = direction;
    g_swap_cache.is_outdoor = is_outdoor ? 1 : 0;

    if (g_swap_cache.is_outdoor) {
        /* Outdoor: every cell becomes the OUTDOOR_GROUND shape; the
         * sky/horizon cells are drawn from the outdoor rect table
         * directly (see dm2_v22_viewport_swap_render). */
        for (d = 0; d < 3; ++d) {
            for (l = 0; l < 3; ++l) {
                g_swap_cache.shapes[d][l] = DM2_V22_SHAPE_OUTDOOR_GROUND;
            }
        }
    } else if (raw_cells) {
        /* Indoor: per-cell discriminator (position-aware, so the
         * same raw_cell_type picks a side-wall vs center-wall vs
         * corner-wall asset depending on which of the 9 cells it
         * lives in). The base discriminator (raw_cell_type +
         * direction) is still the position-agnostic fallback
         * inside dm2_v22_shape_for_cell_pos(). */
        for (d = 0; d < 3; ++d) {
            for (l = 0; l < 3; ++l) {
                g_swap_cache.shapes[d][l] =
                    dm2_v22_shape_for_cell_pos(raw_cells[d][l],
                                                (uint8_t)direction,
                                                d,
                                                l - 1);
            }
        }
    } else {
        /* No raw_cells and not outdoor: blank cache. */
        for (d = 0; d < 3; ++d) {
            for (l = 0; l < 3; ++l) {
                g_swap_cache.shapes[d][l] = DM2_V22_SHAPE_NONE;
            }
        }
    }

    g_swap_cache.populated = 1;
    g_cells_painted_indoor = 0;
    g_cells_painted_outdoor = 0;

    /* Forward the indoor cells to the sibling shape cache so any
     * other V22 consumer (overlay pass, debug inspector) sees the
     * same raw_cell_type + direction that the swap uses. Outdoor
     * cells are not forwarded because the outdoor path does not
     * use the sibling shape cache. */
    if (!g_swap_cache.is_outdoor && raw_cells) {
        dm2_v22_shape_cache_update(direction, raw_cells);
    }
}

int dm2_v22_viewport_swap_populated(void) {
    return g_swap_cache.populated;
}

int dm2_v22_viewport_swap_active(void) {
    if (!g_swap_cache.populated) return 0;
    if (!dm2_v22_inplace_draw_active()) return 0;
    if (!dm2_v22_get_installed()) return 0;
    if (dm2_v22_best_available_shape_source(3) != DM2_V22_SHAPE_SOURCE_V2_MODERN) {
        return 0;
    }
    return 1;
}

int dm2_v22_viewport_swap_cells_painted_indoor(void) {
    return g_cells_painted_indoor;
}

int dm2_v22_viewport_swap_cells_painted_outdoor(void) {
    return g_cells_painted_outdoor;
}

/* ── Render ──────────────────────────────────────────────────────── */

/* Clamp helper. */
static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Paint a single RGBA bitmap into the framebuffer using nearest-
 * neighbor scaling. The bitmap is keyed by (category, asset_id)
 * via dm2_v22_inplace_get_bitmap_by_id, which gives the per-cell
 * swap a clean direct lookup independent of the sibling shape
 * cache. Returns 1 on success, 0 on missing bitmap. */
static int paint_cell_for_shape(unsigned char* framebuffer, int fbW, int fbH,
                                 int dst_x, int dst_y, int dst_w, int dst_h,
                                 Dm2_V22_ShapeType shape) {
    const char* category = NULL;
    const char* asset_id = NULL;
    int w = 0, h = 0;
    const uint32_t* rgba;
    int x, y;

    if (!resolve_shape_mapping(shape, &category, &asset_id)) return 0;
    if (!category || !asset_id) return 0;

    rgba = dm2_v22_inplace_get_bitmap_by_id(category, asset_id, &w, &h);
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
             * dm2_v22_inplace_render_pass (the existing V22 pass). */
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

int dm2_v22_viewport_swap_render(unsigned char* framebuffer,
                                   int fbW, int fbH,
                                   int is_outdoor) {
    int cells_painted = 0;
    if (!framebuffer || fbW <= 0 || fbH <= 0) return 0;
    if (!dm2_v22_viewport_swap_active()) return 0;
    if (!dm2_v22_viewport_swap_populated()) return 0;

    if (is_outdoor) {
        /* Outdoor T600: paint the 3 outdoor rects (sky, horizon, ground). */
        int i;
        static const Dm2_V22_ShapeType kOutdoorShapes[3] = {
            DM2_V22_SHAPE_OUTDOOR_SKY,
            DM2_V22_SHAPE_OUTDOOR_HORIZON,
            DM2_V22_SHAPE_OUTDOOR_GROUND
        };
        for (i = 0; i < 3; ++i) {
            const DM2_V22_OutdoorCellRect* r = &dm2_v22_kOutdoorCellRects[i];
            int dx = clampi(r->x, 0, fbW);
            int dy = clampi(r->y, 0, fbH);
            int dw = clampi(r->x + r->w, 0, fbW) - dx;
            int dh = clampi(r->y + r->h, 0, fbH) - dy;
            if (dw <= 0 || dh <= 0) continue;
            if (paint_cell_for_shape(framebuffer, fbW, fbH,
                                      dx, dy, dw, dh,
                                      kOutdoorShapes[i])) {
                cells_painted++;
            }
        }
        g_cells_painted_outdoor += cells_painted;
        return cells_painted;
    }

    /* Indoor T560: paint the 9 cells (D0..D2 x L/C/R). */
    {
        int depth, lateral;
        for (depth = 0; depth < 3; ++depth) {
            for (lateral = -1; lateral <= 1; ++lateral) {
                const DM2_V22_CellRect* rect =
                    &dm2_v22_kCellRects[depth][lateral + 1];
                Dm2_V22_ShapeType shape = g_swap_cache.shapes[depth][lateral + 1];
                if (shape == DM2_V22_SHAPE_NONE) continue;
                {
                    int dx = clampi(rect->x, 0, fbW);
                    int dy = clampi(rect->y, 0, fbH);
                    int dw = clampi(rect->x + rect->w, 0, fbW) - dx;
                    int dh = clampi(rect->y + rect->h, 0, fbH) - dy;
                    if (dw <= 0 || dh <= 0) continue;
                    if (paint_cell_for_shape(framebuffer, fbW, fbH,
                                              dx, dy, dw, dh,
                                              shape)) {
                        cells_painted++;
                    }
                }
            }
        }
    }
    g_cells_painted_indoor += cells_painted;
    return cells_painted;
}

/* ── Source evidence ─────────────────────────────────────────────── */

const char* dm2_v22_viewport_swap_source_evidence(void) {
    return "SKULL.ASM T520/T560/T600 (DM2 viewport ticks: indoor T560 + outdoor T600); "
           "ReDMCSB DUNVIEW.C:2962-3070 (outdoor sky/ground composition order); "
           "ReDMCSB DUNVIEW.C:4351-4382 F0112 (ceiling pit — outdoor has no ceiling); "
           "ReDMCSB DUNVIEW.C:6239-6675 (per-cell D3L2/D3C/D3R2/D2L/D2R/D1L2/"
                "D1C/D1R2/D0L/D0R wall/floor zone tables — owns the position-"
                "aware indoor T560 per-cell discriminator); "
           "include/dm2_v22_modern_assets_pc34.h (asset pack paths + flags); "
           "include/dm2_v22_inplace_draw_pc34.h (cache bitmap lookup); "
           "include/dm2_v22_shape_cache_pc34.h (raw cell type store); "
           "dm2_v22_inplace_draw_pc34.c (cache file format + bitmap blit); "
           "dm2_v22_modern_assets_pc34.c (manifest path resolution); "
           "include/csb_v22_shapes.h (parallel CSB_V22_ShapeType pattern); "
           "v22_inplace_cache.bin (build-time RGBA pack from PNG via PIL).";
}
