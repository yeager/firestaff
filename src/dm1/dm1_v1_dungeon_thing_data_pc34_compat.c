#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

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

const char *dm1_v1_dungeon_thing_data_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C:F0156_DUNGEON_GetThingData:1584-1638; "
           "G0284_apuc_ThingData[type] + index * "
           "G0235_auc_Graphic559_ThingDataByteCount[type]";
}
