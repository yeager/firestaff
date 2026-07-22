#include "dm1_v1_f0407_completion_c11_route_pc34_compat.h"

#include <string.h>

enum { DM1_V1_F0407_C11_ENABLE_CHAMPION_ACTION_PC34 = 11 };

int dm1_v1_f0407_completion_c11_route_build_pc34(
    const DM1_V1_F0407CompletionC11RouteInputPc34 *input,
    DM1_V1_F0407CompletionC11RouteReceiptPc34 *outReceipt)
{
    const DM1_ActionF0407CompletionPlanPc34 *plan;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!input || !(plan = input->completion) || !plan->valid ||
        input->championOrdinal < 1 || input->championOrdinal > 4 ||
        input->sourceTick == 0 || plan->preservesExistingActionDisable ||
        plan->disabledTicks <= 0 || plan->actionDisabledIndex < 0 ||
        plan->actionDisabledIndex >= DM1_GRAPHIC560_ACTION_COUNT ||
        plan->actionDisabledIndex == DM1_ACTION_THROW ||
        plan->actionEnableSlotOrdinal < 0 || plan->actionEnableSlotOrdinal > 1) {
        return 0;
    }
    outReceipt->accepted = 1;
    outReceipt->eventType = DM1_V1_F0407_C11_ENABLE_CHAMPION_ACTION_PC34;
    outReceipt->championOrdinal = input->championOrdinal;
    outReceipt->actionIndex = plan->actionDisabledIndex;
    outReceipt->actionEnableSlotOrdinal = plan->actionEnableSlotOrdinal;
    outReceipt->sourceTick = input->sourceTick;
    outReceipt->fireAtTick = input->sourceTick + (unsigned int)plan->disabledTicks;
    return 1;
}
