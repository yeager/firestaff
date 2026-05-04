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
} M11_GameStateId;

/* ── Transition result ────────────────────────────────────────────── */
typedef enum {
    DM1_TRANS_OK = 0,               /* Transition succeeded */
    DM1_TRANS_INVALID,              /* Invalid transition */
    DM1_TRANS_BLOCKED,              /* Transition blocked (e.g. save in progress) */
    DM1_TRANS_SAME_STATE            /* Already in target state */
} M11_TransitionResult;

/* ── State transition callback ────────────────────────────────────── */
typedef void (*M11_StateCallback)(M11_GameStateId prevState,
                                  M11_GameStateId newState,
                                  void *userdata);

/* ── Game state persistent data ───────────────────────────────────── */
typedef struct {
    /* Current state */
    M11_GameStateId currentState;
    M11_GameStateId previousState;

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
    M11_StateCallback onEnter;
    M11_StateCallback onExit;
    void *callbackUserdata;
} M11_GameStateMachine;

/* ── Initialization ───────────────────────────────────────────────── */

/* Initialize game state machine (equivalent to pre-STARTUP1 state). */
void m11_game_state_init(M11_GameStateMachine *sm);

/* Set transition callbacks (optional). */
void m11_game_state_set_callbacks(M11_GameStateMachine *sm,
                                  M11_StateCallback onEnter,
                                  M11_StateCallback onExit,
                                  void *userdata);

/* ── State transitions ────────────────────────────────────────────── */

/* Request transition to a new state. Validates the transition. */
M11_TransitionResult m11_game_state_transition(M11_GameStateMachine *sm,
                                                M11_GameStateId newState);

/* Get current game state. */
M11_GameStateId m11_game_state_current(const M11_GameStateMachine *sm);

/* Get previous game state. */
M11_GameStateId m11_game_state_previous(const M11_GameStateMachine *sm);

/* Check if a transition from current state to target is valid. */
int m11_game_state_can_transition(const M11_GameStateMachine *sm,
                                  M11_GameStateId target);

/* ── Convenience state setters (match STARTUP2.C patterns) ────────── */

/* Start a new game (F0462_START_StartGame_CPSEF path). */
M11_TransitionResult m11_game_state_start_new_game(M11_GameStateMachine *sm);

/* Load a saved game (F0435_STARTEND_LoadGame path). */
M11_TransitionResult m11_game_state_load_game(M11_GameStateMachine *sm);

/* Enter dungeon gameplay (after entrance). */
M11_TransitionResult m11_game_state_enter_dungeon(M11_GameStateMachine *sm);

/* Open inventory for champion. */
M11_TransitionResult m11_game_state_open_inventory(M11_GameStateMachine *sm,
                                                    int championOrdinal);

/* Close inventory, return to dungeon. */
M11_TransitionResult m11_game_state_close_inventory(M11_GameStateMachine *sm);

/* Party died — transition to game over. */
M11_TransitionResult m11_game_state_party_died(M11_GameStateMachine *sm);

/* Game won — transition to victory/credits. */
M11_TransitionResult m11_game_state_victory(M11_GameStateMachine *sm);

/* Request game restart. */
M11_TransitionResult m11_game_state_request_restart(M11_GameStateMachine *sm);

/* ── Query ────────────────────────────────────────────────────────── */

/* Return human-readable name for a state. */
const char *m11_game_state_name(M11_GameStateId state);

/* Source evidence string. */
const char *m11_game_state_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GAME_STATE_PC34_COMPAT_H */
