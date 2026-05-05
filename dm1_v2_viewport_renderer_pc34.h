#ifndef FIRESTAFF_DM1_V2_VIEWPORT_RENDERER_PC34_H
#define FIRESTAFF_DM1_V2_VIEWPORT_RENDERER_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V2_VIEWPORT_W 224
#define DM1_V2_VIEWPORT_H 136
#define DM1_V2_VIEWPORT_BYTE_W 112
#define DM1_V2_VIEWPORT_BLACK_AREA_H 37
#define DM1_V2_VIEWPORT_CEILING_H 29
#define DM1_V2_VIEWPORT_FLOOR_Y 66
#define DM1_V2_VIEWPORT_FLOOR_H 70
#define DM1_V2_MAX_DEPTH 4
#define DM1_V2_FOG_LEVELS 8
#define DM1_V2_FIELD_ASPECT_COUNT 12
#define DM1_V2_WALL_SET_COUNT 15

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} DM1_V2_Color;

typedef enum {
    DM1_V2_VIEW_MATERIAL_BLACK = 0,
    DM1_V2_VIEW_MATERIAL_CEILING,
    DM1_V2_VIEW_MATERIAL_WALL,
    DM1_V2_VIEW_MATERIAL_FLOOR,
    DM1_V2_VIEW_MATERIAL_OUT_OF_BOUNDS
} DM1_V2_ViewMaterial;

typedef enum {
    DM1_V2_FIELD_D3C = 0,
    DM1_V2_FIELD_D3L,
    DM1_V2_FIELD_D3R,
    DM1_V2_FIELD_D2C,
    DM1_V2_FIELD_D2L,
    DM1_V2_FIELD_D2R,
    DM1_V2_FIELD_D1C,
    DM1_V2_FIELD_D1L,
    DM1_V2_FIELD_D1R,
    DM1_V2_FIELD_D0C,
    DM1_V2_FIELD_D0L,
    DM1_V2_FIELD_D0R
} DM1_V2_FieldAspectId;

typedef struct {
    uint8_t nativeBitmapRelativeIndex;
    uint8_t baseStartUnitIndex;
    uint8_t transparentColor;
    uint8_t mask;
    uint8_t byteWidth;
    uint8_t height;
    uint8_t x;
    uint8_t bitPlaneWordCount;
} DM1_V2_FieldAspect;

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

int dm1_v2_vp_source_width(void);
int dm1_v2_vp_source_height(void);
int dm1_v2_vp_source_byte_width(void);
DM1_V2_ViewMaterial dm1_v2_vp_material_at(int x, int y);
const DM1_V2_FieldAspect* dm1_v2_vp_field_aspect(DM1_V2_FieldAspectId id);
int16_t dm1_v2_vp_wall_set_default(int idx);

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