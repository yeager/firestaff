#ifndef DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_NONE_PC34_COMPAT 0
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C012_PC34_COMPAT 12
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C016_PC34_COMPAT 16
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C040_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C100_PC34_COMPAT 100
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C111_PC34_COMPAT 111
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C140_PC34_COMPAT 140
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C159_ZONE_PC34_COMPAT 159
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C160_RESURRECT_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C161_REINCARNATE_PC34_COMPAT 161
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C162_CANCEL_PC34_COMPAT 162
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_C175_PORTRAIT_PC34_COMPAT 175
#define DM1_V1_MIRROR_CANDIDATE_C159_CLICK_ROTATION_COMBO_M568_PANEL_PC34_COMPAT 568

typedef struct Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat {
    int contractOnly;
    const char *commandNameRowAnchor;
    const char *commandC040DispatchAnchor;
    const char *commandStatusInventoryGuardAnchor;
    const char *commandSpellActionGuardAnchor;
    const char *commandSaveGuardAnchor;
    const char *reviveCandidatePublishGateAnchor;
    const char *reviveCandidatePublishAnchor;
    const char *reviveCandidateClearAnchor;
    const char *movesensPublishAnchor;
    const char *championLeaderHandAnchor;
    const char *panelPortraitNameZoneAnchor;
    const char *defsPanelCommandAnchor;
    const char *defsNameZoneAnchor;
    const char *defsPanelZoneAnchor;
    const char *contractScope;
} Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat {
    int contractOnly;
    unsigned int partyChampionCount;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int leaderIndex;
    int leaderEmptyHanded;
    int leaderHandThingOrdinal;
    int c040PanelOpen;
    int panelKind;
    int c040PanelGraphic;
    int c159RowClickCount;
    int c159MappedLeaderCommand;
    int c159BlockedByG0299Count;
    int f0367StatusDispatchCount;
    int f0368SetLeaderCount;
    int panelDispatchCount;
    int resurrectDispatchCount;
    int reincarnateDispatchCount;
    int cancelDispatchCount;
    int candidatePublishCount;
    int candidateClearCount;
    int spellDispatchCount;
    int actionDispatchCount;
    int saveDispatchCount;
    int leaderHandPutCount;
    int leaderHandRemoveCount;
    int slotRouteCount;
} Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat;

typedef struct Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat {
    const Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat *evidence;
    int panelCommand;
    int panelCommandValid;
    int c159Clicked;
    int c159MapsToC016;
    int c159BlockedByG0299;
    int c159SetLeaderSkipped;
    int panelStillOwnedAfterC159;
    int panelCommandDispatchedAfterC159;
    int candidateClearedByPanelCommand;
    int cancelRemovedCandidateChampion;
    int acceptedCandidateChampionRemains;
    int nonPanelInputsBlockedByG0299;
    int leaderHandPreserved;
    int noLeaderHandRoutes;
    int noSlotRoutes;
    unsigned int partyChampionCountBefore;
    unsigned int partyChampionCountAfterC159;
    unsigned int partyChampionCountAfterPanel;
    unsigned int candidateOrdinalBefore;
    unsigned int candidateOrdinalAfterC159;
    unsigned int candidateOrdinalAfterPanel;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int leaderHandThingBefore;
    int leaderHandThingAfter;
    int c040PanelOpenBefore;
    int c040PanelOpenAfterC159;
    int c040PanelOpenAfterPanel;
    int c159BlockedCountBefore;
    int c159BlockedCountAfter;
    int f0367StatusDispatchCountBefore;
    int f0367StatusDispatchCountAfter;
    int f0368SetLeaderCountBefore;
    int f0368SetLeaderCountAfter;
    int panelDispatchCountBefore;
    int panelDispatchCountAfter;
    int candidateClearCountBefore;
    int candidateClearCountAfter;
} Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat;

void DM1_V1_MirrorCandidateC159ClickRotationCombo_InitPc34Compat(
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state);

int DM1_V1_MirrorCandidateC159ClickRotationCombo_PublishCandidatePc34Compat(
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state);

int DM1_V1_MirrorCandidateC159ClickRotationCombo_RunPc34Compat(
    Dm1V1MirrorCandidateC159ClickRotationComboStatePc34Compat *state,
    int panelCommand,
    Dm1V1MirrorCandidateC159ClickRotationComboResultPc34Compat *outResult);

const Dm1V1MirrorCandidateC159ClickRotationComboEvidencePc34Compat *
DM1_V1_MirrorCandidateC159ClickRotationCombo_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
