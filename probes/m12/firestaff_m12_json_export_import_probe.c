/*
 * firestaff_m12_json_export_import_probe.c
 *
 * Headless probe: verify M12_Config_ExportJSON and M12_Config_ImportJSON
 * round-trip all known fields including per-game options and string paths.
 *
 * Run:
 *   cd build && cmake --build . --target firestaff_m12_json_export_import_probe
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_m12_json_export_import_probe
 *
 * Refs: include/config_m12.h, src/engine/config_m12.c
 */

#include "config_m12.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <process.h>
static int portable_mkdir(const char* path) { return mkdir(path) == 0; }
static char* portable_mkdtemp(char* templ) {
    char* marker = strstr(templ, "XXXXXX");
    int i;
    if (!marker) return NULL;
    for (i = 0; i < 1000; ++i) {
        snprintf(marker, 7, "%06ld", ((long)_getpid() + i) % 1000000L);
        if (mkdir(templ) == 0) return templ;
    }
    return NULL;
}
static int portable_setenv(const char* n, const char* v, int o) {
    (void)o; return _putenv_s(n, v);
}
#else
#include <unistd.h>
static int portable_mkdir(const char* path) { return mkdir(path, 0777) == 0; }
static char* portable_mkdtemp(char* templ) { return mkdtemp(templ); }
static int portable_setenv(const char* n, const char* v, int o) { return setenv(n, v, o); }
#endif

typedef struct { int total; int passed; } Tally;

static void rec(Tally* t, const char* id, int ok, const char* msg) {
    t->total++;
    if (ok) { t->passed++; printf("PASS %s %s\n", id, msg); }
    else     { printf("FAIL %s %s\n", id, msg); }
}

static int files_equal_str(const char* path, const char* needle) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;
    char buf[256];
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    buf[n] = '\0';
    return strstr(buf, needle != NULL ? needle : "") != NULL;
}

