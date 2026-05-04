#ifndef FIRESTAFF_DM1_V1_GAME_LOOP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GAME_LOOP_PC34_COMPAT_H

/*
 * DM1 V1 Game Loop Orchestrator — source-locked to ReDMCSB GAMELOOP.C
 *
 * F0002_MAIN_GameLoop_CPSDF: the infinite loop that drives the game.
 * Each tick: process new map, timeline events, dungeon view draw,
 * command processing, sound, damage/wounds, death check, input wait.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   GAMELOOP.C: F0002 (main game loop), F0003 (process new party map)
 *   Tick sequence: newMap → timeline → dungeonViewDraw → commands →
 *                  sound → damage → deathCheck → inputWait
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Game loop phase identifiers — matches F0002 execution order */
typedef enum {
    DM1_PHASE_NEW_MAP = 0,
    DM1_PHASE_TIMELINE,
    DM1_PHASE_DUNGEON_VIEW_DRAW,
    DM1_PHASE_MOUSE_POINTER_UPDATE,
    DM1_PHASE_COMMAND_HIGHLIGHT,
    DM1_PHASE_SOUND,
    DM1_PHASE_DAMAGE_WOUNDS,
    DM1_PHASE_DEATH_CHECK,
    DM1_PHASE_INPUT_WAIT,
    DM1_PHASE_COUNT
} M11_GameLoopPhase;

/* Game state identifiers */
typedef enum {
    DM1_GAME_UNINIT = 0,
    DM1_GAME_TITLE,
    DM1_GAME_ENTRANCE,
    DM1_GAME_PLAYING,
    DM1_GAME_PAUSED,
    DM1_GAME_INVENTORY,
    DM1_GAME_MAP,
    DM1_GAME_VICTORY,
    DM1_GAME_DEATH,
    DM1_GAME_STATE_COUNT
} M11_GameState;

/* Vertical blank wait constants — from F0002 */
#define M11_WAIT_FOR_INPUT_MAX_VBLANK_COUNT_DEFAULT 10
#define M11_WAIT_FOR_INPUT_MAX_VBLANK_COUNT_EXTENDED 12

/* Per-tick result from game loop orchestrator */
typedef struct {
    M11_GameLoopPhase lastPhaseCompleted;
    int newMapProcessed;           /* F0003 was called */
    int newMapIndex;               /* map index if processed, else -1 */
    int timelineEventsProcessed;   /* count of timeline events fired */
    int dungeonViewDrawn;          /* F0128 equivalent called */
    int mousePointerUpdated;       /* G0325/G0326 flags handled */
    int commandHighlightDisabled;  /* F0363 called */
    int soundPlayed;               /* F0065 pending sound played */
    int damageApplied;             /* F0320 damage/wounds drawn */
    int partyDead;                 /* G0303 party dead flag */
    int inventoryOpen;             /* G0423 inventory champion ordinal != 0 */
    int partyResting;              /* G0300 party is resting */
    int stopWaitingForInput;       /* G0321 stop waiting flag */
    int vblankWaitCount;           /* G0317 current vblank wait count */
    int exitRequested;             /* game exit requested */
} M11_GameLoopTickResult;

/* Game loop persistent state */
typedef struct {
    M11_GameState gameState;
    int waitForInputMaxVBlankCount;     /* G0318: 10 or 12 depending on platform */
    int waitForInputVBlankCount;        /* G0317: reset to 0 each tick */
    int newPartyMapIndex;               /* G0327: -1 if none pending */
    int partyDead;                      /* G0303 */
    int partyResting;                   /* G0300 */
    int inventoryChampionOrdinal;       /* G0423: 0 = no inventory open */
    int setMousePointerToObject;        /* G0325 */
    int refreshMousePointer;            /* G0326 */
    int stopWaitingForInput;            /* G0321 */
    int exitGameImmediately;            /* G2151 (PC34 extension) */
    uint32_t tickCount;                 /* total ticks processed */
    uint32_t lastTickMs;                /* timestamp of last tick */
} M11_GameLoopState;

/*
 * Initialize game loop state. Call once at game start.
 * Sets waitForInputMaxVBlankCount based on platform variant.
 */
void m11_game_loop_init(M11_GameLoopState *state, int extendedVBlankWait);

/*
 * Process one tick of the game loop (F0002 body).
 * Returns per-tick result. Caller is responsible for calling subsystems
 * (timeline, dungeon view, sound, etc.) based on the result flags.
 *
 * This is a pure orchestration function — it sets flags indicating what
 * should happen, but does not call rendering or I/O directly.
 */
M11_GameLoopTickResult m11_game_loop_tick(M11_GameLoopState *state, uint32_t nowMs);

/*
 * Request a new party map transition (sets G0327).
 * Called when the party enters stairs, teleporter, or pit to a new level.
 */
void m11_game_loop_request_new_map(M11_GameLoopState *state, int newMapIndex);

/*
 * Signal that the party has died (sets G0303).
 */
void m11_game_loop_set_party_dead(M11_GameLoopState *state);

/*
 * Open/close inventory for a champion.
 * ordinal=0 closes inventory.
 */
void m11_game_loop_set_inventory(M11_GameLoopState *state, int championOrdinal);

/*
 * Set/clear party resting state.
 */
void m11_game_loop_set_resting(M11_GameLoopState *state, int resting);

/*
 * Request game exit.
 */
void m11_game_loop_request_exit(M11_GameLoopState *state);

/*
 * Check if game loop should continue (not dead, not exit requested).
 */
int m11_game_loop_should_continue(const M11_GameLoopState *state);

/*
 * Source evidence string for auditing.
 */
const char *m11_game_loop_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GAME_LOOP_PC34_COMPAT_H */
