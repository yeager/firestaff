
#ifndef FIRESTAFF_CSB_V2_CHAOS_ENHANCED_H
#define FIRESTAFF_CSB_V2_CHAOS_ENHANCED_H
#include <stdint.h>
#include "csb_v2_phase_gate_pc34.h"

/* Phase gate: all functions in this header belong to
 * CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION.
 * V1 DSA script dispatch (CSBWin/DSA.cpp) is unaffected.
 * See csb_v2_phase_gate_pc34.h Phase 0 rules.
 *
 * Retired CSB V2.2 chaos feedback boundary. The former glow, particle and
 * projectile effects were not source-material-backed. Every entry point now
 * preserves V1 ownership and produces no modern visual. */

typedef struct {
    int script_id;
    float glow_alpha;
    uint32_t glow_color;
    int particle_emitter_id;
} CSB_V2_ScriptVisual;

void csb_v2_chaos_init(void);
void csb_v2_chaos_on_trigger(int script_id, int flag_index);
void csb_v2_chaos_tick(float dt);
int csb_v2_chaos_active_count(void);
int csb_v2_chaos_particle_count(void);
int csb_v2_chaos_fire_projectile(float sx, float sy,
                                 float tx, float ty,
                                 float speed,
                                 int vfx_type);
void csb_v2_chaos_render_overlay(float *outR,
                                 float *outG,
                                 float *outB,
                                 float *outAlpha);
const char *csb_v2_chaos_source_evidence(void);
#endif
