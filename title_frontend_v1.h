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

typedef struct V1_TitleFrontendSequenceDecision {
    unsigned int requestedStepOrdinal;
    unsigned int renderFrameOrdinal;
    unsigned int completedAnimationLoops;
    int handoffReady;
    V1_TitleFrontendSequenceAction action;
} V1_TitleFrontendSequenceDecision;

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
