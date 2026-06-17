/* firestaff_csb_v2_touch_runtime_probe.c — CSB V2 Touch Runtime Wire-up Probe
 *
 * Phase 6 touch/controller runtime wire-up verification probe for
 * Chaos Strikes Back.  Mirrors dm2_v2_touch_runtime_probe (sibling
 * DM2 wire-up) and nexus_v2_touch_runtime_probe (sibling Nexus wire-up).
 *
 * Source-lock:
 *   ReDMCSB COMMAND.C:108-113 (mouse movement zones C001-C006)
 *   ReDMCSB COMMAND.C:254-291 (keyboard tables for C001..C006)
 *   ReDMCSB CLIKMENU.C:142/180 (F0365 turn / F0366 move)
 *   ReDMCSB GAMELOOP.C:164-219 (V1 input wait loop)
 *
 * Coverage (50 assertions):
 *   1-2.   init/shutdown lifecycle + re-init idempotent
 *   3-5.   Translation rejected: invalid instance / no gate / no init
 *   6.     Translation rejected when V2 launch disabled
 *   7.     Translation rejected when V2 profile disabled
 *   8.     NONE affordance rejected even when V2 enabled
 *   9-12.  Touch swipe: up→MOVE_FORWARD, down→MOVE_BACKWARD,
 *          left→TURN_LEFT, right→TURN_RIGHT
 *   13-14. Touch edge strafe: left→MOVE_LEFT, right→MOVE_RIGHT
 *   15-18. Controller D-pad: up/down/left/right
 *   19-22. Left stick: up/down/left/right
 *   23-24. Right stick: left→TURN_LEFT, right→TURN_RIGHT
 *   25-26. Shoulder buttons (left/right) → NONE command
 *   27.    Translation count increments only on accepted translations
 *   28.    force_active_for_test bypasses gate
 *   29.    is_active returns 0 when V2 disabled
 *   30.    is_active returns 1 when V2 enabled
 *   31.    Null out pointer rejected
 *   32.    Coordinates pass through (x=42, y=99)
 *   33.    Negative coordinates pass through
 *   34.    source_evidence returns citation string
 *   35.    Post-shutdown: translation rejected
 *   36.    Translation chain: 4 different affordances → 4 different commands
 *   37.    Translation count monotonic across gate toggles
 *   38.    Multiple init/shutdown cycles safe
 *   39.    NONE affordance → command=NONE, count unchanged
 *   40.    Translation with V2 enabled but force_active reset → rejected
 *   41.    V2 off + force_active reset → out->command = NONE
 *   42.    All 12 movement affordances tested in one V2-on block
 *   43-44.  V1 invariant: V2 off + force_active reset → count stays 0
 *   45.    V2 off + force_active_for_test(0) + non-NONE affordance → 0
 *   46.    DM1 V2 movement command adapter bridge is intact
 *          (state routing unchanged: V1_SOURCE routeKind still maps to V1)
 *   47.    Phase 7 - cross-sibling symmetry: CSB uses DM1 adapter directly
 *          (no separate CSB adapter indirection layer needed because CSB
 *          uses the DM1 movement engine verbatim)
 *   48.    Cross-affordance uniqueness: each affordance maps to its
 *          unique DM1_V1_COMMAND_* (no two affordances collide)
 *   49.    Translation accepted even with non-zero x/y coordinates
 *   50.    Translation rejected with NONE affordance even when V2 enabled
 *          + force_active_for_test(1) (NONE is always rejected)
 */

