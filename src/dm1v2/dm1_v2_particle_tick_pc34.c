/*
 * v2_particle_tick — DM1 V2 Phase 4 runtime tick bridge.
 *
 * Maps the v2_particle_update() entry point (called by v22_runtime_enhanced_tick
 * via the weak symbol v2_particle_tick) to the actual particle system update.
 * Both signatures are void(float dt); the wrapper is purely a linking resolution
 * so the weak declaration in dm1_v2_runtime_pc34.c resolves to this definition
 * when the firestaff_v2 static library is linked.
 *
 * Source: Firestaff DM1 V2 Phase 4 bridging requirement.
 */

#include "dm1_v2_particle_system_pc34.h"

void v2_particle_tick(float dt) {
    v2_particle_update(dt);
}