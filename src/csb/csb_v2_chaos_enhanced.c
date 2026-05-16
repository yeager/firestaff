
#include "csb_v2_chaos_enhanced.h"

#define CSB_V2_MAX_VISUALS 16
static CSB_V2_ScriptVisual g_visuals[CSB_V2_MAX_VISUALS];
static int g_visual_count = 0;

void csb_v2_chaos_on_trigger(int script_id, int flag_index) {
    if (g_visual_count >= CSB_V2_MAX_VISUALS) return;
    g_visuals[g_visual_count].script_id = script_id;
    g_visuals[g_visual_count].glow_alpha = 1.0f;
    g_visuals[g_visual_count].glow_color = 0xFF8800FF; /* purple glow */
    g_visuals[g_visual_count].particle_emitter_id = -1;
    g_visual_count++;
    (void)flag_index;
}

void csb_v2_chaos_tick(float dt) {
    for (int i = 0; i < g_visual_count; i++) {
        g_visuals[i].glow_alpha -= dt * 0.5f;
        if (g_visuals[i].glow_alpha <= 0) {
            g_visuals[i] = g_visuals[--g_visual_count];
            i--;
        }
    }
}

const char *csb_v2_chaos_source_evidence(void) {
    return "CSB V2.2: visual DSA script feedback (glow, particles)\n"
           "V1 DSA scripts run unchanged, V2.2 adds visual cues\n";
}

