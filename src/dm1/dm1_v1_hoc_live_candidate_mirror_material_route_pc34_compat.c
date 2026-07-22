#include "dm1_v1_hoc_live_candidate_mirror_material_route_pc34_compat.h"

#include <string.h>

static int material_valid(const DM1_V1_HocLiveMaterialEvidencePc34 *material,
                          int graphicIndex)
{
    return material && material->sourceOwned && material->graphicIndex == graphicIndex &&
           material->width > 0 && material->height > 0 && material->sourceHash != 0u;
}

static int c026_matches_mirror(const DM1_V1_HocLiveMaterialEvidencePc34 *material,
                               const DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 *mirror)
{
    return material_valid(material, DM1_V1_CHAMPION_MIRROR_PORTRAIT_GRAPHIC_PC34_COMPAT) &&
           material->sourceX == mirror->render.sourceX &&
           material->sourceY == mirror->render.sourceY &&
           material->width == mirror->render.width &&
           material->height == mirror->render.height &&
           material->dstX == mirror->render.dstX &&
           material->dstY == mirror->render.dstY;
}

int DM1_V1_HocLiveCandidateMirrorMaterialRoute_BuildReceiptPc34(
    const DM1_V1_HocLiveCandidateMirrorMaterialRouteInputPc34 *input,
    DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34 *outReceipt)
{
    DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34 receipt;
    const DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 *mirror;
    const DM1_V1_HocCandidateClickPresentationReceiptPc34 *candidate;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB DUNGEON.C F0172/F0174 C127; REVIVE.C F0280; PANEL.C "
        "F0344-F0352 C040; DUNVIEW.C:3913-3928 C026 live source route";
    mirror = input->mirrorDecision;
    candidate = input->candidatePresentation;
    if (!mirror || !candidate || !mirror->valid || !mirror->consumedF0172Sensor ||
        !mirror->drawFrontWallOverlay || !mirror->drawChampionPortraitAsWallOverlay ||
        !mirror->suppressHostFallbackVisuals || !mirror->frontWall.valid ||
        !mirror->frontWall.isFrontMirror || !mirror->render.valid ||
        !mirror->render.drawChampionPortrait || !mirror->hostDraw.valid ||
        !mirror->hostDraw.sourceAssetsBound || !mirror->hostDraw.sourceOwnedExecution ||
        mirror->hostDraw.drawMirrorBackingFallbackRect ||
        !mirror->hostDraw.suppressHostFallbackVisuals || !candidate->valid ||
        !candidate->sourceOwned || !candidate->portraitHit ||
        !candidate->consumedC127Sensor || !candidate->opensCandidatePanel ||
        !candidate->drawC026Portrait || !candidate->drawC040Panel ||
        !candidate->panel.valid || !candidate->panel.sourceOwned ||
        !candidate->panel.redrawC026Portrait || !candidate->panel.redrawC040Panel ||
        !input->c127.sourceOwned ||
        input->c127.sensorType != DM1_SENSOR_WALL_CHAMPION_PORTRAIT ||
        input->c127.sensorCell != input->c127.visibleWallCell ||
        input->c127.sensorData < 0 ||
        input->c127.sensorData != mirror->frontWall.championPortraitRenderIndex ||
        input->c127.sensorData != mirror->render.renderIndex ||
        input->c127.sensorData != (int)candidate->mirrorOrdinal ||
        mirror->frontWall.championPortraitOrdinal != input->c127.sensorData + 1 ||
        candidate->panel.mirrorOrdinal != candidate->mirrorOrdinal ||
        candidate->panel.nextPanelGeneration != candidate->panelGeneration ||
        !c026_matches_mirror(&input->c026, mirror) || !material_valid(&input->c040, 40)) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.consumedC127 = 1;
    receipt.admitC026Portrait = 1;
    receipt.admitC040Panel = 1;
    receipt.suppressFallbackVisuals = 1;
    receipt.mirrorOrdinal = candidate->mirrorOrdinal;
    receipt.candidateChampionOrdinal = candidate->candidateChampionOrdinal;
    receipt.sensorGeneration = input->c127.sensorGeneration;
    receipt.panelGeneration = candidate->panelGeneration;
    receipt.c026 = input->c026;
    receipt.c040 = input->c040;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocLiveCandidateMirrorMaterialRoute_SourceEvidencePc34(void)
{
    return "ReDMCSB DUNGEON.C F0172/F0174 consumes real C127 sensor data; "
           "REVIVE.C/PANEL.C publish the candidate C040 state; DUNVIEW.C 3913-3928 "
           "consumes the matching original C026 atlas rectangle with no host fallback.";
}
