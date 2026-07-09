#ifndef FIRESTAFF_DM1_V1_GAME_STATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GAME_STATE_PC34_COMPAT_H

/* DM1 V1 Game State Machine — source-locked from ReDMCSB
 * STARTUP.C F0108: G0120_STARTUP_GameInit
 * STARTUP.C F0110: G0122_STARTUP_LoadGame
 * STARTUP.C F0117: G0129_STARTUP_ResetGame
 * MODE.C: G2183_MODE_CoordinatesChanged — coordinate mode switching
 *
 * Game states:
 * - TITLE_SCREEN: Title animation/swoosh, logo display
 * - NEWGAME_INITIALIZE: Initialize champions, inventory, dungeon state
 * - LOAD_GAME_INITIALIZE: Load from save file, restore state
 * - INVENTORY: Inventory management screen
 * - MAP_SCREEN: World map / area selection
 * - DUNGEON_VIEWPORT: Main gameplay loop (move, cast, interact)
 * - ENTRANCE_SCREEN: Dungeon entrance transition
 * - RESURRECT_SCREEN: Champion death/respawn dialog
 * - GAME_OVER: Death/game over screen
 *
 * State transitions follow STARTUP.C patterns:
 * TITLE → NEWGAME → ENTRANCE → DUNGEON_VIEWPORT
 * DUNGEON_VIEWPORT → RESURRECT → ENTRANCE (if champion dies)
 * DUNGEON_VIEWPORT → MAP_SCREEN → ENTRANCE (area change)
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   STARTUP1.C: F0448_STARTUP1_InitializeMemoryManager_CPSADEF
 *   STARTUP2.C: F0462_START_StartGame_CPSEF — game start setup
 *               F0463_START_InitializeGame_CPSADEF — full init sequence
 *               F0460_START_InitializeGraphicData — graphic memory setup
 *               F0456_START_DrawDisabledMenus — pause screen state
 *               F0457_START_DrawEnabledMenus_CPSF — resume from pause
 *   ENDGAME.C:  F0444_STARTEND_Endgame — game over / victory
 *   TITLE.C:    F0437_STARTEND_DrawTitle — title swoosh animation
 *   Key globals:
 *     G0298_B_NewGame — new game vs loaded game
 *     G0300_B_PartyIsResting — resting flag
 *     G0301_B_GameTimeTicking — game time active
 *     G0302_B_GameWon — victory flag
 *     G0303_i_PartyDeath — party dead
 *     G0523_B_RestartGameRequested — restart requested
 *     G0524_B_RestartGameAllowed — restart allowed
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Game state identifiers ───────────────────────────────────────── */
typedef enum {
    DM1_STATE_NONE = 0,
    DM1_STATE_TITLE_SCREEN,         /* Title swoosh + logo */
    DM1_STATE_NEWGAME_INIT,         /* New game initialization */
    DM1_STATE_LOADGAME_INIT,        /* Load game from save */
    DM1_STATE_ENTRANCE_SCREEN,      /* Entrance/champion selection */
    DM1_STATE_DUNGEON_VIEWPORT,     /* Main gameplay */
    DM1_STATE_INVENTORY,            /* Inventory screen */
    DM1_STATE_MAP_SCREEN,           /* Map/area selection */
    DM1_STATE_RESURRECT_SCREEN,     /* Resurrect/reincarnate dialog */
    DM1_STATE_GAME_OVER,            /* Party death screen */
    DM1_STATE_VICTORY,              /* Game won / credits */
    DM1_STATE_RESTART,              /* Restart requested */
    DM1_STATE_COUNT
} DM1_V1_GameStateIdPc34;

/* ── Transition result ────────────────────────────────────────────── */
typedef enum {
    DM1_TRANS_OK = 0,               /* Transition succeeded */
    DM1_TRANS_INVALID,              /* Invalid transition */
    DM1_TRANS_BLOCKED,              /* Transition blocked (e.g. save in progress) */
    DM1_TRANS_SAME_STATE            /* Already in target state */
} DM1_V1_TransitionResultPc34;

/* ── State transition callback ────────────────────────────────────── */
typedef void (*DM1_V1_StateCallbackPc34)(DM1_V1_GameStateIdPc34 prevState,
                                  DM1_V1_GameStateIdPc34 newState,
                                  void *userdata);

/* ── Game state persistent data ───────────────────────────────────── */
typedef struct {
    /* Current state */
    DM1_V1_GameStateIdPc34 currentState;
    DM1_V1_GameStateIdPc34 previousState;

    /* Key flags from STARTUP2.C / GAMELOOP.C */
    int newGame;                    /* G0298_B_NewGame */
    int gameTimeTicking;            /* G0301_B_GameTimeTicking */
    int gameWon;                    /* G0302_B_GameWon */
    int partyDead;                  /* G0303_i_PartyDeath */
    int partyResting;               /* G0300_B_PartyIsResting */
    int restartRequested;           /* G0523_B_RestartGameRequested */
    int restartAllowed;             /* G0524_B_RestartGameAllowed */
    int inventoryChampionOrdinal;   /* G0423_i_InventoryChampionOrdinal */
    int disabledMenusDrawn;         /* G2167_B_DisabledMenusDrawn */

    /* Transition counter */
    uint32_t transitionCount;

    /* Optional callbacks */
    DM1_V1_StateCallbackPc34 onEnter;
    DM1_V1_StateCallbackPc34 onExit;
    void *callbackUserdata;
} DM1_V1_GameStateMachinePc34;

