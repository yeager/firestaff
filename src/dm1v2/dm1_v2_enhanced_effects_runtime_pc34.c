#include "dm1_v2_enhanced_effects_runtime_pc34.h"
#include "dm1_v2_lighting_dynamic_pc34.h"
#include "dm1_v2_particle_system_pc34.h"

/*
 * Presentation-only enhanced effects gate.
 *
 * Source-lock anchors:
 * - ReDMCSB PANEL.C:F0337 and DATA.C:359-360 own the source palette light
 *   choice; V2 lighting is a presentation overlay only.
 * - ReDMCSB PROJEXPL.C:43-92,95-165,817-864,987-994 and
 *   DUNVIEW.C:6816-6831 describe field/projectile visual metadata; this
 *   wrapper only advances V2 particles after the render-presentation gate.
 * - dm1_v2_phase_gate_pc34.c cites COORD.C:1721-1722 and DUNVIEW.C:2999-3000:
 *   V2 may present the 224x136 V1 picture only when V2 presentation is
 *   explicit.
 */

static int dm1_v2_enhanced_effects_runtime_render_lights_indexed(
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int viewportX,
    int viewportY,
    unsigned char lightIndex)
{
    int changed = 0;
    int ly;
    for (ly = 0; ly < M11_V2_LIGHT_MAP_SIZE; ++ly) {
        int lx;
        for (lx = 0; lx < M11_V2_LIGHT_MAP_SIZE; ++lx) {
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;
            unsigned int strength;
            int x;
            int y;
            v2_light_get_tile(lx, ly, &r, &g, &b);
            strength = (unsigned int)r + (unsigned int)g + (unsigned int)b;
            if (strength < 96u) {
                continue;
            }
            x = viewportX + (lx * 224) / M11_V2_LIGHT_MAP_SIZE;
            y = viewportY + (ly * 136) / M11_V2_LIGHT_MAP_SIZE;
            if (x >= 0 && x < framebufferWidth &&
                y >= 0 && y < framebufferHeight) {
                framebuffer[y * framebufferWidth + x] = lightIndex;
                ++changed;
            }
        }
    }
    return changed;
}

int dm1_v2_enhanced_effects_runtime_tick(
    const DM1_V2_PhaseGateConfig* gateConfig,
    const DM1_V2_Settings* settings,
    float dt)
{
    DM1_V2_PhaseGateDecision decision =
        dm1_v2_phase_gate_decide(gateConfig, DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    if (!decision.v2PresentationAllowed) {
        return 0;
    }

    v2_particle_tick(dt);

    if (settings && settings->dynamicLightingEnabled) {
        v2_light_tick(dt);
        v22_light_tick(dt);
    }

    return 1;
}

int dm1_v2_enhanced_effects_runtime_render_indexed(
    const DM1_V2_PhaseGateConfig* gateConfig,
    const DM1_V2_Settings* settings,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int viewportX,
    int viewportY)
{
    DM1_V2_PhaseGateDecision decision;
    unsigned char effectIndex = 15u;
    int changed = 0;
    if (!framebuffer || framebufferWidth <= 0 || framebufferHeight <= 0) {
        return 0;
    }
    decision = dm1_v2_phase_gate_decide(
        gateConfig, DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    if (!decision.v2PresentationAllowed) {
        return 0;
    }
    if (settings && !settings->palette_enhanced) {
        effectIndex = 11u;
    }

    /* ReDMCSB owns the source thing/effect ordering in DUNVIEW.C F0115 and
     * PROJEXPL.C F0213/F0220. This is a presentation-only indexed overlay
     * for already-seeded V2 particles; it does not create, move, or materialise
     * source projectiles/explosions. */
    changed += v2_particle_blit_indexed(framebuffer, framebufferWidth,
                                        framebufferHeight, viewportX, viewportY,
                                        effectIndex);
    if (settings && settings->dynamicLightingEnabled) {
        changed += dm1_v2_enhanced_effects_runtime_render_lights_indexed(
            framebuffer, framebufferWidth, framebufferHeight,
            viewportX, viewportY, effectIndex);
    }
    return changed;
}

const char* dm1_v2_enhanced_effects_runtime_source_evidence(void) {
    return "DM1 V2 enhanced effects runtime is gated by "
           "dm1_v2_phase_gate_decide(RENDER_PRESENTATION): COORD.C:1721-1722; "
           "DUNVIEW.C:2999-3000. Lighting remains presentation-only over "
           "PANEL.C:F0337/DATA.C:359-360 palette semantics, and field/projectile "
           "particles remain visual metadata from PROJEXPL.C/DUNVIEW.C.";
}
