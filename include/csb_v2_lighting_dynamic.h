#ifndef FIRESTAFF_CSB_V2_LIGHTING_DYNAMIC_H
#define FIRESTAFF_CSB_V2_LIGHTING_DYNAMIC_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V2_LIGHT_MAX_SOURCES 32
#define CSB_V2_LIGHT_MAP_SIZE 32

typedef struct {
    float x;
    float y;
    float radius;
    uint8_t baseIntensity;
    uint8_t intensity;
    uint8_t colorR;
    uint8_t colorG;
    uint8_t colorB;
    float flickerPhase;
    int flicker;
    int active;
} CSB_V2_LightSource;

typedef struct {
    uint8_t r[CSB_V2_LIGHT_MAP_SIZE][CSB_V2_LIGHT_MAP_SIZE];
    uint8_t g[CSB_V2_LIGHT_MAP_SIZE][CSB_V2_LIGHT_MAP_SIZE];
    uint8_t b[CSB_V2_LIGHT_MAP_SIZE][CSB_V2_LIGHT_MAP_SIZE];
} CSB_V2_LightMap;

/* ReDMCSB PANEL.C:418-428 selects the canonical dungeon palette from torch
 * charges and magical light. CSB V2 mirrors that source palette first, then
 * adds presentation-only local light on top. */
typedef struct {
    uint8_t sourcePaletteIndex;
    uint8_t sourceLightAmountFloor;
    uint8_t darknessPercent;
    uint8_t shadowAlpha;
    bool enhancedEffectsEnabled;
    bool deterministicFallback;
} CSB_V2_SourcePaletteLighting;

typedef enum {
    CSB_V2_LIGHT_EVENT_NORMAL = 0,
    CSB_V2_LIGHT_EVENT_FLICKER_TORCH,
    CSB_V2_LIGHT_EVENT_DARKNESS_BURST,
    CSB_V2_LIGHT_EVENT_MAGICAL_PULSE,
    CSB_V2_LIGHT_EVENT_CHAOS_SURGE,
    CSB_V2_LIGHT_EVENT_COUNT
} CSB_V2_LightEventType;

extern const uint8_t k_csb_v2_source_palette_light_amount_floor[6];

void csb_v2_light_init(void);

CSB_V2_SourcePaletteLighting csb_v2_light_build_source_palette_lighting(
    int sourcePaletteIndex,
    bool enhancedEffectsEnabled);

int csb_v2_light_add_source(float x,
                            float y,
                            float radius,
                            uint8_t intensity,
                            uint8_t r,
                            uint8_t g,
                            uint8_t b,
                            int flicker);
void csb_v2_light_remove_source(int index);

void csb_v2_light_compute_map(void);
void csb_v2_light_get_tile(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b);
void csb_v2_light_update_flicker(float dtSeconds);
void csb_v2_light_tick(float dtSeconds);

void csb_v2_light_set_ambient(float level);
float csb_v2_light_get_ambient(void);
void csb_v2_light_set_dungeon_level(int level);
int csb_v2_light_get_dungeon_level(void);

void csb_v2_light_event_trigger(CSB_V2_LightEventType type,
                                float durationSeconds,
                                float intensity);
void csb_v2_light_event_tick(float dtSeconds);
int csb_v2_light_event_is_active(void);
CSB_V2_LightEventType csb_v2_light_event_current_type(void);

const char *csb_v2_light_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_LIGHTING_DYNAMIC_H */
