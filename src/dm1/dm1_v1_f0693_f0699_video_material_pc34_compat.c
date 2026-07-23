#include "dm1_v1_f0693_f0699_video_material_pc34_compat.h"

#include <string.h>

uint32_t dm1_v1_f0693_f0699_video_fnv1a_pc34(
    const unsigned char* bytes, int byteCount)
{
    uint32_t hash = 2166136261u;
    int index;

    if (!bytes || byteCount <= 0) return 0u;
    for (index = 0; index < byteCount; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

int dm1_v1_f0693_f0699_video_present_pc34(
    const DM1_V1_F0693F0699VideoRasterPc34* raster,
    const RedmcsbF0698ZonePc34Compat* zone,
    const DM1_V1_F0693F0699VideoHostPc34* host,
    DM1_V1_F0693F0699VideoReceiptPc34* outReceipt)
{
    ReDMCSBF0699VideoInterruptPc34Compat interruptState;
    ReDMCSBF0693WaitVerticalBlankPc34Compat verticalBlankGate;
    uint32_t rasterFingerprint;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!raster || !zone || !host || !raster->graphicsDatOwned ||
        !raster->indexedPixels ||
        raster->width != DM1_V1_F0693_F0699_VIDEO_WIDTH_PC34 ||
        raster->height != DM1_V1_F0693_F0699_VIDEO_HEIGHT_PC34 ||
        raster->indexedPixelCount != DM1_V1_F0693_F0699_VIDEO_PIXEL_COUNT_PC34 ||
        zone->left < 0 || zone->right < zone->left ||
        zone->right >= DM1_V1_F0693_F0699_VIDEO_WIDTH_PC34 ||
        zone->top < 0 || zone->bottom < zone->top ||
        zone->bottom >= DM1_V1_F0693_F0699_VIDEO_HEIGHT_PC34 ||
        !host->getVector255 || !host->invertDriver.invert_box ||
        !host->deliverVerticalBlank ||
        host->invertDriver.context != host->context) {
        return 0;
    }
    rasterFingerprint = dm1_v1_f0693_f0699_video_fnv1a_pc34(
        raster->indexedPixels, raster->indexedPixelCount);
    if (!rasterFingerprint || rasterFingerprint != raster->indexedPixelsFNV1a) {
        return 0;
    }

    memset(&interruptState, 0, sizeof(interruptState));
    if (!F0699_InitVideoInterrupt_PC34(&interruptState, host->getVector255,
                                        host->context) ||
        !redmcsb_f0698_invert_box_pc34_compat(&host->invertDriver, zone)) {
        return 0;
    }
    memset(&verticalBlankGate, 0, sizeof(verticalBlankGate));
    verticalBlankGate.deliver_vertical_blank = host->deliverVerticalBlank;
    verticalBlankGate.context = &verticalBlankGate;
    if (!F0693_WaitVerticalBlank_PC34(&verticalBlankGate)) return 0;

    outReceipt->valid = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->graphicIndex = raster->graphicIndex;
    outReceipt->zone = *zone;
    outReceipt->rasterFingerprint = rasterFingerprint;
    return 1;
}
