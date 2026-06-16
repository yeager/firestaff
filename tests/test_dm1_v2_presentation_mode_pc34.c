/* test_dm1_v2_presentation_mode_pc34.c */
#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v2_settings_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_failed = 0, g_total = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}
static void t_default(void) {
    const DM1_V2_PresentationModeState* s;
    dm1_v2_presentation_mode_reset();
    s = dm1_v2_presentation_mode_state();
    check(s->kind == DM1_V2_PM_V1_FAITHFUL, "default kind=V1");
    check(s->v2Active == 0, "default v2Active=0");
    check(dm1_v2_presentation_mode_is_v1() == 1, "is_v1 default true");
}
static void t_set_v20(void) {
    const DM1_V2_PresentationModeState* s;
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    s = dm1_v2_presentation_mode_state();
    check(s->kind == DM1_V2_PM_V20_FILTERED, "V20 kind");
    check(s->v20FilterActive == 1, "V20 v20=1");
    check(s->v21UpscaleActive == 0, "V20 v21=0");
    check(s->v22ModernActive == 0, "V20 v22=0");
    check(s->upscaleScale == 2, "V20 upscaleScale=2");
}
static void t_set_v21(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set(DM1_V2_PM_V21_UPSCALED);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V21_UPSCALED, "V21 kind");
    check(dm1_v2_presentation_mode_state()->v21UpscaleActive == 1, "V21 v21=1");
    check(dm1_v2_presentation_mode_is_v21() == 1, "V21 is_v21 true");
}
static void t_set_v22_with_pack(void) {
    const DM1_V2_PresentationModeState* s;
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    s = dm1_v2_presentation_mode_state();
    check(s->kind == DM1_V2_PM_V22_MODERN, "V22+pack kind=V22");
    check(s->v22ModernActive == 1, "V22+pack v22=1");
    check(s->modernPackAvailable == 1, "V22+pack pack=1");
}
static void t_set_v22_no_pack(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(0);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V21_UPSCALED,
          "V22 no pack -> V21 (fallback)");
}
static void t_m12_enum(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set_m12(0);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V1_FAITHFUL, "M12 0 V1");
    dm1_v2_presentation_mode_set_m12(1);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V20_FILTERED, "M12 1 V20");
    dm1_v2_presentation_mode_set_m12(2);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V21_UPSCALED, "M12 2 V21");
    dm1_v2_presentation_mode_set_m12(3);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V22_MODERN, "M12 3 V22");
    dm1_v2_presentation_mode_set_m12(99);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V1_FAITHFUL, "M12 99 V1");
    dm1_v2_presentation_mode_set_m12(-1);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V1_FAITHFUL, "M12 -1 V1");
}
static void t_names(void) {
    check(strcmp(dm1_v2_presentation_mode_name(DM1_V2_PM_V1_FAITHFUL), "V1") == 0, "name V1");
    check(strcmp(dm1_v2_presentation_mode_name(DM1_V2_PM_V20_FILTERED), "V2.0") == 0, "name V2.0");
    check(strcmp(dm1_v2_presentation_mode_name(DM1_V2_PM_V21_UPSCALED), "V2.1") == 0, "name V2.1");
    check(strcmp(dm1_v2_presentation_mode_name(DM1_V2_PM_V22_MODERN), "V2.2") == 0, "name V2.2");
}
static void t_set_count(void) {
    uint32_t c0, c1, c2;
    dm1_v2_presentation_mode_reset();
    c0 = dm1_v2_presentation_mode_state()->setCount;
    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    c1 = dm1_v2_presentation_mode_state()->setCount;
    dm1_v2_presentation_mode_set(DM1_V2_PM_V21_UPSCALED);
    c2 = dm1_v2_presentation_mode_state()->setCount;
    check(c1 > c0, "setCount++ V20");
    check(c2 > c1, "setCount++ V21");
}
static void t_reset(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    dm1_v2_presentation_mode_reset();
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V1_FAITHFUL, "reset V1");
    check(dm1_v2_presentation_mode_state()->v2Active == 0, "reset v2Active=0");
    check(dm1_v2_presentation_mode_state()->upscaleScale == 1, "reset scale=1");
}
static void t_evidence(void) {
    const char* ev = dm1_v2_presentation_mode_source_evidence();
    check(ev != NULL, "ev non-null");
    check(strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "F0359") != NULL, "ev F0359");
    check(strstr(ev, "V22") != NULL, "ev V22");
}
static void t_resolve(void) {
    check(dm1_v2_presentation_mode_resolve(DM1_V2_PM_V1_FAITHFUL, 0) == DM1_V2_PM_V1_FAITHFUL, "V1->V1");
    check(dm1_v2_presentation_mode_resolve(DM1_V2_PM_V22_MODERN, 0) == DM1_V2_PM_V21_UPSCALED, "V22 no pack V21");
    check(dm1_v2_presentation_mode_resolve(DM1_V2_PM_V22_MODERN, 1) == DM1_V2_PM_V22_MODERN, "V22+pack V22");
    check(dm1_v2_presentation_mode_resolve(DM1_V2_PM_V21_UPSCALED, 0) == DM1_V2_PM_V21_UPSCALED, "V21 V21");
}
static void t_pack_change(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    check(dm1_v2_presentation_mode_is_v22() == 1, "V22+pack is_v22 true");
    dm1_v2_presentation_mode_set_modern_pack_available(0);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V21_UPSCALED, "pack off mid-V22 V21");
}
static void t_settings(void) {
    DM1_V2_Settings s20, s21, s22;
    memset(&s20, 0x55, sizeof(s20));
    memset(&s21, 0x55, sizeof(s21));
    memset(&s22, 0x55, sizeof(s22));
    v2_settings_apply_v20_defaults(&s20);
    v2_settings_apply_v21_defaults(&s21);
    v2_settings_apply_v22_defaults(&s22);
    check(s20.viewport_scale == 1, "V20 scale=1");
    check(s21.viewport_scale == 2, "V21 scale=2");
    check(s22.viewport_scale == 2, "V22 scale=2");
    check(s20.use_epx == 0 && s21.use_epx == 1 && s22.use_epx == 0, "EPX pattern");
    check(s21.use_bilinear == 0 && s22.use_bilinear == 1, "bilinear V22 only");
    check(s20.palette_enhanced == 1, "V20 palette on");
    check(s22.palette_enhanced == 1, "V22 palette on");
}
static void t_reseat(void) {
    const DM1_V2_PresentationModeState* s;
    uint32_t c0;
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    c0 = dm1_v2_presentation_mode_state()->setCount;
    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    s = dm1_v2_presentation_mode_state();
    check(s->kind == DM1_V2_PM_V20_FILTERED, "V20-after-V22 kind");
    check(s->v22ModernActive == 0, "V20-after-V22 v22 off");
    check(s->setCount == c0 + 1, "setCount++ on re-set");
}
int main(void) {
    printf("=== DM1 V2 presentation-mode unit test ===\n");
    t_default(); t_set_v20(); t_set_v21();
    t_set_v22_with_pack(); t_set_v22_no_pack();
    t_m12_enum(); t_names(); t_set_count(); t_reset();
    t_evidence(); t_resolve(); t_pack_change();
    t_settings(); t_reseat();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
