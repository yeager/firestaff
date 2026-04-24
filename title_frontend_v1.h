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

/*
 * Map a finite TITLE presentation step onto the source-backed TITLE frame bank.
 * Steps 1..53 render source frames 1..53.  Later steps hold frame 53 and mark
 * handoffReady so callers can stop TITLE cleanly before entering the menu path.
 * This is deterministic implementation policy only; it does not claim original
 * wall-clock cadence or emulator handoff timing.
 */
V1_TitleFrontendSequenceDecision V1_TitleFrontend_DecideSequenceStep(unsigned int requestedStepOrdinal);

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

#ifdef __cplusplus
}
#endif

#endif
