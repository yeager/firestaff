#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_INVENTORY_TOGGLE_WHILE_PANEL_LIVE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_INVENTORY_TOGGLE_WHILE_PANEL_LIVE_PC34_COMPAT_H

typedef struct DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveStatePc34 {
    int contractOnly;
    int command;
    int partyChampionCount;
    int leaderIndex;
    int leaderHandThing;
    int panelContent;
    int panelGraphic;
    int candidateChampionOrdinal;
    int openChestThing;
    int f0282DispatchCount;
    int f0355CallCount;
    int rejectedWhileLiveCount;
} DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveStatePc34;

typedef struct DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveResultPc34 {
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
    int f0355CallsWhileLive;
    int f0355CallsAfterClear;
    int rejectedWhileLive;
    int rejectedAfterClear;
} DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveResultPc34;

typedef struct DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveSpecPc34 {
    const char *sourceEvidence;
    const char *nonOverlap;
    const char *f0380Anchor;
    const char *f0355Anchor;
    const char *f0280Anchor;
    const char *f0282Anchor;
    const char *defsAnchor;
    int c040PanelContent;
    int c040PanelGraphic;
    int inventoryToggleChampion0;
    int inventoryToggleChampion3;
    int closeInventory;
} DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveSpecPc34;

const DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveSpecPc34 *
dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_spec_pc34(
    void);

const char *
dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_source_evidence_pc34(
    void);

void
dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_init_pc34(
    DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveStatePc34 *state);

int
dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_run_pc34(
    DM1_V1_MirrorCandidateC040InventoryToggleWhilePanelLiveResultPc34 *out);

#endif
