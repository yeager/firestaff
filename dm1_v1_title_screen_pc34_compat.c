#include "dm1_v1_title_screen_pc34_compat.h"
#include <stdlib.h>
#include <string.h>

void m11_ts_init(M11_TS_TitleState* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_TS_TitleState));
    state->active_buffer = 0;
    state->use_byte_coords = true;
    state->initialized = false;
}

bool m11_ts_load_title_graphics(M11_TS_TitleState* state, const uint8_t* data, uint32_t size) {
    if (!state || !data || size == 0) return false;
    
    state->screen_buffers[0] = (uint8_t*)malloc(320 * 200);
    state->screen_buffers[1] = (uint8_t*)malloc(320 * 200);
    if (!state->screen_buffers[0] || !state->screen_buffers[1]) {
        free(state->screen_buffers[0]);
        free(state->screen_buffers[1]);
        return false;
    }
    
    state->title_bitmap = (uint8_t*)malloc(size);
    if (!state->title_bitmap) {
        free(state->screen_buffers[0]);
        free(state->screen_buffers[1]);
        return false;
    }
    
    memcpy(state->title_bitmap, data, size);
    state->master_bitmap = state->title_bitmap;
    state->initialized = true;
    return true;
}

bool m11_ts_animate_zoom(M11_TS_TitleState* state, uint32_t frame) {
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
    
    return true;
}

void m11_ts_draw_title(M11_TS_TitleState* state) {
    if (!state || !state->initialized) return;
    
    uint8_t* target = state->screen_buffers[state->active_buffer];
    if (!target) return;
    
    memset(target, 0, 320 * 200);
    
    uint8_t step = state->active_buffer % DM1_TITLE_ZOOM_STEPS;
    M11_TS_ZoomStep* zs = &state->zoom_steps[step];
    
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

void m11_ts_set_credits_palette(M11_TS_TitleState* state) {
    if (!state) return;
    (void)state;
}

void m11_ts_cleanup(M11_TS_TitleState* state) {
    if (!state) return;
    free(state->screen_buffers[0]);
    free(state->screen_buffers[1]);
    free(state->title_bitmap);
    memset(state, 0, sizeof(M11_TS_TitleState));
}
