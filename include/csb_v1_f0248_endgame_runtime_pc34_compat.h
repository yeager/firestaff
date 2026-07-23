#ifndef FIRESTAFF_CSB_V1_F0248_ENDGAME_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0248_ENDGAME_RUNTIME_PC34_COMPAT_H

#include "csb_v1_dungeon_world_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Runtime-owned receipt for ReDMCSB TIMELINE.C F0248's C018 branch. */
typedef struct {
    int valid;
    int delay_ticks;
    int palette_curtain;
    int restart_disabled;
    int game_won;
    int f0666_entered;
    int startend_endgame_requested;
} CSB_V1_F0248EndgameRuntimeReceipt_PC34;

/* Consumes only F0731's C018 result.  A prior victory or malformed delay is
 * rejected so the runtime cannot promote a synthetic endgame transition. */
int csb_v1_f0248_endgame_consume_pc34_compat(
    const struct SensorTriggerResult_Compat *sensor_result,
    int already_won,
    CSB_V1_F0248EndgameRuntimeReceipt_PC34 *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
