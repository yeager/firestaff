#ifndef FIRESTAFF_MAIN_LOOP_M11_H
#define FIRESTAFF_MAIN_LOOP_M11_H

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
} M11_PhaseA_Options;

void M11_PhaseA_SetDefaultOptions(M11_PhaseA_Options* opts);

/* Run the Phase-A proof-of-life loop. Returns 0 on clean shutdown, non-
   zero on error. Safe to call multiple times (each call initialises +
   tears down the render module). */
int  M11_PhaseA_Run(const M11_PhaseA_Options* opts);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MAIN_LOOP_M11_H */
