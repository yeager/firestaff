#ifndef FIRESTAFF_DM1_V1_TITLE_SCREEN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_TITLE_SCREEN_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#define DM1_TITLE_ZOOM_STEPS 18

typedef struct {
    int16_t x, y, w, h;
    uint8_t* bitmap;
} DM1_V1_TitleZoomStepPc34;

typedef struct {
    uint8_t* screen_buffers[2];
    uint8_t active_buffer;
    DM1_V1_TitleZoomStepPc34 zoom_steps[DM1_TITLE_ZOOM_STEPS];
    uint8_t* title_bitmap;
    uint8_t* master_bitmap;
    bool use_byte_coords;
    bool initialized;
} DM1_V1_TitleStatePc34;

#ifdef __cplusplus
extern "C" {
#endif

void DM1_V1_Title_InitPc34Compat(DM1_V1_TitleStatePc34* state);
bool DM1_V1_Title_LoadGraphicsPc34Compat(DM1_V1_TitleStatePc34* state, const uint8_t* data, uint32_t size);
bool DM1_V1_Title_AnimateZoomPc34Compat(DM1_V1_TitleStatePc34* state, uint32_t frame);
void DM1_V1_Title_DrawPc34Compat(DM1_V1_TitleStatePc34* state);
void DM1_V1_Title_SetCreditsPalettePc34Compat(DM1_V1_TitleStatePc34* state);
void DM1_V1_Title_CleanupPc34Compat(DM1_V1_TitleStatePc34* state);

/* Compatibility aliases for older M11 call sites. */
typedef DM1_V1_TitleZoomStepPc34 M11_TS_ZoomStep;
typedef DM1_V1_TitleStatePc34 M11_TS_TitleState;
#define m11_ts_init DM1_V1_Title_InitPc34Compat
#define m11_ts_load_title_graphics DM1_V1_Title_LoadGraphicsPc34Compat
#define m11_ts_animate_zoom DM1_V1_Title_AnimateZoomPc34Compat
#define m11_ts_draw_title DM1_V1_Title_DrawPc34Compat
#define m11_ts_set_credits_palette DM1_V1_Title_SetCreditsPalettePc34Compat
#define m11_ts_cleanup DM1_V1_Title_CleanupPc34Compat

#ifdef __cplusplus
}
#endif

#endif
