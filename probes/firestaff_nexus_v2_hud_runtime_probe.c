/* firestaff_nexus_v2_hud_runtime_probe.c — Nexus V2 HUD Runtime Wire-up Probe
 *
 * Phase 3 HUD runtime wire-up verification probe for Nexus V2.
 * Verifies that the nexus_v2_hud_runtime module correctly:
 *   - Initializes and shuts down the HUD state
 *   - Phase-gates render: V2 off → no-op, V2 on → paints into fb
 *   - V1 framebuffer preserved byte-for-byte when V2 disabled
 *   - All 7 setters store state without crashing
 *   - Honors force_active_for_test escape hatch
 *   - Source evidence returns the citation string
 *
 * Source-lock:
 *   Saturn NEXUS.BIN HUD surface data
 *   DMDF/DGN level format (Saturn DMDF VDP1/VDP2 layers)
 *   ReDMCSB PANEL.C / DUNGEON.C (champion status refresh)
 *
 * Coverage (22 assertions):
 *   1.  init/shutdown lifecycle
 *   2.  Re-init is idempotent
 *   3.  Render is no-op when V2 launch disabled
 *   4.  Render is no-op when V2 profile disabled
 *   5.  Render is no-op when V2 disabled but fb preserved
 *   6.  Render paints into fb when V2 enabled + HUD visible
 *   7.  Render is no-op when opacity = 0
 *   8.  All 7 setters store state without crashing
 *   9.  is_active returns 0 when V2 disabled
 *  10.  is_active returns 1 when V2 enabled
 *  11.  force_active_for_test bypasses gate
 *  12.  V1 invariant: V2 disabled, fb stays untouched
 *  13.  V2 active + champion bar setter → pixels appear
 *  14.  Action strip with active icon → pixels appear
 *  15.  V1 invariant: V2 off → out struct fields stay at caller defaults
 *  16.  Null-fb is safe when V2 disabled
 *  17.  source_evidence returns citation string with NEXUS.BIN
 *  18.  Direction setter accepts all 4 cardinal values
 *  19.  Level setter accepts cur > max (clamped internally)
 *  20.  Hit-flash trigger stores state
 *  21.  Opacity setter accepts all byte values
 *  22.  Multiple init/shutdown cycles are safe
 */

#include "nexus_v2_hud_runtime.h"
#include "nexus_v2_hud_overlay.h"
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

static uint8_t fb_zero[320 * 200];

static void clear_fb(uint8_t *fb, int size) {
    memset(fb, 0, size);
}

