#include "csb_v2_vfx_particles.h"

#include <math.h>
#include <string.h>

/* CSB V2.2 Particle VFX System
 *
 * Source-lock anchors:
 * - CSBWin/Graphics.cpp: explosion/projectile rendering (fireball, lightning)
 * - CSBWin/Chaos.cpp: spell cast visual triggers
 * - ReDMCSB PANEL.C: torch light rendering
 *
 * Presentation-only. V1 projectile hit detection and chaos magic
 * DSA scripts are unaffected.
 */

/* ---- Colour palettes per VFX type ---- */

static const uint8_t k_fire_palette_r[8]   = { 255, 255, 200, 150, 100, 80, 50, 20 };
static const uint8_t k_fire_palette_g[8]   = { 180, 120,  60,  20,   0,  0,  0,  0 };
static const uint8_t k_fire_palette_b[8]   = {   0,   0,   0,   0,   0,  0,  0,  0 };

static const uint8_t k_smoke_palette_r[8]   = { 120, 100,  80,  60,  50,  40,  30,  20 };
static const uint8_t k_smoke_palette_g[8]   = { 120, 100,  80,  60,  50,  40,  30,  20 };
static const uint8_t k_smoke_palette_b[8]   = { 120, 100,  80,  60,  50,  40,  30,  20 };

static const uint8_t k_spark_palette_r[8]   = { 200, 255, 255, 200, 150, 100,  80,  50 };
static const uint8_t k_spark_palette_g[8]   = { 200, 200, 100,  50,  20,   0,   0,   0 };
static const uint8_t k_spark_palette_b[8]   = { 255, 255, 200, 255, 255, 200, 150, 100 };

static const uint8_t k_glow_palette_r[8]   = { 200, 150, 100, 150, 200, 150, 100,  80 };
static const uint8_t k_glow_palette_g[8]   = { 100, 150, 200, 150, 100, 150, 200, 150 };
static const uint8_t k_glow_palette_b[8]   = { 255, 255, 255, 200, 150, 200, 255, 255 };

static const uint8_t k_lightning_palette_r[8] = { 255, 200, 150, 200, 255, 200, 150, 200 };
static const uint8_t k_lightning_palette_g[8] = { 255, 255, 200, 150, 100, 200, 255, 200 };
static const uint8_t k_lightning_palette_b[8] = { 255, 255, 255, 255, 200, 150, 100, 255 };

static const uint8_t k_explosion_palette_r[8] = { 255, 255, 200, 150, 100,  80,  50,  20 };
static const uint8_t k_explosion_palette_g[8] = { 200, 150, 100,  50,  20,   0,   0,   0 };
static const uint8_t k_explosion_palette_b[8] = {   0,   0,   0,   0,   0,   0,   0,   0 };

static const uint8_t k_chaos_palette_r[8]   = { 200, 150, 255, 100, 200, 150, 255, 100 };
static const uint8_t k_chaos_palette_g[8]   = {  50, 100, 150, 200,  50, 100, 150, 200 };
static const uint8_t k_chaos_palette_b[8]   = { 255, 200, 150, 100, 255, 200, 150, 100 };

#define PALETTE_GET(NAME, LIFE_RATIO) do {                    \
    int idx = (int)((LIFE_RATIO) * 7.0f);                     \
    if (idx > 7) idx = 7;                                    \
    r = NAME##_palette_r[idx];                               \
    g = NAME##_palette_g[idx];                               \
    b = NAME##_palette_b[idx];                               \
} while(0)

/* ---- Static state ---- */

static CSB_V2_Particle      g_particles[CSB_V2_VFX_MAX_PARTICLES];
static CSB_V2_Emitter       g_emitters[CSB_V2_VFX_MAX_EMITTERS];
static CSB_V2_ProjectileVFX g_projectiles[CSB_V2_VFX_MAX_PROJECTILES];
static CSB_V2_FieldVFX      g_fields[CSB_V2_VFX_MAX_FIELD];

/* ---- Helpers ---- */

