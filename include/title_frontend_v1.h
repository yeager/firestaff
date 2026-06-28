#ifndef FIRESTAFF_TITLE_FRONTEND_V1_H
#define FIRESTAFF_TITLE_FRONTEND_V1_H

#include <stddef.h>

#include "title_dat_loader_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum V1_TitleFrontendSequenceAction {
    V1_TITLE_FRONTEND_SEQUENCE_RENDER_TITLE = 0,
    V1_TITLE_FRONTEND_SEQUENCE_HOLD_LAST_FRAME = 1
} V1_TitleFrontendSequenceAction;

typedef enum V1_TitleFrontendSurface {
    V1_TITLE_FRONTEND_SURFACE_TITLE = 0,
    V1_TITLE_FRONTEND_SURFACE_MENU = 1
} V1_TitleFrontendSurface;

typedef struct V1_TitleFrontendSequenceDecision {
    unsigned int requestedStepOrdinal;
    unsigned int renderFrameOrdinal;
    unsigned int completedAnimationLoops;
    int handoffReady;
    V1_TitleFrontendSequenceAction action;
} V1_TitleFrontendSequenceDecision;

typedef struct V1_TitleFrontendHandoffDecision {
    V1_TitleFrontendSequenceDecision title;
    V1_TitleFrontendSurface surface;
    int enteredMenuAfterHandoff;
} V1_TitleFrontendHandoffDecision;

typedef struct V1_TitleFrontendRenderResult {
    unsigned int requestedFrameOrdinal;
    unsigned int renderedFrameOrdinal;
    unsigned int totalFrames;
    unsigned int paletteOrdinal;
    unsigned int durationFrames;
    unsigned int width;
    unsigned int height;
    int usedOriginalTitleData;
} V1_TitleFrontendRenderResult;

typedef enum V1_TitleFrontendSourceEventKind {
    V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS = 0,
    V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT = 1,
    V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK = 2,
    V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT = 3,
    V1_TITLE_FRONTEND_SOURCE_EVENT_FINAL_GUARD_VBLANK = 4,
    V1_TITLE_FRONTEND_SOURCE_EVENT_MENU_ELIGIBLE = 5
} V1_TitleFrontendSourceEventKind;

typedef struct V1_TitleFrontendSourceAnimationStep {
    unsigned int sourceStepOrdinal;
    V1_TitleFrontendSourceEventKind kind;
    unsigned int vblankBeforeEvent;
    unsigned int zoomSourceIndex;
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
    const char* sourceLineEvidence;
} V1_TitleFrontendSourceAnimationStep;

typedef struct V1_TitleFrontendSourceTiming {
    unsigned int zoomStepCount;
    unsigned int vblankBeforeEachZoomStep;
    unsigned int postZoomVblankCount;
    unsigned int finalFadeGuardVblankCount;
    unsigned int firstMenuEligibleStep;
    unsigned int sourceAnimationStepCount;
    const char* sourceFile;
    const char* sourceFunction;
    const char* evidenceNote;
} V1_TitleFrontendSourceTiming;

typedef enum V1_TitleFrontendRuntimeSource {
    V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP = 0,
    V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001 = 1,
    V1_TITLE_FRONTEND_RUNTIME_SOURCE_TITLE_DAT_FALLBACK = 2
} V1_TitleFrontendRuntimeSource;

typedef struct V1_TitleFrontendRuntimeSourceDecision {
    V1_TitleFrontendRuntimeSource source;
    int graphicsC001Usable;
    int titleDatFallbackUsable;
    const char* sourceLineEvidence;
} V1_TitleFrontendRuntimeSourceDecision;

/*
 * Map a finite TITLE presentation step onto the source-backed TITLE frame bank.
 * Steps 1..53 render source frames 1..53.  Later steps hold frame 53 and mark
 * handoffReady so callers can stop TITLE cleanly before entering the menu path.
 * ReDMCSB TITLE.C source-lock: the PC/ST path waits one vertical blank before
 * each of 18 zoom blits, waits two more vertical blanks after the zoom loop,
 * and waits one final vertical blank after the Master/Strikes Back fade
 * (BUG0_71). Runtime wait helpers map those waits onto Firestaff's canonical
 * V1 55 ms tick so TITLE does not replay at display-refresh speed. This helper
 * still maps over the decoded 53-frame TITLE.DAT bank; use
 * V1_TitleFrontend_GetSourceTimingEvidence() when callers need the locked
 * original control-flow cadence evidence.
 */
V1_TitleFrontendSequenceDecision V1_TitleFrontend_DecideSequenceStep(unsigned int requestedStepOrdinal);

V1_TitleFrontendSourceTiming V1_TitleFrontend_GetSourceTimingEvidence(void);
unsigned int V1_TitleFrontend_GetRuntimeFrameDelayMs(const V1_TitleFrontendSourceTiming* timing);
unsigned int V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(const V1_TitleFrontendSourceTiming* timing);

