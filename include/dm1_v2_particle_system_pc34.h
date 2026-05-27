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
    float gravity;
} M11_V2_Particle;

typedef struct {
    float x, y;
    float rate;
    float spread;
    M11_V2_Particle particle_template;
    float gravity;
    int max_particles;
    int active_count;
    float accumulator;
} M11_V2_ParticleEmitter;

void v2_particle_init(void);
void v2_particle_emit(int emitter_idx, float x, float y);
void v2_particle_update(float dt);
void v2_particle_draw_all(void);
void v2_particle_clear(void);

/* Create an emitter and return its index, or -1 on failure.
 * Use v2_emitter_preset_get() + this function for preset-based creation.
 * Source: Firestaff DM1 V2 Phase 4. */
int v2_particle_emitter_create(float x, float y, float rate,
    float spread, float life, float size, float gravity,
    uint32_t color, int max_count);

#ifdef __cplusplus
}
#endif

#endif
