#include "dm1_v1_mirror_candidate_thought_project_post_rotation_anchor_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gAssertions;
static int gFailures;

static void check_anchor(int condition, const char *message, const char *anchor)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
        printf("FAIL: %s [%s]\n", message, anchor);
    }
}

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%d expected=%d [%s]\n",
               message, actual, expected, anchor);
    }
}

static void check_uint_eq(unsigned int actual, unsigned int expected,
                          const char *message, const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%u expected=%u [%s]\n",
               message, actual, expected, anchor);
    }
}

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_EvidencePc34Compat();

    check_anchor(e != NULL && e->contractOnly == 1,
                 "evidence accessor returns contract-only metadata",
                 "CHAMPION.C:F0284:93-131");
    check_anchor(strstr(e->rotationAnchor, "F0284") != NULL,
                 "rotation anchor names F0284",
                 e->rotationAnchor);
    check_anchor(strstr(e->rotationAnchor, "93-131") != NULL,
                 "rotation anchor cites approximate line range",
                 e->rotationAnchor);
    check_anchor(strstr(e->dungeonViewCellAnchor, "F0115") != NULL,
                 "dungeon view anchor names F0115 object pass",
                 e->dungeonViewCellAnchor);
    check_anchor(strstr(e->dungeonViewCellAnchor, "4817-4868") != NULL,
                 "dungeon view anchor cites cell-to-absolute mapping range",
                 e->dungeonViewCellAnchor);
    check_anchor(strstr(e->dungeonViewDepthAnchor, "7843-7875") != NULL,
                 "D1C depth dispatch range is cited",
                 e->dungeonViewDepthAnchor);
    check_anchor(strstr(e->defsCellAnchor, "2642-2677") != NULL,
                 "DEFS.H view cell and order range is cited",
                 e->defsCellAnchor);
    check_anchor(strstr(e->contractScope, "post-rotation") != NULL,
                 "contract scope names post-rotation anchor behavior",
                 e->contractScope);
    check_anchor(strstr(e->contractScope, "front-of-party") != NULL,
                 "contract scope says front-of-party cell is recomputed",
                 e->contractScope);
    check_uint_eq(DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_CELL_ORDER_D1C_PC34_COMPAT,
                  0x0218u,
                  "test uses DEFS.H C0x0218 D1C cell-order zone",
                  e->defsCellAnchor);
}

static void test_initial_project_is_front_tile(void)
{
    Dm1V1MirrorCandidatePostRotationStatePc34Compat state;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat project;
    const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectPostRotation_InitPc34Compat(&state);
    check_int_eq(state.partyMapX, 10, "fixture party X is stable",
                 e->dungeonViewCellAnchor);
    check_int_eq(state.partyMapY, 10, "fixture party Y is stable",
                 e->dungeonViewCellAnchor);
    check_int_eq(state.partyDirection,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_NORTH_PC34_COMPAT,
                 "fixture starts facing north",
                 e->rotationAnchor);
    check_int_eq(state.thoughtProjectActive, 0,
                 "fixture starts with no active thought-project tile",
                 e->contractScope);
    check_int_eq(state.anchorMapX, -1, "fixture has no pre-project anchor X",
                 e->contractScope);
    check_int_eq(state.anchorMapY, -1, "fixture has no pre-project anchor Y",
                 e->contractScope);
    check_int_eq(state.crossChampionTransformActive, 1,
                 "fixture has a cross-champion transform candidate",
                 e->rotationAnchor);
    check_int_eq(state.transformSourceCell,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_FRONT_RIGHT_PC34_COMPAT,
                 "source champion starts in the front-right view cell",
                 e->defsCellAnchor);
    check_int_eq(state.transformTargetCell,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_BACK_LEFT_PC34_COMPAT,
                 "target champion starts in the back-left view cell",
                 e->defsCellAnchor);

    check_anchor(
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_ProjectInFrontPc34Compat(
            &state, &project) == 1,
        "project command activates the thought-project tile in front",
        e->dungeonViewCellAnchor);

    check_int_eq(project.activeBefore, 0,
                 "project sees inactive thought-project before dispatch",
                 e->contractScope);
    check_int_eq(project.activeAfter, 1,
                 "project leaves active thought-project after dispatch",
                 e->contractScope);
    check_int_eq(project.expectedFrontBeforeX, 10,
                 "north-facing expected front X is party X",
                 e->dungeonViewCellAnchor);
    check_int_eq(project.expectedFrontBeforeY, 9,
                 "north-facing expected front Y is one row north",
                 e->dungeonViewCellAnchor);
    check_int_eq(project.anchorAfterX, 10,
                 "project anchor X is north-front cell",
                 e->dungeonViewCellAnchor);
    check_int_eq(project.anchorAfterY, 9,
                 "project anchor Y is north-front cell",
                 e->dungeonViewCellAnchor);
    check_int_eq(project.originalAnchorX, -1,
                 "original anchor snapshot was empty before first project",
                 e->contractScope);
    check_int_eq(state.originalAnchorMapX, 10,
                 "state records original anchor X after project",
                 e->contractScope);
    check_int_eq(state.originalAnchorMapY, 9,
                 "state records original anchor Y after project",
                 e->contractScope);
    check_uint_eq(project.bitmapAfter,
                  DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_BITMAP_PC34_COMPAT,
                  "project records thought-project bitmap",
                  e->dungeonViewCellAnchor);
    check_int_eq(project.depthAfter,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_DEPTH_D1C_PC34_COMPAT,
                 "project records D1C depth slot",
                 e->dungeonViewDepthAnchor);
    check_uint_eq(project.projectDispatchCountAfter,
                  project.projectDispatchCountBefore + 1u,
                  "project increments dispatch once",
                  e->contractScope);
}

