#ifndef FIRESTAFF_DM1_V1_TITLE_SCREEN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_TITLE_SCREEN_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#define DM1_TITLE_ZOOM_STEPS 18

typedef struct {
    int16_t x, y, w, h;
    uint8_t* bitmap;
} M11_TS_ZoomStep;

typedef struct {
    uint8_t* screen_buffers[2];
    uint8_t active_buffer;
    M11_TS_ZoomStep zoom_steps[DM1_TITLE_ZOOM_STEPS];
    uint8_t* title_bitmap;
    uint8_t* master_bitmap;
    bool use_byte_coords;
    bool initialized;
} M11_TS_TitleState;

#ifdef __cplusplus
extern "C" {
#endif

void m11_ts_init(M11_TS_TitleState* state);
bool m11_ts_load_title_graphics(M11_TS_TitleState* state, const uint8_t* data, uint32_t size);
bool m11_ts_animate_zoom(M11_TS_TitleState* state, uint32_t frame);
void m11_ts_draw_title(M11_TS_TitleState* state);
void m11_ts_set_credits_palette(M11_TS_TitleState* state);
void m11_ts_cleanup(M11_TS_TitleState* state);

#ifdef __cplusplus
}
#endif

#endif
