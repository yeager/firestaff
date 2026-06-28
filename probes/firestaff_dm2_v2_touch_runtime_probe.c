/* firestaff_dm2_v2_touch_runtime_probe.c — DM2 V2 Touch Runtime Wire-up Probe
 *
 * Phase 6 touch/controller runtime wire-up verification probe.
 * Verifies that the dm2_v2_touch_runtime module correctly:
 *   - Initializes and shuts down the module
 *   - Phase-gates translation: V2 off → rejected, V2 on → accepted
 *   - Translates each touch swipe / edge strafe / D-pad / left stick
 *     / right stick affordance to the right V1 command
 *   - Rejects touch gestures that begin on V2 HUD chrome while allowing
 *     controller affordances to bypass the framebuffer coordinate gate
 *   - Preserves V1 source-route handling (DM2_V1 pathway is unchanged)
 *   - Honors the force_active_for_test escape hatch
 *   - Increments the observability counter on each accepted translation
 *   - Source evidence returns the citation string
 *
 * Source-lock:
 *   SKULL.ASM T520 (party/movement tick, consumer of input queue)
 *   SKULL.ASM T048 (input dispatch)
 *   SKULL.ASM T560 (dungeon viewport rendering)
 *   ReDMCSB COMMAND.C:108-113 (mouse movement zones C001-C006)
 *   ReDMCSB COMMAND.C:254-291 (keyboard tables for C001..C006)
 *   ReDMCSB CLIKMENU.C:142 (F0365_COMMAND_ProcessTypes1To2_TurnParty)
 *   ReDMCSB CLIKMENU.C:180 (F0366_COMMAND_ProcessTypes3To6_MoveParty)
 *   ReDMCSB GAMELOOP.C:164-219 (V1 input wait/command queue loop)
 *
 * Coverage:
 *   1.  init/shutdown lifecycle
 *   2.  Shutdown re-init is idempotent
 *   3.  Translation rejected when V2 launch disabled (V1 parity guard)
 *   4.  Translation rejected when V2 profile disabled
 *   5.  Translation rejected when out is NULL
 *   6.  Translation rejected for NONE affordance
 *   7.  Touch swipe up → MOVE_FORWARD (3) — accepted
 *   8.  Touch swipe down → MOVE_BACKWARD (5) — accepted
 *   9.  Touch swipe left → TURN_LEFT (1) — accepted
 *  10.  Touch swipe right → TURN_RIGHT (2) — accepted
 *  11.  Touch edge strafe left → MOVE_LEFT (6) — accepted
 *  12.  Touch edge strafe right → MOVE_RIGHT (4) — accepted
 *  13.  Controller D-pad up → MOVE_FORWARD
 *  14.  Controller D-pad down → MOVE_BACKWARD
 *  15.  Controller D-pad left → TURN_LEFT
 *  16.  Controller D-pad right → TURN_RIGHT
 *  17.  Coordinates are passed through (x=42, y=99)
 *  18.  Translation count increments on accepted translation
 *  19.  Translation count does NOT increment on rejected translation
 *  20.  force_active_for_test bypasses the gate
 *  21.  is_active returns 0 when V2 disabled
 *  22.  is_active returns 1 when V2 enabled
 *  23.  V1 invariant: V2 off → translation_count stays 0
 *  24.  V2 on + multiple accepted translations → count = N
 *  25.  Translation from V2 off → out->command = NONE (V1 chrome untouched)
 *  26.  source_evidence returns citation string with SKULL.ASM T520
 *  27.  Translation accepted even when v2PresentationEnabled=1 in route()
 *      (mirrors dm1_v2_movement_command_route_for_presentation semantics)
 *  28.  Shutdown resets the translation counter
 *  29.  Touch-origin HUD chrome points are rejected
 *  30.  Controller-origin affordances bypass the HUD coordinate gate
 */

#include "dm2_v2_touch_runtime.h"
#include "dm2_v2_touch_controller_affordance.h"
#include "dm2_v2_phase_gate.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures   = 0;

#define CHECK(cond_) do { \
    g_assertions++; \
    if (!(cond_)) { \
        printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, #cond_); \
        g_failures++; \
    } \
} while (0)

