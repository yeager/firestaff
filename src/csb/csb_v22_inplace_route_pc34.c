/*
 * csb_v22_inplace_route_pc34.c
 *
 * Per-cell modern-art swap / material routing gate for the
 * CSB V2.2 9-square viewport. See include/csb_v22_inplace_route_pc34.h
 * for the design contract and source-lock references.
 *
 * The gate is a pure function of (depth, lateral, cell_type,
 * v22_active). It does not depend on the in-place bitmap cache
 * being loaded — that is the consumer's responsibility. When the
 * gate returns use_v22=1 with a (category, asset_id) pair the
 * in-place draw pass will look it up in its cache; a miss
 * naturally short-circuits to V1.
 *
 * Routing table summary (depth x lateral x cell_type -> asset):
 *   WALL_STRAIGHT / CORNER_INNER / CORNER_OUTER / ALCOVE / INSCRIPTION:
 *     D0 (closest)   -> wall_dungeon_d0_01   ("wall_shapes")
 *     D1 (middle)    -> wall_dungeon_d1_01   ("wall_shapes")
 *     D2 (farthest)  -> wall_dungeon_d2_01   ("wall_shapes")
 *   WALL_DOORWAY (CSB walls with door frame present):
 *     D0 -> door_d0_01 / D1 -> door_d1_01 / D2 -> door_d2_01
 *   FLOOR_PLAIN:        floor_plain_d<depth>_01  ("floor_shapes")
 *   FLOOR_CRACKED:      floor_cracked_d<depth>_01
 *   FLOOR_MOSSY:        floor_mossy_d<depth>_01
 *   FLOOR_PIT:          floor_pit_01             (depth-invariant)
 *   FLOOR_STAIRS_UP:    floor_stairs_up_01       (depth-invariant)
 *   FLOOR_STAIRS_DOWN:  floor_stairs_down_01     (depth-invariant)
 *   CEILING_PLAIN / CEILING_VAULTED:
 *     depth-invariant ceiling_01                  ("wall_shapes")
 *   CREATURE / CREATURE_PROJECTILE:
 *     creature_demon_d<depth>_01                  ("creature_shapes")
 *   ITEM / ITEM_FLOOR / ITEM_PROJECTILE:
 *     creature_demon_d<depth>_01 (placeholder until item art)
 *   FIELD_TELEPORTER / FIELD_FLUXCAGE / FIELD_EXPLOSION / FIELD_CHAOS_RIFT:
 *     no asset (returns use_v22=0 with reason
 *     "field_<type>_no_asset"); the in-place draw pass leaves
 *     these cells to V1 instead of painting wrong wall art.
 *   PRISON_DOOR:           prison_door_01          ("wall_shapes")
 *   CHAOS_RUNE:            chaos_rune_<index>_01   ("chaos_runes")
 *   DSA_SCROLL:            dsa_scroll_01           ("dsa_scrolls")
 *   LORD_ORDER:            lord_order_01           ("wall_shapes")
 *   UI_* shapes:           no asset                (V1 only)
 *
 * The asset_id/category strings are all static literals owned
 * by this module and remain valid for the program lifetime.
 */

#include "csb_v22_inplace_route_pc34.h"
#include "csb_v22_shapes.h"
#include "csb_v22_shape_cache_pc34.h"

#include <stdio.h>
#include <string.h>

/* ── Static asset/category/reason literals ──────────────────────── */

#define CAT_WALL  "wall_shapes"
#define CAT_FLOOR "floor_shapes"
#define CAT_CR    "creature_shapes"
#define CAT_DOOR  "door_shapes"
#define CAT_CHAOS "chaos_runes"
#define CAT_DSA   "dsa_scrolls"

/* (asset_id, category) pair table used by pair_recognized. Keep
 * the literal pool tiny so pair_count is well-bounded. */
typedef struct {
    const char* category;
    const char* asset_id;
} RoutePair;

