#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM1_V1_ViewportRuntimeMaterializationInputPc34 input;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 decision;

    memset(&input, 0, sizeof(input));
    input.relativeForward = 1;
    input.elementType = DUNGEON_ELEMENT_CORRIDOR;
    input.floorItemCount = 1;
    input.mapIndex = 2;
    input.mapX = 3;
    input.mapY = 4;
    input.runtimeOrigin = DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34;
    if (!dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &decision) ||
        !decision.valid || !decision.consumedF0172SquareFacts ||
        !decision.consumedF0115ThingPass || !decision.drawFloorItems ||
        decision.itemZone < 0 || decision.drawRuntimeProjectiles ||
        decision.drawDeferredSpellEffects || !decision.noM11Fallback) {
        return 1;
    }
    input.elementType = DUNGEON_ELEMENT_WALL;
    if (!dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &decision) ||
        decision.drawFloorItems || decision.itemZone >= 0) {
        return 1;
    }
    input.elementType = DUNGEON_ELEMENT_CORRIDOR;
    input.floorItemCount = 0;
    input.projectileCount = 1; /* stale square count, not a live F0115 effect */
    if (!dm1_v1_viewport_runtime_materialization_decide_pc34(&input, &decision) ||
        decision.drawFloorItems || decision.drawRuntimeProjectiles ||
        decision.projectileZone >= 0 || decision.liveProjectileCount != 0) {
        return 1;
    }

    puts("ok: DM1 F0115 item request reaches only matching host material lane");
    return 0;
}
