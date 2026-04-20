#ifndef FIRESTAFF_MENU_STARTUP_M12_H
#define FIRESTAFF_MENU_STARTUP_M12_H

#include "asset_status_m12.h"
#include "card_art_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M12_MENU_INPUT_NONE = 0,
    M12_MENU_INPUT_UP,
    M12_MENU_INPUT_DOWN,
    M12_MENU_INPUT_LEFT,
    M12_MENU_INPUT_RIGHT,
    M12_MENU_INPUT_ACCEPT,
    M12_MENU_INPUT_BACK
} M12_MenuInput;

typedef enum {
    M12_MENU_VIEW_MAIN = 0,
    M12_MENU_VIEW_SETTINGS,
    M12_MENU_VIEW_MESSAGE
} M12_MenuView;

typedef enum {
    M12_MENU_ENTRY_GAME = 0,
    M12_MENU_ENTRY_SETTINGS
} M12_MenuEntryKind;

typedef enum {
    M12_MENU_SOURCE_BUILTIN_CATALOG = 0,
    M12_MENU_SOURCE_CUSTOM_DUNGEON,
    M12_MENU_SOURCE_SYSTEM
} M12_MenuSourceKind;

typedef struct {
    const char* title;
    const char* gameId;
    M12_MenuEntryKind kind;
    M12_MenuSourceKind sourceKind;
    int available;
} M12_MenuEntry;

typedef struct {
    int languageIndex;
    int graphicsIndex;
    int windowModeIndex;
} M12_MenuSettingsState;

typedef struct M12_StartupMenuState {
    M12_MenuEntry entries[4];
    M12_GameCardArt cardArt[4];
    int selectedIndex;
    int settingsSelectedIndex;
    int shouldExit;
    int activatedIndex;
    M12_MenuView view;
    const char* messageLine1;
    const char* messageLine2;
    const char* messageLine3;
    M12_MenuSettingsState settings;
    M12_AssetStatus assetStatus;
} M12_StartupMenuState;

void M12_StartupMenu_Init(M12_StartupMenuState* state);
void M12_StartupMenu_InitWithDataDir(M12_StartupMenuState* state,
                                     const char* dataDir);
void M12_StartupMenu_HandleInput(M12_StartupMenuState* state,
                                 M12_MenuInput input);
void M12_StartupMenu_Draw(const M12_StartupMenuState* state,
                          unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight);

int M12_StartupMenu_GetEntryCount(void);
const M12_MenuEntry* M12_StartupMenu_GetEntry(const M12_StartupMenuState* state,
                                              int index);
int M12_StartupMenu_GetRenderPaletteLevel(const M12_StartupMenuState* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MENU_STARTUP_M12_H */
