#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_graphics_dat_header_pc34_compat.h"
#include "title_dat_loader_v1.h"
#include "title_frontend_v1.h"


static const char* kind_name(V1_TitleFrontendSourceEventKind kind) {
    switch (kind) {
        case V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS: return "PRESENTS";
        case V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT: return "ZOOM_BLIT";
        case V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK: return "POST_ZOOM_VBLANK";
        case V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT: return "MASTER_STRIKES_BACK_BLIT";
        case V1_TITLE_FRONTEND_SOURCE_EVENT_FINAL_GUARD_VBLANK: return "FINAL_GUARD_VBLANK";
        case V1_TITLE_FRONTEND_SOURCE_EVENT_MENU_ELIGIBLE: return "MENU_ELIGIBLE";
    }
    return "UNKNOWN";
}

struct IndexRange {
    const char* name;
    unsigned int first;
    unsigned int last;
};

static const struct IndexRange kMenuRanges[] = {
    {"boot_script_menu_candidate_a", 304u, 319u},
    {"boot_script_menu_candidate_b_left", 360u, 367u},
    {"boot_script_menu_candidate_b_core", 368u, 383u},
    {"boot_script_menu_candidate_b_right", 384u, 391u},
    {"boot_script_menu_candidate_b_wide", 360u, 391u}
};

static int check_title_manifest(const char* titlePath) {
    V1_TitleManifest manifest;
    char err[256];
    unsigned int palette1 = 0u;
    unsigned int palette2 = 0u;
    unsigned int durationSum = 0u;
    unsigned int i;
    int ok = 1;

    memset(&manifest, 0, sizeof(manifest));
    memset(err, 0, sizeof(err));
    if (!V1_Title_ParseManifest(titlePath, &manifest, err, sizeof(err))) {
        printf("titleManifestOk=0\n");
        printf("titleManifestError=%s\n", err[0] ? err : "unknown");
        return 0;
    }
    for (i = 0; i < manifest.itemCount; ++i) {
        const V1_TitleRecord* r = &manifest.records[i];
        if (r->frameOrdinal != 0u) {
            if (r->paletteOrdinal == 1u) palette1++;
            if (r->paletteOrdinal == 2u) palette2++;
            durationSum += 1u;
        }
    }
    printf("titleManifestOk=1\n");
    printf("titleFileBytes=%lu\n", manifest.fileBytes);
    printf("titleItemCount=%u\n", manifest.itemCount);
    printf("titleFrameCount=%u\n", manifest.frameCount);
    printf("titleEncodedImageCount=%u\n", manifest.encodedImageCount);
    printf("titleDeltaLayerCount=%u\n", manifest.deltaLayerCount);
    printf("titlePaletteBreakFrames=%u+%u\n", palette1, palette2);
    printf("titleSourceRecordDurationSum=%u\n", durationSum);
    if (manifest.itemCount != V1_TITLE_DAT_ITEM_COUNT) ok = 0;
    if (manifest.frameCount != V1_TITLE_DAT_FRAME_MAX) ok = 0;
    if (palette1 != 37u || palette2 != 16u) ok = 0;
    printf("titleManifestInvariantOk=%d\n", ok);
    return ok;
}


