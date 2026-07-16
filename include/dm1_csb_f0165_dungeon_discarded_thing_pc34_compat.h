#ifndef FIRESTAFF_DM1_CSB_F0165_DUNGEON_DISCARDED_THING_PC34_COMPAT_H
#define FIRESTAFF_DM1_CSB_F0165_DUNGEON_DISCARDED_THING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DUNGEON.C F0165_DUNGEON_GetDiscardedThing (1923-2075).
 * The callbacks expose already-owned dungeon records and list mutations.
 * This adapter never allocates, clears, links, or manufactures a Thing. */

#define DM1_CSB_F0165_THING_NONE       0xffffu
#define DM1_CSB_F0165_THING_ENDOFLIST  0xfffeu

enum {
    DM1_CSB_F0165_THING_TYPE_SENSOR = 3,
    DM1_CSB_F0165_THING_TYPE_GROUP = 4,
    DM1_CSB_F0165_THING_TYPE_WEAPON = 5,
    DM1_CSB_F0165_THING_TYPE_ARMOUR = 6,
    DM1_CSB_F0165_THING_TYPE_POTION = 8,
    DM1_CSB_F0165_THING_TYPE_JUNK = 10,
    DM1_CSB_F0165_THING_TYPE_PROJECTILE = 14,
    DM1_CSB_F0165_THING_TYPE_EXPLOSION = 15
};

enum DM1_CSB_F0165_DiscardAction {
    DM1_CSB_F0165_DISCARD_GROUP,
    DM1_CSB_F0165_DISCARD_PROJECTILE,
    DM1_CSB_F0165_DISCARD_OBJECT
};

typedef struct {
    uint8_t last_discarded_map[16];
} DM1_CSB_F0165_DiscardState;

typedef struct {
    int (*get_map_bounds)(void *context, uint16_t map_index,
                          uint16_t *out_max_x, uint16_t *out_max_y);
    uint16_t (*get_first_thing)(void *context, uint16_t map_index,
                                uint16_t map_x, uint16_t map_y);
    uint16_t (*get_next_thing)(void *context, uint16_t thing);
    uint8_t (*get_thing_type)(void *context, uint16_t thing);
    int (*sensor_is_enabled)(void *context, uint16_t thing);
    /* For GROUP this covers DoNotDiscard or a non-empty possession slot;
     * for armour, weapon, potion, and junk it is DoNotDiscard. */
    int (*thing_is_protected)(void *context, uint16_t thing);
    /* Owns all source mutations: group possessions/drop-delete, projectile
     * event/unlink-delete, or F0267-style object removal respectively. */
    int (*discard_thing)(void *context, enum DM1_CSB_F0165_DiscardAction action,
                         uint16_t thing, uint16_t map_index,
                         uint16_t map_x, uint16_t map_y);
} DM1_CSB_F0165_DungeonOps;

/* Returns a type-and-index THING, with cell bits cleared, or THING_NONE.
 * `party_map_index` must be a valid map index; map dimensions are inclusive
 * maxima, matching DUNGEON.C's `x <= Width`, `y <= Height` scan. */
uint16_t F0165_DUNGEON_GetDiscardedThing(
    DM1_CSB_F0165_DiscardState *state,
    const DM1_CSB_F0165_DungeonOps *ops,
    void *context,
    uint16_t thing_type,
    uint16_t map_count,
    uint16_t party_map_index,
    int16_t party_map_x,
    int16_t party_map_y);

uint16_t F0165_DUNGEON_GetDiscardedThing_Compat(
    DM1_CSB_F0165_DiscardState *state,
    const DM1_CSB_F0165_DungeonOps *ops,
    void *context,
    uint16_t thing_type,
    uint16_t map_count,
    uint16_t party_map_index,
    int16_t party_map_x,
    int16_t party_map_y);

#ifdef __cplusplus
}
#endif

#endif
