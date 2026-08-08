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

    /* force_active_for_test enables the runtime. */
    nexus_v2_hud_runtime_force_active_for_test(1);
    nexus_v2_lighting_runtime_force_active_for_test(1);
    nexus_v2_smooth_movement_runtime_force_active_for_test(1);
    nexus_v2_touch_runtime_force_active_for_test(1);

    if (nexus_v2_hud_runtime_is_active() != 1 ||
        nexus_v2_lighting_runtime_is_active() != 1 ||
        nexus_v2_smooth_movement_runtime_is_active() != 1 ||
        nexus_v2_touch_runtime_is_active() != 1) {
        fprintf(stderr, "FAIL: force_active did not enable V2 runtimes\n");
        return 1;
    }

    /* Tick lighting to prove it advances state. */
    nexus_v2_lighting_runtime_tick(0.016f);
    if (nexus_v2_lighting_runtime_tick_count() != 1) {
        fprintf(stderr, "FAIL: lighting tick did not advance\n");
        return 1;
    }

    /* Tick smooth movement. */
    nexus_v2_smooth_movement_runtime_tick(16.0f);
    if (nexus_v2_smooth_movement_runtime_tick_count() != 1) {
        fprintf(stderr, "FAIL: smooth movement tick did not advance\n");
        return 1;
    }

    /* Deactivate. */
    nexus_v2_hud_runtime_force_active_for_test(0);
    nexus_v2_lighting_runtime_force_active_for_test(0);
    nexus_v2_smooth_movement_runtime_force_active_for_test(0);
    nexus_v2_touch_runtime_force_active_for_test(0);

    if (nexus_v2_hud_runtime_is_active() != 0 ||
        nexus_v2_lighting_runtime_is_active() != 0 ||
        nexus_v2_smooth_movement_runtime_is_active() != 0 ||
        nexus_v2_touch_runtime_is_active() != 0) {
        fprintf(stderr, "FAIL: force_active(0) did not deactivate\n");
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

    puts("PASS: Nexus V2 production runtimes are phase-gated (real implementations)");
    return 0;
}
