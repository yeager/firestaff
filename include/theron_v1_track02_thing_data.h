#ifndef THERON_V1_TRACK02_THING_DATA_H
#define THERON_V1_TRACK02_THING_DATA_H

#include <stddef.h>
#include <stdint.h>

#define THERON_ITEM_CATEGORY_COUNT   16
#define THERON_MAX_GROUND_REFS       2048
#define THERON_MAX_ITEMS_PER_CAT     512

typedef enum {
    /* Source binding: DMBUILDER6/src/item.c:itemBytes[] and
     * DMBUILDER6/src/dm.h:CATEGORYTYPE; loading order is preserved by
     * loadTheronsQuestDungeonData() in DMBUILDER6/src/loaddungeon.c. */
    THERON_CAT_DOOR        = 0,
    THERON_CAT_TELEPORTER  = 1,
    THERON_CAT_TEXT        = 2,
    THERON_CAT_ACTUATOR    = 3,
    THERON_CAT_MONSTER     = 4,
    THERON_CAT_WEAPON      = 5,
    THERON_CAT_CLOTHING    = 6,
    THERON_CAT_SCROLL      = 7,
    THERON_CAT_POTION      = 8,
    THERON_CAT_CHEST       = 9,
    /* Compatibility spelling retained for callers that treat a chest as a
     * generic container; both names are the same source category. */
    THERON_CAT_CONTAINER   = THERON_CAT_CHEST,
    THERON_CAT_MISC        = 10,
    THERON_CAT_MISSILE     = 14,
    THERON_CAT_CLOUD       = 15,
} Theron_ItemCategory;

static const size_t theron_item_bytes[THERON_ITEM_CATEGORY_COUNT] = {
    4, 6, 4, 8,
    16, 4, 4, 4,
    4, 8, 4, 0,
    0, 0, 8, 4
};

typedef struct {
    uint16_t   ground_ref_count;
    uint16_t   ground_refs[THERON_MAX_GROUND_REFS];
    uint16_t   object_counts[THERON_ITEM_CATEGORY_COUNT];
    uint8_t    items[THERON_ITEM_CATEGORY_COUNT][THERON_MAX_ITEMS_PER_CAT * 16];
    uint16_t   text_data_count;
    uint16_t   text_data[1024];
} Theron_ThingData;

typedef struct {
    uint16_t next_ref;
    uint8_t type;
    uint8_t position;
    uint16_t health[4];
    uint8_t number;
    uint8_t direction_flags;
} Theron_Track02MonsterRecord;

typedef struct {
    uint8_t type;
    uint8_t keep;
    uint8_t cursed;
    uint8_t poisoned;
    uint8_t charges;
    uint8_t broken;
    uint8_t unknown;
} Theron_Track02WeaponRecord;

typedef struct {
    uint8_t type;
    uint8_t keep;
    uint8_t cursed;
    uint8_t dump;
    uint8_t broken;
    uint8_t unknown;
} Theron_Track02ClothingRecord;

typedef struct {
    uint16_t reftxt;
    uint8_t closed;
    uint8_t type;
} Theron_Track02ScrollRecord;

typedef struct {
    uint8_t power;
    uint8_t type;
    uint8_t unknown;
    uint8_t keep;
} Theron_Track02PotionRecord;

typedef struct {
    int16_t chested;
    uint16_t data1;
    uint16_t unknown;
} Theron_Track02ChestRecord;

typedef struct {
    uint8_t type;
    uint8_t keep;
    uint8_t unknown;
    uint8_t capacity;
} Theron_Track02MiscRecord;

typedef struct {
    unsigned int category;
    uint16_t next_ref;
    union {
        Theron_Track02MonsterRecord monster;
        Theron_Track02WeaponRecord weapon;
        Theron_Track02ClothingRecord clothing;
        Theron_Track02ScrollRecord scroll;
        Theron_Track02PotionRecord potion;
        Theron_Track02ChestRecord chest;
        Theron_Track02MiscRecord misc;
    } value;
} Theron_Track02ItemRecord;

int theron_v1_track02_thing_data_load(
    const uint8_t *ud_data,
    size_t ud_size,
    unsigned int dungeon_index,
    const uint16_t *object_counts,
    unsigned int ground_ref_count,
    Theron_ThingData *out);

unsigned int theron_v1_track02_compute_ground_ref_count(
    const uint8_t *tiles_flat,
    unsigned int total_tiles);

/* Decodes the source-bound fields for categories 4..10. Categories 14/15
 * intentionally remain raw-only until their source consumers are identified. */
int theron_v1_track02_item_record_decode(
    unsigned int category,
    const uint8_t *raw,
    size_t raw_size,
    Theron_Track02ItemRecord *out);

#endif
