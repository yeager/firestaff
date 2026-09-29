#ifndef FIRESTAFF_DM1_ENTRANCE_M11_H
#define FIRESTAFF_DM1_ENTRANCE_M11_H

#include "m11_game_view.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M11_ENTRANCE_PHASE_WAIT = 1,
    M11_ENTRANCE_PHASE_CREDITS = 2,
    M11_ENTRANCE_PHASE_DOORS = 3
};

/* Optional same-thread observer for the real synchronous entrance event pump.
 * Called after audio service and before input polling, never while focus is
 * suspended. The audio owner is borrowed and must not be freed by observers.
 * activeWaitMs excludes focus suspension and includes Credits time. */
typedef void (*M11_EntranceObserver)(void* user, int phase,
                                     uint64_t activeWaitMs,
                                     const M11_AudioState* audio);

int M11_Entrance_RunSourceTransition(
    M11_GameViewState* gameView,
    int autoEnterAfterMs,
    const DM1_V1_EntranceFullStartRenderReceiptPc34* entranceReceipt,
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* mediaReceipt,
    M11_EntranceObserver observer,
    void* user);

#ifdef __cplusplus
}
#endif
#endif
