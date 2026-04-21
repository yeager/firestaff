#ifndef FIRESTAFF_M11_GAME_VIEW_H
#define FIRESTAFF_M11_GAME_VIEW_H

#include "menu_startup_m12.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M11_GAME_VIEW_PATH_CAPACITY = 512
};

typedef enum {
    M11_GAME_INPUT_IGNORED = 0,
    M11_GAME_INPUT_REDRAW = 1,
    M11_GAME_INPUT_RETURN_TO_MENU = 2
} M11_GameInputResult;

typedef enum {
    M11_GAME_SOURCE_BUILTIN_CATALOG = 0,
    M11_GAME_SOURCE_CUSTOM_DUNGEON,
    M11_GAME_SOURCE_DIRECT_DUNGEON
} M11_GameSourceKind;

typedef struct {
    const char* title;
    const char* gameId;
    const char* dataDir;
    const char* sourceId;
    const char* dungeonPath;
    M11_GameSourceKind sourceKind;
} M11_GameLaunchSpec;

enum {
    M11_MESSAGE_LOG_CAPACITY = 6,
    M11_MESSAGE_MAX_LENGTH = 80
};

typedef struct {
    char text[M11_MESSAGE_MAX_LENGTH];
    unsigned char color;
} M11_LogEntry;

typedef struct {
    M11_LogEntry entries[M11_MESSAGE_LOG_CAPACITY];
    int writeIndex;
    int count;
} M11_MessageLog;

typedef struct {
    int active;
    int startedFromLauncher;
    char title[64];
    char sourceId[32];
    M11_GameSourceKind sourceKind;
    char dungeonPath[M11_GAME_VIEW_PATH_CAPACITY];
    char lastAction[32];
    char lastOutcome[64];
    char inspectTitle[64];
    char inspectDetail[128];
    uint32_t lastWorldHash;
    struct TickResult_Compat lastTickResult;
    struct GameWorld_Compat world;
    M11_MessageLog messageLog;
    int resting;
    int partyDead;
    uint32_t exploredBits[32]; /* 32 * 32 = 1024 cells tracked per level */
} M11_GameViewState;

void M11_GameView_Init(M11_GameViewState* state);
void M11_GameView_Shutdown(M11_GameViewState* state);
int M11_GameView_Start(M11_GameViewState* state, const M11_GameLaunchSpec* spec);
int M11_GameView_OpenSelectedMenuEntry(M11_GameViewState* state,
                                       const M12_StartupMenuState* menuState);
int M11_GameView_StartDm1(M11_GameViewState* state, const char* dataDir);
int M11_GameView_GetQuickSavePath(const M11_GameViewState* state,
                                  char* out,
                                  size_t outSize);
int M11_GameView_QuickSave(M11_GameViewState* state);
int M11_GameView_QuickLoad(M11_GameViewState* state);
M11_GameInputResult M11_GameView_AdvanceIdleTick(M11_GameViewState* state);
M11_GameInputResult M11_GameView_HandleInput(M11_GameViewState* state,
                                             M12_MenuInput input);
M11_GameInputResult M11_GameView_HandlePointer(M11_GameViewState* state,
                                               int x,
                                               int y,
                                               int primaryButton);
void M11_GameView_Draw(const M11_GameViewState* state,
                       unsigned char* framebuffer,
                       int framebufferWidth,
                       int framebufferHeight);
int M11_GameView_PickupItem(M11_GameViewState* state);
int M11_GameView_DropItem(M11_GameViewState* state);
int M11_GameView_CountChampionItems(const M11_GameViewState* state, int championIndex);
void M11_MessageLog_Push(M11_MessageLog* log, const char* text, unsigned char color);
int M11_GameView_GetMessageLogCount(const M11_GameViewState* state);
const char* M11_GameView_GetMessageLogEntry(const M11_GameViewState* state, int reverseIndex);

/* Post-move environmental transition check (pits, teleporters).
 * Returns 1 if a transition occurred. */
int M11_GameView_CheckPostMoveTransitions(M11_GameViewState* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_GAME_VIEW_H */
