#ifndef DM1_V1_MOVEMENT_COMMAND_CORE_PC34_COMPAT_H
#define DM1_V1_MOVEMENT_COMMAND_CORE_PC34_COMPAT_H

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_movement_timing_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_sensor_execution_pc34_compat.h"
#include "m11_v1_turning_presentation_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 party movement command seam.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC locks the queue,
 *   respects movement cooldown gates, dequeues one command, then dispatches
 *   C001/C002 turns to F0365 and C003..C006 steps to F0366.
 * - CLIKMENU.C:142-179 F0365 removes/re-adds the party on the same square
 *   around F0284_CHAMPION_SetPartyDirection and releases input wait.
 * - CLIKMENU.C:180-347 F0366 resolves relative step destination, takes
 *   stairs immediately on turn/backward/current-square and target-square
 *   stairs consequences, rejects walls/closed doors/closed real
 *   fake-walls/groups, calls F0267_MOVE for source/destination/sensor
 *   mutation, applies movement cooldown only after accepted non-stairs party
 *   movement, and releases input wait after accepted command effects.
 * - MOVESENS.C:752-783 timestamps true party square changes; MOVESENS.C:1553-1794
 *   walks party leave/enter sensors; GAMELOOP.C:90 and DRAWVIEW.C:709-724 redraw
 *   and present the viewport from the mutated party state on the next loop.
 */
struct Dm1V1MovementCommandCoreResultPc34Compat {
    struct Dm1V1InputQueueProcessResultPc34Compat queue;
    struct MovementResult_Compat movement;
    struct Dm1V1MovementTimingResultPc34Compat timing;
    struct M11V1TurningPresentationResultPc34Compat turning;
    struct SensorEffectList_Compat leaveEffects;
    struct SensorEffectList_Compat enterEffects;
    int commandHandled;
    int turnApplied;
    int stepApplied;
    int movementBlocked;
    int blockedByGroup;
    int inputDiscardRequested;
    int stopWaitingForPlayerInput;
    int viewportRedrawRequested;
    int sourceMapIndex;
    int sourceMapX;
    int sourceMapY;
    int sourceDirection;
    int stairTransitionApplied;
};

int DM1_V1_MovementCommandCore_ProcessOnePc34Compat(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    struct PartyState_Compat* party,
    int disabledMovementTicks,
    int projectileDisabledMovementTicks,
    int lastProjectileDisabledMovementDirection,
    unsigned long currentGameTick,
    unsigned long previousLastPartyMovementTime,
    const int footwearIcons[CHAMPION_MAX_PARTY],
    struct Dm1V1MovementCommandCoreResultPc34Compat* outResult);

const char* DM1_V1_MovementCommandCore_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
