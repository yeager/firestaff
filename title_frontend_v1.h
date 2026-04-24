#ifndef FIRESTAFF_TITLE_FRONTEND_V1_H
#define FIRESTAFF_TITLE_FRONTEND_V1_H

#include <stddef.h>

#include "title_dat_loader_v1.h"

#ifdef __cplusplus
extern "C" {
#endif

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
 * Render one pass-57 decoded original TITLE frame into the PC34 4bpp
 * screen-bitmap format used by the M9/V1 frontend path.  The caller owns
 * cadence: requestedFrameOrdinal is wrapped over the original 53-frame bank
 * for deterministic implementation playback only; this function does not
 * claim original wall-clock timing.
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
