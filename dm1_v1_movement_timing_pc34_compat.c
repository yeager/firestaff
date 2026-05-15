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
 * - GAMELOOP.C:150-155 decrements G0310_i_DisabledMovementTicks and
 *   G0311_i_ProjectileDisabledMovementTicks independently once per game loop.
 * - GAMELOOP.C:47-50 sets the per-input wait window; PC-34/I34E uses 12
 *   vertical blanks.
 * - GAMELOOP.C:164-219 drains queued keyboard input, processes one queue slot,
 *   and repeats until input stops waiting and game time is ticking.
 * - IO.C:772-778 and 1015-1019 advance the input wait VBlank counter and set
 *   G0321_B_StopWaitingForPlayerInput; they do not decrement movement cooldowns.
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

void DM1_V1_MovementTiming_DecrementCooldownsPc34Compat(
    int* disabledMovementTicks,
    int* projectileDisabledMovementTicks)
{
    if (disabledMovementTicks && *disabledMovementTicks > 0) {
        --*disabledMovementTicks;
    }
    if (projectileDisabledMovementTicks && *projectileDisabledMovementTicks > 0) {
        --*projectileDisabledMovementTicks;
    }
}

int DM1_V1_MovementTiming_InputWaitMaxVBlanksPc34Compat(void)
{
    return DM1_V1_INPUT_WAIT_MAX_VBLANKS_PC34_COMPAT;
}

int DM1_V1_MovementTiming_InputWaitStopsAfterVBlanksPc34Compat(int accumulatedVBlanks)
{
    return accumulatedVBlanks >= DM1_V1_INPUT_WAIT_MAX_VBLANKS_PC34_COMPAT;
}

int DM1_V1_MovementTiming_VBlankWaitDecrementsMovementCooldownPc34Compat(void)
{
    return 0;
}

const char* DM1_V1_MovementTiming_SourceEvidencePc34Compat(void)
{
    return "COMMAND.C:2095-2100,2118-2127; CLIKMENU.C:142-174,237-269,317-346; CHAMPION.C:1180-1215,2065-2191; MOVESENS.C:752-775; GAMELOOP.C:47-50,150-155,164-219; IO.C:772-778,1015-1019";
}