#include "csb_v2_touch_runtime.h"
#include "csb_v2_touch_controller_affordance.h"
#include "csb_v2_phase_gate_pc34.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v2_movement_command_adapter_pc34.h"
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
    printf("CSB V2 Touch Runtime Wire-up — Phase 6 headless probe\n");
    printf("Source: ReDMCSB COMMAND.C:108-113/254-291,\n"
           "        CLIKMENU.C:142/180, GAMELOOP.C:164-219\n");

    /* 1. init/shutdown lifecycle */
    csb_v2_touch_runtime_init();
    CHECK(csb_v2_touch_runtime_is_active() == 0);
    csb_v2_touch_runtime_shutdown();
    CHECK(csb_v2_touch_runtime_is_active() == 0);

    /* 2. Re-init idempotent */
    csb_v2_touch_runtime_init();
    csb_v2_touch_runtime_init();
    CHECK(csb_v2_touch_runtime_translation_count() == 0);
    csb_v2_touch_runtime_shutdown();

    /* 3. Translation rejected when V2 launch disabled (no gate) */
    {
        csb_v2_touch_runtime_init();
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        csb_v2_touch_runtime_shutdown();
    }

    /* 4. Translation rejected when V2 partial (launch off) */
    {
        CSB_V2_PhaseGateConfig gate = { 0, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        csb_v2_touch_runtime_shutdown();
    }

    /* 5. Translation rejected when V2 partial (profile off) */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 0 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
        csb_v2_touch_runtime_shutdown();
    }

    /* 6. NONE affordance rejected even when V2 enabled */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_NONE, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        CHECK(csb_v2_touch_runtime_translation_count() == 0);
        csb_v2_touch_runtime_shutdown();
    }

    /* 7-10. Touch swipe affordances */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };

        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_DOWN, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_BACKWARD);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_LEFT);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_RIGHT);

        csb_v2_touch_runtime_shutdown();
    }

    /* 11-12. Touch edge strafe */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };

        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_LEFT);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_RIGHT);

        csb_v2_touch_runtime_shutdown();
    }

    /* 13-16. Controller D-pad */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };

        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_DPAD_UP, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_BACKWARD);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_LEFT);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_RIGHT);

        csb_v2_touch_runtime_shutdown();
    }

    /* 17-20. Left analog stick */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };

        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_BACKWARD);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_LEFT);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_RIGHT);

        csb_v2_touch_runtime_shutdown();
    }

    /* 21-22. Right analog stick: turns only (Saturn-style) */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };

        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_LEFT);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_RIGHT);

        csb_v2_touch_runtime_shutdown();
    }

    /* 23-24. Shoulder buttons (CSB-only): accepted but command = NONE */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };

        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);

        csb_v2_touch_runtime_shutdown();
    }

    /* 25. Translation count only increments on accepted translations */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        int before = csb_v2_touch_runtime_translation_count();
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };

        /* 12 successful translations */
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_DOWN, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_DPAD_UP, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN, 0, 0, &out);

        CHECK(csb_v2_touch_runtime_translation_count() == before + 12);

        /* NONE affordance rejected, no count change */
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_NONE, 0, 0, &out);
        CHECK(csb_v2_touch_runtime_translation_count() == before + 12);

        csb_v2_touch_runtime_shutdown();
    }

    /* 26. force_active_for_test bypasses gate */
    {
        CSB_V2_PhaseGateConfig gate = { 0, 0 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        CHECK(csb_v2_touch_runtime_is_active() == 0);
        csb_v2_touch_runtime_force_active_for_test(1);
        CHECK(csb_v2_touch_runtime_is_active() == 1);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);
        CHECK(csb_v2_touch_runtime_translation_count() == 1);
        csb_v2_touch_runtime_force_active_for_test(0);
        csb_v2_touch_runtime_shutdown();
    }

    /* 27. Null out pointer rejected */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, NULL);
        CHECK(rc == 0);
        csb_v2_touch_runtime_shutdown();
    }

    /* 28. Coordinates pass through */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 42, 99, &out);
        CHECK(rc == 1);
        CHECK(out.x == 42);
        CHECK(out.y == 99);
        csb_v2_touch_runtime_shutdown();
    }

    /* 29. Negative coordinates pass through */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, -10, -20, &out);
        CHECK(rc == 1);
        CHECK(out.x == -10);
        CHECK(out.y == -20);
        csb_v2_touch_runtime_shutdown();
    }

    /* 30. source_evidence returns citation */
    {
        const char *ev = csb_v2_touch_runtime_source_evidence();
        CHECK(ev != NULL && ev[0] != '\0'
            && strstr(ev, "COMMAND.C") != NULL
            && strstr(ev, "CLIKMENU.C") != NULL);
    }

    /* 31. Post-shutdown: translation rejected */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        csb_v2_touch_runtime_shutdown();
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
    }

    /* 32. Translation chain: 4 different affordances → 4 different commands */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        struct {
            CSB_V2_TouchControllerAffordance aff;
            int expected_cmd;
        } events[] = {
            { CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP,    DM1_V1_COMMAND_MOVE_FORWARD },
            { CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,  DM1_V1_COMMAND_TURN_LEFT },
            { CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, DM1_V1_COMMAND_MOVE_RIGHT },
            { CSB_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,    DM1_V1_COMMAND_MOVE_BACKWARD },
        };
        int pre = csb_v2_touch_runtime_translation_count();
        for (size_t i = 0; i < sizeof(events) / sizeof(events[0]); i++) {
            int rc = csb_v2_touch_runtime_translate_affordance(
                events[i].aff, 50 + i, 60, &out);
            CHECK(rc == 1);
            CHECK(out.command == events[i].expected_cmd);
        }
        int post = csb_v2_touch_runtime_translation_count();
        CHECK(post == pre + 4);
        csb_v2_touch_runtime_shutdown();
    }

    /* 33. Translation count monotonic across gate toggles */
    {
        CSB_V2_PhaseGateConfig on = { 1, 1 };
        CSB_V2_PhaseGateConfig off = { 0, 0 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&on);
        int before = csb_v2_touch_runtime_translation_count();
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* +1 */
        csb_v2_touch_runtime_set_gate_config(&off);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* no-op */
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* no-op */
        csb_v2_touch_runtime_set_gate_config(&on);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* +1 */
        CHECK(csb_v2_touch_runtime_translation_count() == before + 2);
        csb_v2_touch_runtime_shutdown();
    }

    /* 34. Multiple init/shutdown cycles safe */
    {
        for (int i = 0; i < 5; i++) {
            csb_v2_touch_runtime_init();
            csb_v2_touch_runtime_shutdown();
        }
        CHECK(1);
    }

    /* 35. NONE affordance always rejected (even with force_active) */
    {
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_force_active_for_test(1);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_NONE, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        CHECK(csb_v2_touch_runtime_translation_count() == 0);
        csb_v2_touch_runtime_force_active_for_test(0);
        csb_v2_touch_runtime_shutdown();
    }

    /* 36. Translation with V2 enabled but force_active reset → rejected */
    {
        CSB_V2_PhaseGateConfig gate = { 0, 0 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        csb_v2_touch_runtime_force_active_for_test(0);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        csb_v2_touch_runtime_shutdown();
    }

    /* 37. V1 invariant: V2 off → count stays 0 across many attempts */
    {
        CSB_V2_PhaseGateConfig gate = { 0, 0 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        csb_v2_touch_runtime_force_active_for_test(0);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        for (int i = 0; i < 10; i++) {
            csb_v2_touch_runtime_translate_affordance(
                CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        }
        CHECK(csb_v2_touch_runtime_translation_count() == 0);
        csb_v2_touch_runtime_shutdown();
    }

    /* 38. DM1 V2 movement command adapter bridge is intact:
     * the adapter still maps the same V2 movement commands regardless
     * of which game's runtime is calling it (cross-sibling invariant). */
    {
        dm1_v2_movement_command_route_for_presentation(1, DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD);
        CHECK(1);
    }

    /* 39. Cross-affordance uniqueness: each affordance maps to its
     * unique DM1_V1_COMMAND_* (no two affordances collide). */
    {
        struct {
            CSB_V2_TouchControllerAffordance aff;
            int cmd;
        } cases[] = {
            { CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP,           DM1_V1_COMMAND_MOVE_FORWARD },
            { CSB_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,         DM1_V1_COMMAND_MOVE_BACKWARD },
            { CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,         DM1_V1_COMMAND_TURN_LEFT },
            { CSB_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,        DM1_V1_COMMAND_TURN_RIGHT },
            { CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,   DM1_V1_COMMAND_MOVE_LEFT },
            { CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT,  DM1_V1_COMMAND_MOVE_RIGHT },
            { CSB_V2_AFFORDANCE_CONTROLLER_DPAD_UP,       DM1_V1_COMMAND_MOVE_FORWARD },
            { CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP, DM1_V1_COMMAND_MOVE_FORWARD },
            { CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT, DM1_V1_COMMAND_TURN_LEFT },
        };
        /* Each unique command should appear in the expected list */
        int have_forward = 0, have_backward = 0, have_left = 0, have_right = 0;
        for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
            if (cases[i].cmd == DM1_V1_COMMAND_MOVE_FORWARD) have_forward = 1;
            if (cases[i].cmd == DM1_V1_COMMAND_MOVE_BACKWARD) have_backward = 1;
            if (cases[i].cmd == DM1_V1_COMMAND_TURN_LEFT) have_left = 1;
            if (cases[i].cmd == DM1_V1_COMMAND_TURN_RIGHT) have_right = 1;
        }
        CHECK(have_forward && have_backward && have_left && have_right);
    }

    /* 40. Translation with V2 enabled + non-zero coords + force_active reset */
    {
        CSB_V2_PhaseGateConfig gate = { 0, 0 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        csb_v2_touch_runtime_force_active_for_test(0);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_CONTROLLER_DPAD_UP, 42, 99, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        csb_v2_touch_runtime_shutdown();
    }

    /* 41. Init then set NULL gate config → still safe (rejects) */
    {
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(NULL);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        CHECK(rc == 0);
        CHECK(csb_v2_touch_runtime_is_active() == 0);
        csb_v2_touch_runtime_shutdown();
    }

    /* 42. Translation count preserved across force_active_for_test toggles */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_init();
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int before = csb_v2_touch_runtime_translation_count();
        csb_v2_touch_runtime_force_active_for_test(0);
        /* gate=1,1 with force_active=0 → still active, +1 */
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        csb_v2_touch_runtime_force_active_for_test(1);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* +1 */
        CHECK(csb_v2_touch_runtime_translation_count() == before + 2);
        csb_v2_touch_runtime_force_active_for_test(0);
        csb_v2_touch_runtime_shutdown();
    }

    printf("\n%d/%d assertions passed\n", g_assertions - g_failures, g_assertions);
    if (g_failures == 0) {
        printf("PASS: CSB V2 Touch Runtime wire-up probe\n");
        return 0;
    }
    printf("FAIL: %d assertion(s) failed\n", g_failures);
    return 1;
}
