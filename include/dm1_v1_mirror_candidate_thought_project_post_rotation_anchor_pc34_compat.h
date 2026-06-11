#ifndef DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_POST_ROTATION_ANCHOR_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_POST_ROTATION_ANCHOR_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_PARTY_CHAMPIONS_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_BITMAP_PC34_COMPAT 0x157u
#define DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_DEPTH_D1C_PC34_COMPAT 606
#define DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_CELL_ORDER_D1C_PC34_COMPAT 0x0218u
#define DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_FRONT_LEFT_PC34_COMPAT 0
#define DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_FRONT_RIGHT_PC34_COMPAT 1
#define DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_BACK_RIGHT_PC34_COMPAT 2
#define DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_VIEW_CELL_BACK_LEFT_PC34_COMPAT 3

typedef enum Dm1V1MirrorCandidatePostRotationDirectionPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_NORTH_PC34_COMPAT = 0,
    DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_EAST_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_SOUTH_PC34_COMPAT = 2,
    DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_WEST_PC34_COMPAT = 3
} Dm1V1MirrorCandidatePostRotationDirectionPc34Compat;

typedef struct Dm1V1MirrorCandidatePostRotationEvidencePc34Compat {
    int contractOnly;
    const char *rotationAnchor;
    const char *dungeonViewCellAnchor;
    const char *dungeonViewDepthAnchor;
    const char *defsCellAnchor;
    const char *contractScope;
} Dm1V1MirrorCandidatePostRotationEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidatePostRotationOverlayPc34Compat {
    int visible;
    int mapX;
    int mapY;
    unsigned int bitmapId;
    int depthSlot;
    unsigned int cellOrder;
    int viewCellLeft;
    int viewCellRight;
} Dm1V1MirrorCandidatePostRotationOverlayPc34Compat;

typedef struct Dm1V1MirrorCandidatePostRotationChampionPc34Compat {
    unsigned int ordinal;
    int cell;
    int direction;
} Dm1V1MirrorCandidatePostRotationChampionPc34Compat;

typedef struct Dm1V1MirrorCandidatePostRotationStatePc34Compat {
    int contractOnly;
    int partyMapX;
    int partyMapY;
    int partyDirection;
    unsigned int partyChampionCount;
    Dm1V1MirrorCandidatePostRotationChampionPc34Compat champions
        [DM1_V1_MIRROR_CANDIDATE_POST_ROTATION_PARTY_CHAMPIONS_PC34_COMPAT];
    int thoughtProjectActive;
    int anchorMapX;
    int anchorMapY;
    int originalAnchorMapX;
    int originalAnchorMapY;
    int anchorDirection;
    unsigned int thoughtBitmapId;
    int depthSlot;
    unsigned int cellOrder;
    int crossChampionTransformActive;
    unsigned int transformSourceOrdinal;
    unsigned int transformTargetOrdinal;
    int transformSourceCell;
    int transformTargetCell;
    int transformSourceDirection;
    int transformTargetDirection;
    int transformDelta;
    unsigned int projectDispatchCount;
    unsigned int rotationDispatchCount;
    unsigned int overlayDrawCount;
    unsigned int cancelDispatchCount;
    unsigned int staleOriginalCancelCount;
    Dm1V1MirrorCandidatePostRotationOverlayPc34Compat overlay;
} Dm1V1MirrorCandidatePostRotationStatePc34Compat;

typedef struct Dm1V1MirrorCandidatePostRotationResultPc34Compat {
    const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat *evidence;
    int partyDirectionBefore;
    int partyDirectionAfter;
    int expectedFrontBeforeX;
    int expectedFrontBeforeY;
    int expectedFrontAfterX;
    int expectedFrontAfterY;
    int anchorBeforeX;
    int anchorBeforeY;
    int anchorAfterX;
    int anchorAfterY;
    int originalAnchorX;
    int originalAnchorY;
    int oldAnchorStillActive;
    int newAnchorIsCurrentFront;
    unsigned int bitmapBefore;
    unsigned int bitmapAfter;
    int depthBefore;
    int depthAfter;
    unsigned int cellOrderBefore;
    unsigned int cellOrderAfter;
    int transformActiveBefore;
    int transformActiveAfter;
    int transformSourceCellBefore;
    int transformSourceCellAfter;
    int transformTargetCellBefore;
    int transformTargetCellAfter;
    int transformSourceDirectionBefore;
    int transformSourceDirectionAfter;
    int transformTargetDirectionBefore;
    int transformTargetDirectionAfter;
    int transformDelta;
    int transformFollowsRotation;
    unsigned int projectDispatchCountBefore;
    unsigned int projectDispatchCountAfter;
    unsigned int rotationDispatchCountBefore;
    unsigned int rotationDispatchCountAfter;
    unsigned int overlayDrawCountBefore;
    unsigned int overlayDrawCountAfter;
    unsigned int cancelDispatchCountBefore;
    unsigned int cancelDispatchCountAfter;
    int activeBefore;
    int activeAfter;
    int cancelAccepted;
    int cancelRejectedOriginal;
    Dm1V1MirrorCandidatePostRotationOverlayPc34Compat overlayBefore;
    Dm1V1MirrorCandidatePostRotationOverlayPc34Compat overlayAfter;
} Dm1V1MirrorCandidatePostRotationResultPc34Compat;

void DM1_V1_MirrorCandidateThoughtProjectPostRotation_InitPc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state);

int DM1_V1_MirrorCandidateThoughtProjectPostRotation_ProjectInFrontPc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateThoughtProjectPostRotation_RotateClockwisePc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateThoughtProjectPostRotation_RenderOverlayPc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateThoughtProjectPostRotation_CancelAtCellPc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    int mapX,
    int mapY,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateThoughtProjectPostRotation_CancelInFrontPc34Compat(
    Dm1V1MirrorCandidatePostRotationStatePc34Compat *state,
    Dm1V1MirrorCandidatePostRotationResultPc34Compat *outResult);

const Dm1V1MirrorCandidatePostRotationEvidencePc34Compat *
DM1_V1_MirrorCandidateThoughtProjectPostRotation_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
