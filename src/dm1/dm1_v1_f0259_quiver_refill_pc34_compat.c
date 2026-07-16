#include "dm1_v1_f0259_quiver_refill_pc34_compat.h"

#include <string.h>

#include "memory_dungeon_dat_pc34_compat.h"

int DM1_V1_F0259_PlanQuiverRefillPc34Compat(
    const struct ChampionState_Compat* champion,
    int championIndex,
    int destinationSlot,
    struct DM1F0259QuiverRefillPlanPc34* outPlan)
{
    static const int sourceSlots[] = {
        /* ReDMCSB DEFS.H C12, C07, C08, C09; ChampionState uses the
         * M11 canonical mapping for those four original quiver slots. */
        CHAMPION_SLOT_QUIVER_1,
        CHAMPION_SLOT_QUIVER_3,
        CHAMPION_SLOT_QUIVER_2,
        CHAMPION_SLOT_QUIVER_4
    };
    int i;

    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->championIndex = championIndex;
    outPlan->destinationSlot = destinationSlot;
    outPlan->sourceSlot = -1;
    outPlan->thing = THING_NONE;
    if (!champion || championIndex < 0 ||
        destinationSlot < 0 || destinationSlot >= CHAMPION_SLOT_COUNT) {
        return 0;
    }
    outPlan->valid = 1;
    if (champion->inventory[destinationSlot] != THING_NONE) return 1;

    /* TIMELINE.C F0258/F0259: only weapons move, and the source priority
     * is C12 first then C07 through C09. */
    for (i = 0; i < (int)(sizeof(sourceSlots) / sizeof(sourceSlots[0])); ++i) {
        int sourceSlot = sourceSlots[i];
        unsigned short thing = champion->inventory[sourceSlot];
        if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON) continue;
        outPlan->moved = 1;
        outPlan->sourceSlot = sourceSlot;
        outPlan->thing = thing;
        return 1;
    }
    return 1;
}
