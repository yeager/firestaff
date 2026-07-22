#include "dm1_v1_f0407_completion_c11_route_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)
int main(void) {
    DM1_ActionF0407CompletionPlanPc34 plan;
    DM1_V1_F0407CompletionC11RouteInputPc34 input;
    DM1_V1_F0407CompletionC11RouteReceiptPc34 receipt;
    memset(&plan, 0, sizeof(plan)); memset(&input, 0, sizeof(input));
    plan.valid = 1; plan.disabledTicks = 6; plan.actionDisabledIndex = DM1_ACTION_BASH;
    plan.actionEnableSlotOrdinal = 1; input.championOrdinal = 2; input.sourceTick = 100; input.completion = &plan;
    CHECK(dm1_v1_f0407_completion_c11_route_build_pc34(&input, &receipt));
    CHECK(receipt.accepted && receipt.eventType == 11 && receipt.championOrdinal == 2 &&
          receipt.actionIndex == DM1_ACTION_BASH && receipt.fireAtTick == 106);
    plan.preservesExistingActionDisable = 1;
    CHECK(!dm1_v1_f0407_completion_c11_route_build_pc34(&input, &receipt));
    plan.preservesExistingActionDisable = 0; plan.actionDisabledIndex = DM1_ACTION_THROW;
    CHECK(!dm1_v1_f0407_completion_c11_route_build_pc34(&input, &receipt));
    plan.actionDisabledIndex = DM1_ACTION_BASH; input.championOrdinal = 0;
    CHECK(!dm1_v1_f0407_completion_c11_route_build_pc34(&input, &receipt));
    printf("%s\n", failures ? "failed" : "ok: F0407 completion C11 route"); return failures ? 1 : 0;
}
