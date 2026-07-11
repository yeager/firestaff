#include "m11_v1_turning_presentation_pc34_compat.h"
#include "memory_mov05_f0284_cell_rotation_pc34_compat.h"

#include <string.h>

static int dm1_v1_turning_normalize_direction(int value)
{
    return value & 3;
}

int DM1_V1_Turning_IsTurnCommandPc34Compat(int command)
{
    /* COMMAND.C:2150-2152 dispatches only C001/C002 into F0365. */
    return command == DM1_V1_COMMAND_TURN_LEFT || command == DM1_V1_COMMAND_TURN_RIGHT;
}

int DM1_V1_Turning_TargetDirectionPc34Compat(int currentDirection, int command)
{
    /* CLIKMENU.C:171-173 calls F0284_CHAMPION_SetPartyDirection with
     * M021_NORMALIZE(G0308_i_PartyDirection + (right ? 1 : 3)).
     */
    if (command == DM1_V1_COMMAND_TURN_RIGHT) {
        return dm1_v1_turning_normalize_direction(currentDirection + 1);
    }
    if (command == DM1_V1_COMMAND_TURN_LEFT) {
        return dm1_v1_turning_normalize_direction(currentDirection + 3);
    }
    return dm1_v1_turning_normalize_direction(currentDirection);
}

DM1_V1_TurningPresentationResultPc34Compat
DM1_V1_Turning_ApplyOriginalPresentationPc34Compat(
    int presentationMode,
    int command,
    int currentDirection,
    DM1_V1_TurningChampionPosePc34Compat* poses,
    int poseCount)
{
    DM1_V1_TurningPresentationResultPc34Compat result;
    int i;

    memset(&result, 0, sizeof(result));
    result.command = command;
    result.oldDirection = dm1_v1_turning_normalize_direction(currentDirection);
    result.newDirection = result.oldDirection;
    result.renderDirection = result.oldDirection;

    if (presentationMode != DM1_V1_TURNING_PRESENTATION_MODE_ORIGINAL_PC34 ||
        !DM1_V1_Turning_IsTurnCommandPc34Compat(command)) {
        return result;
    }

    result.newDirection = DM1_V1_Turning_TargetDirectionPc34Compat(result.oldDirection, command);
    result.delta = result.newDirection - result.oldDirection;
    if (result.delta < 0) {
        result.delta += 4;
    }

    /* Source-locked facts:
     * - CLIKMENU.C:156 sets G0321_B_StopWaitingForPlayerInput immediately.
     * - CLIKMENU.C:158-162 highlights the left/right arrow box before the
     *   direction mutation.
     * - CLIKMENU.C:167-173 has no forward-square legality/wall check; on a
     *   non-stairs square it processes current-square sensors around F0284.
     * - CHAMPION.C:118-130 F0284 rotates every champion Cell and Direction by
     *   the same delta, then stores G0308_i_PartyDirection.
     * - GAMELOOP.C:90 and DRAWVIEW.C:721-722 draw/present the viewport from
     *   the already-mutated party direction; no source loop emits partial
     *   yaw/intermediate frames for a 90-degree turn.
     */
    result.applied = 1;
    result.quarterTurnSteps = 1;
    result.animationFrames = 1;
    result.intermediateFrames = 0;
    result.renderDirection = result.newDirection;
    result.waitsForViewportVBlank = 1;
    result.redrawOnNextGameLoop = 1;
    result.stopWaitingForPlayerInput = 1;
    result.wallBlockCheck = 0;
    result.highlightLeft = (command == DM1_V1_COMMAND_TURN_LEFT);
    result.highlightRight = (command == DM1_V1_COMMAND_TURN_RIGHT);

    if (poses) {
        for (i = 0; i < poseCount; ++i) {
            poses[i].cell = dm1_v1_turning_normalize_direction(poses[i].cell + result.delta);
            poses[i].direction = dm1_v1_turning_normalize_direction(poses[i].direction + result.delta);
        }
    }

    return result;
}

DM1_V1_TurningPresentationResultPc34Compat
DM1_V1_Turning_ApplyPartyOriginalPresentationPc34Compat(
    int presentationMode,
    int command,
    struct PartyState_Compat* party)
{
    DM1_V1_TurningPresentationResultPc34Compat result;
    int oldDirection;

    if (!party) {
        memset(&result, 0, sizeof(result));
        result.command = command;
        return result;
    }

    oldDirection = party->direction;
    result = DM1_V1_Turning_ApplyOriginalPresentationPc34Compat(
        presentationMode, command, oldDirection, 0, 0);
    if (!result.applied) {
        return result;
    }

    (void)F0284_CHAMPION_SetPartyDirection_Compat(party, result.newDirection);
    return result;
}

const char* DM1_V1_Turning_PresentationSourceEvidencePc34Compat(void)
{
    return "COMMAND.C:396-405,2045-2156; CLIKMENU.C:142-174; CHAMPION.C:117-130; GAMELOOP.C:90,215-219; DRAWVIEW.C:709-724";
}
