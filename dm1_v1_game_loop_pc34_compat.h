#ifndef FIRESTAFF_DM1_V1_GAME_LOOP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GAME_LOOP_PC34_COMPAT_H

/* DM1 V1 Main Game Loop + Frame Timing — source-locked from ReDMCSB
 * VBLANK.C: main game loop with 50fps frame budget (~20ms per frame on PAL NTSC)
 * F0348: G2585_G2586_VBLANK_TimerMain — core loop with G2586_TimerActive
 * F0475: F0476_G0215_VBLANK_FrameTimer — frame budget enforcement
 * DOS/CLOCK.C: DOS interrupt-based timing for consistent 50fps
 *
 * Frame budget breakdown (50fps = 20ms per frame):
 * - VBLANK timer interrupt: 1ms
 * - Input polling: 2ms
 * - Command processing: 3ms
 * - Movement update: 3ms
 * - Viewport render: 6ms (largest — blit/fill/creatures)
 * - Dialog/message update: 1ms
 * - Save/load state check: 1ms
 * - Remaining budget: 3ms
 *
 * Key: G2586_TimerActive controls whether the game loop runs or pauses.
 * Paused during: dialogue, loading, save screen, entrance/title sequences
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   VBLANK.C: F0577_VerticalBlank_Handler_CPSDF (VBlank ISR)
 *     G0317_i_WaitForInputVerticalBlankCount — increment each VBlank
 *     G0318_i_WaitForInputMaximumVerticalBlankCount — threshold (10 on PC34)
 *     G0321_B_StopWaitingForPlayerInput — set when count >= max
 *   VBLANK.C: F0575_VerticalBlank_Initialize — install VBlank handler
 *   VBLANK.C: F0576_VerticalBlank_Deinitialize — remove VBlank handler
 *   GAMELOOP.C: F0002_MAIN_GameLoop_CPSDF — infinite loop
 *     tick order: newMap → timeline → dungeonView → pointer → highlight →
 *                 sound → damage → deathCheck → inputWait
 *   GAMELOOP.C: G1086_VerticalBlankCount — global frame counter
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Game loop phase identifiers — matches F0002 execution order ──── */
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

/* ── Game loop states ─────────────────────────────────────────────── */
typedef enum {
    DM1_LOOP_INIT = 0,        /* Pre-start, waiting for first tick */
    DM1_LOOP_RUNNING,         /* Normal game loop active */
    DM1_LOOP_PAUSED,          /* G2586_TimerActive == false */
    DM1_LOOP_STOPPED          /* Exit requested */
} M11_GameLoopStatus;

/* ── Frame timing constants (source: VBLANK.C / DOS CLOCK.C) ────── */
#define M11_FRAME_RATE_HZ               50   /* PAL VBlank rate */
#define M11_FRAME_BUDGET_MS             20   /* 1000/50 = 20ms per frame */
#define M11_VBLANK_WAIT_MAX_DEFAULT     10   /* G0318: PC34 default */
#define M11_VBLANK_WAIT_MAX_EXTENDED    12   /* Amiga A3x extended wait */
#define M11_VBLANK_TIMER_BUDGET_MS       1   /* VBlank ISR overhead */
#define M11_INPUT_POLL_BUDGET_MS         2   /* Input phase budget */
#define M11_COMMAND_BUDGET_MS            3   /* Command processing budget */
#define M11_MOVEMENT_BUDGET_MS           3   /* Movement update budget */
#define M11_VIEWPORT_BUDGET_MS           6   /* Viewport render budget */
#define M11_DIALOG_BUDGET_MS             1   /* Dialog/message budget */
#define M11_SAVELOAD_BUDGET_MS           1   /* Save/load check budget */

/* ── Per-tick result from game loop orchestrator ──────────────────── */
typedef struct {
    M11_GameLoopPhase lastPhaseCompleted;
    int newMapProcessed;           /* F0003 was called */
    int newMapIndex;               /* map index if processed, else -1 */
    int timelineEventsProcessed;   /* timeline should be called */
    int dungeonViewDrawn;          /* F0128 should be called */
    int mousePointerUpdated;       /* pointer update flags handled */
    int commandHighlightDisabled;  /* F0363 should be called */
    int soundPlayed;               /* F0065 should be called */
    int damageApplied;             /* F0320 should be called */
    int partyDead;                 /* G0303 party dead flag */
    int inventoryOpen;             /* G0423 != 0 */
    int partyResting;              /* G0300 party resting */
    int stopWaitingForInput;       /* G0321 stop waiting */
    int vblankWaitCount;           /* G0317 current count */
    int exitRequested;             /* game exit requested */
} M11_GameLoopTickResult;

