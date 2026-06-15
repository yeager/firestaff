/* ReDMCSB source-lock evidence:
 * ReDMCSB COMMAND.C F0359:1985-1990 gates the M568 C040 panel route on
 * G0415_ui_LeaderEmptyHanded before scanning C160/C161/C162.
 * ReDMCSB REVIVE.C F0282:744-806 owns the cancel/resurrect/reincarnate
 * side effects only after COMMAND.C dispatches a C040 panel command.
 * ReDMCSB REVIVE.C F0280:124-132 refuses candidate publication when the
 * leader hand is already occupied; this gate starts after publication to
 * isolate the C040 button regression.
 */
#ifndef DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_M568_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_C160_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_C161_PC34_COMPAT 161
#define DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_C162_PC34_COMPAT 162
#define DM1_V1_MIRROR_CANDIDATE_OCCUPIED_HAND_PANEL_OBJECT_PC34_COMPAT 0x4a11u

typedef struct Dm1V1MirrorCandidateOccupiedHandPanelEvidencePc34Compat {
    const char *commandPanelGateAnchor;
    const char *commandPanelButtonsAnchor;
    const char *revivePanelEffectsAnchor;
    const char *revivePublishHandAnchor;
    const char *nonOverlapNote;
    const char *contractScope;
} Dm1V1MirrorCandidateOccupiedHandPanelEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateOccupiedHandPanelStatePc34Compat {
    int panelContent;
    int c040PanelOpen;
    int leaderHandEmpty;
    unsigned int leaderHandObject;
    unsigned int candidateChampionOrdinal;
    unsigned int candidateIdentityToken;
    unsigned int partyChampionCount;
    unsigned int inventoryChampionOrdinal;
    int f0358HitScanCount;
    int f0282DispatchCount;
    int cancelClearCount;
    int resurrectClearCount;
    int reincarnateClearCount;
    int screenUpdateEnableCount;
    int clickConsumedCount;
} Dm1V1MirrorCandidateOccupiedHandPanelStatePc34Compat;

typedef struct Dm1V1MirrorCandidateOccupiedHandPanelResultPc34Compat {
    const Dm1V1MirrorCandidateOccupiedHandPanelEvidencePc34Compat *evidence;
    int command;
    int accepted;
    int blockedByOccupiedLeaderHand;
    int f0358HitScanSkipped;
    int f0282NotInvoked;
    int commandIgnored;
    int panelContentBefore;
    int panelContentAfter;
    int c040PanelOpenBefore;
    int c040PanelOpenAfter;
    int leaderHandEmptyBefore;
    int leaderHandEmptyAfter;
    unsigned int leaderHandObjectBefore;
    unsigned int leaderHandObjectAfter;
    unsigned int candidateChampionOrdinalBefore;
    unsigned int candidateChampionOrdinalAfter;
    unsigned int candidateIdentityTokenBefore;
    unsigned int candidateIdentityTokenAfter;
    unsigned int partyChampionCountBefore;
    unsigned int partyChampionCountAfter;
    unsigned int inventoryChampionOrdinalBefore;
    unsigned int inventoryChampionOrdinalAfter;
    int f0358HitScanCountBefore;
    int f0358HitScanCountAfter;
    int f0282DispatchCountBefore;
    int f0282DispatchCountAfter;
    int cancelClearCountBefore;
    int cancelClearCountAfter;
    int resurrectClearCountBefore;
    int resurrectClearCountAfter;
    int reincarnateClearCountBefore;
    int reincarnateClearCountAfter;
    int screenUpdateEnableCountBefore;
    int screenUpdateEnableCountAfter;
    int clickConsumedCountBefore;
    int clickConsumedCountAfter;
    int candidateStatePreserved;
    int panelStatePreserved;
    int leaderHandPreserved;
    int noReviveSideEffects;
} Dm1V1MirrorCandidateOccupiedHandPanelResultPc34Compat;

void DM1_V1_MirrorCandidateOccupiedHandPanel_InitPc34Compat(
    Dm1V1MirrorCandidateOccupiedHandPanelStatePc34Compat *state);

int DM1_V1_MirrorCandidateOccupiedHandPanel_ClickPc34Compat(
    Dm1V1MirrorCandidateOccupiedHandPanelStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateOccupiedHandPanelResultPc34Compat *outResult);

const Dm1V1MirrorCandidateOccupiedHandPanelEvidencePc34Compat *
DM1_V1_MirrorCandidateOccupiedHandPanel_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
