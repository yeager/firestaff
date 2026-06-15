#ifndef DM1_V1_MIRROR_CANDIDATE_C040_REDRAW_AFTER_CHEST_CLOSE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_C040_REDRAW_AFTER_CHEST_CLOSE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB anchors: CHEST.C F0333:30-67 opens/closes the chest list;
 * CHEST.C F0334:117-132 rewrites visible C537..C544 slots on close;
 * CHAMPION.C F0297:243-268, F0298:270-298, F0300:511-515,
 * F0301:606-614, F0302:662-714, and F0284:93-131 own hand state,
 * C30+ ownership, list walking, and champion switches. PANEL.C
 * F0344:1390-1406/F0345, F0346:1619-1657/F0347, and F0352 keep the
 * C040 panel drawn/redrawn. REVIVE.C F0280:124-132 and F0282:744-806
 * open and clear the mirror candidate. COMMAND.C F0359:1985-1990 writes
 * the mirror queue. DEFS.H:2088, 810-817, 1874-1878, 2200, 3001-3008,
 * 5694, and 5876-5881 name C10_COLOR_FLESH, C30..C37, C38, C040,
 * M568/M569, G0299, and G0425/G0426.
 */

#define DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT 8

enum {
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_NONE_PC34_COMPAT = 0xFFFF,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C10_FLESH_PC34_COMPAT = 10,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C30_SLOT_PC34_COMPAT = 30,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C37_SLOT_PC34_COMPAT = 37,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C38_BOX_PC34_COMPAT = 38,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C040_PANEL_PC34_COMPAT = 40,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C070_MOUTH_PC34_COMPAT = 70,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C537_VISIBLE_PC34_COMPAT = 537,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C544_VISIBLE_PC34_COMPAT = 544,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_C545_ZONE_PC34_COMPAT = 545,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_M568_CANDIDATE_PC34_COMPAT = 568,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_M569_CHEST_PC34_COMPAT = 569,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_G0299_CANDIDATE_PC34_COMPAT = 299,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_G0425_CHEST_LIST_PC34_COMPAT = 425,
    DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_G0426_OPEN_CHEST_PC34_COMPAT = 426
};

typedef struct Dm1V1MirrorC040RedrawAfterChestCloseEvidencePc34Compat {
    int contractOnly;
    const char *chestListOpenCloseAnchor;
    const char *chestVisibleCloseRewriteAnchor;
    const char *championHandStateAnchor;
    const char *championOwnershipAnchor;
    const char *championListWalkAnchor;
    const char *championSwitchAnchor;
    const char *panelDrawHookAnchor;
    const char *panelStateAnchor;
    const char *panelRedrawOnCloseAnchor;
    const char *candidateOpenAnchor;
    const char *candidateCloseAnchor;
    const char *mirrorQueueAnchor;
    const char *defsAnchor;
    const char *contractScope;
} Dm1V1MirrorC040RedrawAfterChestCloseEvidencePc34Compat;

typedef struct Dm1V1MirrorC040RedrawAfterChestCloseSpecPc34Compat {
    unsigned int deterministicSeed;
    int leaderIndex;
    int partyChampionCount;
    unsigned int candidateOrdinal;
    int c040PanelGraphic;
    int c10PanelColor;
    int m568CandidatePanel;
    int m569ChestPanel;
    int c545MouthZone;
    int c070MouthCommand;
    int openChestThing;
    int candidateMarkerThing;
} Dm1V1MirrorC040RedrawAfterChestCloseSpecPc34Compat;

