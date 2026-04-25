#ifndef FIRESTAFF_CONFIG_M12_H
#define FIRESTAFF_CONFIG_M12_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_CONFIG_PATH_CAPACITY = 512,
    M12_CONFIG_DATA_DIR_CAPACITY = 512,
    M12_CONFIG_GAME_COUNT = 3  /* DM1, CSB, DM2 */
};

typedef struct {
    int languageIndex;
    int languageExplicit;
    int graphicsIndex;
    int windowModeIndex;
    int scaleModeIndex;
    int integerScaling;
    int scalingFilterIndex;
    int vsyncIndex;
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
