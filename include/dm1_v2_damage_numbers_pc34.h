#ifndef FIRESTAFF_DM1_V2_DAMAGE_NUMBERS_PC34_H
#define FIRESTAFF_DM1_V2_DAMAGE_NUMBERS_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    float x, y;
    int value;
    uint8_t color;
    float life;
    float vy;
} M11_V2_DamagePopup;

typedef struct {
    M11_V2_DamagePopup popups[32];
    int count;
} M11_V2_DamageDisplay;

void v2_damage_init(void);
void v2_damage_spawn(float x, float y, int value, uint8_t color);
void v2_damage_update(float dt);
void v2_damage_render(uint8_t* fb, int w, int h);
void v2_damage_clear(void);

#ifdef __cplusplus
}
#endif

#endif
