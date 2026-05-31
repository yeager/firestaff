
#ifndef NEXUS_V2_PARTICLES_H
#define NEXUS_V2_PARTICLES_H
#include <stdint.h>

/* Particle system for Nexus V2.2.
 * Dust, sparks, magic trails, torch embers, water drips. */

#define NEXUS_MAX_PARTICLES 512

typedef enum {
    NEXUS_PART_DUST = 0,
    NEXUS_PART_SPARK,
    NEXUS_PART_MAGIC,
    NEXUS_PART_EMBER,
    NEXUS_PART_WATER_DRIP,
    NEXUS_PART_BLOOD,
    NEXUS_PART_SMOKE,
} Nexus_ParticleType;

typedef struct {
    float x, y, z;
    float vx, vy, vz;
    float life, max_life;
    uint8_t color;
    Nexus_ParticleType type;
    int active;
} Nexus_Particle;

typedef struct {
    Nexus_Particle particles[NEXUS_MAX_PARTICLES];
    int count;
} Nexus_V2_ParticleSystem;

void nexus_v2_particles_init(Nexus_V2_ParticleSystem *ps);
void nexus_v2_particles_tick(Nexus_V2_ParticleSystem *ps, float dt);
void nexus_v2_particles_emit(Nexus_V2_ParticleSystem *ps,
    Nexus_ParticleType type, float x, float y, float z, int count);
void nexus_v2_particles_render(Nexus_V2_ParticleSystem *ps,
    uint32_t *rgba, int w, int h, const uint32_t *palette,
    float cam_x, float cam_z, int cam_dir);
const char *nexus_v2_particles_source_evidence(void);

#endif