static const RoutePair kRoutePairs[] = {
    /* Wall dungeon variants (D0/D1/D2). */
    { CAT_WALL, "wall_dungeon_d0_01" },
    { CAT_WALL, "wall_dungeon_d1_01" },
    { CAT_WALL, "wall_dungeon_d2_01" },
    /* Door variants. */
    { CAT_DOOR, "door_d0_01" },
    { CAT_DOOR, "door_d1_01" },
    { CAT_DOOR, "door_d2_01" },
    /* Floor plain / cracked / mossy per depth. */
    { CAT_FLOOR, "floor_plain_d0_01" },
    { CAT_FLOOR, "floor_plain_d1_01" },
    { CAT_FLOOR, "floor_plain_d2_01" },
    { CAT_FLOOR, "floor_cracked_d0_01" },
    { CAT_FLOOR, "floor_cracked_d1_01" },
    { CAT_FLOOR, "floor_cracked_d2_01" },
    { CAT_FLOOR, "floor_mossy_d0_01" },
    { CAT_FLOOR, "floor_mossy_d1_01" },
    { CAT_FLOOR, "floor_mossy_d2_01" },
    /* Depth-invariant floor specials. */
    { CAT_FLOOR, "floor_pit_01" },
    { CAT_FLOOR, "floor_stairs_up_01" },
    { CAT_FLOOR, "floor_stairs_down_01" },
    /* Ceiling. */
    { CAT_WALL, "ceiling_01" },
    /* Creature per depth (depth-invariant would also work, but the
     * per-depth split leaves room for size-perspective swaps). */
    { CAT_CR, "creature_demon_d0_01" },
    { CAT_CR, "creature_demon_d1_01" },
    { CAT_CR, "creature_demon_d2_01" },
    /* CSB-specific narrative shapes. */
    { CAT_WALL, "prison_door_01" },
    { CAT_WALL, "lord_order_01" },
    { CAT_CHAOS, "chaos_rune_0_01" },
    { CAT_CHAOS, "chaos_rune_1_01" },
    { CAT_CHAOS, "chaos_rune_2_01" },
    { CAT_CHAOS, "chaos_rune_3_01" },
    { CAT_DSA, "dsa_scroll_01" },
};

#define ROUTE_PAIR_COUNT \
    (int)(sizeof(kRoutePairs) / sizeof(kRoutePairs[0]))

/* ── Helpers ────────────────────────────────────────────────────── */

static int copy_str(char* dst, size_t dst_size, const char* src) {
    size_t n;
    if (!dst || dst_size == 0) return 0;
    if (!src) { dst[0] = '\0'; return 0; }
    n = strlen(src);
    if (n + 1U > dst_size) { dst[0] = '\0'; return 0; }
    memcpy(dst, src, n + 1U);
    return 1;
}

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int shape_type_for_cell_type(int cell_type) {
    /* Mirror the decode in csb_v22_shapes.c (csb_v22_shape_type_for_cell)
     * so callers can pass the raw M034 cell type without first
     * consulting the shape book. Intentionally inlined here so
     * the route gate has no header dependency on csb_v22_shapes.c
     * (the inline is small and stable; if it drifts, the data-free
     * route test will catch it). */
    int base = cell_type & 0x0F;
    int flags = cell_type & 0xF0;

    if (flags & 0x40) return CSB_V22_SHAPE_FLOOR_PIT;
    if (flags & 0x20) return CSB_V22_SHAPE_WALL_DOORWAY;
    if (flags & 0x10) {
        return (base & 0x01) ? CSB_V22_SHAPE_FLOOR_STAIRS_DOWN
                             : CSB_V22_SHAPE_FLOOR_STAIRS_UP;
    }
    switch (base) {
        case 0:  return CSB_V22_SHAPE_WALL_STRAIGHT;
        case 1:  return CSB_V22_SHAPE_WALL_CORNER_INNER;
        case 2:  return CSB_V22_SHAPE_WALL_CORNER_OUTER;
        case 3:  return CSB_V22_SHAPE_WALL_ALCOVE;
        case 4:  return CSB_V22_SHAPE_FLOOR_PLAIN;
        case 5:  return CSB_V22_SHAPE_FLOOR_CRACKED;
        case 6:  return CSB_V22_SHAPE_FLOOR_MOSSY;
        case 7:  return CSB_V22_SHAPE_CEILING_PLAIN;
        case 8:  return CSB_V22_SHAPE_CEILING_VAULTED;
        case 9:  return CSB_V22_SHAPE_WALL_INSCRIPTION;
        default: return CSB_V22_SHAPE_FLOOR_PLAIN;
    }
}