static int palette_for_type(int vfx_type) {
    switch (vfx_type) {
        case CSB_V2_VFX_FIRE:         return 0;
        case CSB_V2_VFX_SMOKE:        return 1;
        case CSB_V2_VFX_SPARK:         return 2;
        case CSB_V2_VFX_MAGICAL_GLOW:  return 3;
        case CSB_V2_VFX_LIGHTNING:     return 4;
        case CSB_V2_VFX_EXPLOSION:     return 5;
        case CSB_V2_VFX_CHAOS_MIST:    return 6;
        default:                       return 0;
    }
}

static void particle_color(int vfx_type, float life_ratio,
                           uint8_t *outR, uint8_t *outG, uint8_t *outB) {
    uint8_t r = 255, g = 255, b = 255;
    switch (vfx_type) {
        case CSB_V2_VFX_FIRE:         PALETTE_GET(k_fire,       life_ratio); break;
        case CSB_V2_VFX_SMOKE:        PALETTE_GET(k_smoke,      life_ratio); break;
        case CSB_V2_VFX_SPARK:        PALETTE_GET(k_spark,     life_ratio); break;
        case CSB_V2_VFX_MAGICAL_GLOW: PALETTE_GET(k_glow,      life_ratio); break;
        case CSB_V2_VFX_LIGHTNING:    PALETTE_GET(k_lightning, life_ratio); break;
        case CSB_V2_VFX_EXPLOSION:   PALETTE_GET(k_explosion, life_ratio); break;
        case CSB_V2_VFX_CHAOS_MIST:  PALETTE_GET(k_chaos,     life_ratio); break;
        default:                      r = 255; g = 255; b = 255; break;
    }
    if (outR) *outR = r;
    if (outG) *outG = g;
    if (outB) *outB = b;
}

/* Deterministic pseudo-random (LCG) — for reproducible particle bursts */
static float det_random(uint32_t *seed) {
    *seed = (*seed * 1664525u + 1013904223u) >> 16;
    return (float)(*seed & 0xFFFFu) / 65535.0f;
}

/* ---- Emitter: spawn particles from an emitter ---- */

