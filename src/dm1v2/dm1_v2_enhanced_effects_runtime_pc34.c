#include "dm1_v2_enhanced_effects_runtime_pc34.h"

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

    (void)settings;
    (void)dt;
    /* ReDMCSB advances source projectiles, explosions and palette light.
     * Generated V2 particles/lighting are not advanced here. */

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
    int changed = 0;
    if (!framebuffer || framebufferWidth <= 0 || framebufferHeight <= 0) {
        return 0;
    }
    decision = dm1_v2_phase_gate_decide(
        gateConfig, DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    if (!decision.v2PresentationAllowed) {
        return 0;
    }
    (void)settings;
    (void)viewportX;
    (void)viewportY;
    /* No generated indexed pixels are admitted. Authenticated V1
     * projectile/explosion and F0337 palette routes own this surface. */
    return changed;
}

const char* dm1_v2_enhanced_effects_runtime_source_evidence(void) {
    return "DM1 V2 enhanced effects runtime is gated by "
           "dm1_v2_phase_gate_decide(RENDER_PRESENTATION): COORD.C:1721-1722; "
           "DUNVIEW.C:2999-3000. Lighting remains presentation-only over "
           "PANEL.C:F0337/DATA.C:359-360 palette semantics, and field/projectile "
           "particles remain visual metadata from PROJEXPL.C/DUNVIEW.C.";
}
