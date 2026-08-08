#include <stdio.h>

#include "nexus_v2_hud_runtime.h"
#include "nexus_v2_lighting_runtime.h"
#include "nexus_v2_smooth_movement_runtime.h"
#include "nexus_v2_touch_runtime.h"

int main(void)
{
    const char *hud_ev;
    const char *lighting_ev;
    const char *smooth_ev;
    const char *touch_ev;
    struct Dm1V1QueuedCommandPc34Compat command;

    nexus_v2_hud_runtime_init();
    nexus_v2_lighting_runtime_init();
    nexus_v2_smooth_movement_runtime_init();
    nexus_v2_touch_runtime_init();

    /* Without gate config, all runtimes should be inactive. */
    if (nexus_v2_hud_runtime_is_active() != 0 ||
        nexus_v2_lighting_runtime_is_active() != 0 ||
        nexus_v2_smooth_movement_runtime_is_active() != 0 ||
        nexus_v2_touch_runtime_is_active() != 0) {
        fprintf(stderr, "FAIL: V2 runtimes active without gate config\n");
        return 1;
    }

    command.command = DM1_V1_COMMAND_MOVE_FORWARD;
    command.x = 7;
    command.y = 9;

    /* Test hooks must remain inert in the retail library. */
    nexus_v2_hud_runtime_force_active_for_test(1);
    nexus_v2_lighting_runtime_force_active_for_test(1);
    nexus_v2_smooth_movement_runtime_force_active_for_test(1);
    nexus_v2_touch_runtime_force_active_for_test(1);

    if (nexus_v2_hud_runtime_is_active() != 0 ||
        nexus_v2_lighting_runtime_is_active() != 0 ||
        nexus_v2_smooth_movement_runtime_is_active() != 0 ||
        nexus_v2_touch_runtime_is_active() != 0 ||
        nexus_v2_lighting_runtime_get_state() != NULL ||
        nexus_v2_smooth_movement_runtime_get_state() != NULL ||
        nexus_v2_lighting_runtime_tick_count() != 0 ||
        nexus_v2_smooth_movement_runtime_tick_count() != 0 ||
        nexus_v2_touch_runtime_translation_count() != 0 ||
        nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 7, 9, &command) != 0 ||
        command.command != DM1_V1_COMMAND_NONE || command.x != 7 ||
        command.y != 9) {
        fprintf(stderr, "FAIL: capture-gated V2 route leaked into production\n");
        return 1;
    }

    /* Evidence strings present. */
    hud_ev = nexus_v2_hud_runtime_source_evidence();
    lighting_ev = nexus_v2_lighting_runtime_source_evidence();
    smooth_ev = nexus_v2_smooth_movement_runtime_source_evidence();
    touch_ev = nexus_v2_touch_runtime_source_evidence();
    if (!hud_ev || !lighting_ev || !smooth_ev || !touch_ev ||
        !hud_ev[0] || !lighting_ev[0] || !smooth_ev[0] || !touch_ev[0]) {
        fprintf(stderr, "FAIL: missing source evidence\n");
        return 1;
    }

    nexus_v2_hud_runtime_shutdown();
    nexus_v2_lighting_runtime_shutdown();
    nexus_v2_smooth_movement_runtime_shutdown();
    nexus_v2_touch_runtime_shutdown();

    puts("PASS: Nexus V2 production routes remain capture-gated (fail-closed adapters)");
    return 0;
}
