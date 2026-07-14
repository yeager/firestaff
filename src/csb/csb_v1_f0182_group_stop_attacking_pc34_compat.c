#include "csb_v1_f0182_group_stop_attacking_pc34_compat.h"

void csb_v1_f0182_group_stop_attacking_pc34(
    CSB_V1_GroupActiveGroupPc34 *active_group,
    int16_t map_x,
    int16_t map_y,
    CSB_V1_GroupDeleteEventsPc34 delete_events,
    void *delete_events_context)
{
    int creature_index;

    /* ReDMCSB GROUP.C F0182 lines 374-381 clears MASK0x0080_IS_ATTACKING
     * for each of the four Aspect bytes before calling F0181. */
    for (creature_index = 0; creature_index < 4; ++creature_index) {
        active_group->aspect[creature_index] &= (uint8_t)~0x80u;
    }
    delete_events(delete_events_context, map_x, map_y);
}