static int check_source_animation_schedule(void) {
    V1_TitleFrontendSourceAnimationStep firstZoom;
    V1_TitleFrontendSourceAnimationStep lastZoom;
    V1_TitleFrontendSourceAnimationStep master;
    V1_TitleFrontendSourceAnimationStep finalGuard;
    unsigned int i;
    unsigned int zoomCount = 0u;
    unsigned int postWaitCount = 0u;
    unsigned int vblankZoomCount = 0u;
    int ok = 1;

    memset(&firstZoom, 0, sizeof(firstZoom));
    memset(&lastZoom, 0, sizeof(lastZoom));
    memset(&master, 0, sizeof(master));
    memset(&finalGuard, 0, sizeof(finalGuard));
    printf("sourceAnimationStepCount=%u\n", V1_TitleFrontend_GetSourceAnimationStepCount());
    for (i = 1u; i <= V1_TitleFrontend_GetSourceAnimationStepCount(); ++i) {
        V1_TitleFrontendSourceAnimationStep step;
        if (!V1_TitleFrontend_GetSourceAnimationStep(i, &step)) {
            ok = 0;
            continue;
        }
        printf("sourceAnimationStep[%u]=kind:%s vblank:%u zoomIndex:%u box:%u,%u,%u,%u evidence:%s\n",
               i,
               kind_name(step.kind),
               step.vblankBeforeEvent,
               step.zoomSourceIndex,
               step.x,
               step.y,
               step.width,
               step.height,
               step.sourceLineEvidence ? step.sourceLineEvidence : "");
        if (step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT) {
            zoomCount++;
            if (step.vblankBeforeEvent == 1u) vblankZoomCount++;
            if (zoomCount == 1u) firstZoom = step;
            lastZoom = step;
        } else if (step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK) {
            postWaitCount++;
        } else if (step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT) {
            master = step;
        } else if (step.kind == V1_TITLE_FRONTEND_SOURCE_EVENT_FINAL_GUARD_VBLANK) {
            finalGuard = step;
        }
    }
    if (V1_TitleFrontend_GetSourceAnimationStepCount() != 23u) ok = 0;
    if (zoomCount != 18u || vblankZoomCount != 18u) ok = 0;
    if (postWaitCount != 2u) ok = 0;
    if (firstZoom.zoomSourceIndex != 17u || firstZoom.width != 48u || firstZoom.height != 12u || firstZoom.x != 136u || firstZoom.y != 74u) ok = 0;
    if (lastZoom.zoomSourceIndex != 0u || lastZoom.width != 320u || lastZoom.height != 80u || lastZoom.x != 0u || lastZoom.y != 40u) ok = 0;
    if (master.kind != V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT || master.y != 118u || master.height != 57u) ok = 0;
    if (finalGuard.kind != V1_TITLE_FRONTEND_SOURCE_EVENT_FINAL_GUARD_VBLANK || finalGuard.vblankBeforeEvent != 1u) ok = 0;
    printf("titleSourceAnimationScheduleInvariantOk=%d\n", ok);
    return ok;
}

static int check_title_handoff(void) {
    V1_TitleFrontendSourceTiming timing = V1_TitleFrontend_GetSourceTimingEvidence();
    unsigned int steps[] = {0u, 1u, 52u, 53u, 54u, 106u};
    unsigned int i;
    int ok = 1;
    printf("sourceTimingFile=%s\n", timing.sourceFile);
    printf("sourceTimingFunction=%s\n", timing.sourceFunction);
    printf("sourceZoomStepCount=%u\n", timing.zoomStepCount);
    printf("sourceVblankBeforeEachZoomStep=%u\n", timing.vblankBeforeEachZoomStep);
    printf("sourcePostZoomVblankCount=%u\n", timing.postZoomVblankCount);
    printf("sourceFinalFadeGuardVblankCount=%u\n", timing.finalFadeGuardVblankCount);
    printf("sourceFirstMenuEligibleStep=%u\n", timing.firstMenuEligibleStep);
    printf("sourceTimingAnimationStepCount=%u\n", timing.sourceAnimationStepCount);
    printf("sourceTimingEvidence=%s\n", timing.evidenceNote);
    if (timing.zoomStepCount != 18u) ok = 0;
    if (timing.vblankBeforeEachZoomStep != 1u) ok = 0;
    if (timing.postZoomVblankCount != 2u) ok = 0;
    if (timing.finalFadeGuardVblankCount != 1u) ok = 0;
    if (timing.firstMenuEligibleStep != 54u) ok = 0;
    if (timing.sourceAnimationStepCount != 23u) ok = 0;
    for (i = 0; i < sizeof(steps) / sizeof(steps[0]); ++i) {
        V1_TitleFrontendHandoffDecision hold = V1_TitleFrontend_DecideTitleMenuHandoffStep(steps[i], 0);
        V1_TitleFrontendHandoffDecision enter = V1_TitleFrontend_DecideTitleMenuHandoffStep(steps[i], 1);
        printf("handoffStep[%u]=%u holdSurface=%s holdFrame=%u holdReady=%d enterSurface=%s enterFrame=%u enterReady=%d enteredMenu=%d\n",
               i,
               steps[i],
               hold.surface == V1_TITLE_FRONTEND_SURFACE_MENU ? "MENU" : "TITLE",
               hold.title.renderFrameOrdinal,
               hold.title.handoffReady,
               enter.surface == V1_TITLE_FRONTEND_SURFACE_MENU ? "MENU" : "TITLE",
               enter.title.renderFrameOrdinal,
               enter.title.handoffReady,
               enter.enteredMenuAfterHandoff);
    }
    if (V1_TitleFrontend_DecideTitleMenuHandoffStep(53u, 1).surface != V1_TITLE_FRONTEND_SURFACE_TITLE) ok = 0;
    if (V1_TitleFrontend_DecideTitleMenuHandoffStep(54u, 1).surface != V1_TITLE_FRONTEND_SURFACE_MENU) ok = 0;
    if (V1_TitleFrontend_DecideTitleMenuHandoffStep(54u, 1).title.renderFrameOrdinal != V1_TITLE_DAT_FRAME_MAX) ok = 0;
    if (V1_TitleFrontend_DecideTitleMenuHandoffStep(54u, 0).surface != V1_TITLE_FRONTEND_SURFACE_TITLE) ok = 0;
    printf("titleSourceTimingInvariantOk=%d\n", ok);
    printf("titleHandoffInvariantOk=%d\n", ok);
    return ok;
}

