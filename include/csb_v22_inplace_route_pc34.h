/*
 * csb_v22_inplace_route_pc34.h
 *
 * CSB V2.2 GPU render path: per-cell modern-art swap / material
 * routing gate.
 *
 * Why this module exists
 * ----------------------
 * The first cut of csb_v22_inplace_draw_pc34.c maps every CSB
 * V2.2 cell to a single asset_id by shape *type* alone (all
 * walls -> wall_dungeon_01, all floors -> floor_plain_01, ...).
 * That works for the smoke test but is too coarse for the
 * 9-square CSB viewport:
 *
 *   - PIT cells (CSB_V22_SHAPE_FLOOR_PIT) currently fall through
 *     the cracked-floor bucket and end up with floor_cracked_01.
 *   - STAIRS_UP / STAIRS_DOWN cells (CSB_V22_SHAPE_FLOOR_STAIRS_*)
 *     fall through the same cracked bucket and never get a
 *     distinct stairs art.
 *   - Field cells (CSB_V22_SHAPE_FIELD_TELEPORTER /
 *     FIELD_FLUXCAGE / FIELD_EXPLOSION / FIELD_CHAOS_RIFT) hit
 *     the `default` arm and get the wall_dungeon_01 asset,
 *     which is the wrong art (an in-place paint of a wall on
 *     top of a teleporter).
 *   - CSB-only narrative shapes (CSB_V22_SHAPE_PRISON_DOOR,
 *     CSB_V22_SHAPE_CHAOS_RUNE, CSB_V22_SHAPE_DSA_SCROLL,
 *     CSB_V22_SHAPE_LORD_ORDER) also fall through to the wall
 *     bucket. The CSB asset pack has dedicated prison / chaos
 *     rune / dsa / lord_order art entries that should be used
 *     instead of wall_dungeon_01.
 *
 * This module adds a per-cell material-routing gate that takes
 * (depth, lateral, cell_type) and returns a structured
 * CSB_V22_AssetRouteDecision. The decision is consumed by
 * csb_v22_inplace_draw_pc34.c (its get_cell_asset_id /
 * get_cell_bitmap paths) so the in-place draw pass routes
 * per-cell instead of by type alone.
 *
 * Routing contract
 * ----------------
 *   use_v22 == 0  -> caller MUST render V1. The V22 active bit
 *                    is off for this cell, or the gate refused
 *                    to assign an asset.
 *   use_v22 == 1  -> caller SHOULD render V22 via the asset_id
 *                    and category returned. If the bitmap cache
 *                    does not contain (category, asset_id), the
 *                    in-place draw pass naturally falls back to
 *                    V1 (a NULL bitmap short-circuits the blit).
 *   fallback_reason is a static short string that names the
 *                    contract that was applied (e.g.
 *                    "wall_dungeon_by_depth",
 *                    "field_teleporter_no_asset",
 *                    "csb_prison_door_iron",
 *                    "v22_inactive"). This is the seam a future
 *                    pixel/material verification gate can read
 *                    to assert routing without depending on
 *                    actual PNG hashes.
 *
 * Source-lock anchors
 *   ReDMCSB DUNVIEW.C:6697-6816  composition order (CSB 9-square)
 *   CSBWin/Viewport.cpp:7290     CSB 9-square viewport layout
 *   CSBWin/Chaos.cpp:60-69       chaos rune / DSA dispatch
 *   ReDMCSB ENTRANCE.C           CSB prison door intro
 *   ReDMCSB DEFS.H:922           M034_SQUARE_TYPE cell-type decode
 *   csb_v22_shape_cache_pc34.h   per-cell V22 shape cache
 *   csb_v22_inplace_draw_pc34.h  in-place draw consumer
 *
 * Module: src/csb/csb_v22_inplace_route_pc34.c
 * Test:   tests/test_csb_v22_inplace_route_pc34.c
 * Probe:  probes/firestaff_csb_v22_per_cell_route_probe.c
 */

