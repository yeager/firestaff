/*
 * title_frontend_v1.c — Pass 58.
 *
 * Bounded frontend binding for the pass-57 original TITLE renderer.  This
 * module adapts decoded 320x200 TITLE palette-index frames into the existing
 * PC34 4bpp screen-bitmap format used by the V1/M9 frontend.  It intentionally
 * does not infer original animation cadence.  Presentation callers can use the
 * finite sequence decision helper to hold the final source frame at the handoff
 * seam.  ReDMCSB TITLE.C now source-locks the original PC/ST control-flow
 * cadence used for the zoom/fade seam evidence.
 */

#include "title_frontend_v1.h"

#include <string.h>

static void title_frontend_set_err(char* dst, size_t cap, const char* msg) {
    size_t n;
    if (!dst || cap == 0) return;
    n = strlen(msg);
    if (n >= cap) n = cap - 1;
    memcpy(dst, msg, n);
    dst[n] = '\0';
}

static void title_frontend_write_u16_le(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v & 0xffu);
    p[1] = (unsigned char)((v >> 8) & 0xffu);
}

typedef struct TitleFrontendCapture {
    unsigned int targetOrdinal;
    unsigned int totalFrames;
    int copied;
    V1_TitleFrontendRenderResult* result;
    unsigned char* screenBitmap;
} TitleFrontendCapture;

static void title_frontend_pack_4bpp(const V1_TitleRenderFrame* frame,
                                     unsigned char* screenBitmap) {
    unsigned int x;
    unsigned int y;
    const unsigned int bytesPerRow = 160u;

    title_frontend_write_u16_le(screenBitmap - 4, frame->width);
    title_frontend_write_u16_le(screenBitmap - 2, frame->height);
    memset(screenBitmap, 0, bytesPerRow * frame->height);
    for (y = 0; y < frame->height; ++y) {
        unsigned char* row = screenBitmap + y * bytesPerRow;
        const unsigned char* src = frame->colorIndices + y * frame->width;
        for (x = 0; x < frame->width; x += 2u) {
            unsigned int hi = src[x] & 0x0fu;
            unsigned int lo = (x + 1u < frame->width) ? (src[x + 1u] & 0x0fu) : 0u;
            row[x >> 1] = (unsigned char)((hi << 4) | lo);
        }
    }
}

static int title_frontend_on_frame(const V1_TitleRenderFrame* frame,
                                   void* userData,
                                   char* errMsg,
                                   size_t errMsgBytes) {
    TitleFrontendCapture* capture = (TitleFrontendCapture*)userData;
    if (!capture || !frame || !capture->screenBitmap) {
        title_frontend_set_err(errMsg, errMsgBytes, "invalid TITLE frontend callback arguments");
        return 0;
    }
    capture->totalFrames++;
    if (frame->frameOrdinal == capture->targetOrdinal) {
        if (frame->width != 320u || frame->height != 200u || !frame->colorIndices) {
            title_frontend_set_err(errMsg, errMsgBytes, "TITLE frontend frame has unsupported dimensions");
            return 0;
        }
        title_frontend_pack_4bpp(frame, capture->screenBitmap);
        if (capture->result) {
            capture->result->renderedFrameOrdinal = frame->frameOrdinal;
            capture->result->paletteOrdinal = frame->paletteOrdinal;
            capture->result->durationFrames = frame->durationFrames;
            capture->result->width = frame->width;
            capture->result->height = frame->height;
            capture->result->usedOriginalTitleData = 1;
        }
        capture->copied = 1;
    }
    return 1;
}

V1_TitleFrontendSequenceDecision V1_TitleFrontend_DecideSequenceStep(unsigned int requestedStepOrdinal) {
    V1_TitleFrontendSequenceDecision decision;
    unsigned int step = requestedStepOrdinal;

    memset(&decision, 0, sizeof(decision));
    if (step == 0u) step = 1u;
    decision.requestedStepOrdinal = requestedStepOrdinal;
    if (step <= V1_TITLE_DAT_FRAME_MAX) {
        decision.renderFrameOrdinal = step;
        decision.action = V1_TITLE_FRONTEND_SEQUENCE_RENDER_TITLE;
        decision.completedAnimationLoops = 0u;
        decision.handoffReady = (step == V1_TITLE_DAT_FRAME_MAX) ? 1 : 0;
    } else {
        decision.renderFrameOrdinal = V1_TITLE_DAT_FRAME_MAX;
        decision.action = V1_TITLE_FRONTEND_SEQUENCE_HOLD_LAST_FRAME;
        decision.completedAnimationLoops = 1u;
        decision.handoffReady = 1;
    }
    return decision;
}

V1_TitleFrontendHandoffDecision V1_TitleFrontend_DecideTitleMenuHandoffStep(unsigned int requestedStepOrdinal,
                                                                            int enterMenuAfterHandoff) {
    V1_TitleFrontendHandoffDecision decision;
    unsigned int step = requestedStepOrdinal;

    memset(&decision, 0, sizeof(decision));
    if (step == 0u) step = 1u;
    decision.title = V1_TitleFrontend_DecideSequenceStep(requestedStepOrdinal);
    decision.surface = V1_TITLE_FRONTEND_SURFACE_TITLE;
    if (enterMenuAfterHandoff && step > V1_TITLE_DAT_FRAME_MAX && decision.title.handoffReady) {
        decision.surface = V1_TITLE_FRONTEND_SURFACE_MENU;
        decision.enteredMenuAfterHandoff = 1;
    }
    return decision;
}


