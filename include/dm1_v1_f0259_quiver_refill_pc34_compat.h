#ifndef DM1_V1_F0259_QUIVER_REFILL_PC34_COMPAT_H
#define DM1_V1_F0259_QUIVER_REFILL_PC34_COMPAT_H

#include "memory_champion_state_pc34_compat.h"

/* ReDMCSB TIMELINE.C F0259 handles C11_EVENT_ENABLE_CHAMPION_ACTION's
 * delayed ready-hand refill. aux0 is the champion index and aux1 is the
 * destination inventory slot when carried by TIMELINE_EVENT_MOVE_TIMER. */
#define DM1_F0259_MOVE_TIMER_AUX4_PC34 0xF0259

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
