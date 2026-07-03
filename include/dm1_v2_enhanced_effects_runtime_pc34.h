#ifndef FIRESTAFF_DM1_V2_ENHANCED_EFFECTS_RUNTIME_PC34_H
#define FIRESTAFF_DM1_V2_ENHANCED_EFFECTS_RUNTIME_PC34_H

#include "dm1_v2_phase_gate_pc34.h"
#include "dm1_v2_settings_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

int dm1_v2_enhanced_effects_runtime_tick(
    const DM1_V2_PhaseGateConfig* gateConfig,
    const DM1_V2_Settings* settings,
    float dt);
int dm1_v2_enhanced_effects_runtime_render_indexed(
    const DM1_V2_PhaseGateConfig* gateConfig,
    const DM1_V2_Settings* settings,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int viewportX,
    int viewportY);
const char* dm1_v2_enhanced_effects_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_ENHANCED_EFFECTS_RUNTIME_PC34_H */
