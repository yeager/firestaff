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
    M12_CONFIG_GAME_COUNT = 4  /* DM1, CSB, DM2, Nexus */
};

typedef struct {
    int languageIndex;
    int languageExplicit;
    int graphicsIndex;
    int rendererBackendIndex;
    int windowModeIndex;
    int scaleModeIndex;
    int integerScaling;
    int scalingFilterIndex;
    int vsyncIndex;
    int wasdMovementEnabled;
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
} M12_Config;

void M12_Config_SetDefaults(M12_Config* config);
int M12_Config_Load(M12_Config* config, const char* dataDirOverride);
int M12_Config_Save(const M12_Config* config);
const char* M12_Config_GetPath(const M12_Config* config);
int M12_Config_GetAutoLanguageIndex(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CONFIG_M12_H */
