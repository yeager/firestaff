#include "dm1_v1_chest_admission_f0333_f0334_pc34_compat.h"

#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <string.h>

static uint32_t f0333_hash(const unsigned char *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t i;
    if (!bytes || !count) return 0u;
    for (i = 0; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int f0333_raw_size(int type)
{
    static const unsigned char sizes[DUNGEON_THING_TYPE_COUNT] = {
        4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
    };
    return type >= 0 && type < DUNGEON_THING_TYPE_COUNT ? sizes[type] : 0;
}

static int f0333_valid_thing(const struct DungeonThings_Compat *things,
                             unsigned short thing, uint32_t *ioHash)
{
    const unsigned char *raw;
    int type;
    int index;
    int size;
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) return 0;
    type = (int)THING_GET_TYPE(thing);
    index = (int)THING_GET_INDEX(thing);
    if (type < 0 || type >= DUNGEON_THING_TYPE_COUNT) return 0;
    size = f0333_raw_size(type);
    raw = dm1_v1_dungeon_get_thing_data_pc34(things, thing);
    if (!raw || !size || index < 0 || index >= things->thingCounts[type]) return 0;
    *ioHash ^= f0333_hash(raw, (size_t)size);
    *ioHash *= 16777619u;
    return *ioHash != 0u;
}

static int f0333_base(const struct DungeonThings_Compat *things,
                      unsigned short containerThing,
                      DM1_ChestAdmissionReceiptF0333F0334Pc34 *receipt,
                      const unsigned char **outRaw)
{
    const unsigned char *raw;
    int index;
    if (!things || !things->loaded || !things->containers ||
        containerThing == THING_NONE || containerThing == THING_ENDOFLIST ||
        THING_GET_TYPE(containerThing) != THING_TYPE_CONTAINER) return 0;
    index = (int)THING_GET_INDEX(containerThing);
    raw = dm1_v1_dungeon_get_thing_data_pc34(things, containerThing);
    if (!raw || index < 0 || index >= things->containerCount ||
        index >= things->thingCounts[THING_TYPE_CONTAINER] ||
        things->containers[index].next !=
            (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8)) ||
        things->containers[index].slot !=
            (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8))) return 0;
    receipt->containerThing = containerThing;
    receipt->rawContainerFNV1a = f0333_hash(raw, 8u);
    receipt->rawChainFNV1a = 2166136261u;
    if (!receipt->rawContainerFNV1a) return 0;
    if (outRaw) *outRaw = raw;
    return 1;
}

int dm1_v1_chest_open_admit_f0333_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short containerThing,
    DM1_ChestAdmissionReceiptF0333F0334Pc34 *outReceipt)
{
    DM1_ChestAdmissionReceiptF0333F0334Pc34 receipt;
    unsigned short thing;
    int index;
    int i;
    int steps = 0;
    int maximumSteps = 0;
    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    for (i = 0; i < DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34; ++i)
        receipt.slots[i] = THING_NONE;
    receipt.sourceAnchor =
        "ReDMCSB CHEST.C F0333:30-75; DUNGEON.C F0156/F0159 raw C09 chain";
    *outReceipt = receipt;
    if (!f0333_base(things, containerThing, &receipt, NULL)) return 1;
    for (i = 0; i < DUNGEON_THING_TYPE_COUNT; ++i) {
        if (things->thingCounts[i] > 0) maximumSteps += things->thingCounts[i];
    }
    if (maximumSteps <= 0) return 1;
    index = (int)THING_GET_INDEX(containerThing);
    thing = things->containers[index].slot;
    while (thing != THING_ENDOFLIST && thing != THING_NONE) {
        int prior;
        if (++steps > maximumSteps) return 1;
        for (prior = 0; prior < receipt.slotCount; ++prior)
            if (receipt.slots[prior] == thing) return 1;
        if (!f0333_valid_thing(things, thing, &receipt.rawChainFNV1a)) return 1;
        if (receipt.slotCount < DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34) {
            receipt.slots[receipt.slotCount++] = thing;
        }
        thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
    }
    receipt.valid = receipt.rawChainFNV1a != 0u;
    *outReceipt = receipt;
    return 1;
}

int dm1_v1_chest_close_admit_f0334_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short containerThing,
    const unsigned short slots[DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34],
    DM1_ChestAdmissionReceiptF0333F0334Pc34 *outReceipt)
{
    DM1_ChestAdmissionReceiptF0333F0334Pc34 receipt;
    int i;
    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    for (i = 0; i < DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34; ++i)
        receipt.slots[i] = THING_NONE;
    receipt.sourceAnchor =
        "ReDMCSB CHEST.C F0334:112-133; DUNGEON.C F0156/F0163 raw C09 rewrite";
    *outReceipt = receipt;
    if (!slots || !f0333_base(things, containerThing, &receipt, NULL)) return 1;
    for (i = 0; i < DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34; ++i) {
        int prior;
        unsigned short thing = slots[i];
        if (thing == THING_NONE || thing == THING_ENDOFLIST) continue;
        for (prior = 0; prior < i; ++prior)
            if (receipt.slots[prior] == thing) return 1;
        if (!f0333_valid_thing(things, thing, &receipt.rawChainFNV1a)) return 1;
        receipt.slots[i] = thing;
        ++receipt.slotCount;
    }
    receipt.valid = receipt.rawChainFNV1a != 0u;
    *outReceipt = receipt;
    return 1;
}