#ifndef FIRESTAFF_CSB_V22_INPLACE_ROUTE_PC34_H
#define FIRESTAFF_CSB_V22_INPLACE_ROUTE_PC34_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CSB 9-square: depth 0..2, lateral -1..+1. */
#define CSB_V22_DEPTH_COUNT 3
#define CSB_V22_LATERAL_RANGE 1   /* |lateral| <= 1 */

/* Asset-id + category length caps. Categories are short tags
 * ("wall_shapes", "floor_shapes", ...); asset_ids are short
 * identifiers from the modern_asset_manifest.json entries. */
#define CSB_V22_ASSET_ID_MAX  48
#define CSB_V22_CATEGORY_MAX  24
#define CSB_V22_REASON_MAX    56

typedef struct {
    int   use_v22;                                 /* 0/1 */
    char  asset_id[CSB_V22_ASSET_ID_MAX];          /* "" if no asset */
    char  category[CSB_V22_CATEGORY_MAX];          /* "" if no asset */
    char  fallback_reason[CSB_V22_REASON_MAX];     /* routing diagnostic */
    int   shape_type;                              /* CSB_V22_ShapeType */
} CSB_V22_AssetRouteDecision;

/* Per-cell material-routing gate.
 *
 * depth  in {0, 1, 2}        — D0 closest, D2 farthest.
 * lateral in {-1, 0, +1}     — left, center, right.
 * cell_type is the raw M034_SQUARE_TYPE value (csb_v22_shape_for_cell
 *   decodes this into CSB_V22_ShapeType; this gate re-uses the same
 *   decode so callers can pass the raw cell type without first calling
 *   the shape book).
 * v22_active is 1 when CSB V2.2 is the active presentation mode and
 *   the V22 in-place cache is loaded; the gate respects it and
 *   short-circuits to use_v22=0 outside V22.
 *
 * Out-params: `out` is always set; on success use_v22=1 and
 * asset_id/category hold a non-empty string. On V22-inactive or
 * no-asset paths use_v22=0 and the strings are empty, but
 * fallback_reason is always populated. */
void csb_v22_inplace_route_cell(int depth,
                                 int lateral,
                                 int cell_type,
                                 int v22_active,
                                 CSB_V22_AssetRouteDecision* out);

/* Variant helpers — return the static asset_id / category / reason
 * a routed cell WOULD pick for the given shape type. These are the
 * single source of truth that csb_v22_inplace_draw_pc34.c and the
 * probes/tests should consult when checking routing invariants.
 *
 * The shape type is the CSB_V22_ShapeType (decoded by the shape
 * book). The reason string is bounded by CSB_V22_REASON_MAX; the
 * asset_id by CSB_V22_ASSET_ID_MAX; the category by CSB_V22_CATEGORY_MAX.
 *
 * The output parameters are always written; if a shape type has no
 * assigned asset the asset_id / category are written as empty
 * strings and the function returns 0. Otherwise it returns 1.
 *
 * use_v22_local==0 forces the V22-inactive short-circuit (returns 0
 * with fallback_reason="v22_inactive"). */
int csb_v22_inplace_route_for_shape(int shape_type,
                                      int v22_local,
                                      char* out_asset_id,
                                      size_t out_asset_id_size,
                                      char* out_category,
                                      size_t out_category_size,
                                      char* out_reason,
                                      size_t out_reason_size);

/* Reset the per-cell material-routing gate's internal state. The
 * gate is otherwise stateless (its decision is a pure function of
 * inputs), but this hook lets a future readiness probe assert
 * "no stale routing across reload". */
void csb_v22_inplace_route_reset(void);

/* Return 1 if the (category, asset_id) pair is one the route gate
 * is willing to return. Used by the in-place draw cache to detect
 * manifest gaps (e.g. a wall_d2_door_01 referenced by the gate but
 * not present in the modern_asset_manifest.json). */
int csb_v22_inplace_route_pair_recognized(const char* category,
                                            const char* asset_id);

/* Number of unique (category, asset_id) pairs the route gate can
 * emit. Useful for tests/probes that want to bound their
 * expectations without enumerating the full table. */
int csb_v22_inplace_route_pair_count(void);

/* Source evidence citation. */
const char* csb_v22_inplace_route_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V22_INPLACE_ROUTE_PC34_H */