static void emit_from_emitter(int ei) {
    CSB_V2_Emitter *em = &g_emitters[ei];
    int i;

    if (!em->active) return;

    /* Find a free particle slot */
    for (i = 0; i < CSB_V2_VFX_MAX_PARTICLES; i++) {
        CSB_V2_Particle *p = &g_particles[i];
        if (p->active) continue;

        /* Spawn particle */
        uint32_t seed = (uint32_t)(i * 7919 + ei * 104729 + (uint32_t)(em->age * 1000.0f));
        float angle = det_random(&seed) * 6.28318530f; /* 0..2π */
        float speed = 0.2f + det_random(&seed) * 0.8f;

        /* VFX-type-specific initial velocity */
        float vx = 0.0f, vy = 0.0f;
        switch (em->vfx_type) {
            case CSB_V2_VFX_FIRE:
                vy = -0.3f - det_random(&seed) * 0.4f;
                vx = (det_random(&seed) - 0.5f) * 0.3f;
                break;
            case CSB_V2_VFX_SMOKE:
                vy = -0.1f - det_random(&seed) * 0.2f;
                vx = (det_random(&seed) - 0.5f) * 0.15f;
                break;
            case CSB_V2_VFX_SPARK:
                vx = cosf(angle) * speed;
                vy = sinf(angle) * speed;
                break;
            case CSB_V2_VFX_MAGICAL_GLOW:
                vy = -0.05f - det_random(&seed) * 0.1f;
                vx = (det_random(&seed) - 0.5f) * 0.2f;
                break;
            case CSB_V2_VFX_LIGHTNING:
                vx = cosf(angle) * speed * 2.0f;
                vy = sinf(angle) * speed * 2.0f;
                break;
            case CSB_V2_VFX_EXPLOSION:
                vx = cosf(angle) * speed * 1.5f;
                vy = sinf(angle) * speed * 1.5f;
                break;
            case CSB_V2_VFX_CHAOS_MIST:
                vy = -0.05f + (det_random(&seed) - 0.5f) * 0.1f;
                vx = (det_random(&seed) - 0.5f) * 0.3f;
                break;
            default:
                vx = cosf(angle) * speed;
                vy = sinf(angle) * speed;
                break;
        }

        /* Life varies by type */
        float max_life;
        switch (em->vfx_type) {
            case CSB_V2_VFX_FIRE:       max_life = 0.4f + det_random(&seed) * 0.3f; break;
            case CSB_V2_VFX_SMOKE:      max_life = 0.8f + det_random(&seed) * 0.6f; break;
            case CSB_V2_VFX_SPARK:      max_life = 0.1f + det_random(&seed) * 0.2f; break;
            case CSB_V2_VFX_MAGICAL_GLOW: max_life = 0.5f + det_random(&seed) * 0.4f; break;
            case CSB_V2_VFX_LIGHTNING:  max_life = 0.05f + det_random(&seed) * 0.1f; break;
            case CSB_V2_VFX_EXPLOSION:  max_life = 0.3f + det_random(&seed) * 0.3f; break;
            case CSB_V2_VFX_CHAOS_MIST: max_life = 0.6f + det_random(&seed) * 0.5f; break;
            default:                     max_life = 0.5f; break;
        }

        float life_ratio = 1.0f;
        uint8_t cr, cg, cb;
        particle_color(em->vfx_type, life_ratio, &cr, &cg, &cb);

        p->x = em->x + (det_random(&seed) - 0.5f) * 0.2f;
        p->y = em->y + (det_random(&seed) - 0.5f) * 0.2f;
        p->vx = vx;
        p->vy = vy;
        p->life = max_life;
        p->max_life = max_life;
        p->size = 2.0f + det_random(&seed) * 4.0f;
        p->rotation = 0.0f;
        p->rotation_speed = (det_random(&seed) - 0.5f) * 3.0f;
        p->r = cr;
        p->g = cg;
        p->b = cb;
        p->alpha = 255;
        p->type = (uint8_t)em->vfx_type;
        p->active = 1;
        break; /* one particle per call */
    }
}

/* ---- Public API ---- */

void csb_v2_vfx_init(void) {
    memset(g_particles, 0, sizeof(g_particles));
    memset(g_emitters, 0, sizeof(g_emitters));
    memset(g_projectiles, 0, sizeof(g_projectiles));
    memset(g_fields, 0, sizeof(g_fields));
}

