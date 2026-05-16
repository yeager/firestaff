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

#define V2_MAX_PARTICLES 512
#define V2_MAX_EMITTERS 16

typedef struct {
    float x, y, vx, vy;
    float life, max_life;
    float size;
    uint32_t color;
    int active;
} V2_Particle;

typedef struct {
    float x, y;
    float rate; /* particles per second */
    float spread;
    float base_life;
    float base_size;
    float gravity;
    uint32_t color;
    int max_count;
    float accumulator;
    int active;
} V2_Emitter;

static V2_Particle g_particles[V2_MAX_PARTICLES];
static V2_Emitter g_emitters[V2_MAX_EMITTERS];

static uint32_t g_pseed = 1234;
static float prand(void) {
    g_pseed = g_pseed * 1103515245 + 12345;
    return ((float)((g_pseed >> 16) & 0x7FFF) / 32767.0f);
}

int v2_particle_emitter_create(float x, float y, float rate,
    float spread, float life, float size, float gravity,
    uint32_t color, int max_count)
{
    int i;
    for (i = 0; i < V2_MAX_EMITTERS; i++) {
        if (!g_emitters[i].active) {
            g_emitters[i].x = x; g_emitters[i].y = y;
            g_emitters[i].rate = rate; g_emitters[i].spread = spread;
            g_emitters[i].base_life = life; g_emitters[i].base_size = size;
            g_emitters[i].gravity = gravity; g_emitters[i].color = color;
            g_emitters[i].max_count = max_count; g_emitters[i].accumulator = 0;
            g_emitters[i].active = 1;
            return i;
        }
    }
    return -1;
}

void v2_particle_tick(float dt) {
    int i, j;
    /* Update emitters — spawn new particles */
    for (i = 0; i < V2_MAX_EMITTERS; i++) {
        if (!g_emitters[i].active) continue;
        g_emitters[i].accumulator += g_emitters[i].rate * dt;
        while (g_emitters[i].accumulator >= 1.0f) {
            g_emitters[i].accumulator -= 1.0f;
            /* Find free particle */
            for (j = 0; j < V2_MAX_PARTICLES; j++) {
                if (!g_particles[j].active) {
                    g_particles[j].x = g_emitters[i].x + (prand() - 0.5f) * g_emitters[i].spread;
                    g_particles[j].y = g_emitters[i].y + (prand() - 0.5f) * g_emitters[i].spread;
                    g_particles[j].vx = (prand() - 0.5f) * g_emitters[i].spread * 2;
                    g_particles[j].vy = (prand() - 0.5f) * g_emitters[i].spread * 2;
                    g_particles[j].life = g_emitters[i].base_life * (0.5f + prand() * 0.5f);
                    g_particles[j].max_life = g_particles[j].life;
                    g_particles[j].size = g_emitters[i].base_size;
                    g_particles[j].color = g_emitters[i].color;
                    g_particles[j].active = 1;
                    break;
                }
            }
        }
    }
    /* Update particles */
    for (j = 0; j < V2_MAX_PARTICLES; j++) {
        if (!g_particles[j].active) continue;
        g_particles[j].x += g_particles[j].vx * dt;
        g_particles[j].y += g_particles[j].vy * dt;
        g_particles[j].vy += 50.0f * dt; /* default gravity */
        g_particles[j].life -= dt;
        if (g_particles[j].life <= 0) g_particles[j].active = 0;
    }
}

void v2_particle_emitter_destroy(int id) {
    if (id >= 0 && id < V2_MAX_EMITTERS) g_emitters[id].active = 0;
}

void v2_particle_clear_all(void) {
    memset(g_particles, 0, sizeof(g_particles));
    memset(g_emitters, 0, sizeof(g_emitters));
}

int v2_particle_active_count(void) {
    int i, c = 0;
    for (i = 0; i < V2_MAX_PARTICLES; i++) if (g_particles[i].active) c++;
    return c;
}

