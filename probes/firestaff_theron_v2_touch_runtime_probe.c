#include "theron_v2_touch_runtime.h"
#include <stdio.h>
#include <string.h>

static int g_failed = 0;
static int g_total = 0;

#define CHECK(cond) do { \
    g_total++; \
    if (!(cond)) { \
        g_failed++; \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static void p_phase_gate(void)
{
    THERON_V2_PhaseGateConfig off = { 0, 0 };
    THERON_V2_PhaseGateConfig on = { 1, 1 };
    struct Dm1V1QueuedCommandPc34Compat out;

    theron_v2_touch_runtime_init();
    theron_v2_touch_runtime_set_gate_config(&off);
    CHECK(theron_v2_touch_runtime_is_active() == 0);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 72, 72, &out) == 0);
    CHECK(out.command == DM1_V1_COMMAND_NONE);
    CHECK(theron_v2_touch_runtime_translation_count() == 0);

    theron_v2_touch_runtime_set_gate_config(&on);
    CHECK(theron_v2_touch_runtime_is_active() == 1);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 72, 72, &out) == 1);
    CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);
    CHECK(out.x == 72);
    CHECK(out.y == 72);
    CHECK(theron_v2_touch_runtime_translation_count() == 1);
    theron_v2_touch_runtime_shutdown();
}

static void p_command_mapping(void)
{
    THERON_V2_PhaseGateConfig on = { 1, 1 };
    struct Dm1V1QueuedCommandPc34Compat out;

    theron_v2_touch_runtime_init();
    theron_v2_touch_runtime_set_gate_config(&on);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_DOWN, 64, 64, &out) == 1);
    CHECK(out.command == DM1_V1_COMMAND_MOVE_BACKWARD);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, 64, 64, &out) == 1);
    CHECK(out.command == DM1_V1_COMMAND_TURN_LEFT);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, 64, 64, &out) == 1);
    CHECK(out.command == DM1_V1_COMMAND_MOVE_RIGHT);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT, 64, 64, &out) == 1);
    CHECK(out.command == DM1_V1_COMMAND_MOVE_LEFT);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT, 64, 64, &out) == 1);
    CHECK(out.command == DM1_V1_COMMAND_TURN_RIGHT);
    CHECK(theron_v2_touch_runtime_translation_count() == 5);
    theron_v2_touch_runtime_shutdown();
}

static void p_hud_chrome_gate(void)
{
    THERON_V2_PhaseGateConfig on = { 1, 1 };
    struct Dm1V1QueuedCommandPc34Compat out;

    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(12, 12) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(12, 24) == 0);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(12, 184) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(12, 192) == 0);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(12, 208) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(12, 222) == 0);

    theron_v2_touch_runtime_init();
    theron_v2_touch_runtime_set_gate_config(&on);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 12, 12, &out) == 0);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 12, 184, &out) == 0);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 12, 208, &out) == 0);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_CONTROLLER_DPAD_UP, 12, 12, &out) == 1);
    CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);
    CHECK(theron_v2_touch_runtime_translation_count() == 1);
    theron_v2_touch_runtime_shutdown();
}

static void p_determinism(void)
{
    THERON_V2_PhaseGateConfig on = { 1, 1 };
    int i;
    int accepted = 0;

    theron_v2_touch_runtime_init();
    theron_v2_touch_runtime_set_gate_config(&on);
    for (i = 0; i < 50; ++i) {
        struct Dm1V1QueuedCommandPc34Compat out;
        int rc = theron_v2_touch_runtime_translate_affordance(
            THERON_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT, 40, 80, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_LEFT);
        accepted++;
    }
    CHECK(theron_v2_touch_runtime_translation_count() == accepted);
    theron_v2_touch_runtime_shutdown();
}

static void p_source_evidence(void)
{
    const char *ev = theron_v2_touch_runtime_source_evidence();
    CHECK(ev != NULL);
    CHECK(strstr(ev, "THQUEST.ASM") != NULL);
    CHECK(strstr(ev, "COMMAND.C:2045-2155") != NULL);
    CHECK(strstr(ev, "V2 invariant") != NULL);
}

int main(void)
{
    printf("Theron V2 Touch Runtime Probe - Phase 6\n");
    p_phase_gate();
    p_command_mapping();
    p_hud_chrome_gate();
    p_determinism();
    p_source_evidence();
    printf("theron_v2_touch_runtime_probe: %d checks, %d failures\n", g_total, g_failed);
    return g_failed ? 1 : 0;
}
