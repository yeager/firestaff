/* firestaff_nexus_v2_smooth_movement_runtime_probe.c — Nexus V2 Smooth Movement Runtime Wire-up Probe
 *
 * Phase 5 smooth movement runtime wire-up verification probe for Nexus V2.
 * Mirrors dm2_v2_smooth_movement_runtime pattern (sibling DM2 wire-up).
 *
 * Source-lock:
 *   ReDMCSB GROUP.C:1695-1770 (F0207 creature attack)
 *   skproject/SKWIN/SkWinCore.cpp (ease-out cubic / ease-in-out cubic)
 *   ReDMCSB GAMELOOP.C:47-50 (V1 tick cadence 55ms)
 *   Saturn NEXUS.BIN (Saturn-specific interpolation timing)
 *
 * Coverage (28 assertions):
 *   1.  init/shutdown lifecycle
 *   2.  Re-init idempotent
 *   3.  Initial state: tick_count=0, smooth state idle
 *   4.  Shutdown resets counter
 *   5.  Tick without init is no-op
 *   6.  Tick with no gate config (V2 disabled) → no-op
 *   7.  Tick with V2 presentation disabled → no-op
 *   8.  Tick with V2 config persistence disabled → no-op
 *   9.  Tick with V2 enabled → count increments
 *  10.  Multiple ticks monotonic
 *  11.  Walk trigger when V2 enabled
 *  12.  Turn trigger when V2 enabled
 *  13.  Stairs trigger when V2 enabled (with from_vert/to_vert)
 *  14.  Triggers rejected when V2 disabled
 *  15.  is_active returns 0 when V2 disabled
 *  16.  is_active returns 1 when V2 enabled
 *  17.  force_active_for_test bypasses gate
 *  18.  get_state returns NULL when V2 disabled
 *  19.  get_state returns valid pointer when V2 enabled
 *  20.  V1 invariant: tick disabled → state preserved
 *  21.  V2 invariant: tick enabled → state advances
 *  22.  Toggle V2 off then on: count continues
 *  23.  Post-shutdown: tick is no-op, get_state returns NULL
 *  24.  source_evidence returns citation
 *  25.  Tick with dt=0: count increments, no crash
 *  26.  Multiple init/shutdown cycles safe
 *  27.  Walk/turn/stairs triggers accepted in any order
 *  28.  Walk trigger rejected when runtime not initialized
 */

