#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_CANCEL_THEN_REOPEN_SAME_TICK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_CANCEL_THEN_REOPEN_SAME_TICK_PC34_COMPAT_H

typedef struct DM1_V1_MirrorCandidateC040CancelThenReopenSameTickStatePc34 {
    int contractOnly;
    int command;
    int partyChampionCount;
    int candidateChampionOrdinal;
    int leaderHandThing;
    int panelContent;
    int panelGraphic;
    int partyMapX;
    int partyMapY;
    int partyDirection;
    int sensorCellThing;
    int mirrorCellThing;
    int f0282DispatchCount;
    int f0280DispatchCount;
    int f0280Rejected;
    int f0355CallCount;
} DM1_V1_MirrorCandidateC040CancelThenReopenSameTickStatePc34;

typedef struct DM1_V1_MirrorCandidateC040CancelThenReopenSameTickResultPc34 {
    int accepted;
    int assertionCount;
    int partyCountBefore;
    int partyCountMid;
    int partyCountAfter;
    int candidateOrdinalBefore;
    int candidateOrdinalMid;
    int candidateOrdinalAfter;
    int panelContentBefore;
    int panelContentMid;
    int panelContentAfter;
    int panelGraphicBefore;
    int panelGraphicAfter;
    int leaderHandEmptyBefore;
    int leaderHandEmptyAfter;
    int f0282Dispatched;
    int f0280Dispatched;
    int f0280NotRejected;
    int f0355Called;
    int partyMovedToFreshSensor;
    int sameTick;
} DM1_V1_MirrorCandidateC040CancelThenReopenSameTickResultPc34;

typedef struct DM1_V1_MirrorCandidateC040CancelThenReopenSameTickSpecPc34 {
    const char *sourceEvidence;
    const char *nonOverlap;
    const char *f0280Anchor;
    const char *f0282Anchor;
    const char *f0355Anchor;
    const char *f0378Anchor;
    const char *f0275Anchor;
    const char *defsAnchor;
    int c040PanelContent;
    int c040PanelGraphic;
    int partyCountBefore;
    int partyCountMid;
    int initialCandidateOrdinal;
    int reopenedCandidateOrdinal;
} DM1_V1_MirrorCandidateC040CancelThenReopenSameTickSpecPc34;

const DM1_V1_MirrorCandidateC040CancelThenReopenSameTickSpecPc34 *
dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_spec_pc34(void);

const char *
dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_source_evidence_pc34(
    void);

void
dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_init_pc34(
    DM1_V1_MirrorCandidateC040CancelThenReopenSameTickStatePc34 *state);

int dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_run_pc34(
    DM1_V1_MirrorCandidateC040CancelThenReopenSameTickResultPc34 *out);

#endif
