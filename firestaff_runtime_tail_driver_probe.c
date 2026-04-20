#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boot_plan_script_pc34_compat.h"
#include "startup_runtime_driver_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

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
    printf("frame,graphic,driverFramePrepared,driverFramePublished,driverFrameSkippedByDispatcher,entryKind,entryShouldUseBitmapPath,entryShouldSkipBitmapExport,startupTickStage,startupTickReady,startupTickCompleted,mainLoopStage,mainLoopReady,mainLoopEntered,dispatchStage,startupDispatchReady,startupDispatched,firstFrameStage,dispatchReady,firstFrameAttempted,startupWiringRuntimeAssetsReady,startupWiringPersistentSpecialsReady,startupWiringViewportPathReady,startupWiringPrerequisitesReady,bootStage,bootSucceeded\n");

    for (i = startFrame; i <= endFrame; ++i) {
        unsigned int viewportGraphicIndex = script->steps[i - 1].viewportGraphicIndex;
        struct StartupRuntimeDriverResult_Compat result;
        char outputPath[512];
        memset(&result, 0, sizeof(result));
        snprintf(outputPath, sizeof(outputPath), "./exports_driver_probe_%s", script->name);
        if (!F9003_RUNTIME_RunStartupFrameDriver_Compat(
                graphicsDatPath,
                outputPath,
                dialogGraphicIndex,
                viewportGraphicIndex,
                i,
                &result)) {
            fprintf(stderr, "failed: driver probe at frame %u\n", i);
            return 1;
        }
        printf("%u,%u,%d,%d,%d,%u,%d,%d,%u,%d,%d,%u,%d,%d,%u,%d,%d,%u,%d,%d,%d,%d,%d,%d,%u,%d\n",
               i,
               viewportGraphicIndex,
               result.framePrepared,
               result.framePublished,
               result.frameSkippedByDispatcher,
               (unsigned int)result.entryClassification.kind,
               result.entryClassification.shouldUseBitmapPath,
               result.entryClassification.shouldSkipBitmapExport,
               (unsigned int)result.startupTick.stage,
               result.startupTick.startupTickReady,
               result.startupTick.startupTickCompleted,
               (unsigned int)result.startupTick.mainLoop.stage,
               result.startupTick.mainLoop.mainLoopReady,
               result.startupTick.mainLoop.mainLoopEntered,
               (unsigned int)result.startupTick.mainLoop.dispatch.stage,
               result.startupTick.mainLoop.dispatch.startupDispatchReady,
               result.startupTick.mainLoop.dispatch.startupDispatched,
               (unsigned int)result.startupTick.mainLoop.dispatch.firstFrame.stage,
               result.startupTick.mainLoop.dispatch.firstFrame.dispatchReady,
               result.startupTick.mainLoop.dispatch.firstFrame.firstFrameAttempted,
               result.startupTick.mainLoop.dispatch.firstFrame.startup.runtimeAssetsReady,
               result.startupTick.mainLoop.dispatch.firstFrame.startup.persistentSpecialsReady,
               result.startupTick.mainLoop.dispatch.firstFrame.startup.viewportPathReady,
               result.startupTick.mainLoop.dispatch.firstFrame.startup.firstFramePrerequisitesReady,
               (unsigned int)result.startupTick.mainLoop.dispatch.firstFrame.startup.boot.stage,
               result.startupTick.mainLoop.dispatch.firstFrame.startup.boot.succeeded);
        F9003_RUNTIME_FreeStartupFrameDriver_Compat(&result);
    }
    return 0;
}
