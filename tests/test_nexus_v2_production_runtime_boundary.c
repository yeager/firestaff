#include <stdio.h>

#include "nexus_v2_lighting_runtime.h"
#include "nexus_v2_smooth_movement_runtime.h"

int main(void)
{
    const char *lighting_evidence;
    const char *smooth_evidence;

    nexus_v2_lighting_runtime_init();
    nexus_v2_smooth_movement_runtime_init();
    nexus_v2_lighting_runtime_force_active_for_test(1);
    nexus_v2_smooth_movement_runtime_force_active_for_test(1);

    if (nexus_v2_lighting_runtime_is_active() != 0 ||
        nexus_v2_lighting_runtime_get_state() != NULL ||
        nexus_v2_lighting_runtime_tick_count() != 0 ||
        nexus_v2_smooth_movement_runtime_is_active() != 0 ||
        nexus_v2_smooth_movement_runtime_get_state() != NULL ||
        nexus_v2_smooth_movement_runtime_tick_count() != 0) {
        fprintf(stderr, "FAIL: Nexus V2 procedural runtime leaked into production\n");
        return 1;
    }
    lighting_evidence = nexus_v2_lighting_runtime_source_evidence();
    smooth_evidence = nexus_v2_smooth_movement_runtime_source_evidence();
    if (!lighting_evidence || !smooth_evidence ||
        !lighting_evidence[0] || !smooth_evidence[0]) {
        fprintf(stderr, "FAIL: production boundary lacks source evidence\n");
        return 1;
    }
    puts("PASS: Nexus V2 lighting and smooth-movement production routes are blocked");
    return 0;
}