static void test_overlay_before_rotation_uses_project_bitmap_and_depth(void)
{
    Dm1V1MirrorCandidatePostRotationStatePc34Compat state;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat project;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat overlay;
    const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectPostRotation_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateThoughtProjectPostRotation_ProjectInFrontPc34Compat(
        &state, &project);
    check_anchor(
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_RenderOverlayPc34Compat(
            &state, &overlay) == 1,
        "overlay renders while anchor is on the current front cell",
        e->dungeonViewCellAnchor);

    check_int_eq(overlay.overlayAfter.visible, 1,
                 "overlay is visible before rotation",
                 e->dungeonViewCellAnchor);
    check_int_eq(overlay.overlayAfter.mapX, project.anchorAfterX,
                 "overlay X matches projected anchor",
                 e->dungeonViewCellAnchor);
    check_int_eq(overlay.overlayAfter.mapY, project.anchorAfterY,
                 "overlay Y matches projected anchor",
                 e->dungeonViewCellAnchor);
    check_uint_eq(overlay.overlayAfter.bitmapId, project.bitmapAfter,
                  "overlay bitmap matches project bitmap before rotation",
                  e->dungeonViewCellAnchor);
    check_int_eq(overlay.overlayAfter.depthSlot, project.depthAfter,
                 "overlay depth matches project depth before rotation",
                 e->dungeonViewDepthAnchor);
    check_uint_eq(overlay.overlayAfter.cellOrder, project.cellOrderAfter,
                  "overlay cell order matches project cell order before rotation",
                  e->defsCellAnchor);
    check_int_eq(overlay.overlayAfter.viewCellLeft,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_BACK_LEFT_PC34_COMPAT,
                 "overlay exposes back-left D1C cell-order nibble",
                 e->defsCellAnchor);
    check_int_eq(overlay.overlayAfter.viewCellRight,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_BACK_RIGHT_PC34_COMPAT,
                 "overlay exposes back-right D1C cell-order nibble",
                 e->defsCellAnchor);
    check_uint_eq(overlay.overlayDrawCountAfter,
                  overlay.overlayDrawCountBefore + 1u,
                  "overlay draw count increments once",
                  e->dungeonViewDepthAnchor);
}

