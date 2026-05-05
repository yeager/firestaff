#ifndef FIRESTAFF_DM1_V2_RUNTIME_PC34_H
#define FIRESTAFF_DM1_V2_RUNTIME_PC34_H

#include <stdint.h>
#include "dm1_v2_movement_engine_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V2_RUNTIME_STOPPED = 0,
    DM1_V2_RUNTIME_RUNNING = 1
} DM1_V2_RuntimeMode;

typedef struct {
    DM1_V2_RuntimeMode mode;
    uint32_t tickCount;
    uint32_t lastTickMs;
    DM1_V2_PlayerPos player;
    DM1_V2_MoveParams movement;
    DM1_V2_ViewportState viewport;
    int lastCommand;
} DM1_V2_RuntimeState;

void dm1_v2_runtime_init(DM1_V2_RuntimeState* runtime);
void dm1_v2_runtime_start(DM1_V2_RuntimeState* runtime, uint32_t nowMs);
void dm1_v2_runtime_stop(DM1_V2_RuntimeState* runtime);
int dm1_v2_runtime_is_running(const DM1_V2_RuntimeState* runtime);
void dm1_v2_runtime_tick(DM1_V2_RuntimeState* runtime, uint32_t nowMs);
int dm1_v2_runtime_apply_command(DM1_V2_RuntimeState* runtime, int command, uint32_t nowMs);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_RUNTIME_PC34_H */
