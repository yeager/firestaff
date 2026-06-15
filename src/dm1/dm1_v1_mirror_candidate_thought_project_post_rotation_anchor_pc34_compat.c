#include "dm1_v1_mirror_candidate_thought_project_post_rotation_anchor_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors for this contract_only=1 runtime regression:
 * CHAMPION.C:F0284_CHAMPION_SetPartyDirection:93-131 computes the clockwise
 * delta, rotates every champion Cell/Direction with M021_NORMALIZE, stores
 * G0308_i_PartyDirection, and redraws changed object icons.
 * DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:
 * 4817-4868 converts each DEFS.H view cell ordinal to an absolute cell with
 * M021_NORMALIZE(viewCell + direction) before selecting the object bitmap.
 * DUNVIEW.C:F0122_DUNGEONVIEW_DrawSquareD1L and adjacent D1 center dispatch:
 * 7489-7524, 7658-7691, 7843-7875 keep D1 object drawing in a stable depth
 * slot while changing the ordered cell nibbles passed to F0115.
 * DEFS.H:2642-2677 names the front/back view cells and C0x0218/C0x0128/
 * C0x0032/C0x0041 cell-order zones used by the DUNVIEW object pass.
 */

static const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat s_evidence = {
    1,
    "CHAMPION.C:F0284_CHAMPION_SetPartyDirection:93-131",
    "DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4817-4868",
    "DUNVIEW.C:F0122/F0123/D1C object dispatch:7489-7524,7658-7691,7843-7875",
    "DEFS.H:2642-2677 view cells and cell-order zones",
    "contract_only=1 post-rotation thought-project anchor re-resolve; "
    "front-of-party cell must be recomputed after F0284-style rotation"
};

static int normalize_direction(int direction)
{
    direction %= 4;
    if (direction < 0) {
        direction += 4;
    }
    return direction;
}

static int rotate_cell(int cell, int delta)
{
    return normalize_direction(cell + delta);
}

static void front_delta(int direction, int *dx, int *dy)
{
    static const int kDx[4] = { 0, 1, 0, -1 };
    static const int kDy[4] = { -1, 0, 1, 0 };
    int normalized = normalize_direction(direction);

    *dx = kDx[normalized];
    *dy = kDy[normalized];
}

static void resolve_front(const Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
                          int direction,
                          int *mapX,
                          int *mapY)
{
    int dx = 0;
    int dy = 0;

    front_delta(direction, &dx, &dy);
    *mapX = state->partyMapX + dx;
    *mapY = state->partyMapY + dy;
}

static void reset_overlay(Dm1V1MirrorCandidatePostRotationOverlayPc34Compat *overlay)
{
    memset(overlay, 0, sizeof(*overlay));
    overlay->mapX = -1;
    overlay->mapY = -1;
}

static void snapshot_begin(
    const Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    if (!state) {
        return;
    }
    result->partyDirectionBefore = state->partyDirection;
    result->partyDirectionAfter = state->partyDirection;
    resolve_front(state, state->partyDirection,
                  &result->expectedFrontBeforeX,
                  &result->expectedFrontBeforeY);
    result->expectedFrontAfterX = result->expectedFrontBeforeX;
    result->expectedFrontAfterY = result->expectedFrontBeforeY;
    result->anchorBeforeX = state->anchorMapX;
    result->anchorBeforeY = state->anchorMapY;
    result->anchorAfterX = state->anchorMapX;
    result->anchorAfterY = state->anchorMapY;
    result->originalAnchorX = state->originalAnchorMapX;
    result->originalAnchorY = state->originalAnchorMapY;
    result->bitmapBefore = state->thoughtBitmapId;
    result->bitmapAfter = state->thoughtBitmapId;
    result->depthBefore = state->depthSlot;
    result->depthAfter = state->depthSlot;
    result->cellOrderBefore = state->cellOrder;
    result->cellOrderAfter = state->cellOrder;
    result->transformActiveBefore = state->crossChampionTransformActive;
    result->transformActiveAfter = state->crossChampionTransformActive;
    result->transformSourceCellBefore = state->transformSourceCell;
    result->transformSourceCellAfter = state->transformSourceCell;
    result->transformTargetCellBefore = state->transformTargetCell;
    result->transformTargetCellAfter = state->transformTargetCell;
    result->transformSourceDirectionBefore = state->transformSourceDirection;
    result->transformSourceDirectionAfter = state->transformSourceDirection;
    result->transformTargetDirectionBefore = state->transformTargetDirection;
    result->transformTargetDirectionAfter = state->transformTargetDirection;
    result->projectDispatchCountBefore = state->projectDispatchCount;
    result->projectDispatchCountAfter = state->projectDispatchCount;
    result->rotationDispatchCountBefore = state->rotationDispatchCount;
    result->rotationDispatchCountAfter = state->rotationDispatchCount;
    result->overlayDrawCountBefore = state->overlayDrawCount;
    result->overlayDrawCountAfter = state->overlayDrawCount;
    result->cancelDispatchCountBefore = state->cancelDispatchCount;
    result->cancelDispatchCountAfter = state->cancelDispatchCount;
    result->activeBefore = state->thoughtProjectActive;
    result->activeAfter = state->thoughtProjectActive;
    result->overlayBefore = state->overlay;
    result->overlayAfter = state->overlay;
}

