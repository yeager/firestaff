#ifndef FIRESTAFF_DM1_V2_SMOOTH_MOVEMENT_PC34_H
#define FIRESTAFF_DM1_V2_SMOOTH_MOVEMENT_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float from_x, from_y;
    float to_x, to_y;
    float progress;
    float speed;
    bool active;
} M11_V2_SmoothMove;

typedef struct {
    float from_angle;
    float to_angle;
    float progress;
    float speed;
    bool active;
} M11_V2_SmoothRotation;

void v2_smooth_init(void);
void v2_smooth_start_move(float from_x, float from_y, float to_x, float to_y, float speed);
void v2_smooth_update(float dt);
void v2_smooth_get_position(float *x, float *y);
bool v2_smooth_is_moving(void);
void v2_smooth_start_rotation(float from_angle, float to_angle, float speed);
float v2_smooth_get_rotation(void);

#ifdef __cplusplus
}
#endif

void v22_smooth_start_turn(float from_angle, float to_angle, int frames);

int v22_smooth_tick(float *out_x, float *out_y, float *out_angle);

void v22_smooth_start_turn_v1sync(float from_angle, float to_angle);

void v22_smooth_update_from_clock(const V2_AnimClock *clock);

float v22_smooth_get_x(void);

float v22_smooth_get_y(void);

float v22_smooth_get_angle(void);

#endif
