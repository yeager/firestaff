#include "dm1_v2_settings_pc34.h"

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
    (void)_putenv("APPDATA=.firestaff-v2-settings-test");
#else
    char tmpl[] = "/tmp/firestaff-v2-settings-XXXXXX";
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
    DM1_V2_Settings settings;
    DM1_V2_Settings roundtrip;
    const char* path;

    set_test_home();

    M12_Config_SetDefaults(&cfg);
    path = M12_Config_GetPath(&cfg);
    CHECK(strstr(path, "startup-menu.toml") != NULL);

    dm1_v2_settings_from_m12_config(&settings, &cfg);
    CHECK(settings.scalePercent == 100);
    CHECK(settings.smoothingEnabled == 1);
    CHECK(settings.dynamicLightingEnabled == 1);
    CHECK(settings.accessibilityTouchEnabled == 0);
    CHECK(settings.aspectMode == DM1_V2_ASPECT_ORIGINAL_4_3);
    CHECK(strcmp(dm1_v2_settings_aspect_id(settings.aspectMode), "4:3-original") == 0);

    settings.scalePercent = 250;
    settings.smoothingEnabled = 0;
    settings.dynamicLightingEnabled = 0;
    settings.accessibilityTouchEnabled = 1;
    settings.aspectMode = DM1_V2_ASPECT_WIDESCREEN_16_9;
    dm1_v2_settings_apply_to_m12_config(&cfg, &settings);

    CHECK(cfg.graphicsIndex == 0); /* V1 remains the default presentation mode. */
    CHECK(cfg.dm1V2ScalePercent == 250);
    CHECK(cfg.dm1V2SmoothingEnabled == 0);
    CHECK(cfg.dm1V2DynamicLightingEnabled == 0);
    CHECK(cfg.dm1V2AccessibilityTouchEnabled == 1);
    CHECK(cfg.dm1V2AspectMode == 1);
    cfg.displayAspectMode = 0;
    CHECK(M12_Config_Save(&cfg) == 1);

    CHECK(file_contains(path, "dm1_v2_scale_percent = 250"));
    CHECK(file_contains(path, "dm1_v2_smoothing_enabled = 0"));
    CHECK(file_contains(path, "dm1_v2_dynamic_lighting_enabled = 0"));
    CHECK(file_contains(path, "dm1_v2_accessibility_touch_enabled = 1"));
    CHECK(file_contains(path, "dm1_v2_aspect_mode = 1"));
    CHECK(file_contains(path, "display_aspect_mode = 0"));

    CHECK(M12_Config_Load(&loaded, NULL) == 1);
    dm1_v2_settings_from_m12_config(&roundtrip, &loaded);
    CHECK(roundtrip.scalePercent == 250);
    CHECK(roundtrip.smoothingEnabled == 0);
    CHECK(roundtrip.dynamicLightingEnabled == 0);
    CHECK(roundtrip.accessibilityTouchEnabled == 1);
    CHECK(roundtrip.aspectMode == DM1_V2_ASPECT_WIDESCREEN_16_9);
    CHECK(loaded.displayAspectMode == 0);
    CHECK(strcmp(dm1_v2_settings_aspect_id(roundtrip.aspectMode), "16:9-widescreen") == 0);

    cfg.dm1V2ScalePercent = 999;
    cfg.dm1V2SmoothingEnabled = -1;
    cfg.dm1V2DynamicLightingEnabled = 7;
    cfg.dm1V2AccessibilityTouchEnabled = 42;
    cfg.dm1V2AspectMode = 99;
    dm1_v2_settings_from_m12_config(&settings, &cfg);
    CHECK(settings.scalePercent == 400);
    CHECK(settings.smoothingEnabled == 1);
    CHECK(settings.dynamicLightingEnabled == 1);
    CHECK(settings.accessibilityTouchEnabled == 1);
    CHECK(settings.aspectMode == DM1_V2_ASPECT_ORIGINAL_4_3);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_settings_pc34: ok");
    return 0;
}
