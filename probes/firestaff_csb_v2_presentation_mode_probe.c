/**
 * firestaff_csb_v2_presentation_mode_probe.c
 *
 * Headless probe for CSB V2 presentation-mode selection. Parallel
 * to firestaff_dm1_v2_presentation_mode_probe.c.
 */
#include "csb_v2_presentation_mode_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}
static void p_default(void) {
    csb_v2_presentation_mode_reset();
    const CSB_V2_PresentationModeState* s = csb_v2_presentation_mode_state();
    check(s->kind == CSB_V2_PM_V1_FAITHFUL, "CSB default V1");
    check(s->v2Active == 0, "CSB v2Active=0");
}
static void p_each_mode(void) {
    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set(CSB_V2_PM_V20_FILTERED);
    check(csb_v2_presentation_mode_state()->v20FilterActive == 1, "CSB V20 v20=1");
    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set(CSB_V2_PM_V21_UPSCALED);
    check(csb_v2_presentation_mode_state()->v21UpscaleActive == 1, "CSB V21 v21=1");
    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set(CSB_V2_PM_V22_MODERN);
    check(csb_v2_presentation_mode_state()->v22ModernActive == 1, "CSB V22+pack v22=1");
}
static void p_m12(void) {
    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set_m12(0); check(csb_v2_presentation_mode_get() == CSB_V2_PM_V1_FAITHFUL, "CSB M12 0 V1");
    csb_v2_presentation_mode_set_m12(1); check(csb_v2_presentation_mode_get() == CSB_V2_PM_V20_FILTERED, "CSB M12 1 V20");
    csb_v2_presentation_mode_set_m12(2); check(csb_v2_presentation_mode_get() == CSB_V2_PM_V21_UPSCALED, "CSB M12 2 V21");
    csb_v2_presentation_mode_set_m12(3); check(csb_v2_presentation_mode_get() == CSB_V2_PM_V22_MODERN, "CSB M12 3 V22");
    csb_v2_presentation_mode_set_m12(99); check(csb_v2_presentation_mode_get() == CSB_V2_PM_V1_FAITHFUL, "CSB M12 99 V1");
}
static void p_fallback(void) {
    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set_modern_pack_available(0);
    csb_v2_presentation_mode_set(CSB_V2_PM_V22_MODERN);
    check(csb_v2_presentation_mode_get() == CSB_V2_PM_V21_UPSCALED, "CSB V22 no pack V21");
}
static void p_pack_turnoff(void) {
    csb_v2_presentation_mode_reset();
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set(CSB_V2_PM_V22_MODERN);
    check(csb_v2_presentation_mode_is_v22() == 1, "CSB V22+pack is_v22 true");
    csb_v2_presentation_mode_set_modern_pack_available(0);
    check(csb_v2_presentation_mode_get() == CSB_V2_PM_V21_UPSCALED, "CSB pack off V22->V21");
}
static void p_resolve(void) {
    check(csb_v2_presentation_mode_resolve(CSB_V2_PM_V1_FAITHFUL, 0) == CSB_V2_PM_V1_FAITHFUL, "CSB V1->V1");
    check(csb_v2_presentation_mode_resolve(CSB_V2_PM_V22_MODERN, 0) == CSB_V2_PM_V21_UPSCALED, "CSB V22 no pack V21");
    check(csb_v2_presentation_mode_resolve(CSB_V2_PM_V22_MODERN, 1) == CSB_V2_PM_V22_MODERN, "CSB V22+pack V22");
}
static void p_evidence(void) {
    const char* ev = csb_v2_presentation_mode_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "CSB ev non-trivial");
    check(strstr(ev, "F0359") != NULL, "CSB ev F0359");
    check(strstr(ev, "F0365") != NULL, "CSB ev F0365");
    check(strstr(ev, "F0366") != NULL, "CSB ev F0366");
    check(strstr(ev, "CSBWin") != NULL, "CSB ev CSBWin");
    check(strstr(ev, "DUNGEON.C") != NULL, "CSB ev DUNGEON.C");
    check(strstr(ev, "ENTRANCE.C") != NULL, "CSB ev ENTRANCE.C");
}
static void p_independent(void) {
    dm1_v2_presentation_mode_reset();
    csb_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set(CSB_V2_PM_V21_UPSCALED);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V22_MODERN, "DM1 V22");
    check(csb_v2_presentation_mode_get() == CSB_V2_PM_V21_UPSCALED, "CSB V21 independent");
    csb_v2_presentation_mode_reset();
    check(csb_v2_presentation_mode_get() == CSB_V2_PM_V1_FAITHFUL, "CSB reset V1");
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V22_MODERN, "DM1 still V22");
}
int main(void) {
    printf("=== CSB V2 presentation-mode probe ===\n");
    p_default(); p_each_mode(); p_m12(); p_fallback();
    p_pack_turnoff(); p_resolve(); p_evidence(); p_independent();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
