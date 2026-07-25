#include "dm1_v1_f0245_corridor_pc34_compat.h"
#include <assert.h>
#include <string.h>
int main(void) {
    DM1F0245CorridorEntryPc34 entries[2] = {{2,0},{3,0}};
    (void)entries;
    struct DungeonTextString_Compat texts[1];
    struct DungeonSensor_Compat sensors[1];
    DM1F0245CorridorContextPc34 context;
    DM1F0245CorridorPlanPc34 plan;
    (void)plan;
    memset(texts,0,sizeof(texts)); memset(sensors,0,sizeof(sensors)); memset(&context,0,sizeof(context));
    texts[0].visible=0; sensors[0].sensorType=6; sensors[0].sensorData=12; sensors[0].value=3; sensors[0].localMultiple=(2u | (5u<<4));
    context.eventEffect=0; context.mapDifficulty=7;
    assert(DM1_V1_F0245_PlanCorridorEventPc34Compat(entries,2,texts,1,sensors,1,&context,&plan));
    assert(plan.textMutationCount==1 && plan.textVisibleAfter[0]==1);
    assert(plan.generatorCount==1 && plan.generatorCreatureType[0]==12);
    assert(plan.generatorCreatureCount[0]==2 && plan.generatorHealthMultiplier[0]==2);
    assert(plan.generatorDisabled[0] && plan.generatorReenableTicks[0]==5);
    return 0;
}
