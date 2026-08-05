#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "firestaff/dm1/v1/ordered_cells_to_attack_pc34_compat.h"

#include <string.h>

enum {
    kObjectInfoFirstScroll = 0,
    kObjectInfoFirstContainer = 1,
    kObjectInfoFirstPotion = 2,
    kObjectInfoFirstWeapon = 23,
    kObjectInfoFirstArmour = 69,
    kObjectInfoFirstJunk = 127,
    kObjectInfoCount = 180,

    kIconWeaponTorchUnlit = 4,
    kIconJunkWater = 8,
    kIconJunkJewelSymalUnequipped = 10,
    kIconJunkIllumuletUnequipped = 12,
    kIconWeaponFlamittEmpty = 14,
    kIconWeaponEyeOfTimeEmpty = 16,
    kIconWeaponStormringEmpty = 18,
    kIconWeaponStaffOfClawsEmpty = 20,
    kIconWeaponBoltBladeStormEmpty = 23,
    kIconWeaponFuryRaBladeEmpty = 25,
    kIconScrollOpen = 30,
    kIconJunkCompassNorth = 0
};

/* ReDMCSB DUNGEON.C G0237_as_Graphic559_ObjectInfo[180].Type, PC3.4 I34E.
 * F0032 returns exactly this column after F0141 selects the row. */
static const unsigned char s_object_info_type[kObjectInfoCount] = {
    30, 144, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157,
    158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 195, 16,
    18, 4, 14, 20, 23, 25, 27, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64, 65, 66, 135, 143, 28, 80, 81, 82,
    112, 114, 67, 83, 68, 84, 69, 70, 85, 86, 71, 87,
    119, 72, 88, 113, 89, 73, 74, 90, 103, 104, 96, 97,
    98, 105, 106, 108, 107, 75, 91, 76, 92, 99, 115, 100,
    77, 93, 116, 109, 101, 78, 94, 117, 110, 102, 79, 95,
    118, 111, 140, 141, 142, 194, 196, 0, 8, 10, 12, 146,
    147, 125, 126, 127, 176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191, 128, 129, 130, 131,
    168, 169, 170, 171, 172, 173, 174, 175, 120, 121, 122, 123,
    124, 132, 133, 134, 136, 137, 138, 139, 192, 193, 197, 198,
};

/* ReDMCSB DUNGEON.C G0237_as_Graphic559_ObjectInfo[180].AllowedSlots.
 * F0141 selects this row from the loaded PC3.4 Thing before CHAMPION.C
 * F0302/F0303 decides whether a hand, inventory, or chest slot may own it. */
static const unsigned short s_object_info_allowed_slots[kObjectInfoCount] = {
    0x0500, 0x0200, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0501, 0x0501, 0x0501, 0x0501,
    0x0501, 0x0501, 0x0501, 0x0501, 0x0501, 0x0501, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500,
    0x0500, 0x0400, 0x0400, 0x0040, 0x0040, 0x0040, 0x0040, 0x05C0, 0x0040, 0x0040, 0x0040, 0x0040,
    0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0040, 0x0440, 0x0040, 0x0040,
    0x0040, 0x0040, 0x05C0, 0x05C0, 0x0440, 0x05C0, 0x05C0, 0x05C0, 0x0040, 0x0040, 0x0540, 0x0540,
    0x0040, 0x0040, 0x0040, 0x0040, 0x0440, 0x0040, 0x0440, 0x0040, 0x0040, 0x040C, 0x040C, 0x0410,
    0x0420, 0x0420, 0x0408, 0x0410, 0x0408, 0x0410, 0x0408, 0x0408, 0x0410, 0x0410, 0x0408, 0x0410,
    0x0420, 0x0408, 0x0410, 0x0420, 0x0410, 0x0408, 0x0408, 0x0410, 0x0402, 0x0402, 0x0402, 0x0402,
    0x0402, 0x0400, 0x0200, 0x0200, 0x0200, 0x0408, 0x0410, 0x0408, 0x0410, 0x0402, 0x0420, 0x0402,
    0x0008, 0x0010, 0x0420, 0x0200, 0x0402, 0x0008, 0x0010, 0x0420, 0x0200, 0x0402, 0x0008, 0x0010,
    0x0420, 0x0200, 0x0402, 0x0408, 0x0010, 0x0420, 0x0408, 0x0500, 0x0501, 0x0504, 0x0504, 0x0500,
    0x0400, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500,
    0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0500, 0x0200, 0x0500, 0x0500, 0x0500,
    0x0501, 0x0501, 0x0501, 0x0501, 0x0401, 0x0401, 0x0501, 0x0501, 0x0504, 0x0504, 0x0504, 0x0504,
    0x0504, 0x0500, 0x0500, 0x0500, 0x0400, 0x0500, 0x0500, 0x0504, 0x0500, 0x0500, 0x0000, 0x0400
};

