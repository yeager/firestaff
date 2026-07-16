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
    input.relativeForward = 1;
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

static int verify_non_d0c_c15_order(int depth, int expectedViewSquare,
                                    int expectedRow)
{
    DM1_V1_ViewportRuntimeMaterializationInputPc34 input;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 decision;
    struct ExplosionList_Compat explosions;
    const int types[] = {
        C000_EXPLOSION_FIREBALL,
        C050_EXPLOSION_FLUXCAGE,
        C100_EXPLOSION_REBIRTH_STEP1,
        C101_EXPLOSION_REBIRTH_STEP2,
        C000_EXPLOSION_FIREBALL
    };
    int i;

    memset(&input, 0, sizeof(input));
    memset(&decision, 0, sizeof(decision));
    memset(&explosions, 0, sizeof(explosions));
    input.relativeForward = depth;
    input.relativeSide = 0;
    input.elementType = 1;
    input.mapIndex = 2;
    input.mapX = 11;
    input.mapY = 12;
    input.liveExplosions = &explosions;
    input.runtimeOrigin = DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34;
    explosions.count = (int)(sizeof(types) / sizeof(types[0]));
    for (i = 0; i < explosions.count; ++i) {
        explosions.entries[i].slotIndex = 111 + i;
        explosions.entries[i].reserved0 = 1;
        explosions.entries[i].explosionType = types[i];
        explosions.entries[i].mapIndex = input.mapIndex;
        explosions.entries[i].mapX = input.mapX;
        explosions.entries[i].mapY = input.mapY;
        explosions.entries[i].currentFrame = i;
        explosions.entries[i].maxFrames = 5;
        explosions.entries[i].attack = 32 * (i + 1);
    }
    if (!dm1_v1_viewport_runtime_materialization_decide_pc34(&input,
                                                               &decision) ||
        !decision.valid || decision.viewSquare != expectedViewSquare ||
        decision.row != expectedRow || decision.liveExplosionCount != 5 ||
        decision.liveRenderableExplosionCount != 5 ||
        decision.liveRenderableExplosionTypes[0] != C000_EXPLOSION_FIREBALL ||
        decision.liveRenderableExplosionTypes[1] != C050_EXPLOSION_FLUXCAGE ||
        decision.liveRenderableExplosionTypes[2] !=
            C100_EXPLOSION_REBIRTH_STEP1 ||
        decision.liveRenderableExplosionTypes[3] !=
            C101_EXPLOSION_REBIRTH_STEP2 ||
        decision.liveRenderableExplosionTypes[4] != C000_EXPLOSION_FIREBALL ||
        decision.liveRenderableExplosionFrames[4] != 4 ||
        decision.liveRenderableExplosionAttacks[0] != 32) {
        fprintf(stderr, "non-D0C C15 order changed at depth %d\n", depth);
        return 0;
    }
    return 1;
}

int main(void)
{
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 allEffects;
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 noFluxcage;

    /* The current runtime decision API starts at D1C, where F0115 receives
     * the active C15 effect sequence. */
    if (!build_mixed_d0c_receipt(0, &allEffects) ||
        !build_mixed_d0c_receipt(1, &noFluxcage)) {
        fprintf(stderr, "D0C receipt audit could not build current M10 state\n");
        return 1;
    }

    /* The runtime receipt preserves the active D1C source order and lets the
     * consumer choose the source-specific draw route. */
    if (!allEffects.valid || allEffects.viewSquare != 3 || allEffects.row != 8 ||
        allEffects.liveExplosionCount != 4 ||
        allEffects.liveRenderableExplosionCount != 4 ||
        allEffects.liveRenderableExplosionTypes[0] !=
            C100_EXPLOSION_REBIRTH_STEP1 ||
        allEffects.liveRenderableExplosionTypes[1] !=
            C101_EXPLOSION_REBIRTH_STEP2 ||
        allEffects.liveRenderableExplosionTypes[2] !=
            C050_EXPLOSION_FLUXCAGE ||
        allEffects.liveRenderableExplosionTypes[3] != C000_EXPLOSION_FIREBALL ||
        !noFluxcage.valid || noFluxcage.liveExplosionCount != 3 ||
        noFluxcage.liveRenderableExplosionCount != 3 ||
        noFluxcage.liveRenderableExplosionTypes[0] !=
            C100_EXPLOSION_REBIRTH_STEP1 ||
        noFluxcage.liveRenderableExplosionTypes[1] !=
            C101_EXPLOSION_REBIRTH_STEP2 ||
        noFluxcage.liveRenderableExplosionTypes[2] != C000_EXPLOSION_FIREBALL) {
        fprintf(stderr,
                "D0C C15 source receipt changed: all=%d/%d/%d/%d noFlux=%d/%d/%d/%d\n",
                allEffects.valid, allEffects.viewSquare, allEffects.row,
                allEffects.liveExplosionCount, noFluxcage.valid,
                noFluxcage.viewSquare, noFluxcage.row,
                noFluxcage.liveExplosionCount);
        return 1;
    }

    /* Generic pattern lookup remains independent of the runtime receipt. */
    if (dm1_v1_explosion_pattern_graphic_index(
            DM1_EXPLOSION_TYPE_REBIRTH_STEP1, 160) != -1 ||
        dm1_v1_explosion_pattern_graphic_index(
            DM1_EXPLOSION_FLUXCAGE, 160) != -1 ||
        dm1_v1_explosion_pattern_graphic_index(
            DM1_EXPLOSION_TYPE_REBIRTH_STEP2, 160) != 491) {
        fprintf(stderr, "current generic pattern route changed; re-audit D0C exclusion\n");
        return 1;
    }

    /* ReDMCSB DUNVIEW.C F0115:5915-6120 restarts C15 for every presented
     * square. D1/D2/D3 retain ordinary C000 source order through F0114;
     * C050, C100, and C101 retain their source-owned non-ordinary routes. */
    if (!verify_non_d0c_c15_order(1, 3, 8) ||
        !verify_non_d0c_c15_order(2, 6, 5) ||
        !verify_non_d0c_c15_order(3, 11, 0)) {
        return 1;
    }

    puts("ok: D0C ordinary C15 receipt keeps C100/C050 out of M636");
    return 0;
}