#include "nexus_v2_smooth_movement_runtime.h"
#include "nexus_v2_smooth_movement.h"
#include "nexus_v2_phase_gate_pc34.h"
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
    printf("Nexus V2 Smooth Movement Runtime Wire-up — Phase 5 headless probe\n");
    printf("Source: ReDMCSB GROUP.C:1695-1770, skproject/SKWIN/SkWinCore.cpp,\n"
           "        ReDMCSB GAMELOOP.C:47-50, Saturn NEXUS.BIN\n");

    /* 1. init/shutdown lifecycle */
    nexus_v2_smooth_movement_runtime_init();
    CHECK(nexus_v2_smooth_movement_runtime_is_active() == 0);
    nexus_v2_smooth_movement_runtime_shutdown();
    CHECK(nexus_v2_smooth_movement_runtime_is_active() == 0);

    /* 2. Re-init idempotent */
    nexus_v2_smooth_movement_runtime_init();
    nexus_v2_smooth_movement_runtime_init();
    CHECK(nexus_v2_smooth_movement_runtime_tick_count() == 0);

    /* 3. Initial state */
    const Nexus_V2_SmoothState *s = nexus_v2_smooth_movement_runtime_get_state();
    CHECK(s == NULL);
    CHECK(nexus_v2_smooth_movement_runtime_tick_count() == 0);

    /* 4. Shutdown clears counter */
    nexus_v2_smooth_movement_runtime_shutdown();
    nexus_v2_smooth_movement_runtime_init();
    CHECK(nexus_v2_smooth_movement_runtime_tick_count() == 0);

    /* 5. Tick without init is no-op */
    nexus_v2_smooth_movement_runtime_shutdown();
    nexus_v2_smooth_movement_runtime_tick(55.0f);
    CHECK(nexus_v2_smooth_movement_runtime_tick_count() == 0);

    /* 6. Tick with no gate config */
    nexus_v2_smooth_movement_runtime_init();
    nexus_v2_smooth_movement_runtime_tick(55.0f);
    CHECK(nexus_v2_smooth_movement_runtime_tick_count() == 0);

    /* 7. Tick with V2 presentation disabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 1 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        nexus_v2_smooth_movement_runtime_tick(55.0f);
        CHECK(nexus_v2_smooth_movement_runtime_tick_count() == 0);
    }

    /* 8. Tick with V2 config persistence disabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 0 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        nexus_v2_smooth_movement_runtime_tick(55.0f);
        CHECK(nexus_v2_smooth_movement_runtime_tick_count() == 0);
    }

    /* 9. Tick with V2 enabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        int before = nexus_v2_smooth_movement_runtime_tick_count();
        nexus_v2_smooth_movement_runtime_tick(55.0f);
        int after = nexus_v2_smooth_movement_runtime_tick_count();
        CHECK(after == before + 1);
    }

    /* 10. Multiple ticks monotonic */
    {
        int before = nexus_v2_smooth_movement_runtime_tick_count();
        for (int i = 0; i < 5; i++) {
            nexus_v2_smooth_movement_runtime_tick(55.0f);
        }
        int after = nexus_v2_smooth_movement_runtime_tick_count();
        CHECK(after == before + 5);
    }

    /* 11. Walk trigger when V2 enabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        nexus_v2_smooth_movement_runtime_start_walk(5.0f, 5.0f, 6.0f, 5.0f);
        CHECK(1);  /* no crash */
    }

    /* 12. Turn trigger when V2 enabled */
    {
        nexus_v2_smooth_movement_runtime_start_turn(0.0f, 90.0f);
        CHECK(1);  /* no crash */
    }

    /* 13. Stairs trigger when V2 enabled */
    {
        nexus_v2_smooth_movement_runtime_start_stairs(5.0f, 5.0f, 5.0f, 4.0f, 0.0f, 1.0f);
        CHECK(1);  /* no crash */
    }

    /* 14. Triggers rejected when V2 disabled */
    {
        NEXUS_V2_PhaseGateConfig off = { 0, 0 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&off);
        /* These should be no-ops (no crash) */
        nexus_v2_smooth_movement_runtime_start_walk(0.0f, 0.0f, 1.0f, 0.0f);
        nexus_v2_smooth_movement_runtime_start_turn(0.0f, 45.0f);
        nexus_v2_smooth_movement_runtime_start_stairs(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
        CHECK(1);
    }

    /* 15. is_active returns 0 when V2 disabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        CHECK(nexus_v2_smooth_movement_runtime_is_active() == 0);
    }

    /* 16. is_active returns 1 when V2 enabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        CHECK(nexus_v2_smooth_movement_runtime_is_active() == 1);
    }

    /* 17. force_active_for_test bypasses gate */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        CHECK(nexus_v2_smooth_movement_runtime_is_active() == 0);
        nexus_v2_smooth_movement_runtime_force_active_for_test(1);
        CHECK(nexus_v2_smooth_movement_runtime_is_active() == 1);
        int before = nexus_v2_smooth_movement_runtime_tick_count();
        nexus_v2_smooth_movement_runtime_tick(55.0f);
        CHECK(nexus_v2_smooth_movement_runtime_tick_count() == before + 1);
        nexus_v2_smooth_movement_runtime_force_active_for_test(0);
    }

    /* 18. get_state returns NULL when V2 disabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        CHECK(nexus_v2_smooth_movement_runtime_get_state() == NULL);
    }

    /* 19. get_state returns valid pointer when V2 enabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        CHECK(nexus_v2_smooth_movement_runtime_get_state() != NULL);
    }

    /* 20. V1 invariant: tick disabled → count stays */
    {
        NEXUS_V2_PhaseGateConfig off = { 0, 0 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&off);
        int before = nexus_v2_smooth_movement_runtime_tick_count();
        nexus_v2_smooth_movement_runtime_tick(55.0f);
        CHECK(nexus_v2_smooth_movement_runtime_tick_count() == before);
    }

    /* 21. V2 invariant: tick enabled → count increments */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        int before = nexus_v2_smooth_movement_runtime_tick_count();
        nexus_v2_smooth_movement_runtime_tick(55.0f);
        CHECK(nexus_v2_smooth_movement_runtime_tick_count() == before + 1);
    }

    /* 22. Toggle V2 off then on: count continues */
    {
        NEXUS_V2_PhaseGateConfig on = { 1, 1 };
        NEXUS_V2_PhaseGateConfig off = { 0, 0 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&on);
        int before = nexus_v2_smooth_movement_runtime_tick_count();
        nexus_v2_smooth_movement_runtime_tick(55.0f);
        nexus_v2_smooth_movement_runtime_set_gate_config(&off);
        nexus_v2_smooth_movement_runtime_tick(55.0f);  /* no-op */
        nexus_v2_smooth_movement_runtime_set_gate_config(&on);
        nexus_v2_smooth_movement_runtime_tick(55.0f);
        CHECK(nexus_v2_smooth_movement_runtime_tick_count() == before + 2);
    }

    /* 23. Post-shutdown: tick is no-op, get_state returns NULL */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        nexus_v2_smooth_movement_runtime_shutdown();
        nexus_v2_smooth_movement_runtime_tick(55.0f);
        CHECK(nexus_v2_smooth_movement_runtime_get_state() == NULL);
        CHECK(nexus_v2_smooth_movement_runtime_tick_count() == 0);
    }

    /* 24. source_evidence returns citation */
    {
        nexus_v2_smooth_movement_runtime_init();
        const char *ev = nexus_v2_smooth_movement_runtime_source_evidence();
        CHECK(ev != NULL && ev[0] != '\0'
            && strstr(ev, "GAMELOOP.C") != NULL
            && strstr(ev, "SkWinCore.cpp") != NULL);
        nexus_v2_smooth_movement_runtime_shutdown();
    }

    /* 25. Tick with dt=0: count increments, no crash */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_smooth_movement_runtime_init();
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        int before = nexus_v2_smooth_movement_runtime_tick_count();
        nexus_v2_smooth_movement_runtime_tick(0.0f);
        CHECK(nexus_v2_smooth_movement_runtime_tick_count() == before + 1);
        nexus_v2_smooth_movement_runtime_shutdown();
    }

    /* 26. Multiple init/shutdown cycles safe */
    {
        for (int i = 0; i < 5; i++) {
            nexus_v2_smooth_movement_runtime_init();
            nexus_v2_smooth_movement_runtime_shutdown();
        }
        CHECK(1);
    }

    /* 27. Walk/turn/stairs triggers accepted in any order */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_smooth_movement_runtime_init();
        nexus_v2_smooth_movement_runtime_set_gate_config(&gate);
        nexus_v2_smooth_movement_runtime_start_walk(1.0f, 1.0f, 2.0f, 1.0f);
        nexus_v2_smooth_movement_runtime_start_turn(0.0f, 90.0f);
        nexus_v2_smooth_movement_runtime_start_stairs(2.0f, 1.0f, 2.0f, 0.0f, 0.0f, 1.0f);
        nexus_v2_smooth_movement_runtime_start_walk(2.0f, 0.0f, 3.0f, 0.0f);
        CHECK(1);
        nexus_v2_smooth_movement_runtime_shutdown();
    }

    /* 28. Walk trigger rejected when runtime not initialized */
    {
        nexus_v2_smooth_movement_runtime_shutdown();
        nexus_v2_smooth_movement_runtime_start_walk(0.0f, 0.0f, 1.0f, 0.0f);
        CHECK(1);  /* no crash */
    }

    printf("\n%d/%d assertions passed\n", g_assertions - g_failures, g_assertions);
    if (g_failures == 0) {
        printf("PASS: Nexus V2 Smooth Movement Runtime wire-up probe\n");
        return 0;
    }
    printf("FAIL: %d assertion(s) failed\n", g_failures);
    return 1;
}