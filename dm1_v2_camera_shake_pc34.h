#ifndef FIRESTAFF_DM1_V2_CAMERA_SHAKE_PC34_H
#define FIRESTAFF_DM1_V2_CAMERA_SHAKE_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float intensity;
    float decay;
    bool active;
    float offset_x;
    float offset_y;
    float frequency;
    float phase;
} M11_V2_CameraShake;

void v2_shake_init(void);
void v2_shake_trigger(float intensity, float decay_rate);
void v2_shake_update(float dt);
void v2_shake_get_offset(float *x, float *y);
bool v2_shake_is_active(void);
void v2_shake_stop(void);

#ifdef __cplusplus
}
#endif

#endif
