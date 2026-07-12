#include "dm1_v1_f0245_corridor_pc34_compat.h"

#include <string.h>

int DM1_V1_F0245_PlanCorridorEventPc34Compat(
    const DM1F0245CorridorEntryPc34 *entries, int entryCount,
    const struct DungeonTextString_Compat *texts, int textCount,
    const struct DungeonSensor_Compat *sensors, int sensorCount,
    const DM1F0245CorridorContextPc34 *context,
    DM1F0245CorridorPlanPc34 *outPlan)
{
    int i;
    if (!entries || !texts || !sensors || !context || !outPlan ||
        entryCount < 0 || textCount < 0 || sensorCount < 0) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    for (i = 0; i < entryCount; ++i) {
        const DM1F0245CorridorEntryPc34 *entry = &entries[i];
        if (entry->entryType == DM1_F0245_ENTRY_TEXT_PC34 &&
            entry->entryIndex >= 0 && entry->entryIndex < textCount &&
            outPlan->textMutationCount < 16) {
            int visible = texts[entry->entryIndex].visible ? 1 : 0;
            if (context->eventEffect == 2) visible = !visible;
            else if (context->eventEffect == 0) visible = 1;
            else if (context->eventEffect == 1) visible = 0;
            else continue;
            outPlan->textIndex[outPlan->textMutationCount] = entry->entryIndex;
            outPlan->textVisibleAfter[outPlan->textMutationCount++] = visible;
        } else if (entry->entryType == DM1_F0245_ENTRY_SENSOR_PC34 &&
                   entry->entryIndex >= 0 && entry->entryIndex < sensorCount &&
                   sensors[entry->entryIndex].sensorType == 6 &&
                   outPlan->generatorCount < 16) {
            const struct DungeonSensor_Compat *sensor = &sensors[entry->entryIndex];
            int slot = outPlan->generatorCount++;
            int count = (int)(sensor->value & 7u);
            if (sensor->value & 8u) count = context->randomCountValue;
            else --count;
            if (count < 0) count = 0;
            outPlan->generatorSensorIndex[slot] = entry->entryIndex;
            outPlan->generatorCreatureType[slot] = sensor->sensorData;
            outPlan->generatorCreatureCount[slot] = count;
            outPlan->generatorHealthMultiplier[slot] =
                (sensor->localMultiple & 0x0fu) ?
                (sensor->localMultiple & 0x0fu) : context->mapDifficulty;
            outPlan->generatorDisabled[slot] = 1;
            outPlan->generatorReenableTicks[slot] =
                ((sensor->localMultiple >> 4) & 0xffu) > 127 ?
                ((((sensor->localMultiple >> 4) & 0xffu) - 126) << 6) :
                ((sensor->localMultiple >> 4) & 0xffu);
        }
    }
    return 1;
}
