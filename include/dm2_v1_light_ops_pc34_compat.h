#ifndef FIRESTAFF_DM2_V1_LIGHT_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_LIGHT_OPS_PC34_COMPAT_H

/*
 * dm2_v1_light_ops_pc34_compat.h — DM2 V1 light operations from
 * skproject/SKULLWIN/c_light.cpp.
 *
 * Callback-based implementations of:
 *   DM2_RECALC_LIGHT_LEVEL               c_light.cpp:16
 *   DM2_PROCEED_LIGHT                    c_light.cpp:596
 */

#include <stdint.h>
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM2_RECALC_LIGHT_LEVEL (c_light.cpp:16) ----
 * Recompute the effective light level for the party's current position.
 * Collects light from party-held items (GDAT DBSPEC flag 0x10),
 * sorts by charge, accumulates via table1d6702, adds ambient/savegame/weather
 * light, clamps to 0-5, stores result via set_light_level callback. */
typedef struct {
    /* Map tile query */
    uint8_t (*get_map_tile_byte)(void *ctx, int16_t map_idx, int offset);

    /* Party item enumeration */
    int16_t (*get_leader_item)(void *ctx);
    int16_t (*get_heros_in_party)(void *ctx);
    int16_t (*get_hero_item)(void *ctx, int hero_idx, int hand);
    /* Returns -1 for no item, else item record index */

    /* Item property queries */
    uint16_t (*query_gdat_dbspec_word)(void *ctx, int16_t item, int key);
    int16_t (*add_item_charge)(void *ctx, int16_t item, int mode);

    /* GDAT lookup for map-specific light adjustment */
    int16_t (*query_gdat_entry_data_index)(void *ctx, int a, int b, int c, int d);

    /* Global state (read) */
    int16_t v1e0974;        /* ambient light */
    int16_t savegame_light; /* savegames1_w00 contribution */
    int16_t v1d6c02;        /* GDAT category for map adjustment */
    int16_t v1e147f;        /* weather flag */
    int16_t v1e1480;        /* weather light index */
    int16_t v1e1476;        /* weather light enabled */
    int16_t v1e0978;        /* current map has light flag */
    int16_t v1e024c;        /* party x*y or tile index */
    int16_t map_index;      /* current map index */

    /* Lookup tables */
    const int16_t *table1d6702;  /* light accumulation table */
    int table1d6702_size;
    const int16_t *table1d6712;  /* weather light table */
    int table1d6712_size;

    /* Output */
    void (*set_light_level)(void *ctx, int16_t level);
} DM2_V1_RecalcLightLevelCallbacks;

void dm2_v1_recalc_light_level_pc34(
    const DM2_V1_RecalcLightLevelCallbacks *cb, void *ctx);

/* ---- DM2_PROCEED_LIGHT (c_light.cpp:596) ----
 * Initiate a light spell effect: compute step, delay, and queue timer.
 * light_type: 6 = darkness, 0x26 = torch, 0x27 = bright.
 * intensity: current running value. */
typedef struct {
    int16_t *global_light;
    const int16_t *light_table;
    int light_table_size;
    uint32_t game_tick;
    void (*queue_light_timer)(void *ctx, int16_t value, uint32_t fire_tick);
    void (*recalc_light)(void *ctx);
} DM2_V1_ProceedLightCallbacks;

void dm2_v1_proceed_light(
    uint16_t light_type, int16_t intensity,
    const DM2_V1_ProceedLightCallbacks *cb, void *ctx);

/* ---- DM2_CHECK_RECOMPUTE_LIGHT (c_light.cpp:490) ----
 * Check dirty flag, recompute light map if needed. */
typedef struct {
    int (*is_light_dirty)(void *ctx);
    void (*recompute_light_map)(void *ctx);
    void (*clear_light_dirty)(void *ctx);
} DM2_V1_CheckRecomputeLightCallbacks;