static void snapshot_finish(
    const Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    int delta,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *result)
{
    int frontX = 0;
    int frontY = 0;

    if (!state || !result) {
        return;
    }
    result->partyDirectionAfter = state->partyDirection;
    resolve_front(state, state->partyDirection, &frontX, &frontY);
    result->expectedFrontAfterX = frontX;
    result->expectedFrontAfterY = frontY;
    result->anchorAfterX = state->anchorMapX;
    result->anchorAfterY = state->anchorMapY;
    result->bitmapAfter = state->thoughtBitmapId;
    result->depthAfter = state->depthSlot;
    result->cellOrderAfter = state->cellOrder;
    result->transformActiveAfter = state->crossChampionTransformActive;
    result->transformSourceCellAfter = state->transformSourceCell;
    result->transformTargetCellAfter = state->transformTargetCell;
    result->transformSourceDirectionAfter = state->transformSourceDirection;
    result->transformTargetDirectionAfter = state->transformTargetDirection;
    result->transformDelta = delta;
    result->transformFollowsRotation =
        state->crossChampionTransformActive &&
        result->transformActiveBefore &&
        result->transformSourceCellAfter ==
            rotate_cell(result->transformSourceCellBefore, delta) &&
        result->transformTargetCellAfter ==
            rotate_cell(result->transformTargetCellBefore, delta) &&
        result->transformSourceDirectionAfter ==
            rotate_cell(result->transformSourceDirectionBefore, delta) &&
        result->transformTargetDirectionAfter ==
            rotate_cell(result->transformTargetDirectionBefore, delta);
    result->projectDispatchCountAfter = state->projectDispatchCount;
    result->rotationDispatchCountAfter = state->rotationDispatchCount;
    result->overlayDrawCountAfter = state->overlayDrawCount;
    result->cancelDispatchCountAfter = state->cancelDispatchCount;
    result->activeAfter = state->thoughtProjectActive;
    result->overlayAfter = state->overlay;
    result->oldAnchorStillActive =
        state->thoughtProjectActive &&
        state->anchorMapX == state->originalAnchorMapX &&
        state->anchorMapY == state->originalAnchorMapY;
    result->newAnchorIsCurrentFront =
        state->thoughtProjectActive &&
        state->anchorMapX == frontX &&
        state->anchorMapY == frontY;
}

void DM1_V1_MirrorCandidateThoughtProjectPostRotation_InitPc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state)
{
    unsigned int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyMapX = 10;
    state->partyMapY = 10;
    state->partyDirection =
        DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_NORTH_PC34_COMPAT;
    state->partyChampionCount =
        DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_PARTY_CHAMPIONS_PC34_COMPAT;
    state->anchorMapX = -1;
    state->anchorMapY = -1;
    state->originalAnchorMapX = -1;
    state->originalAnchorMapY = -1;
    state->anchorDirection = state->partyDirection;
    state->thoughtBitmapId =
        DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_BITMAP_PC34_COMPAT;
    state->depthSlot =
        DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_DEPTH_D1C_PC34_COMPAT;
    state->cellOrder =
        DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_CELL_ORDER_D1C_PC34_COMPAT;
    state->crossChampionTransformActive = 1;
    state->transformSourceOrdinal = 3;
    state->transformTargetOrdinal = 4;
    state->transformSourceCell =
        DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_FRONT_RIGHT_PC34_COMPAT;
    state->transformTargetCell =
        DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_BACK_LEFT_PC34_COMPAT;
    state->transformSourceDirection = state->partyDirection;
    state->transformTargetDirection =
        DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_EAST_PC34_COMPAT;
    for (i = 0; i < state->partyChampionCount; ++i) {
        state->champions[i].ordinal = i + 1u;
        state->champions[i].cell = (int)i;
        state->champions[i].direction = state->partyDirection;
    }
    reset_overlay(&state->overlay);
}

