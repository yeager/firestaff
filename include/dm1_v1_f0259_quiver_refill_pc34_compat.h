#ifndef DM1_V1_F0259_QUIVER_REFILL_PC34_COMPAT_H
#define DM1_V1_F0259_QUIVER_REFILL_PC34_COMPAT_H

#include "memory_champion_state_pc34_compat.h"

struct DM1F0259QuiverRefillPlanPc34 {
    int valid;
    int moved;
    int championIndex;
    int destinationSlot;
    int sourceSlot;
    unsigned short thing;
};

int DM1_V1_F0259_PlanQuiverRefillPc34Compat(
    const struct ChampionState_Compat* champion,
    int championIndex,
    int destinationSlot,
    struct DM1F0259QuiverRefillPlanPc34* outPlan);

#endif
