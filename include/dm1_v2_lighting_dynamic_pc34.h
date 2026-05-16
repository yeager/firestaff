#ifndef FIRESTAFF_DM1_V2_LIGHTING_DYNAMIC_PC34_H
#define FIRESTAFF_DM1_V2_LIGHTING_DYNAMIC_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define M11_V2_LIGHT_MAX_SOURCES 64
#define M11_V2_LIGHT_MAP_SIZE 32

typedef struct {
    float x, y;
    float radius;
    uint8_t intensity;
    uint8_t color_r, color_g, color_b;
    float flicker_phase;
} M11_V2_LightSource;

typedef struct {
    uint8_t r[M11_V2_LIGHT_MAP_SIZE][M11_V2_LIGHT_MAP_SIZE];
    uint8_t g[M11_V2_LIGHT_MAP_SIZE][M11_V2_LIGHT_MAP_SIZE];
    uint8_t b[M11_V2_LIGHT_MAP_SIZE][M11_V2_LIGHT_MAP_SIZE];
} M11_V2_LightMap;

void v2_light_init(void);
int v2_light_add_source(float x, float y, float radius, uint8_t intensity, uint8_t r, uint8_t g, uint8_t b);
void v2_light_remove_source(int idx);
void v2_light_compute_map(void);
void v2_light_get_tile(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b);
void v2_light_update_flicker(float dt);

#ifdef __cplusplus
}
#endif

#endif