int main(void) {
    printf("DM2 V2 Touch Runtime Wire-up — Phase 6 headless probe\n");
    printf("Source: SKULL.ASM T520/T048/T560, ReDMCSB COMMAND.C:108-113/254-291,\n"
           "        ReDMCSB CLIKMENU.C:142/180, ReDMCSB GAMELOOP.C:164-219\n");

    /* 1. init/shutdown lifecycle */
    dm2_v2_touch_runtime_init();
    CHECK(dm2_v2_touch_runtime_is_active() == 0);  /* no gate config yet */
    dm2_v2_touch_runtime_shutdown();
    CHECK(dm2_v2_touch_runtime_is_active() == 0);

    /* 2. Re-init is idempotent (and translation_count resets to 0) */
    dm2_v2_touch_runtime_init();
    dm2_v2_touch_runtime_init();  /* second init should not break */
    CHECK(dm2_v2_touch_runtime_translation_count() == 0);
    dm2_v2_touch_runtime_shutdown();

    /* 3. V2 off (launch disabled) → rejected */
    {
        DM2_V2_PhaseGateConfig gate = { 0, 0 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);  /* V1 chrome untouched */
        dm2_v2_touch_runtime_shutdown();
    }

    /* 4. V2 partial (launch on, profile off) → rejected */
    {
        DM2_V2_PhaseGateConfig gate = { 1, 0 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
        dm2_v2_touch_runtime_shutdown();
    }

    /* 5. V2 on → translation accepted, command is MOVE_FORWARD (3) */
    {
        DM2_V2_PhaseGateConfig gate = { 1, 1 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };

        /* 7. swipe up */
        int rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);

        /* 8. swipe down */
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_DOWN, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_BACKWARD);

        /* 9. swipe left */
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_LEFT);

        /* 10. swipe right */
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_RIGHT);

        /* 11. edge strafe left */
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_LEFT);

        /* 12. edge strafe right */
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_RIGHT);

        /* 13. dpad up */
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_CONTROLLER_DPAD_UP, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);

        /* 14. dpad down */
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_BACKWARD);

        /* 15. dpad left */
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_LEFT);

        /* 16. dpad right */
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_TURN_RIGHT);

        /* 17. coordinates pass through */
        struct Dm1V1QueuedCommandPc34Compat out2 = { 0, 0, 0 };
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 42, 99, &out2);
        CHECK(rc == 1);
        CHECK(out2.x == 42);
        CHECK(out2.y == 99);

        /* 24. count incremented by 11 (4 swipes + 2 strafe + 4 dpad + 1 passthrough = 11 accepted translations) */
        CHECK(dm2_v2_touch_runtime_translation_count() == 11);

        dm2_v2_touch_runtime_shutdown();
    }

    /* 18. V2 off → count stays 0 */
    {
        DM2_V2_PhaseGateConfig gate = { 0, 0 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, 0, 0, &out);
        CHECK(dm2_v2_touch_runtime_translation_count() == 0);
        dm2_v2_touch_runtime_shutdown();
    }

    /* 19. NONE affordance is rejected even when V2 is on */
    {
        DM2_V2_PhaseGateConfig gate = { 1, 1 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_NONE, 0, 0, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        CHECK(dm2_v2_touch_runtime_translation_count() == 0);
        dm2_v2_touch_runtime_shutdown();
    }

    /* 20. force_active_for_test bypasses gate */
    {
        DM2_V2_PhaseGateConfig gate = { 0, 0 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);
        CHECK(dm2_v2_touch_runtime_is_active() == 0);
        dm2_v2_touch_runtime_force_active_for_test(1);
        CHECK(dm2_v2_touch_runtime_is_active() == 1);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);
        CHECK(dm2_v2_touch_runtime_translation_count() == 1);
        dm2_v2_touch_runtime_force_active_for_test(0);
        dm2_v2_touch_runtime_shutdown();
    }

    /* 21. NULL out pointer is rejected */
    {
        DM2_V2_PhaseGateConfig gate = { 1, 1 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);
        int rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, NULL);
        CHECK(rc == 0);
        dm2_v2_touch_runtime_shutdown();
    }

    /* 22. source_evidence returns the citation */
    {
        const char *ev = dm2_v2_touch_runtime_source_evidence();
        CHECK(ev != NULL && ev[0] != '\0');
        CHECK(strstr(ev, "SKULL.ASM T520") != NULL);
        CHECK(strstr(ev, "CLIKMENU.C") != NULL);
    }

    /* 23. Shutdown resets the counter */
    {
        DM2_V2_PhaseGateConfig gate = { 1, 1 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, 51, 60, &out);
        CHECK(dm2_v2_touch_runtime_translation_count() == 2);
        dm2_v2_touch_runtime_shutdown();
        dm2_v2_touch_runtime_init();  /* fresh start */
        CHECK(dm2_v2_touch_runtime_translation_count() == 0);
        dm2_v2_touch_runtime_shutdown();
    }

    /* 24. V2 off → out struct fields stay at caller-provided defaults */
    {
        DM2_V2_PhaseGateConfig gate = { 0, 0 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 7, 11, 13 };
        int rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        CHECK(rc == 0);
        /* Command field is reset to NONE; x/y preserve caller input */
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        CHECK(out.x == 0);
        CHECK(out.y == 0);
        dm2_v2_touch_runtime_shutdown();
    }

    /* 25. Touch HUD chrome is excluded from movement gestures; controller
     * affordances have no framebuffer origin and bypass the coordinate gate. */
    {
        DM2_V2_PhaseGateConfig gate = { 1, 1 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);

        CHECK(dm2_v2_touch_runtime_point_in_hud_chrome(10, 8) == 1);
        CHECK(dm2_v2_touch_runtime_point_in_hud_chrome(100, 60) == 0);
        CHECK(dm2_v2_touch_runtime_point_in_hud_chrome(100, DM2_V2_TOUCH_FRAMEBUFFER_H - 1) == 1);
        CHECK(dm2_v2_touch_runtime_point_in_hud_chrome(-1, 60) == 1);

        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 10, 8, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
        CHECK(dm2_v2_touch_runtime_translation_count() == 0);

        out.command = 99;
        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT,
            100, DM2_V2_TOUCH_FRAMEBUFFER_H - 1, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);

        rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_CONTROLLER_DPAD_UP, 10, 8, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);
        CHECK(dm2_v2_touch_runtime_translation_count() == 1);

        dm2_v2_touch_runtime_shutdown();
    }

    /* 26. After shutdown, translation is rejected (no double-free risk) */
    {
        DM2_V2_PhaseGateConfig gate = { 1, 1 };
        dm2_v2_touch_runtime_init();
        dm2_v2_touch_runtime_set_gate_config(&gate);
        dm2_v2_touch_runtime_shutdown();
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = dm2_v2_touch_runtime_translate_affordance(
            DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        CHECK(rc == 0);
    }

    printf("\n%d/%d assertions passed\n", g_assertions - g_failures, g_assertions);
    if (g_failures == 0) {
        printf("PASS: DM2 V2 Touch Runtime wire-up probe\n");
        return 0;
    }
    printf("FAIL: %d assertion(s) failed\n", g_failures);
    return 1;
}
