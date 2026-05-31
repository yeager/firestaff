#include "config_m12.h"
#include "fs_portable_compat.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

static void m12_copy_string(char* out, size_t outSize, const char* value) {
    if (!out || outSize == 0U) {
        return;
    }
    if (!value) {
        value = "";
    }
    snprintf(out, outSize, "%s", value);
}

static int m12_string_equals(const char* a, const char* b) {
    return a && b && strcmp(a, b) == 0;
}

static int m12_read_quoted_value(char* out, size_t outSize, const char* raw);

static void m12_trim(char* text) {
    char* start;
    char* end;
    size_t len;
    if (!text) {
        return;
    }
    start = text;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') {
        ++start;
    }
    if (start != text) {
        memmove(text, start, strlen(start) + 1U);
    }
    len = strlen(text);
    while (len > 0U) {
        end = &text[len - 1U];
        if (*end != ' ' && *end != '\t' && *end != '\r' && *end != '\n') {
            break;
        }
        *end = '\0';
        --len;
    }
}

static int m12_parse_int(const char* value, int fallback) {
    char* end = NULL;
    long parsed;
    if (!value || value[0] == '\0') {
        return fallback;
    }
    parsed = strtol(value, &end, 10);
    if (!end || *end != '\0') {
        return fallback;
    }
    return (int)parsed;
}

static int m12_parse_presentation_mode(const char* raw, int fallback) {
    char value[64];
    char quoted[64];
    size_t i;
    if (!raw || raw[0] == '\0') {
        return fallback;
    }
    if (raw[0] == '"') {
        if (!m12_read_quoted_value(quoted, sizeof(quoted), raw)) {
            return fallback;
        }
        raw = quoted;
    }
    snprintf(value, sizeof(value), "%s", raw);
    m12_trim(value);
    for (i = 0U; value[i] != '\0'; ++i) {
        value[i] = (char)tolower((unsigned char)value[i]);
        if (value[i] == '_') {
            value[i] = '-';
        }
    }
    if (m12_string_equals(value, "v1") ||
        m12_string_equals(value, "v1-original") ||
        m12_string_equals(value, "original")) {
        return 0;
    }
    if (m12_string_equals(value, "v2") ||
        m12_string_equals(value, "v2-filtered") ||
        m12_string_equals(value, "filtered")) {
        return 1;
    }
    if (m12_string_equals(value, "v2.1") ||
        m12_string_equals(value, "v21") ||
        m12_string_equals(value, "v2-enhanced-2d") ||
        m12_string_equals(value, "enhanced-2d") ||
        m12_string_equals(value, "upscaled")) {
        return 2;
    }
    if (m12_string_equals(value, "v3") ||
        m12_string_equals(value, "v3-modern-3d") ||
        m12_string_equals(value, "modern-3d")) {
        return 3;
    }
    return fallback;
}

static const char* m12_presentation_mode_id(int index) {
    switch (index) {
        case 0:
            return "v1-original";
        case 1:
            return "v2-filtered";
        case 2:
            return "v2-enhanced-2d";
        case 3:
            return "v3-modern-3d";
        default:
            return "v1-original";
    }
}

static int m12_starts_with_lang(const char* value, const char* langCode) {
    size_t i;
    if (!value || !langCode) {
        return 0;
    }
    for (i = 0U; langCode[i] != '\0'; ++i) {
        if (value[i] == '\0') {
            return 0;
        }
        if (tolower((unsigned char)value[i]) != tolower((unsigned char)langCode[i])) {
            return 0;
        }
    }
    return value[i] == '\0' || value[i] == '_' || value[i] == '-' || value[i] == '.' || value[i] == '@';
}

int M12_Config_GetAutoLanguageIndex(void) {
    const char* candidates[] = {
        getenv("LC_ALL"),
        getenv("LC_MESSAGES"),
        getenv("LANG")
    };
    size_t i;
    for (i = 0U; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        const char* value = candidates[i];
        if (!value || value[0] == '\0') {
            continue;
        }
        if (m12_starts_with_lang(value, "sv")) {
            return 1;
        }
        if (m12_starts_with_lang(value, "fr")) {
            return 2;
        }
    }
    return 0;
}

/* m12_build_parent_dir, m12_ensure_directory, m12_default_data_dir,
 * m12_default_config_path — replaced by fs_portable_compat. */

static void m12_default_config_path(char* out, size_t outSize) {
    char configDir[FSP_PATH_MAX];
    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        if (FSP_JoinPath(out, outSize, configDir, "startup-menu.toml")) {
            return;
        }
    }
    m12_copy_string(out, outSize, "startup-menu.toml");
}

static int m12_read_quoted_value(char* out, size_t outSize, const char* raw) {
    size_t src = 0U;
    size_t dst = 0U;
    if (!out || outSize == 0U || !raw) {
        return 0;
    }
    if (raw[0] != '"') {
        return 0;
    }
    ++src;
    while (raw[src] != '\0') {
        if (raw[src] == '"') {
            out[dst] = '\0';
            return 1;
        }
        if (raw[src] == '\\' && raw[src + 1U] != '\0') {
            ++src;
        }
        if (dst + 1U >= outSize) {
            return 0;
        }
        out[dst++] = raw[src++];
    }
    return 0;
}

static void m12_escape_and_write(FILE* fp, const char* value) {
    const char* p;
    fputc('"', fp);
    for (p = value ? value : ""; *p != '\0'; ++p) {
        if (*p == '\\' || *p == '"') {
            fputc('\\', fp);
        }
        fputc(*p, fp);
    }
    fputc('"', fp);
}

void M12_Config_SetDefaults(M12_Config* config) {
    if (!config) {
        return;
    }
    memset(config, 0, sizeof(*config));
    config->languageIndex = M12_Config_GetAutoLanguageIndex();
    config->languageExplicit = 0;
    config->graphicsIndex = 0;
    config->rendererBackendIndex = 0;
    config->windowModeIndex = 1;
    config->scaleModeIndex = 4;
    config->displayAspectMode = 2; /* content-native aspect */
    config->integerScaling = 0; /* smooth FIT for full-window content */
    config->scalingFilterIndex = 0;
    config->vsyncIndex = 1;
    config->wasdMovementEnabled = 1;
    config->inputModeIndex = 0;
    config->touchControlsIndex = 0;
    config->movementModeIndex = 0;
    config->viewportStyleIndex = 0;
    config->debugOverlayIndex = 0;
    config->developerGatesIndex = 0;
    config->windowWidth = 960;
    config->windowHeight = 540;
    config->audioMasterVolume = 128;
    config->audioMusicVolume = 128;
    config->audioSfxVolume = 128;
    config->audioMuted = 0;
    config->fontScale = 1;
    config->highContrast = 0;
    config->colorblindMode = 0;
    config->autoPause = 0;
    config->controlSchemeIndex = 0;  /* default: original DM-trogen */
    config->themeIndex = 0;
    config->bgAnimationPreset = 0;
    config->dm1V2ScalePercent = 100;
    config->dm1V2SmoothingEnabled = 1;
    config->dm1V2DynamicLightingEnabled = 1;
    config->dm1V2AccessibilityTouchEnabled = 0;
    config->dm1V2AspectMode = 0;
    config->dm1V2SmoothTurnPanEnabled = 0;
    config->dm1V2CrtScanlinesEnabled = 0;
    config->dm1V2CrtScanlineStrength = 35;
    config->dm1V2PaletteCorrectionEnabled = 0;
    config->dm1V2PaletteGamma = 220;
    config->dm1V2PaletteBrightness = 0;
    config->dm1V2PaletteContrast = 0;
    config->dm1V2DitherCleanupEnabled = 0;
    config->dm1V2SharpeningEnabled = 0;
    config->dm1V2SharpeningStrength = 30;
    config->dm1V2PhosphorPersistenceEnabled = 0;
    config->dm1V2PhosphorDecay = 60;
    config->dm1V2ColorPreset = 0;
    config->dm1V2PixelGridEnabled = 0;
    config->dm1V2PixelGridIntensity = 20;
    config->dm1V2MotionBlurEnabled = 0;
    config->dm1V2MotionBlurStrength = 30;
    config->gameSpeedMultiplier = 100;
    config->minimapEnabled = 0;
    config->minimapSize = 128;
    config->minimapCorner = 0;
    config->autoMapEnabled = 1;
    config->combatLogEnabled = 0;
    config->combatLogMaxLines = 200;
    config->soundtrackMode = 0;
    config->customMusicPath[0] = '\0';
    config->ambientEnabled = 0;
    config->ambientVolume = 40;
    config->uiScale = 100;
    config->quickResumeEnabled = 1;
    config->customDungeonPath[0] = '\0';
    config->screenshotPath[0] = '\0';
    config->streamerMode = 0;
    config->v22_modern_assets_installed = 0;
    config->cloudSyncEnabled = 0;
    config->cloudSyncPolicy = 0;  /* M12_SYNC_POLICY_NEWER_WINS */
    config->cloudSyncDir[0] = '\0';
    FSP_GetDefaultOriginalsDir(config->dataDir, sizeof(config->dataDir));
    m12_default_config_path(config->path, sizeof(config->path));
}

