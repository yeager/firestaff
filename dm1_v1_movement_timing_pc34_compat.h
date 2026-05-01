#ifndef DM1_V1_MOVEMENT_TIMING_PC34_COMPAT_H
#define DM1_V1_MOVEMENT_TIMING_PC34_COMPAT_H

#include <stdint.h>

#include "memory_champion_state_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Dm1V1MovementTimingResultPc34Compat {
    int disabledMovementTicks;
    int projectileDisabledMovementTicks;
    unsigned long lastPartyMovementTime;
    int scentRecorded;
    int scentDelayTicks;
};

uint16_t DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat(
    uint16_t load,
    uint16_t maxLoad,
    unsigned short wounds,
    int footwearIcon);

int DM1_V1_MovementTiming_ComputePartyStepTicksPc34Compat(
    const struct PartyState_Compat* party,
    const int footwearIcons[CHAMPION_MAX_PARTY]);

struct Dm1V1MovementTimingResultPc34Compat DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat(
    const struct PartyState_Compat* party,
    int sourceMapIndex,
    int sourceMapX,
    int sourceMapY,
    unsigned long currentGameTick,
    unsigned long previousLastPartyMovementTime,
    const int footwearIcons[CHAMPION_MAX_PARTY]);

void DM1_V1_MovementTiming_DecrementCooldownsPc34Compat(
    int* disabledMovementTicks,
    int* projectileDisabledMovementTicks);

const char* DM1_V1_MovementTiming_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
