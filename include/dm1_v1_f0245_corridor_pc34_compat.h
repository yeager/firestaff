#ifndef FIRESTAFF_DM1_V1_F0245_CORRIDOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0245_CORRIDOR_PC34_COMPAT_H

#include "memory_dungeon_dat_pc34_compat.h"

enum { DM1_F0245_ENTRY_TEXT_PC34 = 2, DM1_F0245_ENTRY_SENSOR_PC34 = 3 };

typedef struct {
    int entryType;
    int entryIndex;
} DM1F0245CorridorEntryPc34;

typedef struct {
    int eventEffect;
    int mapDifficulty;
    int randomCountValue;
} DM1F0245CorridorContextPc34;

typedef struct {
    int textMutationCount;
    int textIndex[16];
    int textVisibleAfter[16];
    int generatorCount;
    int generatorSensorIndex[16];
    int generatorCreatureType[16];
    int generatorCreatureCount[16];
    int generatorHealthMultiplier[16];
    int generatorDisabled[16];
    int generatorReenableTicks[16];
} DM1F0245CorridorPlanPc34;

/* ReDMCSB TIMELINE.C F0245: walk a corridor square in linked-list order.
 * This adapter plans TEXTSTRING visibility and C006 group-generation state;
 * the caller owns world writes, RNG, group allocation, sound, and timeline. */
int DM1_V1_F0245_PlanCorridorEventPc34Compat(
    const DM1F0245CorridorEntryPc34 *entries, int entryCount,
    const struct DungeonTextString_Compat *texts, int textCount,
    const struct DungeonSensor_Compat *sensors, int sensorCount,
    const DM1F0245CorridorContextPc34 *context,
    DM1F0245CorridorPlanPc34 *outPlan);

#endif
