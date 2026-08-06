/* Nexus V2 HUD Runtime Integration — smoke test
 * Validates the full init → set state → render → shutdown lifecycle
 * of the HUD runtime module without any game data or SDL.
 */

#include "nexus_v2_hud_runtime.h"
#include <stdio.h>
#include <string.h>

#define FB_W 320
#define FB_H 200

static int s_pass = 0;
static int s_fail = 0;

static void check(const char *name, int cond) {
    if (cond) { s_pass++; }
    else { printf("  FAIL: %s\n", name); s_fail++; }
}

int main(void) {
    uint8_t fb[FB_W * FB_H];
    const char *evidence;

    printf("=== Nexus V2 HUD Runtime Integration test ===\n");

    /* Not active before init */
    check("pre-init: not active", !nexus_v2_hud_runtime_is_active());

    /* Init */
    nexus_v2_hud_runtime_init();
    check("post-init: not active without gate",
          !nexus_v2_hud_runtime_is_active());

    /* Force active for test */
    nexus_v2_hud_runtime_force_active_for_test(1);
    check("force_active: is active", nexus_v2_hud_runtime_is_active());

    /* Set state */
    nexus_v2_hud_runtime_set_direction(2);
    nexus_v2_hud_runtime_set_level(3, 15);
    nexus_v2_hud_runtime_set_party_gold(42);
    nexus_v2_hud_runtime_set_champion(0, 100, 80, 60, true, false);
    nexus_v2_hud_runtime_set_action_active(0);
    nexus_v2_hud_runtime_set_opacity(200);

    /* Render into framebuffer */
    memset(fb, 0, sizeof(fb));
    nexus_v2_hud_runtime_render(fb, FB_W, FB_H);

    /* The current overlay is intentionally a no-draw diagnostic surface.
     * Production uses nexus_v2_hud_runtime_noop.c until an authenticated
     * Saturn HUD/VDP1/VDP2 placement capture exists.  Force-active is only a
     * lifecycle test hook; it must not manufacture pixels. */
    {
        int nonzero = 0;
        for (int i = 0; i < FB_W * FB_H; i++) {
            if (fb[i]) nonzero++;
        }
        check("render: no synthetic pixels written", nonzero == 0);
    }

    /* Render with zero opacity is no-op */
    nexus_v2_hud_runtime_set_opacity(0);
    memset(fb, 0, sizeof(fb));
    nexus_v2_hud_runtime_render(fb, FB_W, FB_H);
    {
        int nonzero = 0;
        for (int i = 0; i < FB_W * FB_H; i++) {
            if (fb[i]) nonzero++;
        }
        check("render: zero opacity = no pixels", nonzero == 0);
    }

    /* Hit flash */
    nexus_v2_hud_runtime_set_opacity(255);
    nexus_v2_hud_runtime_trigger_hit_flash();

    /* Source evidence */
    evidence = nexus_v2_hud_runtime_source_evidence();
    check("evidence: non-NULL", evidence != NULL);
    check("evidence: mentions Phase 3",
          evidence && strstr(evidence, "Phase 3") != NULL);

    /* Shutdown */
    nexus_v2_hud_runtime_shutdown();
    check("post-shutdown: not active", !nexus_v2_hud_runtime_is_active());

    /* Re-init after shutdown */
    nexus_v2_hud_runtime_init();
    nexus_v2_hud_runtime_force_active_for_test(1);
    check("re-init: active again", nexus_v2_hud_runtime_is_active());
    nexus_v2_hud_runtime_shutdown();

    printf("=== %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail ? 1 : 0;
}