/* ReDMCSB DUNGEON.C G0243_as_Graphic559_CreatureInfo[27].Attributes,
 * PC 3.4 I34E.  F0144 must use the raw C04 GROUP.Type, not a decoded
 * runtime mirror that may lag a loaded/save-game record. */
static const unsigned short s_creature_attributes_f0144_pc34[27] = {
    0x0482, 0x0480, 0x4510, 0x04B4, 0x0701, 0x0581, 0x070C,
    0x0300, 0x5864, 0x0282, 0x1480, 0x18C6, 0x1280, 0x14A2,
    0x05B8, 0x0381, 0x0680, 0x04A0, 0x0280, 0x4060, 0x10DE,
    0x0082, 0x1480, 0x78AA, 0x068A, 0x78AA, 0x78AA
};

static unsigned char *dm1_v1_dungeon_group_raw_mutable_pc34(
    struct DungeonThings_Compat *things,
    int groupIndex)
{
    if (!things || !things->loaded || !things->groups || groupIndex < 0 ||
        groupIndex >= things->groupCount ||
        groupIndex >= things->thingCounts[THING_TYPE_GROUP] ||
        !things->rawThingData[THING_TYPE_GROUP]) {
        return NULL;
    }
    return things->rawThingData[THING_TYPE_GROUP] + groupIndex * 16;
}

static const unsigned char *dm1_v1_dungeon_group_raw_pc34(
    const struct DungeonThings_Compat *things,
    int groupIndex)
{
    if (!things || !things->loaded || !things->groups || groupIndex < 0 ||
        groupIndex >= things->groupCount ||
        groupIndex >= things->thingCounts[THING_TYPE_GROUP] ||
        !things->rawThingData[THING_TYPE_GROUP]) {
        return NULL;
    }
    return things->rawThingData[THING_TYPE_GROUP] + groupIndex * 16;
}

static int dm1_v1_dungeon_active_group_index_pc34(
    const struct CreatureAIState_Compat *activeGroups,
    int activeGroupCount,
    int groupIndex)
{
    int index;

    if (!activeGroups || activeGroupCount < 0 || groupIndex < 0) return -1;
    for (index = 0; index < activeGroupCount; ++index) {
        if (activeGroups[index].reserved0 == groupIndex) return index;
    }
    return -1;
}

int dm1_v1_dungeon_get_group_cells_f0145_pc34(
    const struct DungeonThings_Compat *things,
    const struct CreatureAIState_Compat *activeGroups,
    int activeGroupCount,
    int partyMapIndex,
    int mapIndex,
    int groupIndex,
    unsigned int *outCells)
{
    const unsigned char *raw;
    int activeIndex;

    if (outCells) *outCells = 0;
    if (!outCells) return 0;
    raw = dm1_v1_dungeon_group_raw_pc34(things, groupIndex);
    if (!raw) return 0;
    if (mapIndex != partyMapIndex) {
        *outCells = raw[5];
        return 1;
    }
    activeIndex = dm1_v1_dungeon_active_group_index_pc34(
        activeGroups, activeGroupCount, groupIndex);
    if (activeIndex < 0) return 0;
    *outCells = (unsigned int)activeGroups[activeIndex].groupCells & 0xffu;
    return 1;
}

