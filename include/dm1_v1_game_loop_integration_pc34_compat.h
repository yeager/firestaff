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
    DM1_V1_LOOP_INTEGRATION_STATE_INIT_PC34 = 0,
    DM1_V1_LOOP_INTEGRATION_STATE_TITLE_PC34,         /* TITLE.C: title screen display */
    DM1_V1_LOOP_INTEGRATION_STATE_ENTRANCE_PC34,      /* ENTRANCE.C: champion selection hall */
    DM1_V1_LOOP_INTEGRATION_STATE_DUNGEON_PC34,       /* GAMELOOP.C F0002: main dungeon gameplay */
    DM1_V1_LOOP_INTEGRATION_STATE_INVENTORY_PC34,     /* MENU.C: inventory/champion screen */
    DM1_V1_LOOP_INTEGRATION_STATE_MAP_PC34,           /* Map display (if available) */
    DM1_V1_LOOP_INTEGRATION_STATE_ENDGAME_PC34,       /* ENDGAME.C: victory sequence */
    DM1_V1_LOOP_INTEGRATION_STATE_DEATH_PC34,         /* Party death screen */
    DM1_V1_LOOP_INTEGRATION_STATE_SAVE_LOAD_PC34,     /* Save/load menu */
    DM1_V1_LOOP_INTEGRATION_STATE_RESTART_PC34        /* G0523_B_RestartGameRequested from DECOMPDU.C */
} DM1_V1_LoopIntegrationGameStatePc34;

typedef void (*DM1_V1_LoopIntegrationStateCallbackPc34)(void* userdata);

/* Per-state enter/exit callbacks */
typedef struct {
    DM1_V1_LoopIntegrationStateCallbackPc34 on_enter;
    DM1_V1_LoopIntegrationStateCallbackPc34 on_exit;
    DM1_V1_LoopIntegrationStateCallbackPc34 on_update;
    void* userdata;
} DM1_V1_LoopIntegrationStateHandlerPc34;

/* Frame timing — G0317/G0318 from GAMELOOP.C */
typedef struct {
    uint32_t frame_count;       /* Total frames since game start */
    uint32_t vblank_count;      /* G0317_i_WaitForInputVerticalBlankCount */
    uint16_t max_vblank_wait;   /* G0318 = 10 (PC34) or 12 (later platforms) */
    uint32_t last_frame_ms;     /* Timestamp of last frame */
    uint32_t target_frame_ms;   /* Target frame duration (~55ms for 18.2Hz) */
    bool     paused;
} DM1_V1_LoopIntegrationFrameTimingPc34;

typedef struct {
    DM1_V1_LoopIntegrationGameStatePc34   current_state;
    DM1_V1_LoopIntegrationGameStatePc34   pending_state;  /* For deferred transitions */
    bool               transition_pending;
    DM1_V1_LoopIntegrationStateHandlerPc34 handlers[10];  /* One per DM1_V1_LoopIntegrationGameStatePc34 */
    DM1_V1_LoopIntegrationFrameTimingPc34 timing;
    bool               restart_requested; /* G0523 mirror */
    bool               running;
} DM1_V1_LoopIntegrationStatePc34;

void DM1_V1_LoopIntegration_InitPc34Compat(DM1_V1_LoopIntegrationStatePc34* state);
void DM1_V1_LoopIntegration_RegisterHandlerPc34Compat(DM1_V1_LoopIntegrationStatePc34* state, DM1_V1_LoopIntegrationGameStatePc34 gs,
                              DM1_V1_LoopIntegrationStateCallbackPc34 on_enter,
                              DM1_V1_LoopIntegrationStateCallbackPc34 on_exit,
                              DM1_V1_LoopIntegrationStateCallbackPc34 on_update,
                              void* userdata);
void DM1_V1_LoopIntegration_SetStatePc34Compat(DM1_V1_LoopIntegrationStatePc34* state, DM1_V1_LoopIntegrationGameStatePc34 new_state);
DM1_V1_LoopIntegrationGameStatePc34 DM1_V1_LoopIntegration_GetStatePc34Compat(const DM1_V1_LoopIntegrationStatePc34* state);
void DM1_V1_LoopIntegration_FrameUpdatePc34Compat(DM1_V1_LoopIntegrationStatePc34* state, uint32_t current_ms);
bool DM1_V1_LoopIntegration_IsDungeonRunningPc34Compat(const DM1_V1_LoopIntegrationStatePc34* state);
void DM1_V1_LoopIntegration_RequestRestartPc34Compat(DM1_V1_LoopIntegrationStatePc34* state);
void DM1_V1_LoopIntegration_StopPc34Compat(DM1_V1_LoopIntegrationStatePc34* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GAME_LOOP_INTEGRATION_PC34_COMPAT_H */
