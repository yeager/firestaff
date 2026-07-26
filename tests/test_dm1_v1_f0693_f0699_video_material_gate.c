#include "asset_loader_m11.h"
#include "dm1_v1_f0693_f0699_video_material_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TestHost {
    int vectorRequests;
    int initialized;
    int inversions;
} TestHost;

static int s_vblank_deliveries;
static TestHost* s_initialization_host;

static void initialize_unused_globals(char** first, char** second)
{
    if (s_initialization_host) ++s_initialization_host->initialized;
    if (first) *first = NULL;
    if (second) *second = NULL;
}

static const ReDMCSBF0699VideoDriverPc34Compat* get_vector(
    unsigned int interruptNumber, void* context)
{
    static const ReDMCSBF0699VideoDriverPc34Compat driver = {
        initialize_unused_globals
    };
    TestHost* host = (TestHost*)context;

    if (!host || interruptNumber != REDMCSB_F0699_DM_VIDEO_INTERRUPT_PC34) {
        return NULL;
    }
    ++host->vectorRequests;
    return &driver;
}

static void invert_box(void* context, int16_t left, int16_t right,
                       int16_t top, int16_t bottom)
{
    TestHost* host = (TestHost*)context;

    if (host && left == 0 && right == 319 && top == 0 && bottom == 199) {
        ++host->inversions;
    }
}

static void deliver_vertical_blank(void* context)
{
    ReDMCSBF0693WaitVerticalBlankPc34Compat* gate =
        (ReDMCSBF0693WaitVerticalBlankPc34Compat*)context;

    if (!gate) return;
    ++s_vblank_deliveries;
    F0693_VerticalBlankCallback_PC34(gate);
}

static int load_graphics(M11_AssetLoader* loader, char* path, size_t pathSize)
{
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char* home = getenv("HOME");

    if (root && root[0]) {
        snprintf(path, pathSize, "%s/GRAPHICS.DAT", root);
    } else if (home && home[0]) {
        snprintf(path, pathSize, "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    } else {
        return 0;
    }
    return M11_AssetLoader_Init(loader, path);
}

static const M11_AssetSlot* load_real_full_screen_raster(M11_AssetLoader* loader)
{
    unsigned int graphicIndex;

    if (!loader) return NULL;
    for (graphicIndex = 0; graphicIndex < loader->graphicCount; ++graphicIndex) {
        unsigned short width = 0;
        unsigned short height = 0;
        const M11_AssetSlot* slot;

        if (!M11_AssetLoader_QuerySize(loader, graphicIndex, &width, &height) ||
            width != DM1_V1_F0693_F0699_VIDEO_WIDTH_PC34 ||
            height != DM1_V1_F0693_F0699_VIDEO_HEIGHT_PC34) {
            continue;
        }
        slot = M11_AssetLoader_Load(loader, graphicIndex);
        if (slot && slot->loaded && slot->pixels) return slot;
    }
    return NULL;
}

int main(void)
{
    M11_AssetLoader loader;
    const M11_AssetSlot* rasterSlot;
    DM1_V1_F0693F0699VideoRasterPc34 raster;
    DM1_V1_F0693F0699VideoHostPc34 host;
    DM1_V1_F0693F0699VideoReceiptPc34 receipt;
    RedmcsbF0698ZonePc34Compat zone = { 0, 319, 0, 199 };
    TestHost testHost;
    char path[2048];
    int result;

    memset(&loader, 0, sizeof(loader));
    if (!load_graphics(&loader, path, sizeof(path))) {
        if (getenv("FIRESTAFF_DM1_DATA_DIR")) return 1;
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    rasterSlot = load_real_full_screen_raster(&loader);
    if (!rasterSlot) {
        M11_AssetLoader_Shutdown(&loader);
        if (getenv("FIRESTAFF_DM1_DATA_DIR")) return 1;
        puts("SKIP: no 320x200 PC34 raster in GRAPHICS.DAT");
        return 0;
    }
    memset(&raster, 0, sizeof(raster));
    raster.graphicsDatOwned = 1;
    raster.graphicIndex = (int)rasterSlot->graphicIndex;
    raster.width = (int)rasterSlot->width;
    raster.height = (int)rasterSlot->height;
    raster.indexedPixelCount = raster.width * raster.height;
    raster.indexedPixels = rasterSlot->pixels;
    raster.indexedPixelsFNV1a = dm1_v1_f0693_f0699_video_fnv1a_pc34(
        raster.indexedPixels, raster.indexedPixelCount);

    memset(&testHost, 0, sizeof(testHost));
    s_vblank_deliveries = 0;
    s_initialization_host = &testHost;
    memset(&host, 0, sizeof(host));
    host.getVector255 = get_vector;
    host.context = &testHost;
    host.invertDriver.invert_box = invert_box;
    host.invertDriver.context = &testHost;
    host.deliverVerticalBlank = deliver_vertical_blank;
    result = dm1_v1_f0693_f0699_video_present_pc34(
        &raster, &zone, &host, &receipt);
    if (!result || !receipt.valid || !receipt.suppressSyntheticFallback ||
        receipt.graphicIndex != raster.graphicIndex ||
        testHost.vectorRequests != 1 || testHost.initialized != 1 ||
        testHost.inversions != 1 || s_vblank_deliveries != 1) {
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }

    ++raster.indexedPixelsFNV1a;
    if (dm1_v1_f0693_f0699_video_present_pc34(
            &raster, &zone, &host, &receipt) || testHost.inversions != 1) {
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    --raster.indexedPixelsFNV1a;
    zone.right = 320;
    if (dm1_v1_f0693_f0699_video_present_pc34(
            &raster, &zone, &host, &receipt) || testHost.inversions != 1) {
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    zone.right = 319;
    host.getVector255 = NULL;
    if (dm1_v1_f0693_f0699_video_present_pc34(
            &raster, &zone, &host, &receipt) || testHost.inversions != 1) {
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    host.getVector255 = get_vector;
    host.invertDriver.context = NULL;
    if (dm1_v1_f0693_f0699_video_present_pc34(
            &raster, &zone, &host, &receipt) || testHost.inversions != 1) {
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0693/F0698/F0699 video material gate");
    return 0;
}