int dm1_v1_dungeon_set_group_cells_f0146_pc34(
    struct DungeonThings_Compat *things,
    struct CreatureAIState_Compat *activeGroups,
    int activeGroupCount,
    int partyMapIndex,
    int mapIndex,
    int groupIndex,
    unsigned int cells)
{
    unsigned char *raw;
    int activeIndex;

    raw = dm1_v1_dungeon_group_raw_mutable_pc34(things, groupIndex);
    if (!raw) return 0;
    if (mapIndex != partyMapIndex) {
        things->groups[groupIndex].cells = (unsigned char)(cells & 0xffu);
        raw[5] = (unsigned char)(cells & 0xffu);
        return 1;
    }
    activeIndex = dm1_v1_dungeon_active_group_index_pc34(
        activeGroups, activeGroupCount, groupIndex);
    if (activeIndex < 0) return 0;
    activeGroups[activeIndex].groupCells = (int)(cells & 0xffu);
    return 1;
}

int dm1_v1_dungeon_get_group_directions_f0147_pc34(
    const struct DungeonThings_Compat *things,
    const struct CreatureAIState_Compat *activeGroups,
    int activeGroupCount,
    int partyMapIndex,
    int mapIndex,
    int groupIndex,
    unsigned int *outDirections)
{
    static const unsigned char groupDirections[4] = {
        0x00u, 0x55u, 0xaau, 0xffu
    };
    const unsigned char *raw;
    int activeIndex;

    if (outDirections) *outDirections = 0;
    if (!outDirections) return 0;
    raw = dm1_v1_dungeon_group_raw_pc34(things, groupIndex);
    if (!raw) return 0;
    if (mapIndex != partyMapIndex) {
        *outDirections = groupDirections[raw[15] & 0x03u];
        return 1;
    }
    activeIndex = dm1_v1_dungeon_active_group_index_pc34(
        activeGroups, activeGroupCount, groupIndex);
    if (activeIndex < 0) return 0;
    *outDirections = (unsigned int)activeGroups[activeIndex].groupDirection &
        0xffu;
    return 1;
}

int dm1_v1_dungeon_set_group_directions_f0148_pc34(
    struct DungeonThings_Compat *things,
    struct CreatureAIState_Compat *activeGroups,
    int activeGroupCount,
    int partyMapIndex,
    int mapIndex,
    int groupIndex,
    unsigned int directions)
{
    unsigned char *raw;
    int activeIndex;

    raw = dm1_v1_dungeon_group_raw_mutable_pc34(things, groupIndex);
    if (!raw) return 0;
    if (mapIndex != partyMapIndex) {
        things->groups[groupIndex].direction = (unsigned char)(directions & 3u);
        raw[15] = (unsigned char)((raw[15] & ~0x03u) | (directions & 3u));
        return 1;
    }
    if (directions > 0xffu) return 0;
    activeIndex = dm1_v1_dungeon_active_group_index_pc34(
        activeGroups, activeGroupCount, groupIndex);
    if (activeIndex < 0) return 0;
    activeGroups[activeIndex].groupDirection = (int)directions;
    return 1;
}

const unsigned char *dm1_v1_dungeon_get_thing_data_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing)
{
    int type;
    int index;
    int byteCount;

    if (!things || !things->loaded || thing == THING_NONE ||
        thing == THING_ENDOFLIST) {
        return NULL;
    }

    type = THING_GET_TYPE(thing);
    index = THING_GET_INDEX(thing);
    if (type < 0 || type >= DUNGEON_THING_TYPE_COUNT || index < 0 ||
        index >= things->thingCounts[type] || !things->rawThingData[type]) {
        return NULL;
    }

    byteCount = s_thingDataByteCount[type];
    if (byteCount <= 0) return NULL;

    /* DUNGEON.C F0156: type base plus index times the original record size. */
    return things->rawThingData[type] + index * byteCount;
}

int dm1_v1_dungeon_get_creature_attributes_f0144_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing,
    unsigned short *outAttributes)
{
    const unsigned char *rawGroup;
    int creatureType;

    if (outAttributes) *outAttributes = 0;
    if (!outAttributes || THING_GET_TYPE(thing) != THING_TYPE_GROUP) return 0;
    rawGroup = dm1_v1_dungeon_get_thing_data_pc34(things, thing);
    if (!rawGroup) return 0;

    /* PC3.4 GROUP.Type is raw byte 4 of the 16-byte C04 record. */
    creatureType = rawGroup[4];
    if (creatureType < 0 ||
        creatureType >= (int)(sizeof(s_creature_attributes_f0144_pc34) /
                              sizeof(s_creature_attributes_f0144_pc34[0]))) {
        return 0;
    }
    *outAttributes = s_creature_attributes_f0144_pc34[creatureType];
    return 1;
}

