#ifndef DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_VISIBLE_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_MOUSE_LEFT_PC34_COMPAT 0x0002u
#define DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_MOUSE_RIGHT_PC34_COMPAT 0x0001u
#define DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_C040_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_C159_ZONE_PC34_COMPAT 159
#define DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_M568_PANEL_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_LEADER_HAND_THING_PC34_COMPAT 0x0BEEu

typedef struct Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat {
    int contractOnly;
    const char *commandPanelDispatchAnchor;
    const char *commandClickRoutingAnchor;
    const char *championPartyDirectionAnchor;
    const char *championLeaderHandPutAnchor;
    const char *championLeaderHandRemoveAnchor;
    const char *championSlotWriteClearAnchor;
    const char *panelPortraitRedrawAnchor;
    const char *defsColorAnchor;
    const char *defsHandSlotAnchor;
    const char *defsRosterAnchor;
    const char *defsGlobalsAnchor;
    const char *g0299GuardAnchor;
    const char *nonOverlapNote;
    const char *contractScope;
} Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat {
    int contractOnly;
    int c040PanelOpen;
    int panelKind;
    int leaderIndex;
    int leaderHandEmpty;
    unsigned int leaderHandThingOrdinal;
    int visibleCandidateCount;
    int visibleCandidateIndex;
    unsigned int visibleCandidateOrdinals[
        DM1_V1_MIRROR_CANDIDATE_LEFT_CLICK_ROTATION_VISIBLE_COUNT_PC34_COMPAT];
    unsigned int g0299CandidateChampionOrdinal;
    int leftClickDispatchCount;
    int mirrorCandidateHandlerCount;
    int rotationDispatchCount;
    int c159NameRowDispatchCount;
    int c159SetLeaderCount;
    int statusBoxDispatchCount;
    int spellRuneDispatchCount;
    int saveDispatchCount;
    int leaderHandPutCount;
    int leaderHandRemoveCount;
    int slotWriteCount;
    int slotClearCount;
    int portraitRedrawCount;
} Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat;

typedef struct Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat {
    const Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat *evidence;
    int eventDispatchedToMirrorCandidateHandler;
    int candidateAdvancedByOne;
    int candidateStayedWithinVisibleSet;
    int rotationViewOnly;
    int noC159NameRowSideEffect;
    int noG0299GuardedSideEffect;
    int noLeaderHandSwap;
    int noSlotWriteOrClear;
    int c040PanelStillOpen;
    int visibleCandidateCountBefore;
    int visibleCandidateCountAfter;
    int visibleCandidateIndexBefore;
    int visibleCandidateIndexAfter;
    unsigned int candidateOrdinalBefore;
    unsigned int candidateOrdinalAfter;
    unsigned int g0299Before;
    unsigned int g0299After;
    unsigned int leaderHandThingBefore;
    unsigned int leaderHandThingAfter;
    int leaderHandEmptyBefore;
    int leaderHandEmptyAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int leftClickDispatchCountBefore;
    int leftClickDispatchCountAfter;
    int mirrorCandidateHandlerCountBefore;
    int mirrorCandidateHandlerCountAfter;
    int rotationDispatchCountBefore;
    int rotationDispatchCountAfter;
    int c159NameRowDispatchCountBefore;
    int c159NameRowDispatchCountAfter;
    int c159SetLeaderCountBefore;
    int c159SetLeaderCountAfter;
    int statusBoxDispatchCountBefore;
    int statusBoxDispatchCountAfter;
    int spellRuneDispatchCountBefore;
    int spellRuneDispatchCountAfter;
    int saveDispatchCountBefore;
    int saveDispatchCountAfter;
    int leaderHandPutCountBefore;
    int leaderHandPutCountAfter;
    int leaderHandRemoveCountBefore;
    int leaderHandRemoveCountAfter;
    int slotWriteCountBefore;
    int slotWriteCountAfter;
    int slotClearCountBefore;
    int slotClearCountAfter;
    int portraitRedrawCountBefore;
    int portraitRedrawCountAfter;
} Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat;

void DM1_V1_MirrorCandidateLeftClickRotation_InitPc34Compat(
    Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat *state);

int DM1_V1_MirrorCandidateLeftClickRotation_ApplyPc34Compat(
    Dm1V1MirrorCandidateLeftClickRotationStatePc34Compat *state,
    unsigned int mouseButtons,
    Dm1V1MirrorCandidateLeftClickRotationResultPc34Compat *outResult);

const Dm1V1MirrorCandidateLeftClickRotationEvidencePc34Compat *
DM1_V1_MirrorCandidateLeftClickRotation_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
