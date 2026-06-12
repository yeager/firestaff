#ifndef DM1_V1_MIRROR_CANDIDATE_C545_DROP_WHILE_PANEL_LIVE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_C545_DROP_WHILE_PANEL_LIVE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB anchors: CHEST.C F0334:117-132 owns G0426 close and G0425
 * relink; CHAMPION.C F0297:243-268 and F0298:270-298 own leader-hand
 * put/remove; COMMAND.C F0378:1973-1983 and F0380:2045-2159 preserve
 * C545/panel dispatch identity; REVIVE.C F0280:124-132 and F0282:744-806
 * publish and clear G0299; PANEL.C F0346/F0347:1619-1657 redraws C040
 * while G0299 is live; UTAMSCR.C F0077/F0078:141-150 and BLITMASK.C
 * F0133:30-33 bracket redraw/mask behavior; DEFS.H:338-340, 810-817,
 * 1874-1878, 2200, 3001-3008, 5694, and 5876-5881 name C162, C30..C37,
 * C38, C040, M568/M569, G0299, G0425, and G0426.
 */

#define DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_C545_DROP_CELL_LIST_CAP_PC34_COMPAT 4

enum {
    DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT = 0xFFFF,
    DM1_V1_MIRROR_C545_DROP_END_PC34_COMPAT = 0xFFFE,
    DM1_V1_MIRROR_C545_DROP_C160_RESURRECT_PC34_COMPAT = 160,
    DM1_V1_MIRROR_C545_DROP_C162_CANCEL_PC34_COMPAT = 162,
    DM1_V1_MIRROR_C545_DROP_C30_SLOT_PC34_COMPAT = 30,
    DM1_V1_MIRROR_C545_DROP_C37_SLOT_PC34_COMPAT = 37,
    DM1_V1_MIRROR_C545_DROP_C38_SLOT_BOX_PC34_COMPAT = 38,
    DM1_V1_MIRROR_C545_DROP_C040_PANEL_PC34_COMPAT = 40,
    DM1_V1_MIRROR_C545_DROP_C070_MOUTH_PC34_COMPAT = 70,
    DM1_V1_MIRROR_C545_DROP_C545_ZONE_PC34_COMPAT = 545,
    DM1_V1_MIRROR_C545_DROP_M569_CHEST_PANEL_PC34_COMPAT = 4,
    DM1_V1_MIRROR_C545_DROP_M568_CANDIDATE_PANEL_PC34_COMPAT = 5
};

typedef struct Dm1V1MirrorC545DropEvidencePc34Compat {
    int contractOnly;
    const char *chestCloseAnchor;
    const char *leaderHandPutAnchor;
    const char *leaderHandRemoveAnchor;
    const char *commandDispatchAnchor;
    const char *commandQueueAnchor;
    const char *candidateOpenAnchor;
    const char *candidateClickAnchor;
    const char *panelAnchor;
    const char *mouseAnchor;
    const char *blitmaskAnchor;
    const char *defsAnchor;
    const char *contractScope;
} Dm1V1MirrorC545DropEvidencePc34Compat;

typedef struct Dm1V1MirrorC545DropSpecPc34Compat {
    int leaderIndex;
    int partyChampionCount;
    unsigned int candidateOrdinal;
    int partyTailChampion;
    int leaderHandThing;
    int previousCellThing;
    int openChestThing;
    int panelGraphic;
    int panelId;
    int c545Zone;
    int c070Command;
} Dm1V1MirrorC545DropSpecPc34Compat;

typedef struct Dm1V1MirrorC545DropStatePc34Compat {
    int contractOnly;
    int leaderIndex;
    int partyChampionCount;
    int partyTailChampion;
    int leaderHandThing;
    unsigned int candidateOrdinal;
    unsigned int g0299CandidateOrdinal;
    int panelOpen;
    int panelContent;
    int openChestThing;
    int cellThings[DM1_V1_MIRROR_C545_DROP_CELL_LIST_CAP_PC34_COMPAT];
    int cellThingCount;
    int chestSlots[DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT];
    int f0334CloseCount;
    int f0298RemoveCount;
    int f0378DispatchCount;
    int f0380QueueCount;
    int f0280OpenCount;
    int f0282CancelCount;
    int f0297PutCount;
    int cellThingAddedCount;
    int mouseEnableCount;
    int mouseDisableCount;
    int panelRedrawCount;
    int blitmaskCount;
    int mutationGuardCount;
} Dm1V1MirrorC545DropStatePc34Compat;

typedef struct Dm1V1MirrorC545DropResultPc34Compat {
    const Dm1V1MirrorC545DropEvidencePc34Compat *evidence;
    const Dm1V1MirrorC545DropSpecPc34Compat *spec;
    int accepted;
    int initialLeaderHand;
    int initialPanelOpen;
    int initialPanelContent;
    int initialOpenChestThing;
    int initialPartyTailChampion;
    int initialCellThingCount;
    unsigned int initialCandidateOrdinal;
    int droppedThing;
    int firstCellThing;
    int droppedCellThing;
    int f0334CloseCount;
    int f0298RemoveCount;
    int f0378DispatchCount;
    int f0380QueueCount;
    int f0280OpenCount;
    int f0282CancelCount;
    int f0297PutCount;
    int cellThingAddedCount;
    int leaderHandEmpty;
    int panelOpen;
    unsigned int candidateOrdinal;
    int openChestThing;
    int partyTailChampion;
    int thenResurrectClickClearsCandidate;
    int mutationGuardsOk;
    int panelOpenAfterResurrectClick;
    unsigned int candidateOrdinalAfterResurrectClick;
    int partyTailAfterResurrectClick;
    int openChestAfterResurrectClick;
    int cellThingCountAfterDrop;
    int cellThingCountAfterResurrectClick;
    int chestSlotsBefore[DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT];
    int chestSlotsAfterDrop[DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT];
    int chestSlotsAfterResurrectClick
        [DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT];
    int rejectsNullState;
    int rejectsNullResult;
    int rejectsNonContract;
    int rejectsNoPanel;
    int rejectsNoCandidate;
    int rejectsEmptyLeaderHand;
    int rejectsNoOpenChest;
    int rejectsTailMismatch;
} Dm1V1MirrorC545DropResultPc34Compat;

void DM1_V1_MirrorCandidateC545DropWhilePanelLive_InitPc34Compat(
    Dm1V1MirrorC545DropStatePc34Compat *state);

int DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
    Dm1V1MirrorC545DropStatePc34Compat *state,
    Dm1V1MirrorC545DropResultPc34Compat *outResult);

const Dm1V1MirrorC545DropEvidencePc34Compat *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_EvidencePc34Compat(void);

const Dm1V1MirrorC545DropSpecPc34Compat *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_SpecPc34Compat(void);

const char *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
