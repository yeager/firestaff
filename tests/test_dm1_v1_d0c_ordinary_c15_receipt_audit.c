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

    /* ReDMCSB DUNVIEW.C F0115:5932-6074: C100/C101 use C3000/C3007
     * only on D3C..D1C, while C050 is retained for F0113 field drawing.
     * D0C must therefore not promote any of these to its ordinary M636 C15
     * presentation. This test audits the current receipt rather than claiming
     * that the absent M11 D0C consumer already implements the source rule. */
    if (!build_mixed_d0c_receipt(0, &allEffects) ||
        !build_mixed_d0c_receipt(1, &noFluxcage)) {
        fprintf(stderr, "D0C receipt audit could not build current M10 state\n");
        return 1;
    }

    /* Current HEAD has no D0C M10 receipt at all: its F0115 row mapper
     * admits only D1..D3. That is the exact reason C100/C101/C050 cannot be
     * proven excluded from an ordinary D0C presenter yet. */
    if (allEffects.valid || allEffects.viewSquare != -1 || allEffects.row != -1 ||
        allEffects.liveExplosionCount != 0 || allEffects.liveExplosionSlot != -1 ||
        allEffects.liveExplosionType != -1 || noFluxcage.valid ||
        noFluxcage.liveExplosionCount != 0 || noFluxcage.liveExplosionSlot != -1 ||
        noFluxcage.liveExplosionType != -1) {
        fprintf(stderr,
                "current D0C receipt audit changed: all=%d/%d/%d/%d noFlux=%d/%d/%d/%d\n",
                allEffects.valid, allEffects.viewSquare, allEffects.row,
                allEffects.liveExplosionCount, noFluxcage.valid,
                noFluxcage.viewSquare, noFluxcage.row,
                noFluxcage.liveExplosionCount);
        return 1;
    }

    /* The generic pattern lookup is not a D0C admission gate: C100 and C050
     * reject there, but C101 maps to fire art and needs the missing view-square
     * ownership rule to stay out of D0C. */
    if (dm1_v1_explosion_pattern_graphic_index(
            DM1_EXPLOSION_TYPE_REBIRTH_STEP1, 160) != -1 ||
        dm1_v1_explosion_pattern_graphic_index(
            DM1_EXPLOSION_FLUXCAGE, 160) != -1 ||
        dm1_v1_explosion_pattern_graphic_index(
            DM1_EXPLOSION_TYPE_REBIRTH_STEP2, 160) != 491) {
        fprintf(stderr, "current generic pattern route changed; re-audit D0C exclusion\n");
        return 1;
    }

    puts("ok: current HEAD lacks a source-owned D0C ordinary-C15 receipt");
    return 0;
}
