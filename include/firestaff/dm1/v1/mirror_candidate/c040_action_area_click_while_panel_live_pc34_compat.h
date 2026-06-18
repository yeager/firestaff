#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_ACTION_AREA_CLICK_WHILE_PANEL_LIVE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_ACTION_AREA_CLICK_WHILE_PANEL_LIVE_PC34_COMPAT_H

typedef struct DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveStatePc34 {
    int contractOnly;
    int command;
    int partyChampionCount;
    int leaderIndex;
    int leaderHandThing;
    int panelContent;
    int panelGraphic;
    int candidateChampionOrdinal;
    int f0282DispatchCount;
    int f0371CallCount;
    int rejectedWhileLiveCount;
} DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveStatePc34;

typedef struct DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveResultPc34 {
    int accepted;
    int assertionCount;
    int partyCountBefore;
    int leaderHandEmptyBefore;
    int panelContentBefore;
    int panelContentAfterLiveToggles;
    int panelContentAfterClear;
    int candidateOrdinalBefore;
    int candidateOrdinalAfterLiveToggles;
    int candidateOrdinalAfterClear;
    int f0282Dispatched;
    int f0371CallsWhileLive;
    int f0371CallsAfterClear;
    int rejectedWhileLive;
    int rejectedAfterClear;
} DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveResultPc34;

typedef struct DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveSpecPc34 {
    const char *sourceEvidence;
    const char *nonOverlap;
    const char *f0380Anchor;
    const char *f0371Anchor;
    const char *f0280Anchor;
    const char *f0282Anchor;
    const char *defsAnchor;
    int c040PanelContent;
    int c040PanelGraphic;
    int actionAreaClick;
} DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveSpecPc34;

const DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveSpecPc34 *
dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_spec_pc34(
    void);

const char *
dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_source_evidence_pc34(
    void);

void
dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_init_pc34(
    DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveStatePc34 *state);

int
dm1_v1_mirror_candidate_c040_action_area_click_while_panel_live_run_pc34(
    DM1_V1_MirrorCandidateC040ActionAreaClickWhilePanelLiveResultPc34 *out);

#endif
