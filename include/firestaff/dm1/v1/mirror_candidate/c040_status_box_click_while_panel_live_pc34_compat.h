/*
 * c040_status_box_click_while_panel_live_pc34_compat.h
 *
 * Public surface for the DM1 V1 mirror-candidate C040 status-box-click
 * (C012..C015 champion-status-box click) while the C040 candidate
 * panel is live. Contract-only runtime regression.
 *
 * Source-locked against (ReDMCSB):
 *   COMMAND.C F0380:2159-2161  C012..C015 status-box click gate on
 *                              (champion_index < G0305) && !G0299
 *   COMMAND.C F0367            C012..C015 status-box click entry point
 *   REVIVE.C  F0280:124-132    publishes G0299 when candidate panel opens
 *   REVIVE.C  F0282:744-806    clears G0299 on C162 cancel
 *   DEFS.H                     C012..C015, C040/M568, G0299, G0305, G0411
 *
 * Disjoint from pass787 (C111 action-area), pass786 (C100 with G0514),
 * pass785 (C007..C011 inventory-toggle), pass784 (cancel-then-reopen).
 */

#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_STATUS_BOX_CLICK_WHILE_PANEL_LIVE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_STATUS_BOX_CLICK_WHILE_PANEL_LIVE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Spec: source anchors + c040 panel content/graphic + the
 * champion-index range that the F0367 status-box click handler dispatches. */
typedef struct DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveSpecPc34 {
    const char *sourceEvidence;
    const char *nonOverlap;
    const char *f0380Anchor;
    const char *f0367Anchor;
    const char *f0280Anchor;
    const char *f0282Anchor;
    const char *defsAnchor;
    int c040PanelContent;
    int c040PanelGraphic;
    int statusBoxClickChampion0;
    int statusBoxClickChampion3;
} DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveSpecPc34;

/* State: live mirror-candidate C040 panel + status-box dispatch counters. */
typedef struct DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveStatePc34 {
    int contractOnly;
    int partyChampionCount;
    int leaderIndex;
    int leaderHandThing;
    int panelContent;
    int panelGraphic;
    int candidateChampionOrdinal;
    int rejectedOutOfRangeCount;
    int rejectedWhileLiveCount;
    int f0367CallCount;
    int f0282DispatchCount;
    int command;
} DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveStatePc34;

/* Result: before/after telemetry for the live→clear→dispatch sequence. */
typedef struct DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveResultPc34 {
    int candidateOrdinalBefore;
    int panelContentBefore;
    int leaderHandEmptyBefore;
    int partyCountBefore;
    int f0367CallsWhileLive;
    int rejectedWhileLive;
    int rejectedOutOfRange;
    int panelContentAfterLiveToggles;
    int candidateOrdinalAfterLiveToggles;
    int panelContentAfterClear;
    int candidateOrdinalAfterClear;
    int f0367CallsAfterClear;
    int f0282Dispatched;
    int rejectedAfterClear;
    int accepted;
    int assertionCount;
    int contractHold;
} DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveResultPc34;

const DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveSpecPc34 *
dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_spec_pc34(
    void);

const char *
DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveSourceEvidencePc34(void);

const char *
dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_source_evidence_pc34(
    void);

void
dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_init_pc34(
    DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveStatePc34 *state);

int
dm1_v1_mirror_candidate_c040_status_box_click_while_panel_live_run_pc34(
    DM1_V1_MirrorCandidateC040StatusBoxClickWhilePanelLiveResultPc34 *out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_STATUS_BOX_CLICK_WHILE_PANEL_LIVE_PC34_COMPAT_H */