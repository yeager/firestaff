#include "dm1_v2_camera_shake_pc34.h"

/* PC34 has no authenticated screen-shake route. Keep this compatibility
 * surface inert until one is recovered from the original executable. */
void v2_shake_init(void) {}
void v2_shake_trigger(float intensity, float decay_rate) {
    (void)intensity;
    (void)decay_rate;
}
void v2_shake_update(float dt) { (void)dt; }
void v2_shake_get_offset(float *x, float *y) {
    if (x) *x = 0.0f;
    if (y) *y = 0.0f;
}
bool v2_shake_is_active(void) { return false; }
void v2_shake_stop(void) {}

void v22_shake_add_trauma(float amount) { (void)amount; }
void v22_shake_tick(float dt, float *out_dx, float *out_dy) {
    (void)dt;
    if (out_dx) *out_dx = 0.0f;
    if (out_dy) *out_dy = 0.0f;
}
void v22_shake_reset(void) {}
float v22_shake_get_trauma(void) { return 0.0f; }
