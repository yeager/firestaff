#ifndef FIRESTAFF_DM1_V2_FOOTSTEP_AUDIO_PC34_H
#define FIRESTAFF_DM1_V2_FOOTSTEP_AUDIO_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    STONE,
    WATER,
    METAL,
    WOOD,
    DIRT
} M11_V2_SurfaceType;

typedef struct {
    M11_V2_SurfaceType surface;
    uint8_t volume;
    float pitch_variance;
    bool echo_enabled;
} M11_V2_FootstepConfig;

void v2_footstep_init(void);
void v2_footstep_set_surface(M11_V2_SurfaceType type);
void v2_footstep_trigger(bool left_right);
void v2_footstep_set_echo(bool enabled);
int v2_footstep_get_sample(int16_t* buf, int* len);

#ifdef __cplusplus
}
#endif

#endif
