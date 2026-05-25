#ifndef FIRESTAFF_MAIN_LOOP_M11_H
#define FIRESTAFF_MAIN_LOOP_M11_H

#include "menu_startup_m12.h"

#include <stdint.h>

/*
 * main_loop_m11 — M11 Phase A stub.
 *
 * Phase A only needs a tiny driver: open a window, pump events for a
 * bounded duration, then close. The full semi-fixed timestep main loop
 * (tick orchestrator + render interpolation + audio pump) arrives in
 * Phase I.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Options passed to the Phase-A stub loop. */
typedef struct {
    int windowWidth;       /* Default: 640 */
    int windowHeight;      /* Default: 400 */
    int scaleMode;         /* M11_SCALE_* */
    int durationMs;        /* <0 = run until exit, 0 = close
                              immediately after one present. */
    int presentEveryMs;    /* How often to present during the loop.
                              Default 16 (≈60Hz). */
    const char* script;    /* Optional comma-separated input script:
                              up,down,left,right,enter,esc. */
    const char* dataDir;   /* Optional override for asset detection.
                              Falls back to FIRESTAFF_DATA. */
    const char* gameId;    /* Optional game to pre-select: dm1, csb, dm2,
                              nexus1, theron. Overrides auto-detection. */
} M11_PhaseA_Options;

void M11_PhaseA_SetDefaultOptions(M11_PhaseA_Options* opts);

/* Run the Phase-A proof-of-life loop. Returns 0 on clean shutdown, non-
   zero on error. Safe to call multiple times (each call initialises +
   tears down the render module). */
int  M11_PhaseA_Run(const M11_PhaseA_Options* opts);
void M11_ApplyStartupMenuRuntime(M12_StartupMenuState* menuState);

/* Source-locked entrance wait policy: interactive builds must not auto-enter
   after launcher handoff; only headless/autotest runs may use a timeout. */
int M11_Entrance_ShouldAutoEnterForTimeout(int allowHeadlessTimeout,
                                           int autoEnterAfterMs,
                                           uint64_t elapsedMs);

enum M11_EntranceRuntimeCommandId {
    M11_ENTRANCE_RUNTIME_COMMAND_NONE = 0,
    M11_ENTRANCE_RUNTIME_COMMAND_ENTER_DUNGEON = 200,
    M11_ENTRANCE_RUNTIME_COMMAND_ENTER_BONUS_DUNGEON = 201,
    M11_ENTRANCE_RUNTIME_COMMAND_RESUME = 202,
    M11_ENTRANCE_RUNTIME_COMMAND_DRAW_CREDITS = 203,
    M11_ENTRANCE_RUNTIME_COMMAND_QUIT = 216
};

/* Source-locked entrance/menu pointer dispatch. Coordinates are already in
   the 320x200 DM1 framebuffer space; buttonMask uses ReDMCSB mouse masks. */
int M11_Entrance_DispatchSourceLockedPointerCommand(int framebufferX,
                                                    int framebufferY,
                                                    unsigned int buttonMask);

/* Runtime keyboard guard for the current click-only entrance semantics. */
int M11_Entrance_DispatchSourceLockedKeyCommand(int keyCode);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MAIN_LOOP_M11_H */
