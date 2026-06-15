#ifndef DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_ROW_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_NONE_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_RIGHT_PC34_COMPAT 0x0001u
#define DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_MOUSE_LEFT_PC34_COMPAT 0x0002u
#define DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_PANEL_CONTENT_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_C040_PANEL_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_C159_ZONE_PC34_COMPAT 159
#define DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_THING_NONE_PC34_COMPAT 0xFFFFu

typedef struct Dm1V1MirrorCandidatePickupRightClickRowPc34Compat {
    int zone;
    int left;
    int right;
    int top;
    int bottom;
    unsigned int championOrdinal;
    unsigned int leaderHandThing;
    int present;
} Dm1V1MirrorCandidatePickupRightClickRowPc34Compat;

typedef struct Dm1V1MirrorCandidatePickupRightClickEvidencePc34Compat {
    const char *candidatePublishAnchor;
    const char *candidateClearAnchor;
    const char *commandGateAnchor;
    const char *championLeaderHandAnchor;
    const char *mirrorCellAnchor;
    const char *c159NameRowAnchor;
    const char *panelEmptyHandAnchor;
    const char *contractScope;
} Dm1V1MirrorCandidatePickupRightClickEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidatePickupRightClickStatePc34Compat {
    int active;
    int partyChampionCount;
    int preC040PartyChampionCount;
    unsigned int g0299CandidateChampionOrdinal;
    unsigned int candidateChampionOrdinal;
    unsigned int leaderHandThing;
    int leaderHandEmpty;
    int leaderIndex;
    int panelContent;
    int c040PanelOpen;
    int c040PanelPixelsDrawn;
    int candidatePublishCount;
    int candidateClearCount;
    int c040PanelPublishCount;
    int c040PanelClearCount;
    int leaderHandPutCount;
    int leaderHandRemoveCount;
    int leftClickCommandCount;
    int spellActionDispatchCount;
    Dm1V1MirrorCandidatePickupRightClickRowPc34Compat
        rows[DM1_V1_MIRROR_CANDIDATE_PICKUP_RIGHT_CLICK_ROW_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidatePickupRightClickStatePc34Compat;

typedef struct Dm1V1MirrorCandidatePickupRightClickResultPc34Compat {
    const Dm1V1MirrorCandidatePickupRightClickEvidencePc34Compat *evidence;
    int consumed;
    int resolvedRowIndex;
    int resolvedZone;
    int rightClickOnly;
    int deadzoneSkipped;
    int emptyRowNoop;
    int rejectedPanelNotPending;
    int rejectedLeaderHandFull;
    int publishedCandidate;
    int clearedCandidate;
    int noDoublePublish;
    int noLeftClickCommand;
    int c159ChampionIconGuardHeld;
    int partyChampionCountBefore;
    int partyChampionCountAfter;
    unsigned int g0299Before;
    unsigned int g0299After;
    unsigned int candidateChampionOrdinalBefore;
    unsigned int candidateChampionOrdinalAfter;
    unsigned int leaderHandThingBefore;
    unsigned int leaderHandThingAfter;
    int leaderHandEmptyBefore;
    int leaderHandEmptyAfter;
    int c040PanelOpenBefore;
    int c040PanelOpenAfter;
    int c040PanelPixelsBefore;
    int c040PanelPixelsAfter;
    int candidatePublishCountBefore;
    int candidatePublishCountAfter;
    int candidateClearCountBefore;
    int candidateClearCountAfter;
    int c040PanelPublishCountBefore;
    int c040PanelPublishCountAfter;
    int c040PanelClearCountBefore;
    int c040PanelClearCountAfter;
    int leaderHandPutCountBefore;
    int leaderHandPutCountAfter;
    int leaderHandRemoveCountBefore;
    int leaderHandRemoveCountAfter;
    int leftClickCommandCountBefore;
    int leftClickCommandCountAfter;
    int spellActionDispatchCountBefore;
    int spellActionDispatchCountAfter;
} Dm1V1MirrorCandidatePickupRightClickResultPc34Compat;

void DM1_V1_MirrorCandidatePickupRightClick_InitPc34Compat(
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat *state);

int DM1_V1_MirrorCandidatePickupRightClick_ApplyPc34Compat(
    Dm1V1MirrorCandidatePickupRightClickStatePc34Compat *state,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1MirrorCandidatePickupRightClickResultPc34Compat *outResult);

const Dm1V1MirrorCandidatePickupRightClickEvidencePc34Compat *
DM1_V1_MirrorCandidatePickupRightClick_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
