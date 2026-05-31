
#ifndef NEXUS_V2_ATMOSPHERE_H
#define NEXUS_V2_ATMOSPHERE_H
#include <stdint.h>

/* Atmospheric effects for Nexus V2.2.
 * - Distance fog (depth-based darkening)
 * - Ambient occlusion (corner/edge darkening)
 * - Color grading (dungeon-level tint) */

typedef struct {
    float fog_start;      /* start distance (squares) */
    float fog_end;        /* full fog distance */
    float fog_r, fog_g, fog_b;  /* fog color */
    float ao_strength;    /* 0-1 */
    float tint_r, tint_g, tint_b;  /* level color tint */
} Nexus_V2_Atmosphere;

void nexus_v2_atmosphere_init(Nexus_V2_Atmosphere *atm, int level_index);
void nexus_v2_apply_fog(uint32_t *rgba, int w, int h, const Nexus_V2_Atmosphere *atm);
void nexus_v2_apply_ao(uint32_t *rgba, int w, int h, float strength);
const char *nexus_v2_atmosphere_source_evidence(void);

#endif