static void test_clockwise_rotation_re_resolves_anchor_to_new_front(void)
{
    Dm1V1MirrorCandidatePostRotationStatePc34Compat state;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat project;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat rotate;
    const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectPostRotation_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateThoughtProjectPostRotation_ProjectInFrontPc34Compat(
        &state, &project);
    check_anchor(
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_RotateClockwisePc34Compat(
            &state, &rotate) == 1,
        "clockwise rotation re-resolves active anchor to new front",
        e->rotationAnchor);

    check_int_eq(rotate.partyDirectionBefore,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_NORTH_PC34_COMPAT,
                 "rotation starts from north",
                 e->rotationAnchor);
    check_int_eq(rotate.partyDirectionAfter,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_EAST_PC34_COMPAT,
                 "rotation ends facing east",
                 e->rotationAnchor);
    check_int_eq(rotate.expectedFrontBeforeX, 10,
                 "old front X was north of party",
                 e->dungeonViewCellAnchor);
    check_int_eq(rotate.expectedFrontBeforeY, 9,
                 "old front Y was north of party",
                 e->dungeonViewCellAnchor);
    check_int_eq(rotate.expectedFrontAfterX, 11,
                 "new front X is one column east",
                 e->dungeonViewCellAnchor);
    check_int_eq(rotate.expectedFrontAfterY, 10,
                 "new front Y is party row after clockwise rotation",
                 e->dungeonViewCellAnchor);
    check_int_eq(rotate.anchorBeforeX, 10,
                 "anchor before rotation is old front X",
                 e->dungeonViewCellAnchor);
    check_int_eq(rotate.anchorBeforeY, 9,
                 "anchor before rotation is old front Y",
                 e->dungeonViewCellAnchor);
    check_int_eq(rotate.anchorAfterX, 11,
                 "anchor after rotation is new front X",
                 e->dungeonViewCellAnchor);
    check_int_eq(rotate.anchorAfterY, 10,
                 "anchor after rotation is new front Y",
                 e->dungeonViewCellAnchor);
    check_int_eq(rotate.originalAnchorX, 10,
                 "snapshot keeps original old-front anchor X",
                 e->contractScope);
    check_int_eq(rotate.originalAnchorY, 9,
                 "snapshot keeps original old-front anchor Y",
                 e->contractScope);
    check_int_eq(rotate.oldAnchorStillActive, 0,
                 "old north-front anchor is not still active",
                 e->contractScope);
    check_int_eq(rotate.newAnchorIsCurrentFront, 1,
                 "new east-front anchor is current front",
                 e->contractScope);
    check_uint_eq(rotate.rotationDispatchCountAfter,
                  rotate.rotationDispatchCountBefore + 1u,
                  "rotation dispatch increments once",
                  e->rotationAnchor);
    check_uint_eq(rotate.projectDispatchCountAfter,
                  rotate.projectDispatchCountBefore,
                  "rotation does not re-dispatch project command",
                  e->contractScope);
    check_int_eq(state.champions[0].cell, 1,
                 "champion zero cell rotates clockwise like F0284",
                 e->rotationAnchor);
    check_int_eq(state.champions[1].cell, 2,
                 "champion one cell rotates clockwise like F0284",
                 e->rotationAnchor);
}

static void test_cross_champion_transform_follows_rotation(void)
{
    Dm1V1MirrorCandidatePostRotationStatePc34Compat state;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat project;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat rotate;
    const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectPostRotation_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateThoughtProjectPostRotation_ProjectInFrontPc34Compat(
        &state, &project);
    (void)DM1_V1_MirrorCandidateThoughtProjectPostRotation_RotateClockwisePc34Compat(
        &state, &rotate);

    check_int_eq(rotate.transformActiveBefore, 1,
                 "transform is active before rotation",
                 e->rotationAnchor);
    check_int_eq(rotate.transformActiveAfter, 1,
                 "transform remains active after rotation",
                 e->rotationAnchor);
    check_int_eq(rotate.transformDelta, 1,
                 "transform records one clockwise step",
                 e->rotationAnchor);
    check_int_eq(rotate.transformSourceCellBefore,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_FRONT_RIGHT_PC34_COMPAT,
                 "source champion begins in front-right cell",
                 e->defsCellAnchor);
    check_int_eq(rotate.transformSourceCellAfter,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_BACK_RIGHT_PC34_COMPAT,
                 "source champion cell rotates to back-right",
                 e->defsCellAnchor);
    check_int_eq(rotate.transformTargetCellBefore,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_BACK_LEFT_PC34_COMPAT,
                 "target champion begins in back-left cell",
                 e->defsCellAnchor);
    check_int_eq(rotate.transformTargetCellAfter,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_FRONT_LEFT_PC34_COMPAT,
                 "target champion cell wraps to front-left",
                 e->defsCellAnchor);
    check_int_eq(rotate.transformSourceDirectionBefore,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_NORTH_PC34_COMPAT,
                 "source direction begins north",
                 e->rotationAnchor);
    check_int_eq(rotate.transformSourceDirectionAfter,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_EAST_PC34_COMPAT,
                 "source direction rotates east",
                 e->rotationAnchor);
    check_int_eq(rotate.transformTargetDirectionBefore,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_EAST_PC34_COMPAT,
                 "target direction begins east",
                 e->rotationAnchor);
    check_int_eq(rotate.transformTargetDirectionAfter,
                 DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_SOUTH_PC34_COMPAT,
                 "target direction rotates south",
                 e->rotationAnchor);
    check_int_eq(rotate.transformFollowsRotation, 1,
                 "cross-champion transform follows the clockwise delta",
                 e->rotationAnchor);
}

