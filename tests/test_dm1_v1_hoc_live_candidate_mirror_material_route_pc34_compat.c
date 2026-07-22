#include "dm1_v1_hoc_live_candidate_mirror_material_route_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

static void mirror_decision(DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 *decision)
{
    memset(decision, 0, sizeof(*decision));
    decision->valid = 1; decision->consumedF0172Sensor = 1;
    decision->drawFrontWallOverlay = 1; decision->drawChampionPortraitAsWallOverlay = 1;
    decision->suppressHostFallbackVisuals = 1;
    decision->frontWall.valid = 1; decision->frontWall.isFrontMirror = 1;
    decision->frontWall.championPortraitOrdinal = 4;
    decision->frontWall.championPortraitRenderIndex = 3;
    decision->render.valid = 1; decision->render.drawChampionPortrait = 1;
    decision->render.renderIndex = 3; decision->render.sourceX = 96;
    decision->render.sourceY = 0; decision->render.width = 32; decision->render.height = 29;
    decision->render.dstX = 96; decision->render.dstY = 35;
    decision->hostDraw.valid = 1; decision->hostDraw.sourceAssetsBound = 1;
    decision->hostDraw.sourceOwnedExecution = 1;
    decision->hostDraw.suppressHostFallbackVisuals = 1;
}

static void candidate_receipt(DM1_V1_HocCandidateClickPresentationReceiptPc34 *candidate)
{
    memset(candidate, 0, sizeof(*candidate));
    candidate->valid = 1; candidate->sourceOwned = 1; candidate->portraitHit = 1;
    candidate->consumedC127Sensor = 1; candidate->opensCandidatePanel = 1;
    candidate->drawC026Portrait = 1; candidate->drawC040Panel = 1;
    candidate->mirrorOrdinal = 3; candidate->candidateChampionOrdinal = 4;
    candidate->panelGeneration = 18;
    candidate->panel.valid = 1; candidate->panel.sourceOwned = 1;
    candidate->panel.redrawC026Portrait = 1; candidate->panel.redrawC040Panel = 1;
    candidate->panel.mirrorOrdinal = 3; candidate->panel.nextPanelGeneration = 18;
}

int main(void)
{
    DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 mirror;
    DM1_V1_HocCandidateClickPresentationReceiptPc34 candidate;
    DM1_V1_HocLiveCandidateMirrorMaterialRouteInputPc34 input;
    DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34 receipt;
    int ok = 1;

    mirror_decision(&mirror); candidate_receipt(&candidate);
    memset(&input, 0, sizeof(input));
    input.mirrorDecision = &mirror; input.candidatePresentation = &candidate;
    input.c127.sourceOwned = 1; input.c127.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    input.c127.sensorData = 3; input.c127.sensorCell = 7; input.c127.visibleWallCell = 7;
    input.c127.sensorGeneration = 21;
    input.c026.sourceOwned = 1; input.c026.graphicIndex = 26;
    input.c026.sourceX = 96; input.c026.sourceY = 0; input.c026.width = 32;
    input.c026.height = 29; input.c026.dstX = 96; input.c026.dstY = 35;
    input.c026.sourceHash = 0x2626u;
    input.c040.sourceOwned = 1; input.c040.graphicIndex = 40;
    input.c040.width = 144; input.c040.height = 73; input.c040.sourceHash = 0x4040u;

    ok &= expect(DM1_V1_HocLiveCandidateMirrorMaterialRoute_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.consumedC127 && receipt.admitC026Portrait &&
                 receipt.admitC040Panel && receipt.suppressFallbackVisuals &&
                 receipt.mirrorOrdinal == 3 && receipt.c026.sourceHash == 0x2626u,
                 "live C127/C026/C040 source evidence admits candidate mirror route");

    input.c026.sourceX = 64;
    ok &= expect(DM1_V1_HocLiveCandidateMirrorMaterialRoute_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "mismatched original C026 rectangle fails closed without fallback");

    if (!ok) return 1;
    puts("ok: DM1 HoC live candidate mirror route requires C127/C026/C040 evidence");
    return 0;
}
