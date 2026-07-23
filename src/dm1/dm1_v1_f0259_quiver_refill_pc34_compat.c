#include "dm1_v1_f0259_quiver_refill_pc34_compat.h"

#include <string.h>

#include "memory_dungeon_dat_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

static unsigned short read_u16_le_f0259(const unsigned char* raw)
{
    return (unsigned short)((unsigned short)raw[0] |
                            ((unsigned short)raw[1] << 8));
}

static int weapon_matches_dungeon_raw_f0259(
    const struct DungeonThings_Compat* things,
    unsigned short thing)
{
    const struct DungeonWeapon_Compat* weapon;
    const unsigned char* raw;
    unsigned short bits;
    int index;

    if (!things || !things->loaded || !things->weapons ||
        THING_GET_TYPE(thing) != THING_TYPE_WEAPON) {
        return 0;
    }
    index = THING_GET_INDEX(thing);
    if (index < 0 || index >= things->weaponCount ||
        index >= things->thingCounts[THING_TYPE_WEAPON]) {
        return 0;
    }
    raw = dm1_v1_dungeon_get_thing_data_pc34(things, thing);
    if (!raw) return 0;
    weapon = &things->weapons[index];
    bits = read_u16_le_f0259(raw + 2);
    return read_u16_le_f0259(raw) == weapon->next &&
           (bits & 0x007fu) == weapon->type &&
           ((bits >> 7) & 1u) == weapon->doNotDiscard &&
           ((bits >> 8) & 1u) == weapon->cursed &&
           ((bits >> 9) & 1u) == weapon->poisoned &&
           ((bits >> 10) & 0x0fu) == weapon->chargeCount &&
           ((bits >> 14) & 1u) == weapon->broken &&
           ((bits >> 15) & 1u) == weapon->lit;
}

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

int DM1_V1_F0259_ApplyQuiverRefillFromDungeonPc34Compat(
    struct ChampionState_Compat* champion,
    int championIndex,
    int destinationSlot,
    const struct DungeonThings_Compat* things,
    struct DM1F0259QuiverRefillPlanPc34* outPlan)
{
    struct DM1F0259QuiverRefillPlanPc34 plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    if (!DM1_V1_F0259_PlanQuiverRefillPc34Compat(
            champion, championIndex, destinationSlot, &plan)) {
        return 0;
    }
    *outPlan = plan;
    if (!plan.moved) return 1;
    if (!weapon_matches_dungeon_raw_f0259(things, plan.thing)) {
        /* Fail closed: preserve both inventory slots when the decoded item
         * cannot be tied to its original 4-byte C05 record. */
        outPlan->moved = 0;
        outPlan->sourceSlot = -1;
        outPlan->thing = THING_NONE;
        return 0;
    }
    champion->inventory[plan.destinationSlot] = plan.thing;
    champion->inventory[plan.sourceSlot] = THING_NONE;
    return 1;
}