int32_t dm2_v1_check_recompute_light(
    const DM2_V1_CheckRecomputeLightCallbacks *cb, void *ctx);

/* sklight.cpp::DM2_CHECK_RECOMPUTE_LIGHT prepares mode 7/action 0x17 only
 * when the source radius is nonzero. This is a preparation receipt, not a
 * FIND_WALK_PATH completion signal. */
typedef struct {
    uint8_t traversal_required;
    uint8_t radius;
    int16_t v1e0974;
    int16_t v1e0978;
} DM2_V1_Mode7LightPreparation;

int dm2_v1_mode7_light_prepare(uint16_t source_radius,
                              DM2_V1_Mode7LightPreparation *out);

/* SK1C9A.cpp action 0x17 samples background light only if the cached tile
 * state has bit 0x10. This decision does not complete traversal. */
int dm2_v1_mode7_action23_samples_tile(uint16_t cached_tile_state);
/* Action-23 GO_THERE edge: 1 admitted, 0 rejected, -1 unresolved. */
int dm2_v1_mode7_go_there_tile_admission(uint8_t raw_tile,
                                          int first_record_link);
int dm2_v1_mode7_go_there_empty_class0_admission(
    uint8_t raw_tile, int first_record_link, int party_square);
int dm2_v1_mode7_go_there_class1_raw30_admission(
    uint8_t raw_tile, int no_creature_proven, int party_square);
int dm2_v1_mode7_go_there_class1_no_record_admission(
    uint8_t raw_tile, int first_record_link, int party_square);
int dm2_v1_mode7_go_there_class0_record_admission(
    uint8_t raw_tile, int first_record_link,
    int no_creature_proven, int party_square);
int dm2_v1_mode7_go_there_class5_b0_admission(
    uint8_t raw_tile, int teleporter_detail_present);
int dm2_v1_mode7_go_there_class5_b8_admission(
    uint8_t raw_tile, uint16_t db1_word2);
int dm2_v1_mode8_class1_creature_admission(
    uint8_t raw_tile, const DM2_V1_FirstCreatureReceipt *creature,
    uint16_t source_mask, int party_square);
int dm2_v1_mode7_class1_creature_admission(
    uint8_t raw_tile, const DM2_V1_FirstCreatureReceipt *creature,
    uint8_t score, uint16_t ai_word10, int party_square);
int dm2_v1_mode8_class4_single_db0_admission(
    uint8_t raw_tile, const DM2_V1_FirstCreatureReceipt *creature,
    uint16_t source_mask, int party_square);

/* One admitted FIND_WALK_PATH action-0x17 node. Returns 0 for a source skip,
 * 1 after the callback succeeds, and -1 if an admitted source is unresolved.
 * The callback must implement ADD_BACKGROUND_LIGHT_FROM_TILE with flags 4. */
typedef int (*DM2_V1_Mode7AddBackgroundLight)(
    void *ctx, uint8_t radius, int16_t map, int16_t x, int16_t y,
    uint8_t source_flags);
int dm2_v1_mode7_action23_visit_tile(
    uint16_t cached_tile_state, uint8_t radius,
    int16_t map, int16_t x, int16_t y,
    DM2_V1_Mode7AddBackgroundLight add_background_light, void *ctx);

/* The terminal ADD_BACKGROUND_LIGHT_FROM_TILE update. Inputs are the
 * source-resolved RG6 (tile), RG5 (darkness), and vw_24 (weather) terms;
 * resolving those terms still requires the stone-room and GDAT branches.
 * This updates two live accumulators for one tile, not traversal status. */
int dm2_v1_mode7_light_accumulate_tile(
    uint8_t distance, int16_t tile_light, int16_t darkness,
    int16_t weather_light, int16_t *v1e0974, int16_t *v1e0978);

/* Resolve the flags-4/type-zero floor branch from an authenticated dungeon
 * receipt. Other SUMMARIZE_STONE_ROOM classes are intentionally rejected. */
