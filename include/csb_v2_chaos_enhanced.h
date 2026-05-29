
#ifndef FIRESTAFF_CSB_V2_CHAOS_ENHANCED_H
#define FIRESTAFF_CSB_V2_CHAOS_ENHANCED_H
#include <stdint.h>
#include "csb_v2_phase_gate_pc34.h"

/* Phase gate: all functions in this header belong to
 * CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION.
 * V1 DSA script dispatch (CSBWin/DSA.cpp) is unaffected.
 * See csb_v2_phase_gate_pc34.h Phase 0 rules.
 *
 * CSB V2.2 Enhanced Chaos — visual DSA script feedback
 * V1: DSA scripts run silently.
 * V2.2: visual cues when scripts trigger (glow, sound, particles). */

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
void csb_v2_chaos_render_overlay(float *outR,
                                 float *outG,
                                 float *outB,
                                 float *outAlpha);
const char *csb_v2_chaos_source_evidence(void);
#endif
