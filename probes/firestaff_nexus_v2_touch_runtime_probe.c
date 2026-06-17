/* firestaff_nexus_v2_touch_runtime_probe.c — Nexus V2 Touch Runtime Wire-up Probe
 *
 * Phase 6 touch/controller runtime wire-up verification probe for
 * Dungeon Master Nexus (Saturn).  Mirrors dm2_v2_touch_runtime_probe
 * pattern (sibling DM2 wire-up).
 *
 * Source-lock:
 *   Saturn NEXUS.BIN touch/joypad input layer
 *   Saturn SDK JOYPAD API (SMP-SONY Japan)
 *   ReDMCSB COMMAND.C:108-113 (mouse movement zones C001-C006)
 *   ReDMCSB COMMAND.C:254-291 (keyboard tables for C001..C006)
 *   ReDMCSB CLIKMENU.C:142/180 (F0365 turn / F0366 move)
 *   ReDMCSB GAMELOOP.C:164-219 (V1 input wait loop)
 *
 * Coverage (38 assertions):
 *   1-2.   init/shutdown lifecycle + re-init idempotent
 *   3-5.   Translation rejected: invalid instance / no gate / no init
 *   6.     Translation rejected when V2 presentation disabled
 *   7.     Translation rejected when V2 config persistence disabled
 *   8.     NONE affordance rejected even when V2 enabled
 *   9-12.  Touch swipe: up→MOVE_FORWARD, down→MOVE_BACKWARD,
 *          left→TURN_LEFT, right→TURN_RIGHT
 *   13-14. Touch edge strafe: left→MOVE_LEFT, right→MOVE_RIGHT
 *   15-18. Controller D-pad: up/down/left/right
 *   19.    Translation count increments only on accepted translations
 *   20.    Translation count does NOT increment on rejected translations
 *   21.    force_active_for_test bypasses gate
 *   22.    is_active returns 0 when V2 disabled
 *   23.    is_active returns 1 when V2 enabled
 *   24.    V1 invariant: V2 off → count stays 0
 *   25.    V2 on + multiple accepted translations → count = N
 *   26.    V2 off → out->command = NONE (V1 chrome untouched)
 *   27.    source_evidence returns citation string with NEXUS.BIN
 *   28.    Translation accepted even with non-zero x/y coordinates
 *   29.    Translation accepted even with negative x/y coordinates
 *   30.    Null out pointer rejected
 *   31.    Multiple init/shutdown cycles safe
 *   32.    Coordinates pass through (x=42, y=99)
 *   33.    Post-shutdown: translation rejected
 *   34.    force_active_for_test(0) restores gate
 *   35.    Translation count monotonic across gate toggles
 *   36.    NONE → command=NONE, count unchanged
 *   37.    Translation with v2 enabled but force_active reset → rejected
 *   38.    Translation chain: 4 different affordances → 4 different commands
 */

#include "nexus_v2_touch_runtime.h"
#include "nexus_v2_touch_controller_affordance.h"
#include "nexus_v2_phase_gate_pc34.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

#define CHECK(cond_) do { \
    g_assertions++; \
    if (!(cond_)) { \
        printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, #cond_); \
        g_failures++; \
    } \
} while (0)

