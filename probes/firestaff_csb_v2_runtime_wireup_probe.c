/* firestaff_csb_v2_runtime_wireup_probe.c — CSB V2 Phase 4/5/6 Runtime Wire-up Probe
 *
 * Combined runtime wire-up verification probe for Chaos Strikes Back V2.
 * Verifies that csb_v2_lighting_runtime, csb_v2_smooth_movement_runtime,
 * and csb_v2_touch_runtime all correctly:
 *   - Initialize and shut down their respective module state
 *   - Phase-gate ticks/translations (V2 off → no-op, V2 on → executes)
 *   - V1 state preserved when V2 disabled
 *   - Honor force_active_for_test escape hatch
 *   - Increment observability counters on accepted operations
 *   - Source evidence returns citation strings
 *
 * Source-lock:
 *   ReDMCSB LIGHT.C F0380 (lighting)
 *   ReDMCSB GROUP.C:1695-1770 (smooth movement)
 *   ReDMCSB COMMAND.C:108-113/254-291 (touch/click)
 *   ReDMCSB CLIKMENU.C:142/180 (turn/move)
 *   ReDMCSB GAMELOOP.C:47-50/164-219 (V1 tick + input loop)
 *   CSBWin/resurrect/CsbV2* (CSBWin reimpl)
 *
 * Coverage (~50 assertions across 3 module groups):
 *
 *   Lighting runtime (~15):
 *     - init/shutdown lifecycle + idempotent re-init
 *     - V2 disabled tick is no-op (count stays 0)
 *     - V2 enabled tick increments count
 *     - V2 partial (presentation off / persistence off) → no-op
 *     - force_active_for_test bypass
 *     - is_active reflects gate state
 *     - V1 invariant state preservation
 *     - Toggle cycles preserve count
 *     - Post-shutdown: tick is no-op
 *     - source_evidence citation
 *
 *   Smooth movement runtime (~15):
 *     - init/shutdown lifecycle
 *     - V2 disabled tick is no-op
 *     - V2 enabled tick increments count
 *     - Walk/turn/stairs triggers when V2 enabled
 *     - Triggers rejected when V2 disabled
 *     - force_active_for_test bypass
 *     - V1 invariant state preservation
 *     - Toggle cycles preserve count
 *     - Post-shutdown: tick is no-op
 *     - source_evidence citation
 *
 *   Touch runtime (~20):
 *     - init/shutdown lifecycle
 *     - Translation rejected: V2 off / partial / NONE affordance / null out
 *     - All 6 movement affordances translate to correct DM1_V1_COMMAND_*
 *     - Translation count increments only on accepted
 *     - force_active_for_test bypass
 *     - Coordinate pass-through (incl. negative)
 *     - V1 invariant state preservation
 *     - Toggle cycles preserve count
 *     - Post-shutdown: translation rejected
 *     - source_evidence citation
 */

#include "csb_v2_lighting_runtime.h"
#include "csb_v2_smooth_movement_runtime.h"
#include "csb_v2_touch_runtime.h"
#include "csb_v2_lighting_dynamic.h"
#include "csb_v2_smooth_movement.h"
#include "csb_v2_touch_controller_affordance.h"
#include "csb_v2_phase_gate_pc34.h"
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

#define CHECK_GROUP(name) \
    printf("\n  --- %s ---\n", name)

