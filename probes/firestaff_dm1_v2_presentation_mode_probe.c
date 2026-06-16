/**
 * firestaff_dm1_v2_presentation_mode_probe.c
 *
 * Headless probe for DM1 V2 presentation-mode selection. Mirrors
 * tests/test_dm1_v2_presentation_mode_pc34.c but runs as a
 * dedicated CI probe.
 */
#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v2_settings_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}
static void p_default(void) {
    dm1_v2_presentation_mode_reset();
    const DM1_V2_PresentationModeState* s = dm1_v2_presentation_mode_state();
    check(s->kind == DM1_V2_PM_V1_FAITHFUL, "default kind=V1");
    check(s->v2Active == 0, "default v2Active=0");
}
static void p_each_mode(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    check(dm1_v2_presentation_mode_state()->v20FilterActive == 1, "V20 v20=1");
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set(DM1_V2_PM_V21_UPSCALED);
    check(dm1_v2_presentation_mode_state()->v21UpscaleActive == 1, "V21 v21=1");
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    check(dm1_v2_presentation_mode_state()->v22ModernActive == 1, "V22+pack v22=1");
}
static void p_m12(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set_m12(0); check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V1_FAITHFUL, "M12 0 V1");
    dm1_v2_presentation_mode_set_m12(1); check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V20_FILTERED, "M12 1 V20");
    dm1_v2_presentation_mode_set_m12(2); check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V21_UPSCALED, "M12 2 V21");
    dm1_v2_presentation_mode_set_m12(3); check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V22_MODERN, "M12 3 V22");
    dm1_v2_presentation_mode_set_m12(99); check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V1_FAITHFUL, "M12 99 V1");
    dm1_v2_presentation_mode_set_m12(-1); check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V1_FAITHFUL, "M12 -1 V1");
}
static void p_fallback(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(0);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V21_UPSCALED, "V22 no pack V21");
}
static void p_pack_turnoff(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    check(dm1_v2_presentation_mode_is_v22() == 1, "V22+pack is_v22 true");
    dm1_v2_presentation_mode_set_modern_pack_available(0);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V21_UPSCALED, "pack off V22->V21");
}
static void p_resolve(void) {
    check(dm1_v2_presentation_mode_resolve(DM1_V2_PM_V1_FAITHFUL, 0) == DM1_V2_PM_V1_FAITHFUL, "V1->V1");
    check(dm1_v2_presentation_mode_resolve(DM1_V2_PM_V22_MODERN, 0) == DM1_V2_PM_V21_UPSCALED, "V22 no pack V21");
    check(dm1_v2_presentation_mode_resolve(DM1_V2_PM_V22_MODERN, 1) == DM1_V2_PM_V22_MODERN, "V22+pack V22");
}
static void p_settings(void) {
    DM1_V2_Settings s20, s21, s22;
    memset(&s20, 0x55, sizeof(s20));
    memset(&s21, 0x55, sizeof(s21));
    memset(&s22, 0x55, sizeof(s22));
    v2_settings_apply_v20_defaults(&s20);
    v2_settings_apply_v21_defaults(&s21);
    v2_settings_apply_v22_defaults(&s22);
    check(s20.viewport_scale == 1 && s21.viewport_scale == 2 && s22.viewport_scale == 2, "scales");
    check(s20.use_epx == 0 && s21.use_epx == 1 && s22.use_epx == 0, "EPX pattern");
    check(s21.use_bilinear == 0 && s22.use_bilinear == 1, "bilinear V22 only");
}
static void p_evidence(void) {
    const char* ev = dm1_v2_presentation_mode_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "F0359") != NULL, "ev F0359");
    check(strstr(ev, "F0365") != NULL, "ev F0365");
    check(strstr(ev, "CSBWin") != NULL, "ev CSBWin");
}
static void p_names(void) {
    check(strcmp(dm1_v2_presentation_mode_name(DM1_V2_PM_V1_FAITHFUL), "V1") == 0, "V1");
    check(strcmp(dm1_v2_presentation_mode_name(DM1_V2_PM_V20_FILTERED), "V2.0") == 0, "V2.0");
    check(strcmp(dm1_v2_presentation_mode_name(DM1_V2_PM_V21_UPSCALED), "V2.1") == 0, "V2.1");
    check(strcmp(dm1_v2_presentation_mode_name(DM1_V2_PM_V22_MODERN), "V2.2") == 0, "V2.2");
}
static void p_set_count(void) {
    uint32_t c0, c1, c2;
    dm1_v2_presentation_mode_reset();
    c0 = dm1_v2_presentation_mode_state()->setCount;
    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    c1 = dm1_v2_presentation_mode_state()->setCount;
    dm1_v2_presentation_mode_set(DM1_V2_PM_V21_UPSCALED);
    c2 = dm1_v2_presentation_mode_state()->setCount;
    check(c1 > c0, "setCount V20++");
    check(c2 > c1, "setCount V21++");
}
int main(void) {
    printf("=== DM1 V2 presentation-mode probe ===\n");
    p_default(); p_each_mode(); p_m12(); p_fallback();
    p_pack_turnoff(); p_resolve(); p_settings();
    p_evidence(); p_names(); p_set_count();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
