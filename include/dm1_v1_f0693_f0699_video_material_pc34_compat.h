#ifndef FIRESTAFF_DM1_V1_F0693_F0699_VIDEO_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0693_F0699_VIDEO_MATERIAL_PC34_COMPAT_H

#include "redmcsb_f0693_wait_vertical_blank_pc34_compat.h"
#include "redmcsb_f0698_invert_box_pc34_compat.h"
#include "redmcsb_f0699_video_interrupt_pc34_compat.h"

#include <stdint.h>

enum {
    DM1_V1_F0693_F0699_VIDEO_WIDTH_PC34 = 320,
    DM1_V1_F0693_F0699_VIDEO_HEIGHT_PC34 = 200,
    DM1_V1_F0693_F0699_VIDEO_PIXEL_COUNT_PC34 =
        DM1_V1_F0693_F0699_VIDEO_WIDTH_PC34 *
        DM1_V1_F0693_F0699_VIDEO_HEIGHT_PC34
};

/* A caller supplies an already-decoded PC34 full-screen raster. This module
 * neither constructs a framebuffer nor converts a palette. */
typedef struct DM1_V1_F0693F0699VideoRasterPc34 {
    int graphicsDatOwned;
    int graphicIndex;
    int width;
    int height;
    int indexedPixelCount;
    const unsigned char* indexedPixels;
    uint32_t indexedPixelsFNV1a;
} DM1_V1_F0693F0699VideoRasterPc34;

/* IMAGE.C owns one G2156 video-driver table. Keep vector lookup and inversion
 * on the same host context instead of splicing callbacks from unrelated host
 * surfaces. F0693 receives its own installed VBlank callback. */
typedef struct DM1_V1_F0693F0699VideoHostPc34 {
    ReDMCSBF0699GetVectorPc34Compat getVector255;
    void* context;
    RedmcsbF0698VideoDriverPc34Compat invertDriver;
    ReDMCSBF0693VBlankCallbackPc34Compat deliverVerticalBlank;
} DM1_V1_F0693F0699VideoHostPc34;

typedef struct DM1_V1_F0693F0699VideoReceiptPc34 {
    int valid;
    int suppressSyntheticFallback;
    int graphicIndex;
    RedmcsbF0698ZonePc34Compat zone;
    uint32_t rasterFingerprint;
} DM1_V1_F0693F0699VideoReceiptPc34;

uint32_t dm1_v1_f0693_f0699_video_fnv1a_pc34(
    const unsigned char* bytes, int byteCount);

/* ReDMCSB IMAGE.C F0699 installs vector 255 before the F0698 driver call;
 * F0693 then waits for the host VBlank callback. Missing raw material or any
 * primitive rejects before the invert callback is invoked. */
int dm1_v1_f0693_f0699_video_present_pc34(
    const DM1_V1_F0693F0699VideoRasterPc34* raster,
    const RedmcsbF0698ZonePc34Compat* zone,
    const DM1_V1_F0693F0699VideoHostPc34* host,
    DM1_V1_F0693F0699VideoReceiptPc34* outReceipt);

#endif
