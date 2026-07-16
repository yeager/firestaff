#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1l_d1r_f0115_thing_pass_pc34_compat.h"

#include <string.h>
#include <stdio.h>

int main(void)
{
    const DM1V1D1LD1RF0115LanePc34Data *lane;
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 projectile;
    DM1_V1_ChampionMirrorRuntimeRenderInputPc34 input;
    DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 decision;

    lane = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    if (!lane || !dm1_v1_viewport_d1l_d1r_f0115_runtime_thing_receipt_pc34(
                     lane, 14, 1, 1, 1, &projectile)) {
        return 1;
    }
    memset(&input, 0, sizeof(input));
    input.wallSquareVisible = 1;
    input.sensorType = 127;
    input.sensorData = 13;
    input.ornamentOrdinal = 4;
    input.thingCell = 2;
    input.visibleWallCell = 2;
    input.backingAssetAvailable = 1;
    input.runtimeThingReceipt = &projectile;
    if (!DM1_V1_ChampionMirror_BuildRuntimeRenderDecisionPc34(
            &input, &decision) || !decision.valid ||
        !decision.drawFrontWallOverlay ||
        !decision.drawChampionPortraitAsWallOverlay ||
        decision.drawFloorObject || !decision.drawRuntimeProjectile ||
        !decision.suppressMaterializedItemPayload ||
        !decision.suppressMirrorAsFloorItem ||
        !decision.suppressMirrorAsProjectile ||
        !decision.suppressMirrorAsSpellEffect ||
        !decision.hostDraw.drawMirrorBackingAsset ||
        decision.hostDraw.drawMirrorBackingFallbackRect) {
        return 1;
    }
    puts("ok: DM1 C127 mirror remains a wall overlay across F0115 projectile pass");
    return 0;
}
