#ifndef FIRESTAFF_DM1_V1_TURNING_PRESENTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_TURNING_PRESENTATION_PC34_COMPAT_H

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Guard for source-faithful V1 presentation mode.  Callers in V2/V3 or debug
 * presentation paths must pass a different mode and receive a no-op result.
 */
#define DM1_V1_TURNING_PRESENTATION_MODE_ORIGINAL_PC34 1
#define DM1_V1_TURNING_PRESENTATION_MODE_OTHER_PC34    0

typedef struct DM1_V1_TurningChampionPosePc34Compat {
    int cell;
    int direction;
} DM1_V1_TurningChampionPosePc34Compat;

typedef struct DM1_V1_TurningPresentationResultPc34Compat {
    int applied;
    int command;
    int oldDirection;
    int newDirection;
    int delta;
    int quarterTurnSteps;
    int animationFrames;
    int intermediateFrames;
    int renderDirection;
    int waitsForViewportVBlank;
    int redrawOnNextGameLoop;
    int stopWaitingForPlayerInput;
    int wallBlockCheck;
    int highlightLeft;
    int highlightRight;
} DM1_V1_TurningPresentationResultPc34Compat;

int DM1_V1_Turning_IsTurnCommandPc34Compat(int command);
int DM1_V1_Turning_TargetDirectionPc34Compat(int currentDirection,
                                             int command);

DM1_V1_TurningPresentationResultPc34Compat
DM1_V1_Turning_ApplyOriginalPresentationPc34Compat(
    int presentationMode,
    int command,
    int currentDirection,
    DM1_V1_TurningChampionPosePc34Compat* poses,
    int poseCount);

DM1_V1_TurningPresentationResultPc34Compat
DM1_V1_Turning_ApplyPartyOriginalPresentationPc34Compat(
    int presentationMode,
    int command,
    struct PartyState_Compat* party);

const char* DM1_V1_Turning_PresentationSourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_TURNING_PRESENTATION_PC34_COMPAT_H */