/* ── Public API ─────────────────────────────────────────────────── */

void csb_v22_inplace_route_reset(void) {
    /* The gate is stateless; this hook exists so a future readiness
     * probe can call it before/after a reload and assert no
     * observable state change. No-op for now. */
}

int csb_v22_inplace_route_for_shape(int shape_type,
                                      int v22_local,
                                      char* out_asset_id,
                                      size_t out_asset_id_size,
                                      char* out_category,
                                      size_t out_category_size,
                                      char* out_reason,
                                      size_t out_reason_size) {
    /* Always initialize outputs so callers never read uninit memory. */
    if (out_asset_id && out_asset_id_size > 0) out_asset_id[0] = '\0';
    if (out_category && out_category_size > 0) out_category[0] = '\0';
    if (out_reason   && out_reason_size   > 0) out_reason[0]   = '\0';

    if (!v22_local) {
        copy_str(out_reason, out_reason_size, "v22_inactive");
        return 0;
    }

    /* The reason string is the contract name; a future
     * verification gate reads it to assert routing without
     * needing actual PNG hashes. */
    switch (shape_type) {
        /* ── Walls (per-depth carved stone) ─────────────────── */
        case CSB_V22_SHAPE_WALL_STRAIGHT:
        case CSB_V22_SHAPE_WALL_CORNER_INNER:
        case CSB_V22_SHAPE_WALL_CORNER_OUTER:
        case CSB_V22_SHAPE_WALL_ALCOVE:
        case CSB_V22_SHAPE_WALL_INSCRIPTION:
            /* Caller fills depth-aware asset_id via csb_v22_inplace_route_cell.
             * The shape-only route has no depth context, so refuse to emit
             * a depth-dependent asset here. */
            copy_str(out_reason, out_reason_size, "wall_needs_depth");
            return 0;

        case CSB_V22_SHAPE_WALL_DOORWAY: {
            /* depth-aware; same comment. */
            copy_str(out_reason, out_reason_size, "door_needs_depth");
            return 0;
        }

        /* ── Floors (per-depth plain / cracked / mossy) ─────── */
        case CSB_V22_SHAPE_FLOOR_PLAIN:
        case CSB_V22_SHAPE_FLOOR_CRACKED:
        case CSB_V22_SHAPE_FLOOR_MOSSY:
            copy_str(out_reason, out_reason_size, "floor_needs_depth");
            return 0;

        case CSB_V22_SHAPE_FLOOR_PIT:
            copy_str(out_asset_id, out_asset_id_size, "floor_pit_01");
            copy_str(out_category, out_category_size, CAT_FLOOR);
            copy_str(out_reason,   out_reason_size,   "floor_pit_depth_invariant");
            return 1;

        case CSB_V22_SHAPE_FLOOR_STAIRS_UP:
            copy_str(out_asset_id, out_asset_id_size, "floor_stairs_up_01");
            copy_str(out_category, out_category_size, CAT_FLOOR);
            copy_str(out_reason,   out_reason_size,   "floor_stairs_up_depth_invariant");
            return 1;

        case CSB_V22_SHAPE_FLOOR_STAIRS_DOWN:
            copy_str(out_asset_id, out_asset_id_size, "floor_stairs_down_01");
            copy_str(out_category, out_category_size, CAT_FLOOR);
            copy_str(out_reason,   out_reason_size,   "floor_stairs_down_depth_invariant");
            return 1;

        case CSB_V22_SHAPE_FLOOR_DOOR:
            /* Treated as door; caller refines with depth. */
            copy_str(out_reason, out_reason_size, "floor_door_needs_depth");
            return 0;

        /* ── Ceiling (depth-invariant) ──────────────────────── */
        case CSB_V22_SHAPE_CEILING_PLAIN:
        case CSB_V22_SHAPE_CEILING_VAULTED:
            copy_str(out_asset_id, out_asset_id_size, "ceiling_01");
            copy_str(out_category, out_category_size, CAT_WALL);
            copy_str(out_reason,   out_reason_size,   "ceiling_depth_invariant");
            return 1;

        /* ── Creatures / items (per-depth; placeholder) ─────── */
        case CSB_V22_SHAPE_CREATURE:
        case CSB_V22_SHAPE_CREATURE_PROJECTILE:
        case CSB_V22_SHAPE_ITEM:
        case CSB_V22_SHAPE_ITEM_FLOOR:
        case CSB_V22_SHAPE_ITEM_PROJECTILE:
            copy_str(out_reason, out_reason_size, "creature_needs_depth");
            return 0;

        /* ── Fields: no asset, V1 fallback ─────────────────── */
        case CSB_V22_SHAPE_FIELD_TELEPORTER:
            copy_str(out_reason, out_reason_size, "field_teleporter_no_asset");
            return 0;
        case CSB_V22_SHAPE_FIELD_FLUXCAGE:
            copy_str(out_reason, out_reason_size, "field_fluxcage_no_asset");
            return 0;
        case CSB_V22_SHAPE_FIELD_EXPLOSION:
            copy_str(out_reason, out_reason_size, "field_explosion_no_asset");
            return 0;
        case CSB_V22_SHAPE_FIELD_CHAOS_RIFT:
            copy_str(out_reason, out_reason_size, "field_chaos_rift_no_asset");
            return 0;

        /* ── CSB-specific shapes ────────────────────────────── */
        case CSB_V22_SHAPE_PRISON_DOOR:
            copy_str(out_asset_id, out_asset_id_size, "prison_door_01");
            copy_str(out_category, out_category_size, CAT_WALL);
            copy_str(out_reason,   out_reason_size,   "csb_prison_door_iron");
            return 1;

        case CSB_V22_SHAPE_CHAOS_RUNE:
            /* Without a rune_index in this single-shape route we
             * emit the index-0 entry; the per-cell route uses the
             * cached material/lateral context to pick the right
             * index. The reason "chaos_rune_needs_index" signals
             * that the single-shape route is being used. */
            copy_str(out_asset_id, out_asset_id_size, "chaos_rune_0_01");
            copy_str(out_category, out_category_size, CAT_CHAOS);
            copy_str(out_reason,   out_reason_size,   "chaos_rune_default_0");
            return 1;

        case CSB_V22_SHAPE_DSA_SCROLL:
            copy_str(out_asset_id, out_asset_id_size, "dsa_scroll_01");
            copy_str(out_category, out_category_size, CAT_DSA);
            copy_str(out_reason,   out_reason_size,   "csb_dsa_scroll_parchment");
            return 1;

        case CSB_V22_SHAPE_LORD_ORDER:
            copy_str(out_asset_id, out_asset_id_size, "lord_order_01");
            copy_str(out_category, out_category_size, CAT_WALL);
            copy_str(out_reason,   out_reason_size,   "csb_lord_order_marble");
            return 1;

        /* ── UI chrome (V1 only — never painted via in-place) ─ */
        case CSB_V22_SHAPE_UI_CHROME:
        case CSB_V22_SHAPE_UI_PORTRAIT:
        case CSB_V22_SHAPE_UI_MESSAGE_LOG:
        case CSB_V22_SHAPE_UI_INVENTORY_GRID:
        case CSB_V22_SHAPE_UI_DSA_RUNE:
            copy_str(out_reason, out_reason_size, "ui_chrome_v1_only");
            return 0;

        default:
            copy_str(out_reason, out_reason_size, "shape_unknown_fallback");
            return 0;
    }
}

