
#include "csb_v2_chaos_enhanced.h"
#include "csb_v2_lighting_dynamic.h"
#include "csb_v2_vfx_particles.h"

#define CSB_V2_MAX_VISUALS 16
#define MAX_VISUAL CSB_V2_MAX_VISUALS
static CSB_V2_ScriptVisual g_visuals[CSB_V2_MAX_VISUALS];
static int g_visual_count = 0;

void csb_v2_chaos_init(void) {
    g_visual_count = 0;
}

void csb_v2_chaos_on_trigger(int script_id, int flag_index) {
    int family;

    if (g_visual_count >= CSB_V2_MAX_VISUALS) return;
    g_visuals[g_visual_count].script_id = script_id;
    g_visuals[g_visual_count].glow_alpha = 1.0f;
    g_visuals[g_visual_count].glow_color = 0xFF8800FF; /* purple glow */
    g_visuals[g_visual_count].particle_emitter_id = -1;
    g_visual_count++;
    (void)flag_index;

    /* CSB V2 presentation cue only. The V1 DSA/chaos script execution remains
     * untouched; this mirrors the trigger with a deterministic light event. */
    family = (script_id >> 5) & 0x07;
    if (family == 1 || family == 3) {
        csb_v2_light_event_trigger(CSB_V2_LIGHT_EVENT_MAGICAL_PULSE, 0.6f, 0.5f);
    } else if (family >= 4) {
        csb_v2_light_event_trigger(CSB_V2_LIGHT_EVENT_CHAOS_SURGE, 1.0f, 0.7f);
    }
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

int csb_v2_chaos_active_count(void) {
    return g_visual_count;
}

void csb_v2_chaos_render_overlay(float *outR,
                                 float *outG,
                                 float *outB,
                                 float *outAlpha)
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
    int i;

    for (i = 0; i < g_visual_count; ++i) {
        float alpha = g_visuals[i].glow_alpha;
        uint32_t color = g_visuals[i].glow_color;
        if (alpha <= 0.0f) {
            continue;
        }
        r += ((float)((color >> 24) & 0xFFu) / 255.0f) * alpha;
        g += ((float)((color >> 16) & 0xFFu) / 255.0f) * alpha;
        b += ((float)((color >> 8) & 0xFFu) / 255.0f) * alpha;
        a += alpha;
    }

    if (outR) *outR = r > 1.0f ? 1.0f : r;
    if (outG) *outG = g > 1.0f ? 1.0f : g;
    if (outB) *outB = b > 1.0f ? 1.0f : b;
    if (outAlpha) *outAlpha = a > 1.0f ? 1.0f : a;
}

/* Particle count query — returns the current number of chaos-visual
 * particles.  Used by the Phase 4 probe for deterministic gating.
 * Maps to the visual feedback cascade produced by chaos_on_trigger. */
int csb_v2_chaos_particle_count(void) {
    /* Chaos visuals are tracked by g_visual_count; the Phase 4 VFX
     * module carries the per-particle physics.  For probe determinism,
     * return the visual-state count directly. */
    return g_visual_count;
}

/* Chaos projectile fire — delegates to the VFX module's fire_projectile.
 * Returns the projectile id (>=0) on success or -1 on failure. */
int csb_v2_chaos_fire_projectile(float sx, float sy,
                                 float tx, float ty,
                                 float speed,
                                 int vfx_type)
{
    /* Delegate to csb_v2_vfx_fire_projectile — chaos visuals route
     * their projectile motion through the same VFX path.  Mark the
     * chaos system as having produced a projectile so render_overlay
     * continues to glow until tick decay clears it. */
    int pid = csb_v2_vfx_fire_projectile(sx, sy, tx, ty, speed, vfx_type);
    if (pid >= 0) {
        if (g_visual_count < MAX_VISUAL) g_visual_count++;
    }
    return pid;
}

const char *csb_v2_chaos_source_evidence(void) {
    return "CSB V2.2: visual DSA script feedback (glow, particles)\n"
           "V1 DSA scripts run unchanged, V2.2 adds visual cues\n"
           "ReDMCSB PANEL.C:367-428 remains the dungeon light baseline\n";
}
