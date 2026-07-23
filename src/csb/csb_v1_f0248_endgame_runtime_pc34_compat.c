#include "csb_v1_f0248_endgame_runtime_pc34_compat.h"

#include <string.h>

int csb_v1_f0248_endgame_consume_pc34_compat(
    const struct SensorTriggerResult_Compat *sensor_result,
    int already_won,
    CSB_V1_F0248EndgameRuntimeReceipt_PC34 *out_receipt)
{
    CSB_EndgameResult endgame;
    int delay_seconds;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!sensor_result || already_won ||
        !sensor_result->triggered ||
        !sensor_result->endGameGameWon ||
        !sensor_result->endGameRestartGameAllowedCleared ||
        !sensor_result->endGamePresentationRequested ||
        sensor_result->endGameDelayTicks < 0 ||
        sensor_result->endGameDelayTicks % 60 != 0) {
        return 0;
    }

    /* ReDMCSB TIMELINE.C F0248 C018 passes Remote.Value to the ENDGAME.C
     * sequence, whose CHANGE8_02 delay is expressed in 60 Hz ticks. */
    delay_seconds = sensor_result->endGameDelayTicks / 60;
    memset(&endgame, 0, sizeof(endgame));
    csb_endgame_trigger(delay_seconds, &endgame);
    if (!endgame.gameWon || endgame.restartAllowed ||
        !endgame.paletteSetCurtain || !endgame.endgameFnCalled ||
        !endgame.startendEndgameFn ||
        endgame.delayTicks != sensor_result->endGameDelayTicks) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->delay_ticks = endgame.delayTicks;
    out_receipt->palette_curtain = endgame.paletteSetCurtain;
    out_receipt->restart_disabled = !endgame.restartAllowed;
    out_receipt->game_won = endgame.gameWon;
    out_receipt->f0666_entered = endgame.endgameFnCalled;
    out_receipt->startend_endgame_requested = endgame.startendEndgameFn;
    return 1;
}