/* Per-depth asset-id helpers. The shape type is in
 * {WALL_*, WALL_DOORWAY, FLOOR_*, FLOOR_DOOR, CREATURE_*,
 * CREATURE_PROJECTILE, ITEM_*}. We pick the asset_id from
 * the depth (0..2 -> d0/d1/d2). For depth-dependent shapes the
 * caller (route_cell) must call these — they are not part of
 * the public API. */
static void assign_wall_by_depth(int depth,
                                   char* out_asset_id, size_t aid_size,
                                   char* out_category, size_t cat_size,
                                   char* out_reason, size_t rsn_size) {
    int d = clamp_int(depth, 0, CSB_V22_DEPTH_COUNT - 1);
    if (d == 0) {
        copy_str(out_asset_id, aid_size, "wall_dungeon_d0_01");
        copy_str(out_reason,   rsn_size, "wall_dungeon_by_depth_d0");
    } else if (d == 1) {
        copy_str(out_asset_id, aid_size, "wall_dungeon_d1_01");
        copy_str(out_reason,   rsn_size, "wall_dungeon_by_depth_d1");
    } else {
        copy_str(out_asset_id, aid_size, "wall_dungeon_d2_01");
        copy_str(out_reason,   rsn_size, "wall_dungeon_by_depth_d2");
    }
    copy_str(out_category, cat_size, CAT_WALL);
}