void csb_v2_vfx_tick(float dtSeconds) {
    int i;

    /* ---- Update emitters ---- */
    for (i = 0; i < CSB_V2_VFX_MAX_EMITTERS; i++) {
        CSB_V2_Emitter *em = &g_emitters[i];
        if (!em->active) continue;

        em->age += dtSeconds;
        if (!em->looping && em->duration > 0.0f && em->age >= em->duration) {
            em->active = 0;
            continue;
        }

        /* Emit particles based on emit_rate */
        em->emit_accum += em->emit_rate * dtSeconds;
        while (em->emit_accum >= 1.0f) {
            emit_from_emitter(i);
            em->emit_accum -= 1.0f;
        }
    }

    /* ---- Update particles ---- */
    for (i = 0; i < CSB_V2_VFX_MAX_PARTICLES; i++) {
        CSB_V2_Particle *p = &g_particles[i];
        if (!p->active) continue;

        p->life -= dtSeconds;
        if (p->life <= 0.0f) {
            p->active = 0;
            continue;
        }

        /* Update position */
        p->x += p->vx * dtSeconds;
        p->y += p->vy * dtSeconds;

        /* Gravity/damping per type */
        switch (p->type) {
            case CSB_V2_VFX_FIRE:
                p->vy -= 0.1f * dtSeconds; /* floats up faster */
                p->vx *= 1.0f - (0.5f * dtSeconds);
                break;
            case CSB_V2_VFX_SMOKE:
                p->vy -= 0.02f * dtSeconds;
                p->vx *= 1.0f - (0.3f * dtSeconds);
                break;
            case CSB_V2_VFX_SPARK:
                p->vy += 0.5f * dtSeconds; /* gravity */
                p->vx *= 1.0f - (1.0f * dtSeconds);
                break;
            case CSB_V2_VFX_MAGICAL_GLOW:
                p->vy -= 0.05f * dtSeconds;
                p->vx *= 1.0f - (0.2f * dtSeconds);
                break;
            case CSB_V2_VFX_LIGHTNING:
                p->vx *= 1.0f - (2.0f * dtSeconds);
                p->vy *= 1.0f - (2.0f * dtSeconds);
                break;
            case CSB_V2_VFX_EXPLOSION:
                p->vx *= 1.0f - (1.5f * dtSeconds);
                p->vy *= 1.0f - (1.5f * dtSeconds);
                break;
            case CSB_V2_VFX_CHAOS_MIST:
                p->vy -= 0.01f * dtSeconds;
                p->vx += sinf(p->x * 10.0f) * 0.02f * dtSeconds;
                break;
            default:
                break;
        }

        p->rotation += p->rotation_speed * dtSeconds;

        /* Fade alpha */
        float life_ratio = p->life / p->max_life;
        if (life_ratio < 0.3f) {
            p->alpha = (uint8_t)((life_ratio / 0.3f) * 255.0f);
        }
    }

    /* ---- Update projectiles ---- */
    for (i = 0; i < CSB_V2_VFX_MAX_PROJECTILES; i++) {
        CSB_V2_ProjectileVFX *proj = &g_projectiles[i];
        if (!proj->active) continue;

        float dx = proj->target_x - proj->x;
        float dy = proj->target_y - proj->y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < 0.001f) {
            proj->active = 0;
            continue;
        }

        /* Advance progress along path */
        float step = (proj->speed * dtSeconds) / dist;
        proj->progress += step;
        if (proj->progress >= 1.0f) {
            proj->x = proj->target_x;
            proj->y = proj->target_y;
            proj->active = 0;
            continue;
        }

        /* Interpolate position */
        float nx = dx / dist;
        float ny = dy / dist;
        float speed_tiles_per_sec = proj->speed;
        proj->x += nx * speed_tiles_per_sec * dtSeconds;
        proj->y += ny * speed_tiles_per_sec * dtSeconds;
    }

    /* ---- Update field VFX ---- */
    for (i = 0; i < CSB_V2_VFX_MAX_FIELD; i++) {
        CSB_V2_FieldVFX *fv = &g_fields[i];
        if (!fv->active) continue;
        fv->timer += dtSeconds;
        if (fv->timer >= 0.15f) {
            fv->timer -= 0.15f;
            fv->frame = (fv->frame + 1) & 7;
        }
    }
}

int csb_v2_vfx_add_emitter(float x, float y,
                           float radius,
                           float emit_rate,
                           int vfx_type,
                           int looping,
                           float duration) {
    int i;
    if (vfx_type <= CSB_V2_VFX_NONE || vfx_type >= CSB_V2_VFX_COUNT) return -1;

    for (i = 0; i < CSB_V2_VFX_MAX_EMITTERS; i++) {
        CSB_V2_Emitter *em = &g_emitters[i];
        if (em->active) continue;

        em->x = x;
        em->y = y;
        em->radius = radius;
        em->emit_rate = emit_rate > 0.0f ? emit_rate : 1.0f;
        em->emit_accum = 0.0f;
        em->vfx_type = (int8_t)vfx_type;
        em->active = 1;
        em->looping = looping ? 1 : 0;
        em->duration = duration;
        em->age = 0.0f;
        return i;
    }
    return -1;
}

void csb_v2_vfx_remove_emitter(int emitter_index) {
    if (emitter_index < 0 || emitter_index >= CSB_V2_VFX_MAX_EMITTERS) return;
    g_emitters[emitter_index].active = 0;
}