/* ── Initialization ───────────────────────────────────────────────── */

/* Initialize game state machine (equivalent to pre-STARTUP1 state). */
void DM1_V1_GameState_InitPc34Compat(DM1_V1_GameStateMachinePc34 *sm);

/* Set transition callbacks (optional). */
void DM1_V1_GameState_SetCallbacksPc34Compat(DM1_V1_GameStateMachinePc34 *sm,
                                  DM1_V1_StateCallbackPc34 onEnter,
                                  DM1_V1_StateCallbackPc34 onExit,
                                  void *userdata);

/* ── State transitions ────────────────────────────────────────────── */

/* Request transition to a new state. Validates the transition. */
DM1_V1_TransitionResultPc34 DM1_V1_GameState_TransitionPc34Compat(DM1_V1_GameStateMachinePc34 *sm,
                                                DM1_V1_GameStateIdPc34 newState);

/* Get current game state. */
DM1_V1_GameStateIdPc34 DM1_V1_GameState_CurrentPc34Compat(const DM1_V1_GameStateMachinePc34 *sm);

/* Get previous game state. */
DM1_V1_GameStateIdPc34 DM1_V1_GameState_PreviousPc34Compat(const DM1_V1_GameStateMachinePc34 *sm);

/* Check if a transition from current state to target is valid. */
int DM1_V1_GameState_CanTransitionPc34Compat(const DM1_V1_GameStateMachinePc34 *sm,
                                  DM1_V1_GameStateIdPc34 target);

/* ── Convenience state setters (match STARTUP2.C patterns) ────────── */

/* Start a new game (F0462_START_StartGame_CPSEF path). */
DM1_V1_TransitionResultPc34 DM1_V1_GameState_StartNewGamePc34Compat(DM1_V1_GameStateMachinePc34 *sm);

/* Load a saved game (F0435_STARTEND_LoadGame path). */
DM1_V1_TransitionResultPc34 DM1_V1_GameState_LoadGamePc34Compat(DM1_V1_GameStateMachinePc34 *sm);

/* Enter dungeon gameplay (after entrance). */
DM1_V1_TransitionResultPc34 DM1_V1_GameState_EnterDungeonPc34Compat(DM1_V1_GameStateMachinePc34 *sm);

/* Open inventory for champion. */
DM1_V1_TransitionResultPc34 DM1_V1_GameState_OpenInventoryPc34Compat(DM1_V1_GameStateMachinePc34 *sm,
                                                    int championOrdinal);

/* Close inventory, return to dungeon. */
DM1_V1_TransitionResultPc34 DM1_V1_GameState_CloseInventoryPc34Compat(DM1_V1_GameStateMachinePc34 *sm);

/* Party died — transition to game over. */
DM1_V1_TransitionResultPc34 DM1_V1_GameState_PartyDiedPc34Compat(DM1_V1_GameStateMachinePc34 *sm);

/* Game won — transition to victory/credits. */
DM1_V1_TransitionResultPc34 DM1_V1_GameState_VictoryPc34Compat(DM1_V1_GameStateMachinePc34 *sm);

/* Request game restart. */
DM1_V1_TransitionResultPc34 DM1_V1_GameState_RequestRestartPc34Compat(DM1_V1_GameStateMachinePc34 *sm);

/* ── Query ────────────────────────────────────────────────────────── */

/* Return human-readable name for a state. */
const char *DM1_V1_GameState_NamePc34Compat(DM1_V1_GameStateIdPc34 state);

/* Source evidence string. */
const char *DM1_V1_GameState_SourceEvidencePc34Compat(void);

typedef DM1_V1_GameStateIdPc34 M11_GameStateId;
typedef DM1_V1_TransitionResultPc34 M11_TransitionResult;
typedef DM1_V1_StateCallbackPc34 M11_StateCallback;
typedef DM1_V1_GameStateMachinePc34 M11_GameStateMachine;

#define m11_game_state_init DM1_V1_GameState_InitPc34Compat
#define m11_game_state_set_callbacks DM1_V1_GameState_SetCallbacksPc34Compat
#define m11_game_state_transition DM1_V1_GameState_TransitionPc34Compat
#define m11_game_state_current DM1_V1_GameState_CurrentPc34Compat
#define m11_game_state_previous DM1_V1_GameState_PreviousPc34Compat
#define m11_game_state_can_transition DM1_V1_GameState_CanTransitionPc34Compat
#define m11_game_state_start_new_game DM1_V1_GameState_StartNewGamePc34Compat
#define m11_game_state_load_game DM1_V1_GameState_LoadGamePc34Compat
#define m11_game_state_enter_dungeon DM1_V1_GameState_EnterDungeonPc34Compat
#define m11_game_state_open_inventory DM1_V1_GameState_OpenInventoryPc34Compat
#define m11_game_state_close_inventory DM1_V1_GameState_CloseInventoryPc34Compat
#define m11_game_state_party_died DM1_V1_GameState_PartyDiedPc34Compat
#define m11_game_state_victory DM1_V1_GameState_VictoryPc34Compat
#define m11_game_state_request_restart DM1_V1_GameState_RequestRestartPc34Compat
#define m11_game_state_name DM1_V1_GameState_NamePc34Compat
#define m11_game_state_source_evidence DM1_V1_GameState_SourceEvidencePc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GAME_STATE_PC34_COMPAT_H */