static void m12_parse_line(M12_Config* config, char* line) {
    char* equals;
    char* key;
    char* value;
    char quoted[M12_CONFIG_DATA_DIR_CAPACITY];
    if (!config || !line) {
        return;
    }
    m12_trim(line);
    if (line[0] == '\0' || line[0] == '#') {
        return;
    }
    equals = strchr(line, '=');
    if (!equals) {
        return;
    }
    *equals = '\0';
    key = line;
    value = equals + 1;
    m12_trim(key);
    m12_trim(value);

    if (m12_string_equals(key, "language_index")) {
        config->languageIndex = m12_parse_int(value, config->languageIndex);
        return;
    }
    if (m12_string_equals(key, "language_explicit")) {
        config->languageExplicit = m12_parse_int(value, config->languageExplicit) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "presentation_mode")) {
        config->graphicsIndex = m12_parse_presentation_mode(value, config->graphicsIndex);
        return;
    }
    if (m12_string_equals(key, "graphics_index") ||
        m12_string_equals(key, "presentation_mode_index")) {
        config->graphicsIndex = m12_parse_int(value, config->graphicsIndex);
        return;
    }
    if (m12_string_equals(key, "renderer_backend_index")) {
        config->rendererBackendIndex = m12_parse_int(value, config->rendererBackendIndex);
        return;
    }
    if (m12_string_equals(key, "window_mode_index")) {
        config->windowModeIndex = m12_parse_int(value, config->windowModeIndex);
        return;
    }
    if (m12_string_equals(key, "scale_mode_index")) {
        config->scaleModeIndex = m12_parse_int(value, config->scaleModeIndex);
        return;
    }
    if (m12_string_equals(key, "display_aspect_mode")) {
        { int v = m12_parse_int(value, config->displayAspectMode); config->displayAspectMode = (v >= 0 && v <= 2) ? v : 2; }
        return;
    }
    if (m12_string_equals(key, "integer_scaling")) {
        config->integerScaling = m12_parse_int(value, config->integerScaling) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "scaling_filter_index")) {
        config->scalingFilterIndex = m12_parse_int(value, config->scalingFilterIndex);
        return;
    }
    if (m12_string_equals(key, "vsync_index")) {
        config->vsyncIndex = m12_parse_int(value, config->vsyncIndex);
        return;
    }
    if (m12_string_equals(key, "wasd_movement_enabled")) {
        config->wasdMovementEnabled = m12_parse_int(value, config->wasdMovementEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "input_mode_index")) {
        config->inputModeIndex = m12_parse_int(value, config->inputModeIndex);
        return;
    }
    if (m12_string_equals(key, "touch_controls_index")) {
        config->touchControlsIndex = m12_parse_int(value, config->touchControlsIndex);
        return;
    }
    if (m12_string_equals(key, "movement_mode_index")) {
        config->movementModeIndex = m12_parse_int(value, config->movementModeIndex);
        return;
    }
    if (m12_string_equals(key, "viewport_style_index")) {
        config->viewportStyleIndex = m12_parse_int(value, config->viewportStyleIndex);
        return;
    }
    if (m12_string_equals(key, "debug_overlay_index")) {
        config->debugOverlayIndex = m12_parse_int(value, config->debugOverlayIndex);
        return;
    }
    if (m12_string_equals(key, "developer_gates_index")) {
        config->developerGatesIndex = m12_parse_int(value, config->developerGatesIndex);
        return;
    }
    if (m12_string_equals(key, "window_width")) {
        config->windowWidth = m12_parse_int(value, config->windowWidth);
        return;
    }
    if (m12_string_equals(key, "window_height")) {
        config->windowHeight = m12_parse_int(value, config->windowHeight);
        return;
    }
    if (m12_string_equals(key, "audio_master_volume")) {
        config->audioMasterVolume = m12_parse_int(value, config->audioMasterVolume);
        return;
    }
    if (m12_string_equals(key, "audio_music_volume")) {
        config->audioMusicVolume = m12_parse_int(value, config->audioMusicVolume);
        return;
    }
    if (m12_string_equals(key, "audio_sfx_volume")) {
        config->audioSfxVolume = m12_parse_int(value, config->audioSfxVolume);
        return;
    }
    if (m12_string_equals(key, "audio_muted")) {
        config->audioMuted = m12_parse_int(value, config->audioMuted);
        return;
    }
    if (m12_string_equals(key, "game_speed_multiplier")) {
        int v = m12_parse_int(value, config->gameSpeedMultiplier);
        if (v != 50 && v != 100 && v != 150 && v != 200) v = 100;
        config->gameSpeedMultiplier = v;
        return;
    }
    if (m12_string_equals(key, "minimap_enabled")) {
        config->minimapEnabled = m12_parse_int(value, config->minimapEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "minimap_size")) {
        int v = m12_parse_int(value, config->minimapSize);
        if (v < 64) v = 64; else if (v > 256) v = 256;
        config->minimapSize = v;
        return;
    }
    if (m12_string_equals(key, "minimap_corner")) {
        int v = m12_parse_int(value, config->minimapCorner);
        if (v < 0 || v > 3) v = 0;
        config->minimapCorner = v;
        return;
    }
    if (m12_string_equals(key, "auto_map_enabled")) {
        config->autoMapEnabled = m12_parse_int(value, config->autoMapEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "combat_log_enabled")) {
        config->combatLogEnabled = m12_parse_int(value, config->combatLogEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "combat_log_max_lines")) {
        int v = m12_parse_int(value, config->combatLogMaxLines);
        if (v < 50) v = 50; else if (v > 500) v = 500;
        config->combatLogMaxLines = v;
        return;
    }
    if (strncmp(key, "game_", 5) == 0) {
        int gameIndex = -1;
        char field[64];
        if (sscanf(key, "game_%d_%63s", &gameIndex, field) == 2 &&
            gameIndex >= 0 && gameIndex < M12_CONFIG_GAME_COUNT) {
            if (m12_string_equals(field, "use_patch")) {
                config->gameUsePatch[gameIndex] = m12_parse_int(value, config->gameUsePatch[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "version_index")) {
                config->gameVersionIndex[gameIndex] = m12_parse_int(value, config->gameVersionIndex[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "language_index")) {
                config->gameLanguageIndex[gameIndex] = m12_parse_int(value, config->gameLanguageIndex[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "cheats_enabled")) {
                config->gameCheatsEnabled[gameIndex] = m12_parse_int(value, config->gameCheatsEnabled[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "speed")) {
                config->gameSpeed[gameIndex] = m12_parse_int(value, config->gameSpeed[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "aspect_ratio")) {
                config->gameAspectRatio[gameIndex] = m12_parse_int(value, config->gameAspectRatio[gameIndex]);
                return;
            }
            if (m12_string_equals(field, "resolution")) {
                config->gameResolution[gameIndex] = m12_parse_int(value, config->gameResolution[gameIndex]);
                return;
            }
        }
    }
    if (m12_string_equals(key, "font_scale")) {
        int val = m12_parse_int(value, config->fontScale);
        if (val < 1) val = 1;
        if (val > 3) val = 3;
        config->fontScale = val;
        return;
    }
    if (m12_string_equals(key, "high_contrast")) {
        config->highContrast = m12_parse_int(value, config->highContrast) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "colorblind_mode")) {
        int val = m12_parse_int(value, config->colorblindMode);
        if (val < 0) val = 0;
        if (val > 3) val = 3;
        config->colorblindMode = val;
        return;
    }
    if (m12_string_equals(key, "auto_pause")) {
        config->autoPause = m12_parse_int(value, config->autoPause) ? 1 : 0;
        } else if (strcmp(key, "control_scheme") == 0) {
            config->controlSchemeIndex = m12_parse_int(value, config->controlSchemeIndex);
            if (config->controlSchemeIndex < 0 || config->controlSchemeIndex > 1) config->controlSchemeIndex = 0;
        return;
    }
    if (m12_string_equals(key, "theme_index")) {
        int val = m12_parse_int(value, config->themeIndex);
        if (val < 0) val = 0;
        if (val >= 4) val = 3;
        config->themeIndex = val;
        return;
    }
    if (m12_string_equals(key, "bg_animation_preset")) {
        int val = m12_parse_int(value, config->bgAnimationPreset);
        if (val < 0) val = 0;
        if (val >= 4) val = 3;
        config->bgAnimationPreset = val;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_scale_percent")) {
        int val = m12_parse_int(value, config->dm1V2ScalePercent);
        if (val < 100) val = 100;
        if (val > 400) val = 400;
        config->dm1V2ScalePercent = val;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_smoothing_enabled")) {
        config->dm1V2SmoothingEnabled = m12_parse_int(value, config->dm1V2SmoothingEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_dynamic_lighting_enabled")) {
        config->dm1V2DynamicLightingEnabled = m12_parse_int(value, config->dm1V2DynamicLightingEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_accessibility_touch_enabled")) {
        config->dm1V2AccessibilityTouchEnabled = m12_parse_int(value, config->dm1V2AccessibilityTouchEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_aspect_mode")) {
        config->dm1V2AspectMode = m12_parse_int(value, config->dm1V2AspectMode) == 1 ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_smooth_turn_pan_enabled")) {
        config->dm1V2SmoothTurnPanEnabled = m12_parse_int(value, config->dm1V2SmoothTurnPanEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_crt_scanlines_enabled")) {
        config->dm1V2CrtScanlinesEnabled = m12_parse_int(value, config->dm1V2CrtScanlinesEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_crt_scanline_strength")) {
        int val = m12_parse_int(value, config->dm1V2CrtScanlineStrength);
        if (val < 0) val = 0;
        if (val > 100) val = 100;
        config->dm1V2CrtScanlineStrength = val;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_palette_correction_enabled")) {
        config->dm1V2PaletteCorrectionEnabled = m12_parse_int(value, config->dm1V2PaletteCorrectionEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_palette_gamma")) {
        int val = m12_parse_int(value, config->dm1V2PaletteGamma);
        if (val < 80) val = 80;
        if (val > 260) val = 260;
        config->dm1V2PaletteGamma = val;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_palette_brightness")) {
        int val = m12_parse_int(value, config->dm1V2PaletteBrightness);
        if (val < -50) val = -50;
        if (val > 50) val = 50;
        config->dm1V2PaletteBrightness = val;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_palette_contrast")) {
        int val = m12_parse_int(value, config->dm1V2PaletteContrast);
        if (val < -50) val = -50;
        if (val > 50) val = 50;
        config->dm1V2PaletteContrast = val;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_dither_cleanup_enabled")) {
        config->dm1V2DitherCleanupEnabled = m12_parse_int(value, config->dm1V2DitherCleanupEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_sharpening_enabled")) {
        config->dm1V2SharpeningEnabled = m12_parse_int(value, config->dm1V2SharpeningEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_sharpening_strength")) {
        int val = m12_parse_int(value, config->dm1V2SharpeningStrength);
        if (val < 0) val = 0;
        if (val > 100) val = 100;
        config->dm1V2SharpeningStrength = val;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_phosphor_persistence_enabled")) {
        config->dm1V2PhosphorPersistenceEnabled = m12_parse_int(value, config->dm1V2PhosphorPersistenceEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_phosphor_decay")) {
        int val = m12_parse_int(value, config->dm1V2PhosphorDecay);
        if (val < 0) val = 0;
        if (val > 100) val = 100;
        config->dm1V2PhosphorDecay = val;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_color_preset")) {
        int val = m12_parse_int(value, config->dm1V2ColorPreset);
        if (val < 0) val = 0;
        if (val > 6) val = 6;
        config->dm1V2ColorPreset = val;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_pixel_grid_enabled")) {
        config->dm1V2PixelGridEnabled = m12_parse_int(value, config->dm1V2PixelGridEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_pixel_grid_intensity")) {
        int val = m12_parse_int(value, config->dm1V2PixelGridIntensity);
        if (val < 0) val = 0;
        if (val > 100) val = 100;
        config->dm1V2PixelGridIntensity = val;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_motion_blur_enabled")) {
        config->dm1V2MotionBlurEnabled = m12_parse_int(value, config->dm1V2MotionBlurEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "dm1_v2_motion_blur_strength")) {
        int val = m12_parse_int(value, config->dm1V2MotionBlurStrength);
        if (val < 0) val = 0;
        if (val > 100) val = 100;
        config->dm1V2MotionBlurStrength = val;
        return;
    }
    if (m12_string_equals(key, "soundtrack_mode")) {
        int val = m12_parse_int(value, config->soundtrackMode);
        if (val < 0) val = 0;
        if (val > 2) val = 2;
        config->soundtrackMode = val;
        return;
    }
    if (m12_string_equals(key, "custom_music_path") &&
        m12_read_quoted_value(quoted, sizeof(quoted), value)) {
        m12_copy_string(config->customMusicPath, sizeof(config->customMusicPath), quoted);
        return;
    }
    if (m12_string_equals(key, "ambient_enabled")) {
        config->ambientEnabled = m12_parse_int(value, config->ambientEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "ambient_volume")) {
        int val = m12_parse_int(value, config->ambientVolume);
        if (val < 0) val = 0;
        if (val > 100) val = 100;
        config->ambientVolume = val;
        return;
    }
    if (m12_string_equals(key, "ui_scale")) {
        int val = m12_parse_int(value, config->uiScale);
        if (val <= 100) val = 100;
        else if (val <= 150) val = 150;
        else val = 200;
        config->uiScale = val;
        return;
    }
    if (m12_string_equals(key, "data_dir") &&
        m12_read_quoted_value(quoted, sizeof(quoted), value)) {
        m12_copy_string(config->dataDir, sizeof(config->dataDir), quoted);
        return;
    }
    if (m12_string_equals(key, "quick_resume_enabled")) {
        config->quickResumeEnabled = m12_parse_int(value, config->quickResumeEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "custom_dungeon_path") &&
        m12_read_quoted_value(quoted, sizeof(quoted), value)) {
        m12_copy_string(config->customDungeonPath, sizeof(config->customDungeonPath), quoted);
        return;
    }
    if (m12_string_equals(key, "screenshot_path") &&
        m12_read_quoted_value(quoted, sizeof(quoted), value)) {
        m12_copy_string(config->screenshotPath, sizeof(config->screenshotPath), quoted);
        return;
    }
    if (m12_string_equals(key, "streamer_mode")) {
        config->streamerMode = m12_parse_int(value, config->streamerMode) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "cloud_sync_enabled")) {
        config->cloudSyncEnabled = m12_parse_int(value, config->cloudSyncEnabled) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "cloud_sync_policy")) {
        config->cloudSyncPolicy = m12_parse_int(value, config->cloudSyncPolicy);
        return;
    }
    if (m12_string_equals(key, "cloud_sync_dir") &&
        m12_read_quoted_value(quoted, sizeof(quoted), value)) {
        m12_copy_string(config->cloudSyncDir, sizeof(config->cloudSyncDir), quoted);
        return;
    }
    if (m12_string_equals(key, "v22_modern_assets_installed")) {
        config->v22_modern_assets_installed = m12_parse_int(value, 0) ? 1 : 0;
        return;
    }
    if (m12_string_equals(key, "last_save_path") &&
        m12_read_quoted_value(quoted, sizeof(quoted), value)) {
        m12_copy_string(config->lastSavePath, sizeof(config->lastSavePath), quoted);
    }
}

int M12_Config_Save(const M12_Config* config) {
    char parentDir[M12_CONFIG_PATH_CAPACITY];
    char tmpPath[M12_CONFIG_PATH_CAPACITY + 16];
    FILE* fp;
    if (!config || config->path[0] == '\0') {
        return 0;
    }
    if (FSP_ParentDir(parentDir, sizeof(parentDir), config->path)) {
        if (!FSP_CreateDirectoryRecursive(parentDir)) {
            return 0;
        }
    }
    snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", config->path);
    fp = fopen(tmpPath, "wb");
    if (!fp) {
        return 0;
    }
    fprintf(fp, "# Firestaff startup menu config\n");
    fprintf(fp, "language_index = %d\n", config->languageIndex);
    fprintf(fp, "language_explicit = %d\n", config->languageExplicit ? 1 : 0);
    fprintf(fp, "presentation_mode_index = %d\n", config->graphicsIndex);
    fprintf(fp, "graphics_index = %d\n", config->graphicsIndex);
    fputs("presentation_mode = ", fp);
    m12_escape_and_write(fp, m12_presentation_mode_id(config->graphicsIndex));
    fputc('\n', fp);
    fprintf(fp, "renderer_backend_index = %d\n", config->rendererBackendIndex);
    fprintf(fp, "window_mode_index = %d\n", config->windowModeIndex);
    fprintf(fp, "scale_mode_index = %d\n", config->scaleModeIndex);
    fprintf(fp, "display_aspect_mode = %d\n", config->displayAspectMode);
    fprintf(fp, "integer_scaling = %d\n", config->integerScaling ? 1 : 0);
    fprintf(fp, "scaling_filter_index = %d\n", config->scalingFilterIndex);
    fprintf(fp, "vsync_index = %d\n", config->vsyncIndex);
    fprintf(fp, "wasd_movement_enabled = %d\n", config->wasdMovementEnabled ? 1 : 0);
    fprintf(fp, "input_mode_index = %d\n", config->inputModeIndex);
    fprintf(fp, "touch_controls_index = %d\n", config->touchControlsIndex);
    fprintf(fp, "movement_mode_index = %d\n", config->movementModeIndex);
    fprintf(fp, "viewport_style_index = %d\n", config->viewportStyleIndex);
    fprintf(fp, "debug_overlay_index = %d\n", config->debugOverlayIndex);
    fprintf(fp, "developer_gates_index = %d\n", config->developerGatesIndex);
    fprintf(fp, "window_width = %d\n", config->windowWidth);
    fprintf(fp, "window_height = %d\n", config->windowHeight);
    fprintf(fp, "audio_master_volume = %d\n", config->audioMasterVolume);
    fprintf(fp, "audio_music_volume = %d\n", config->audioMusicVolume);
    fprintf(fp, "audio_sfx_volume = %d\n", config->audioSfxVolume);
    fprintf(fp, "audio_muted = %d\n", config->audioMuted ? 1 : 0);
    fprintf(fp, "game_speed_multiplier = %d\n", config->gameSpeedMultiplier);
    fprintf(fp, "minimap_enabled = %d\n", config->minimapEnabled ? 1 : 0);
    fprintf(fp, "minimap_size = %d\n", config->minimapSize);
    fprintf(fp, "minimap_corner = %d\n", config->minimapCorner);
    fprintf(fp, "auto_map_enabled = %d\n", config->autoMapEnabled ? 1 : 0);
    fprintf(fp, "combat_log_enabled = %d\n", config->combatLogEnabled ? 1 : 0);
    fprintf(fp, "combat_log_max_lines = %d\n", config->combatLogMaxLines);
    {
        int gi;
        for (gi = 0; gi < M12_CONFIG_GAME_COUNT; ++gi) {
            fprintf(fp, "game_%d_use_patch = %d\n", gi, config->gameUsePatch[gi]);
            fprintf(fp, "game_%d_version_index = %d\n", gi, config->gameVersionIndex[gi]);
            fprintf(fp, "game_%d_language_index = %d\n", gi, config->gameLanguageIndex[gi]);
            fprintf(fp, "game_%d_cheats_enabled = %d\n", gi, config->gameCheatsEnabled[gi]);
            fprintf(fp, "game_%d_speed = %d\n", gi, config->gameSpeed[gi]);
            fprintf(fp, "game_%d_aspect_ratio = %d\n", gi, config->gameAspectRatio[gi]);
            fprintf(fp, "game_%d_resolution = %d\n", gi, config->gameResolution[gi]);
        }
    }
    fprintf(fp, "font_scale = %d\n", config->fontScale);
    fprintf(fp, "high_contrast = %d\n", config->highContrast ? 1 : 0);
    fprintf(fp, "colorblind_mode = %d\n", config->colorblindMode);
    fprintf(fp, "auto_pause = %d\n", config->autoPause ? 1 : 0);
    fprintf(fp, "control_scheme = %d\n", config->controlSchemeIndex);
    fprintf(fp, "theme_index = %d\n", config->themeIndex);
    fprintf(fp, "bg_animation_preset = %d\n", config->bgAnimationPreset);
    fprintf(fp, "dm1_v2_scale_percent = %d\n", config->dm1V2ScalePercent);
    fprintf(fp, "dm1_v2_smoothing_enabled = %d\n", config->dm1V2SmoothingEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_dynamic_lighting_enabled = %d\n", config->dm1V2DynamicLightingEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_accessibility_touch_enabled = %d\n", config->dm1V2AccessibilityTouchEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_aspect_mode = %d\n", config->dm1V2AspectMode == 1 ? 1 : 0);
    fprintf(fp, "dm1_v2_smooth_turn_pan_enabled = %d\n", config->dm1V2SmoothTurnPanEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_crt_scanlines_enabled = %d\n", config->dm1V2CrtScanlinesEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_crt_scanline_strength = %d\n", config->dm1V2CrtScanlineStrength);
    fprintf(fp, "dm1_v2_palette_correction_enabled = %d\n", config->dm1V2PaletteCorrectionEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_palette_gamma = %d\n", config->dm1V2PaletteGamma);
    fprintf(fp, "dm1_v2_palette_brightness = %d\n", config->dm1V2PaletteBrightness);
    fprintf(fp, "dm1_v2_palette_contrast = %d\n", config->dm1V2PaletteContrast);
    fprintf(fp, "dm1_v2_dither_cleanup_enabled = %d\n", config->dm1V2DitherCleanupEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_sharpening_enabled = %d\n", config->dm1V2SharpeningEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_sharpening_strength = %d\n", config->dm1V2SharpeningStrength);
    fprintf(fp, "dm1_v2_phosphor_persistence_enabled = %d\n", config->dm1V2PhosphorPersistenceEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_phosphor_decay = %d\n", config->dm1V2PhosphorDecay);
    fprintf(fp, "dm1_v2_color_preset = %d\n", config->dm1V2ColorPreset);
    fprintf(fp, "dm1_v2_pixel_grid_enabled = %d\n", config->dm1V2PixelGridEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_pixel_grid_intensity = %d\n", config->dm1V2PixelGridIntensity);
    fprintf(fp, "dm1_v2_motion_blur_enabled = %d\n", config->dm1V2MotionBlurEnabled ? 1 : 0);
    fprintf(fp, "dm1_v2_motion_blur_strength = %d\n", config->dm1V2MotionBlurStrength);
    fprintf(fp, "soundtrack_mode = %d\n", config->soundtrackMode);
    fputs("custom_music_path = ", fp);
    m12_escape_and_write(fp, config->customMusicPath);
    fputc('\n', fp);
    fprintf(fp, "ambient_enabled = %d\n", config->ambientEnabled ? 1 : 0);
    fprintf(fp, "ambient_volume = %d\n", config->ambientVolume);
    fprintf(fp, "ui_scale = %d\n", config->uiScale);
    fprintf(fp, "quick_resume_enabled = %d\n", config->quickResumeEnabled ? 1 : 0);
    fputs("custom_dungeon_path = ", fp); m12_escape_and_write(fp, config->customDungeonPath); fputc('\n', fp);
    fputs("screenshot_path = ", fp); m12_escape_and_write(fp, config->screenshotPath); fputc('\n', fp);
    fprintf(fp, "streamer_mode = %d\n", config->streamerMode ? 1 : 0);
    fprintf(fp, "cloud_sync_enabled = %d\n", config->cloudSyncEnabled ? 1 : 0);
    fprintf(fp, "cloud_sync_policy = %d\n", config->cloudSyncPolicy);
    fprintf(fp, "cloud_sync_dir = "); m12_escape_and_write(fp, config->cloudSyncDir); fputc('\n', fp);
    fprintf(fp, "v22_modern_assets_installed = %d\n", config->v22_modern_assets_installed ? 1 : 0);
    fputs("data_dir = ", fp);
    m12_escape_and_write(fp, config->dataDir);
    fputc('\n', fp);
    fputs("last_save_path = ", fp);
    m12_escape_and_write(fp, config->lastSavePath);
    fputc('\n', fp);
    if (fclose(fp) != 0) {
        remove(tmpPath);
        return 0;
    }
    if (rename(tmpPath, config->path) != 0) {
        remove(tmpPath);
        return 0;
    }
    return 1;
}

int M12_Config_Load(M12_Config* config, const char* dataDirOverride) {
    FILE* fp;
    char line[1024];
    int hadExistingFile;
    int shouldSave = 0;
    if (!config) {
        return 0;
    }
    M12_Config_SetDefaults(config);
    if (dataDirOverride && dataDirOverride[0] != '\0') {
        m12_copy_string(config->dataDir, sizeof(config->dataDir), dataDirOverride);
    }
    fp = fopen(config->path, "rb");
    if (!fp) {
        M12_Config_Save(config);
        return 0;
    }
    hadExistingFile = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        m12_parse_line(config, line);
    }
    fclose(fp);
    if (!config->languageExplicit) {
        int autoLanguage = M12_Config_GetAutoLanguageIndex();
        if (config->languageIndex != autoLanguage) {
            config->languageIndex = autoLanguage;
            shouldSave = 1;
        }
    }
    if (dataDirOverride && dataDirOverride[0] != '\0') {
        m12_copy_string(config->dataDir, sizeof(config->dataDir), dataDirOverride);
        shouldSave = 1;
    }
    if (config->dataDir[0] == '\0') {
        FSP_ResolveDataDir(config->dataDir, sizeof(config->dataDir), NULL);
        shouldSave = 1;
    }
    if (shouldSave) {
        M12_Config_Save(config);
    }
    return hadExistingFile;
}

const char* M12_Config_GetPath(const M12_Config* config) {
    if (!config || config->path[0] == '\0') {
        return "startup-menu.toml";
    }
    return config->path;
}

void M12_Config_SetLastSavePath(const char* path) {
    M12_Config config;
    M12_Config_Load(&config, NULL);
    m12_copy_string(config.lastSavePath, sizeof(config.lastSavePath),
                    path ? path : "");
    M12_Config_Save(&config);
}

/* ── JSON export path helper ─────────────────────────────────────────── */
static void m12_default_export_path(char* out, size_t outSize) {
    char configDir[FSP_PATH_MAX];
    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        if (FSP_JoinPath(out, outSize, configDir, "firestaff-settings-export.json")) {
            return;
        }
    }
    m12_copy_string(out, outSize, "firestaff-settings-export.json");
}

/* ── JSON string serialization helpers ───────────────────────────────── */
static void m12_json_write_string(FILE* fp, const char* value) {
    const char* p;
    fputc('"', fp);
    for (p = value ? value : ""; *p != '\0'; ++p) {
        switch (*p) {
            case '\\': fputs("\\\\", fp); break;
            case '"':  fputs("\\\"", fp); break;
            case '\n': fputs("\\n", fp);  break;
            case '\r': fputs("\\r", fp);  break;
            case '\t': fputs("\\t", fp);  break;
            default:   fputc(*p, fp);       break;
        }
    }
    fputc('"', fp);
}

/* ── M12_Config_ExportJSON ─────────────────────────────────────────────
 * Serializes the full M12_Config to a JSON file at exportPath.
 * Returns 1 on success, 0 on failure.
 * If exportPath is NULL, uses the default path in the user config dir.
 * ───────────────────────────────────────────────────────────────────── */
int M12_Config_ExportJSON(const M12_Config* config, const char* exportPath) {
    char pathBuf[FSP_PATH_MAX];
    char tmpPathBuf[FSP_PATH_MAX + 16];
    const char* path;
    FILE* fp;
    int gi;

    if (!config) {
        return 0;
    }

    if (!exportPath || exportPath[0] == '\0') {
        m12_default_export_path(pathBuf, sizeof(pathBuf));
        path = pathBuf;
    } else {
        path = exportPath;
    }

    /* Ensure parent directory exists */
    {
        char parentDir[FSP_PATH_MAX];
        if (FSP_ParentDir(parentDir, sizeof(parentDir), path)) {
            FSP_CreateDirectoryRecursive(parentDir);
        }
    }

    snprintf(tmpPathBuf, sizeof(tmpPathBuf), "%s.tmp", path);
    fp = fopen(tmpPathBuf, "wb");
    if (!fp) {
        return 0;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"version\": \"1.0\",\n");
    fprintf(fp, "  \"type\": \"firestaff-launcher-settings\",\n");

    /* ── Global settings ── */
    fprintf(fp, "  \"language_index\": %d,\n", config->languageIndex);
    fprintf(fp, "  \"language_explicit\": %d,\n", config->languageExplicit ? 1 : 0);
    fprintf(fp, "  \"graphics_index\": %d,\n", config->graphicsIndex);
    fprintf(fp, "  \"renderer_backend_index\": %d,\n", config->rendererBackendIndex);
    fprintf(fp, "  \"window_mode_index\": %d,\n", config->windowModeIndex);
    fprintf(fp, "  \"scale_mode_index\": %d,\n", config->scaleModeIndex);
    fprintf(fp, "  \"display_aspect_mode\": %d,\n", config->displayAspectMode);
    fprintf(fp, "  \"integer_scaling\": %d,\n", config->integerScaling ? 1 : 0);
    fprintf(fp, "  \"scaling_filter_index\": %d,\n", config->scalingFilterIndex);
    fprintf(fp, "  \"vsync_index\": %d,\n", config->vsyncIndex);
    fprintf(fp, "  \"wasd_movement_enabled\": %d,\n", config->wasdMovementEnabled ? 1 : 0);
    fprintf(fp, "  \"input_mode_index\": %d,\n", config->inputModeIndex);
    fprintf(fp, "  \"touch_controls_index\": %d,\n", config->touchControlsIndex);
    fprintf(fp, "  \"movement_mode_index\": %d,\n", config->movementModeIndex);
    fprintf(fp, "  \"viewport_style_index\": %d,\n", config->viewportStyleIndex);
    fprintf(fp, "  \"debug_overlay_index\": %d,\n", config->debugOverlayIndex);
    fprintf(fp, "  \"developer_gates_index\": %d,\n", config->developerGatesIndex);
    fprintf(fp, "  \"window_width\": %d,\n", config->windowWidth);
    fprintf(fp, "  \"window_height\": %d,\n", config->windowHeight);
    fprintf(fp, "  \"audio_master_volume\": %d,\n", config->audioMasterVolume);
    fprintf(fp, "  \"audio_music_volume\": %d,\n", config->audioMusicVolume);
    fprintf(fp, "  \"audio_sfx_volume\": %d,\n", config->audioSfxVolume);
    fprintf(fp, "  \"audio_muted\": %d,\n", config->audioMuted ? 1 : 0);
    fprintf(fp, "  \"font_scale\": %d,\n", config->fontScale);
    fprintf(fp, "  \"high_contrast\": %d,\n", config->highContrast ? 1 : 0);
    fprintf(fp, "  \"colorblind_mode\": %d,\n", config->colorblindMode);
    fprintf(fp, "  \"auto_pause\": %d,\n", config->autoPause ? 1 : 0);
    fprintf(fp, "  \"control_scheme\": %d,\n", config->controlSchemeIndex);
    fprintf(fp, "  \"theme_index\": %d,\n", config->themeIndex);
    fprintf(fp, "  \"bg_animation_preset\": %d,\n", config->bgAnimationPreset);
    fprintf(fp, "  \"dm1_v2_scale_percent\": %d,\n", config->dm1V2ScalePercent);
    fprintf(fp, "  \"dm1_v2_smoothing_enabled\": %d,\n", config->dm1V2SmoothingEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_dynamic_lighting_enabled\": %d,\n", config->dm1V2DynamicLightingEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_accessibility_touch_enabled\": %d,\n", config->dm1V2AccessibilityTouchEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_aspect_mode\": %d,\n", config->dm1V2AspectMode ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_smooth_turn_pan_enabled\": %d,\n", config->dm1V2SmoothTurnPanEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_crt_scanlines_enabled\": %d,\n", config->dm1V2CrtScanlinesEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_crt_scanline_strength\": %d,\n", config->dm1V2CrtScanlineStrength);
    fprintf(fp, "  \"dm1_v2_palette_correction_enabled\": %d,\n", config->dm1V2PaletteCorrectionEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_palette_gamma\": %d,\n", config->dm1V2PaletteGamma);
    fprintf(fp, "  \"dm1_v2_palette_brightness\": %d,\n", config->dm1V2PaletteBrightness);
    fprintf(fp, "  \"dm1_v2_palette_contrast\": %d,\n", config->dm1V2PaletteContrast);
    fprintf(fp, "  \"dm1_v2_dither_cleanup_enabled\": %d,\n", config->dm1V2DitherCleanupEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_sharpening_enabled\": %d,\n", config->dm1V2SharpeningEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_sharpening_strength\": %d,\n", config->dm1V2SharpeningStrength);
    fprintf(fp, "  \"dm1_v2_phosphor_persistence_enabled\": %d,\n", config->dm1V2PhosphorPersistenceEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_phosphor_decay\": %d,\n", config->dm1V2PhosphorDecay);
    fprintf(fp, "  \"dm1_v2_color_preset\": %d,\n", config->dm1V2ColorPreset);
    fprintf(fp, "  \"dm1_v2_pixel_grid_enabled\": %d,\n", config->dm1V2PixelGridEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_pixel_grid_intensity\": %d,\n", config->dm1V2PixelGridIntensity);
    fprintf(fp, "  \"dm1_v2_motion_blur_enabled\": %d,\n", config->dm1V2MotionBlurEnabled ? 1 : 0);
    fprintf(fp, "  \"dm1_v2_motion_blur_strength\": %d,\n", config->dm1V2MotionBlurStrength);
    fprintf(fp, "  \"game_speed_multiplier\": %d,\n", config->gameSpeedMultiplier);
    fprintf(fp, "  \"minimap_enabled\": %d,\n", config->minimapEnabled ? 1 : 0);
    fprintf(fp, "  \"minimap_size\": %d,\n", config->minimapSize);
    fprintf(fp, "  \"minimap_corner\": %d,\n", config->minimapCorner);
    fprintf(fp, "  \"auto_map_enabled\": %d,\n", config->autoMapEnabled ? 1 : 0);
    fprintf(fp, "  \"combat_log_enabled\": %d,\n", config->combatLogEnabled ? 1 : 0);
    fprintf(fp, "  \"combat_log_max_lines\": %d,\n", config->combatLogMaxLines);
    fprintf(fp, "  \"soundtrack_mode\": %d,\n", config->soundtrackMode);
    fprintf(fp, "  \"ambient_enabled\": %d,\n", config->ambientEnabled ? 1 : 0);
    fprintf(fp, "  \"ambient_volume\": %d,\n", config->ambientVolume);
    fprintf(fp, "  \"ui_scale\": %d,\n", config->uiScale);
    fprintf(fp, "  \"quick_resume_enabled\": %d,\n", config->quickResumeEnabled ? 1 : 0);
    fprintf(fp, "  \"streamer_mode\": %d,\n", config->streamerMode ? 1 : 0);
    fprintf(fp, "  \"cloud_sync_enabled\": %d,\n", config->cloudSyncEnabled ? 1 : 0);
    fprintf(fp, "  \"cloud_sync_policy\": %d,\n", config->cloudSyncPolicy);
    fprintf(fp, "  \"cloud_sync_dir\": "); m12_json_write_string(fp, config->cloudSyncDir); fprintf(fp, ","); fputc('\n', fp);
    fprintf(fp, "  \"custom_music_path\": ");
    m12_json_write_string(fp, config->customMusicPath);
    fprintf(fp, ",\n  \"custom_dungeon_path\": ");
    m12_json_write_string(fp, config->customDungeonPath);
    fprintf(fp, ",\n  \"screenshot_path\": ");
    m12_json_write_string(fp, config->screenshotPath);
    fprintf(fp, ",\n  \"data_dir\": ");
    m12_json_write_string(fp, config->dataDir);
    fprintf(fp, ",\n  \"last_save_path\": ");
    m12_json_write_string(fp, config->lastSavePath);
    fprintf(fp, "\n,\n");

    /* ── Per-game settings ── */
    fprintf(fp, "  \"game_options\": [\n");
    for (gi = 0; gi < M12_CONFIG_GAME_COUNT; ++gi) {
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"use_patch\": %d,\n", config->gameUsePatch[gi]);
        fprintf(fp, "      \"version_index\": %d,\n", config->gameVersionIndex[gi]);
        fprintf(fp, "      \"language_index\": %d,\n", config->gameLanguageIndex[gi]);
        fprintf(fp, "      \"cheats_enabled\": %d,\n", config->gameCheatsEnabled[gi]);
        fprintf(fp, "      \"speed\": %d,\n", config->gameSpeed[gi]);
        fprintf(fp, "      \"aspect_ratio\": %d,\n", config->gameAspectRatio[gi]);
        fprintf(fp, "      \"resolution\": %d\n", config->gameResolution[gi]);
        fprintf(fp, "    }%s\n", (gi < M12_CONFIG_GAME_COUNT - 1) ? "," : "");
    }
    fprintf(fp, "  ]\n");

    fprintf(fp, "}\n");

    if (fclose(fp) != 0) {
        remove(tmpPathBuf);
        return 0;
    }
    if (rename(tmpPathBuf, path) != 0) {
        remove(tmpPathBuf);
        return 0;
    }
    return 1;
}

/* ── JSON import helpers ─────────────────────────────────────────────── */

static int m12_json_next_token(FILE* fp, char* tokenBuf, size_t tokenBufSize) {
    int c;
    size_t pos = 0;
    /* Skip whitespace */
    do {
        c = fgetc(fp);
        if (c == EOF) return 0;
    } while (c == ' ' || c == '\t' || c == '\n' || c == '\r');

    if (c == '{' || c == '}' || c == '[' || c == ']' || c == ':' || c == ',') {
        tokenBuf[0] = (char)c;
        tokenBuf[1] = '\0';
        return 1;
    }

    if (c == '"') {
        /* Read a string token */
        tokenBuf[0] = (char)c;
        pos = 1;
        while (pos < tokenBufSize - 1) {
            c = fgetc(fp);
            if (c == EOF) break;
            if (c == '\\' && pos < tokenBufSize - 2) {
                tokenBuf[pos++] = (char)c;
                c = fgetc(fp);
                if (c == EOF) break;
                tokenBuf[pos++] = (char)c;
            } else {
                tokenBuf[pos++] = (char)c;
                if (c == '"') break;
            }
        }
        tokenBuf[pos] = '\0';
        return 1;
    }

    /* Number or true/false/null */
    tokenBuf[0] = (char)c;
    pos = 1;
    while (pos < tokenBufSize - 1) {
        c = fgetc(fp);
        if (c == EOF || c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
            c == ',' || c == '}' || c == ']') {
            if (c != EOF) ungetc(c, fp);
            break;
        }
        tokenBuf[pos++] = (char)c;
    }
    tokenBuf[pos] = '\0';
    return 1;
}

static void m12_json_read_string_content(const char* quoted, char* out, size_t outSize) {
    size_t src = 0, dst = 0;
    if (!quoted || quoted[0] != '"') {
        m12_copy_string(out, outSize, "");
        return;
    }
    src = 1;
    while (src < strlen(quoted) && dst < outSize - 1) {
        if (quoted[src] == '\\' && src + 1 < strlen(quoted)) {
            ++src;
            switch (quoted[src]) {
                case 'n': out[dst++] = '\n'; break;
                case 'r': out[dst++] = '\r'; break;
                case 't': out[dst++] = '\t'; break;
                case '\\': out[dst++] = '\\'; break;
                case '"': out[dst++] = '"'; break;
                default: out[dst++] = quoted[src]; break;
            }
        } else if (quoted[src] == '"') {
            break;
        } else {
            out[dst++] = quoted[src];
        }
        ++src;
    }
    out[dst] = '\0';
}

/* ── M12_Config_ImportJSON ─────────────────────────────────────────────
 * Deserializes settings from a JSON file at importPath into config.
 * Returns 1 on success, 0 on failure (file not found, parse error).
 * ───────────────────────────────────────────────────────────────────── */
int M12_Config_ImportJSON(M12_Config* config, const char* importPath) {
    char pathBuf[FSP_PATH_MAX];
    const char* path;
    FILE* fp;
    char token[256];
    char key[128];
    int gi;

    if (!config) {
        return 0;
    }

    if (!importPath || importPath[0] == '\0') {
        m12_default_export_path(pathBuf, sizeof(pathBuf));
        path = pathBuf;
    } else {
        path = importPath;
    }

    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }

    /* Read opening brace */
    if (!m12_json_next_token(fp, token, sizeof(token)) || strcmp(token, "{") != 0) {
        fclose(fp);
        return 0;
    }

    /* Parse JSON object — read key-value pairs */
    while (m12_json_next_token(fp, token, sizeof(token))) {
        if (strcmp(token, "}") == 0) {
            break;  /* End of object */
        }
        if (strcmp(token, ",") == 0) {
            continue;  /* Skip comma, read next key */
        }
        if (token[0] != '"') {
            /* Unexpected token */
            fclose(fp);
            return 0;
        }

        /* Extract key name from quoted string */
        {
            size_t i = 0;
            size_t src = 1;
            while (src < strlen(token) - 1 && i < sizeof(key) - 1) {
                if (token[src] == '\\' && src + 1 < strlen(token) - 1) {
                    ++src;
                }
                key[i++] = token[src++];
            }
            key[i] = '\0';
        }

        /* Expect colon */
        if (!m12_json_next_token(fp, token, sizeof(token)) || strcmp(token, ":") != 0) {
            fclose(fp);
            return 0;
        }

        /* Read value token */
        if (!m12_json_next_token(fp, token, sizeof(token))) {
            fclose(fp);
            return 0;
        }

        /* ── Match key and assign value ── */
        #define SET_INT(K, field) \
            if (strcmp(key, K) == 0) { \
                config->field = m12_parse_int(token, config->field); \
                continue; \
            }
        #define SET_BOOL(K, field) \
            if (strcmp(key, K) == 0) { \
                if (strcmp(token, "true") == 0) config->field = 1; \
                else if (strcmp(token, "false") == 0) config->field = 0; \
                else config->field = m12_parse_int(token, config->field) ? 1 : 0; \
                continue; \
            }
        #define SET_STRING(K, field, size) \
            if (strcmp(key, K) == 0) { \
                char tmp_[size]; \
                m12_json_read_string_content(token, tmp_, sizeof(tmp_)); \
                m12_copy_string(config->field, sizeof(config->field), tmp_); \
                continue; \
            }

        SET_INT("language_index", languageIndex)
        SET_INT("language_explicit", languageExplicit)
        SET_INT("graphics_index", graphicsIndex)
        SET_INT("renderer_backend_index", rendererBackendIndex)
        SET_INT("window_mode_index", windowModeIndex)
        SET_INT("scale_mode_index", scaleModeIndex)
        SET_INT("display_aspect_mode", displayAspectMode)
        SET_INT("integer_scaling", integerScaling)
        SET_INT("scaling_filter_index", scalingFilterIndex)
        SET_INT("vsync_index", vsyncIndex)
        SET_INT("wasd_movement_enabled", wasdMovementEnabled)
        SET_INT("input_mode_index", inputModeIndex)
        SET_INT("touch_controls_index", touchControlsIndex)
        SET_INT("movement_mode_index", movementModeIndex)
        SET_INT("viewport_style_index", viewportStyleIndex)
        SET_INT("debug_overlay_index", debugOverlayIndex)
        SET_INT("developer_gates_index", developerGatesIndex)
        SET_INT("window_width", windowWidth)
        SET_INT("window_height", windowHeight)
        SET_INT("audio_master_volume", audioMasterVolume)
        SET_INT("audio_music_volume", audioMusicVolume)
        SET_INT("audio_sfx_volume", audioSfxVolume)
        SET_BOOL("audio_muted", audioMuted)
        SET_INT("font_scale", fontScale)
        SET_BOOL("high_contrast", highContrast)
        SET_INT("colorblind_mode", colorblindMode)
        SET_BOOL("auto_pause", autoPause)
        SET_INT("control_scheme", controlSchemeIndex)
        SET_INT("theme_index", themeIndex)
        SET_INT("bg_animation_preset", bgAnimationPreset)
        SET_INT("dm1_v2_scale_percent", dm1V2ScalePercent)
        SET_BOOL("dm1_v2_smoothing_enabled", dm1V2SmoothingEnabled)
        SET_BOOL("dm1_v2_dynamic_lighting_enabled", dm1V2DynamicLightingEnabled)
        SET_BOOL("dm1_v2_accessibility_touch_enabled", dm1V2AccessibilityTouchEnabled)
        SET_BOOL("dm1_v2_aspect_mode", dm1V2AspectMode)
        SET_BOOL("dm1_v2_smooth_turn_pan_enabled", dm1V2SmoothTurnPanEnabled)
        SET_BOOL("dm1_v2_crt_scanlines_enabled", dm1V2CrtScanlinesEnabled)
        SET_INT("dm1_v2_crt_scanline_strength", dm1V2CrtScanlineStrength)
        SET_BOOL("dm1_v2_palette_correction_enabled", dm1V2PaletteCorrectionEnabled)
        SET_INT("dm1_v2_palette_gamma", dm1V2PaletteGamma)
        SET_INT("dm1_v2_palette_brightness", dm1V2PaletteBrightness)
        SET_INT("dm1_v2_palette_contrast", dm1V2PaletteContrast)
        SET_BOOL("dm1_v2_dither_cleanup_enabled", dm1V2DitherCleanupEnabled)
        SET_BOOL("dm1_v2_sharpening_enabled", dm1V2SharpeningEnabled)
        SET_INT("dm1_v2_sharpening_strength", dm1V2SharpeningStrength)
        SET_BOOL("dm1_v2_phosphor_persistence_enabled", dm1V2PhosphorPersistenceEnabled)
        SET_INT("dm1_v2_phosphor_decay", dm1V2PhosphorDecay)
        SET_INT("dm1_v2_color_preset", dm1V2ColorPreset)
        SET_BOOL("dm1_v2_pixel_grid_enabled", dm1V2PixelGridEnabled)
        SET_INT("dm1_v2_pixel_grid_intensity", dm1V2PixelGridIntensity)
        SET_BOOL("dm1_v2_motion_blur_enabled", dm1V2MotionBlurEnabled)
        SET_INT("dm1_v2_motion_blur_strength", dm1V2MotionBlurStrength)
        SET_INT("game_speed_multiplier", gameSpeedMultiplier)
        SET_BOOL("minimap_enabled", minimapEnabled)
        SET_INT("minimap_size", minimapSize)
        SET_INT("minimap_corner", minimapCorner)
        SET_BOOL("auto_map_enabled", autoMapEnabled)
        SET_BOOL("combat_log_enabled", combatLogEnabled)
        SET_INT("combat_log_max_lines", combatLogMaxLines)
        SET_INT("soundtrack_mode", soundtrackMode)
        SET_BOOL("ambient_enabled", ambientEnabled)
        SET_INT("ambient_volume", ambientVolume)
        SET_INT("ui_scale", uiScale)
        SET_BOOL("quick_resume_enabled", quickResumeEnabled)
        SET_BOOL("streamer_mode", streamerMode)
        SET_BOOL("cloud_sync_enabled", cloudSyncEnabled)
        SET_INT("cloud_sync_policy", cloudSyncPolicy)
        SET_STRING("custom_music_path", customMusicPath, M12_CONFIG_DATA_DIR_CAPACITY)
        SET_STRING("custom_dungeon_path", customDungeonPath, M12_CONFIG_DATA_DIR_CAPACITY)
        SET_STRING("screenshot_path", screenshotPath, M12_CONFIG_DATA_DIR_CAPACITY)
        SET_STRING("data_dir", dataDir, M12_CONFIG_DATA_DIR_CAPACITY)
        SET_STRING("last_save_path", lastSavePath, M12_CONFIG_LAST_SAVE_PATH_CAPACITY)

        /* Handle game_options array */
        if (strcmp(key, "game_options") == 0) {
            if (strcmp(token, "[") != 0) {
                /* Skip to matching ] */
                int depth = 1;
                while (depth > 0 && m12_json_next_token(fp, token, sizeof(token))) {
                    if (strcmp(token, "[") == 0) depth++;
                    else if (strcmp(token, "]") == 0) depth--;
                }
                continue;
            }
            for (gi = 0; gi < M12_CONFIG_GAME_COUNT; ++gi) {
                char t2[256];
                /* Expect { */
                if (!m12_json_next_token(fp, t2, sizeof(t2)) || strcmp(t2, "{") != 0) break;

                /* Parse game object */
                while (m12_json_next_token(fp, t2, sizeof(t2))) {
                    if (strcmp(t2, "}") == 0) break;
                    if (strcmp(t2, ",") == 0) continue;
                    if (t2[0] != '"') {
                        /* Skip */
                        if (!m12_json_next_token(fp, token, sizeof(token))) break;
                        if (!m12_json_next_token(fp, token, sizeof(token))) break; /* colon + value */
                        continue;
                    }
                    /* Extract field name */
                    {
                        size_t ki = 0, si = 1;
                        while (si < strlen(t2) - 1 && ki < sizeof(key) - 1) {
                            if (t2[si] == '\\' && si + 1 < strlen(t2) - 1) ++si;
                            key[ki++] = t2[si++];
                        }
                        key[ki] = '\0';
                    }
                    /* Expect : */
                    if (!m12_json_next_token(fp, t2, sizeof(t2)) || strcmp(t2, ":") != 0) break;
                    /* Read value */
                    if (!m12_json_next_token(fp, t2, sizeof(t2))) break;

                    if (gi < M12_CONFIG_GAME_COUNT) {
                        SET_INT("use_patch", gameUsePatch[gi])
                        SET_INT("version_index", gameVersionIndex[gi])
                        SET_INT("language_index", gameLanguageIndex[gi])
                        SET_INT("cheats_enabled", gameCheatsEnabled[gi])
                        SET_INT("speed", gameSpeed[gi])
                        SET_INT("aspect_ratio", gameAspectRatio[gi])
                        SET_INT("resolution", gameResolution[gi])
                    }
                }
                /* Check for end of array */
                {
                    char t3[256];
                    int peek;
                    peek = fgetc(fp);
                    if (peek != EOF) ungetc(peek, fp);
                    if (peek == ',') {
                        m12_json_next_token(fp, t3, sizeof(t3)); /* consume , */
                        (void)t3;
                    }
                }
            }
            /* Skip to matching ] */
            {
                int depth = 1;
                while (depth > 0 && m12_json_next_token(fp, token, sizeof(token))) {
                    if (strcmp(token, "[") == 0) depth++;
                    else if (strcmp(token, "]") == 0) depth--;
                }
            }
            continue;
        }

        #undef SET_INT
        #undef SET_BOOL
        #undef SET_STRING

        (void)0; /* placeholder to absorb the defines */
    }

    fclose(fp);
    return 1;
}

/* Get the default export/import path */
const char* M12_Config_GetExportPath(void) {
    static char pathBuf[FSP_PATH_MAX];
    m12_default_export_path(pathBuf, sizeof(pathBuf));
    return pathBuf;
}
