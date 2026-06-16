/* test_theron_v2_presentation_mode_pc34.c
 *
 * Theron V2 presentation-mode unit test. Parallel to the DM1 + CSB
 * V2 presentation-mode tests.
 */
#include "theron_v2_presentation_mode_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_failed = 0, g_total = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void t_default(void) {
    const Theron_V2_PresentationModeState* s;
    theron_v2_presentation_mode_reset();
    s = theron_v2_presentation_mode_state();
    check(s->kind == THERON_V2_PM_V1_FAITHFUL, "default V1");
    check(s->v2Active == 0, "default v2Active=0");
    check(theron_v2_presentation_mode_is_v1() == 1, "is_v1 default true");
}

static void t_set_v20(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set(THERON_V2_PM_V20_FILTERED);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V20_FILTERED, "V20 kind");
    check(theron_v2_presentation_mode_state()->v20FilterActive == 1, "V20 v20=1");
    check(theron_v2_presentation_mode_state()->v21UpscaleActive == 0, "V20 v21=0");
    check(theron_v2_presentation_mode_state()->upscaleScale == 2, "V20 scale=2");
}

static void t_set_v21(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set(THERON_V2_PM_V21_UPSCALED);
    check(theron_v2_presentation_mode_is_v21() == 1, "V21 is_v21 true");
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V21_UPSCALED, "V21 kind");
}

static void t_v22_with_pack(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V22_MODERN, "V22+pack kind");
    check(theron_v2_presentation_mode_is_v22() == 1, "V22+pack is_v22 true");
}

static void t_v22_no_pack(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(0);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V21_UPSCALED, "V22 no pack V21");
    check(theron_v2_presentation_mode_is_v22() == 0, "V22 no pack is_v22 false");
}

static void t_m12_enum(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set_m12(0);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V1_FAITHFUL, "M12 0 V1");
    theron_v2_presentation_mode_set_m12(1);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V20_FILTERED, "M12 1 V20");
    theron_v2_presentation_mode_set_m12(2);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V21_UPSCALED, "M12 2 V21");
    theron_v2_presentation_mode_set_m12(3);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V22_MODERN, "M12 3 V22");
    theron_v2_presentation_mode_set_m12(99);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V1_FAITHFUL, "M12 99 V1");
    theron_v2_presentation_mode_set_m12(-1);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V1_FAITHFUL, "M12 -1 V1");
}

static void t_names(void) {
    check(strcmp(theron_v2_presentation_mode_name(THERON_V2_PM_V1_FAITHFUL), "V1") == 0, "V1");
    check(strcmp(theron_v2_presentation_mode_name(THERON_V2_PM_V20_FILTERED), "V2.0") == 0, "V2.0");
    check(strcmp(theron_v2_presentation_mode_name(THERON_V2_PM_V21_UPSCALED), "V2.1") == 0, "V2.1");
    check(strcmp(theron_v2_presentation_mode_name(THERON_V2_PM_V22_MODERN), "V2.2") == 0, "V2.2");
}

static void t_set_count(void) {
    uint32_t c0, c1;
    theron_v2_presentation_mode_reset();
    c0 = theron_v2_presentation_mode_state()->setCount;
    theron_v2_presentation_mode_set(THERON_V2_PM_V20_FILTERED);
    c1 = theron_v2_presentation_mode_state()->setCount;
    check(c1 > c0, "setCount++");
}

static void t_reset(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    theron_v2_presentation_mode_reset();
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V1_FAITHFUL, "reset V1");
}

static void t_evidence(void) {
    const char* ev = theron_v2_presentation_mode_source_evidence();
    check(ev != NULL, "ev non-null");
    check(strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "F0359") != NULL, "ev F0359");
    check(strstr(ev, "THQUEST") != NULL, "ev THQUEST");
    check(strstr(ev, "HuC6260") != NULL || strstr(ev, "HuC6270") != NULL, "ev HuC62xx");
    check(strstr(ev, "MOVESENS") != NULL, "ev MOVESENS");
}

static void t_resolve(void) {
    check(theron_v2_presentation_mode_resolve(THERON_V2_PM_V1_FAITHFUL, 0) == THERON_V2_PM_V1_FAITHFUL, "V1->V1");
    check(theron_v2_presentation_mode_resolve(THERON_V2_PM_V22_MODERN, 0) == THERON_V2_PM_V21_UPSCALED, "V22 no pack V21");
    check(theron_v2_presentation_mode_resolve(THERON_V2_PM_V22_MODERN, 1) == THERON_V2_PM_V22_MODERN, "V22+pack V22");
}

static void t_pack_change(void) {
    theron_v2_presentation_mode_reset();
    theron_v2_presentation_mode_set_modern_pack_available(1);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    check(theron_v2_presentation_mode_is_v22() == 1, "V22+pack is_v22");
    theron_v2_presentation_mode_set_modern_pack_available(0);
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V21_UPSCALED, "pack off V21");
}

static void t_independent_from_dm1_csb(void) {
    dm1_v2_presentation_mode_reset();
    csb_v2_presentation_mode_reset();
    theron_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    csb_v2_presentation_mode_set_modern_pack_available(1);
    csb_v2_presentation_mode_set(CSB_V2_PM_V21_UPSCALED);
    theron_v2_presentation_mode_set_modern_pack_available(0);
    theron_v2_presentation_mode_set(THERON_V2_PM_V22_MODERN);
    check(dm1_v2_presentation_mode_get() == DM1_V2_PM_V22_MODERN, "DM1 V22");
    check(csb_v2_presentation_mode_get() == CSB_V2_PM_V21_UPSCALED, "CSB V21");
    check(theron_v2_presentation_mode_get() == THERON_V2_PM_V21_UPSCALED, "Theron V22 no pack V21");
    check(theron_v2_presentation_mode_is_v22() == 0, "Theron not V22");
    dm1_v2_presentation_mode_reset();
    csb_v2_presentation_mode_reset();
    theron_v2_presentation_mode_reset();
}

int main(void) {
    printf("=== Theron V2 presentation-mode test ===\n");
    t_default(); t_set_v20(); t_set_v21();
    t_v22_with_pack(); t_v22_no_pack();
    t_m12_enum(); t_names(); t_set_count(); t_reset();
    t_evidence(); t_resolve(); t_pack_change();
    t_independent_from_dm1_csb();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
