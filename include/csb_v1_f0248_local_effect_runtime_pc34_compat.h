#ifndef FIRESTAFF_CSB_V1_F0248_LOCAL_EFFECT_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0248_LOCAL_EFFECT_RUNTIME_PC34_COMPAT_H

#include "dm1_v1_sensor_trigger_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* F0270 output consumed after one F0248 wall-sensor list. */
typedef struct {
    int valid;
    int award_steal_experience;
    int leader_only;
    int rotation_effect;
    int map_x;
    int map_y;
    int cell;
} CSB_V1_F0248LocalEffectReceipt_PC34;

int csb_v1_f0248_local_effect_consume_pc34_compat(
    const struct SensorTriggerResult_Compat *sensor_result,
    int source_map_x,
    int source_map_y,
    int source_cell,
    CSB_V1_F0248LocalEffectReceipt_PC34 *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
