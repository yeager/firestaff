#include "dm1_v2_particle_system_pc34.h"
#include <math.h>
#include <stdlib.h>
static uint32_t g_particle_rand_state = 1;
static uint32_t g_particle_rand(void) {
    g_particle_rand_state = g_particle_rand_state * 1103515245 + 12345;
    return (g_particle_rand_state >> 16) & 0x7FFF;
}
void v2_particle_set_seed(uint32_t seed) { g_particle_rand_state = seed; }

static M11_V2_Particle g_particles[M11_V2_MAX_PARTICLES];
static M11_V2_ParticleEmitter g_emitters[M11_V2_MAX_EMITTERS];
static int g_particle_count = 0;

void v2_particle_init(void) {
    int i;
    for (i = 0; i < M11_V2_MAX_PARTICLES; i++) {
        g_particles[i].life = 0.0f;
    }
    for (i = 0; i < M11_V2_MAX_EMITTERS; i++) {
        g_emitters[i].active_count = 0;
        g_emitters[i].accumulator = 0.0f;
        g_emitters[i].particle_template.life = 0.0f;
    }
    g_particle_count = 0;
}

int v2_particle_emitter_create(float x, float y, float rate,
    float spread, float life, float size, float gravity,
    uint32_t color, int max_count)
{
    int idx;
    if (g_emitters[M11_V2_MAX_EMITTERS - 1].particle_template.life != 0.0f) {
        /* All slots full — search for one with zero active particles */
        for (idx = 0; idx < M11_V2_MAX_EMITTERS; idx++) {
            if (g_emitters[idx].active_count == 0 && g_emitters[idx].accumulator == 0.0f)
                break;
        }
        if (idx == M11_V2_MAX_EMITTERS) return -1;
    } else {
        idx = 0;
        while (g_emitters[idx].particle_template.life != 0.0f) {
            idx++;
            if (idx >= M11_V2_MAX_EMITTERS) return -1;
        }
    }
    g_emitters[idx].x = x;
    g_emitters[idx].y = y;
    g_emitters[idx].rate = rate;
    g_emitters[idx].spread = spread;
    g_emitters[idx].max_particles = max_count;
    g_emitters[idx].accumulator = 0.0f;
    g_emitters[idx].active_count = 0;
    g_emitters[idx].particle_template.life = life;
    g_emitters[idx].particle_template.size = size;
    g_emitters[idx].particle_template.gravity = gravity;
    g_emitters[idx].particle_template.color = color;
    return idx;
}

void v2_particle_emit(int emitter_idx, float x, float y) {
    M11_V2_ParticleEmitter *em;
    M11_V2_Particle *p;
    float angle, speed;
    if (emitter_idx < 0 || emitter_idx >= M11_V2_MAX_EMITTERS) return;
    if (g_emitters[emitter_idx].particle_template.life == 0.0f) return;
    em = &g_emitters[emitter_idx];

    em->accumulator += em->rate;
    while (em->accumulator >= 1.0f) {
        em->accumulator -= 1.0f;
        if (g_particle_count < M11_V2_MAX_PARTICLES) {
            p = &g_particles[g_particle_count++];
            p->x = x;
            p->y = y;
            angle = (float)((g_particle_rand() % 360) * 17) / 1000.0f;
            speed = (float)(g_particle_rand() % 100) * 0.01f;
            p->vx = cosf(angle) * speed * em->spread;
            p->vy = sinf(angle) * speed * em->spread;
            p->life = em->particle_template.life;
            p->color = em->particle_template.color;
            p->size = em->particle_template.size;
            p->alpha = em->particle_template.alpha;
            p->gravity = em->particle_template.gravity;
            em->active_count++;
        }
    }
}

void v2_particle_update(float dt) {
    int i, write_idx = 0;
    for (i = 0; i < g_particle_count; i++) {
        M11_V2_Particle *p = &g_particles[i];
        p->life -= dt;
        if (p->life > 0.0f) {
            p->vy += p->gravity * dt;
            p->x += p->vx * dt;
            p->y += p->vy * dt;
            p->alpha = fminf(1.0f, p->life);
            g_particles[write_idx++] = *p;
        }
    }
    g_particle_count = write_idx;
}

void v2_particle_draw_all(void) {
    int i;
    for (i = 0; i < g_particle_count; i++) {
        (void)g_particles[i].x;
        (void)g_particles[i].y;
    }
}

/*
 * Determinism: particle emission direction/speed uses a fixed LCG seed
 * so that the same sequence of emitter-create + emit calls always produces
 * identical particle bursts across runs and platforms.
 * Source: Firestaff DM1 V2 Phase 4 followup.
 */

void v2_particle_tick(float dt) {
    v2_particle_update(dt);
    v2_particle_draw_all();
}

/* Mark an emitter inactive and reclaim its slot.
 * Does not compact g_emitters[]; dead slots are skipped by emitter-create.
 * Source: Firestaff DM1 V2 Phase 4 followup.
 */
void v2_particle_emitter_remove(int emitter_idx) {
    if (emitter_idx < 0 || emitter_idx >= M11_V2_MAX_EMITTERS) return;
    g_emitters[emitter_idx].particle_template.life = 0.0f;
    g_emitters[emitter_idx].active_count = 0;
    g_emitters[emitter_idx].accumulator = 0.0f;
}

void v2_particle_clear(void) {
    int i;
    g_particle_count = 0;
    for (i = 0; i < M11_V2_MAX_EMITTERS; i++) {
        g_emitters[i].active_count = 0;
        g_emitters[i].accumulator = 0.0f;
    }
}

/* V2 Particle System — generic emitter/particle update/render engine */
