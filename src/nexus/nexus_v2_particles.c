
#include "nexus_v2_particles.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

static float randf(void) { return (float)rand() / (float)RAND_MAX; }

void nexus_v2_particles_init(Nexus_V2_ParticleSystem *ps) {
    if (ps) memset(ps, 0, sizeof(*ps));
}

static void spawn_one(Nexus_V2_ParticleSystem *ps,
    Nexus_ParticleType type, float x, float y, float z)
{
    Nexus_Particle *p = NULL;
    int i;
    for (i = 0; i < NEXUS_MAX_PARTICLES; i++) {
        if (!ps->particles[i].active) { p = &ps->particles[i]; break; }
    }
    if (!p) return;
    memset(p, 0, sizeof(*p));
    p->x = x + (randf() - 0.5f) * 0.3f;
    p->y = y + randf() * 0.2f;
    p->z = z + (randf() - 0.5f) * 0.3f;
    p->type = type;
    p->active = 1;

    switch (type) {
    case NEXUS_PART_DUST:
        p->vx = (randf()-0.5f)*0.1f; p->vy = -0.02f; p->vz = (randf()-0.5f)*0.1f;
        p->max_life = p->life = 2.0f + randf(); p->color = 7; break;
    case NEXUS_PART_SPARK:
        p->vx = (randf()-0.5f)*0.5f; p->vy = 0.3f+randf()*0.3f; p->vz = (randf()-0.5f)*0.5f;
        p->max_life = p->life = 0.3f + randf()*0.3f; p->color = 14; break;
    case NEXUS_PART_MAGIC:
        p->vx = (randf()-0.5f)*0.2f; p->vy = 0.1f+randf()*0.2f; p->vz = (randf()-0.5f)*0.2f;
        p->max_life = p->life = 1.0f + randf(); p->color = 4; break;
    case NEXUS_PART_EMBER:
        p->vx = (randf()-0.5f)*0.1f; p->vy = 0.15f+randf()*0.1f; p->vz = (randf()-0.5f)*0.1f;
        p->max_life = p->life = 1.5f + randf(); p->color = 12; break;
    case NEXUS_PART_WATER_DRIP:
        p->vy = -0.4f; p->max_life = p->life = 0.8f; p->color = 13; break;
    case NEXUS_PART_BLOOD:
        p->vx = (randf()-0.5f)*0.3f; p->vy = 0.2f; p->vz = (randf()-0.5f)*0.3f;
        p->max_life = p->life = 0.5f; p->color = 12; break;
    case NEXUS_PART_SMOKE:
        p->vx = (randf()-0.5f)*0.05f; p->vy = 0.08f; p->vz = (randf()-0.5f)*0.05f;
        p->max_life = p->life = 3.0f + randf()*2.0f; p->color = 6; break;
    }
    if (ps->count < NEXUS_MAX_PARTICLES) ps->count++;
}

void nexus_v2_particles_emit(Nexus_V2_ParticleSystem *ps,
    Nexus_ParticleType type, float x, float y, float z, int count)
{
    int i;
    if (!ps) return;
    for (i = 0; i < count; i++) spawn_one(ps, type, x, y, z);
}

void nexus_v2_particles_tick(Nexus_V2_ParticleSystem *ps, float dt) {
    int i;
    if (!ps) return;
    for (i = 0; i < NEXUS_MAX_PARTICLES; i++) {
        Nexus_Particle *p = &ps->particles[i];
        if (!p->active) continue;
        p->x += p->vx * dt;
        p->y += p->vy * dt;
        p->z += p->vz * dt;
        p->life -= dt;
        /* Gravity for sparks/blood */
        if (p->type == NEXUS_PART_SPARK || p->type == NEXUS_PART_BLOOD)
            p->vy -= 0.8f * dt;
        if (p->life <= 0 || p->y < -0.5f) {
            p->active = 0; ps->count--;
        }
    }
}

void nexus_v2_particles_render(Nexus_V2_ParticleSystem *ps,
    uint32_t *rgba, int w, int h, const uint32_t *palette,
    float cam_x, float cam_z, int cam_dir)
{
    int i;
    float dx_map[4] = {0, 1, 0, -1};
    float dz_map[4] = {-1, 0, 1, 0};
    (void)cam_dir;

    if (!ps || !rgba || !palette) return;
    for (i = 0; i < NEXUS_MAX_PARTICLES; i++) {
        Nexus_Particle *p = &ps->particles[i];
        float rel_x, rel_z, depth;
        int sx, sy;
        float alpha;
        if (!p->active) continue;

        /* Transform to screen-space (simplified) */
        rel_x = p->x - cam_x;
        rel_z = p->z - cam_z;
        depth = -(rel_z * dz_map[cam_dir & 3] + rel_x * dx_map[cam_dir & 3]);
        if (depth < 0.1f) continue;

        sx = w/2 + (int)((rel_x * dx_map[(cam_dir+1)&3] + rel_z * dz_map[(cam_dir+1)&3]) / depth * w * 0.5f);
        sy = h/2 - (int)(p->y / depth * h * 0.5f);

        if (sx < 0 || sx >= w || sy < 0 || sy >= h) continue;

        alpha = p->life / p->max_life;
        if (alpha > 0.3f) {
            uint32_t col = palette[p->color];
            /* Alpha blend */
            uint32_t bg = rgba[sy * w + sx];
            int br = (bg>>16)&0xFF, bg2 = (bg>>8)&0xFF, bb = bg&0xFF;
            int fr = (col>>16)&0xFF, fg = (col>>8)&0xFF, fb = col&0xFF;
            int rr = (int)(fr * alpha + br * (1-alpha));
            int gg = (int)(fg * alpha + bg2 * (1-alpha));
            int bbb = (int)(fb * alpha + bb * (1-alpha));
            rgba[sy * w + sx] = 0xFF000000 | (rr<<16) | (gg<<8) | bbb;
        }
    }
}

const char *nexus_v2_particles_source_evidence(void) {
    return
        "Nexus V2.2: particle system — dust, sparks, magic, ember, water, blood, smoke\n"
        "  Source: Saturn VDP1 point sprites / blitter particle blits\n"
        "  Source: ReDMCSB DUNGEON.C (spell/torch particle emit gates)\n"
        "  Source: ReDMCSB COMMAND.C F0209 (particle-type binding per action)";
}