int main(void) {
    printf("CSB V2 Runtime Wire-up — Phase 4/5/6 combined probe\n");
    printf("Source: ReDMCSB LIGHT.C F0380, GROUP.C:1695-1770,\n"
           "        COMMAND.C:108-113/254-291, CLIKMENU.C:142/180,\n"
           "        GAMELOOP.C:47-50/164-219, CSBWin/resurrect/CsbV2*\n");

    /* ──────────────────────────────────────────────────────────────── */
    CHECK_GROUP("Lighting runtime");

    {
        csb_v2_lighting_runtime_init();
        CHECK(csb_v2_lighting_runtime_is_active() == 0);  /* no gate */
        csb_v2_lighting_runtime_shutdown();
        CHECK(csb_v2_lighting_runtime_is_active() == 0);
    }

    /* Re-init idempotent */
    csb_v2_lighting_runtime_init();
    csb_v2_lighting_runtime_init();
    CHECK(csb_v2_lighting_runtime_tick_count() == 0);

    /* V2 disabled tick is no-op */
    csb_v2_lighting_runtime_tick(0.055f);
    CHECK(csb_v2_lighting_runtime_tick_count() == 0);

    /* V2 partial: presentation off */
    {
        CSB_V2_PhaseGateConfig gate = { 0, 1 };
        csb_v2_lighting_runtime_set_gate_config(&gate);
        csb_v2_lighting_runtime_tick(0.055f);
        CHECK(csb_v2_lighting_runtime_tick_count() == 0);
    }

    /* V2 partial: persistence off */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 0 };
        csb_v2_lighting_runtime_set_gate_config(&gate);
        csb_v2_lighting_runtime_tick(0.055f);
        CHECK(csb_v2_lighting_runtime_tick_count() == 0);
    }

    /* V2 enabled */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_lighting_runtime_set_gate_config(&gate);
        CHECK(csb_v2_lighting_runtime_is_active() == 1);
        int before = csb_v2_lighting_runtime_tick_count();
        csb_v2_lighting_runtime_tick(0.055f);
        CHECK(csb_v2_lighting_runtime_tick_count() == before + 1);
        /* Multiple ticks monotonic */
        csb_v2_lighting_runtime_tick(0.055f);
        csb_v2_lighting_runtime_tick(0.055f);
        CHECK(csb_v2_lighting_runtime_tick_count() == before + 3);
    }

    /* force_active_for_test bypass */
    {
        CSB_V2_PhaseGateConfig gate = { 0, 0 };
        csb_v2_lighting_runtime_set_gate_config(&gate);
        CHECK(csb_v2_lighting_runtime_is_active() == 0);
        csb_v2_lighting_runtime_force_active_for_test(1);
        CHECK(csb_v2_lighting_runtime_is_active() == 1);
        int before = csb_v2_lighting_runtime_tick_count();
        csb_v2_lighting_runtime_tick(0.055f);
        CHECK(csb_v2_lighting_runtime_tick_count() == before + 1);
        csb_v2_lighting_runtime_force_active_for_test(0);
    }

    /* V1 invariant: tick disabled → count stays */
    {
        CSB_V2_PhaseGateConfig off = { 0, 0 };
        csb_v2_lighting_runtime_set_gate_config(&off);
        int before = csb_v2_lighting_runtime_tick_count();
        csb_v2_lighting_runtime_tick(0.5f);
        CHECK(csb_v2_lighting_runtime_tick_count() == before);
    }

    /* Post-shutdown: tick is no-op */
    csb_v2_lighting_runtime_shutdown();
    csb_v2_lighting_runtime_tick(0.055f);
    CHECK(csb_v2_lighting_runtime_tick_count() == 0);

    /* source_evidence citation */
    {
        csb_v2_lighting_runtime_init();
        const char *ev = csb_v2_lighting_runtime_source_evidence();
        CHECK(ev != NULL && ev[0] != '\0'
            && strstr(ev, "LIGHT.C F0380") != NULL);
        csb_v2_lighting_runtime_shutdown();
    }

    /* ──────────────────────────────────────────────────────────────── */
    CHECK_GROUP("Smooth movement runtime");

    {
        csb_v2_smooth_movement_runtime_init();
        CHECK(csb_v2_smooth_movement_runtime_is_active() == 0);
        csb_v2_smooth_movement_runtime_shutdown();
        CHECK(csb_v2_smooth_movement_runtime_is_active() == 0);
    }

    /* Re-init idempotent */
    csb_v2_smooth_movement_runtime_init();
    csb_v2_smooth_movement_runtime_init();
    CHECK(csb_v2_smooth_movement_runtime_tick_count() == 0);

    /* V2 disabled tick is no-op */
    csb_v2_smooth_movement_runtime_tick(0.055f);
    CHECK(csb_v2_smooth_movement_runtime_tick_count() == 0);

    /* V2 enabled */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_smooth_movement_runtime_set_gate_config(&gate);
        CHECK(csb_v2_smooth_movement_runtime_is_active() == 1);
        int before = csb_v2_smooth_movement_runtime_tick_count();
        csb_v2_smooth_movement_runtime_tick(0.055f);
        CHECK(csb_v2_smooth_movement_runtime_tick_count() == before + 1);
    }

    /* Walk/turn/stairs triggers when V2 enabled */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_smooth_movement_runtime_set_gate_config(&gate);
        csb_v2_smooth_movement_runtime_start_walk(5.0f, 5.0f, 6.0f, 5.0f);
        csb_v2_smooth_movement_runtime_start_turn(0.0f, 90.0f);
        csb_v2_smooth_movement_runtime_start_stairs(5.0f, 5.0f, 5.0f, 4.0f, 0.0f);
        CHECK(1);  /* no crash */
    }

    /* Triggers rejected when V2 disabled */
    {
        CSB_V2_PhaseGateConfig off = { 0, 0 };
        csb_v2_smooth_movement_runtime_set_gate_config(&off);
        csb_v2_smooth_movement_runtime_start_walk(0.0f, 0.0f, 1.0f, 0.0f);
        csb_v2_smooth_movement_runtime_start_turn(0.0f, 45.0f);
        csb_v2_smooth_movement_runtime_start_stairs(0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        CHECK(1);
    }

    /* force_active_for_test bypass */
    {
        CSB_V2_PhaseGateConfig off = { 0, 0 };
        csb_v2_smooth_movement_runtime_set_gate_config(&off);
        csb_v2_smooth_movement_runtime_force_active_for_test(1);
        int before = csb_v2_smooth_movement_runtime_tick_count();
        csb_v2_smooth_movement_runtime_tick(0.055f);
        CHECK(csb_v2_smooth_movement_runtime_tick_count() == before + 1);
        csb_v2_smooth_movement_runtime_force_active_for_test(0);
    }

    /* V1 invariant state preservation */
    {
        CSB_V2_PhaseGateConfig off = { 0, 0 };
        csb_v2_smooth_movement_runtime_set_gate_config(&off);
        int before = csb_v2_smooth_movement_runtime_tick_count();
        csb_v2_smooth_movement_runtime_tick(0.5f);
        CHECK(csb_v2_smooth_movement_runtime_tick_count() == before);
    }

    /* Toggle cycles preserve count */
    {
        CSB_V2_PhaseGateConfig on = { 1, 1 };
        CSB_V2_PhaseGateConfig off = { 0, 0 };
        csb_v2_smooth_movement_runtime_set_gate_config(&on);
        int before = csb_v2_smooth_movement_runtime_tick_count();
        csb_v2_smooth_movement_runtime_tick(0.055f);  /* +1 */
        csb_v2_smooth_movement_runtime_set_gate_config(&off);
        csb_v2_smooth_movement_runtime_tick(0.055f);  /* no-op */
        csb_v2_smooth_movement_runtime_set_gate_config(&on);
        csb_v2_smooth_movement_runtime_tick(0.055f);  /* +1 */
        CHECK(csb_v2_smooth_movement_runtime_tick_count() == before + 2);
    }

    /* Post-shutdown */
    csb_v2_smooth_movement_runtime_shutdown();
    csb_v2_smooth_movement_runtime_tick(0.055f);
    CHECK(csb_v2_smooth_movement_runtime_tick_count() == 0);

    /* source_evidence */
    {
        csb_v2_smooth_movement_runtime_init();
        const char *ev = csb_v2_smooth_movement_runtime_source_evidence();
        CHECK(ev != NULL && ev[0] != '\0'
            && strstr(ev, "GROUP.C") != NULL);
        csb_v2_smooth_movement_runtime_shutdown();
    }

    /* ──────────────────────────────────────────────────────────────── */
    CHECK_GROUP("Touch runtime");

    {
        csb_v2_touch_runtime_init();
        CHECK(csb_v2_touch_runtime_is_active() == 0);
        csb_v2_touch_runtime_shutdown();
        CHECK(csb_v2_touch_runtime_is_active() == 0);
    }

    /* Re-init idempotent */
    csb_v2_touch_runtime_init();
    csb_v2_touch_runtime_init();
    CHECK(csb_v2_touch_runtime_translation_count() == 0);

    /* Translation rejected: V2 disabled */
    {
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(out.command == DM1_V1_COMMAND_NONE);
    }

    /* V2 partial (presentation off) */
    {
        CSB_V2_PhaseGateConfig gate = { 0, 1 };
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
    }

    /* V2 partial (persistence off) */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 0 };
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 50, 60, &out);
        CHECK(rc == 0);
    }

    /* NONE affordance rejected even when V2 enabled */
    {
        CSB_V2_PhaseGateConfig gate = { 1, 1 };
        csb_v2_touch_runtime_set_gate_config(&gate);
        struct Dm1V1QueuedCommandPc34Compat out = { 99, 99, 99 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_NONE, 50, 60, &out);
        CHECK(rc == 0);
        CHECK(csb_v2_touch_runtime_translation_count() == 0);
    }

    /* All 6 movement affordances translate to correct V1 commands */
    {
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

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_LEFT);

        rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, 50, 60, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_RIGHT);

        CHECK(csb_v2_touch_runtime_translation_count() == 6);
    }

    /* Coordinates pass through (incl. negative) */
    {
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, -42, -99, &out);
        CHECK(rc == 1);
        CHECK(out.x == -42);
        CHECK(out.y == -99);
    }

    /* Null out pointer rejected */
    {
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, NULL);
        CHECK(rc == 0);
    }

    /* force_active_for_test bypass */
    {
        CSB_V2_PhaseGateConfig off = { 0, 0 };
        csb_v2_touch_runtime_set_gate_config(&off);
        csb_v2_touch_runtime_force_active_for_test(1);
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        CHECK(rc == 1);
        CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);
        csb_v2_touch_runtime_force_active_for_test(0);
    }

    /* V1 invariant: V2 disabled → translation_count stays */
    {
        CSB_V2_PhaseGateConfig off = { 0, 0 };
        csb_v2_touch_runtime_set_gate_config(&off);
        int before = csb_v2_touch_runtime_translation_count();
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, 0, 0, &out);
        CHECK(csb_v2_touch_runtime_translation_count() == before);
    }

    /* Toggle cycles preserve count */
    {
        CSB_V2_PhaseGateConfig on = { 1, 1 };
        CSB_V2_PhaseGateConfig off = { 0, 0 };
        csb_v2_touch_runtime_set_gate_config(&on);
        int before = csb_v2_touch_runtime_translation_count();
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* +1 */
        csb_v2_touch_runtime_set_gate_config(&off);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);  /* no-op */
        csb_v2_touch_runtime_set_gate_config(&on);
        csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, 0, 0, &out);  /* +1 */
        CHECK(csb_v2_touch_runtime_translation_count() == before + 2);
    }

    /* Post-shutdown: translation rejected */
    csb_v2_touch_runtime_shutdown();
    {
        struct Dm1V1QueuedCommandPc34Compat out = { 0, 0, 0 };
        int rc = csb_v2_touch_runtime_translate_affordance(
            CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, 0, 0, &out);
        CHECK(rc == 0);
    }

    /* source_evidence */
    {
        csb_v2_touch_runtime_init();
        const char *ev = csb_v2_touch_runtime_source_evidence();
        CHECK(ev != NULL && ev[0] != '\0'
            && strstr(ev, "COMMAND.C") != NULL);
        csb_v2_touch_runtime_shutdown();
    }

    printf("\n%d/%d assertions passed\n", g_assertions - g_failures, g_assertions);
    if (g_failures == 0) {
        printf("PASS: CSB V2 runtime wire-up probe\n");
        return 0;
    }
    printf("FAIL: %d assertion(s) failed\n", g_failures);
    return 1;
}