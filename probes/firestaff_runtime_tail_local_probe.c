#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boot_plan_script_pc34_compat.h"
#include "memory_graphics_dat_startup_pc34_compat.h"
#include "memory_graphics_dat_bitmap_path_pc34_compat.h"
#include "screen_bitmap_present_pc34_compat.h"
#include "dialog_frontend_pc34_compat.h"
#include "memory_graphics_dat_menu_state_pc34_compat.h"

struct Img3CommandMetrics_Compat {
    unsigned long commandCount;
    unsigned long repeatedCommandCount;
    unsigned long explicitColorCommandCount;
    unsigned long copyPreviousLineCommandCount;
    unsigned long totalPixels;
    unsigned long maxRunLength;
    unsigned long kindCount[8];
    unsigned long kindPixelCount[8];
};

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static unsigned short read_u16_le(const unsigned char* p) {
    return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

static unsigned char read_nibble_at(const unsigned char* src, unsigned int nibbleIndex) {
    unsigned char packed = src[nibbleIndex >> 1];
    return (nibbleIndex & 1U) ? (packed & 0x0F) : (packed >> 4);
}

static unsigned int img3_read_pixel_count_from_nibbles(const unsigned char* src, unsigned int* nibbleIndex) {
    unsigned int value = read_nibble_at(src, (*nibbleIndex)++);
    if (value == 15U) {
        value = (read_nibble_at(src, (*nibbleIndex)++) << 4) | read_nibble_at(src, (*nibbleIndex)++);
        if (value == 255U) {
            value = (read_nibble_at(src, (*nibbleIndex)++) << 12)
                  | (read_nibble_at(src, (*nibbleIndex)++) << 8)
                  | (read_nibble_at(src, (*nibbleIndex)++) << 4)
                  | read_nibble_at(src, (*nibbleIndex)++);
        } else {
            value += 17U;
        }
    } else {
        value += 2U;
    }
    return value;
}

static int analyze_img3_command_metrics(
    const unsigned char* rawGraphic,
    unsigned short width,
    unsigned short height,
    struct Img3CommandMetrics_Compat* outMetrics) {
    unsigned int nibbleIndex = 8U;
    unsigned long totalPixels = (unsigned long)width * (unsigned long)height;
    unsigned long producedPixels = 0;
    unsigned int i;

    if ((rawGraphic == 0) || (outMetrics == 0)) return 0;
    memset(outMetrics, 0, sizeof(*outMetrics));

    for (i = 0; i < 6U; ++i) {
        (void)read_nibble_at(rawGraphic, nibbleIndex++);
    }
    while (producedPixels < totalPixels) {
        unsigned char command = read_nibble_at(rawGraphic, nibbleIndex++);
        unsigned char kind = command & 0x07U;
        unsigned int runLength;

        if (kind > 7U) return 0;
        if (kind == 7U) {
            (void)read_nibble_at(rawGraphic, nibbleIndex++);
            outMetrics->explicitColorCommandCount++;
        } else if (kind == 6U) {
            outMetrics->copyPreviousLineCommandCount++;
        }
        if (command & 0x08U) {
            runLength = img3_read_pixel_count_from_nibbles(rawGraphic, &nibbleIndex);
            outMetrics->repeatedCommandCount++;
        } else {
            runLength = 1U;
        }
        outMetrics->commandCount++;
        outMetrics->kindCount[kind]++;
        outMetrics->kindPixelCount[kind] += (unsigned long)runLength;
        producedPixels += (unsigned long)runLength;
        if ((unsigned long)runLength > outMetrics->maxRunLength) {
            outMetrics->maxRunLength = (unsigned long)runLength;
        }
        if (producedPixels > totalPixels) {
            return 0;
        }
    }
    outMetrics->totalPixels = producedPixels;
    return 1;
}

static unsigned char get_nibble(const unsigned char* bitmap, unsigned short x, unsigned short y) {
    unsigned short width = read_u16_le(bitmap - 4);
    unsigned short bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
    const unsigned char* row = bitmap + ((unsigned long)y * (unsigned long)bytesPerRow);
    unsigned char packed = row[x >> 1];
    return (x & 1) ? (packed & 0x0F) : (packed >> 4);
}

static unsigned long patch_sad_same_size(
    const unsigned char* a,
    const unsigned char* b,
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2) {
    unsigned short x, y;
    unsigned long sad = 0;
    for (y = y1; y < y2; ++y) {
        for (x = x1; x < x2; ++x) {
            unsigned char av = get_nibble(a, x, y);
            unsigned char bv = get_nibble(b, x, y);
            sad += (unsigned long)((av > bv) ? (av - bv) : (bv - av));
        }
    }
    return sad;
}

static unsigned long patch_diff_pixel_count_same_size(
    const unsigned char* a,
    const unsigned char* b,
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2) {
    unsigned short x, y;
    unsigned long count = 0;
    for (y = y1; y < y2; ++y) {
        for (x = x1; x < x2; ++x) {
            if (get_nibble(a, x, y) != get_nibble(b, x, y)) {
                count++;
            }
        }
    }
    return count;
}

static unsigned long patch_value_sum_same_size(
    const unsigned char* a,
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2) {
    unsigned short x, y;
    unsigned long sum = 0;
    for (y = y1; y < y2; ++y) {
        for (x = x1; x < x2; ++x) {
            sum += (unsigned long)get_nibble(a, x, y);
        }
    }
    return sum;
}

static void patch_signed_delta_same_size(
    const unsigned char* a,
    const unsigned char* b,
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2,
    unsigned long* outPositiveDeltaSum,
    unsigned long* outNegativeDeltaSum,
    long* outSignedDeltaSum) {
    unsigned short x, y;
    unsigned long positive = 0;
    unsigned long negative = 0;
    long signedSum = 0;
    for (y = y1; y < y2; ++y) {
        for (x = x1; x < x2; ++x) {
            int av = (int)get_nibble(a, x, y);
            int bv = (int)get_nibble(b, x, y);
            int delta = av - bv;
            if (delta > 0) {
                positive += (unsigned long)delta;
            } else if (delta < 0) {
                negative += (unsigned long)(-delta);
            }
            signedSum += (long)delta;
        }
    }
    if (outPositiveDeltaSum) *outPositiveDeltaSum = positive;
    if (outNegativeDeltaSum) *outNegativeDeltaSum = negative;
    if (outSignedDeltaSum) *outSignedDeltaSum = signedSum;
}

static void patch_signed_delta_halves_same_size(
    const unsigned char* a,
    const unsigned char* b,
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2,
    long* outLeftSignedDelta,
    long* outRightSignedDelta,
    long* outTopSignedDelta,
    long* outBottomSignedDelta) {
    unsigned short xMid = (unsigned short)(x1 + ((x2 - x1) / 2));
    unsigned short yMid = (unsigned short)(y1 + ((y2 - y1) / 2));
    unsigned short x, y;
    long left = 0, right = 0, top = 0, bottom = 0;
    for (y = y1; y < y2; ++y) {
        for (x = x1; x < x2; ++x) {
            long delta = (long)((int)get_nibble(a, x, y) - (int)get_nibble(b, x, y));
            if (x < xMid) left += delta; else right += delta;
            if (y < yMid) top += delta; else bottom += delta;
        }
    }
    if (outLeftSignedDelta) *outLeftSignedDelta = left;
    if (outRightSignedDelta) *outRightSignedDelta = right;
    if (outTopSignedDelta) *outTopSignedDelta = top;
    if (outBottomSignedDelta) *outBottomSignedDelta = bottom;
}

static void patch_signed_delta_row_bands_same_size(
    const unsigned char* a,
    const unsigned char* b,
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2,
    long* outTopBand,
    long* outMidBand,
    long* outBottomBand) {
    unsigned short height = (unsigned short)(y2 - y1);
    unsigned short band1 = (unsigned short)(y1 + (height / 3));
    unsigned short band2 = (unsigned short)(y1 + ((2 * height) / 3));
    unsigned short x, y;
    long top = 0, mid = 0, bottom = 0;
    for (y = y1; y < y2; ++y) {
        for (x = x1; x < x2; ++x) {
            long delta = (long)((int)get_nibble(a, x, y) - (int)get_nibble(b, x, y));
            if (y < band1) top += delta;
            else if (y < band2) mid += delta;
            else bottom += delta;
        }
    }
    if (outTopBand) *outTopBand = top;
    if (outMidBand) *outMidBand = mid;
    if (outBottomBand) *outBottomBand = bottom;
}

static void patch_positive_negative_row_bands_same_size(
    const unsigned char* a,
    const unsigned char* b,
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2,
    unsigned long* outTopPos,
    unsigned long* outTopNeg,
    unsigned long* outMidPos,
    unsigned long* outMidNeg,
    unsigned long* outBottomPos,
    unsigned long* outBottomNeg) {
    unsigned short height = (unsigned short)(y2 - y1);
    unsigned short band1 = (unsigned short)(y1 + (height / 3));
    unsigned short band2 = (unsigned short)(y1 + ((2 * height) / 3));
    unsigned short x, y;
    unsigned long topPos = 0, topNeg = 0, midPos = 0, midNeg = 0, bottomPos = 0, bottomNeg = 0;
    for (y = y1; y < y2; ++y) {
        for (x = x1; x < x2; ++x) {
            int delta = (int)get_nibble(a, x, y) - (int)get_nibble(b, x, y);
            if (y < band1) {
                if (delta > 0) topPos += (unsigned long)delta;
                else if (delta < 0) topNeg += (unsigned long)(-delta);
            } else if (y < band2) {
                if (delta > 0) midPos += (unsigned long)delta;
                else if (delta < 0) midNeg += (unsigned long)(-delta);
            } else {
                if (delta > 0) bottomPos += (unsigned long)delta;
                else if (delta < 0) bottomNeg += (unsigned long)(-delta);
            }
        }
    }
    if (outTopPos) *outTopPos = topPos;
    if (outTopNeg) *outTopNeg = topNeg;
    if (outMidPos) *outMidPos = midPos;
    if (outMidNeg) *outMidNeg = midNeg;
    if (outBottomPos) *outBottomPos = bottomPos;
    if (outBottomNeg) *outBottomNeg = bottomNeg;
}

static void patch_histogram_same_size(
    const unsigned char* a,
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2,
    unsigned long histogram[16]) {
    unsigned short x, y;
    unsigned int i;
    for (i = 0; i < 16; ++i) histogram[i] = 0;
    for (y = y1; y < y2; ++y) {
        for (x = x1; x < x2; ++x) {
            histogram[get_nibble(a, x, y)]++;
        }
    }
}

static unsigned long best_match_sad(
    const unsigned char* patchBitmap,
    unsigned short patchWidth,
    unsigned short patchHeight,
    const unsigned char* candidateBitmap) {
    unsigned short candWidth = read_u16_le(candidateBitmap - 4);
    unsigned short candHeight = read_u16_le(candidateBitmap - 2);
    unsigned short x, y;
    unsigned long best = 0xFFFFFFFFUL;

    if (candWidth < patchWidth || candHeight < patchHeight) {
        return 0xFFFFFFFFUL;
    }
    for (y = 0; y <= (unsigned short)(candHeight - patchHeight); ++y) {
        for (x = 0; x <= (unsigned short)(candWidth - patchWidth); ++x) {
            unsigned short px, py;
            unsigned long sad = 0;
            for (py = 0; py < patchHeight; ++py) {
                for (px = 0; px < patchWidth; ++px) {
                    unsigned char av = get_nibble(patchBitmap, px, py);
                    unsigned char bv = get_nibble(candidateBitmap, (unsigned short)(x + px), (unsigned short)(y + py));
                    sad += (unsigned long)((av > bv) ? (av - bv) : (bv - av));
                }
            }
            if (sad < best) {
                best = sad;
            }
        }
    }
    return best;
}

static int compute_nonzero_bbox(
    const unsigned char* bitmap,
    unsigned short* outX1,
    unsigned short* outY1,
    unsigned short* outX2,
    unsigned short* outY2,
    unsigned long* outNonzeroCount) {
    unsigned short width = read_u16_le(bitmap - 4);
    unsigned short height = read_u16_le(bitmap - 2);
    unsigned short x, y;
    int found = 0;
    unsigned short minX = 0, minY = 0, maxX = 0, maxY = 0;
    unsigned long nonzero = 0;

    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            unsigned char value = get_nibble(bitmap, x, y);
            if (value == 0) continue;
            if (!found) {
                minX = maxX = x;
                minY = maxY = y;
                found = 1;
            } else {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
            nonzero++;
        }
    }
    if (!found) {
        if (outX1) *outX1 = 0;
        if (outY1) *outY1 = 0;
        if (outX2) *outX2 = 0;
        if (outY2) *outY2 = 0;
        if (outNonzeroCount) *outNonzeroCount = 0;
        return 0;
    }
    if (outX1) *outX1 = minX;
    if (outY1) *outY1 = minY;
    if (outX2) *outX2 = (unsigned short)(maxX + 1);
    if (outY2) *outY2 = (unsigned short)(maxY + 1);
    if (outNonzeroCount) *outNonzeroCount = nonzero;
    return 1;
}

