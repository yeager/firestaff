#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boot_plan_script_pc34_compat.h"
#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_graphics_dat_startup_pc34_compat.h"
#include "memory_graphics_dat_viewport_path_pc34_compat.h"
#include "memory_graphics_dat_menu_activate_consequence_pc34_compat.h"
#include "dialog_frontend_pc34_compat.h"
#include "screen_bitmap_present_pc34_compat.h"
#include "host_video_pgm_backend_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

struct ProbeRuntime {
    struct MemoryGraphicsDatState_Compat fileState;
    struct MemoryGraphicsDatStartupResult_Compat startup;
    unsigned char* viewportGraphicBuffer;
    unsigned char* viewportStorage;
    unsigned char* viewportBitmap;
    unsigned char* screenStorage;
    unsigned char* screenBitmap;
    unsigned int viewportBytes;
    unsigned int screenBytes;
    int initialized;
};

static void free_runtime(struct ProbeRuntime* runtime) {
    if (runtime->startup.runtimeInitialized || runtime->initialized) {
        F0479_MEMORY_FreeStartupGraphicsChain_Compat(&runtime->startup);
    }
    free(runtime->viewportGraphicBuffer);
    free(runtime->viewportStorage);
    free(runtime->screenStorage);
    memset(runtime, 0, sizeof(*runtime));
}

static int initialize_runtime(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatHeader_Compat* header,
    unsigned int dialogGraphicIndex,
    unsigned int initialViewportGraphicIndex,
    struct ProbeRuntime* runtime) {
    unsigned int i;
    unsigned int maxCompressed = 16384U;
    unsigned int maxBitmapBytes = 8192U;

    memset(runtime, 0, sizeof(*runtime));
    if ((header != 0) && (header->graphicCount > 0)) {
        for (i = 0; i < header->graphicCount; ++i) {
            unsigned int width = header->widthHeight[i].Width;
            unsigned int height = header->widthHeight[i].Height;
            unsigned int bitmapBytes = (((width + 1U) & 0xFFFEU) >> 1U) * height;
            if (header->compressedByteCounts[i] > maxCompressed) {
                maxCompressed = header->compressedByteCounts[i];
            }
            if (bitmapBytes > maxBitmapBytes) {
                maxBitmapBytes = bitmapBytes;
            }
        }
    }
    runtime->viewportBytes = maxCompressed + 64U;
    runtime->screenBytes = 320U * 200U / 2U;
    runtime->viewportGraphicBuffer = (unsigned char*)calloc((size_t)runtime->viewportBytes, 1);
    runtime->viewportStorage = (unsigned char*)calloc((size_t)maxBitmapBytes + 64U, 1);
    runtime->screenStorage = (unsigned char*)calloc((size_t)runtime->screenBytes + 4U, 1);
    if ((runtime->viewportGraphicBuffer == 0) || (runtime->viewportStorage == 0) || (runtime->screenStorage == 0)) {
        free_runtime(runtime);
        return 0;
    }
    runtime->viewportBitmap = runtime->viewportStorage + 4;
    runtime->screenBitmap = runtime->screenStorage + 4;
    if (!F0479_MEMORY_StartupGraphicsChain_Compat(
            graphicsDatPath,
            &runtime->fileState,
            dialogGraphicIndex,
            initialViewportGraphicIndex,
            runtime->viewportGraphicBuffer,
            runtime->viewportBitmap,
            &runtime->startup)) {
        free_runtime(runtime);
        return 0;
    }
    runtime->initialized = 1;
    return 1;
}

