/* Nexus V2 smooth-movement production boundary.
 *
 * The ease/interpolation implementation remains available to explicit
 * probes, but the supplied Saturn corpus has no authenticated presentation
 * timing or camera consumer.  Keep M11's lifecycle ABI and expose no
 * synthetic viewport interpolation from the production Nexus library.
 */

#include "nexus_v2_smooth_movement_runtime.h"
#include <stddef.h>

void nexus_v2_smooth_movement_runtime_init(void) {}
void nexus_v2_smooth_movement_runtime_shutdown(void) {}
void nexus_v2_smooth_movement_runtime_set_gate_config(
    const NEXUS_V2_PhaseGateConfig *config) { (void)config; }
void nexus_v2_smooth_movement_runtime_tick(float dt_ms) { (void)dt_ms; }
int nexus_v2_smooth_movement_runtime_is_active(void) { return 0; }
const Nexus_V2_SmoothState *nexus_v2_smooth_movement_runtime_get_state(void)
{
    return NULL;
}
void nexus_v2_smooth_movement_runtime_start_walk(
    float from_x, float from_y, float to_x, float to_y)
{
    (void)from_x; (void)from_y; (void)to_x; (void)to_y;
}
void nexus_v2_smooth_movement_runtime_start_turn(
    float from_angle, float to_angle)
{
    (void)from_angle; (void)to_angle;
}
void nexus_v2_smooth_movement_runtime_start_stairs(
    float from_x, float from_y, float to_x, float to_y,
    float from_vert, float to_vert)
{
    (void)from_x; (void)from_y; (void)to_x; (void)to_y;
    (void)from_vert; (void)to_vert;
}
void nexus_v2_smooth_movement_runtime_force_active_for_test(int active)
{
    (void)active;
}
int nexus_v2_smooth_movement_runtime_tick_count(void) { return 0; }
const char *nexus_v2_smooth_movement_runtime_source_evidence(void)
{
    return "Nexus V2 smooth-movement production route blocked: Saturn "
           "presentation timing/camera owner unverified; procedural "
           "interpolation is probe-only";
}
