/**
 * firestaff_theron_v2_presentation_mode_probe.c
 *
 * Theron V2 presentation-mode headless probe.
 */
#include "theron_v2_presentation_mode_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_default(void) {
    theron_v2_presentation_mode_reset();
    const Theron_V2_PresentationModeState* s = theron_v2_presentation_mode_state();
    check(s->kind == THERON_V2_PM_V1_FAITHFUL, "default V1");
    check(s->v2Active == 0, "default v2Active=0");
}

static void p_each_mode(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set(THERON_V2_PM_V20_FILTERED);
    check(theron_v2_presentation_mode_state()->v20FilterActive == 1, "V20 v20=1");
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set(THERON_V2_PM_V21_UPSCALED);
    check(theron_v2_presentation_mode_state()->v21UpscaleActive == 1, "V21 v21=1");
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    check(theron_v2_presentation_mode_state()->v22ModernActive == 1, "V22+pack v22=1");
}

static void p_m12(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set_m12(0); check(theron_v2_presentation_mode_get() == THERON_V2_PM_V1_FAITHFUL, "M12 0 V1");
    theron_v2_presentation_mode_set_m12(1); check(theron_v2_presentation_mode_get() == THERON_V2_PM_V20_FILTERED, "M12 1 V20");
    theron_v2_presentation_mode_set_m12(2); check(theron_v2_presentation_mode_get() == THERON_V2_PM_V21_UPSCALED, "M12 2 V21");
    theron_v2_presentation_mode_set_m12(3); check(theron_v2_presentation_mode_get() == THERON_V2_PM_V22_MODERN, "M12 3 V22");
    theron_v2_presentation_mode_set_m12(99); check(theron_v2_presentation_mode_get() == THERON_V2_PM_V1_FAITHFUL, "M12 99 V1");
}

static void p_fallback(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(0);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V21_UPSCALED, "V22 no pack V21");
}

static void p_pack_turnoff(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    check(theron_v2_presentation_mode_is_v22() == 1, "V22+pack is_v22");
    theron_v2_presentation_mode_set_modern_pack_available(0);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V21_UPSCALED, "pack off V21");
}

static void p_resolve(void) {
    check(theron_v2_presentation_mode_resolve(THERON_V2_PM_V1_FAITHFUL, 0) == THERON_V2_PM_V1_FAITHFUL, "V1->V1");
    check(theron_v2_presentation_mode_resolve(THERON_V2_PM_V22_MODERN, 0) == THERON_V2_PM_V21_UPSCALED, "V22 no pack V21");
    check(theron_v2_presentation_mode_resolve(THERON_V2_PM_V22_MODERN, 1) == THERON_V2_PM_V22_MODERN, "V22+pack V22");
}

static void p_evidence(void) {
    const char* ev = theron_v2_presentation_mode_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "F0359") != NULL, "ev F0359");
    check(strstr(ev, "THQUEST") != NULL, "ev THQUEST");
    check(strstr(ev, "MOVESENS") != NULL, "ev MOVESENS");
}

static void p_independent(void) {
    dm1_v2_presentation_mode_reset();
    csb_v2_presentation_mode_reset();
    theron_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set(CSB_V2_PM_V22_MODERN);
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V22_MODERN, "DM1 V22");
    check(csb_v2_presentation_mode_get() == CSB_V2_PM_V22_MODERN, "CSB V22");
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V22_MODERN, "Theron V22 (3 independent)");
    dm1_v2_presentation_mode_reset();
    csb_v2_presentation_mode_reset();
    theron_v2_presentation_mode_reset();
}

int main(void) {
    printf("=== Theron V2 presentation-mode probe ===\n");
    p_default(); p_each_mode(); p_m12(); p_fallback();
    p_pack_turnoff(); p_resolve(); p_evidence(); p_independent();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
