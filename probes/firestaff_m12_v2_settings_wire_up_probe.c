/**
 * firestaff_m12_v2_settings_wire_up_probe.c
 *
 * M12 V2 settings wire-up probe. Verifies that the per-game V2
 * settings bridge (csb_v2_settings_apply_to_runtime,
 * theron_v2_settings_apply_to_runtime) correctly pushes the
 * user-saved scale + bilinear into the live V2 runtime, which is
 * what M11_GameView_OpenSelectedMenuEntry does right before
 * M11_GameView_Start.
 *
 * Schema: firestaff.m12_v2_settings_wire_up.v1
 */
#include "csb_v2_settings_pc34.h"
#include "csb_v2_texture_upscale_pc34.h"
#include "theron_v2_settings_pc34.h"
#include "theron_v2_texture_upscale_pc34.h"
#include <stdio.h>

static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_csb_scale_400(void) {
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.scalePercent = 400;
    csb_v2_settings_apply_to_runtime(&s);
    check(csb_v2_upscale_get_scale() == 4, "CSB scale=400 -> runtime scale=4");
}

static void p_csb_scale_200(void) {
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.scalePercent = 200;
    csb_v2_settings_apply_to_runtime(&s);
    check(csb_v2_upscale_get_scale() == 2, "CSB scale=200 -> runtime scale=2");
}

static void p_csb_scale_100(void) {
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.scalePercent = 100;
    csb_v2_settings_apply_to_runtime(&s);
    check(csb_v2_upscale_get_scale() == 1, "CSB scale=100 -> runtime scale=1");
}

static void p_csb_bilinear(void) {
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.bilinearEnabled = 1;
    csb_v2_settings_apply_to_runtime(&s);
    check(csb_v2_upscale_get_bilinear() == 1, "CSB bilinear=1 -> runtime bilinear=1");
    s.bilinearEnabled = 0;
    csb_v2_settings_apply_to_runtime(&s);
    check(csb_v2_upscale_get_bilinear() == 0, "CSB bilinear=0 -> runtime bilinear=0");
}

static void p_theron_scale_400(void) {
    Theron_V2_Settings s;
    theron_v2_settings_defaults(&s);
    s.scalePercent = 400;
    theron_v2_settings_apply_to_runtime(&s);
    check(theron_v2_upscale_get_scale() == 4, "Theron scale=400 -> runtime scale=4");
}

static void p_theron_scale_200(void) {
    Theron_V2_Settings s;
    theron_v2_settings_defaults(&s);
    s.scalePercent = 200;
    theron_v2_settings_apply_to_runtime(&s);
    check(theron_v2_upscale_get_scale() == 2, "Theron scale=200 -> runtime scale=2");
}

static void p_theron_scale_100(void) {
    Theron_V2_Settings s;
    theron_v2_settings_defaults(&s);
    s.scalePercent = 100;
    theron_v2_settings_apply_to_runtime(&s);
    check(theron_v2_upscale_get_scale() == 1, "Theron scale=100 -> runtime scale=1");
}

static void p_theron_bilinear(void) {
    Theron_V2_Settings s;
    theron_v2_settings_defaults(&s);
    s.bilinearEnabled = 1;
    theron_v2_settings_apply_to_runtime(&s);
    check(theron_v2_upscale_get_bilinear() == 1, "Theron bilinear=1 -> runtime bilinear=1");
    s.bilinearEnabled = 0;
    theron_v2_settings_apply_to_runtime(&s);
    check(theron_v2_upscale_get_bilinear() == 0, "Theron bilinear=0 -> runtime bilinear=0");
}

static void p_clamps_invalid(void) {
    /* 999 clamps to 4 (max). 50 clamps to 1 (min). */
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.scalePercent = 999;
    csb_v2_settings_apply_to_runtime(&s);
    check(csb_v2_upscale_get_scale() == 4, "CSB scale=999 clamps to 4");
    s.scalePercent = 50;
    csb_v2_settings_apply_to_runtime(&s);
    check(csb_v2_upscale_get_scale() == 1, "CSB scale=50 clamps to 1");
    Theron_V2_Settings t;
    theron_v2_settings_defaults(&t);
    t.scalePercent = 999;
    theron_v2_settings_apply_to_runtime(&t);
    check(theron_v2_upscale_get_scale() == 4, "Theron scale=999 clamps to 4");
    t.scalePercent = 50;
    theron_v2_settings_apply_to_runtime(&t);
    check(theron_v2_upscale_get_scale() == 1, "Theron scale=50 clamps to 1");
}

static void p_independent_globals(void) {
    /* CSB and Theron have separate global configs. Setting CSB must
     * not leak into Theron. */
    CSB_V2_Settings cs;
    csb_v2_settings_defaults(&cs);
    cs.scalePercent = 400;
    csb_v2_settings_apply_to_runtime(&cs);
    Theron_V2_Settings th;
    theron_v2_settings_defaults(&th);
    th.scalePercent = 100;
    theron_v2_settings_apply_to_runtime(&th);
    check(csb_v2_upscale_get_scale() == 4, "CSB scale=4 after Theron push");
    check(theron_v2_upscale_get_scale() == 1, "Theron scale=1 after CSB push");
}

int main(void) {
    printf("=== M12 V2 settings wire-up probe ===\n");
    p_csb_scale_400();
    p_csb_scale_200();
    p_csb_scale_100();
    p_csb_bilinear();
    p_theron_scale_400();
    p_theron_scale_200();
    p_theron_scale_100();
    p_theron_bilinear();
    p_clamps_invalid();
    p_independent_globals();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
