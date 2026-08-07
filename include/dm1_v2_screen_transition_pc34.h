#ifndef FIRESTAFF_DM1_V2_SCREEN_TRANSITION_PC34_H
#define FIRESTAFF_DM1_V2_SCREEN_TRANSITION_PC34_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FADE_BLACK,
    FADE_WHITE,
    DISSOLVE,
    WIPE_LEFT,
    WIPE_DOWN,
    PIXELATE
} M11_V2_TransitionType;

/* Compatibility-only API. PC34 has no independent screen-transition layer:
 * the apply entries copy source pixels unchanged, state is always inactive,
 * and the V2.2 fade has zero opacity. */
void v2_transition_init(void);
void v2_transition_start(M11_V2_TransitionType type, float speed, int w, int h);
void v2_transition_update(float dt);
void v2_transition_apply(const uint8_t *src, uint8_t *dst, int w, int h);
bool v2_transition_is_active(void);
void v2_transition_skip(void);

void v2_screen_transition_start(int kind, float duration_ms);
void v2_screen_transition_update(float dt_ms);
float v2_screen_transition_progress(void);
int v2_screen_transition_is_done(void);
void v2_screen_transition_apply(const uint8_t *src, uint8_t *dst, int w, int h);

void v22_screen_fade_start(int fade_in);
void v22_screen_fade_update(float dt_ms);
float v22_screen_fade_alpha(void);
int v22_screen_fade_is_done(void);

#ifdef __cplusplus
}
#endif

#endif