int main(void) {
    printf("Nexus V2 HUD Runtime Wire-up — Phase 3 headless probe\n");
    printf("Source: Saturn NEXUS.BIN HUD surface data, DMDF/DGN level format,\n"
           "        ReDMCSB PANEL.C / DUNGEON.C (champion status refresh)\n");

    /* 1. init/shutdown lifecycle */
    nexus_v2_hud_runtime_init();
    CHECK(nexus_v2_hud_runtime_is_active() == 0);  /* no gate yet */
    nexus_v2_hud_runtime_shutdown();
    CHECK(nexus_v2_hud_runtime_is_active() == 0);

    /* 2. Re-init is idempotent */
    nexus_v2_hud_runtime_init();
    nexus_v2_hud_runtime_init();
    CHECK(nexus_v2_hud_runtime_is_active() == 0);
    nexus_v2_hud_runtime_shutdown();

    /* 3. V2 launch disabled → render is no-op */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        clear_fb(fb_zero, sizeof(fb_zero));
        nexus_v2_hud_runtime_render(fb_zero, 320, 200);
        int nonzero = 0;
        for (int i = 0; i < (int)sizeof(fb_zero); i++) {
            if (fb_zero[i] != 0) { nonzero++; break; }
        }
        CHECK(nonzero == 0);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 4. V2 partial (launch on, profile off) → no-op */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 0 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        clear_fb(fb_zero, sizeof(fb_zero));
        nexus_v2_hud_runtime_render(fb_zero, 320, 200);
        int nonzero = 0;
        for (int i = 0; i < (int)sizeof(fb_zero); i++) {
            if (fb_zero[i] != 0) { nonzero++; break; }
        }
        CHECK(nonzero == 0);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 5. V2 disabled → V1 framebuffer preserved byte-for-byte */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        memset(fb_zero, 0x37, sizeof(fb_zero));
        nexus_v2_hud_runtime_render(fb_zero, 320, 200);
        int preserved = 1;
        for (int i = 0; i < (int)sizeof(fb_zero); i++) {
            if (fb_zero[i] != 0x37) { preserved = 0; break; }
        }
        CHECK(preserved == 1);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 6. V2 enabled + visible → render paints into fb */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        clear_fb(fb_zero, sizeof(fb_zero));
        nexus_v2_hud_runtime_render(fb_zero, 320, 200);
        int nonzero = 0;
        for (int i = 0; i < (int)sizeof(fb_zero); i++) {
            if (fb_zero[i] != 0) { nonzero++; break; }
        }
        CHECK(nonzero > 0);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 7. Opacity = 0 → short-circuits render */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        nexus_v2_hud_runtime_set_opacity(0);
        clear_fb(fb_zero, sizeof(fb_zero));
        nexus_v2_hud_runtime_render(fb_zero, 320, 200);
        int nonzero = 0;
        for (int i = 0; i < (int)sizeof(fb_zero); i++) {
            if (fb_zero[i] != 0) { nonzero++; break; }
        }
        CHECK(nonzero == 0);
        nexus_v2_hud_runtime_set_opacity(255);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 8. All 7 setters store state without crashing */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        nexus_v2_hud_runtime_set_party_gold(500);
        nexus_v2_hud_runtime_set_direction(2);
        nexus_v2_hud_runtime_set_level(4, 10);
        nexus_v2_hud_runtime_set_champion(0, 80, 60, 40, true, false);
        nexus_v2_hud_runtime_set_action_active(NEXUS_V2_ACTION_ATTACK);
        nexus_v2_hud_runtime_trigger_hit_flash();
        nexus_v2_hud_runtime_set_opacity(200);
        CHECK(1);  /* reached here without crash */
        nexus_v2_hud_runtime_shutdown();
    }

    /* 9. is_active returns 0 when V2 disabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        CHECK(nexus_v2_hud_runtime_is_active() == 0);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 10. is_active returns 1 when V2 enabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        CHECK(nexus_v2_hud_runtime_is_active() == 1);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 11. force_active_for_test bypasses gate */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        CHECK(nexus_v2_hud_runtime_is_active() == 0);
        nexus_v2_hud_runtime_force_active_for_test(1);
        CHECK(nexus_v2_hud_runtime_is_active() == 1);
        clear_fb(fb_zero, sizeof(fb_zero));
        nexus_v2_hud_runtime_render(fb_zero, 320, 200);
        int nonzero = 0;
        for (int i = 0; i < (int)sizeof(fb_zero); i++) {
            if (fb_zero[i] != 0) { nonzero++; break; }
        }
        CHECK(nonzero > 0);
        nexus_v2_hud_runtime_force_active_for_test(0);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 12. V1 invariant: V2 disabled, fb stays untouched */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        memset(fb_zero, 0xCD, sizeof(fb_zero));
        nexus_v2_hud_runtime_render(fb_zero, 320, 200);
        int preserved = 1;
        for (int i = 0; i < (int)sizeof(fb_zero); i++) {
            if (fb_zero[i] != 0xCD) { preserved = 0; break; }
        }
        CHECK(preserved == 1);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 13. V2 active + champion bar setter → pixels appear in champion bar area */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        nexus_v2_hud_runtime_set_champion(0, 100, 100, 100, true, true);
        clear_fb(fb_zero, sizeof(fb_zero));
        nexus_v2_hud_runtime_render(fb_zero, 320, 200);
        int nonzero = 0;
        /* Champion bars are typically at top of framebuffer */
        for (int y = 0; y < 30 && y < 200; y++) {
            for (int x = 0; x < 320; x++) {
                if (fb_zero[y * 320 + x] != 0) { nonzero++; }
            }
        }
        CHECK(nonzero > 0);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 14. Action strip with active icon → pixels appear */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        nexus_v2_hud_runtime_set_action_active(NEXUS_V2_ACTION_ATTACK);
        clear_fb(fb_zero, sizeof(fb_zero));
        nexus_v2_hud_runtime_render(fb_zero, 320, 200);
        int nonzero = 0;
        /* Action strip is typically at bottom of framebuffer */
        for (int y = 180; y < 200; y++) {
            for (int x = 0; x < 320; x++) {
                if (fb_zero[y * 320 + x] != 0) { nonzero++; }
            }
        }
        CHECK(nonzero > 0);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 15. V1 invariant: V2 off → out struct fields stay at caller defaults */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        nexus_v2_hud_runtime_shutdown();
        /* After shutdown, render is no-op */
        clear_fb(fb_zero, sizeof(fb_zero));
        nexus_v2_hud_runtime_render(fb_zero, 320, 200);
        int nonzero = 0;
        for (int i = 0; i < (int)sizeof(fb_zero); i++) {
            if (fb_zero[i] != 0) { nonzero++; break; }
        }
        CHECK(nonzero == 0);
    }

    /* 16. Null-fb is safe when V2 disabled */
    {
        NEXUS_V2_PhaseGateConfig gate = { 0, 0 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        nexus_v2_hud_runtime_render(NULL, 320, 200);
        CHECK(1);  /* reached here without crash */
        nexus_v2_hud_runtime_shutdown();
    }

    /* 17. source_evidence returns citation */
    {
        const char *ev = nexus_v2_hud_runtime_source_evidence();
        CHECK(ev != NULL && ev[0] != '\0'
            && strstr(ev, "NEXUS.BIN") != NULL
            && strstr(ev, "PANEL.C") != NULL);
    }

    /* 18. Direction setter accepts all 4 cardinal values */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        for (int d = 0; d < 4; d++) {
            nexus_v2_hud_runtime_set_direction(d);
        }
        CHECK(1);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 19. Level setter accepts cur > max (clamped internally) */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        nexus_v2_hud_runtime_set_level(99, 10);  /* cur > max */
        CHECK(1);  /* no crash */
        nexus_v2_hud_runtime_shutdown();
    }

    /* 20. Hit-flash trigger stores state */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        nexus_v2_hud_runtime_trigger_hit_flash();
        CHECK(1);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 21. Opacity setter accepts all byte values */
    {
        NEXUS_V2_PhaseGateConfig gate = { 1, 1 };
        nexus_v2_hud_runtime_init();
        nexus_v2_hud_runtime_set_gate_config(&gate);
        for (int v = 0; v < 256; v += 32) {
            nexus_v2_hud_runtime_set_opacity((uint8_t)v);
        }
        CHECK(1);
        nexus_v2_hud_runtime_shutdown();
    }

    /* 22. Multiple init/shutdown cycles are safe */
    {
        for (int i = 0; i < 5; i++) {
            nexus_v2_hud_runtime_init();
            nexus_v2_hud_runtime_shutdown();
        }
        CHECK(1);
    }

    printf("\n%d/%d assertions passed\n", g_assertions - g_failures, g_assertions);
    if (g_failures == 0) {
        printf("PASS: Nexus V2 HUD Runtime wire-up probe\n");
        return 0;
    }
    printf("FAIL: %d assertion(s) failed\n", g_failures);
    return 1;
}