int DM1_V1_MirrorCandidateThoughtProjectPostRotation_ProjectInFrontPc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *outResult)
{
    int frontX = 0;
    int frontY = 0;

    if (!state || !outResult || !state->contractOnly) {
        return 0;
    }
    snapshot_begin(state, outResult);
    resolve_front(state, state->partyDirection, &frontX, &frontY);
    state->thoughtProjectActive = 1;
    state->anchorMapX = frontX;
    state->anchorMapY = frontY;
    state->originalAnchorMapX = frontX;
    state->originalAnchorMapY = frontY;
    state->anchorDirection = state->partyDirection;
    ++state->projectDispatchCount;
    snapshot_finish(state, 0, outResult);
    return outResult->newAnchorIsCurrentFront;
}

int DM1_V1_MirrorCandidateThoughtProjectPostRotation_RotateClockwisePc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *outResult)
{
    unsigned int i;
    int oldDirection;
    int newDirection;
    int delta;

    if (!state || !outResult || !state->contractOnly) {
        return 0;
    }
    snapshot_begin(state, outResult);
    oldDirection = state->partyDirection;
    newDirection = normalize_direction(oldDirection + 1);
    delta = normalize_direction(newDirection - oldDirection);
    for (i = 0; i < state->partyChampionCount; ++i) {
        state->champions[i].cell = rotate_cell(state->champions[i].cell, delta);
        state->champions[i].direction =
            rotate_cell(state->champions[i].direction, delta);
    }
    state->partyDirection = newDirection;
    state->anchorDirection = newDirection;
    if (state->thoughtProjectActive) {
        resolve_front(state, newDirection, &state->anchorMapX, &state->anchorMapY);
    }
    if (state->crossChampionTransformActive) {
        state->transformSourceCell = rotate_cell(state->transformSourceCell, delta);
        state->transformTargetCell = rotate_cell(state->transformTargetCell, delta);
        state->transformSourceDirection =
            rotate_cell(state->transformSourceDirection, delta);
        state->transformTargetDirection =
            rotate_cell(state->transformTargetDirection, delta);
        state->transformDelta = delta;
    }
    ++state->rotationDispatchCount;
    snapshot_finish(state, delta, outResult);
    return outResult->newAnchorIsCurrentFront;
}

int DM1_V1_MirrorCandidateThoughtProjectPostRotation_RenderOverlayPc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *outResult)
{
    int frontX = 0;
    int frontY = 0;

    if (!state || !outResult || !state->contractOnly) {
        return 0;
    }
    snapshot_begin(state, outResult);
    reset_overlay(&state->overlay);
    resolve_front(state, state->partyDirection, &frontX, &frontY);
    if (state->thoughtProjectActive &&
        state->anchorMapX == frontX &&
        state->anchorMapY == frontY) {
        state->overlay.visible = 1;
        state->overlay.mapX = state->anchorMapX;
        state->overlay.mapY = state->anchorMapY;
        state->overlay.bitmapId = state->thoughtBitmapId;
        state->overlay.depthSlot = state->depthSlot;
        state->overlay.cellOrder = state->cellOrder;
        state->overlay.viewCellLeft =
            DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_BACK_LEFT_PC34_COMPAT;
        state->overlay.viewCellRight =
            DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_BACK_RIGHT_PC34_COMPAT;
        ++state->overlayDrawCount;
    }
    snapshot_finish(state, 0, outResult);
    return state->overlay.visible;
}

int DM1_V1_MirrorCandidateThoughtProjectPostRotation_CancelAtCellPc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    int mapX,
    int mapY,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *outResult)
{
    if (!state || !outResult || !state->contractOnly) {
        return 0;
    }
    snapshot_begin(state, outResult);
    if (state->thoughtProjectActive &&
        state->anchorMapX == mapX &&
        state->anchorMapY == mapY) {
        state->thoughtProjectActive = 0;
        state->anchorMapX = -1;
        state->anchorMapY = -1;
        state->anchorDirection = -1;
        ++state->cancelDispatchCount;
        reset_overlay(&state->overlay);
        outResult->cancelAccepted = 1;
    } else if (mapX == state->originalAnchorMapX &&
               mapY == state->originalAnchorMapY) {
        ++state->staleOriginalCancelCount;
        outResult->cancelRejectedOriginal = 1;
    }
    snapshot_finish(state, 0, outResult);
    return outResult->cancelAccepted;
}

int DM1_V1_MirrorCandidateThoughtProjectPostRotation_CancelInFrontPc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *outResult)
{
    int frontX = 0;
    int frontY = 0;

    if (!state) {
        return 0;
    }
    resolve_front(state, state->partyDirection, &frontX, &frontY);
    return DM1_V1_MirrorCandidateThoughtProjectPostRotation_CancelAtCellPc34Compat(
        state, frontX, frontY, outResult);
}

const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat *
DM1_V1_MirrorCandidateThoughtProjectPostRotation_EvidencePc34Compat(void)
{
    return &s_evidence;
}
