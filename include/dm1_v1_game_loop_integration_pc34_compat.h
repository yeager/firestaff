/* DM1 V1 Game Loop Integration — source-locked from ReDMCSB
 * GAMELOOP.C F0002_MAIN_GameLoop_CPSDF: main infinite loop
 * GAMELOOP.C G0318_i_WaitForInputMaximumVerticalBlankCount = 10 (PC34)
 * VBLANK.C: vertical blank interrupt handler, frame timing
 * DM.C: platform init (VGA/EGA/Tandy, Sound Blaster/AdLib, Mouse/Joystick)
 * INPUT.C: keyboard/mouse input processing */
#ifndef FIRESTAFF_DM1_V1_GAME_LOOP_INTEGRATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GAME_LOOP_INTEGRATION_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Game states — derived from GAMELOOP.C/TITLE.C/ENTRANCE.C/ENDGAME.C flow */
typedef enum {
    M11_GL_STATE_INIT = 0,
    M11_GL_STATE_TITLE,         /* TITLE.C: title screen display */
    M11_GL_STATE_ENTRANCE,      /* ENTRANCE.C: champion selection hall */
    M11_GL_STATE_DUNGEON,       /* GAMELOOP.C F0002: main dungeon gameplay */
    M11_GL_STATE_INVENTORY,     /* MENU.C: inventory/champion screen */
    M11_GL_STATE_MAP,           /* Map display (if available) */
    M11_GL_STATE_ENDGAME,       /* ENDGAME.C: victory sequence */
    M11_GL_STATE_DEATH,         /* Party death screen */
    M11_GL_STATE_SAVE_LOAD,     /* Save/load menu */
    M11_GL_STATE_RESTART        /* G0523_B_RestartGameRequested from DECOMPDU.C */
} M11_GL_GameState;

typedef void (*M11_GL_StateCallback)(void* userdata);

/* Per-state enter/exit callbacks */
typedef struct {
    M11_GL_StateCallback on_enter;
    M11_GL_StateCallback on_exit;
    M11_GL_StateCallback on_update;
    void* userdata;
} M11_GL_StateHandler;

/* Frame timing — G0317/G0318 from GAMELOOP.C */
typedef struct {
    uint32_t frame_count;       /* Total frames since game start */
    uint32_t vblank_count;      /* G0317_i_WaitForInputVerticalBlankCount */
    uint16_t max_vblank_wait;   /* G0318 = 10 (PC34) or 12 (later platforms) */
    uint32_t last_frame_ms;     /* Timestamp of last frame */
    uint32_t target_frame_ms;   /* Target frame duration (~55ms for 18.2Hz) */
    bool     paused;
} M11_GL_FrameTiming;

typedef struct {
    M11_GL_GameState   current_state;
    M11_GL_GameState   pending_state;  /* For deferred transitions */
    bool               transition_pending;
    M11_GL_StateHandler handlers[10];  /* One per M11_GL_GameState */
    M11_GL_FrameTiming timing;
    bool               restart_requested; /* G0523 mirror */
    bool               running;
} M11_GL_State;

void m11_gl_init(M11_GL_State* state);
void m11_gl_register_handler(M11_GL_State* state, M11_GL_GameState gs,
                              M11_GL_StateCallback on_enter,
                              M11_GL_StateCallback on_exit,
                              M11_GL_StateCallback on_update,
                              void* userdata);
void m11_gl_set_state(M11_GL_State* state, M11_GL_GameState new_state);
M11_GL_GameState m11_gl_get_state(const M11_GL_State* state);
void m11_gl_frame_update(M11_GL_State* state, uint32_t current_ms);
bool m11_gl_is_dungeon_running(const M11_GL_State* state);
void m11_gl_request_restart(M11_GL_State* state);
void m11_gl_stop(M11_GL_State* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GAME_LOOP_INTEGRATION_PC34_COMPAT_H */
