/* Nexus V2 lighting production boundary.
 *
 * The procedural lighting implementation remains available to explicit
 * probes, but the supplied Saturn corpus has no authenticated VDP1/VDP2
 * lighting consumer.  Keep M11's lifecycle ABI and expose no state or
 * framebuffer-affecting route from the production Nexus library.
 */

#include "nexus_v2_lighting_runtime.h"
#include <stddef.h>

void nexus_v2_lighting_runtime_init(void) {}
void nexus_v2_lighting_runtime_shutdown(void) {}
void nexus_v2_lighting_runtime_set_gate_config(
    const NEXUS_V2_PhaseGateConfig *config) { (void)config; }
void nexus_v2_lighting_runtime_tick(float dt_seconds) { (void)dt_seconds; }
int nexus_v2_lighting_runtime_is_active(void) { return 0; }
const Nexus_V2_LightingState *nexus_v2_lighting_runtime_get_state(void)
{
    return NULL;
}
void nexus_v2_lighting_runtime_force_active_for_test(int active)
{
    (void)active;
}
int nexus_v2_lighting_runtime_tick_count(void) { return 0; }
const char *nexus_v2_lighting_runtime_source_evidence(void)
{
    return "Nexus V2 lighting production route blocked: Saturn VDP1/VDP2 "
           "lighting owner unverified; procedural implementation is probe-only";
}
