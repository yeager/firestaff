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
 *   m11_engine_init()     — allocate and wire all subsystems
 *   m11_engine_tick()     — advance one game frame (F0002 body)
 *   m11_engine_shutdown() — tear down all subsystems
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
} M11_EngineConfig;

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
    M11_GameStateId     currentState;
    M11_GameLoopPhase   lastPhase;
    bool                partyDead;
    bool                gameWon;
    bool                exitRequested;
    uint32_t            gameTime;
    uint32_t            frameNumber;
} M11_EngineTickResult;

/* ── Engine aggregate state ───────────────────────────────────────── */
typedef struct {
    /* ── Subsystem states ── */
    M11_GameStateMachine              stateMachine;
    M11_GameLoopState                 gameLoop;
    M11_GL_State                      loopIntegration;
    M11_InputState                    input;
    M11_DD_DungeonData                dungeonData;
    DM1_Viewport3DState               viewport3d;
    struct Dm1V1MovementPipelinePc34Compat movementPipeline;
    M11_DG_State                      dialog;
    M11_CK_State                      clickRouting;
    M11_SL_State                      saveLoad;
    M11_ScreenState                   screen;

    /* ── Engine metadata ── */
    M11_EngineConfig                  config;
    bool                              initialized;
    uint32_t                          totalTicks;
} M11_Engine;

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
bool m11_engine_init(M11_Engine *engine, const M11_EngineConfig *config);

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
M11_EngineTickResult m11_engine_tick(M11_Engine *engine, uint32_t nowMs);

/*
 * Shut down all engine subsystems.  Safe to call on partially
 * initialized or already-shut-down engine.
 *
 * Equivalent to DM.C cleanup path.
 */
void m11_engine_shutdown(M11_Engine *engine);

/* ── Engine queries ───────────────────────────────────────────────── */

/* Get current game state. */
M11_GameStateId m11_engine_get_state(const M11_Engine *engine);

/* Get pointer to the central dungeon data (for subsystem queries). */
M11_DD_DungeonData *m11_engine_get_dungeon_data(M11_Engine *engine);

/* Get pointer to input state (for feeding external events). */
M11_InputState *m11_engine_get_input(M11_Engine *engine);

/* Get pointer to screen state (for presentation). */
M11_ScreenState *m11_engine_get_screen(M11_Engine *engine);

/* Get pointer to save/load state. */
M11_SL_State *m11_engine_get_save_load(M11_Engine *engine);

/* ── Engine actions (wrappers for common multi-subsystem ops) ────── */

/* Start a new game.  Loads dungeon level 0, places party at entrance. */
bool m11_engine_new_game(M11_Engine *engine);

/* Load a saved game from slot. */
bool m11_engine_load_game(M11_Engine *engine, uint8_t slot);

/* Save current game to slot. */
bool m11_engine_save_game(M11_Engine *engine, uint8_t slot);

/* Request game exit.  Next tick will return exitRequested=true. */
void m11_engine_request_exit(M11_Engine *engine);

/* ── Module manifest ──────────────────────────────────────────────── */

/*
 * Return the number of dm1_v1_* modules compiled into the engine.
 * This is the authoritative module count.
 */
int m11_engine_module_count(void);

/*
 * Return the name of dm1_v1_* module at index (0-based).
 * Returns NULL if index is out of range.
 */
const char *m11_engine_module_name(int index);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *m11_engine_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_ENGINE_PC34_COMPAT_H */
