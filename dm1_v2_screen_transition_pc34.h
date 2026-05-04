#ifndef FIRESTAFF_DM1_V2_SCREEN_TRANSITION_PC34_H
#define FIRESTAFF_DM1_V2_SCREEN_TRANSITION_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef enum {
    FADE_BLACK,
    FADE_WHITE,
    DISSOLVE,
    WIPE_LEFT,
    WIPE_DOWN,
    PIXELATE
} M11_V2_TransitionType;

typedef struct {
    M11_V2_TransitionType type;
    float progress;
    float speed;
    bool active;
    int w;
    int h;
} M11_V2_Transition;

void v2_transition_init(void);
void v2_transition_start(M11_V2_TransitionType type, float speed, int w, int h);
void v2_transition_update(float dt);
void v2_transition_apply(uint8_t* src, uint8_t* dst, int w, int h);
bool v2_transition_is_active(void);
void v2_transition_skip(void);

#ifdef __cplusplus
}
#endif

#endif
