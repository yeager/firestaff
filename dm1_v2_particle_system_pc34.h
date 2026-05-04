#ifndef FIRESTAFF_DM1_V2_PARTICLE_SYSTEM_PC34_H
#define FIRESTAFF_DM1_V2_PARTICLE_SYSTEM_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define M11_V2_MAX_PARTICLES 256
#define M11_V2_MAX_EMITTERS 16

typedef struct {
    float x, y;
    float vx, vy;
    float life;
    uint32_t color;
    float size;
    float alpha;
} M11_V2_Particle;

typedef struct {
    float x, y;
    float rate;
    float spread;
    M11_V2_Particle particle_template;
    int max_particles;
    int active_count;
    float accumulator;
} M11_V2_ParticleEmitter;

void v2_particle_init(void);
void v2_particle_emit(int emitter_idx, float x, float y);
void v2_particle_update(float dt);
void v2_particle_draw_all(void);
void v2_particle_clear(void);

#ifdef __cplusplus
}
#endif

#endif