/*
 * Select the runtime source for the DM1 V1 TITLE animation.  The original
 * ReDMCSB PC path loads GRAPHICS.DAT C001_GRAPHIC_TITLE in TITLE.C F0437
 * before drawing PRESENTS / DUNGEON MASTER / STRIKES BACK.  Firestaff may use
 * the decoded TITLE.DAT bank only as a last-resort visible fallback when C001
 * is missing or too small for the source blits.  A usable C001 graphic always
 * wins, even if TITLE.DAT is also present.
 */
V1_TitleFrontendRuntimeSourceDecision V1_TitleFrontend_SelectRuntimeSource(
    int graphicsC001CandidateAvailable,
    unsigned int graphicsC001Width,
    unsigned int graphicsC001Height,
    int titleDatFallbackAvailable);

/*
 * ReDMCSB TITLE.C source animation event schedule for the PC/F20 path.
 * This is separate from the decoded 53-frame TITLE.DAT bank: TITLE.C builds
 * 18 shrinked title bitmaps from 320x80 down to 48x12, then presents them
 * in reverse order (small -> full) with one M526_WaitVerticalBlank() before
 * each blit. It then waits two more VBlanks, blits the Master/Strikes Back
 * strip at y=118..174, fades, waits the BUG0_71 guard VBlank, and only then
 * the caller may enter the next surface.
 */
unsigned int V1_TitleFrontend_GetSourceAnimationStepCount(void);
int V1_TitleFrontend_GetSourceAnimationStep(unsigned int sourceStepOrdinal,
                                            V1_TitleFrontendSourceAnimationStep* outStep);

/*
 * Deterministic implementation handoff for callers that want a finite
 * TITLE->menu path.  With enterMenuAfterHandoff=0 this preserves the pass-59
 * hold-last-frame policy.  With enterMenuAfterHandoff=1, steps after the
 * source DO boundary (step 53) switch caller-owned presentation to the menu
 * surface while retaining frame 53 as the last valid TITLE frame for evidence.
 * This is not an original cadence or emulator handoff timing claim.
 */
V1_TitleFrontendHandoffDecision V1_TitleFrontend_DecideTitleMenuHandoffStep(unsigned int requestedStepOrdinal,
                                                                            int enterMenuAfterHandoff);

/*
 * Render one pass-57 decoded original TITLE frame into the PC34 4bpp
 * screen-bitmap format used by the M9/V1 frontend path.  The caller owns
 * cadence: requestedFrameOrdinal is wrapped over the original 53-frame bank
 * for deterministic low-level sampling only; presentation callers that model
 * the finite TITLE->menu seam should use V1_TitleFrontend_DecideSequenceStep
 * first and render its renderFrameOrdinal.
 */
int V1_TitleFrontend_RenderFrameToScreen(const char* titleDatPath,
                                         unsigned int requestedFrameOrdinal,
                                         unsigned char* screenBitmap,
                                         V1_TitleFrontendRenderResult* outResult,
                                         char* errMsg,
                                         size_t errMsgBytes);

/*
 * Map a TITLE source animation step kind to the source-locked
 * VGA_PALETTE_PC34_SPECIAL_* palette index that the original F20E PC 3.4
 * TITLE.C F0437 routine applies at that step.  Returns:
 *   - VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS for the PRESENTS blit
 *     (TITLE.C:319-324, palette C12_PRESENTS — white on black).
 *   - VGA_PALETTE_PC34_SPECIAL_TITLE for ZOOM_BLIT and
 *     MASTER_STRIKES_BACK_BLIT steps (TITLE.C:340-402, palette
 *     C13_DUNGEON + C14_MASTER — warm browns/golds with red on 0x0F).
 *   - VGA_PALETTE_PC34_SPECIAL_TITLE for all other steps so that any
 *     present blit the runtime adds later still gets the DUNGEON+MASTER
 *     palette.  The PRESENTS phase must never borrow the DUNGEON+MASTER
 *     palette — that is the v2.7.4 release regression this helper was
 *     added to guard.  This helper is the single source of truth for
 *     the title-step → palette mapping; main_loop_m11.c and any other
 *     caller must use it instead of hard-coding a palette index.
 */
int V1_TitleFrontend_GetStepPalette(V1_TitleFrontendSourceEventKind kind,
                                    int* outSpecialPalette);

/*
 * Map a decoded TITLE/TITLE.DAT fallback frame palette ordinal onto the
 * same ReDMCSB TITLE.C palette phases as the GRAPHICS.DAT C001 path.
 * The PC 3.4 TITLE bank uses paletteOrdinal=1 for the PRESENTS frame
 * and later ordinals for the DUNGEON/MASTER zoom frames.  Keep this
 * fallback behind the same source-backed helper as the normal path so
 * callers do not hard-code special palette constants.
 */
int V1_TitleFrontend_GetFallbackFramePalette(unsigned int paletteOrdinal,
                                             int* outSpecialPalette);

#ifdef __cplusplus
}
#endif

#endif