int dm1_v1_dungeon_is_creature_allowed_on_map_f0139_pc34(
    const struct DungeonThings_Compat *things,
    const struct DungeonDatState_Compat *dungeon,
    unsigned short groupThing,
    int mapIndex,
    DM1_V1_F0139_CreatureAllowedOnMapReceipt_PC34 *outReceipt)
{
    DM1_V1_F0139_CreatureAllowedOnMapReceipt_PC34 receipt;
    const struct DungeonMapDesc_Compat *map;
    const unsigned char *raw;
    int groupIndex;
    int i;

    memset(&receipt, 0, sizeof(receipt));
    receipt.groupThing = groupThing;
    receipt.groupIndex = -1;
    receipt.mapIndex = mapIndex;
    receipt.creatureType = -1;
    receipt.matchedIndex = -1;
    receipt.sourceLineStart = 1052;
    receipt.sourceLineEnd = 1079;
    receipt.sourceSymbol = "F0139_DUNGEON_IsCreatureAllowedOnMap";

    if (!things || !dungeon || !dungeon->loaded || !dungeon->maps ||
        mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount ||
        THING_GET_TYPE(groupThing) != THING_TYPE_GROUP) {
        if (outReceipt) *outReceipt = receipt;
        return 0;
    }

    groupIndex = (int)THING_GET_INDEX(groupThing);
    receipt.groupIndex = groupIndex;
    raw = dm1_v1_dungeon_get_thing_data_pc34(things, groupThing);
    if (!raw) {
        if (outReceipt) *outReceipt = receipt;
        return 0;
    }

    map = &dungeon->maps[mapIndex];
    receipt.creatureType = (int)raw[4];
    receipt.allowedCreatureTypeCount = (int)map->creatureTypeCount;
    if (map->creatureTypeCount > sizeof(map->allowedCreatureTypes)) {
        if (outReceipt) *outReceipt = receipt;
        return 0;
    }

    receipt.valid = 1;
    for (i = 0; i < (int)map->creatureTypeCount; ++i) {
        if ((int)map->allowedCreatureTypes[i] == receipt.creatureType) {
            receipt.matchedIndex = i;
            if (outReceipt) *outReceipt = receipt;
            return 1;
        }
    }

    if (outReceipt) *outReceipt = receipt;
    return 0;
}

unsigned short dm1_v1_group_get_thing_f0175_pc34(
    const struct DungeonDatState_Compat *dungeon,
    const struct DungeonThings_Compat *things,
    int mapIndex,
    int mapX,
    int mapY)
{
    unsigned short thing;

    /* GROUP.C F0175: F0161 establishes the real square-list head, then
     * F0159 advances until the first C04 GROUP or ENDOFLIST. */
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        dungeon, things, mapIndex, mapX, mapY);
    while (thing != THING_ENDOFLIST && THING_GET_TYPE(thing) != THING_TYPE_GROUP) {
        thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
    }
    return thing;
}

int dm1_v1_group_get_creature_ordinal_in_cell_f0176_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short groupThing,
    unsigned int groupCells,
    unsigned int groupDirections,
    unsigned int cell)
{
    const unsigned char *rawGroup;
    unsigned short creatureAttributes;
    unsigned int creatureIndex;
    unsigned int creatureCell;

    if (THING_GET_TYPE(groupThing) != THING_TYPE_GROUP ||
        !dm1_v1_dungeon_get_creature_attributes_f0144_pc34(
            things, groupThing, &creatureAttributes)) {
        return 0;
    }
    rawGroup = dm1_v1_dungeon_get_thing_data_pc34(things, groupThing);
    if (!rawGroup) return 0;

    /* GROUP.C F0176: a centered creature is present in every cell. */
    if ((groupCells & 0xffu) == 0xffu) return 1;

    creatureIndex = ((unsigned int)rawGroup[14] >> 5) & 0x03u;
    cell &= 0x03u;
    if ((creatureAttributes & 0x0003u) == 0x0001u) {
        if ((groupDirections & 0x0001u) == (cell & 0x0001u)) {
            cell = (cell + 3u) & 0x03u;
        }
        do {
            creatureCell = (groupCells >> (creatureIndex << 1)) & 0x03u;
            if (creatureCell == cell || creatureCell == ((cell + 1u) & 0x03u)) {
                return (int)creatureIndex + 1;
            }
        } while (creatureIndex-- != 0u);
    } else {
        do {
            creatureCell = (groupCells >> (creatureIndex << 1)) & 0x03u;
            if (creatureCell == cell) return (int)creatureIndex + 1;
        } while (creatureIndex-- != 0u);
    }
    return 0;
}