static void test_post_rotation_overlay_keeps_bitmap_depth_and_new_cell(void)
{
    Dm1V1MirrorCandidatePostRotationStatePc34Compat state;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat project;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat overlayBefore;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat rotate;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat overlayAfter;
    const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectPostRotation_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateThoughtProjectPostRotation_ProjectInFrontPc34Compat(
        &state, &project);
    (void)DM1_V1_MirrorCandidateThoughtProjectPostRotation_RenderOverlayPc34Compat(
        &state, &overlayBefore);
    (void)DM1_V1_MirrorCandidateThoughtProjectPostRotation_RotateClockwisePc34Compat(
        &state, &rotate);
    check_anchor(
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_RenderOverlayPc34Compat(
            &state, &overlayAfter) == 1,
        "overlay renders on the post-rotation front cell",
        e->dungeonViewDepthAnchor);

    check_int_eq(overlayAfter.overlayAfter.visible, 1,
                 "post-rotation overlay is visible",
                 e->dungeonViewCellAnchor);
    check_int_eq(overlayAfter.overlayAfter.mapX, rotate.expectedFrontAfterX,
                 "post-rotation overlay X uses new front cell",
                 e->dungeonViewCellAnchor);
    check_int_eq(overlayAfter.overlayAfter.mapY, rotate.expectedFrontAfterY,
                 "post-rotation overlay Y uses new front cell",
                 e->dungeonViewCellAnchor);
    check_int_eq(overlayAfter.overlayAfter.mapX == project.anchorAfterX &&
                     overlayAfter.overlayAfter.mapY == project.anchorAfterY,
                 0,
                 "post-rotation overlay is not on original old-front cell",
                 e->contractScope);
    check_uint_eq(overlayAfter.overlayAfter.bitmapId,
                  overlayBefore.overlayAfter.bitmapId,
                  "post-rotation overlay keeps same thought-project bitmap",
                  e->dungeonViewCellAnchor);
    check_int_eq(overlayAfter.overlayAfter.depthSlot,
                 overlayBefore.overlayAfter.depthSlot,
                 "post-rotation overlay keeps same D1C depth slot",
                 e->dungeonViewDepthAnchor);
    check_uint_eq(overlayAfter.overlayAfter.cellOrder,
                  overlayBefore.overlayAfter.cellOrder,
                  "post-rotation overlay keeps same DEFS.H cell order",
                  e->defsCellAnchor);
    check_int_eq(overlayAfter.depthAfter, overlayBefore.depthAfter,
                 "state depth slot is stable across overlay redraws",
                 e->dungeonViewDepthAnchor);
    check_uint_eq(overlayAfter.bitmapAfter, overlayBefore.bitmapAfter,
                  "state bitmap is stable across overlay redraws",
                  e->dungeonViewCellAnchor);
    check_uint_eq(overlayAfter.overlayDrawCountAfter,
                  overlayAfter.overlayDrawCountBefore + 1u,
                  "post-rotation overlay redraw increments once",
                  e->dungeonViewDepthAnchor);
    check_int_eq(state.overlay.mapX, 11,
                 "live overlay X remains the east-front cell",
                 e->dungeonViewCellAnchor);
    check_int_eq(state.overlay.mapY, 10,
                 "live overlay Y remains the east-front cell",
                 e->dungeonViewCellAnchor);
}

