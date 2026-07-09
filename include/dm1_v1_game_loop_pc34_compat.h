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
} DM1_V1_GameLoopPhasePc34;

/* ── Game loop states ─────────────────────────────────────────────── */
typedef enum {
    DM1_LOOP_INIT = 0,        /* Pre-start, waiting for first tick */
    DM1_LOOP_RUNNING,         /* Normal game loop active */
    DM1_LOOP_PAUSED,          /* G2586_TimerActive == false */
    DM1_LOOP_STOPPED          /* Exit requested */
} DM1_V1_GameLoopStatusPc34;

/* ── Frame timing constants (source: VBLANK.C / DOS CLOCK.C) ────── */
#define DM1_V1_FRAME_RATE_HZ_PC34               50   /* PAL VBlank rate */
#define DM1_V1_FRAME_BUDGET_MS_PC34             20   /* 1000/50 = 20ms per frame */
#define DM1_V1_VBLANK_WAIT_MAX_DEFAULT_PC34     10   /* G0318: PC34 default */
#define DM1_V1_VBLANK_WAIT_MAX_EXTENDED_PC34    12   /* Amiga A3x extended wait */
#define DM1_V1_VBLANK_TIMER_BUDGET_MS_PC34       1   /* VBlank ISR overhead */
#define DM1_V1_INPUT_POLL_BUDGET_MS_PC34         2   /* Input phase budget */
#define DM1_V1_COMMAND_BUDGET_MS_PC34            3   /* Command processing budget */
#define DM1_V1_MOVEMENT_BUDGET_MS_PC34           3   /* Movement update budget */
#define DM1_V1_VIEWPORT_BUDGET_MS_PC34           6   /* Viewport render budget */
#define DM1_V1_DIALOG_BUDGET_MS_PC34             1   /* Dialog/message budget */
#define DM1_V1_SAVELOAD_BUDGET_MS_PC34           1   /* Save/load check budget */

/* ── Per-tick result from game loop orchestrator ──────────────────── */
typedef struct {
    DM1_V1_GameLoopPhasePc34 lastPhaseCompleted;
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
} DM1_V1_GameLoopTickResultPc34;

/* ── Frame timing statistics ──────────────────────────────────────── */
typedef struct {
    uint32_t totalFrames;          /* G1086_VerticalBlankCount equivalent */
    uint32_t droppedFrames;        /* frames that exceeded budget */
    uint32_t longestFrameUs;       /* worst-case frame time in microseconds */
    uint32_t avgFrameUs;           /* running average frame time */
    uint32_t budgetOverrunCount;   /* times any phase exceeded its budget */
} DM1_V1_FrameTimingStatsPc34;

/* ── Game loop persistent state ───────────────────────────────────── */
typedef struct {
    /* Loop status */
    DM1_V1_GameLoopStatusPc34 loopStatus;
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
    DM1_V1_FrameTimingStatsPc34 frameStats;
} DM1_V1_GameLoopStatePc34;

/* ── Initialization ───────────────────────────────────────────────── */

/* Initialize game loop state. extendedVBlankWait selects 12 vs 10. */
void DM1_V1_GameLoop_InitPc34Compat(DM1_V1_GameLoopStatePc34 *state, int extendedVBlankWait);

/* Set configurable tick rate in Hz (default: 50). */
void DM1_V1_GameLoop_SetTickRatePc34Compat(DM1_V1_GameLoopStatePc34 *state, int hz);

/* ── Core loop ────────────────────────────────────────────────────── */

/* Process one tick of the game loop (F0002 body).
 * Pure orchestration — sets flags; caller invokes subsystems. */
DM1_V1_GameLoopTickResultPc34 DM1_V1_GameLoop_TickPc34Compat(DM1_V1_GameLoopStatePc34 *state, uint32_t nowMs);

/* Simulate VBlank interrupt (F0577). Increments G0317 + G1086. */
void DM1_V1_GameLoop_VBlankTickPc34Compat(DM1_V1_GameLoopStatePc34 *state);

/* ── Pause/resume (G2586_TimerActive) ─────────────────────────────── */

/* Pause the game loop (dialogue, loading, save screen). */
void DM1_V1_GameLoop_PausePc34Compat(DM1_V1_GameLoopStatePc34 *state);

/* Resume the game loop. */
void DM1_V1_GameLoop_ResumePc34Compat(DM1_V1_GameLoopStatePc34 *state);

/* Check if game loop is paused. */
int DM1_V1_GameLoop_IsPausedPc34Compat(const DM1_V1_GameLoopStatePc34 *state);

/* ── State mutation ───────────────────────────────────────────────── */

