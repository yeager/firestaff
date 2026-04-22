#ifndef FIRESTAFF_MENU_STARTUP_M12_H
#define FIRESTAFF_MENU_STARTUP_M12_H

#include "asset_status_m12.h"
#include "card_art_m12.h"
#include "creature_art_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M12_MENU_INPUT_NONE = 0,
    M12_MENU_INPUT_UP,
    M12_MENU_INPUT_DOWN,
    M12_MENU_INPUT_LEFT,
    M12_MENU_INPUT_RIGHT,
    M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_STRAFE_RIGHT,
    M12_MENU_INPUT_ACCEPT,
    M12_MENU_INPUT_BACK,
    M12_MENU_INPUT_ACTION,
    M12_MENU_INPUT_CYCLE_CHAMPION,
    M12_MENU_INPUT_REST_TOGGLE,
    M12_MENU_INPUT_USE_STAIRS,
    M12_MENU_INPUT_PICKUP_ITEM,
    M12_MENU_INPUT_DROP_ITEM,
    M12_MENU_INPUT_SPELL_RUNE_1,
    M12_MENU_INPUT_SPELL_RUNE_2,
    M12_MENU_INPUT_SPELL_RUNE_3,
    M12_MENU_INPUT_SPELL_RUNE_4,
    M12_MENU_INPUT_SPELL_RUNE_5,
    M12_MENU_INPUT_SPELL_RUNE_6,
    M12_MENU_INPUT_SPELL_CAST,
    M12_MENU_INPUT_SPELL_CLEAR,
    M12_MENU_INPUT_USE_ITEM,
    M12_MENU_INPUT_MAP_TOGGLE,
    M12_MENU_INPUT_INVENTORY_TOGGLE
} M12_MenuInput;

typedef enum {
    M12_MENU_VIEW_MAIN = 0,
    M12_MENU_VIEW_SETTINGS,
    M12_MENU_VIEW_MESSAGE,
    M12_MENU_VIEW_GAME_OPTIONS
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

typedef enum {
    M12_PRESENTATION_V1_ORIGINAL = 0,
    M12_PRESENTATION_V2_ENHANCED_2D,
    M12_PRESENTATION_V3_MODERN_3D,
    M12_PRESENTATION_MODE_COUNT
} M12_PresentationMode;

typedef enum {
    M12_ASPECT_ORIGINAL = 0,
    M12_ASPECT_4_3,
    M12_ASPECT_16_9,
    M12_ASPECT_16_10,
    M12_ASPECT_COUNT
} M12_AspectRatio;

typedef enum {
    M12_RES_320x200 = 0,
    M12_RES_640x400,
    M12_RES_800x600,
    M12_RES_1024x768,
    M12_RES_1280x960,
    M12_RES_COUNT
} M12_Resolution;

typedef enum {
    M12_GAME_SPEED_SLOWER = -1,
    M12_GAME_SPEED_NORMAL = 0,
    M12_GAME_SPEED_FASTER = 1
} M12_GameSpeed;

typedef enum {
    M12_GAME_OPT_ROW_PATCH = 0,
    M12_GAME_OPT_ROW_LANGUAGE,
    M12_GAME_OPT_ROW_CHEATS,
    M12_GAME_OPT_ROW_SPEED,
    M12_GAME_OPT_ROW_ASPECT,
    M12_GAME_OPT_ROW_RESOLUTION,
    M12_GAME_OPT_ROW_COUNT
} M12_GameOptRow;

typedef struct {
    int usePatch;
    int languageIndex;
    int cheatsEnabled;
    int gameSpeed;
    int aspectRatio;
    int resolution;
} M12_GameOptions;

int M12_GameOptions_SpeedHotkeysEnabled(const M12_GameOptions* opts);
int M12_GameOptions_RowLockedByMode(int row, int presentationMode);

typedef struct {
    const char* gameId;
    int presentationMode;
    M12_GameOptions options;
    int valid;
} M12_LaunchIntent;

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
    M12_GameOptions gameOptions[3];
    int gameOptSelectedRow;
    M12_CreatureArtState creatureArt;
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
int M12_StartupMenu_GetPresentationMode(const M12_StartupMenuState* state);
const char* M12_StartupMenu_GetPresentationModeLabel(const M12_StartupMenuState* state);
M12_LaunchIntent M12_StartupMenu_GetLaunchIntent(const M12_StartupMenuState* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MENU_STARTUP_M12_H */