static int run_menu_probe(
    const char* graphicsDatPath,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    struct MemoryGraphicsDatHeader_Compat* header,
    struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat* outResult) {
    struct MemoryGraphicsDatState_Compat fileState;
    enum MemoryGraphicsDatEvent_Compat events[5] = {
        MEMORY_GRAPHICS_DAT_EVENT_FRAME,
        MEMORY_GRAPHICS_DAT_EVENT_ADVANCE,
        MEMORY_GRAPHICS_DAT_EVENT_ADVANCE,
        MEMORY_GRAPHICS_DAT_EVENT_ACTIVATE,
        MEMORY_GRAPHICS_DAT_EVENT_FRAME
    };
    unsigned int viewportBytes;
    unsigned int bitmapBytes = 8192U;
    unsigned int i;
    unsigned char* viewportGraphicBuffer;
    unsigned char* viewportBitmap;
    int ok;

    memset(&fileState, 0, sizeof(fileState));
    memset(outResult, 0, sizeof(*outResult));
    viewportBytes = header->compressedByteCounts[viewportGraphicIndex];
    if (viewportBytes < 16U) viewportBytes = 16U;
    if ((header != 0) && (header->graphicCount > 0)) {
        for (i = 0; i < header->graphicCount; ++i) {
            unsigned int width = header->widthHeight[i].Width;
            unsigned int height = header->widthHeight[i].Height;
            unsigned int candidate = (((width + 1U) & 0xFFFEU) >> 1U) * height;
            if (candidate > bitmapBytes) {
                bitmapBytes = candidate;
            }
        }
    }
    viewportGraphicBuffer = (unsigned char*)calloc((size_t)viewportBytes + 64U, 1);
    viewportBitmap = (unsigned char*)calloc((size_t)bitmapBytes + 256U, 1);
    if ((viewportGraphicBuffer == 0) || (viewportBitmap == 0)) {
        free(viewportGraphicBuffer);
        free(viewportBitmap);
        return 0;
    }
    printf("menuProbeCallStart=1\n");
    ok = F0479_MEMORY_RunMenuActivateConsequenceMini_Compat(
        graphicsDatPath,
        &fileState,
        dialogGraphicIndex,
        viewportGraphicIndex,
        viewportGraphicBuffer,
        viewportBitmap,
        events,
        5,
        0,
        3,
        dialogGraphicIndex,
        dialogGraphicIndex + 10U,
        MEMORY_GRAPHICS_DAT_MENU_SCREEN_MENU,
        outResult);
    printf("menuProbeCallReturned=%d\n", ok);
    (void)viewportGraphicBuffer;
    (void)viewportBitmap;
    return ok;
}

