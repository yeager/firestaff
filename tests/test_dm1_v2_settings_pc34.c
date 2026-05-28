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
    const char* home;
    char standalonePath[512];
    char missingPath[512];

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
    CHECK(settings.smoothTurnPanEnabled == 0);
    CHECK(settings.viewport_scale == 2);
    CHECK(settings.use_epx == 1);
    CHECK(settings.use_bilinear == 0);
    CHECK(settings.palette_enhanced == 0);
    CHECK(settings.sound_enabled == 1);
    CHECK(settings.music_enabled == 0);
    CHECK(settings.fullscreen == 0);
    CHECK(settings.vsync == 1);
    CHECK(strcmp(dm1_v2_settings_aspect_id(settings.aspectMode), "4:3-original") == 0);

    settings.scalePercent = 250;
    settings.smoothingEnabled = 0;
    settings.dynamicLightingEnabled = 0;
    settings.accessibilityTouchEnabled = 1;
    settings.aspectMode = DM1_V2_ASPECT_WIDESCREEN_16_9;
    settings.smoothTurnPanEnabled = 1;
    dm1_v2_settings_apply_to_m12_config(&cfg, &settings);

    CHECK(cfg.graphicsIndex == 0); /* V1 remains the default presentation mode. */
    CHECK(cfg.dm1V2ScalePercent == 250);
    CHECK(cfg.dm1V2SmoothingEnabled == 0);
    CHECK(cfg.dm1V2DynamicLightingEnabled == 0);
    CHECK(cfg.dm1V2AccessibilityTouchEnabled == 1);
    CHECK(cfg.dm1V2AspectMode == 1);
    CHECK(cfg.dm1V2SmoothTurnPanEnabled == 1);
    cfg.displayAspectMode = 0;
    CHECK(M12_Config_Save(&cfg) == 1);

    CHECK(file_contains(path, "dm1_v2_scale_percent = 250"));
    CHECK(file_contains(path, "dm1_v2_smoothing_enabled = 0"));
    CHECK(file_contains(path, "dm1_v2_dynamic_lighting_enabled = 0"));
    CHECK(file_contains(path, "dm1_v2_accessibility_touch_enabled = 1"));
    CHECK(file_contains(path, "dm1_v2_aspect_mode = 1"));
    CHECK(file_contains(path, "dm1_v2_smooth_turn_pan_enabled = 1"));
    CHECK(file_contains(path, "display_aspect_mode = 0"));

    CHECK(M12_Config_Load(&loaded, NULL) == 1);
    dm1_v2_settings_from_m12_config(&roundtrip, &loaded);
    CHECK(roundtrip.scalePercent == 250);
    CHECK(roundtrip.smoothingEnabled == 0);
    CHECK(roundtrip.dynamicLightingEnabled == 0);
    CHECK(roundtrip.accessibilityTouchEnabled == 1);
    CHECK(roundtrip.aspectMode == DM1_V2_ASPECT_WIDESCREEN_16_9);
    CHECK(roundtrip.smoothTurnPanEnabled == 1);
    CHECK(loaded.displayAspectMode == 0);
    CHECK(strcmp(dm1_v2_settings_aspect_id(roundtrip.aspectMode), "16:9-widescreen") == 0);

    cfg.dm1V2ScalePercent = 999;
    cfg.dm1V2SmoothingEnabled = -1;
    cfg.dm1V2DynamicLightingEnabled = 7;
    cfg.dm1V2AccessibilityTouchEnabled = 42;
    cfg.dm1V2AspectMode = 99;
    cfg.dm1V2SmoothTurnPanEnabled = -2;
    dm1_v2_settings_from_m12_config(&settings, &cfg);
    CHECK(settings.scalePercent == 400);
    CHECK(settings.smoothingEnabled == 1);
    CHECK(settings.dynamicLightingEnabled == 1);
    CHECK(settings.accessibilityTouchEnabled == 1);
    CHECK(settings.aspectMode == DM1_V2_ASPECT_ORIGINAL_4_3);
    CHECK(settings.smoothTurnPanEnabled == 1);

    home = getenv("HOME");
    snprintf(standalonePath, sizeof(standalonePath), "%s/dm1-v2-settings.ini", home ? home : ".");
    snprintf(missingPath, sizeof(missingPath), "%s/dm1-v2-settings-missing.ini", home ? home : ".");
    remove(missingPath);
    settings.scalePercent = 999;
    settings.smoothingEnabled = -3;
    settings.dynamicLightingEnabled = 2;
    settings.accessibilityTouchEnabled = 0;
    settings.aspectMode = DM1_V2_ASPECT_WIDESCREEN_16_9;
    settings.smoothTurnPanEnabled = 3;
    settings.viewport_scale = 64;
    settings.use_epx = 5;
    settings.use_bilinear = 0;
    settings.palette_enhanced = -1;
    settings.sound_enabled = 1;
    settings.music_enabled = 7;
    settings.fullscreen = 0;
    settings.vsync = 9;
    CHECK(v2_settings_save_to_file(&settings, standalonePath) == 0);
    CHECK(file_contains(standalonePath, "scale_percent=400"));
    CHECK(file_contains(standalonePath, "smooth_turn_pan=1"));
    CHECK(file_contains(standalonePath, "viewport_scale=16"));
    CHECK(file_contains(standalonePath, "epx=1"));
    CHECK(file_contains(standalonePath, "palette_enhanced=1"));
    CHECK(file_contains(standalonePath, "music=1"));

    memset(&roundtrip, 0xA5, sizeof(roundtrip));
    CHECK(v2_settings_load_from_file(&roundtrip, standalonePath) == 0);
    CHECK(roundtrip.scalePercent == 400);
    CHECK(roundtrip.smoothingEnabled == 1);
    CHECK(roundtrip.dynamicLightingEnabled == 1);
    CHECK(roundtrip.accessibilityTouchEnabled == 0);
    CHECK(roundtrip.aspectMode == DM1_V2_ASPECT_WIDESCREEN_16_9);
    CHECK(roundtrip.smoothTurnPanEnabled == 1);
    CHECK(roundtrip.viewport_scale == 16);
    CHECK(roundtrip.use_epx == 1);
    CHECK(roundtrip.use_bilinear == 0);
    CHECK(roundtrip.palette_enhanced == 1);
    CHECK(roundtrip.sound_enabled == 1);
    CHECK(roundtrip.music_enabled == 1);
    CHECK(roundtrip.fullscreen == 0);
    CHECK(roundtrip.vsync == 1);

    CHECK(v2_settings_load_from_file(&roundtrip, missingPath) == -1);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_settings_pc34: ok");
    return 0;
}
