/* Nexus V2 touch/controller production boundary.
 *
 * The affordance mapper remains available to explicit probes, but the
 * supplied Saturn corpus has no authenticated NEXUS.BIN input consumer.
 * Do not inject a sibling-DM1 command mapping into the production Nexus
 * queue; retain M11's ABI as a fail-closed no-op until Saturn evidence exists.
 */

#include "nexus_v2_touch_runtime.h"
#include <stddef.h>

void nexus_v2_touch_runtime_init(void) {}
void nexus_v2_touch_runtime_shutdown(void) {}
void nexus_v2_touch_runtime_set_gate_config(
    const NEXUS_V2_PhaseGateConfig *config) { (void)config; }
int nexus_v2_touch_runtime_translate_affordance(
    Nexus_V2_TouchControllerAffordance aff,
    int x, int y,
    struct Dm1V1QueuedCommandPc34Compat *out)
{
    (void)aff;
    if (out) {
        out->command = DM1_V1_COMMAND_NONE;
        out->x = x;
        out->y = y;
    }
    return 0;
}
int nexus_v2_touch_runtime_is_active(void) { return 0; }
void nexus_v2_touch_runtime_force_active_for_test(int active)
{
    (void)active;
}
int nexus_v2_touch_runtime_translation_count(void) { return 0; }
const char *nexus_v2_touch_runtime_source_evidence(void)
{
    return "Nexus V2 touch/controller production route blocked: Saturn "
           "NEXUS.BIN input consumer unverified; affordance mapper is probe-only";
}
