
#ifndef FIRESTAFF_GAME_LOOP_H
#define FIRESTAFF_GAME_LOOP_H

#include <stdint.h>
#include "firestaff_input.h"
#include "nexus_v1_engine.h"
#include "csb_v1_viewport_pc34_compat.h"

/* Firestaff Game Loop — connects V1 engine, V2 rendering, SDL.
 *
 * Architecture:
 *   SDL event poll → input translation → V1 command queue
 *   V1 game tick (55ms) → state update
 *   V2 render (60fps) → EPX upscale → SDL present
 *
 * The game loop respects V1 tick rate exactly.
 * V2 rendering interpolates between V1 states. */

typedef enum {
    FS_GAME_DM1 = 0,
    FS_GAME_CSB,
    FS_GAME_DM2,
    FS_GAME_NEXUS,
} FS_GameId;

typedef enum {
    FS_VERSION_V1 = 0,
    FS_VERSION_V21,
    FS_VERSION_V22,
} FS_GameVersion;

typedef struct {
    FS_GameId game;
    FS_GameVersion version;
    int window_width;
    int window_height;
    int fullscreen;
    int vsync;
    const char *data_dir;
    const char *save_dir;
    int skip_menu;        /* 1=skip startup menu, start game directly */
} FS_GameConfig;

/* Detailed startup error for CLI direct-start diagnostics */
#define FS_ERROR_MSG_MAX 512
typedef struct {
    int code;                          /* 0=ok, negative=error */
    char message[FS_ERROR_MSG_MAX];    /* human-readable error */
    char detail[FS_ERROR_MSG_MAX];     /* technical detail (paths, errno, etc) */
    char suggestion[FS_ERROR_MSG_MAX]; /* how to fix it */
} FS_StartupError;

typedef struct {
    FS_GameConfig config;
    int running;
    int paused;
    uint32_t v1_tick_accumulator_ms;
    uint32_t last_frame_ms;
    uint32_t frame_count;
    /* State */
    int current_level;
    int party_x, party_y;
    int party_direction;
    int in_menu;
    FS_InputQueue input_queue;
    Nexus_V1_Engine nexus_engine;
    CSB_V1_ViewportConfig csb_viewport;  /* CSB V1 viewport state */
    FS_StartupError last_error;
} FS_GameState;

int fs_game_init(FS_GameState *state, const FS_GameConfig *config);
int fs_game_load_assets(FS_GameState *state);
void fs_game_tick_v1(FS_GameState *state);
void fs_game_render_v2(FS_GameState *state);
void fs_game_handle_sdl_event(FS_GameState *state, const void *sdl_event);
void fs_game_run(FS_GameState *state);
void fs_game_shutdown(FS_GameState *state);

#endif

