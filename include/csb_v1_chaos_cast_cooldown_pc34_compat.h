#ifndef FIRESTAFF_CSB_V1_CHAOS_CAST_COOLDOWN_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CHAOS_CAST_COOLDOWN_PC34_COMPAT_H

#include "csb_v1_chaos_magic_pc34_compat.h"

#define CSB_V1_CHAOS_CAST_READY       0
#define CSB_V1_CHAOS_CAST_RUNNING     1
#define CSB_V1_CHAOS_CAST_COOLDOWN    2
#define CSB_V1_CHAOS_CAST_INVALID    -1
#define CSB_V1_CHAOS_CAST_BUSY       -2

typedef struct {
    CSB_V1_ChaosMagicState *chaos;
    int active_script_id;
    int default_cooldown_ticks;
    int cooldown_ticks;
    int dsa_call_depth;
    int dsa_calls_executed;
    int casts_started;
    int casts_completed;
    int casts_canceled;
} CSB_V1_ChaosCastCooldownState;

void csb_v1_chaos_cast_cooldown_init(CSB_V1_ChaosCastCooldownState *state,
    CSB_V1_ChaosMagicState *chaos, int cooldown_ticks);
int csb_v1_chaos_cast_cooldown_begin(CSB_V1_ChaosCastCooldownState *state,
    int script_id);
int csb_v1_chaos_cast_cooldown_tick(CSB_V1_ChaosCastCooldownState *state);
void csb_v1_chaos_cast_cooldown_cancel(CSB_V1_ChaosCastCooldownState *state);

#endif
