#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int build_mixed_d0c_receipt(
    int suppressFluxcages,
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34* outDecision)
{
    DM1_V1_ViewportRuntimeMaterializationInputPc34 input;
    struct ExplosionList_Compat explosions;
    const int types[] = {
        C100_EXPLOSION_REBIRTH_STEP1,
        C101_EXPLOSION_REBIRTH_STEP2,
        C050_EXPLOSION_FLUXCAGE,
        C000_EXPLOSION_FIREBALL
    };
    int i;

    memset(&input, 0, sizeof(input));
    memset(&explosions, 0, sizeof(explosions));
    input.relativeForward = 0;
    input.relativeSide = 0;
    input.elementType = 1;
    input.mapIndex = 2;
    input.mapX = 11;
    input.mapY = 12;
    input.suppressFluxcages = suppressFluxcages;
    input.liveExplosions = &explosions;
    input.runtimeOrigin = DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34;
    explosions.count = (int)(sizeof(types) / sizeof(types[0]));
    for (i = 0; i < explosions.count; ++i) {
        explosions.entries[i].slotIndex = 31 + i;
        explosions.entries[i].reserved0 = 1;
        explosions.entries[i].explosionType = types[i];
        explosions.entries[i].mapIndex = input.mapIndex;
        explosions.entries[i].mapX = input.mapX;
        explosions.entries[i].mapY = input.mapY;
        explosions.entries[i].maxFrames = 4;
        explosions.entries[i].attack = 160;
    }
    return dm1_v1_viewport_runtime_materialization_decide_pc34(&input,
                                                                 outDecision);
}

int main(void)
{
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 allEffects;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 noFluxcage;

    /* ReDMCSB DUNVIEW.C F0115:5932-6074: C100 takes C3000, C050 remains
     * F0113-owned, and D0C admits C101/C000 through its M636 pass. */
    if (!build_mixed_d0c_receipt(0, &allEffects) ||
        !build_mixed_d0c_receipt(1, &noFluxcage)) {
        fprintf(stderr, "D0C receipt audit could not build current M10 state\n");
        return 1;
    }

    /* ReDMCSB DUNVIEW.C F0115:5955-6074 owns D0C's M636 admission: C101
     * and ordinary C000 remain renderable, while C100 stays C3000-blocked
     * and C050 retains F0113 ownership. */
    if (!allEffects.valid || allEffects.viewSquare != 0 || allEffects.row != 11 ||
        allEffects.liveExplosionCount != 4 || allEffects.liveExplosionSourceCount != 4 ||
        allEffects.liveExplosionSourceRoutes[0] !=
            DM1_V1_C15_EXPLOSION_ROUTE_C100_C3000_BLOCKED_PC34 ||
        allEffects.liveExplosionSourceRoutes[1] !=
            DM1_V1_C15_EXPLOSION_ROUTE_C101_D0C_M636_PC34 ||
        allEffects.liveExplosionSourceRoutes[2] !=
            DM1_V1_C15_EXPLOSION_ROUTE_FLUXCAGE_F0113_PC34 ||
        allEffects.liveExplosionSourceRoutes[3] !=
            DM1_V1_C15_EXPLOSION_ROUTE_ORDINARY_D0C_M636_PC34 ||
        allEffects.liveRenderableExplosionCount != 2 ||
        allEffects.liveRenderableExplosionSlots[0] != 32 ||
        allEffects.liveRenderableExplosionSlots[1] != 34 ||
        !noFluxcage.valid || noFluxcage.liveExplosionCount != 3 ||
        noFluxcage.liveExplosionSourceCount != 3 ||
        noFluxcage.liveRenderableExplosionCount != 2) {
        fprintf(stderr,
                "D0C C15 source receipt changed: all=%d/%d/%d/%d noFlux=%d/%d/%d/%d\n",
                allEffects.valid, allEffects.viewSquare, allEffects.row,
                allEffects.liveExplosionCount, noFluxcage.valid,
                noFluxcage.viewSquare, noFluxcage.row,
                noFluxcage.liveExplosionCount);
        return 1;
    }

    /* The generic pattern lookup is not an admission gate: C101's fire art
     * becomes legal only through the D0C M636 route above. */
    if (dm1_v1_explosion_pattern_graphic_index(
            DM1_EXPLOSION_TYPE_REBIRTH_STEP1, 160) != -1 ||
        dm1_v1_explosion_pattern_graphic_index(
            DM1_EXPLOSION_FLUXCAGE, 160) != -1 ||
        dm1_v1_explosion_pattern_graphic_index(
            DM1_EXPLOSION_TYPE_REBIRTH_STEP2, 160) != 491) {
        fprintf(stderr, "current generic pattern route changed; re-audit D0C exclusion\n");
        return 1;
    }

    puts("ok: D0C ordinary C15 receipt keeps C100/C050 out of M636");
    return 0;
}
