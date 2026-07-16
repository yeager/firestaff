#include "dm1_v1_unused_launcher_object_f0166_f0167_pc34_compat.h"

static uint16_t dm1_v1_read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static void dm1_v1_write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)(value & 0xffu);
    bytes[1] = (uint8_t)((value >> 8) & 0xffu);
}

static uint16_t dm1_v1_thing_ref(int thingType, int index)
{
    return (uint16_t)(((thingType & 15) << 10) | (index & 0x03ff));
}

static void dm1_v1_clear_unused_result(
    DM1_V1_UnusedThingResultF0166Pc34 *out,
    int thingType)
{
    if (!out) {
        return;
    }
    out->valid = 0;
    out->thingType = thingType;
    out->thingIndex = -1;
    out->thing = (uint16_t)DM1_V1_F0166_THING_NONE_PC34;
    out->recordSize = 0;
    out->record = 0;
}

static void dm1_v1_clear_launcher_result(
    DM1_V1_LauncherObjectResultF0167Pc34 *out,
    int iconIndex)
{
    if (!out) {
        return;
    }
    out->valid = 0;
    out->iconIndex = iconIndex;
    out->normalizedIconIndex = iconIndex;
    out->thingType = -1;
    out->itemType = -1;
}

static int dm1_v1_launcher_icon_to_object(
    int iconIndex,
    int *outNormalizedIconIndex,
    int *outThingType,
    int *outItemType)
{
    int normalizedIconIndex = iconIndex;
    int thingType = DM1_V1_F0166_THING_TYPE_WEAPON_PC34;
    int itemType;

    if (normalizedIconIndex >= 4 && normalizedIconIndex <= 7) {
        normalizedIconIndex = 4;
    }

    switch (normalizedIconIndex) {
    case 4:
        itemType = 2;
        break;
    case 32:
        itemType = 8;
        break;
    case 51:
        itemType = 27;
        break;
    case 52:
        itemType = 28;
        break;
    case 54:
        itemType = 30;
        break;
    case 55:
        itemType = 31;
        break;
    case 56:
        itemType = 32;
        break;
    case 128:
        thingType = DM1_V1_F0166_THING_TYPE_JUNK_PC34;
        itemType = 25;
        break;
    default:
        return 0;
    }

    if (outNormalizedIconIndex) {
        *outNormalizedIconIndex = normalizedIconIndex;
    }
    if (outThingType) {
        *outThingType = thingType;
    }
    if (outItemType) {
        *outItemType = itemType;
    }
    return 1;
}

const char *DM1_V1_F0166_F0167_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:2079 F0166_DUNGEON_GetUnusedThing takes a thing type, "
        "uses G0235 thing-data byte counts, scans that type's thing records, "
        "and returns the first record whose Generic.Next word is "
        "C0xFFFF_THING_NONE; DUNGEON.C:2142-2200 "
        "F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator maps "
        "projectile-launcher/object-generator icon indices to weapon or junk "
        "object records, then consumes F0166 to allocate the unused thing.";
}

int DM1_V1_Dungeon_GetUnusedThingF0166Pc34Compat(
    DM1_V1_UnusedThingContextF0166Pc34 *context,
    int thingType,
    DM1_V1_UnusedThingResultF0166Pc34 *out)
{
    DM1_V1_MutableThingDataTableF0166Pc34 *table;
    size_t i;

    dm1_v1_clear_unused_result(out, thingType);
    if (!context || thingType < 0 ||
        thingType >= DM1_V1_F0166_THING_TYPE_COUNT_PC34) {
        return 0;
    }
    table = &context->thingData[thingType];
    if (!table->records || table->recordSize < 2) {
        return 0;
    }
    for (i = 0; i < table->recordCount; ++i) {
        uint8_t *record = table->records + (i * table->recordSize);
        if (dm1_v1_read_le16(record) == DM1_V1_F0166_THING_NONE_PC34) {
            if (out) {
                out->valid = 1;
                out->thingType = thingType;
                out->thingIndex = (int)i;
                out->thing = dm1_v1_thing_ref(thingType, (int)i);
                out->recordSize = table->recordSize;
                out->record = record;
            }
            return 1;
        }
    }
    return 0;
}

uint16_t F0166_DUNGEON_GetUnusedThing(
    DM1_V1_UnusedThingContextF0166Pc34 *context,
    int thingType)
{
    DM1_V1_UnusedThingResultF0166Pc34 result;

    if (!DM1_V1_Dungeon_GetUnusedThingF0166Pc34Compat(
            context, thingType, &result)) {
        return (uint16_t)DM1_V1_F0166_THING_NONE_PC34;
    }
    return result.thing;
}

uint16_t F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(
    DM1_V1_UnusedThingContextF0166Pc34 *context,
    int iconIndex,
    DM1_V1_LauncherObjectResultF0167Pc34 *out)
{
    DM1_V1_UnusedThingResultF0166Pc34 unused;
    int normalizedIconIndex;
    int thingType;
    int itemType;

    dm1_v1_clear_launcher_result(out, iconIndex);
    if (!dm1_v1_launcher_icon_to_object(
            iconIndex, &normalizedIconIndex, &thingType, &itemType)) {
        return (uint16_t)DM1_V1_F0166_THING_NONE_PC34;
    }
    if (out) {
        out->normalizedIconIndex = normalizedIconIndex;
        out->thingType = thingType;
        out->itemType = itemType;
    }
    if (!DM1_V1_Dungeon_GetUnusedThingF0166Pc34Compat(
            context, thingType, &unused)) {
        return (uint16_t)DM1_V1_F0166_THING_NONE_PC34;
    }

    dm1_v1_write_le16(unused.record, DM1_V1_F0166_THING_END_OF_LIST_PC34);
    if (unused.recordSize >= 4) {
        dm1_v1_write_le16(unused.record + 2, (uint16_t)itemType);
    }
    if (out) {
        out->valid = 1;
    }
    return unused.thing;
}
