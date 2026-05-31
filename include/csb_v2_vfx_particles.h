#ifndef FIRESTAFF_CSB_V2_VFX_PARTICLES_H
#define FIRESTAFF_CSB_V2_VFX_PARTICLES_H

#include <stdint.h>

/* Phase gate: belongs to CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION.
 * V1 projectile/chaos logic is unaffected.
 * See csb_v2_phase_gate_pc34.h Phase 0 rules.
 *
 * CSB V2.2 Particle VFX System
 * ============================
 * Presentation-only particle effects for CSB chaos magic.
 * Field effects: fire, smoke, spark, magical glow.
 * Projectile effects: fireball, lightning bolt.
 * Source-lock: CSBWin/Graphics.cpp explosion/projectile rendering.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* VFX particle type — maps to CSB chaos magic spell visuals */
typedef enum {
    CSB_V2_VFX_NONE = 0,
    CSB_V2_VFX_FIRE,          /* flame / fireball core */
    CSB_V2_VFX_SMOKE,         /* rising smoke plume */
    CSB_V2_VFX_SPARK,         /* electrical spark / magic shimmer */
    CSB_V2_VFX_MAGICAL_GLOW,  /* enchanted glow aura */
    CSB_V2_VFX_LIGHTNING,     /* lightning bolt */
    CSB_V2_VFX_EXPLOSION,     /* explosive burst */
    CSB_V2_VFX_CHAOS_MIST,    /* chaos magic swirling mist */
    CSB_V2_VFX_COUNT
} CSB_V2_VFXType;

/* A single particle in a VFX effect */
typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float life;         /* remaining life in seconds */
    float max_life;
    float size;
    float rotation;     /* radians */
    float rotation_speed;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t alpha;
    uint8_t type;       /* CSB_V2_VFXType */
    uint8_t active;
} CSB_V2_Particle;

/* A VFX emitter — source of particles */
typedef struct {
    float x;
    float y;
    float radius;       /* effect radius */
    float emit_rate;    /* particles per second */
    float emit_accum;   /* accumulator for emit rate */
    int8_t vfx_type;    /* CSB_V2_VFXType */
    uint8_t active;
    uint8_t looping;
    float duration;     /* 0 = infinite */
    float age;         /* seconds since activation */
} CSB_V2_Emitter;

/* Projectile VFX state */
typedef struct {
    float x;
    float y;
    float target_x;
    float target_y;
    float speed;        /* tiles per second */
    float progress;     /* 0..1 interpolation along path */
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t type;       /* CSB_V2_VFXType */
    uint8_t active;
} CSB_V2_ProjectileVFX;

/* Field VFX — ambient/area effects on dungeon tiles */
typedef struct {
    int tile_x;
    int tile_y;
    int8_t vfx_type;
    uint8_t active;
    uint8_t frame;
    float timer;
} CSB_V2_FieldVFX;

#define CSB_V2_VFX_MAX_PARTICLES 384
#define CSB_V2_VFX_MAX_EMITTERS  16
#define CSB_V2_VFX_MAX_PROJECTILES 8
#define CSB_V2_VFX_MAX_FIELD      64

/* ---- Public API ---- */

/* Initialise the VFX system */
void csb_v2_vfx_init(void);

/* Tick all active emitters, particles, projectiles, field effects.
 * dtSeconds = elapsed time since last frame. */
void csb_v2_vfx_tick(float dtSeconds);

/* Add a particle emitter at (x,y) in dungeon tile coordinates.
 * radius, emit_rate, vfx_type, looping, duration.
 * Returns emitter index or -1 on failure. */
int csb_v2_vfx_add_emitter(float x, float y,
                           float radius,
                           float emit_rate,
                           int vfx_type,
                           int looping,
                           float duration);

/* Remove an emitter (stops new particles, existing particles live out). */
void csb_v2_vfx_remove_emitter(int emitter_index);

/* Trigger a projectile VFX from (sx,sy) to (tx,ty).
 * speed in tiles/second, vfx_type.
 * Returns projectile index or -1. */
int csb_v2_vfx_fire_projectile(float sx, float sy,
                                float tx, float ty,
                                float speed,
                                int vfx_type);

/* Query projectile position for rendering.
 * Fills *outX,*outY in dungeon tile coords, *outType as VFX type,
 * *outAlpha 0..255. Returns 1 if active, 0 if done. */
int csb_v2_vfx_get_projectile(int projectile_index,
                              float *outX, float *outY,
                              int *outType, uint8_t *outAlpha);

/* Place a field VFX on dungeon tile (tx,ty).
 * Returns field VFX index or -1. */
int csb_v2_vfx_add_field(int tx, int ty, int vfx_type);

/* Remove a field VFX */
void csb_v2_vfx_remove_field(int field_index);

/* Query field VFX — returns 1 if active, fills *outFrame (0..7).
 * Useful for tile animation cycling. */
int csb_v2_vfx_get_field(int field_index, uint8_t *outFrame,
                          int *outType, uint8_t *outAlpha);

/* Returns number of active particles (for debugging/stats). */
int csb_v2_vfx_active_particle_count(void);

/* Returns number of active emitters. */
int csb_v2_vfx_active_emitter_count(void);

/* Source evidence string */
const char *csb_v2_vfx_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_VFX_PARTICLES_H */