static void assign_door_by_depth(int depth,
                                   char* out_asset_id, size_t aid_size,
                                   char* out_category, size_t cat_size,
                                   char* out_reason, size_t rsn_size) {
    int d = clamp_int(depth, 0, CSB_V22_DEPTH_COUNT - 1);
    if (d == 0) {
        copy_str(out_asset_id, aid_size, "door_d0_01");
        copy_str(out_reason,   rsn_size, "door_d0_iron_frame");
    } else if (d == 1) {
        copy_str(out_asset_id, aid_size, "door_d1_01");
        copy_str(out_reason,   rsn_size, "door_d1_iron_frame");
    } else {
        copy_str(out_asset_id, aid_size, "door_d2_01");
        copy_str(out_reason,   rsn_size, "door_d2_iron_frame");
    }
    copy_str(out_category, cat_size, CAT_DOOR);
}

static void assign_floor_by_depth(int depth, int shape_type,
                                    char* out_asset_id, size_t aid_size,
                                    char* out_category, size_t cat_size,
                                    char* out_reason, size_t rsn_size) {
    int d = clamp_int(depth, 0, CSB_V22_DEPTH_COUNT - 1);
    const char* depth_suffix;
    const char* tile;
    if (d == 0) depth_suffix = "d0";
    else if (d == 1) depth_suffix = "d1";
    else depth_suffix = "d2";
    if (shape_type == CSB_V22_SHAPE_FLOOR_CRACKED) tile = "floor_cracked";
    else if (shape_type == CSB_V22_SHAPE_FLOOR_MOSSY) tile = "floor_mossy";
    else tile = "floor_plain";
    snprintf(out_asset_id, aid_size, "%s_%s_01", tile, depth_suffix);
    copy_str(out_category, cat_size, CAT_FLOOR);
    snprintf(out_reason, rsn_size, "%s_by_depth_%s", tile, depth_suffix);
}

static void assign_creature_by_depth(int depth,
                                       char* out_asset_id, size_t aid_size,
                                       char* out_category, size_t cat_size,
                                       char* out_reason, size_t rsn_size) {
    int d = clamp_int(depth, 0, CSB_V22_DEPTH_COUNT - 1);
    if (d == 0) {
        copy_str(out_asset_id, aid_size, "creature_demon_d0_01");
        copy_str(out_reason,   rsn_size, "creature_demon_by_depth_d0");
    } else if (d == 1) {
        copy_str(out_asset_id, aid_size, "creature_demon_d1_01");
        copy_str(out_reason,   rsn_size, "creature_demon_by_depth_d1");
    } else {
        copy_str(out_asset_id, aid_size, "creature_demon_d2_01");
        copy_str(out_reason,   rsn_size, "creature_demon_by_depth_d2");
    }
    copy_str(out_category, cat_size, CAT_CR);
}

