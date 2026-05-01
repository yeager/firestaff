#include "dm1_v1_movement_timing_pc34_compat.h"

#include "memory_champion_lifecycle_pc34_compat.h"

/* Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - COMMAND.C:2095-2100 and COMMAND.C:2118-2127 keep movement commands queued while
 *   G0310/G0311 gates are active, then dequeue exactly one command before dispatch.
 * - CLIKMENU.C:256-269 maps C003..C006 to forward/right/back/left relative
 *   step vectors; turns never enter this movement cadence path.
 * - CLIKMENU.C:330-346 sets AL1115_ui_Ticks=1, raises it to the maximum
 *   F0310_CHAMPION_GetMovementTicks among living champions after a successful
 *   party step, stores G0310_i_DisabledMovementTicks, and clears G0311.
 * - CHAMPION.C:1180-1215 defines the load/max-load/wounded-feet/Boots of
 *   Speed tick formula, including BUG0_72 load==maxLoad overloaded cadence.
 * - MOVESENS.C:752-775 updates G0362_l_LastPartyMovementTime and scent timing
 *   only when the party really changes square and has at least one champion.
 */

uint16_t DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat(
    uint16_t load,
    uint16_t maxLoad,
    unsigned short wounds,
    int footwearIcon)
{
    return F0841_LIFECYCLE_ComputeMoveTicks_Compat(load, maxLoad, wounds, footwearIcon);
}

int DM1_V1_MovementTiming_ComputePartyStepTicksPc34Compat(
    const struct PartyState_Compat* party,
    const int footwearIcons[CHAMPION_MAX_PARTY])
{
    int ticks = 1;
    int i;

    if (!party) {
        return ticks;
    }

    for (i = 0; i < party->championCount && i < CHAMPION_MAX_PARTY; ++i) {
        const struct ChampionState_Compat* champion = &party->champions[i];
        int championTicks;
        if (champion->hp.current == 0) {
            continue;
        }
        championTicks = (int)DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat(
            champion->load,
            champion->maxLoad,
            champion->wounds,
            footwearIcons ? footwearIcons[i] : 0);
        if (championTicks > ticks) {
            ticks = championTicks;
        }
    }
    return ticks;
}

struct Dm1V1MovementTimingResultPc34Compat DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat(
    const struct PartyState_Compat* party,
    int sourceMapIndex,
    int sourceMapX,
    int sourceMapY,
    unsigned long currentGameTick,
    unsigned long previousLastPartyMovementTime,
    const int footwearIcons[CHAMPION_MAX_PARTY])
{
    struct Dm1V1MovementTimingResultPc34Compat result;
    result.disabledMovementTicks = DM1_V1_MovementTiming_ComputePartyStepTicksPc34Compat(party, footwearIcons);
    result.projectileDisabledMovementTicks = 0;
    result.lastPartyMovementTime = previousLastPartyMovementTime;
    result.scentRecorded = 0;
    result.scentDelayTicks = 0;

    if (party && party->championCount > 0 &&
        (party->mapIndex != sourceMapIndex || party->mapX != sourceMapX || party->mapY != sourceMapY)) {
        result.scentRecorded = 1;
        result.scentDelayTicks = (int)(currentGameTick - previousLastPartyMovementTime);
        result.lastPartyMovementTime = currentGameTick;
    }

    return result;
}

const char* DM1_V1_MovementTiming_SourceEvidencePc34Compat(void)
{
    return "COMMAND.C:2095-2100,2118-2127; CLIKMENU.C:256-269,330-346; CHAMPION.C:1180-1215; MOVESENS.C:752-775";
}