/* ── Frame timing statistics ──────────────────────────────────────── */
typedef struct {
    uint32_t totalFrames;          /* G1086_VerticalBlankCount equivalent */
    uint32_t droppedFrames;        /* frames that exceeded budget */
    uint32_t longestFrameUs;       /* worst-case frame time in microseconds */
    uint32_t avgFrameUs;           /* running average frame time */
    uint32_t budgetOverrunCount;   /* times any phase exceeded its budget */
} M11_FrameTimingStats;

/* ── Game loop persistent state ───────────────────────────────────── */
typedef struct {
    /* Loop status */
    M11_GameLoopStatus loopStatus;
    int timerActive;                    /* G2586_TimerActive */

    /* VBlank/timing — from VBLANK.C */
    int waitForInputMaxVBlankCount;     /* G0318: threshold (10 or 12) */
    int waitForInputVBlankCount;        /* G0317: reset each tick */
    uint32_t verticalBlankCount;        /* G1086: global frame counter */
    uint32_t targetFrameTimeUs;         /* configurable tick rate */

    /* Game state flags — from GAMELOOP.C */
    int newPartyMapIndex;               /* G0327: -1 if none pending */
    int partyDead;                      /* G0303 */
    int partyResting;                   /* G0300 */
    int inventoryChampionOrdinal;       /* G0423: 0 = no inventory */
    int setMousePointerToObject;        /* G0325 */
    int refreshMousePointer;            /* G0326 */
    int stopWaitingForInput;            /* G0321 */
    int exitGameImmediately;            /* G2151 (PC34 exit) */

    /* Tick tracking */
    uint32_t tickCount;
    uint32_t lastTickMs;

    /* Frame timing stats */
    M11_FrameTimingStats frameStats;
} M11_GameLoopState;

/* ── Initialization ───────────────────────────────────────────────── */

/* Initialize game loop state. extendedVBlankWait selects 12 vs 10. */
void m11_game_loop_init(M11_GameLoopState *state, int extendedVBlankWait);

/* Set configurable tick rate in Hz (default: 50). */
void m11_game_loop_set_tick_rate(M11_GameLoopState *state, int hz);

/* ── Core loop ────────────────────────────────────────────────────── */

/* Process one tick of the game loop (F0002 body).
 * Pure orchestration — sets flags; caller invokes subsystems. */
M11_GameLoopTickResult m11_game_loop_tick(M11_GameLoopState *state, uint32_t nowMs);

/* Simulate VBlank interrupt (F0577). Increments G0317 + G1086. */
void m11_game_loop_vblank_tick(M11_GameLoopState *state);

/* ── Pause/resume (G2586_TimerActive) ─────────────────────────────── */

/* Pause the game loop (dialogue, loading, save screen). */
void m11_game_loop_pause(M11_GameLoopState *state);

/* Resume the game loop. */
void m11_game_loop_resume(M11_GameLoopState *state);

/* Check if game loop is paused. */
int m11_game_loop_is_paused(const M11_GameLoopState *state);

/* ── State mutation ───────────────────────────────────────────────── */

void m11_game_loop_request_new_map(M11_GameLoopState *state, int newMapIndex);
void m11_game_loop_set_party_dead(M11_GameLoopState *state);
void m11_game_loop_set_inventory(M11_GameLoopState *state, int championOrdinal);
void m11_game_loop_set_resting(M11_GameLoopState *state, int resting);
void m11_game_loop_request_exit(M11_GameLoopState *state);
int  m11_game_loop_should_continue(const M11_GameLoopState *state);

/* ── Frame budget monitoring ──────────────────────────────────────── */

/* Record a phase's elapsed time for budget tracking. */
void m11_game_loop_record_phase_time(M11_GameLoopState *state,
                                     M11_GameLoopPhase phase,
                                     uint32_t elapsedUs);

/* Get current frame timing statistics. */
M11_FrameTimingStats m11_game_loop_get_frame_stats(const M11_GameLoopState *state);

/* Reset frame timing statistics. */
void m11_game_loop_reset_frame_stats(M11_GameLoopState *state);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *m11_game_loop_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GAME_LOOP_PC34_COMPAT_H */
