#include "dm1_v2_screen_transition_pc34.h"

#include <string.h>

/* ReDMCSB's PC34 main loop sends the complete party tuple directly to F0128
 * and F0097 presents that composed viewport.  No source-owned transition
 * layer was found between those calls.  Keep these historical V2 entry
 * points inert rather than manufacturing fades, wipes, or pixelation. */

void v2_transition_init(void) {
}

void v2_transition_start(M11_V2_TransitionType type, float speed, int w, int h) {
    (void)type;
    (void)speed;
    (void)w;
    (void)h;
}

void v2_transition_update(float dt) {
    (void)dt;
}

void v2_transition_apply(const uint8_t *src, uint8_t *dst, int w, int h) {
    if (!src || !dst || w <= 0 || h <= 0) return;
    memcpy(dst, src, (size_t)w * (size_t)h);
}

bool v2_transition_is_active(void) {
    return false;
}

void v2_transition_skip(void) {
}

void v2_screen_transition_start(int kind, float duration_ms) {
    (void)kind;
    (void)duration_ms;
}

void v2_screen_transition_update(float dt_ms) {
    (void)dt_ms;
}

float v2_screen_transition_progress(void) {
    return 0.0f;
}

int v2_screen_transition_is_done(void) {
    return 1;
}

void v2_screen_transition_apply(const uint8_t *src, uint8_t *dst, int w, int h) {
    v2_transition_apply(src, dst, w, h);
}

void v22_screen_fade_start(int fade_in) {
    (void)fade_in;
}

void v22_screen_fade_update(float dt_ms) {
    (void)dt_ms;
}

float v22_screen_fade_alpha(void) {
    return 0.0f;
}

int v22_screen_fade_is_done(void) {
    return 1;
}
