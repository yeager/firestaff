#include "dm1_v1_title_screen_pc34_compat.h"
#include <stdlib.h>
#include <string.h>

void DM1_V1_Title_InitPc34Compat(DM1_V1_TitleStatePc34* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_V1_TitleStatePc34));
    state->active_buffer = 0;
    state->use_byte_coords = true;
    state->initialized = false;
}

bool DM1_V1_Title_LoadGraphicsPc34Compat(DM1_V1_TitleStatePc34* state, const uint8_t* data, uint32_t size) {
    uint8_t* screen0;
    uint8_t* screen1;
    uint8_t* title;

    if (!state || !data || size == 0) return false;
    if (size != DM1_V1_TITLE_C001_BYTES_PC34) return false;
    if (state->initialized) return false;

    screen0 = (uint8_t*)malloc(DM1_V1_TITLE_C001_BYTES_PC34);
    screen1 = (uint8_t*)malloc(DM1_V1_TITLE_C001_BYTES_PC34);
    if (!screen0 || !screen1) {
        free(screen0);
        free(screen1);
        return false;
    }

    title = (uint8_t*)malloc(size);
    if (!title) {
        free(screen0);
        free(screen1);
        return false;
    }

    memcpy(title, data, size);
    state->screen_buffers[0] = screen0;
    state->screen_buffers[1] = screen1;
    state->title_bitmap = title;
    state->title_bitmap_size = size;
    state->master_bitmap = state->title_bitmap;
    state->current_zoom_step = 0;
    state->initialized = true;
    return true;
}

bool DM1_V1_Title_AnimateZoomPc34Compat(DM1_V1_TitleStatePc34* state, uint32_t frame) {
    if (!state || !state->initialized) return false;

    uint8_t step = frame % DM1_TITLE_ZOOM_STEPS;
    int16_t w = 48 + step * 16;
    int16_t h = 12 + step * 4;
    int16_t x = (320 - w) / 2;
    int16_t y = (200 - h) / 2;

    state->zoom_steps[step].x = x;
    state->zoom_steps[step].y = y;
    state->zoom_steps[step].w = w;
    state->zoom_steps[step].h = h;
    state->zoom_steps[step].bitmap = state->title_bitmap;
    state->current_zoom_step = step;

    return true;
}

void DM1_V1_Title_DrawPc34Compat(DM1_V1_TitleStatePc34* state) {
    if (!state || !state->initialized) return;

    uint8_t* target = state->screen_buffers[state->active_buffer];
    if (!target) return;

    memset(target, 0, DM1_V1_TITLE_C001_BYTES_PC34);

    uint8_t step = state->current_zoom_step % DM1_TITLE_ZOOM_STEPS;
    DM1_V1_TitleZoomStepPc34* zs = &state->zoom_steps[step];

    if (zs->bitmap && zs->w > 0 && zs->h > 0) {
        for (int16_t dy = 0; dy < zs->h; dy++) {
            for (int16_t dx = 0; dx < zs->w; dx++) {
                int16_t dx_abs = zs->x + dx;
                int16_t dy_abs = zs->y + dy;

                if (dx_abs >= 0 && dx_abs < 320 && dy_abs >= 0 && dy_abs < 200) {
                    target[dy_abs * 320 + dx_abs] = zs->bitmap[dy * zs->w + dx];
                }
            }
        }
    }

    state->active_buffer = 1 - state->active_buffer;
}

void DM1_V1_Title_SetCreditsPalettePc34Compat(DM1_V1_TitleStatePc34* state) {
    if (!state) return;
    (void)state;
}

bool DM1_V1_Title_BuildRealAssetCaptureReceiptPc34Compat(
    const DM1_V1_TitleStatePc34* state,
    uint32_t frame,
    DM1_V1_TitleRealAssetCaptureReceiptPc34* out) {
    uint8_t step;
    int16_t w;
    int16_t h;
    int16_t x;
    int16_t y;

    if (!state || !out) return false;
    memset(out, 0, sizeof(*out));
    if (!state->initialized ||
        !state->title_bitmap ||
        state->title_bitmap_size != DM1_V1_TITLE_C001_BYTES_PC34) {
        out->requiresGraphicsDat = 1;
        out->noHostRenderInference = 1;
        out->reason =
            "title-real-asset-capture: missing exact GRAPHICS.DAT C001 "
            "320x200 source";
        return false;
    }

    step = frame % DM1_TITLE_ZOOM_STEPS;
    w = 48 + step * 16;
    h = 12 + step * 4;
    x = (320 - w) / 2;
    y = (200 - h) / 2;

    out->valid = 1;
    out->initialized = state->initialized ? 1 : 0;
    out->graphicIndex = DM1_V1_TITLE_GRAPHIC_TITLE_PRESENTS_PC34;
    out->loadedByteSize = state->title_bitmap_size;
    out->zoomStepCount = DM1_TITLE_ZOOM_STEPS;
    out->zoomFrameIndex = step;
    out->sourceX = 0;
    out->sourceY = 0;
    out->sourceW = w;
    out->sourceH = h;
    out->viewportX = x;
    out->viewportY = y;
    out->viewportW = w;
    out->viewportH = h;
    out->usesByteCoordinates = state->use_byte_coords ? 1 : 0;
    out->requiresGraphicsDat = 1;
    out->noHostRenderInference = 1;
    out->reason =
        "title-real-asset-capture: GRAPHICS.DAT C562, byte zoom rect, "
        "no M11 host render inference";
    return true;
}

void DM1_V1_Title_CleanupPc34Compat(DM1_V1_TitleStatePc34* state) {
    if (!state) return;
    free(state->screen_buffers[0]);
    free(state->screen_buffers[1]);
    free(state->title_bitmap);
    memset(state, 0, sizeof(DM1_V1_TitleStatePc34));
}

void m11_ts_init(M11_TS_TitleState* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_TS_TitleState));
    state->active_buffer = 0;
    state->initialized = false;
}

bool m11_ts_load_title_graphics(M11_TS_TitleState* state, const uint8_t* data, uint32_t size) {
    return DM1_V1_Title_LoadGraphicsPc34Compat(state, data, size);
}

bool m11_ts_animate_zoom(M11_TS_TitleState* state, uint32_t frame) {
    return DM1_V1_Title_AnimateZoomPc34Compat(state, frame);
}

void m11_ts_cleanup(M11_TS_TitleState* state) {
    if (!state) return;
    free(state->screen_buffers[0]);
    free(state->screen_buffers[1]);
    free(state->title_bitmap);
    memset(state, 0, sizeof(M11_TS_TitleState));
}