void csb_v22_inplace_route_cell(int depth,
                                 int lateral,
                                 int cell_type,
                                 int v22_active,
                                 CSB_V22_AssetRouteDecision* out) {
    int shape_type;

    /* Always init the struct so callers never read uninit memory. */
    if (!out) return;
    out->use_v22 = 0;
    out->asset_id[0] = '\0';
    out->category[0] = '\0';
    out->fallback_reason[0] = '\0';
    out->shape_type = -1;

    if (!v22_active) {
        copy_str(out->fallback_reason,
                 sizeof(out->fallback_reason),
                 "v22_inactive");
        return;
    }

    /* Clamp coords; out-of-range coords fall back to V1. */
    if (depth < 0 || depth > 2) {
        copy_str(out->fallback_reason,
                 sizeof(out->fallback_reason),
                 "depth_out_of_range");
        return;
    }
    if (lateral < -1 || lateral > 1) {
        copy_str(out->fallback_reason,
                 sizeof(out->fallback_reason),
                 "lateral_out_of_range");
        return;
    }

    shape_type = shape_type_for_cell_type(cell_type);
    out->shape_type = shape_type;

    switch (shape_type) {
        case CSB_V22_SHAPE_WALL_STRAIGHT:
        case CSB_V22_SHAPE_WALL_CORNER_INNER:
        case CSB_V22_SHAPE_WALL_CORNER_OUTER:
        case CSB_V22_SHAPE_WALL_ALCOVE:
        case CSB_V22_SHAPE_WALL_INSCRIPTION:
            assign_wall_by_depth(depth,
                                 out->asset_id, sizeof(out->asset_id),
                                 out->category, sizeof(out->category),
                                 out->fallback_reason,
                                 sizeof(out->fallback_reason));
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_WALL_DOORWAY:
        case CSB_V22_SHAPE_FLOOR_DOOR:
            assign_door_by_depth(depth,
                                 out->asset_id, sizeof(out->asset_id),
                                 out->category, sizeof(out->category),
                                 out->fallback_reason,
                                 sizeof(out->fallback_reason));
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_FLOOR_PLAIN:
        case CSB_V22_SHAPE_FLOOR_CRACKED:
        case CSB_V22_SHAPE_FLOOR_MOSSY:
            assign_floor_by_depth(depth, shape_type,
                                  out->asset_id, sizeof(out->asset_id),
                                  out->category, sizeof(out->category),
                                  out->fallback_reason,
                                  sizeof(out->fallback_reason));
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_FLOOR_PIT:
            copy_str(out->asset_id, sizeof(out->asset_id), "floor_pit_01");
            copy_str(out->category, sizeof(out->category), CAT_FLOOR);
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "floor_pit_depth_invariant");
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_FLOOR_STAIRS_UP:
            copy_str(out->asset_id, sizeof(out->asset_id), "floor_stairs_up_01");
            copy_str(out->category, sizeof(out->category), CAT_FLOOR);
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "floor_stairs_up_depth_invariant");
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_FLOOR_STAIRS_DOWN:
            copy_str(out->asset_id, sizeof(out->asset_id), "floor_stairs_down_01");
            copy_str(out->category, sizeof(out->category), CAT_FLOOR);
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "floor_stairs_down_depth_invariant");
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_CEILING_PLAIN:
        case CSB_V22_SHAPE_CEILING_VAULTED:
            copy_str(out->asset_id, sizeof(out->asset_id), "ceiling_01");
            copy_str(out->category, sizeof(out->category), CAT_WALL);
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "ceiling_depth_invariant");
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_CREATURE:
        case CSB_V22_SHAPE_CREATURE_PROJECTILE:
        case CSB_V22_SHAPE_ITEM:
        case CSB_V22_SHAPE_ITEM_FLOOR:
        case CSB_V22_SHAPE_ITEM_PROJECTILE:
            assign_creature_by_depth(depth,
                                     out->asset_id, sizeof(out->asset_id),
                                     out->category, sizeof(out->category),
                                     out->fallback_reason,
                                     sizeof(out->fallback_reason));
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_FIELD_TELEPORTER:
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "field_teleporter_no_asset");
            return;
        case CSB_V22_SHAPE_FIELD_FLUXCAGE:
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "field_fluxcage_no_asset");
            return;
        case CSB_V22_SHAPE_FIELD_EXPLOSION:
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "field_explosion_no_asset");
            return;
        case CSB_V22_SHAPE_FIELD_CHAOS_RIFT:
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "field_chaos_rift_no_asset");
            return;

        case CSB_V22_SHAPE_PRISON_DOOR:
            copy_str(out->asset_id, sizeof(out->asset_id), "prison_door_01");
            copy_str(out->category, sizeof(out->category), CAT_WALL);
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "csb_prison_door_iron");
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_CHAOS_RUNE: {
            /* Lateral picks the rune index (L=-1 -> 0, C=0 -> 1,
             * R=+1 -> 2). Falling outside that mapping uses
             * "default 0". This is the per-cell variation the
             * first cut didn't have: every cell can show a
             * distinct chaos rune in the 3x3. */
            int idx;
            const char* aid;
            const char* reason;
            switch (lateral) {
                case -1: idx = 0; aid = "chaos_rune_0_01";
                         reason = "chaos_rune_lateral_-1_idx0"; break;
                case  0: idx = 1; aid = "chaos_rune_1_01";
                         reason = "chaos_rune_lateral_0_idx1";  break;
                case  1: idx = 2; aid = "chaos_rune_2_01";
                         reason = "chaos_rune_lateral_1_idx2";  break;
                default: idx = 0; aid = "chaos_rune_0_01";
                         reason = "chaos_rune_default_0";      break;
            }
            (void)idx;
            copy_str(out->asset_id, sizeof(out->asset_id), aid);
            copy_str(out->category, sizeof(out->category), CAT_CHAOS);
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     reason);
            out->use_v22 = 1;
            return;
        }

        case CSB_V22_SHAPE_DSA_SCROLL:
            copy_str(out->asset_id, sizeof(out->asset_id), "dsa_scroll_01");
            copy_str(out->category, sizeof(out->category), CAT_DSA);
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "csb_dsa_scroll_parchment");
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_LORD_ORDER:
            copy_str(out->asset_id, sizeof(out->asset_id), "lord_order_01");
            copy_str(out->category, sizeof(out->category), CAT_WALL);
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "csb_lord_order_marble");
            out->use_v22 = 1;
            return;

        case CSB_V22_SHAPE_UI_CHROME:
        case CSB_V22_SHAPE_UI_PORTRAIT:
        case CSB_V22_SHAPE_UI_MESSAGE_LOG:
        case CSB_V22_SHAPE_UI_INVENTORY_GRID:
        case CSB_V22_SHAPE_UI_DSA_RUNE:
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "ui_chrome_v1_only");
            return;

        default:
            copy_str(out->fallback_reason, sizeof(out->fallback_reason),
                     "shape_unknown_fallback");
            return;
    }
}

