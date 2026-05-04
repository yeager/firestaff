#ifndef FIRESTAFF_DM1_V2_HUD_OVERLAY_PC34_H
#define FIRESTAFF_DM1_V2_HUD_OVERLAY_PC34_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int direction;
    float needle_angle;
    bool animated;
} M11_V2_HudCompass;

typedef struct {
    int current_level;
    int max_level;
} M11_V2_HudDepth;

typedef struct {
    M11_V2_HudCompass compass;
    M11_V2_HudDepth depth;
    bool visible;
    uint8_t opacity;
    bool stats_bar_visible;
} M11_V2_HudOverlay;

void v2_hud_init(void);
void v2_hud_set_direction(int dir);
void v2_hud_set_level(int cur, int max);
void v2_hud_render(uint8_t* fb, int w, int h);
void v2_hud_toggle(void);
void v2_hud_set_opacity(uint8_t val);

#ifdef __cplusplus
}
#endif

#endif
