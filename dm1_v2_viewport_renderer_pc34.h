#ifndef FIRESTAFF_DM1_V2_VIEWPORT_RENDERER_PC34_H
#define FIRESTAFF_DM1_V2_VIEWPORT_RENDERER_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V2_VIEWPORT_W 224
#define DM1_V2_VIEWPORT_H 136
#define DM1_V2_MAX_DEPTH 4
#define DM1_V2_FOG_LEVELS 8

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} DM1_V2_Color;

typedef struct {
    int16_t scrollOffX;
    int16_t scrollOffY;
    int16_t scrollTargetX;
    int16_t scrollTargetY;
    int16_t scrollSpeed;
    int16_t scrollProgress;
} DM1_V2_ScrollState;

typedef struct {
    uint8_t fogDensity[DM1_V2_MAX_DEPTH];
    uint8_t lightLevel;
    uint8_t torchRadius;
    uint8_t ambientR;
    uint8_t ambientG;
    uint8_t ambientB;
} DM1_V2_LightConfig;

typedef struct {
    DM1_V2_Color framebuffer[DM1_V2_VIEWPORT_H][DM1_V2_VIEWPORT_W];
    DM1_V2_ScrollState scroll;
    DM1_V2_LightConfig light;
    int dirty;
    int frameCount;
    int32_t lastRenderMs;
} DM1_V2_ViewportState;

void dm1_v2_vp_init(DM1_V2_ViewportState* vp);
void dm1_v2_vp_begin_scroll(DM1_V2_ViewportState* vp, int dx, int dy, int speed);
void dm1_v2_vp_tick_scroll(DM1_V2_ViewportState* vp, int dtMs);
int dm1_v2_vp_is_scrolling(const DM1_V2_ViewportState* vp);
void dm1_v2_vp_set_pixel(DM1_V2_ViewportState* vp, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
DM1_V2_Color dm1_v2_vp_get_pixel(const DM1_V2_ViewportState* vp, int x, int y);
void dm1_v2_vp_clear(DM1_V2_ViewportState* vp, uint8_t r, uint8_t g, uint8_t b);
void dm1_v2_vp_apply_fog(DM1_V2_ViewportState* vp, int depth);
void dm1_v2_vp_apply_light(DM1_V2_ViewportState* vp, int cx, int cy, int radius, uint8_t intensity);
void dm1_v2_vp_mark_dirty(DM1_V2_ViewportState* vp);
int dm1_v2_vp_is_dirty(const DM1_V2_ViewportState* vp);
void dm1_v2_vp_present(DM1_V2_ViewportState* vp, int32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_VIEWPORT_RENDERER_PC34_H */