int dm1_v1_group_get_melee_target_ordinal_f0177_pc34(
    const struct DungeonDatState_Compat *dungeon,
    const struct DungeonThings_Compat *things,
    int mapIndex,
    int groupMapX,
    int groupMapY,
    int partyMapX,
    int partyMapY,
    unsigned int championCell,
    unsigned int groupCells,
    unsigned int groupDirections)
{
    unsigned short groupThing;
    unsigned int direction;
    unsigned int row;
    int cellIndex;
    int creatureOrdinal;

    groupThing = dm1_v1_group_get_thing_f0175_pc34(
        dungeon, things, mapIndex, groupMapX, groupMapY);
    if (groupThing == THING_ENDOFLIST) return 0;

    /* F0228's direct cardinal branches are the complete F0177 melee
     * contract: a non-adjacent/diagonal target has no invented direction. */
    if (groupMapX == partyMapX && groupMapY != partyMapY) {
        direction = groupMapY > partyMapY ? 0u : 2u;
    } else if (groupMapY == partyMapY && groupMapX != partyMapX) {
        direction = groupMapX > partyMapX ? 3u : 1u;
    } else {
        return 0;
    }

    /* PROJEXPL.C F0229: direction*2, vertical CellSource increment, then
     * G0023's selected row. */
    row = direction << 1;
    if ((row & 0x0002u) == 0u) championCell++;
    row += (championCell >> 1) & 0x0001u;
    for (cellIndex = 0; cellIndex < 4; ++cellIndex) {
        int cell = dm1_v1_ordered_cells_to_attack_pc34((int)row, cellIndex);
        if (cell < 0) return 0;
        creatureOrdinal = dm1_v1_group_get_creature_ordinal_in_cell_f0176_pc34(
            things, groupThing, groupCells, groupDirections, (unsigned int)cell);
        if (creatureOrdinal != 0) return creatureOrdinal;
    }
    return 0;
}

int dm1_v1_dungeon_get_object_subtype_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing)
{
    const unsigned char *raw = dm1_v1_dungeon_get_thing_data_pc34(things, thing);
    int type;

    if (!raw) return -1;
    type = (int)THING_GET_TYPE(thing);
    switch (type) {
    case THING_TYPE_SCROLL:
        return 0;
    case THING_TYPE_CONTAINER:
        return (int)((raw[4] >> 1) & 0x03u);
    case THING_TYPE_POTION:
        return (int)(raw[3] & 0x7fu);
    case THING_TYPE_WEAPON:
    case THING_TYPE_ARMOUR:
    case THING_TYPE_JUNK:
        return (int)(raw[2] & 0x7fu);
    default:
        return -1;
    }
}

int dm1_v1_dungeon_get_object_info_index_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing)
{
    int subtype = dm1_v1_dungeon_get_object_subtype_pc34(things, thing);
    int index;

    if (subtype < 0) return -1;
    switch (THING_GET_TYPE(thing)) {
    case THING_TYPE_SCROLL:
        index = kObjectInfoFirstScroll;
        break;
    case THING_TYPE_CONTAINER:
        index = kObjectInfoFirstContainer + subtype;
        break;
    case THING_TYPE_POTION:
        index = kObjectInfoFirstPotion + subtype;
        break;
    case THING_TYPE_WEAPON:
        index = kObjectInfoFirstWeapon + subtype;
        break;
    case THING_TYPE_ARMOUR:
        index = kObjectInfoFirstArmour + subtype;
        break;
    case THING_TYPE_JUNK:
        index = kObjectInfoFirstJunk + subtype;
        break;
    default:
        return -1;
    }
    return index >= 0 && index < kObjectInfoCount ? index : -1;
}