static const char* title_frontend_source_event_line(V1_TitleFrontendSourceEventKind kind) {
    switch (kind) {
        case V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS:
            return "TITLE.C:319-324 draws black screen then presents box 0..319,90..105 from source y=137";
        case V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT:
            return "TITLE.C:340-360 builds 18 shrinked bitmaps; TITLE.C:385-387 waits VBlank then blits in reverse order";
        case V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK:
            return "TITLE.C:395-396 waits two post-zoom vertical blanks";
        case V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT:
            return "TITLE.C:397-402 blits Master/Strikes Back to destination box 0..319,118..174";
        case V1_TITLE_FRONTEND_SOURCE_EVENT_FINAL_GUARD_VBLANK:
            return "TITLE.C:409 / TITLE.C:251 BUG0_71 final M526_WaitVerticalBlank guard before transition";
        case V1_TITLE_FRONTEND_SOURCE_EVENT_MENU_ELIGIBLE:
            return "TITLE.C after final guard VBlank; Firestaff handoff may enter caller-owned menu surface";
    }
    return "TITLE.C source event";
}

unsigned int V1_TitleFrontend_GetSourceAnimationStepCount(void) {
    return 23u;
}

int V1_TitleFrontend_GetSourceAnimationStep(unsigned int sourceStepOrdinal,
                                            V1_TitleFrontendSourceAnimationStep* outStep) {
    V1_TitleFrontendSourceAnimationStep step;
    unsigned int z;
    if (!outStep || sourceStepOrdinal == 0u || sourceStepOrdinal > V1_TitleFrontend_GetSourceAnimationStepCount()) return 0;
    memset(&step, 0, sizeof(step));
    step.sourceStepOrdinal = sourceStepOrdinal;
    if (sourceStepOrdinal == 1u) {
        step.kind = V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS;
        step.x = 0u;
        step.y = 90u;
        step.width = 320u;
        step.height = 16u;
    } else if (sourceStepOrdinal >= 2u && sourceStepOrdinal <= 19u) {
        /* ReDMCSB stores shrinked title index 0 as 320x80 and index 17 as 48x12,
         * then renders from L3496_i_ActualBitmapCount down to 0. */
        z = 19u - sourceStepOrdinal;
        step.kind = V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT;
        step.vblankBeforeEvent = 1u;
        step.zoomSourceIndex = z;
        step.width = 320u - 16u * z;
        step.height = 80u - 4u * z;
        step.x = (320u - step.width) >> 1;
        step.y = (160u - step.height) >> 1;
    } else if (sourceStepOrdinal == 20u || sourceStepOrdinal == 21u) {
        step.kind = V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK;
        step.vblankBeforeEvent = 1u;
    } else if (sourceStepOrdinal == 22u) {
        step.kind = V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT;
        step.x = 0u;
        step.y = 118u;
        step.width = 320u;
        step.height = 57u;
    } else {
        step.kind = V1_TITLE_FRONTEND_SOURCE_EVENT_FINAL_GUARD_VBLANK;
        step.vblankBeforeEvent = 1u;
    }
    step.sourceLineEvidence = title_frontend_source_event_line(step.kind);
    *outStep = step;
    return 1;
}

V1_TitleFrontendSourceTiming V1_TitleFrontend_GetSourceTimingEvidence(void) {
    V1_TitleFrontendSourceTiming timing;
    memset(&timing, 0, sizeof(timing));
    timing.zoomStepCount = 18u;
    timing.vblankBeforeEachZoomStep = 1u;
    timing.postZoomVblankCount = 2u;
    timing.finalFadeGuardVblankCount = 1u;
    timing.firstMenuEligibleStep = V1_TITLE_DAT_FRAME_MAX + 1u;
    timing.sourceAnimationStepCount = V1_TitleFrontend_GetSourceAnimationStepCount();
    timing.sourceFile = "ReDMCSB_WIP20210206/Toolchains/Common/Source/TITLE.C";
    timing.sourceFunction = "F0437_STARTEND_DrawTitle";
    timing.evidenceNote = "PC/F20 TITLE.C path: presents strip, 18 reverse-order zoom blits each preceded by M526_WaitVerticalBlank(), then two M526_WaitVerticalBlank() calls, then Master/Strikes Back blit/fade, then final BUG0_71 M526_WaitVerticalBlank() before transition.";
    return timing;
}

int V1_TitleFrontend_RenderFrameToScreen(const char* titleDatPath,
                                         unsigned int requestedFrameOrdinal,
                                         unsigned char* screenBitmap,
                                         V1_TitleFrontendRenderResult* outResult,
                                         char* errMsg,
                                         size_t errMsgBytes) {
    TitleFrontendCapture capture;
    unsigned int target;

    if (outResult) memset(outResult, 0, sizeof(*outResult));
    if (!titleDatPath || !screenBitmap) {
        title_frontend_set_err(errMsg, errMsgBytes, "null TITLE frontend argument");
        return 0;
    }
    target = requestedFrameOrdinal;
    if (target == 0u) target = 1u;
    target = ((target - 1u) % V1_TITLE_DAT_FRAME_MAX) + 1u;

    if (outResult) {
        outResult->requestedFrameOrdinal = requestedFrameOrdinal;
        outResult->renderedFrameOrdinal = target;
    }
    memset(&capture, 0, sizeof(capture));
    capture.targetOrdinal = target;
    capture.result = outResult;
    capture.screenBitmap = screenBitmap;

    if (!V1_Title_RenderFrames(titleDatPath, title_frontend_on_frame, &capture, errMsg, errMsgBytes)) {
        return 0;
    }
    if (outResult) outResult->totalFrames = capture.totalFrames;
    if (!capture.copied) {
        title_frontend_set_err(errMsg, errMsgBytes, "requested TITLE frame was not rendered");
        return 0;
    }
    return 1;
}
