/**
 * firestaff_csb_v2_settings_probe.c
 *
 * CSB V2 settings bridge headless probe.
 */
#include "csb_v2_settings_pc34.h"
#include "config_m12.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void set_test_home(void) {
#if defined(_WIN32)
    (void)_putenv("APPDATA=.firestaff-csb-v2-settings-probe");
#else
    char tmpl[] = "/tmp/firestaff-csb-v2-probe-XXXXXX";
    char* dir = mkdtemp(tmpl);
    if (dir) {
        setenv("HOME", dir, 1);
        unsetenv("XDG_CONFIG_HOME");
        unsetenv("XDG_DATA_HOME");
    }
#endif
}

static void p_defaults(void) {
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    check(s.scalePercent == 200, "default scale=200");
    check(s.bilinearEnabled == 0, "default bilinear=0");
}

static void p_roundtrip(void) {
    M12_Config cfg, loaded;
    CSB_V2_Settings s, rt;
    set_test_home();
    M12_Config_SetDefaults(&cfg);
    csb_v2_settings_from_m12_config(&s, &cfg);
    s.scalePercent = 350;
    s.bilinearEnabled = 1;
    s.crtScanlinesEnabled = 1;
    s.crtScanlineStrength = 50;
    csb_v2_settings_apply_to_m12_config(&cfg, &s);
    M12_Config_Save(&cfg);
    M12_Config_Load(&loaded, NULL);
    csb_v2_settings_from_m12_config(&rt, &loaded);
    check(rt.scalePercent == 350, "roundtrip scale=350");
    check(rt.bilinearEnabled == 1, "roundtrip bilinear=1");
    check(rt.crtScanlinesEnabled == 1, "roundtrip scanlines=1");
    check(rt.crtScanlineStrength == 50, "roundtrip scanline strength=50");
}

static void p_sanitize(void) {
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    s.scalePercent = 999;
    s.bilinearEnabled = -1;
    s.crtScanlineStrength = 200;
    csb_v2_settings_sanitize(&s);
    check(s.scalePercent == 400, "scale clamp 400");
    check(s.bilinearEnabled == 0, "bilinear clamp 0");
    check(s.crtScanlineStrength == 100, "scanline strength clamp 100");
}

static void p_apply_runtime(void) {
    CSB_V2_Settings s;
    csb_v2_settings_defaults(&s);
    csb_v2_settings_apply_to_runtime(&s);
    csb_v2_settings_apply_to_runtime(NULL);
    check(1, "apply_to_runtime null-safe");
}

static void p_evidence(void) {
    const char* ev = csb_v2_settings_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "CSB") != NULL, "ev CSB");
    check(strstr(ev, "M12") != NULL, "ev M12");
}

int main(void) {
    printf("=== CSB V2 settings bridge probe ===\n");
    p_defaults(); p_roundtrip(); p_sanitize();
    p_apply_runtime(); p_evidence();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