static void print_range_metrics(const char* name,
                                unsigned int first,
                                unsigned int last,
                                const struct MemoryGraphicsDatHeader_Compat* header) {
    unsigned int i;
    unsigned int sumW = 0u;
    unsigned int maxW = 0u;
    unsigned int maxH = 0u;
    unsigned int zeroCount = 0u;

    if (last >= header->graphicCount) {
        printf("menuRange[%s].available=0\n", name);
        return;
    }
    for (i = first; i <= last; ++i) {
        unsigned int w = header->widthHeight[i].Width;
        unsigned int h = header->widthHeight[i].Height;
        if (w == 0u || h == 0u) zeroCount++;
        sumW += w;
        if (w > maxW) maxW = w;
        if (h > maxH) maxH = h;
    }
    printf("menuRange[%s].available=1\n", name);
    printf("menuRange[%s].first=%u\n", name, first);
    printf("menuRange[%s].last=%u\n", name, last);
    printf("menuRange[%s].count=%u\n", name, last - first + 1u);
    printf("menuRange[%s].serialWidthSum=%u\n", name, sumW);
    printf("menuRange[%s].maxWidth=%u\n", name, maxW);
    printf("menuRange[%s].maxHeight=%u\n", name, maxH);
    printf("menuRange[%s].zeroSizedCount=%u\n", name, zeroCount);
}

static int check_menu_metrics(const char* graphicsPath) {
    struct MemoryGraphicsDatState_Compat fileState;
    struct MemoryGraphicsDatHeader_Compat header;
    unsigned int r;
    int ok = 1;

    memset(&fileState, 0, sizeof(fileState));
    memset(&header, 0, sizeof(header));
    if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(graphicsPath, &fileState, &header)) {
        printf("graphicsHeaderOk=0\n");
        return 0;
    }
    printf("graphicsHeaderOk=1\n");
    printf("graphicsDatPath=%s\n", graphicsPath);
    printf("graphicsCount=%u\n", header.graphicCount);
    for (r = 0; r < sizeof(kMenuRanges) / sizeof(kMenuRanges[0]); ++r) {
        print_range_metrics(kMenuRanges[r].name, kMenuRanges[r].first, kMenuRanges[r].last, &header);
    }
    if (header.graphicCount <= 391u) ok = 0;
    if (header.widthHeight[372].Width != 96u || header.widthHeight[372].Height != 111u) ok = 0;
    if (header.widthHeight[378].Width != 160u || header.widthHeight[378].Height != 111u) ok = 0;
    printf("menuMetricInvariantOk=%d\n", ok);
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    return ok;
}

int main(int argc, char** argv) {
    const char* graphicsPath;
    const char* titlePath;
    int ok;

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT [/path/to/TITLE]\n", argv[0]);
        return 2;
    }
    graphicsPath = argv[1];
    titlePath = (argc >= 3) ? argv[2] : 0;

    printf("probe=firestaff_v1_title_menu_cadence_layout\n");
    printf("scope=bounded ReDMCSB TITLE.C source-locked timing plus runtime handoff evidence\n");
    ok = check_menu_metrics(graphicsPath);
    if (titlePath) {
        printf("titleDatPath=%s\n", titlePath);
        ok = check_title_manifest(titlePath) && ok;
    } else {
        printf("titleDatPath=(not provided)\n");
    }
    ok = check_source_animation_schedule() && ok;
    ok = check_title_handoff() && ok;
    printf("originalCadenceClaim=source-locked-pc-st-title-c\n");
    printf("blocker=per-presented-frame emulator dump still needed for pixel/video capture comparison, but TITLE.C control-flow cadence is no longer blocked\n");
    return ok ? 0 : 1;
}
