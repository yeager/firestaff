#include "dm1_v2_phase5_runtime_bridge_pc34.h"
#include "dm1_v2_hud_overlay_pc34.h"
#include <string.h>

/* DM1 V2 Phase 5 source-tick runtime bridge.
 *
 * Source-lock anchors:
 * - ReDMCSB COMMAND.C:2096-2106 rejects source movement commands while
 *   G0310/G0311 movement cooldown/interlock state is active.
 * - ReDMCSB COMMAND.C:2150-2156 dispatches accepted source movement ids only
 *   to F0365_COMMAND_ProcessTypes1To2_TurnParty or
 *   F0366_COMMAND_ProcessTypes3To6_MoveParty.
 * - ReDMCSB CLIKMENU.C:278-329 owns wall/door/fakewall/group collision and
 *   calls F0267_MOVE_GetMoveResult_CPSCE only after the step is accepted.
 * - ReDMCSB CLIKMENU.C:330-346 publishes G0310/G0311 source cooldown after
 *   F0267 returns; CHAMPION.C:1180-1215 computes the per-champion ticks.
 * - ReDMCSB MOVESENS.C:752-818 owns scent timing, source/destination sensor
 *   dispatch, and destination group deletion for party movement.
 * - ReDMCSB GAMELOOP.C:69-155 owns timeline, redraw, game-time, and cooldown
 *   decrement cadence; GAMELOOP.C:215-219 owns command-loop wait cadence.
 * - ReDMCSB DUNVIEW.C:8318-8612 and DRAWVIEW.C:709-722 own viewport redraw
 *   and present cadence from source logical party state.
 *
 * This bridge intentionally receives only const V1 source state plus a V2
 * camera. It can start interpolation from a source-accepted movement/turn tick,
 * but it cannot mutate source cooldowns, collision, sensors, creature timing,
 * or redraw cadence owners. */

static void dm1_v2_phase5_player_from_party(
    const struct PartyState_Compat* party,
    DM1_V2_PlayerPos* outPlayer)
{
    if (!outPlayer) return;
    memset(outPlayer, 0, sizeof(*outPlayer));
    if (!party) return;
    outPlayer->xPixel = (int16_t)(party->mapX * DM1_V2_SUBPIXEL_SCALE);
    outPlayer->yPixel = (int16_t)(party->mapY * DM1_V2_SUBPIXEL_SCALE);
    outPlayer->facingDir = (int16_t)party->direction;
    outPlayer->prevX = outPlayer->xPixel;
    outPlayer->prevY = outPlayer->yPixel;
}

static void dm1_v2_phase5_fill_result(
    const struct Dm1V1MovementPipelinePc34Compat* sourcePipeline,
    const struct Dm1V1MovementPipelineResultPc34Compat* sourceTick,
    DM1_V2_Phase5RuntimeBridgeResultPc34* outResult)
{
    if (!outResult) return;
    memset(outResult, 0, sizeof(*outResult));
    outResult->sourceEvidence = dm1_v2_phase5_runtime_bridge_source_evidence_pc34();
    outResult->redrawCadencePreserved = 1;
    outResult->sourceMutationForbidden = 1;
    if (sourcePipeline) {
        outResult->sourceCooldownTicks = sourcePipeline->disabledMovementTicks;
        outResult->sourceProjectileCooldownTicks = sourcePipeline->projectileDisabledMovementTicks;
        outResult->sourceGameTick = sourcePipeline->gameTick;
        outResult->sourceLastPartyMovementTime = sourcePipeline->lastPartyMovementTime;
    }
    if (sourceTick) {
        outResult->sourceTickAccepted = sourceTick->provenance.commandAccepted;
        outResult->sourceStepAccepted = sourceTick->core.stepApplied ||
            sourceTick->core.stairTransitionApplied || sourceTick->postMove.transitioned;
        outResult->sourceTurnAccepted = sourceTick->core.turnApplied;
        outResult->sourceMovementBlocked = sourceTick->core.movementBlocked;
        outResult->viewportRedrawRequested = sourceTick->viewportDirty;
    }
}

