#ifndef DM1_V1_UNUSED_LAUNCHER_OBJECT_F0166_F0167_PC34_COMPAT_H
#define DM1_V1_UNUSED_LAUNCHER_OBJECT_F0166_F0167_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0166_THING_TYPE_COUNT_PC34 = 16,
    DM1_V1_F0166_THING_NONE_PC34 = 0xffffu,
    DM1_V1_F0166_THING_END_OF_LIST_PC34 = 0xfffeu,
    DM1_V1_F0166_THING_TYPE_WEAPON_PC34 = 5,
    DM1_V1_F0166_THING_TYPE_JUNK_PC34 = 10
};

typedef struct {
    uint8_t *records;
    size_t recordCount;
    size_t recordSize;
} DM1_V1_MutableThingDataTableF0166Pc34;

typedef struct {
    DM1_V1_MutableThingDataTableF0166Pc34 thingData[
        DM1_V1_F0166_THING_TYPE_COUNT_PC34];
} DM1_V1_UnusedThingContextF0166Pc34;

typedef struct {
    int valid;
    int thingType;
    int thingIndex;
    uint16_t thing;
    size_t recordSize;
    uint8_t *record;
} DM1_V1_UnusedThingResultF0166Pc34;

typedef struct {
    int valid;
    int iconIndex;
    int normalizedIconIndex;
    int thingType;
    int itemType;
} DM1_V1_LauncherObjectResultF0167Pc34;

const char *DM1_V1_F0166_F0167_SourceEvidencePc34(void);

uint16_t F0166_DUNGEON_GetUnusedThing(
    DM1_V1_UnusedThingContextF0166Pc34 *context,
    int thingType);

int DM1_V1_Dungeon_GetUnusedThingF0166Pc34Compat(
    DM1_V1_UnusedThingContextF0166Pc34 *context,
    int thingType,
    DM1_V1_UnusedThingResultF0166Pc34 *out);

uint16_t F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator(
    DM1_V1_UnusedThingContextF0166Pc34 *context,
    int iconIndex,
    DM1_V1_LauncherObjectResultF0167Pc34 *out);

#ifdef __cplusplus
}
#endif

#endif
