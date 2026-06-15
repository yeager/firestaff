#ifndef DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_NONE_PC34_COMPAT (-1)

#define DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_COMMAND_DUNGEON_VIEW_PC34_COMPAT 80
#define DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_SOURCE_PANEL_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_SOURCE_PANEL_CONTENT_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_SOURCE_PORTRAIT_SENSOR_PC34_COMPAT 127
#define DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_SOURCE_D1C_WALL_PC34_COMPAT 587

typedef struct Dm1V1MirrorCandidateClickCancelChampionPc34Compat {
    unsigned int championOrdinal;
    int currentHealth;
    int portraitOrdinal;
    int present;
} Dm1V1MirrorCandidateClickCancelChampionPc34Compat;

typedef struct Dm1V1MirrorCandidateClickCancelStatePc34Compat {
    int active;
    int partyChampionCount;
    int preC040PartyChampionCount;
    int candidateAppendCount;
    unsigned int g0299CandidateChampionOrdinal;
    unsigned int candidateChampionOrdinal;
    unsigned int selectedChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int inventoryPanelOpen;
    int leaderIndex;
    unsigned int leaderHandThing;
    int frontD1cMirrorChampionOrdinal;
    int frontD1cMirrorPortraitIndex;
    int mirrorRouteArmed;
    int frontD1cCellVisible;
    int c040PanelOpen;
    int c040PanelPixelsDrawn;
    unsigned int c040AbsencePixelHash;
    Dm1V1MirrorCandidateClickCancelChampionPc34Compat
        party[DM1_V1_MIRROR_CANDIDATE_CLICK_CANCEL_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateClickCancelStatePc34Compat;

typedef struct Dm1V1MirrorCandidateClickCancelResultPc34Compat {
    int command;
    int consumed;
    int ignoredFrontCellOnly;
    int noCandidateBefore;
    int noCandidateAfter;
    int noC040Before;
    int noC040After;
    int candidateAppendCountBefore;
    int candidateAppendCountAfter;
    int partyChampionCountBefore;
    int partyChampionCountAfter;
    unsigned int g0299Before;
    unsigned int g0299After;
    unsigned int candidateChampionOrdinalBefore;
    unsigned int candidateChampionOrdinalAfter;
    unsigned int selectedChampionOrdinalBefore;
    unsigned int selectedChampionOrdinalAfter;
    unsigned int inventoryChampionOrdinalBefore;
    unsigned int inventoryChampionOrdinalAfter;
    int inventoryPanelOpenBefore;
    int inventoryPanelOpenAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    unsigned int leaderHandThingBefore;
    unsigned int leaderHandThingAfter;
    int frontD1cMirrorChampionOrdinalBefore;
    int frontD1cMirrorChampionOrdinalAfter;
    int frontD1cMirrorPortraitIndexBefore;
    int frontD1cMirrorPortraitIndexAfter;
    int mirrorRouteArmedBefore;
    int mirrorRouteArmedAfter;
    int c040PanelPixelsBefore;
    int c040PanelPixelsAfter;
    unsigned int c040AbsencePixelHashBefore;
    unsigned int c040AbsencePixelHashAfter;
    int c040PanelStayedClosed;
    int c040PixelsPreserved;
    int candidateCountStayedZero;
    int candidateIdentityStayedNone;
    int inventoryStayedClosed;
    int mirrorRouteStayedArmed;
    int leaderHandUnchanged;
    int championIdentityUnchanged;
    int partyCountUnchanged;
} Dm1V1MirrorCandidateClickCancelResultPc34Compat;

typedef struct Dm1V1MirrorCandidateClickCancelSpecPc34Compat {
    const char *name;
    int dungeonViewCommand;
    int c040PanelGraphic;
    int c040PanelContent;
    int championPortraitSensor;
    int d1cViewWall;
    const char *contractMarker;
    const char *sourceEvidence;
} Dm1V1MirrorCandidateClickCancelSpecPc34Compat;

extern const Dm1V1MirrorCandidateClickCancelSpecPc34Compat
    DM1_V1_MirrorCandidateClickCancelSpecPc34Compat;

void dm1_v1_mirror_candidate_click_cancel_init_pc34(
    Dm1V1MirrorCandidateClickCancelStatePc34Compat *state);

int dm1_v1_mirror_candidate_click_cancel_select_champion_pc34(
    Dm1V1MirrorCandidateClickCancelStatePc34Compat *state,
    unsigned int championOrdinal);

int dm1_v1_mirror_candidate_click_cancel_front_cell_pc34(
    Dm1V1MirrorCandidateClickCancelStatePc34Compat *state,
    Dm1V1MirrorCandidateClickCancelResultPc34Compat *outResult);

const Dm1V1MirrorCandidateClickCancelSpecPc34Compat *
dm1_v1_mirror_candidate_click_cancel_spec_pc34(void);

const char *dm1_v1_mirror_candidate_click_cancel_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