int csb_v2_vfx_fire_projectile(float sx, float sy,
                                float tx, float ty,
                                float speed,
                                int vfx_type) {
    int i;
    if (vfx_type <= CSB_V2_VFX_NONE || vfx_type >= CSB_V2_VFX_COUNT) return -1;

    for (i = 0; i < CSB_V2_VFX_MAX_PROJECTILES; i++) {
        CSB_V2_ProjectileVFX *proj = &g_projectiles[i];
        if (proj->active) continue;

        proj->x = sx;
        proj->y = sy;
        proj->target_x = tx;
        proj->target_y = ty;
        proj->speed = speed > 0.0f ? speed : 3.0f;
        proj->progress = 0.0f;
        proj->type = (uint8_t)vfx_type;
        proj->active = 1;

        /* Set colour from VFX type */
        particle_color(vfx_type, 1.0f, &proj->r, &proj->g, &proj->b);
        return i;
    }
    return -1;
}

int csb_v2_vfx_get_projectile(int projectile_index,
                              float *outX, float *outY,
                              int *outType, uint8_t *outAlpha) {
    if (projectile_index < 0 || projectile_index >= CSB_V2_VFX_MAX_PROJECTILES) return 0;
    CSB_V2_ProjectileVFX *proj = &g_projectiles[projectile_index];
    if (!proj->active) return 0;
    if (outX) *outX = proj->x;
    if (outY) *outY = proj->y;
    if (outType) *outType = (int)proj->type;
    /* Alpha fades as progress approaches 1 */
    if (outAlpha) {
        float a = 1.0f - proj->progress;
        *outAlpha = (uint8_t)(a * 255.0f);
    }
    return 1;
}

int csb_v2_vfx_add_field(int tx, int ty, int vfx_type) {
    int i;
    if (tx < 0 || ty < 0) return -1;
    if (vfx_type <= CSB_V2_VFX_NONE || vfx_type >= CSB_V2_VFX_COUNT) return -1;

    for (i = 0; i < CSB_V2_VFX_MAX_FIELD; i++) {
        CSB_V2_FieldVFX *fv = &g_fields[i];
        if (fv->active) continue;
        fv->tile_x = tx;
        fv->tile_y = ty;
        fv->vfx_type = (int8_t)vfx_type;
        fv->active = 1;
        fv->frame = 0;
        fv->timer = 0.0f;
        return i;
    }
    return -1;
}

void csb_v2_vfx_remove_field(int field_index) {
    if (field_index < 0 || field_index >= CSB_V2_VFX_MAX_FIELD) return;
    g_fields[field_index].active = 0;
}

int csb_v2_vfx_get_field(int field_index, uint8_t *outFrame,
                          int *outType, uint8_t *outAlpha) {
    if (field_index < 0 || field_index >= CSB_V2_VFX_MAX_FIELD) return 0;
    CSB_V2_FieldVFX *fv = &g_fields[field_index];
    if (!fv->active) return 0;
    if (outFrame) *outFrame = fv->frame;
    if (outType) *outType = (int)fv->vfx_type;
    /* Animate alpha based on frame */
    if (outAlpha) {
        float phase = (fv->frame & 7) / 8.0f;
        float a = 0.6f + 0.4f * sinf(phase * 6.28318530f);
        *outAlpha = (uint8_t)(a * 255.0f);
    }
    return 1;
}

int csb_v2_vfx_active_particle_count(void) {
    int i, count = 0;
    for (i = 0; i < CSB_V2_VFX_MAX_PARTICLES; i++) {
        if (g_particles[i].active) count++;
    }
    return count;
}

int csb_v2_vfx_active_emitter_count(void) {
    int i, count = 0;
    for (i = 0; i < CSB_V2_VFX_MAX_EMITTERS; i++) {
        if (g_emitters[i].active) count++;
    }
    return count;
}

const char *csb_v2_vfx_source_evidence(void) {
    return
        "CSB V2.2 particle VFX: presentation-only\n"
        "CSBWin/Graphics.cpp: fireball/lightning/explosion rendering\n"
        "CSBWin/Chaos.cpp: spell cast visual triggers\n"
        "V1 projectile hit detection and DSA scripts unaffected\n";
}