void DM1_V1_GameLoop_RequestNewMapPc34Compat(DM1_V1_GameLoopStatePc34 *state, int newMapIndex);
void DM1_V1_GameLoop_SetPartyDeadPc34Compat(DM1_V1_GameLoopStatePc34 *state);
void DM1_V1_GameLoop_SetInventoryPc34Compat(DM1_V1_GameLoopStatePc34 *state, int championOrdinal);
void DM1_V1_GameLoop_SetRestingPc34Compat(DM1_V1_GameLoopStatePc34 *state, int resting);
void DM1_V1_GameLoop_RequestExitPc34Compat(DM1_V1_GameLoopStatePc34 *state);
int  DM1_V1_GameLoop_ShouldContinuePc34Compat(const DM1_V1_GameLoopStatePc34 *state);

/* ── Frame budget monitoring ──────────────────────────────────────── */

/* Record a phase's elapsed time for budget tracking. */
void DM1_V1_GameLoop_RecordPhaseTimePc34Compat(DM1_V1_GameLoopStatePc34 *state,
                                     DM1_V1_GameLoopPhasePc34 phase,
                                     uint32_t elapsedUs);

/* Get current frame timing statistics. */
DM1_V1_FrameTimingStatsPc34 DM1_V1_GameLoop_GetFrameStatsPc34Compat(const DM1_V1_GameLoopStatePc34 *state);

/* Reset frame timing statistics. */
void DM1_V1_GameLoop_ResetFrameStatsPc34Compat(DM1_V1_GameLoopStatePc34 *state);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *DM1_V1_GameLoop_SourceEvidencePc34Compat(void);

/* Backward-compatible M11 names for existing shared call sites. */
typedef DM1_V1_GameLoopPhasePc34 M11_GameLoopPhase;
typedef DM1_V1_GameLoopStatusPc34 M11_GameLoopStatus;
typedef DM1_V1_GameLoopTickResultPc34 M11_GameLoopTickResult;
typedef DM1_V1_FrameTimingStatsPc34 M11_FrameTimingStats;
typedef DM1_V1_GameLoopStatePc34 M11_GameLoopState;

#define M11_FRAME_RATE_HZ DM1_V1_FRAME_RATE_HZ_PC34
#define M11_FRAME_BUDGET_MS DM1_V1_FRAME_BUDGET_MS_PC34
#define M11_VBLANK_WAIT_MAX_DEFAULT DM1_V1_VBLANK_WAIT_MAX_DEFAULT_PC34
#define M11_VBLANK_WAIT_MAX_EXTENDED DM1_V1_VBLANK_WAIT_MAX_EXTENDED_PC34
#define M11_VBLANK_TIMER_BUDGET_MS DM1_V1_VBLANK_TIMER_BUDGET_MS_PC34
#define M11_INPUT_POLL_BUDGET_MS DM1_V1_INPUT_POLL_BUDGET_MS_PC34
#define M11_COMMAND_BUDGET_MS DM1_V1_COMMAND_BUDGET_MS_PC34
#define M11_MOVEMENT_BUDGET_MS DM1_V1_MOVEMENT_BUDGET_MS_PC34
#define M11_VIEWPORT_BUDGET_MS DM1_V1_VIEWPORT_BUDGET_MS_PC34
#define M11_DIALOG_BUDGET_MS DM1_V1_DIALOG_BUDGET_MS_PC34
#define M11_SAVELOAD_BUDGET_MS DM1_V1_SAVELOAD_BUDGET_MS_PC34

#define m11_game_loop_init DM1_V1_GameLoop_InitPc34Compat
#define m11_game_loop_set_tick_rate DM1_V1_GameLoop_SetTickRatePc34Compat
#define m11_game_loop_tick DM1_V1_GameLoop_TickPc34Compat
#define m11_game_loop_vblank_tick DM1_V1_GameLoop_VBlankTickPc34Compat
#define m11_game_loop_pause DM1_V1_GameLoop_PausePc34Compat
#define m11_game_loop_resume DM1_V1_GameLoop_ResumePc34Compat
#define m11_game_loop_is_paused DM1_V1_GameLoop_IsPausedPc34Compat
#define m11_game_loop_request_new_map DM1_V1_GameLoop_RequestNewMapPc34Compat
#define m11_game_loop_set_party_dead DM1_V1_GameLoop_SetPartyDeadPc34Compat
#define m11_game_loop_set_inventory DM1_V1_GameLoop_SetInventoryPc34Compat
#define m11_game_loop_set_resting DM1_V1_GameLoop_SetRestingPc34Compat
#define m11_game_loop_request_exit DM1_V1_GameLoop_RequestExitPc34Compat
#define m11_game_loop_should_continue DM1_V1_GameLoop_ShouldContinuePc34Compat
#define m11_game_loop_record_phase_time DM1_V1_GameLoop_RecordPhaseTimePc34Compat
#define m11_game_loop_get_frame_stats DM1_V1_GameLoop_GetFrameStatsPc34Compat
#define m11_game_loop_reset_frame_stats DM1_V1_GameLoop_ResetFrameStatsPc34Compat
#define m11_game_loop_source_evidence DM1_V1_GameLoop_SourceEvidencePc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GAME_LOOP_PC34_COMPAT_H */
