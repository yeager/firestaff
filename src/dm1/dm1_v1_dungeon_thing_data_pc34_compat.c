#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

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

const char *dm1_v1_dungeon_thing_data_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C:F0156_DUNGEON_GetThingData:1584-1638; "
           "G0284_apuc_ThingData[type] + index * "
           "G0235_auc_Graphic559_ThingDataByteCount[type]; "
           "DUNGEON.C:F0141_DUNGEON_GetObjectInfoIndex:1136-1165; "
           "OBJECT.C:F0032_OBJECT_GetType:121-145; "
           "OBJECT.C:F0033_OBJECT_GetIconIndex:147-213; "
           "DUNGEON.C:G0237_as_Graphic559_ObjectInfo[180].Type";
}
