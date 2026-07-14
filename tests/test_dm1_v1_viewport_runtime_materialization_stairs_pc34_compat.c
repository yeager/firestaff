#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check_stair_route(int relative_forward,
                             int relative_side,
                             int expect_f0115,
                             int expect_stair_suppression)
{
    DM1_V1_ViewportRuntimeMaterializationInputPc34 input;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 decision;

    memset(&input, 0, sizeof(input));
    input.relativeForward = relative_forward;
    input.relativeSide = relative_side;
    input.elementType = DUNGEON_ELEMENT_STAIRS;
    input.floorItemCount = 1;
    input.projectileCount = 1;
    input.projectileCell = 2;
    input.runtimeOrigin = DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34;
    if (!dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &decision) ||
        !decision.valid || !decision.consumedF0172SquareFacts ||
        decision.consumedF0115ThingPass != expect_f0115 ||
        decision.f0115EligibleForSquare != expect_f0115 ||
        decision.suppressedF0115ForStairs != expect_stair_suppression) {
        return 0;
    }
    if (!expect_f0115) {
        return !decision.drawFloorItems && !decision.drawRuntimeProjectiles &&
               !decision.drawDeferredSpellEffects && decision.itemZone < 0 &&
               decision.projectileZone < 0;
    }
    return decision.drawFloorItems && decision.drawRuntimeProjectiles &&
           decision.drawDeferredSpellEffects && decision.itemZone >= 0 &&
           decision.projectileZone >= 0;
}

int main(void)
{
    DM1_V1_ViewportRuntimeMaterializationInputPc34 d0_input;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 d0_decision;

    if (!check_stair_route(1, 0, 0, 1) ||  /* F0124 front stairs */
        !check_stair_route(1, -1, 1, 0) || /* F0122 D1L stairs */
        !check_stair_route(1, 1, 1, 0) ||  /* F0123 D1R stairs */
        !check_stair_route(3, -1, 1, 0)) { /* F0116 D3L stairs */
        return 1;
    }

    /* D0 has no F0115 row at all, matching F0125/F0126's early return. */
    memset(&d0_input, 0, sizeof(d0_input));
    d0_input.relativeForward = 0;
    d0_input.relativeSide = -1;
    d0_input.elementType = DUNGEON_ELEMENT_STAIRS;
    d0_input.floorItemCount = 1;
    d0_input.projectileCount = 1;
    d0_input.runtimeOrigin = DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34;
    if (!dm1_v1_viewport_runtime_materialization_decide_pc34(
            &d0_input, &d0_decision) || d0_decision.valid ||
        d0_decision.consumedF0115ThingPass || d0_decision.drawFloorItems ||
        d0_decision.drawRuntimeProjectiles || d0_decision.drawDeferredSpellEffects) {
        return 1;
    }
    puts("ok: DM1 stair branches admit F0115 only on D1-D3 lateral lanes");
    return 0;
}
