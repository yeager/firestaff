#include "csb_v1_f0248_local_effect_runtime_pc34_compat.h"

#include <string.h>

int csb_v1_f0248_local_effect_consume_pc34_compat(
    const struct SensorTriggerResult_Compat *sensor_result,
    int source_map_x,
    int source_map_y,
    int source_cell,
    CSB_V1_F0248LocalEffectReceipt_PC34 *out_receipt)
{
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!sensor_result || !sensor_result->triggered ||
        !sensor_result->isLocal || source_cell < -1 || source_cell > 3) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->map_x = source_map_x;
    out_receipt->map_y = source_map_y;
    out_receipt->cell = source_cell;
    if (sensor_result->localEffectValue ==
        DM1_EFFECT_ADD_300XP_STEAL_SKILL) {
        /* SENSOR.C F0270 calls F0269 with LeaderOnly when the source cell
         * is concrete; only CM1_CELL_ANY shares the 300 XP among the party. */
        out_receipt->award_steal_experience = 1;
        out_receipt->leader_only = (source_cell != -1);
    } else {
        /* ReDMCSB MOVESENS.C F0272 passes M049_LOCAL_EFFECT (not the
         * sensor's resolved remote effect) to F0270. F0271 rotates only for
         * local CLEAR or TOGGLE; local SET is deliberately a no-op. */
        out_receipt->rotation_effect = sensor_result->localEffectValue;
    }
    return 1;
}
