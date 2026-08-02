#ifndef DM2_V1_CREATURE_AI_SPEC_PC34_COMPAT_H
#define DM2_V1_CREATURE_AI_SPEC_PC34_COMPAT_H

/* DM2 creature AI specification table and two-stage lookup.
 * Source: skcrture.cpp:28-38 QUERY_CREATURE_AI_SPEC_FROM_TYPE,
 *         kaitable.h dAITableGenuine[62],
 *         dme.h:1715-1773 AIDefinition (36 bytes).
 *
 * The original lookup is:
 *   1. QUERY_GDAT_CREATURE_WORD_VALUE(creatureType, CREATURE_STAT_AI=0x05)
 *      -> returns AI row index
 *   2. dAITableGenuine[row] or _4976_03a2[row * 0x24]
 *      -> returns the 36-byte AIDefinition
 *
 * This module provides the genuine AI table and the lookup function
 * with callback-based GDAT query for the creature-type-to-AI-row mapping. */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_AI_TABLE_SIZE 62
#define DM2_AI_ENTRY_SIZE 36

/* AIDefinition — 36-byte packed creature AI specification.
 * Field names from dme.h:1715-1773. */
typedef struct {
    uint16_t flags;              /* w0: AI flags bitfield */
    uint8_t  armor_class;        /* b2: armor/defense */
    int8_t   field_03;           /* b3 */
    uint16_t base_hp;            /* w4: initial HP */
    uint8_t  attack_strength;    /* b6: attack strength */
    uint8_t  poison_damage;      /* b7: poison */
    uint8_t  defense;            /* b8: defense (255 = undestroyable) */
    uint8_t  special_flags;      /* b9: 0x40=pit ghost, etc. */
    uint16_t field_0a;           /* w10 */
    uint16_t field_0c;           /* w12 */
    uint16_t attacks_spells;     /* w14: attack and spell commands */
    uint16_t field_10;           /* w16 */
    uint16_t field_12;           /* w18: trigger switch capability */
    uint16_t field_14;           /* w20 */
    uint16_t field_16;           /* w22 */
    uint16_t resistance;         /* w24: fire/poison/magic resistance */
    uint16_t field_1a;           /* w26 */
    uint8_t  field_1c;           /* b28 */
    uint8_t  weight;             /* b29: push resistance (255 = immovable) */
    uint16_t field_1e;           /* w30: 0x0800 = can turn missiles */
    uint16_t field_20;           /* w32 */
    int8_t   field_22;           /* b34 */
    int8_t   cloud_size;         /* b35: death cloud size */
} DM2_AIDefinition;

/* AI flag accessors (bit positions from dme.h). */
#define DM2_AI_FLAG_STATIC_OBJECT   0x0001
#define DM2_AI_FLAG_REFLECTOR       0x0002
#define DM2_AI_FLAG_W0_2            0x0004
#define DM2_AI_FLAG_GHOSTLIKE       0x0008
#define DM2_AI_FLAG_W0_4            0x0010
#define DM2_AI_FLAG_NON_MATERIAL    0x0020
#define DM2_AI_FLAG_PUSH_ON_MOVE    0x0100
#define DM2_AI_FLAG_ABSORBS_MISSILE 0x0200
#define DM2_AI_FLAG_W0_A            0x0400
#define DM2_AI_FLAG_W0_B            0x0800
#define DM2_AI_FLAG_DRAGOTH_SPECIAL 0x1000
#define DM2_AI_FLAG_CAN_TRAVEL_MAPS 0x4000
#define DM2_AI_FLAG_BLOCKS_ROPE     0x8000

/* Get the genuine AI table (62 entries, 36 bytes each).
 * Returns pointer to static const data. */
const DM2_AIDefinition *dm2_v1_creature_ai_table(void);

/* Get AI entry count. */
int dm2_v1_creature_ai_table_count(void);

/* Query AI spec for a creature type.
 * Uses the two-stage lookup:
 *   1. query_gdat_word(creature_type, 0x05) -> AI row index
 *   2. ai_table[row]
 *
 * query_gdat_word: callback to get GDAT creature word value.
 *   Takes (ctx, creature_type, stat_id) and returns the word value.
 *   stat_id will be 0x05 (CREATURE_STAT_AI).
 *
 * Returns pointer to the AI entry, or NULL if row is out of range. */
typedef uint16_t (*DM2_QueryGdatCreatureWordFn)(void *ctx,
    uint8_t creature_type, uint8_t stat_id);

const DM2_AIDefinition *dm2_v1_creature_ai_spec_from_type(
    uint8_t creature_type,
    DM2_QueryGdatCreatureWordFn query_gdat_word,
    void *ctx);

/* Direct table access by AI row index.
 * Returns pointer to the AI entry, or NULL if out of range. */
const DM2_AIDefinition *dm2_v1_creature_ai_spec_by_row(int row);

/* Convenience: is this AI entry a static object? */
int dm2_v1_creature_ai_is_static(const DM2_AIDefinition *ai);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_CREATURE_AI_SPEC_PC34_COMPAT_H */