static void build_patch_bitmap(
    const unsigned char* sourceBitmap,
    unsigned short x1,
    unsigned short y1,
    unsigned short x2,
    unsigned short y2,
    unsigned char* patchStorage) {
    unsigned short width = (unsigned short)(x2 - x1);
    unsigned short height = (unsigned short)(y2 - y1);
    unsigned short bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
    unsigned short x, y;

    patchStorage[0] = (unsigned char)(width & 0xFF);
    patchStorage[1] = (unsigned char)((width >> 8) & 0xFF);
    patchStorage[2] = (unsigned char)(height & 0xFF);
    patchStorage[3] = (unsigned char)((height >> 8) & 0xFF);
    memset(patchStorage + 4, 0, (size_t)bytesPerRow * (size_t)height);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            unsigned char value = get_nibble(sourceBitmap, (unsigned short)(x1 + x), (unsigned short)(y1 + y));
            unsigned char* dst = patchStorage + 4 + ((unsigned long)y * (unsigned long)bytesPerRow) + (x >> 1);
            if (x & 1) {
                *dst = (unsigned char)((*dst & 0xF0) | value);
            } else {
                *dst = (unsigned char)((*dst & 0x0F) | (unsigned char)(value << 4));
            }
        }
    }
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    const char* scriptName;
    unsigned int dialogGraphicIndex = 1;
    unsigned int startFrame = 1;
    unsigned int endFrame = 0;
    const struct BootPlanScript_Compat* script;
    unsigned int i;

    if (argc < 3) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT script_name [start_frame] [end_frame] [dialog_graphic_index]\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    scriptName = argv[2];
    if (argc >= 4) startFrame = (unsigned int)strtoul(argv[3], 0, 10);
    if (argc >= 5) endFrame = (unsigned int)strtoul(argv[4], 0, 10);
    if (argc >= 6) dialogGraphicIndex = (unsigned int)strtoul(argv[5], 0, 10);

    script = F9010_RUNTIME_GetBootPlanScript_Compat(scriptName);
    if (script == 0) {
        fprintf(stderr, "failed: unknown script %s\n", scriptName);
        return 1;
    }
    if (endFrame == 0 || endFrame > script->stepCount) endFrame = script->stepCount;

    printf("ok\n");
    printf("scriptName=%s\n", script->name);
    printf("startFrame=%u\n", startFrame);
    printf("endFrame=%u\n", endFrame);
    printf("frame,graphic,viewportWidth,viewportHeight,compressedBytes,decompressedBytes,offset,txnGraphicsOpened,txnGraphicsClosed,txnUsedViewportBuffer,txnDrawFloorAndCeiling,img3CommandCount,img3RepeatedCount,img3ExplicitCount,img3CopyPrevCount,img3MaxRun,img3Kind0Pixels,img3Kind1Pixels,img3Kind2Pixels,img3Kind3Pixels,img3Kind4Pixels,img3Kind5Pixels,img3Kind6Pixels,img3Kind7Pixels,img3Kind4Minus6,img3Kind4Permille,img3Kind6Permille,img3Kind7Permille,img3Kind3Permille,img3Kind5Permille,img3Kind35Permille,img3Kind35Minus7Permille,img3Kind35To7MilliRatio,menuRequestedEventCount,menuDispatchedEventCount,menuFrameEventCount,menuAdvanceEventCount,menuActivateEventCount,menuIdleEventCount,menuAdvanceTransitionCount,menuFrameCount,menuInitialSelectionIndex,menuFinalSelectionIndex,menuHandledTickCommandCount,patchSignedMinusKind35m7,nonzeroX1,nonzeroY1,nonzeroX2,nonzeroY2,nonzeroCount,composedX,composedY,composedPixelCount,patchDiffPixels,patchValueSum,patchPositiveDelta,patchNegativeDelta,patchSignedDelta,patchLeftDelta,patchRightDelta,patchTopDelta,patchBottomDelta,patchTopBandDelta,patchMidBandDelta,patchBottomBandDelta,patchTopBandPos,patchTopBandNeg,patchMidBandPos,patchMidBandNeg,patchBottomBandPos,patchBottomBandNeg,patchZeroCount,patchHighCount,diffSadVisible,diffWinner,visibleWinner,visible304Sad,visible319Sad\n");

    for (i = startFrame; i <= endFrame; ++i) {
        unsigned int viewportGraphicIndex = script->steps[i - 1].viewportGraphicIndex;
        struct MemoryGraphicsDatState_Compat fileState;
        struct MemoryGraphicsDatStartupResult_Compat startup;
        unsigned int viewportBytes = 16384U;
        unsigned int screenBytes = 320U * 200U / 2U;
        unsigned char* viewportGraphicBuffer = (unsigned char*)calloc((size_t)viewportBytes, 1);
        unsigned char* viewportStorage = (unsigned char*)calloc((size_t)viewportBytes + 8192U, 1);
        unsigned char* screenStorage = (unsigned char*)calloc((size_t)screenBytes + 4U, 1);
        unsigned char* backdropStorage = (unsigned char*)calloc((size_t)screenBytes + 4U, 1);
        unsigned char* patchStorage = (unsigned char*)calloc(4U + 14U * 19U, 1);
        unsigned char* cand304Storage = (unsigned char*)calloc((size_t)4096, 1);
        unsigned char* cand319Storage = (unsigned char*)calloc((size_t)4096, 1);
        unsigned char* viewportBitmap;
        unsigned char* screenBitmap;
        unsigned char* backdropBitmap;
        unsigned char* patchBitmap;
        unsigned char* cand304Bitmap;
        unsigned char* cand319Bitmap;
        struct ScreenBitmapPresentResult_Compat presentResult;
        struct Img3CommandMetrics_Compat img3Metrics;
        struct MemoryGraphicsDatState_Compat menuFileState;
        struct MemoryGraphicsDatMenuStateResult_Compat menuState;
        enum MemoryGraphicsDatEvent_Compat menuEvents[4];
        unsigned short nonzeroX1, nonzeroY1, nonzeroX2, nonzeroY2;
        unsigned int menuRequestedEventCount;
        unsigned int menuDispatchedEventCount;
        unsigned int menuFrameEventCount;
        unsigned int menuAdvanceEventCount;
        unsigned int menuActivateEventCount;
        unsigned int menuIdleEventCount;
        unsigned int menuAdvanceTransitionCount;
        unsigned int menuFrameCount;
        unsigned int menuInitialSelectionIndex;
        unsigned int menuFinalSelectionIndex;
        unsigned int menuHandledTickCommandCount;
        unsigned long nonzeroCount;
        unsigned long patchDiffPixels;
        unsigned long patchValueSum;
        long img3Kind4Minus6;
        unsigned long img3Kind4Permille;
        unsigned long img3Kind6Permille;
        unsigned long img3Kind7Permille;
        unsigned long img3Kind3Permille;
        unsigned long img3Kind5Permille;
        unsigned long img3Kind3Plus5Permille;
        long img3Kind35Minus7Permille;
        unsigned long img3Kind35To7MilliRatio;
        long patchSignedMinusKind35m7;
        unsigned long patchHistogram[16];
        unsigned long patchPositiveDelta;
        unsigned long patchNegativeDelta;
        long patchSignedDelta;
        long patchLeftDelta;
        long patchRightDelta;
        long patchTopDelta;
        long patchBottomDelta;
        long patchTopBandDelta;
        long patchMidBandDelta;
        long patchBottomBandDelta;
        unsigned long patchTopBandPos;
        unsigned long patchTopBandNeg;
        unsigned long patchMidBandPos;
        unsigned long patchMidBandNeg;
        unsigned long patchBottomBandPos;
        unsigned long patchBottomBandNeg;
        unsigned long patchZeroCount;
        unsigned long patchHighCount;
        unsigned long diffSadVisible;
        unsigned long visible304Sad;
        unsigned long visible319Sad;
        const char* diffWinner;
        const char* visibleWinner;

        memset(&fileState, 0, sizeof(fileState));
        memset(&startup, 0, sizeof(startup));
        memset(&presentResult, 0, sizeof(presentResult));
        memset(&img3Metrics, 0, sizeof(img3Metrics));
        memset(&menuFileState, 0, sizeof(menuFileState));
        memset(&menuState, 0, sizeof(menuState));
        if ((viewportGraphicBuffer == 0) || (viewportStorage == 0) || (screenStorage == 0) || (backdropStorage == 0) || (patchStorage == 0) || (cand304Storage == 0) || (cand319Storage == 0)) {
            fprintf(stderr, "failed: allocation error at frame %u\n", i);
            return 1;
        }
        viewportBitmap = viewportStorage + 4;
        screenBitmap = screenStorage + 4;
        backdropBitmap = backdropStorage + 4;
        patchBitmap = patchStorage + 4;
        cand304Bitmap = cand304Storage + 4;
        cand319Bitmap = cand319Storage + 4;

        if (!F0479_MEMORY_StartupGraphicsChain_Compat(
                graphicsDatPath,
                &fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                &startup)) {
            fprintf(stderr, "failed: startup chain at frame %u\n", i);
            return 1;
        }

        F0427_DIALOG_DrawBackdrop_Compat(
            startup.specials.dialogBoxGraphic,
            backdropBitmap,
            &startup.runtimeState.widthHeight[dialogGraphicIndex]);
        F0427_DIALOG_DrawBackdrop_Compat(
            startup.specials.dialogBoxGraphic,
            screenBitmap,
            &startup.runtimeState.widthHeight[dialogGraphicIndex]);
        if (!F9005_SCREEN_OverlayBitmapOnScreen_Compat(viewportBitmap, screenBitmap, 0, &presentResult)) {
            fprintf(stderr, "failed: overlay at frame %u\n", i);
            return 1;
        }

        build_patch_bitmap(screenBitmap, 153, 90, 167, 109, patchStorage);
        build_patch_bitmap(viewportBitmap, 0, 0, 14, 19, cand319Storage);
        diffSadVisible = patch_sad_same_size(screenBitmap, backdropBitmap, 153, 90, 167, 109);

        /* load 304 and 319 candidate bitmaps directly */
        {
            struct MemoryGraphicsDatBitmapPathResult_Compat res304;
            struct MemoryGraphicsDatBitmapPathResult_Compat res319;
            unsigned char* raw304 = (unsigned char*)calloc(4096U, 1);
            unsigned char* raw319 = (unsigned char*)calloc(4096U, 1);
            if ((raw304 == 0) || (raw319 == 0)) return 1;
            memset(&res304, 0, sizeof(res304));
            memset(&res319, 0, sizeof(res319));
            if (F0489_MEMORY_LoadNativeBitmapByIndex_Compat(graphicsDatPath, &startup.runtimeState, &fileState, 304, raw304, cand304Bitmap, &res304) != cand304Bitmap) return 1;
            if (F0489_MEMORY_LoadNativeBitmapByIndex_Compat(graphicsDatPath, &startup.runtimeState, &fileState, 319, raw319, cand319Bitmap, &res319) != cand319Bitmap) return 1;
            free(raw304);
            free(raw319);
        }

        if (!analyze_img3_command_metrics(
                viewportGraphicBuffer,
                startup.viewportSelection.widthHeight.Width,
                startup.viewportSelection.widthHeight.Height,
                &img3Metrics)) {
            fprintf(stderr, "failed: img3 metrics at frame %u\n", i);
            return 1;
        }
        img3Kind4Minus6 = (long)img3Metrics.kindPixelCount[4] - (long)img3Metrics.kindPixelCount[6];
        img3Kind4Permille = (img3Metrics.totalPixels != 0) ? ((1000UL * img3Metrics.kindPixelCount[4]) / img3Metrics.totalPixels) : 0;
        img3Kind6Permille = (img3Metrics.totalPixels != 0) ? ((1000UL * img3Metrics.kindPixelCount[6]) / img3Metrics.totalPixels) : 0;
        img3Kind7Permille = (img3Metrics.totalPixels != 0) ? ((1000UL * img3Metrics.kindPixelCount[7]) / img3Metrics.totalPixels) : 0;
        img3Kind3Permille = (img3Metrics.totalPixels != 0) ? ((1000UL * img3Metrics.kindPixelCount[3]) / img3Metrics.totalPixels) : 0;
        img3Kind5Permille = (img3Metrics.totalPixels != 0) ? ((1000UL * img3Metrics.kindPixelCount[5]) / img3Metrics.totalPixels) : 0;
        img3Kind3Plus5Permille = (img3Metrics.totalPixels != 0) ? ((1000UL * (img3Metrics.kindPixelCount[3] + img3Metrics.kindPixelCount[5])) / img3Metrics.totalPixels) : 0;
        img3Kind35Minus7Permille = (long)img3Kind3Plus5Permille - (long)img3Kind7Permille;
        img3Kind35To7MilliRatio = (img3Metrics.kindPixelCount[7] != 0) ? ((1000UL * (img3Metrics.kindPixelCount[3] + img3Metrics.kindPixelCount[5])) / img3Metrics.kindPixelCount[7]) : 0;
        menuEvents[0] = MEMORY_GRAPHICS_DAT_EVENT_FRAME;
        menuEvents[1] = MEMORY_GRAPHICS_DAT_EVENT_ADVANCE;
        menuEvents[2] = MEMORY_GRAPHICS_DAT_EVENT_ADVANCE;
        menuEvents[3] = MEMORY_GRAPHICS_DAT_EVENT_FRAME;
        menuRequestedEventCount = 0;
        menuDispatchedEventCount = 0;
        menuFrameEventCount = 0;
        menuAdvanceEventCount = 0;
        menuActivateEventCount = 0;
        menuIdleEventCount = 0;
        menuAdvanceTransitionCount = 0;
        menuFrameCount = 0;
        menuInitialSelectionIndex = 0;
        menuFinalSelectionIndex = 0;
        menuHandledTickCommandCount = 0;
        if (F0479_MEMORY_RunMenuStateMini_Compat(
                graphicsDatPath,
                &menuFileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                menuEvents,
                4,
                0,
                3,
                &menuState)) {
            menuRequestedEventCount = menuState.dispatch.requestedEventCount;
            menuDispatchedEventCount = menuState.dispatch.dispatchedEventCount;
            menuFrameEventCount = menuState.dispatch.frameEventCount;
            menuAdvanceEventCount = menuState.dispatch.advanceEventCount;
            menuActivateEventCount = menuState.dispatch.activateEventCount;
            menuIdleEventCount = menuState.dispatch.idleEventCount;
            menuAdvanceTransitionCount = menuState.advanceTransitionCount;
            menuFrameCount = menuState.frameCount;
            menuInitialSelectionIndex = menuState.initialSelectionIndex;
            menuFinalSelectionIndex = menuState.finalSelectionIndex;
            menuHandledTickCommandCount = menuState.dispatch.inputQueue.typedQueue.handledTickCommandCount;
            F0479_MEMORY_FreeMenuStateMini_Compat(&menuState);
        }
        compute_nonzero_bbox(viewportBitmap, &nonzeroX1, &nonzeroY1, &nonzeroX2, &nonzeroY2, &nonzeroCount);
        patchDiffPixels = patch_diff_pixel_count_same_size(screenBitmap, backdropBitmap, 153, 90, 167, 109);
        patchValueSum = patch_value_sum_same_size(screenBitmap, 153, 90, 167, 109);
        patch_signed_delta_same_size(screenBitmap, backdropBitmap, 153, 90, 167, 109, &patchPositiveDelta, &patchNegativeDelta, &patchSignedDelta);
        patch_signed_delta_halves_same_size(screenBitmap, backdropBitmap, 153, 90, 167, 109, &patchLeftDelta, &patchRightDelta, &patchTopDelta, &patchBottomDelta);
        patch_signed_delta_row_bands_same_size(screenBitmap, backdropBitmap, 153, 90, 167, 109, &patchTopBandDelta, &patchMidBandDelta, &patchBottomBandDelta);
        patch_positive_negative_row_bands_same_size(screenBitmap, backdropBitmap, 153, 90, 167, 109, &patchTopBandPos, &patchTopBandNeg, &patchMidBandPos, &patchMidBandNeg, &patchBottomBandPos, &patchBottomBandNeg);
        patch_histogram_same_size(screenBitmap, 153, 90, 167, 109, patchHistogram);
        patchZeroCount = patchHistogram[0];
        patchHighCount = patchHistogram[12] + patchHistogram[13] + patchHistogram[14] + patchHistogram[15];
        patchSignedMinusKind35m7 = patchSignedDelta - img3Kind35Minus7Permille;
        visible304Sad = best_match_sad(patchBitmap, 14, 19, cand304Bitmap);
        visible319Sad = best_match_sad(patchBitmap, 14, 19, cand319Bitmap);
        diffWinner = (diffSadVisible == 0) ? "none" : "304-like";
        visibleWinner = (visible304Sad <= visible319Sad) ? "304" : "319";

        printf("%u,%u,%u,%u,%u,%u,%ld,%d,%d,%d,%d,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%ld,%lu,%lu,%lu,%lu,%lu,%lu,%ld,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%ld,%u,%u,%u,%u,%lu,%u,%u,%lu,%lu,%lu,%lu,%lu,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%s,%s,%lu,%lu\n",
               i,
               viewportGraphicIndex,
               (unsigned int)startup.viewportSelection.widthHeight.Width,
               (unsigned int)startup.viewportSelection.widthHeight.Height,
               (unsigned int)startup.viewportSelection.compressedByteCount,
               (unsigned int)startup.viewportSelection.decompressedByteCount,
               startup.viewportSelection.offset,
               startup.viewportTransaction.graphicsOpened,
               startup.viewportTransaction.graphicsClosed,
               startup.viewportTransaction.session.usedViewportBuffer,
               startup.viewportTransaction.session.drawFloorAndCeilingRequested,
               img3Metrics.commandCount,
               img3Metrics.repeatedCommandCount,
               img3Metrics.explicitColorCommandCount,
               img3Metrics.copyPreviousLineCommandCount,
               img3Metrics.maxRunLength,
               img3Metrics.kindPixelCount[0],
               img3Metrics.kindPixelCount[1],
               img3Metrics.kindPixelCount[2],
               img3Metrics.kindPixelCount[3],
               img3Metrics.kindPixelCount[4],
               img3Metrics.kindPixelCount[5],
               img3Metrics.kindPixelCount[6],
               img3Metrics.kindPixelCount[7],
               img3Kind4Minus6,
               img3Kind4Permille,
               img3Kind6Permille,
               img3Kind7Permille,
               img3Kind3Permille,
               img3Kind5Permille,
               img3Kind3Plus5Permille,
               img3Kind35Minus7Permille,
               img3Kind35To7MilliRatio,
               menuRequestedEventCount,
               menuDispatchedEventCount,
               menuFrameEventCount,
               menuAdvanceEventCount,
               menuActivateEventCount,
               menuIdleEventCount,
               menuAdvanceTransitionCount,
               menuFrameCount,
               menuInitialSelectionIndex,
               menuFinalSelectionIndex,
               menuHandledTickCommandCount,
               patchSignedMinusKind35m7,
               (unsigned int)nonzeroX1,
               (unsigned int)nonzeroY1,
               (unsigned int)nonzeroX2,
               (unsigned int)nonzeroY2,
               nonzeroCount,
               (unsigned int)presentResult.composedX,
               (unsigned int)presentResult.composedY,
               presentResult.composedPixelCount,
               patchDiffPixels,
               patchValueSum,
               patchPositiveDelta,
               patchNegativeDelta,
               patchSignedDelta,
               patchLeftDelta,
               patchRightDelta,
               patchTopDelta,
               patchBottomDelta,
               patchTopBandDelta,
               patchMidBandDelta,
               patchBottomBandDelta,
               patchTopBandPos,
               patchTopBandNeg,
               patchMidBandPos,
               patchMidBandNeg,
               patchBottomBandPos,
               patchBottomBandNeg,
               patchZeroCount,
               patchHighCount,
               diffSadVisible,
               diffWinner,
               visibleWinner,
               visible304Sad,
               visible319Sad);

        F0479_MEMORY_FreeStartupGraphicsChain_Compat(&startup);
        free(viewportGraphicBuffer);
        free(viewportStorage);
        free(screenStorage);
        free(backdropStorage);
        free(patchStorage);
        free(cand304Storage);
        free(cand319Storage);
    }
    return 0;
}