static int render_graphic_with_runtime(
    const char* graphicsDatPath,
    const char* outputPath,
    unsigned int dialogGraphicIndex,
    unsigned int graphicIndex,
    unsigned int frameNumber,
    struct ProbeRuntime* runtime) {
    struct ScreenBitmapPresentResult_Compat presentResult;
    struct HostVideoPgmBackendResult_Compat hostResult;
    struct MemoryGraphicsDatTransactionResult_Compat txResult;
    struct MemoryGraphicsDatSelection_Compat selection;
    const struct GraphicWidthHeight_Compat* sizeInfo;

    memset(&presentResult, 0, sizeof(presentResult));
    memset(&hostResult, 0, sizeof(hostResult));
    memset(&txResult, 0, sizeof(txResult));
    memset(&selection, 0, sizeof(selection));
    memset(runtime->viewportStorage, 0, runtime->viewportBytes + 64U);
    memset(runtime->screenStorage, 0, runtime->screenBytes + 4U);
    if (!F0490_MEMORY_LoadViewportGraphicByIndex_Compat(
            graphicsDatPath,
            &runtime->startup.runtimeState,
            &runtime->fileState,
            graphicIndex,
            0,
            runtime->viewportGraphicBuffer,
            runtime->viewportBitmap,
            &txResult,
            &selection)) {
        return 0;
    }
    sizeInfo = &runtime->startup.runtimeState.widthHeight[dialogGraphicIndex];
    F0427_DIALOG_DrawBackdrop_Compat(runtime->startup.specials.dialogBoxGraphic, runtime->screenBitmap, sizeInfo);
    if (!F9005_SCREEN_OverlayBitmapOnScreen_Compat(runtime->viewportBitmap, runtime->screenBitmap, 0, &presentResult)) {
        return 0;
    }
    return F9002_HOSTVIDEO_PublishFrameToPgm_Compat(runtime->screenBitmap, frameNumber, outputPath, &hostResult);
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    const char* outputPrefix;
    const char* scriptName = "m7_reachability_b";
    unsigned int bootStepCount;
    unsigned int eventGraphicIndex = 1;
    unsigned int dialogGraphicIndex = 1;
    int skipEventRender = 0;
    int skipRuntimeInit = 0;
    unsigned int i;
    struct MemoryGraphicsDatState_Compat fileState;
    struct MemoryGraphicsDatHeader_Compat header;
    const struct BootPlanScript_Compat* script;
    struct ProbeRuntime runtime;
    char outputPath[1024];

    setbuf(stdout, 0);
    setbuf(stderr, 0);
    if (argc < 4) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT /path/to/output_prefix boot_step_count [event_graphic_index]\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    outputPrefix = argv[2];
    bootStepCount = (unsigned int)strtoul(argv[3], 0, 10);
    if (argc >= 5) {
        eventGraphicIndex = (unsigned int)strtoul(argv[4], 0, 10);
    }
    if (argc >= 6) {
        skipEventRender = (int)strtol(argv[5], 0, 10);
    }
    if (argc >= 7) {
        skipRuntimeInit = (int)strtol(argv[6], 0, 10);
    }

    memset(&fileState, 0, sizeof(fileState));
    memset(&header, 0, sizeof(header));
    memset(&runtime, 0, sizeof(runtime));
    if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(graphicsDatPath, &fileState, &header)) {
        fprintf(stderr, "failed: header load\n");
        return 1;
    }
    script = F9010_RUNTIME_GetBootPlanScript_Compat(scriptName);
    if (script == 0) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        fprintf(stderr, "failed: script lookup\n");
        return 1;
    }
    if (bootStepCount > script->stepCount) {
        bootStepCount = script->stepCount;
    }
    if (!skipRuntimeInit) {
        if (!initialize_runtime(graphicsDatPath, &header, dialogGraphicIndex, script->steps[0].viewportGraphicIndex, &runtime)) {
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: runtime init\n");
            return 1;
        }
        printf("runtimeInitOk=1\n");
    } else {
        printf("runtimeInitSkipped=1\n");
    }

    printf("ok\n");
    printf("scriptName=%s\n", scriptName);
    printf("bootStepCount=%u\n", bootStepCount);
    printf("eventGraphicIndex=%u\n", eventGraphicIndex);

    for (i = 0; i < bootStepCount; ++i) {
        snprintf(outputPath, sizeof(outputPath), "%s_boot_%04u.pgm", outputPrefix, i + 1U);
        if (skipRuntimeInit || !render_graphic_with_runtime(
                graphicsDatPath,
                outputPath,
                dialogGraphicIndex,
                script->steps[i].viewportGraphicIndex,
                i + 1U,
                &runtime)) {
            if (!skipRuntimeInit) {
                free_runtime(&runtime);
            }
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: boot render at step %u graphic %u\n", i, script->steps[i].viewportGraphicIndex);
            return 1;
        }
    }

    if (!skipEventRender) {
        if (skipRuntimeInit) {
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: event render requested without runtime init\n");
            return 1;
        }
        memset(&runtime.fileState, 0, sizeof(runtime.fileState));
        snprintf(outputPath, sizeof(outputPath), "%s_event_probe_%04u.pgm", outputPrefix, bootStepCount + 1U);
        if (!render_graphic_with_runtime(
                graphicsDatPath,
                outputPath,
                dialogGraphicIndex,
                eventGraphicIndex,
                bootStepCount + 1U,
                &runtime)) {
            free_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: event render after boot\n");
            return 1;
        }
        printf("eventRenderOk=1\n");
    } else {
        printf("eventRenderSkipped=1\n");
    }
    {
        struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat menuResult;
        memset(&menuResult, 0, sizeof(menuResult));
        if (!run_menu_probe(graphicsDatPath, dialogGraphicIndex, 0U, &header, &menuResult)) {
            if (!skipRuntimeInit) {
                free_runtime(&runtime);
            }
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: menu probe after boot\n");
            return 1;
        }
        printf("menuProbeOk=1\n");
        printf("menuFinalSelectionIndex=%u\n", menuResult.activate.render.menuState.finalSelectionIndex);
        printf("menuActivatedGraphicIndex=%u\n", menuResult.activate.activatedGraphicIndex);
        printf("menuFinalScreen=%u\n", (unsigned int)menuResult.finalScreen);
        F0479_MEMORY_FreeMenuActivateConsequenceMini_Compat(&menuResult);
    }
    if (!skipRuntimeInit) {
        free_runtime(&runtime);
    }
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    return 0;
}
