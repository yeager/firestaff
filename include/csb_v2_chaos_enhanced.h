
#ifndef FIRESTAFF_CSB_V2_CHAOS_ENHANCED_H
#define FIRESTAFF_CSB_V2_CHAOS_ENHANCED_H
#include <stdint.h>

/* CSB V2.2 Enhanced Chaos — visual DSA script feedback
 * V1: DSA scripts run silently.
 * V2.2: visual cues when scripts trigger (glow, sound, particles). */

typedef struct {
    int script_id;
    float glow_alpha;
    uint32_t glow_color;
    int particle_emitter_id;
} CSB_V2_ScriptVisual;

void csb_v2_chaos_on_trigger(int script_id, int flag_index);
void csb_v2_chaos_tick(float dt);
const char *csb_v2_chaos_source_evidence(void);
#endif