typedef struct Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat {
    int contractOnly;
    unsigned int deterministicSeed;
    int leaderIndex;
    int partyChampionCount;
    unsigned int candidateOrdinal;
    unsigned int g0299CandidateOrdinal;
    int candidateMarkerThing;
    int c040PanelOpen;
    int c040PanelGraphic;
    int c040PanelCommand;
    int c040PanelColor;
    int c040PanelOwnerSlot;
    int c038SlotBox;
    int mouthRouteZone;
    int mouthRouteCommand;
    int g0426OpenChestThing;
    int chestOpen;
    int visibleC537ToC544
        [DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT];
    int g0425ChestList
        [DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT];
    int championHandC537ToC544
        [DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT];
    unsigned int panelHashBeforeClose;
    unsigned int panelHashAfterClose;
    int f0333OpenCloseCount;
    int f0334VisibleRewriteCount;
    int f0297HandStateCount;
    int f0298OwnershipCount;
    int f0300ListWalkCount;
    int f0301ListWalkCount;
    int f0302ListWalkCount;
    int f0284ChampionSwitchCount;
    int f0344PanelDrawHookCount;
    int f0346PanelStateCount;
    int f0352PanelRedrawOnCloseCount;
    int f0280CandidateOpenCount;
    int f0282CandidateCloseCount;
    int f0359MirrorQueueWriteCount;
    int panelFlickerCount;
    int redrawClobberCount;
    int candidateLeakCount;
} Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat;

typedef struct Dm1V1MirrorC040RedrawAfterChestCloseResultPc34Compat {
    const Dm1V1MirrorC040RedrawAfterChestCloseEvidencePc34Compat *evidence;
    const Dm1V1MirrorC040RedrawAfterChestCloseSpecPc34Compat *spec;
    int accepted;
    unsigned int deterministicHash;
    unsigned int initialPanelHash;
    unsigned int finalPanelHash;
    int initialPanelOpen;
    int finalPanelOpen;
    int initialPanelGraphic;
    int finalPanelGraphic;
    int initialPanelCommand;
    int finalPanelCommand;
    unsigned int initialCandidateOrdinal;
    unsigned int finalCandidateOrdinal;
    int initialOpenChestThing;
    int finalOpenChestThing;
    int initialChestOpen;
    int finalChestOpen;
    int visibleSlotsCleared;
    int chestListStable;
    int championHandStateStable;
    int panelHashStable;
    int candidateStillLive;
    int noPanelFlicker;
    int noRedrawClobber;
    int noCandidateLeakage;
    int f0333OpenCloseCount;
    int f0334VisibleRewriteCount;
    int f0297HandStateCount;
    int f0298OwnershipCount;
    int f0300ListWalkCount;
    int f0301ListWalkCount;
    int f0302ListWalkCount;
    int f0284ChampionSwitchCount;
    int f0344PanelDrawHookCount;
    int f0346PanelStateCount;
    int f0352PanelRedrawOnCloseCount;
    int f0280CandidateOpenCount;
    int f0282CandidateCloseCount;
    int f0359MirrorQueueWriteCount;
    int visibleBefore
        [DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT];
    int visibleAfter
        [DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT];
    int chestListBefore
        [DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT];
    int chestListAfter
        [DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT];
    int championHandBefore
        [DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT];
    int championHandAfter
        [DM1_V1_MIRROR_C040_REDRAW_AFTER_CHEST_CLOSE_SLOT_COUNT_PC34_COMPAT];
    int rejectsNullState;
    int rejectsNullResult;
    int rejectsNonContract;
    int rejectsNoPanel;
    int rejectsNoCandidate;
    int rejectsNoOpenChest;
    int rejectsWrongMouthRoute;
    int rejectsCandidateLeakPreload;
    int mutationGuardsOk;
} Dm1V1MirrorC040RedrawAfterChestCloseResultPc34Compat;

void dm1_v1_mirror_candidate_c040_redraw_after_chest_close_init_pc34_compat(
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *state);

int dm1_v1_mirror_candidate_c040_redraw_after_chest_close_run_pc34_compat(
    Dm1V1MirrorC040RedrawAfterChestCloseStatePc34Compat *state,
    Dm1V1MirrorC040RedrawAfterChestCloseResultPc34Compat *outResult);

const Dm1V1MirrorC040RedrawAfterChestCloseEvidencePc34Compat *
dm1_v1_mirror_candidate_c040_redraw_after_chest_close_evidence_pc34_compat(
    void);

const Dm1V1MirrorC040RedrawAfterChestCloseSpecPc34Compat *
dm1_v1_mirror_candidate_c040_redraw_after_chest_close_spec_pc34_compat(void);

const char *
dm1_v1_mirror_candidate_c040_redraw_after_chest_close_source_evidence_pc34_compat(
    void);

#ifdef __cplusplus
}
#endif

#endif