int dm2_v1_mode7_flags4_floor_terms(
    const DM2_V1_CLightFlags4FloorReceipt *floor,
    uint8_t weather_index, uint8_t weather_delta,
    int16_t *out_tile_light, int16_t *out_weather_light);

/* A class-2 no-record stone-room receipt may become effective class 1 or 2.
 * With source flags 4, neither branch enters the flags-1 or flags-2 light
 * paths, so its resolved terms are zero. */
int dm2_v1_mode7_flags4_class2_terms(
    const DM2_V1_CLightStoneRoomReceipt *room,
    int16_t *out_tile_light, int16_t *out_darkness,
    int16_t *out_weather_light);
/* Source flags 4 skip the ceiling/record branch (bit 1) and creature
 * branch (bit 2) even on a class-5 DB1 teleporter tile. */
int dm2_v1_mode7_flags4_class5_terms(
    const DM2_V1_CLightStoneRoomReceipt *room,
    int16_t *out_tile_light, int16_t *out_darkness,
    int16_t *out_weather_light);

/* The v1e08ae cache owned by SK1C9A::DM2_19f0_045a. Zero-initialize once;
 * the source cache persists across FIND_WALK_PATH calls. A full traversal
 * supplies effective vb_140 flags and node order; this isolated model only
 * reproduces the cache transition and action-0x17 gate. */
typedef struct {
    int valid;
    int16_t map, x, y;
    uint8_t tile;
} DM2_V1_Mode7TileCache;
typedef int (*DM2_V1_Mode7ReadTile)(void *ctx, int map, int x, int y,
                                    uint8_t *out_tile);
int dm2_v1_mode7_tile_cache_start(
    DM2_V1_Mode7TileCache *cache, int map, int x, int y,
    DM2_V1_Mode7ReadTile read_tile, void *ctx);
int dm2_v1_mode7_tile_cache_node(
    DM2_V1_Mode7TileCache *cache, uint8_t effective_flags,
    int map, int x, int y, DM2_V1_Mode7ReadTile read_tile, void *ctx);
int dm2_v1_mode7_tile_cache_action23_gate(
    const DM2_V1_Mode7TileCache *cache);

/* Live evidence needed for the flags-3 stone-room prepass. Class-5 cells
 * also require a bounded record scan excluding DBE/DBF darkness terms.
 * The producer authenticates teleporter, creature and GDAT F8 inputs. */
typedef struct {
    uint8_t valid;
    uint8_t teleporter_detail_known;
    uint8_t teleporter_present;
    uint8_t creature_query_known;
    uint8_t creature_present;
    uint8_t record_chain_known_no_darkness;
    uint8_t ceiling_gdat_known;
    uint16_t ceiling_gdat_light_word;
    uint16_t creature_f8_word;
} DM2_V1_Mode7Flags3Evidence;

/* One action-0x17 source call for a future shared FIND_WALK_PATH core.
 * SK1C9A's start/prepass uses flags 3, while its edge loop uses flags 4.
 * Returns 0 for a cached-gate skip, 1 for a resolved node, and -1 for an
 * unknown source branch. No result certifies traversal completion. */
typedef struct {
    uint8_t cached_tile;
    uint8_t effective_flags;
    uint8_t source_flags;
    uint8_t distance;
    uint8_t weather_index;
    uint8_t weather_delta;
    const DM2_V1_CLightFlags4FloorReceipt *floor;
    const DM2_V1_CLightStoneRoomReceipt *stone_room;
    const DM2_V1_Mode7Flags3Evidence *prepass;
} DM2_V1_Mode7Action23Node;
int dm2_v1_mode7_on_node(
    const DM2_V1_Mode7Action23Node *node,
    int16_t *v1e0974, int16_t *v1e0978);

static inline int16_t dm2_v1_between_value(int16_t lo, int16_t hi, int16_t val)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_LIGHT_OPS_PC34_COMPAT_H */
