#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <stdlib.h>
#include <string.h>
#include "startup_runtime_driver_pc34_compat.h"
#include "screen_bitmap_present_pc34_compat.h"
#include "dialog_frontend_pc34_compat.h"

void F9003_RUNTIME_FreeStartupFrameDriver_Compat(
struct StartupRuntimeDriverResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeStartupTickMini_Compat(&result->startupTick);
        memset(result, 0, sizeof(*result));
}

int F9003_RUNTIME_RunStartupFrameDriver_Compat(
const char*                               graphicsDatPath      SEPARATOR
const char*                               outputPath           SEPARATOR
unsigned int                              dialogGraphicIndex   SEPARATOR
unsigned int                              viewportGraphicIndex SEPARATOR
unsigned int                              frameNumber          SEPARATOR
struct StartupRuntimeDriverResult_Compat* outResult           FINAL_SEPARATOR
{
        struct MemoryGraphicsDatStartupResult_Compat* startup;
        unsigned int viewportBytes;
        unsigned int screenBytes;
        unsigned char* viewportGraphicBuffer;
        unsigned char* viewportStorage;
        unsigned char* viewportBitmap;
        unsigned char* screenStorage;
        unsigned char* screenBitmap;
        struct ScreenBitmapPresentResult_Compat presentResult;
        struct MemoryGraphicsDatState_Compat fileState;


        memset(outResult, 0, sizeof(*outResult));
        memset(&fileState, 0, sizeof(fileState));
        memset(&presentResult, 0, sizeof(presentResult));
        viewportBytes = 16384U;
        screenBytes = 320U * 200U / 2U;
        viewportGraphicBuffer = (unsigned char*)calloc((size_t)viewportBytes, 1);
        viewportStorage = (unsigned char*)calloc((size_t)viewportBytes + 8192U, 1);
        screenStorage = (unsigned char*)calloc((size_t)screenBytes + 4U, 1);
        if ((viewportGraphicBuffer == 0) || (viewportStorage == 0) || (screenStorage == 0)) {
                free(viewportGraphicBuffer);
                free(viewportStorage);
                free(screenStorage);
                return 0;
        }
        viewportBitmap = viewportStorage + 4;
        screenBitmap = screenStorage + 4;
        if (!F0479_MEMORY_RunStartupTickMini_Compat(
                graphicsDatPath,
                &fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                &outResult->startupTick)) {
                free(viewportGraphicBuffer);
                free(viewportStorage);
                free(screenStorage);
                return 0;
        }
        startup = &outResult->startupTick.mainLoop.dispatch.firstFrame.startup.boot.startup;
        if (!F9012_RUNTIME_ClassifyGraphicsDatEntry_Compat(&startup->runtimeState, viewportGraphicIndex, &outResult->entryClassification)) {
                free(viewportGraphicBuffer);
                free(viewportStorage);
                free(screenStorage);
                F9003_RUNTIME_FreeStartupFrameDriver_Compat(outResult);
                return 0;
        }
        if (outResult->entryClassification.shouldSkipBitmapExport) {
                free(viewportGraphicBuffer);
                free(viewportStorage);
                free(screenStorage);
                outResult->frameSkippedByDispatcher = 1;
                return 1;
        }
        if (!F0490_MEMORY_LoadViewportGraphicByIndex_Compat(
                graphicsDatPath,
                &startup->runtimeState,
                &fileState,
                viewportGraphicIndex,
                0,
                viewportGraphicBuffer,
                viewportBitmap,
                &startup->viewportTransaction,
                &startup->viewportSelection)) {
                free(viewportGraphicBuffer);
                free(viewportStorage);
                free(screenStorage);
                F9003_RUNTIME_FreeStartupFrameDriver_Compat(outResult);
                return 0;
        }
        F0427_DIALOG_DrawBackdrop_Compat(
                startup->specials.dialogBoxGraphic,
                screenBitmap,
                &startup->runtimeState.widthHeight[dialogGraphicIndex]);
        if (!F9006_SCREEN_OverlayViewportBitmapOnScreen_Compat(viewportBitmap, screenBitmap, 0, &presentResult)) {
                free(viewportGraphicBuffer);
                free(viewportStorage);
                free(screenStorage);
                F9003_RUNTIME_FreeStartupFrameDriver_Compat(outResult);
                return 0;
        }
        presentResult.sourceBitmap = 0;
        presentResult.screenBitmap = 0;
        outResult->present = presentResult;
        outResult->framePrepared = 1;
        if (!F9002_HOSTVIDEO_PublishFrameToPgm_Compat(screenBitmap, frameNumber, outputPath, &outResult->hostFrame)) {
                free(viewportGraphicBuffer);
                free(viewportStorage);
                free(screenStorage);
                F9003_RUNTIME_FreeStartupFrameDriver_Compat(outResult);
                return 0;
        }
        outResult->framePublished = 1;
        free(viewportGraphicBuffer);
        free(viewportStorage);
        free(screenStorage);
        return 1;
}