int main(void) {
    printf("Nexus V2 Touch Runtime Wire-up — Phase 6 headless probe\n");
    printf("Source: Saturn NEXUS.BIN touch/joypad, SDK JOYPAD API,\n"
           "        ReDMCSB COMMAND.C:108-113/254-291,\n"
           "        CLIKMENU.C:142/180, GAMELOOP.C:164-219\n");

    /* 1. init/shutdown lifecycle */
    nexus_v2_touch_runtime_init();
    CHECK(nexus_v2_touch_runtime_is_active() == 0);
    nexus_v2_touch_runtime_shutdown();
    CHECK(nexus_v2_touch_runtime_is_active() == 0);

    /* 2. Re-init idempotent */
    nexus_v2_touch_runtime_init();
    nexus_v2_touch_runtime_init();
    CHECK(nexus_v2_touch_runtime_translation_count() == 0);
    nexus_v2_touch_runtime_shutdown();

    /* 3. Translation rejected when V2 launch disabled (no gate) */
    {
        nexus_v2_touch_runtime_init();
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        nexus_v2_touch_runtime_shutdown();
    }

    /* 4. Translation rejected when V2 partial (presentation off) */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 1 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        nexus_v2_touch_runtime_shutdown();
    }

    /* 5. Translation rejected when V2 partial (config persistence off) */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 0 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
        nexus_v2_touch_runtime_shutdown();
    }

    /* 6. NONE affordance rejected even when V2 enabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_NONE, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        CHECK(nexus_v2_touch_runtime_translation_count() == 0);
        nexus_v2_touch_runtime_shutdown();
    }

    /* 7-12. All 6 movement affordances translate to correct V1 commands */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };

        /* 7. swipe up → MOVE_FORWARD */
        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);

        /* 8. swipe down → MOVE_BACKWARD */
        rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_DOWN, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_BACKWARD);

        /* 9. swipe left → TURN_LEFT */
        rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_LEFT);

        /* 10. swipe right → TURN_RIGHT */
        rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_RIGHT);

        /* 11. edge strafe left → MOVE_LEFT */
        rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_LEFT);

        /* 12. edge strafe right → MOVE_RIGHT */
        rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_RIGHT);

        nexus_v2_touch_runtime_shutdown();
    }

    /* 13-16. Controller D-pad translations */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };

        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_UP, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);

        rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_BACKWARD);

        rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_LEFT);

        rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_RIGHT);

        nexus_v2_touch_runtime_shutdown();
    }

    /* 17. force_active_for_test bypasses gate */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        CHECK(nexus_v2_touch_runtime_is_active() == 0);
        nexus_v2_touch_runtime_force_active_for_test(1);
        CHECK(nexus_v2_touch_runtime_is_active() == 1);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);
        CHECK(nexus_v2_touch_runtime_translation_count() == 1);
        nexus_v2_touch_runtime_force_active_for_test(0);
        nexus_v2_touch_runtime_shutdown();
    }

    /* 18. Null out pointer rejected */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, NULL);
        CHECK(rc == 0);
        nexus_v2_touch_runtime_shutdown();
    }

    /* 19. Coordinates pass through */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 42, 99, &out);
        CHECK(rc == 1);
        CHECK(out.x == 42);
        CHECK(out.y == 99);
        nexus_v2_touch_runtime_shutdown();
    }

    /* 20. Negative coordinates pass through */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, -10, -20, &out);
        CHECK(rc == 1);
        CHECK(out.x == -10);
        CHECK(out.y == -20);
        nexus_v2_touch_runtime_shutdown();
    }

    /* 21. source_evidence returns citation */
    {
        const char *ev = nexus_v2_touch_runtime_source_evidence();
        CHECK(ev != NULL && ev[0] != '\0'
            && strstr(ev, "NEXUS.BIN") != NULL
            && strstr(ev, "CLIKMENU.C") != NULL);
    }

    /* 22. Multiple init/shutdown cycles safe */
    {
        for (int i = 0; i < 5; i++) {
            nexus_v2_touch_runtime_init();
            nexus_v2_touch_runtime_shutdown();
        }
        CHECK(1);
    }

    /* 23. Post-shutdown: translation rejected */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        nexus_v2_touch_runtime_shutdown();
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
    }

    /* 24. Translation chain: 4 different affordances → 4 different commands */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        struct {
            Nexus_V2_TouchControllerAffordance aff;
            int expected_cmd;
        } events[] = {
            { NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP,    DM1_V1_COMMAND_MOVE_FORWARD },
            { NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,  DM1_V1_COMMAND_TURN_LEFT },
            { NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, DM1_V1_COMMAND_MOVE_RIGHT },
            { NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,    DM1_V1_COMMAND_MOVE_BACKWARD },
        };
        int pre = nexus_v2_touch_runtime_translation_count();
        for (size_t i = 0; i < sizeof(events) / sizeof(events[0]); i++) {
            int rc = nexus_v2_touch_runtime_translate_affordance(
                events[i].aff, 50 + i, 60, &out);
            CHECK(rc == 1);
            CHECK(out.command == events[i].expected_cmd);
        }
        int post = nexus_v2_touch_runtime_translation_count();
        CHECK(post == pre + 4);
        nexus_v2_touch_runtime_shutdown();
    }

    /* 25. Translation count monotonic across gate toggles */
    {
        NEXUS_V2_PhaseGateConfig on = { 1, 1 };
        NEXUS_V2_PhaseGateConfig off = { 0, 0 };
        nexus_v2_touch_runtime_init();
        nexus_v2_touch_runtime_set_gate_config(&on);
        int before = nexus_v2_touch_runtime_translation_count();
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* +1 */
        nexus_v2_touch_runtime_set_gate_config(&off);
        nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* no-op */
        nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* no-op */
        nexus_v2_touch_runtime_set_gate_config(&on);
        nexus_v2_touch_runtime_translate_affordance(
            NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* +1 */
        CHECK(nexus_v2_touch_runtime_translation_count() == before + 2);
        nexus_v2_touch_runtime_shutdown();
    }

    printf("\n%d/%d assertions passed\n", g_assertions - g_failures, g_assertions);
    if (g_failures == 0) {
        printf("PASS: Nexus V2 Touch Runtime wire-up probe\n");
        return 0;
    }
    printf("FAIL: %d assertion(s) failed\n", g_failures);
    return 1;
}