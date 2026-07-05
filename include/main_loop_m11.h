#ifndef FIRESTAFF_MAIN_LOOP_M11_H
#define FIRESTAFF_MAIN_LOOP_M11_H

#include "menu_startup_m12.h"

#include <stddef.h>
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
                              nexus, theron. Overrides auto-detection. */
    int directLaunch;      /* Non-zero when --game should bypass M12. */
} M11_PhaseA_Options;

void M11_PhaseA_SetDefaultOptions(M11_PhaseA_Options* opts);

/* Run the Phase-A proof-of-life loop. Returns 0 on clean shutdown, non-
   zero on error. Safe to call multiple times (each call initialises +
   tears down the render module). */
int  M11_PhaseA_Run(const M11_PhaseA_Options* opts);
void M11_ApplyStartupMenuRuntime(M12_StartupMenuState* menuState);

/* Map a point from the active presented game surface back to the source
   320x200 DM1 framebuffer. V2.1/V2.2 pass their selected presentation
   resolution here before source-locked mouse zone dispatch. */
int M11_MapPresentedGamePointToSourceForPresentation(int presentationMode,
                                                     int presentationWidth,
                                                     int presentationHeight,
                                                     int* x,
                                                     int* y);

/* Map a point from the source-locked 320x200 DM1 framebuffer out to
   the active presented game surface. The inverse of
   M11_MapPresentedGamePointToSourceForPresentation(), used by touch
   overlay hit-tests, HUD button bounds, and mouse cursor positions
   that need to land on the *presented* surface (V2.0 = 640x400,
   V2.1/V2.2 = user-selected 320x200..3840x2160). V1 original mode is
   a pass-through; V2.1/V2.2 require positive extents. */
int M11_MapSourcePointToPresentedForPresentation(int presentationMode,
                                                 int presentationWidth,
                                                 int presentationHeight,
                                                 int* x,
                                                 int* y);

/* V1 original and V2.0 filtered both present the source-locked 320x200 glyph
   layer.  They must stay nearest-neighbor so small original glyphs such as DM1
   wall inscriptions remain readable when the window is enlarged. */
int M11_ResolveGameScaleFilterForPresentation(int presentationMode,
                                              int requestedScaleFilter);

/* Source-locked game presentation geometry: drives the three-way choice in
   m11_present_game_frame() between PresentScaledIndexed (V20_FILTERED),
   PresentIndexedToResolution (V21_UPSCALED / V22_MODERN with valid extents),
   and Present (V1_ORIGINAL or V21/V22 without user-selected extents).

   The two helpers are promoted from static so the launcher→game handoff
   contract is regression-testable without an SDL window, and they take
   raw mode + extents (matching the existing M11_MapPresentedGamePointTo
   SourceForPresentation contract) so they can be linked from
   input-mapping tests/probes without dragging m11_game_view.h into
   main_loop_m11.h.

   ReDMCSB: COMMAND.C:1379-1449 F0358 / 1641-1660 F0359 mouse-row + primary
            click dispatch against a 320x200 source zone;
            src/engine/main_loop_m11.c (m11_game_indexed_presentation_scale +
            m11_game_presentation_target);
            src/ui/menu_startup_m12.c M12_PresentationMode_AllowsResolutionChoice.

   Returns the integer nearest-source-framebuffer scale factor the present
   path should use (1 for V1/V21/V22, 2 for V20_FILTERED).
   presentationMode is one of M12_PRESENTATION_V*.  Any unknown mode is
   treated as V1 (scale 1). */
int M11_GameView_PresentationIndexedScale(int presentationMode);

/* Returns 1 when the resolved presentation target differs from the
   default 320x200 source framebuffer (i.e. the caller should hand off to
   PresentIndexedToResolution with the user-selected presentationWidth /
   presentationHeight); returns 0 when the default Present path applies.

   V20_FILTERED always resolves to (640x400) here even though it returns 0,
   because the present loop short-circuits V20 via the indexed-scale
   contract above.  V21_UPSCALED / V22_MODERN require
   presentationWidth > 0 AND presentationHeight > 0 to be considered "user
   picked a non-default resolution"; when either is non-positive the
   helpers resolve to (320x200) and return 0 (default Present path).

   V1_ORIGINAL and unknown modes always resolve to (320x200) and return 0.

   outW/outH receive the resolved target (320x200, 640x400, or the
   user-selected resolution) — NULL outW/outH is safe. */
int M11_GameView_PresentationTarget(int presentationMode,
                                    int presentationWidth,
                                    int presentationHeight,
                                    int* outW,
                                    int* outH);

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

/* Resolve the save path used by the DM1 entrance RESUME button.  The launcher
   may already have a validated quick-resume path; use it for DM1 before
   falling back to the historical source-id quicksave filename. */
int M11_Entrance_ResolveDm1ResumeSavePath(const char* sourceId,
                                          int quickResumeAvailable,
                                          const char* quickResumeGameId,
                                          const char* quickResumeSavePath,
                                          char* outPath,
                                          size_t outPathBytes);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MAIN_LOOP_M11_H */
