#ifndef FIRESTAFF_DM1_V1_ENGINE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ENGINE_PC34_COMPAT_H

/*
 * DM1 V1 Engine Integration Layer — pc34 compat.
 *
 * Master header + init/tick/shutdown API that wires the 56 dm1_v1_*
 * modules into a single coherent engine interface.
 *
 * The engine owns:
 *   - Game state machine     (dm1_v1_game_state)
 *   - Game loop orchestrator (dm1_v1_game_loop)
 *   - Input system           (dm1_v1_input_poll)
 *   - Central dungeon data   (dm1_v1_dungeon_data)
 *   - Viewport 3D pipeline   (dm1_v1_viewport_3d)
 *   - Movement pipeline      (dm1_v1_movement_pipeline)
 *   - Creature viewport      (dm1_v1_creature_viewport)
 *   - Blit/fill primitives   (dm1_v1_blit_fill)
 *   - Dialog/scroll messages  (dm1_v1_dialog_scroll)
 *   - Click routing           (dm1_v1_click_routing)
 *   - Save/load system        (dm1_v1_save_load_system)
 *   - Screen framebuffer      (dm1_v1_screen_framebuffer)
 *   - Game loop integration   (dm1_v1_game_loop_integration)
 *   - Event timer             (dm1_v1_event_timer)
 *
 * Callers use:
 *   DM1_V1_Engine_InitPc34Compat()     — allocate and wire all subsystems
 *   DM1_V1_Engine_TickPc34Compat()     — advance one game frame (F0002 body)
 *   DM1_V1_Engine_ShutdownPc34Compat() — tear down all subsystems
 *
 * The engine struct aggregates all subsystem states.  It does NOT
 * allocate heap memory — the caller provides the struct (typically
 * on the stack or as a static).
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   DM.C: F0449_DM_Main — top-level entry: init video, init sound,
 *         init input, load dungeon, run game loop, cleanup.
 *   GAMELOOP.C: F0002_MAIN_GameLoop_CPSDF — per-tick orchestration.
 *   STARTUP2.C: F0462_START_StartGame_CPSEF — new-game init sequence.
 *   STARTUP2.C: F0463_START_InitializeGame_CPSADEF — full init.
 */

#include <stdint.h>
#include <stdbool.h>

/* ── Subsystem headers (the modules this engine wires together) ───── */
#include "dm1_v1_game_state_pc34_compat.h"
#include "dm1_v1_game_loop_pc34_compat.h"
#include "dm1_v1_game_loop_integration_pc34_compat.h"
#include "dm1_v1_input_poll_pc34_compat.h"
#include "dm1_v1_dungeon_data_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_movement_pipeline_pc34_compat.h"
#include "dm1_v1_creature_viewport_pc34_compat.h"
#include "dm1_v1_blit_fill_pc34_compat.h"
#include "dm1_v1_dialog_scroll_pc34_compat.h"
#include "dm1_v1_click_routing_pc34_compat.h"
#include "dm1_v1_save_load_system_pc34_compat.h"
#include "dm1_v1_screen_framebuffer_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Engine configuration ─────────────────────────────────────────── */
typedef struct {
    const char *dungeon_dat_path;    /* Path to DUNGEON.DAT */
    const char *graphics_dat_path;   /* Path to GRAPHICS.DAT */
    const char *save_dir;            /* Save file directory */
    int         tick_rate_hz;        /* Target tick rate (default: 50) */
    int         extended_vblank;     /* Use 12-tick vblank wait (0=10) */
} DM1_V1_EngineConfigPc34;

/* ── Per-tick result ──────────────────────────────────────────────── */
typedef struct {
    /* Which phases completed this tick */
    bool stateTransitioned;
    bool inputProcessed;
    bool movementProcessed;
    bool timelineProcessed;
    bool viewportDrawn;
    bool dialogUpdated;

    /* Key flags for the caller */
    DM1_V1_GameStateIdPc34     currentState;
    DM1_V1_GameLoopPhasePc34   lastPhase;
    bool                partyDead;
    bool                gameWon;
    bool                exitRequested;
    uint32_t            gameTime;
    uint32_t            frameNumber;
} DM1_V1_EngineTickResultPc34;

/* ── Engine aggregate state ───────────────────────────────────────── */
typedef struct {
    /* ── Subsystem states ── */
    DM1_V1_GameStateMachinePc34              stateMachine;
    DM1_V1_GameLoopStatePc34                 gameLoop;
    DM1_V1_LoopIntegrationStatePc34                      loopIntegration;
    DM1_V1_InputStatePc34                    input;
    DM1_V1_DungeonDataPc34                dungeonData;
    DM1_Viewport3DState               viewport3d;
    struct Dm1V1MovementPipelinePc34Compat movementPipeline;
    DM1_V1_DialogStatePc34                      dialog;
    DM1_V1_ClickRoutingStatePc34                      clickRouting;
    DM1_V1_SaveLoadStatePc34                      saveLoad;
    DM1_V1_ScreenStatePc34                   screen;

    /* ── Engine metadata ── */
    DM1_V1_EngineConfigPc34                  config;
    bool                              initialized;
    uint32_t                          totalTicks;
} DM1_V1_EnginePc34;