int dm1_v1_dungeon_get_object_type_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing)
{
    int objectInfoIndex = dm1_v1_dungeon_get_object_info_index_pc34(things, thing);
    return objectInfoIndex >= 0 ? s_object_info_type[objectInfoIndex] : -1;
}

unsigned int dm1_v1_dungeon_get_object_allowed_slots_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing)
{
    int objectInfoIndex = dm1_v1_dungeon_get_object_info_index_pc34(things,
                                                                      thing);
    return objectInfoIndex >= 0
        ? (unsigned int)s_object_info_allowed_slots[objectInfoIndex]
        : 0u;
}

int dm1_v1_dungeon_get_object_icon_index_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing,
    int partyDirection)
{
    const unsigned char *raw;
    int iconIndex = dm1_v1_dungeon_get_object_type_pc34(things, thing);

    if (iconIndex < 0) return -1;
    raw = dm1_v1_dungeon_get_thing_data_pc34(things, thing);
    if (!raw) return -1;

    switch (iconIndex) {
    case kIconJunkCompassNorth:
        return iconIndex + (partyDirection & 3);
    case kIconWeaponTorchUnlit:
        if (raw[3] & 0x80u) {
            static const unsigned char torchTypeForCharge[16] = {
                0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3
            };
            return iconIndex + torchTypeForCharge[(raw[3] >> 1) & 0x0fu];
        }
        break;
    case kIconScrollOpen:
        if ((raw[3] >> 2) & 0x3fu) return iconIndex + 1;
        break;
    case kIconJunkWater:
    case kIconJunkJewelSymalUnequipped:
    case kIconJunkIllumuletUnequipped:
        if ((raw[3] >> 6) & 0x03u) return iconIndex + 1;
        break;
    case kIconWeaponBoltBladeStormEmpty:
    case kIconWeaponFlamittEmpty:
    case kIconWeaponStormringEmpty:
    case kIconWeaponFuryRaBladeEmpty:
    case kIconWeaponEyeOfTimeEmpty:
    case kIconWeaponStaffOfClawsEmpty:
        if ((raw[3] >> 1) & 0x0fu) return iconIndex + 1;
        break;
    default:
        break;
    }
    return iconIndex;
}

int F7017_GetIconIndex(
    const struct DungeonThings_Compat *things,
    unsigned short thing)
{
    return dm1_v1_dungeon_get_object_icon_index_pc34(things, thing, 0);
}

const unsigned char *F7018_GetThingData(
    const struct DungeonThings_Compat *things,
    unsigned short thing)
{
    return dm1_v1_dungeon_get_thing_data_pc34(things, thing);
}

int F7019_GetObjectInfoIndex(
    const struct DungeonThings_Compat *things,
    unsigned short thing)
{
    return dm1_v1_dungeon_get_object_info_index_pc34(things, thing);
}

const char *F7017_F7018_F7019_CEDT004_SourceEvidencePc34(void)
{
    return "CEDT004.C:188 F7017_GetIconIndex, CEDT004.C:203 "
           "F7018_GetThingData, and CEDT004.C:196/212 "
           "F7019_GetObjectInfoIndex are source-named PC34 wrappers over "
           "loaded raw Thing data. They delegate to the existing "
           "DUNGEON.C F0156/F0141 and OBJECT.C F0033 contracts, fail closed "
           "when raw records are absent, and do not synthesize decoded object "
           "records, graphics resources, direction state, or input events.";
}

const char *dm1_v1_dungeon_thing_data_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C:F0156_DUNGEON_GetThingData:1584-1638; "
           "G0284_apuc_ThingData[type] + index * "
           "G0235_auc_Graphic559_ThingDataByteCount[type]; "
           "DUNGEON.C:F0141_DUNGEON_GetObjectInfoIndex:1136-1165; "
           "OBJECT.C:F0032_OBJECT_GetType:121-145; "
           "OBJECT.C:F0033_OBJECT_GetIconIndex:147-213; "
           "DUNGEON.C:G0237_as_Graphic559_ObjectInfo[180].Type; "
           "DUNGEON.C:F0139_DUNGEON_IsCreatureAllowedOnMap:1052-1079; "
           "DUNGEON.C:F0145-F0148 active-group Cells/Directions overlay";
}