static void test_cancel_uses_post_rotation_cell_not_original(void)
{
    Dm1V1MirrorCandidatePostRotationStatePc34Compat state;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat project;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat rotate;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat staleCancel;
    Dm1V1MirrorCandidatePostRotationResultPc34Compat frontCancel;
    const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_EvidencePc34Compat();

    DM1_V1_MirrorCandidateThoughtProjectPostRotation_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateThoughtProjectPostRotation_ProjectInFrontPc34Compat(
        &state, &project);
    (void)DM1_V1_MirrorCandidateThoughtProjectPostRotation_RotateClockwisePc34Compat(
        &state, &rotate);

    check_int_eq(
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_CancelAtCellPc34Compat(
            &state, project.anchorAfterX, project.anchorAfterY, &staleCancel),
        0,
        "cancel at original old-front cell is rejected after rotation",
        e->contractScope);
    check_int_eq(staleCancel.cancelRejectedOriginal, 1,
                 "stale cancel result records original-cell rejection",
                 e->contractScope);
    check_int_eq(staleCancel.activeAfter, 1,
                 "stale cancel leaves thought-project active",
                 e->contractScope);
    check_int_eq(staleCancel.anchorAfterX, rotate.anchorAfterX,
                 "stale cancel leaves anchor on new-front X",
                 e->dungeonViewCellAnchor);
    check_int_eq(staleCancel.anchorAfterY, rotate.anchorAfterY,
                 "stale cancel leaves anchor on new-front Y",
                 e->dungeonViewCellAnchor);
    check_uint_eq(staleCancel.cancelDispatchCountAfter,
                  staleCancel.cancelDispatchCountBefore,
                  "stale cancel does not increment accepted cancel count",
                  e->contractScope);

    check_int_eq(
        DM1_V1_MirrorCandidateThoughtProjectPostRotation_CancelInFrontPc34Compat(
            &state, &frontCancel),
        1,
        "cancel in front accepts the post-rotation cell",
        e->contractScope);
    check_int_eq(frontCancel.cancelAccepted, 1,
                 "front cancel result records accepted cancel",
                 e->contractScope);
    check_int_eq(frontCancel.activeBefore, 1,
                 "front cancel sees active thought-project before cancel",
                 e->contractScope);
    check_int_eq(frontCancel.activeAfter, 0,
                 "front cancel clears thought-project after cancel",
                 e->contractScope);
    check_int_eq(frontCancel.anchorBeforeX, rotate.anchorAfterX,
                 "front cancel starts at post-rotation anchor X",
                 e->dungeonViewCellAnchor);
    check_int_eq(frontCancel.anchorBeforeY, rotate.anchorAfterY,
                 "front cancel starts at post-rotation anchor Y",
                 e->dungeonViewCellAnchor);
    check_int_eq(frontCancel.anchorAfterX, -1,
                 "front cancel clears active anchor X",
                 e->contractScope);
    check_int_eq(frontCancel.anchorAfterY, -1,
                 "front cancel clears active anchor Y",
                 e->contractScope);
    check_uint_eq(frontCancel.cancelDispatchCountAfter,
                  frontCancel.cancelDispatchCountBefore + 1u,
                  "front cancel increments accepted cancel count once",
                  e->contractScope);
    check_int_eq(state.originalAnchorMapX, project.anchorAfterX,
                 "cancel preserves original anchor metadata for audit",
                 e->contractScope);
    check_int_eq(state.originalAnchorMapY, project.anchorAfterY,
                 "cancel preserves original anchor Y metadata for audit",
                 e->contractScope);
}

int main(void)
{
    test_source_lock_metadata();
    test_initial_project_is_front_tile();
    test_overlay_before_rotation_uses_project_bitmap_and_depth();
    test_clockwise_rotation_re_resolves_anchor_to_new_front();
    test_cross_champion_transform_follows_rotation();
    test_post_rotation_overlay_keeps_bitmap_depth_and_new_cell();
    test_cancel_uses_post_rotation_cell_not_original();

    printf("Assertions: %d Failures: %d\n", gAssertions, gFailures);
    return gFailures ? 1 : 0;
}