/* ── Lifecycle API ────────────────────────────────────────────────── */

/*
 * Initialize all engine subsystems from config.
 *
 * Equivalent to DM.C F0449 init sequence:
 *   1. Screen/framebuffer init
 *   2. Input init
 *   3. Game state machine init → TITLE_SCREEN
 *   4. Game loop init (tick rate, vblank config)
 *   5. Dungeon data load (DUNGEON.DAT + GRAPHICS.DAT)
 *   6. Event timer init
 *   7. Movement pipeline init
 *   8. Viewport 3D init (wired to screen back buffer)
 *   9. Dialog/message init
 *  10. Click routing init + dungeon zone setup
 *  11. Save/load init
 *
 * Returns true if all subsystems initialized successfully.
 * On failure, partially initialized subsystems are cleaned up.
 */
bool DM1_V1_Engine_InitPc34Compat(DM1_V1_EnginePc34 *engine, const DM1_V1_EngineConfigPc34 *config);

/*
 * Process one game tick.
 *
 * Equivalent to one iteration of GAMELOOP.C F0002:
 *   1. Poll input (keyboard + mouse)
 *   2. Process command queue → movement pipeline
 *   3. Advance game time + process expired timeline events
 *   4. Update creature AI / group behavior
 *   5. Draw dungeon viewport (if dirty)
 *   6. Update dialog/message bar
 *   7. Check death / victory conditions
 *   8. Frame timing bookkeeping
 *
 * The caller provides nowMs (wall-clock milliseconds) for frame pacing.
 * Returns a result struct describing what happened this tick.
 */
DM1_V1_EngineTickResultPc34 DM1_V1_Engine_TickPc34Compat(DM1_V1_EnginePc34 *engine, uint32_t nowMs);

/*
 * Shut down all engine subsystems.  Safe to call on partially
 * initialized or already-shut-down engine.
 *
 * Equivalent to DM.C cleanup path.
 */
void DM1_V1_Engine_ShutdownPc34Compat(DM1_V1_EnginePc34 *engine);

/* ── Engine queries ───────────────────────────────────────────────── */

/* Get current game state. */
DM1_V1_GameStateIdPc34 DM1_V1_Engine_GetStatePc34Compat(const DM1_V1_EnginePc34 *engine);

/* Get pointer to the central dungeon data (for subsystem queries). */
DM1_V1_DungeonDataPc34 *DM1_V1_Engine_GetDungeonDataPc34Compat(DM1_V1_EnginePc34 *engine);

/* Get pointer to input state (for feeding external events). */
DM1_V1_InputStatePc34 *DM1_V1_Engine_GetInputPc34Compat(DM1_V1_EnginePc34 *engine);

/* Get pointer to screen state (for presentation). */
DM1_V1_ScreenStatePc34 *DM1_V1_Engine_GetScreenPc34Compat(DM1_V1_EnginePc34 *engine);

/* Get pointer to save/load state. */
DM1_V1_SaveLoadStatePc34 *DM1_V1_Engine_GetSaveLoadPc34Compat(DM1_V1_EnginePc34 *engine);

/* ── Engine actions (wrappers for common multi-subsystem ops) ────── */

/* Start a new game.  Loads dungeon level 0, places party at entrance. */
bool DM1_V1_Engine_NewGamePc34Compat(DM1_V1_EnginePc34 *engine);

/* Load a saved game from slot. */
bool DM1_V1_Engine_LoadGamePc34Compat(DM1_V1_EnginePc34 *engine, uint8_t slot);

/* Save current game to slot. */
bool DM1_V1_Engine_SaveGamePc34Compat(DM1_V1_EnginePc34 *engine, uint8_t slot);

/* Request game exit.  Next tick will return exitRequested=true. */
void DM1_V1_Engine_RequestExitPc34Compat(DM1_V1_EnginePc34 *engine);

/* ── Module manifest ──────────────────────────────────────────────── */

/*
 * Return the number of dm1_v1_* modules compiled into the engine.
 * This is the authoritative module count.
 */
int DM1_V1_Engine_ModuleCountPc34Compat(void);

/*
 * Return the name of dm1_v1_* module at index (0-based).
 * Returns NULL if index is out of range.
 */
const char *DM1_V1_Engine_ModuleNamePc34Compat(int index);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *DM1_V1_Engine_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_ENGINE_PC34_COMPAT_H */
