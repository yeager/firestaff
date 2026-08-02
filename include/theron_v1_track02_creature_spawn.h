#ifndef THERON_V1_TRACK02_CREATURE_SPAWN_H
#define THERON_V1_TRACK02_CREATURE_SPAWN_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 *
 * Theron's Quest does NOT have a DM1-style G0243_CreatureInfo per-type
 * stat table.  Instead it uses a CATEGORY-BASED formula system where
 * creature TYPE determines appearance (sprite descriptor) while creature
 * CATEGORY (0-3) determines combat stats via formulas with random
 * multipliers at spawn time.
 *
 * Spawn code at UD 0x0870E5 (PCE $B0E5), identical across all 7 dungeon
 * banks.  HP capped at 900 (0x0384), same as DM1. */

/* Spawn zone descriptor — one per creature type that has regular spawns.
 * From UD 0x274058 (AKUTUBA) through UD 0x274150 (SHADO).
 * THIEF and DEMON have no spawn zones (scripted encounters). */
typedef struct {
    uint16_t map_width;
    uint16_t map_height;
    uint8_t  category;       /* combat category 0-3 */
    uint8_t  count;          /* max creatures per zone */
    uint8_t  param1;         /* scaling parameter */
    uint8_t  param2;         /* secondary parameter */
} Theron_SpawnZoneDesc;

#define THERON_CREATURE_SPAWN_CATEGORY_COUNT  4
#define THERON_CREATURE_HP_CAP                900

/* Category formula multipliers from disassembly:
 *   Cat 0: dice param = 4
 *   Cat 1: multiplier = 21 (0x15)
 *   Cat 2: multiplier = 25 (0x19), then *1.5
 *   Cat 3: dice param = 5, scaling + 1.5*adj */
typedef struct {
    uint8_t  category;
    uint8_t  multiplier;     /* base multiplier (21 or 25), or 0 for dice-based */
    uint8_t  dice_param;     /* random dice parameter (4 or 5), or 0 for mult-based */
    uint8_t  uses_1_5x;      /* 1 if result is scaled by 1.5 */
} Theron_SpawnCategoryFormula;

const Theron_SpawnZoneDesc *theron_v1_track02_spawn_zone(unsigned int creature_index);
const Theron_SpawnCategoryFormula *theron_v1_track02_spawn_formula(unsigned int category);
size_t theron_v1_track02_spawn_zone_count(void);

/* Creature pointer table entry (UD 0x274018, 8 bytes each).
 * sprite_desc_offset and spawn_data_offset are relative to UD 0x274000. */
typedef struct {
    uint16_t sprite_desc_offset;
    uint16_t constant_278a;
    uint16_t spawn_data_offset;
    uint16_t constant_016b;
} Theron_CreaturePointerEntry;

const Theron_CreaturePointerEntry *theron_v1_track02_creature_pointer(unsigned int index);

/* Computed creature stats from category formula + random seed.
 * The seed replaces rand() for deterministic replay. */
typedef struct {
    int16_t hp;
    int16_t attack;
    int16_t defense;
} Theron_SpawnStats;

void theron_v1_track02_compute_spawn_stats(
    unsigned int category, uint8_t param1, uint8_t param2,
    uint16_t rand_seed, Theron_SpawnStats *out);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_CREATURE_SPAWN_H */
