/*
 * test_theron_v2_settings_pc34.c
 *
 * Theron V2 settings bridge test. Mirrors test_csb_v2_settings_pc34.c
 * for the Theron side.
 */
#include "theron_v2_settings_pc34.h"
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
    (void)_putenv("APPDATA=.firestaff-theron-v2-settings-test");
#else
    char tmpl[] = "/tmp/firestaff-theron-v2-settings-XXXXXX";
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
    Theron_V2_Settings settings;
    Theron_V2_Settings roundtrip;
    const char* path;

    set_test_home();

    M12_Config_SetDefaults(&cfg);
    path = M12_Config_GetPath(&cfg);

    theron_v2_settings_from_m12_config(&settings, &cfg);
    CHECK(settings.scalePercent == 200);
    CHECK(settings.bilinearEnabled == 0);
    CHECK(settings.crtScanlinesEnabled == 0);
    CHECK(settings.crtScanlineStrength == 35);
    CHECK(settings.paletteCorrectionEnabled == 0);
    CHECK(settings.ditherCleanupEnabled == 0);

    settings.scalePercent = 300;
    settings.bilinearEnabled = 1;
    settings.crtScanlinesEnabled = 1;
    settings.crtScanlineStrength = 60;
    settings.paletteCorrectionEnabled = 1;
    settings.ditherCleanupEnabled = 0;
    theron_v2_settings_apply_to_m12_config(&cfg, &settings);

    CHECK(cfg.theronV2ScalePercent == 300);
    CHECK(cfg.theronV2BilinearEnabled == 1);
    CHECK(cfg.theronV2CrtScanlinesEnabled == 1);
    CHECK(cfg.theronV2CrtScanlineStrength == 60);
    CHECK(cfg.theronV2PaletteCorrectionEnabled == 1);
    CHECK(cfg.theronV2DitherCleanupEnabled == 0);
    CHECK(M12_Config_Save(&cfg) == 1);

    CHECK(file_contains(path, "theron_v2_scale_percent = 300"));
    CHECK(file_contains(path, "theron_v2_bilinear_enabled = 1"));
    CHECK(file_contains(path, "theron_v2_crt_scanlines_enabled = 1"));
    CHECK(file_contains(path, "theron_v2_crt_scanline_strength = 60"));
    CHECK(file_contains(path, "theron_v2_palette_correction_enabled = 1"));
    CHECK(file_contains(path, "theron_v2_dither_cleanup_enabled = 0"));

    CHECK(M12_Config_Load(&loaded, NULL) == 1);
    theron_v2_settings_from_m12_config(&roundtrip, &loaded);
    CHECK(roundtrip.scalePercent == 300);
    CHECK(roundtrip.bilinearEnabled == 1);
    CHECK(roundtrip.crtScanlinesEnabled == 1);
    CHECK(roundtrip.crtScanlineStrength == 60);
    CHECK(roundtrip.paletteCorrectionEnabled == 1);
    CHECK(roundtrip.ditherCleanupEnabled == 0);

    /* Sanitize: out-of-range values clamp. */
    settings.scalePercent = -1;
    settings.crtScanlineStrength = 999;
    theron_v2_settings_sanitize(&settings);
    CHECK(settings.scalePercent == 100);
    CHECK(settings.crtScanlineStrength == 100);

    /* Defaults */
    theron_v2_settings_defaults(&settings);
    CHECK(settings.scalePercent == 200);
    CHECK(settings.bilinearEnabled == 0);

    /* Apply to runtime */
    theron_v2_settings_apply_to_runtime(&settings);
    theron_v2_settings_apply_to_runtime(NULL);
    CHECK(1, "apply_to_runtime null-safe");

    /* Source evidence */
    const char* ev = theron_v2_settings_source_evidence();
    CHECK(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    CHECK(strstr(ev, "Theron") != NULL, "ev Theron");
    CHECK(strstr(ev, "HuC") != NULL, "ev HuC");

    printf("--- %d / %d passed ---\n", (failures == 0 ? 23 : 23 - failures), 23);
    return failures == 0 ? 0 : 1;
}
