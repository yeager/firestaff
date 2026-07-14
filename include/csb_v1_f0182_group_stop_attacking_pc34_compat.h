#ifndef FIRESTAFF_CSB_V1_F0182_GROUP_STOP_ATTACKING_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0182_GROUP_STOP_ATTACKING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB GROUP.C F0182 operates only on ACTIVE_GROUP.Aspect. */
typedef struct {
    uint8_t aspect[4];
} CSB_V1_GroupActiveGroupPc34;

typedef void (*CSB_V1_GroupDeleteEventsPc34)(
    void *context,
    int16_t map_x,
    int16_t map_y);

/* GROUP.C F0182_GROUP_StopAttacking (line 374): clear the attacking bit of
 * every creature aspect, then delete the square's group reaction events. */
void csb_v1_f0182_group_stop_attacking_pc34(
    CSB_V1_GroupActiveGroupPc34 *active_group,
    int16_t map_x,
    int16_t map_y,
    CSB_V1_GroupDeleteEventsPc34 delete_events,
    void *delete_events_context);

#ifdef __cplusplus
}
#endif

#endif