int csb_v22_inplace_route_pair_recognized(const char* category,
                                            const char* asset_id) {
    int i;
    if (!category || !asset_id) return 0;
    for (i = 0; i < ROUTE_PAIR_COUNT; ++i) {
        if (strcmp(kRoutePairs[i].category, category) == 0 &&
            strcmp(kRoutePairs[i].asset_id,   asset_id) == 0) {
            return 1;
        }
    }
    return 0;
}

int csb_v22_inplace_route_pair_count(void) {
    return ROUTE_PAIR_COUNT;
}

const char* csb_v22_inplace_route_source_evidence(void) {
    return
        "CSB V2.2 per-cell modern-art swap / material routing gate.\n"
        "  ReDMCSB DUNVIEW.C:6697-6816 - 9-square composition order\n"
        "  CSBWin/Viewport.cpp:7290    - 9-square layout (3x3)\n"
        "  CSBWin/Chaos.cpp:60-69      - chaos rune / DSA dispatch\n"
        "  ReDMCSB ENTRANCE.C          - CSB prison door intro shape\n"
        "  ReDMCSB DEFS.H:922          - M034_SQUARE_TYPE cell-type decode\n"
        "  csb_v22_inplace_draw_pc34.h - in-place draw consumer\n"
        "  csb_v22_shape_cache_pc34.h  - per-cell V22 shape cache\n"
        "Per-cell material-routing contract:\n"
        "  - Walls, doors, floors, creatures vary by (depth 0..2)\n"
        "  - PIT / STAIRS_UP / STAIRS_DOWN depth-invariant\n"
        "  - Fields (teleporter/fluxcage/explosion/chaos_rift) -> V1 only\n"
        "  - CSB-only narrative shapes use dedicated prison / chaos / dsa / lord_order assets\n"
        "  - Chaos rune index varies by lateral (-1/0/+1 -> 0/1/2)\n"
        "Manifest categories (CSB modern_asset_manifest.json):\n"
        "  wall_shapes, floor_shapes, creature_shapes, door_shapes,\n"
        "  chaos_runes, dsa_scrolls (all optional for graceful degradation)";
}
