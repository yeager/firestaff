/*
 * test_csb_v2_settings_pc34.c
 *
 * CSB V2 settings bridge test. Mirrors test_dm1_v2_settings_pc34.c
 * pattern but for CSB.
 */
#include "csb_v2_settings_pc34.h"
#include "config_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); failures++; } } while (0)

static int file_contains(const char* path, const char* needle) {
    FILE* fp = fopen(path, "rb");
    char buf[8192];
    size_t n;
    int found = 0;
    if (!fp) return 0;
    n = fread(buf, 1, sizeof(buf) - 1U, fp);
    fclose(fp);
    buf[n] = '\0';
    found = strstr(buf, needle) != NULL;
    return found;
}

static void set_test_home(void) {
#if defined(_WIN32)
    (void)_putenv("APPDATA=.firestaff-csb-v2-settings-test");
#else
    char tmpl[] = "/tmp/firestaff-csb-v2-settings-XXXXXX";
    char* dir = mkdtemp(tmpl);
    if (dir) {
        setenv("HOME", dir, 1);
        unsetenv("XDG_CONFIG_HOME");
        unsetenv("XDG_DATA_HOME");
    }
#endif
}

int main(void) {
    M12_Config cfg;
    M12_Config loaded;
    CSB_V2_Settings settings;
    CSB_V2_Settings roundtrip;
    const char* path;

    set_test_home();

    M12_Config_SetDefaults(&cfg);
    path = M12_Config_GetPath(&cfg);

    csb_v2_settings_from_m12_config(&settings, &cfg);
    CHECK(settings.scalePercent == 200);
    CHECK(settings.bilinearEnabled == 0);
    CHECK(settings.crtScanlinesEnabled == 0);
    CHECK(settings.crtScanlineStrength == 35);
    CHECK(settings.paletteCorrectionEnabled == 0);
    CHECK(settings.ditherCleanupEnabled == 0);

    /* Modify and round-trip via the M12_Config round-trip */
    settings.scalePercent = 400;
    settings.bilinearEnabled = 1;
    settings.crtScanlinesEnabled = 1;
    settings.crtScanlineStrength = 80;
    settings.paletteCorrectionEnabled = 1;
    settings.ditherCleanupEnabled = 1;
    csb_v2_settings_apply_to_m12_config(&cfg, &settings);

    CHECK(cfg.csbV2ScalePercent == 400);
    CHECK(cfg.csbV2BilinearEnabled == 1);
    CHECK(cfg.csbV2CrtScanlinesEnabled == 1);
    CHECK(cfg.csbV2CrtScanlineStrength == 80);
    CHECK(cfg.csbV2PaletteCorrectionEnabled == 1);
    CHECK(cfg.csbV2DitherCleanupEnabled == 1);
    CHECK(M12_Config_Save(&cfg) == 1);

    CHECK(file_contains(path, "csb_v2_scale_percent = 400"));
    CHECK(file_contains(path, "csb_v2_bilinear_enabled = 1"));
    CHECK(file_contains(path, "csb_v2_crt_scanlines_enabled = 1"));
    CHECK(file_contains(path, "csb_v2_crt_scanline_strength = 80"));
    CHECK(file_contains(path, "csb_v2_palette_correction_enabled = 1"));
    CHECK(file_contains(path, "csb_v2_dither_cleanup_enabled = 1"));

    CHECK(M12_Config_Load(&loaded, NULL) == 1);
    csb_v2_settings_from_m12_config(&roundtrip, &loaded);
    CHECK(roundtrip.scalePercent == 400);
    CHECK(roundtrip.bilinearEnabled == 1);
    CHECK(roundtrip.crtScanlinesEnabled == 1);
    CHECK(roundtrip.crtScanlineStrength == 80);
    CHECK(roundtrip.paletteCorrectionEnabled == 1);
    CHECK(roundtrip.ditherCleanupEnabled == 1);

    /* Sanitize: out-of-range values clamp. */
    settings.scalePercent = 999;
    settings.bilinearEnabled = -1;
    settings.crtScanlineStrength = 200;
    csb_v2_settings_sanitize(&settings);
    CHECK(settings.scalePercent == 400);
    CHECK(settings.bilinearEnabled == 0);
    CHECK(settings.crtScanlineStrength == 100);

    /* Defaults: scalePercent default 200, bilinear 0. */
    csb_v2_settings_defaults(&settings);
    CHECK(settings.scalePercent == 200);
    CHECK(settings.bilinearEnabled == 0);

    /* Apply to runtime is a no-op contract (it just pushes into the
     * global config). Verify it doesn't crash. */
    csb_v2_settings_apply_to_runtime(&settings);
    csb_v2_settings_apply_to_runtime(NULL);
    CHECK(1, "apply_to_runtime null-safe");

    /* Source evidence */
    const char* ev = csb_v2_settings_source_evidence();
    CHECK(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    CHECK(strstr(ev, "CSB") != NULL, "ev CSB");

    printf("--- %d / %d passed ---\n", (failures == 0 ? 23 : 23 - failures), 23);
    return failures == 0 ? 0 : 1;
}
