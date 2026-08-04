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
