#ifndef FIRESTAFF_CONFIG_M12_H
#define FIRESTAFF_CONFIG_M12_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_CONFIG_PATH_CAPACITY = 512,
    M12_CONFIG_DATA_DIR_CAPACITY = 512,
    M12_CONFIG_LAST_SAVE_PATH_CAPACITY = 256,
    M12_CONFIG_GAME_COUNT = 5  /* DM1, CSB, DM2, Nexus, Theron */
};

typedef struct {
    int languageIndex;
    int languageExplicit;
    int graphicsIndex;
    int rendererBackendIndex;
    int windowModeIndex;
    int scaleModeIndex;
    int displayAspectMode;   /* 0 = 4:3, 1 = 16:9 */
    int integerScaling;
    int scalingFilterIndex;
    int vsyncIndex;
    int wasdMovementEnabled;
    int inputModeIndex;       /* 0 = auto, 1 = keyboard/mouse, 2 = touch, 3 = gamepad */
    int touchControlsIndex;   /* 0 = off, 1 = minimal, 2 = full, 3 = large */
    int movementModeIndex;    /* 0 = original, 1 = fast, 2 = smooth */
    int viewportStyleIndex;   /* 0 = original, 1 = expanded, 2 = widescreen frame */
    int debugOverlayIndex;    /* 0 = off, 1 = coordinates, 2 = queue, 3 = draw order */
    int developerGatesIndex;  /* 0 = off, 1 = quick, 2 = full */
    int windowWidth;
    int windowHeight;
    int audioMasterVolume;
    int audioMusicVolume;
    int audioSfxVolume;
    int audioMuted;
    char dataDir[M12_CONFIG_DATA_DIR_CAPACITY];
    char path[M12_CONFIG_PATH_CAPACITY];

    /* Per-game options (indexed by game slot 0..M12_CONFIG_GAME_COUNT-1) */
    int gameUsePatch[M12_CONFIG_GAME_COUNT];
    int gameVersionIndex[M12_CONFIG_GAME_COUNT];
    int gameLanguageIndex[M12_CONFIG_GAME_COUNT];
    int gameCheatsEnabled[M12_CONFIG_GAME_COUNT];
    int gameSpeed[M12_CONFIG_GAME_COUNT];
    int gameAspectRatio[M12_CONFIG_GAME_COUNT];
    int gameResolution[M12_CONFIG_GAME_COUNT];

    /* Quick resume: path to the last quicksave file written */
    char lastSavePath[M12_CONFIG_LAST_SAVE_PATH_CAPACITY];

    /* Accessibility options */
    int fontScale;          /* 1-3, default 1 */
    int highContrast;       /* 0 = off, 1 = on */
    int colorblindMode;     /* 0 = off, 1 = deuteranopia, 2 = protanopia, 3 = tritanopia */
    int autoPause;          /* 0 = off, 1 = pause on focus loss */
    int controlSchemeIndex;  /* 0 = original (A/D turn, Q/E strafe), 1 = hybrid (A/D strafe, Q/E turn) */

    /* Theme selector */
    int themeIndex;         /* 0 = classic, see M12_Theme enum in theme_m12.h */

    /* Animated background preset */
    int bgAnimationPreset;  /* 0 = static, see M12_BgPreset in animated_bg_m12.h */

    /* DM1 V2 presentation settings (V2-only; V1 launch path ignores these) */
    int dm1V2ScalePercent;                 /* 100-400, default 100 */
    int dm1V2SmoothingEnabled;             /* 0 = nearest, 1 = smooth */
    int dm1V2DynamicLightingEnabled;       /* 0 = off, 1 = V2 lighting overlays */
    int dm1V2AccessibilityTouchEnabled;    /* 0 = off, 1 = larger/touch-friendly V2 controls */
    int dm1V2AspectMode;                   /* 0 = 4:3 original, 1 = 16:9 widescreen envelope */

    /* DM1 V2.0 filter chain (V2-only; V1 launch path ignores these) */
    int dm1V2CrtScanlinesEnabled;        /* 0 = off, 1 = on (darken even rows) */
    int dm1V2CrtScanlineStrength;        /* 0-100, percent darken; default 35 */
    int dm1V2PaletteCorrectionEnabled;   /* 0 = off, 1 = on */
    int dm1V2PaletteGamma;               /* 80-260 (= 0.80..2.60 x100); default 220 */
    int dm1V2PaletteBrightness;          /* -50..+50 percent; default 0 */
    int dm1V2PaletteContrast;            /* -50..+50 percent; default 0 */
    int dm1V2DitherCleanupEnabled;       /* 0 = off, 1 = on (3x3 mode filter on indexed) */
    int dm1V2SharpeningEnabled;          /* 0 = off, 1 = on */
    int dm1V2SharpeningStrength;         /* 0-100, percent; default 30 */

    /* DM1 V2.0 visual extras: phosphor persistence (CRT afterglow). */
    int dm1V2PhosphorPersistenceEnabled; /* 0 = off, 1 = on */
    int dm1V2PhosphorDecay;              /* 0-100, decay percent; default 60 */

    /* DM1 V2.0 color grading preset (0 = Original/identity). */
    int dm1V2ColorPreset;                /* 0..M11_COLOR_PRESET_COUNT-1 */

    /* DM1 V2.0 pixel grid overlay (post-upscale). */
    int dm1V2PixelGridEnabled;           /* 0 = off, 1 = on */
    int dm1V2PixelGridIntensity;         /* 0-100, percent darken; default 20 */

    /* DM1 V2.0 motion blur during active movement. */
    int dm1V2MotionBlurEnabled;          /* 0 = off, 1 = on */
    int dm1V2MotionBlurStrength;         /* 0-100, percent; default 30 */

    /* Gameplay & QoL */
    int gameSpeedMultiplier;     /* 50/100/150/200; default 100 */
    int minimapEnabled;          /* 0=off, 1=on; default 0 */
    int minimapSize;             /* 64-256; default 128 */
    int minimapCorner;           /* 0=TR,1=TL,2=BR,3=BL; default 0 */
    int autoMapEnabled;          /* 0=off, 1=on; default 1 */
    int combatLogEnabled;        /* 0=off, 1=on; default 0 */
    int combatLogMaxLines;       /* 50-500; default 200 */

    /* Soundtrack selector (audio extra). */
    int soundtrackMode;                  /* 0 = Original, 1 = Remastered, 2 = Custom */
    char customMusicPath[M12_CONFIG_DATA_DIR_CAPACITY];

    /* Ambient sound layer (audio extra). */
    int ambientEnabled;                  /* 0 = off, 1 = on */
    int ambientVolume;                   /* 0-100, default 40 */

    /* UI scale (accessibility extra). Multiplier in percent for
     * HUD and menu text rendering. */
    int uiScale;                         /* 100, 150, 200 (default 100) */
} M12_Config;

void M12_Config_SetDefaults(M12_Config* config);
int M12_Config_Load(M12_Config* config, const char* dataDirOverride);
int M12_Config_Save(const M12_Config* config);
const char* M12_Config_GetPath(const M12_Config* config);
int M12_Config_GetAutoLanguageIndex(void);
void M12_Config_SetLastSavePath(const char* path);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CONFIG_M12_H */

/* Bug-patch settings count */
#define M12_BUG_PATCH_COUNT 9
