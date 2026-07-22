#ifndef FIRESTAFF_DM1_V1_F0407_COMPLETION_C11_ROUTE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0407_COMPLETION_C11_ROUTE_PC34_COMPAT_H

#include "dm1_v1_action_xp_graphic560_pc34_compat.h"

/* ReDMCSB F0407:1620-1628 -> CHAMPION.C F0330 C11 schedule facts. */
typedef struct {
    int championOrdinal;
    unsigned int sourceTick;
    const DM1_ActionF0407CompletionPlanPc34 *completion;
} DM1_V1_F0407CompletionC11RouteInputPc34;

typedef struct {
    int accepted;
    int eventType;
    int championOrdinal;
    int actionIndex;
    int actionEnableSlotOrdinal;
    unsigned int sourceTick;
    unsigned int fireAtTick;
} DM1_V1_F0407CompletionC11RouteReceiptPc34;

/* Produces only the F0407-owned C11 route. F0328-owned THROW stays separate. */
int dm1_v1_f0407_completion_c11_route_build_pc34(
    const DM1_V1_F0407CompletionC11RouteInputPc34 *input,
    DM1_V1_F0407CompletionC11RouteReceiptPc34 *outReceipt);

#endif
