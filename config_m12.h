#ifndef FIRESTAFF_CONFIG_M12_H
#define FIRESTAFF_CONFIG_M12_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M12_CONFIG_PATH_CAPACITY = 512,
    M12_CONFIG_DATA_DIR_CAPACITY = 512
};

typedef struct {
    int languageIndex;
    int graphicsIndex;
    int windowModeIndex;
    char dataDir[M12_CONFIG_DATA_DIR_CAPACITY];
    char path[M12_CONFIG_PATH_CAPACITY];
} M12_Config;

void M12_Config_SetDefaults(M12_Config* config);
int M12_Config_Load(M12_Config* config, const char* dataDirOverride);
int M12_Config_Save(const M12_Config* config);
const char* M12_Config_GetPath(const M12_Config* config);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CONFIG_M12_H */