static void f0334_set_decoded_next(struct DungeonThings_Compat *things,
                                   int type, int index, unsigned short next)
{
    switch (type) {
    case THING_TYPE_DOOR: if (things->doors && index < things->doorCount) things->doors[index].next = next; break;
    case THING_TYPE_TELEPORTER: if (things->teleporters && index < things->teleporterCount) things->teleporters[index].next = next; break;
    case THING_TYPE_TEXTSTRING: if (things->textStrings && index < things->textStringCount) things->textStrings[index].next = next; break;
    case THING_TYPE_SENSOR: if (things->sensors && index < things->sensorCount) things->sensors[index].next = next; break;
    case THING_TYPE_GROUP: if (things->groups && index < things->groupCount) things->groups[index].next = next; break;
    case THING_TYPE_WEAPON: if (things->weapons && index < things->weaponCount) things->weapons[index].next = next; break;
    case THING_TYPE_ARMOUR: if (things->armours && index < things->armourCount) things->armours[index].next = next; break;
    case THING_TYPE_SCROLL: if (things->scrolls && index < things->scrollCount) things->scrolls[index].next = next; break;
    case THING_TYPE_POTION: if (things->potions && index < things->potionCount) things->potions[index].next = next; break;
    case THING_TYPE_CONTAINER: if (things->containers && index < things->containerCount) things->containers[index].next = next; break;
    case THING_TYPE_JUNK: if (things->junks && index < things->junkCount) things->junks[index].next = next; break;
    case THING_TYPE_PROJECTILE: if (things->projectiles && index < things->projectileCount) things->projectiles[index].next = next; break;
    case THING_TYPE_EXPLOSION: if (things->explosions && index < things->explosionCount) things->explosions[index].next = next; break;
    default: break;
    }
}

static int f0334_set_raw_next(struct DungeonThings_Compat *things,
                              unsigned short thing, unsigned short next)
{
    int type = (int)THING_GET_TYPE(thing);
    int index = (int)THING_GET_INDEX(thing);
    int size = f0333_raw_size(type);
    unsigned char *raw;
    if (!things || type < 0 || type >= DUNGEON_THING_TYPE_COUNT || !size ||
        index < 0 || index >= things->thingCounts[type] ||
        !things->rawThingData[type]) return 0;
    raw = things->rawThingData[type] + index * size;
    raw[0] = (unsigned char)(next & 0xffu);
    raw[1] = (unsigned char)(next >> 8);
    f0334_set_decoded_next(things, type, index, next);
    return 1;
}

int dm1_v1_chest_close_commit_f0334_pc34(
    struct DungeonThings_Compat *things,
    unsigned short containerThing,
    const unsigned short slots[DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34],
    DM1_ChestAdmissionReceiptF0333F0334Pc34 *outReceipt)
{
    DM1_ChestAdmissionReceiptF0333F0334Pc34 receipt;
    unsigned short first = THING_ENDOFLIST;
    unsigned short previous = THING_ENDOFLIST;
    int containerIndex;
    int i;
    if (!outReceipt || !dm1_v1_chest_close_admit_f0334_pc34(
            things, containerThing, slots, &receipt) || !receipt.valid) return 0;
    containerIndex = (int)THING_GET_INDEX(containerThing);
    for (i = 0; i < DM1_CHEST_VISIBLE_SLOT_COUNT_F0333_F0334_PC34; ++i) {
        unsigned short thing = slots[i];
        if (thing == THING_NONE || thing == THING_ENDOFLIST) continue;
        if (first == THING_ENDOFLIST) first = thing;
        if (previous != THING_ENDOFLIST && !f0334_set_raw_next(things, previous, thing)) return 0;
        previous = thing;
    }
    if (previous != THING_ENDOFLIST && !f0334_set_raw_next(things, previous, THING_ENDOFLIST)) return 0;
    things->containers[containerIndex].slot = first;
    things->rawThingData[THING_TYPE_CONTAINER][containerIndex * 8 + 2] =
        (unsigned char)(first & 0xffu);
    things->rawThingData[THING_TYPE_CONTAINER][containerIndex * 8 + 3] =
        (unsigned char)(first >> 8);
    *outReceipt = receipt;
    return 1;
}