int dm1_v2_phase5_runtime_bridge_start_camera_from_v1_tick_pc34(
    const struct Dm1V1MovementPipelinePc34Compat* sourcePipeline,
    const struct Dm1V1MovementPipelineResultPc34Compat* sourceTick,
    const struct PartyState_Compat* acceptedParty,
    DM1_V2_CameraController* camera,
    int32_t cameraDurationMs,
    DM1_V2_Phase5RuntimeBridgeResultPc34* outResult)
{
    return dm1_v2_phase5_runtime_bridge_start_camera_from_v1_tick_ex_pc34(
        sourcePipeline,
        sourceTick,
        acceptedParty,
        camera,
        cameraDurationMs,
        0,
        outResult);
}

int dm1_v2_phase5_runtime_bridge_start_camera_from_v1_tick_ex_pc34(
    const struct Dm1V1MovementPipelinePc34Compat* sourcePipeline,
    const struct Dm1V1MovementPipelineResultPc34Compat* sourceTick,
    const struct PartyState_Compat* acceptedParty,
    DM1_V2_CameraController* camera,
    int32_t cameraDurationMs,
    int smoothTurnPanEnabled,
    DM1_V2_Phase5RuntimeBridgeResultPc34* outResult)
{
    DM1_V2_PlayerPos sourcePlayer;
    int accepted;

    dm1_v2_phase5_fill_result(sourcePipeline, sourceTick, outResult);
    if (!sourceTick || !acceptedParty || !camera) return 0;
    if (sourceTick->core.movementBlocked) return 0;

    accepted = sourceTick->provenance.commandAccepted &&
        (sourceTick->anyMovementOccurred || sourceTick->anyTurnOccurred ||
         sourceTick->core.stepApplied || sourceTick->core.turnApplied ||
         sourceTick->core.stairTransitionApplied);
    if (!accepted) return 0;

    if (sourceTick->anyMovementOccurred || sourceTick->core.stepApplied ||
        sourceTick->core.stairTransitionApplied || sourceTick->postMove.transitioned) {
        dm1_v2_phase5_player_from_party(acceptedParty, &sourcePlayer);
        dm1_v2_camera_begin_move(camera, &sourcePlayer, cameraDurationMs);
        if (outResult) {
            outResult->cameraStarted = dm1_v2_camera_is_active(camera);
            outResult->cameraMoveStarted = outResult->cameraStarted;
        }
        return outResult ? outResult->cameraStarted : dm1_v2_camera_is_active(camera);
    }

    if (sourceTick->anyTurnOccurred || sourceTick->core.turnApplied) {
        if (smoothTurnPanEnabled) {
            dm1_v2_camera_begin_turn_pan(camera,
                                         (int16_t)sourceTick->core.sourceDirection,
                                         (int16_t)acceptedParty->direction,
                                         cameraDurationMs);
        } else {
            dm1_v2_camera_begin_turn(camera,
                                     (int16_t)sourceTick->core.sourceDirection,
                                     (int16_t)acceptedParty->direction,
                                     cameraDurationMs);
        }
        if (outResult) {
            outResult->cameraStarted = dm1_v2_camera_is_active(camera);
            outResult->cameraTurnStarted = outResult->cameraStarted;
        }
        /* pass601a: HUD turn-complete signal so renderer may defer overlays
         * until the interpolated turn stabilises on screen.
         * Source-lock: ReDMCSB GAMELOOP.C:90 viewport redraw cadence. */
        if (outResult && outResult->cameraStarted) {
            v22_hud_notify_turn_complete();
        }
        return outResult ? outResult->cameraStarted : dm1_v2_camera_is_active(camera);
    }

    return 0;
}

const char* dm1_v2_phase5_runtime_bridge_source_evidence_pc34(void)
{
    return "ReDMCSB WIP20210206 Toolchains/Common/Source: COMMAND.C:2096-2106 cooldown gate, COMMAND.C:2150-2156 F0365/F0366 dispatch, CLIKMENU.C:278-329 collision/F0267 gate, CLIKMENU.C:330-346 G0310/G0311 cooldown publication, CHAMPION.C:1180-1215 movement ticks, MOVESENS.C:752-818 scent/sensor/destination group side effects, GAMELOOP.C:69-155 timeline/redraw/game-time/cooldown cadence, GAMELOOP.C:215-219 command-loop wait cadence, DUNVIEW.C:8318-8612 viewport draw, DRAWVIEW.C:709-722 viewport present request/vblank";
}
