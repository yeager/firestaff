/*
 * dm1_v1_mirror_candidate_c545_drop_while_panel_live_pc34_compat.h
 *
 * Public surface for the DM1 V1 mirror-candidate C545 drop-while-panel-live
 * (leader-hand drop-to-floor while a C040 candidate panel is live) contract
 * regression.
 *
 * Source-locked against (ReDMCSB):
 *   CHEST.C    F0334:117-132    clears G0426 and relinks non-empty G0425 slots
 *   CHAMPION.C F0297:243-268    seeds the post-candidate leader-hand object
 *   CHAMPION.C F0298:270-298    removes it for the drop-to-floor mutation
 *   COMMAND.C  F0378:1973-1983  dispatches panel input
 *   COMMAND.C  F0380:2045-2159  keeps queued command identity stable until
 *                               the C545 mutation runs
 *   REVIVE.C   F0280:124-132    publishes G0299 only while the hand is empty
 *   REVIVE.C   F0282:744-806    clears G0299 on the later C040 click
 *   PANEL.C    F0346/F0347:1619-1657 keeps C040 drawn while G0299 is non-zero
 *   UTAMSCR.C  F0077/F0078:141-150 brackets pointer redraws
 *   BLITMASK.C F0133:30-33      anchors the masked redraw path used by the
 *                               live panel
 */

#ifndef DM1_V1_MIRROR_CANDIDATE_C545_DROP_WHILE_PANEL_LIVE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_C545_DROP_WHILE_PANEL_LIVE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Symbol constants used by both the module and the test. */
#define DM1_V1_MIRROR_C545_DROP_C040_PANEL_PC34_COMPAT 568
#define DM1_V1_MIRROR_C545_DROP_M568_CANDIDATE_PANEL_PC34_COMPAT 5694
#define DM1_V1_MIRROR_C545_DROP_C545_ZONE_PC34_COMPAT 0xC5
#define DM1_V1_MIRROR_C545_DROP_C070_MOUTH_PC34_COMPAT 0x70
#define DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_C545_DROP_NONE_PC34_COMPAT 0xffffu

/* Forward declarations so the order below can include Evidence in Result. */
typedef struct Dm1V1MirrorC545DropSpecPc34Compat Dm1V1MirrorC545DropSpecPc34Compat;
typedef struct Dm1V1MirrorC545DropEvidencePc34Compat Dm1V1MirrorC545DropEvidencePc34Compat;

/* Spec: pre-canonical contract fixture (leader hand seeded with an
 * open-chest thing, candidate panel live on champion 3). */
typedef struct Dm1V1MirrorC545DropSpecPc34Compat {
    int leaderIndex;
    int partyChampionCount;
    int candidateOrdinal;
    int partyTailChampion;
    int leaderHandThing;
    int previousCellThing;
    int openChestThing;
    int panelGraphic;
    int panelId;
    int c545Zone;
    int c070Command;
} Dm1V1MirrorC545DropSpecPc34Compat;

/* Forward declaration so the Result struct can hold a pointer to the
 * Evidence struct (which is defined later in this header). */
typedef struct Dm1V1MirrorC545DropEvidencePc34Compat Dm1V1MirrorC545DropEvidencePc34Compat;

/* Result: before/after telemetry for the C545 drop + C040 resurrect click. */
typedef struct Dm1V1MirrorC545DropResultPc34Compat {
    const Dm1V1MirrorC545DropSpecPc34Compat *spec;
    const Dm1V1MirrorC545DropEvidencePc34Compat *evidence;
    int accepted;
    int initialLeaderHand;
    int initialOpenChestThing;
    int initialPanelContent;
    int initialPanelOpen;
    int initialPartyTailChampion;
    int initialCandidateOrdinal;
    int initialCellThingCount;
    int chestSlotsBefore[DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT];
    int chestSlotsAfterDrop[DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT];
    int chestSlotsAfterResurrectClick[DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT];
    int leaderHandEmpty;
    int openChestThing;
    int droppedCellThing;
    int droppedThing;
    int firstCellThing;
    int cellThingCount;
    int cellThingCountAfterDrop;
    int cellThingCountAfterResurrectClick;
    int cellThingAddedCount;
    int partyTailChampion;
    int partyTailAfterResurrectClick;
    int candidateOrdinal;
    int candidateOrdinalAfterResurrectClick;
    int panelOpen;
    int panelOpenAfterResurrectClick;
    int openChestAfterResurrectClick;
    int thenResurrectClickClearsCandidate;
    int f0280OpenCount;
    int f0282CancelCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0334CloseCount;
    int f0378DispatchCount;
    int f0380QueueCount;
    int mutationGuardsOk;
    int rejectsNullState;
    int rejectsNullResult;
    int rejectsNonContract;
    int rejectsNoPanel;
    int rejectsNoCandidate;
    int rejectsNoOpenChest;
    int rejectsEmptyLeaderHand;
    int rejectsTailMismatch;
} Dm1V1MirrorC545DropResultPc34Compat;

/* Evidence: source-lock anchor strings + scope statement. */
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

/* State: live C040 panel + C545 mutation counters + chest slots. */
typedef struct Dm1V1MirrorC545DropStatePc34Compat {
    int partyChampionCount;
    int leaderIndex;
    int leaderHandThing;
    int previousCellThing;
    int openChestThing;
    int firstChestSlotThing;
    int candidateOrdinal;
    int partyTailChampion;
    int g0299CandidateOrdinal;
    int panelContent;
    int panelOpen;
    int c545Fired;
    int c040FiredAfterC545;
    int chestSlots[DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT];
    int chestSlotsPreserved;
    int leaderHandReleasedAfterMutation;
    int command;
    int contractOnly;
    int cellThingCount;
    int cellThingAddedCount;
    int cellThings[DM1_V1_MIRROR_C545_DROP_SLOT_COUNT_PC34_COMPAT];
    int f0280OpenCount;
    int f0282CancelCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0334CloseCount;
    int f0378DispatchCount;
    int f0380QueueCount;
    int panelRedrawCount;
    int mouseEnableCount;
    int mouseDisableCount;
    int mutationGuardCount;
    int blitmaskCount;
} Dm1V1MirrorC545DropStatePc34Compat;

const Dm1V1MirrorC545DropSpecPc34Compat *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_SpecPc34Compat(void);

const Dm1V1MirrorC545DropEvidencePc34Compat *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_EvidencePc34Compat(void);

const char *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_SourceEvidencePc34Compat(void);

void
DM1_V1_MirrorCandidateC545DropWhilePanelLive_InitPc34Compat(
    Dm1V1MirrorC545DropStatePc34Compat *state);

int
DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
    Dm1V1MirrorC545DropStatePc34Compat *state,
    Dm1V1MirrorC545DropResultPc34Compat *outResult);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_MIRROR_CANDIDATE_C545_DROP_WHILE_PANEL_LIVE_PC34_COMPAT_H */