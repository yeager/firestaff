#include "dm1_v2_particle_system_pc34.h"
#include <math.h>
#include <stdlib.h>

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
    }
    g_particle_count = 0;
}

void v2_particle_emit(int emitter_idx, float x, float y) {
    M11_V2_ParticleEmitter *em;
    M11_V2_Particle *p;
    float angle, speed;
    if (emitter_idx < 0 || emitter_idx >= M11_V2_MAX_EMITTERS) return;
    em = &g_emitters[emitter_idx];

    em->accumulator += em->rate;
    while (em->accumulator >= 1.0f) {
        em->accumulator -= 1.0f;
        if (g_particle_count < M11_V2_MAX_PARTICLES) {
            p = &g_particles[g_particle_count++];
            p->x = x;
            p->y = y;
            angle = (float)(rand() % 360) * 0.0174532925f;
            speed = (float)(rand() % 100) * 0.01f;
            p->vx = cosf(angle) * speed * em->spread;
            p->vy = sinf(angle) * speed * em->spread;
            p->life = em->particle_template.life;
            p->color = em->particle_template.color;
            p->size = em->particle_template.size;
            p->alpha = em->particle_template.alpha;
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

void v2_particle_clear(void) {
    int i;
    g_particle_count = 0;
    for (i = 0; i < M11_V2_MAX_EMITTERS; i++) {
        g_emitters[i].active_count = 0;
        g_emitters[i].accumulator = 0.0f;
    }
}

/* V2 Particle System — generic emitter/particle update/render engine */