int main(void) {
    Tally t = {0, 0};
    char tpl[] = "/tmp/firestaff-m12-json-probe-XXXXXX";
    char* dir = portable_mkdtemp(tpl);
    char exportPath[512];
    char importPath[512];
    char expectedPath[512];

    if (!dir) { perror("mkdtemp"); return 2; }
    if (portable_setenv("HOME", dir, 1) != 0) { perror("setenv HOME"); return 2; }
    if (portable_setenv("XDG_CONFIG_HOME", dir, 1) != 0) { (void)0; }

    snprintf(exportPath, sizeof(exportPath), "%s/export.json", dir);
    snprintf(importPath, sizeof(importPath), "%s/import.json", dir);
    snprintf(expectedPath, sizeof(expectedPath), "%s/.config/firestaff-settings-export.json", dir);

    /* ── Write config ───────────────────────────────────────────── */
    M12_Config cfg_write;
    M12_Config_SetDefaults(&cfg_write);

    /* Non-default values to detect round-trip errors */
    cfg_write.languageIndex          = 2;
    cfg_write.languageExplicit       = 1;
    cfg_write.graphicsIndex           = 3;
    cfg_write.rendererBackendIndex   = 1;
    cfg_write.windowModeIndex        = 0;
    cfg_write.scaleModeIndex         = 1;
    cfg_write.displayAspectMode      = 1;
    cfg_write.integerScaling         = 1;
    cfg_write.scalingFilterIndex     = 2;
    cfg_write.vsyncIndex             = 0;
    cfg_write.wasdMovementEnabled    = 0;
    cfg_write.inputModeIndex         = 1;
    cfg_write.touchControlsIndex     = 2;
    cfg_write.movementModeIndex      = 1;
    cfg_write.viewportStyleIndex      = 1;
    cfg_write.debugOverlayIndex       = 2;
    cfg_write.developerGatesIndex     = 1;
    cfg_write.windowWidth            = 1280;
    cfg_write.windowHeight           = 720;
    cfg_write.audioMasterVolume      = 64;
    cfg_write.audioMusicVolume       = 96;
    cfg_write.audioSfxVolume         = 192;
    cfg_write.audioMuted             = 1;
    cfg_write.fontScale              = 2;
    cfg_write.highContrast           = 1;
    cfg_write.colorblindMode         = 2;
    cfg_write.autoPause              = 1;
    cfg_write.controlSchemeIndex     = 1;
    cfg_write.themeIndex             = 2;
    cfg_write.bgAnimationPreset      = 1;
    cfg_write.dm1V2ScalePercent      = 200;
    cfg_write.dm1V2SmoothingEnabled  = 0;
    cfg_write.dm1V2DynamicLightingEnabled = 0;
    cfg_write.dm1V2AccessibilityTouchEnabled = 1;
    cfg_write.dm1V2AspectMode        = 1;
    cfg_write.dm1V2SmoothTurnPanEnabled = 1;
    cfg_write.dm1V2CrtScanlinesEnabled = 1;
    cfg_write.dm1V2CrtScanlineStrength = 70;
    cfg_write.dm1V2PaletteCorrectionEnabled = 1;
    cfg_write.dm1V2PaletteGamma      = 180;
    cfg_write.dm1V2PaletteBrightness = 20;
    cfg_write.dm1V2PaletteContrast   = 25;
    cfg_write.dm1V2DitherCleanupEnabled = 1;
    cfg_write.dm1V2SharpeningEnabled = 1;
    cfg_write.dm1V2SharpeningStrength = 60;
    cfg_write.dm1V2PhosphorPersistenceEnabled = 1;
    cfg_write.dm1V2PhosphorDecay    = 80;
    cfg_write.dm1V2ColorPreset       = 3;
    cfg_write.dm1V2PixelGridEnabled  = 1;
    cfg_write.dm1V2PixelGridIntensity = 50;
    cfg_write.dm1V2MotionBlurEnabled = 1;
    cfg_write.dm1V2MotionBlurStrength = 55;
    cfg_write.gameSpeedMultiplier   = 150;
    cfg_write.minimapEnabled        = 1;
    cfg_write.minimapSize           = 200;
    cfg_write.minimapCorner         = 2;
    cfg_write.autoMapEnabled        = 0;
    cfg_write.combatLogEnabled      = 1;
    cfg_write.combatLogMaxLines     = 350;
    cfg_write.soundtrackMode        = 1;
    strcpy(cfg_write.customMusicPath, "/music/custom");
    cfg_write.ambientEnabled        = 1;
    cfg_write.ambientVolume         = 75;
    cfg_write.uiScale               = 150;
    cfg_write.quickResumeEnabled    = 0;
    strcpy(cfg_write.customDungeonPath, "/dungeons/community");
    strcpy(cfg_write.screenshotPath, "/screenshots");
    cfg_write.streamerMode          = 1;

    /* Per-game options */
    {
        int g;
        for (g = 0; g < M12_CONFIG_GAME_COUNT; ++g) {
            cfg_write.gameUsePatch[g]       = (g % 2);
            cfg_write.gameVersionIndex[g]   = g + 1;
            cfg_write.gameLanguageIndex[g]  = (g + 1) % M12_CONFIG_GAME_COUNT;
            cfg_write.gameCheatsEnabled[g]  = (g % 3 == 0) ? 1 : 0;
            cfg_write.gameSpeed[g]          = 50 + g * 25;
            cfg_write.gameAspectRatio[g]    = g % 3;
            cfg_write.gameResolution[g]    = g * 2;
        }
    }

    /* ── Export ─────────────────────────────────────────────────── */
    int ok = M12_Config_ExportJSON(&cfg_write, exportPath);
    rec(&t, "JSON_01", ok == 1, "M12_Config_ExportJSON returns 1 on success");
    rec(&t, "JSON_02",
        files_equal_str(exportPath, "\"version\": \"1.0\""),
        "exported JSON contains version field");
    rec(&t, "JSON_03",
        files_equal_str(exportPath, "\"type\": \"firestaff-launcher-settings\""),
        "exported JSON contains correct type field");
    rec(&t, "JSON_04",
        files_equal_str(exportPath, "\"graphics_index\": 3"),
        "exported JSON contains non-default graphics_index");
    rec(&t, "JSON_05",
        files_equal_str(exportPath, "\"dm1_v2_smooth_turn_pan_enabled\": 1"),
        "exported JSON contains V2 smooth-turn-pan flag");
    rec(&t, "JSON_06",
        files_equal_str(exportPath, "\"custom_music_path\""),
        "exported JSON contains custom_music_path key");
    rec(&t, "JSON_07",
        files_equal_str(exportPath, "\"game_options\""),
        "exported JSON contains game_options array");
    rec(&t, "JSON_08",
        files_equal_str(exportPath, "\"minimap_enabled\": 1"),
        "exported JSON contains non-default minimap_enabled");

    /* ── Import into fresh config ────────────────────────────────── */
    M12_Config cfg_read;
    M12_Config_SetDefaults(&cfg_read);  /* populate with known defaults first */

    ok = M12_Config_ImportJSON(&cfg_read, importPath);
    rec(&t, "JSON_09", ok == 1, "M12_Config_ImportJSON returns 1 on success");
    rec(&t, "JSON_10", cfg_read.languageIndex == 2, "languageIndex survives round-trip");
    rec(&t, "JSON_11", cfg_read.languageExplicit == 1, "languageExplicit survives round-trip");
    rec(&t, "JSON_12", cfg_read.graphicsIndex == 3, "graphicsIndex survives round-trip");
    rec(&t, "JSON_13", cfg_read.audioMuted == 1, "audioMuted survives round-trip");
    rec(&t, "JSON_14", cfg_read.fontScale == 2, "fontScale survives round-trip");
    rec(&t, "JSON_15", cfg_read.highContrast == 1, "highContrast survives round-trip");
    rec(&t, "JSON_16", cfg_read.colorblindMode == 2, "colorblindMode survives round-trip");
    rec(&t, "JSON_17", cfg_read.dm1V2SmoothTurnPanEnabled == 1, "dm1V2SmoothTurnPanEnabled survives");
    rec(&t, "JSON_18", cfg_read.dm1V2CrtScanlinesEnabled == 1, "dm1V2CrtScanlinesEnabled survives");
    rec(&t, "JSON_19", cfg_read.dm1V2CrtScanlineStrength == 70, "dm1V2CrtScanlineStrength survives");
    rec(&t, "JSON_20", cfg_read.dm1V2PaletteGamma == 180, "dm1V2PaletteGamma survives");
    rec(&t, "JSON_21", cfg_read.minimapEnabled == 1, "minimapEnabled survives");
    rec(&t, "JSON_22", cfg_read.minimapSize == 200, "minimapSize survives");
    rec(&t, "JSON_23", cfg_read.minimapCorner == 2, "minimapCorner survives");
    rec(&t, "JSON_24", cfg_read.autoMapEnabled == 0, "autoMapEnabled survives");
    rec(&t, "JSON_25", cfg_read.combatLogEnabled == 1, "combatLogEnabled survives");
    rec(&t, "JSON_26", cfg_read.combatLogMaxLines == 350, "combatLogMaxLines survives");
    rec(&t, "JSON_27", cfg_read.soundtrackMode == 1, "soundtrackMode survives");
    rec(&t, "JSON_28", strcmp(cfg_read.customMusicPath, "/music/custom") == 0,
        "customMusicPath string survives round-trip");
    rec(&t, "JSON_29", cfg_read.ambientEnabled == 1, "ambientEnabled survives");
    rec(&t, "JSON_30", cfg_read.ambientVolume == 75, "ambientVolume survives");
    rec(&t, "JSON_31", cfg_read.uiScale == 150, "uiScale survives");
    rec(&t, "JSON_32", cfg_read.quickResumeEnabled == 0, "quickResumeEnabled survives");
    rec(&t, "JSON_33", strcmp(cfg_read.customDungeonPath, "/dungeons/community") == 0,
        "customDungeonPath string survives round-trip");
    rec(&t, "JSON_34", strcmp(cfg_read.screenshotPath, "/screenshots") == 0,
        "screenshotPath string survives round-trip");
    rec(&t, "JSON_35", cfg_read.streamerMode == 1, "streamerMode survives");
    rec(&t, "JSON_36", cfg_read.dm1V2MotionBlurEnabled == 1, "dm1V2MotionBlurEnabled survives");
    rec(&t, "JSON_37", cfg_read.dm1V2MotionBlurStrength == 55, "dm1V2MotionBlurStrength survives");

    /* Per-game options */
    rec(&t, "JSON_38",
        cfg_read.gameUsePatch[0] == 0 && cfg_read.gameUsePatch[1] == 1,
        "gameUsePatch array survives for first two slots");
    rec(&t, "JSON_39",
        cfg_read.gameVersionIndex[3] == 4 && cfg_read.gameVersionIndex[4] == 5,
        "gameVersionIndex survives for DM4/DM5 slots");
    rec(&t, "JSON_40",
        cfg_read.gameSpeed[0] == 50 && cfg_read.gameSpeed[4] == 150,
        "gameSpeed array survives");
    rec(&t, "JSON_41",
        cfg_read.gameResolution[2] == 4 && cfg_read.gameResolution[3] == 6,
        "gameResolution array survives");
    rec(&t, "JSON_42",
        cfg_read.gameCheatsEnabled[0] == 1 && cfg_read.gameCheatsEnabled[3] == 1,
        "gameCheatsEnabled array survives");

    /* Default path */
    const char* defPath = M12_Config_GetExportPath();
    rec(&t, "JSON_43", defPath != NULL && strlen(defPath) > 0,
        "M12_Config_GetExportPath returns a non-empty string");
    rec(&t, "JSON_44",
        strstr(defPath, "firestaff-settings-export.json") != NULL,
        "default export path contains expected filename");

    /* Import non-existent file returns 0 */
    {
        M12_Config cfg_bad;
        M12_Config_SetDefaults(&cfg_bad);
        int before = cfg_bad.languageIndex;
        int r = M12_Config_ImportJSON(&cfg_bad, "/nonexistent/path/that/does/not/exist.json");
        rec(&t, "JSON_45", r == 0, "M12_Config_ImportJSON returns 0 for missing file");
        rec(&t, "JSON_46", cfg_bad.languageIndex == before,
            "failed import does not modify config");
    }

    printf("# summary: %d/%d invariants passed\n", t.passed, t.total);
    return (t.passed == t.total) ? 0 : 1;
}