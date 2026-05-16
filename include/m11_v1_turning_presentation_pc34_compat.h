#ifndef FIRESTAFF_M11_V1_TURNING_PRESENTATION_PC34_COMPAT_H
#define FIRESTAFF_M11_V1_TURNING_PRESENTATION_PC34_COMPAT_H

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Guard for source-faithful V1 presentation mode.  Callers in V2/V3 or debug
 * presentation paths must pass a different mode and receive a no-op result.
 */
#define M11_V1_TURNING_PRESENTATION_MODE_ORIGINAL 1
#define M11_V1_TURNING_PRESENTATION_MODE_OTHER    0

struct M11V1TurningChampionPosePc34Compat {
    int cell;
    int direction;
};

struct M11V1TurningPresentationResultPc34Compat {
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
};

int m11_v1_turning_is_turn_command_pc34_compat(int command);
int m11_v1_turning_target_direction_pc34_compat(int currentDirection, int command);

struct M11V1TurningPresentationResultPc34Compat
m11_v1_turning_apply_original_presentation_pc34_compat(
    int presentationMode,
    int command,
    int currentDirection,
    struct M11V1TurningChampionPosePc34Compat* poses,
    int poseCount);

struct M11V1TurningPresentationResultPc34Compat
m11_v1_turning_apply_party_original_presentation_pc34_compat(
    int presentationMode,
    int command,
    struct PartyState_Compat* party);

const char* m11_v1_turning_presentation_source_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif
