#ifndef FIRESTAFF_DM1_V1_TITLE_SCREEN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_TITLE_SCREEN_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#define DM1_TITLE_ZOOM_STEPS 18
#define DM1_V1_TITLE_GRAPHIC_TITLE_PRESENTS_PC34 562

typedef struct {
    int16_t x, y, w, h;
    uint8_t* bitmap;
} DM1_V1_TitleZoomStepPc34;

typedef struct {
    uint8_t* screen_buffers[2];
    uint8_t active_buffer;
    DM1_V1_TitleZoomStepPc34 zoom_steps[DM1_TITLE_ZOOM_STEPS];
    uint8_t* title_bitmap;
    uint32_t title_bitmap_size;
    uint8_t* master_bitmap;
    bool use_byte_coords;
    bool initialized;
} DM1_V1_TitleStatePc34;

typedef struct {
    int valid;
    int initialized;
    int graphicIndex;
    uint32_t loadedByteSize;
    int zoomStepCount;
    int zoomFrameIndex;
    int sourceX;
    int sourceY;
    int sourceW;
    int sourceH;
    int viewportX;
    int viewportY;
    int viewportW;
    int viewportH;
    int usesByteCoordinates;
    int requiresGraphicsDat;
    int noHostRenderInference;
    const char *reason;
} DM1_V1_TitleRealAssetCaptureReceiptPc34;

#ifdef __cplusplus
extern "C" {
#endif

void DM1_V1_Title_InitPc34Compat(DM1_V1_TitleStatePc34* state);
bool DM1_V1_Title_LoadGraphicsPc34Compat(DM1_V1_TitleStatePc34* state, const uint8_t* data, uint32_t size);
bool DM1_V1_Title_AnimateZoomPc34Compat(DM1_V1_TitleStatePc34* state, uint32_t frame);
void DM1_V1_Title_DrawPc34Compat(DM1_V1_TitleStatePc34* state);
void DM1_V1_Title_SetCreditsPalettePc34Compat(DM1_V1_TitleStatePc34* state);
bool DM1_V1_Title_BuildRealAssetCaptureReceiptPc34Compat(
    const DM1_V1_TitleStatePc34* state,
    uint32_t frame,
    DM1_V1_TitleRealAssetCaptureReceiptPc34* out);
void DM1_V1_Title_CleanupPc34Compat(DM1_V1_TitleStatePc34* state);

#ifdef __cplusplus
}
#endif

#endif
