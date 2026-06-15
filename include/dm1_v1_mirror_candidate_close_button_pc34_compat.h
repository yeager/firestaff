#ifndef DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_NONE_PC34_COMPAT (-1)

#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_SOURCE_PANEL_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_SOURCE_PANEL_CONTENT_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_SOURCE_CANCEL_COMMAND_PC34_COMPAT 162
#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_COMMAND_PC34_COMPAT 0x4000

#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_FIELD_G0299_PC34_COMPAT \
    "G0299_ui_CandidateChampionOrdinal"
#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_FIELD_G0305_PC34_COMPAT \
    "G0305_ui_PartyChampionCount"
#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_FIELD_G0411_PC34_COMPAT \
    "G0411_i_LeaderIndex"
#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_FIELD_G0423_PC34_COMPAT \
    "G0423_i_InventoryChampionOrdinal"
#define DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_FIELD_G0289_PC34_COMPAT \
    "G0289_i_DungeonView_ChampionPortraitOrdinal"

typedef struct Dm1V1MirrorCandidateCloseButtonChampionPc34Compat {
    unsigned int championOrdinal;
    int currentHealth;
    int portraitOrdinal;
    int present;
} Dm1V1MirrorCandidateCloseButtonChampionPc34Compat;

typedef struct Dm1V1MirrorCandidateCloseButtonStatePc34Compat {
    int active;
    int partyChampionCount;
    int preC040PartyChampionCount;
    unsigned int g0299CandidateChampionOrdinal;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    unsigned int preC040InventoryChampionOrdinal;
    unsigned int leaderHandChampionOrdinal;
    int leaderIndex;
    int frontD1cMirrorChampionOrdinal;
    int panelContent;
    int c040PanelOpen;
    int c040PanelPixelsDrawn;
    Dm1V1MirrorCandidateCloseButtonChampionPc34Compat
        party[DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateCloseButtonStatePc34Compat;

typedef struct Dm1V1MirrorCandidateCloseButtonResultPc34Compat {
    int command;
    int validCloseButtonCommand;
    int closedPanel;
    int ignoredNotCloseButton;
    int resurrectCommandReached;
    int reincarnateCommandReached;
    int candidatePromotedToLeaderHand;
    int candidateIdentityPreserved;
    int candidateChampionOrdinalBefore;
    int candidateChampionOrdinalAfter;
    unsigned int g0299Before;
    unsigned int g0299After;
    unsigned int inventoryChampionOrdinalBefore;
    unsigned int inventoryChampionOrdinalAfter;
    unsigned int inventoryChampionOrdinalPreC040;
    unsigned int leaderHandChampionOrdinalBefore;
    unsigned int leaderHandChampionOrdinalAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int partyChampionCountBefore;
    int partyChampionCountAfter;
    int party0OrdinalBefore;
    int party0OrdinalAfter;
    int party1OrdinalBefore;
    int party1OrdinalAfter;
    int party0HealthBefore;
    int party0HealthAfter;
    int party1HealthBefore;
    int party1HealthAfter;
    int candidateSlotPresentBefore;
    int candidateSlotPresentAfter;
    int previousFrontD1cMirrorChampionOrdinal;
    int newFrontD1cMirrorChampionOrdinal;
    int mirrorRoutePreservedFromOpen;
    int mirrorRouteRearmedByResurrect;
    int frontD1cPortraitIndex;
    int c040PanelCleared;
    int c040PanelPixelsBefore;
    int c040PanelPixelsAfter;
    int actionAreaGateOpenAfterClose;
} Dm1V1MirrorCandidateCloseButtonResultPc34Compat;

typedef struct Dm1V1MirrorCandidateCloseButtonSpecPc34Compat {
    const char *name;
    int c040PanelGraphic;
    int c040PanelContent;
    int closeButtonCommand;
    int sourceCancelCommand;
    const char *contractMarker;
    const char *sourceEvidence;
} Dm1V1MirrorCandidateCloseButtonSpecPc34Compat;

extern const Dm1V1MirrorCandidateCloseButtonSpecPc34Compat
    DM1_V1_MirrorCandidateCloseButtonSpecPc34Compat;

void dm1_v1_mirror_candidate_close_button_init_pc34(
    Dm1V1MirrorCandidateCloseButtonStatePc34Compat *state);

int dm1_v1_mirror_candidate_close_button_pc34(
    Dm1V1MirrorCandidateCloseButtonStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateCloseButtonResultPc34Compat *outResult);

const Dm1V1MirrorCandidateCloseButtonSpecPc34Compat *
dm1_v1_mirror_candidate_close_button_spec_pc34(void);

const char *dm1_v1_mirror_candidate_close_button_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
