#ifndef DM1_V1_MIRROR_CANDIDATE_C545_PICKUP_WHILE_PANEL_LIVE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_C545_PICKUP_WHILE_PANEL_LIVE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB anchors: CHEST.C F0333:30-67 and F0334:117-132 bracket chest
 * inventory materialization/close while C545-compatible pickup/drop paths are
 * nearby; CHAMPION.C F0297:243-268, F0298:270-298, F0300:511-584,
 * F0301:606-660, and F0302:662-713 own leader-hand priority and slot
 * exchange/toggle order; COMMAND.C F0378:1973-1983 and F0380:2045-2159 own
 * command dispatch and pickup flow; REVIVE.C F0280:124-132 and
 * F0282:744-806 must not run for the pickup mutation; PANEL.C
 * F0346/F0347:1619-1657 redraws C040 while the candidate is live;
 * UTAMSCR.C F0077/F0078:141-150 brackets action dispatch screen updates;
 * OBJECT.C F0033:147-212 supplies object-category/icon lookup; BLITMASK.C
 * F0133:30-33 anchors masked redraw; DEFS.H:338-340, 810-817, 1874-1878,
 * 2200, 3001-3008, 3906-3913, 4205-4207, 5694, and 5876-5881 name C162,
 * C30..C37, C38, C040, M568/M569, C537..C544, ornament, G0299, and
 * G0425/G0426.
 */

#define DM1_V1_MIRROR_C545_PICKUP_FLOOR_CAP_PC34_COMPAT 4
#define DM1_V1_MIRROR_C545_PICKUP_SLOT_COUNT_PC34_COMPAT 8

enum {
    DM1_V1_MIRROR_C545_PICKUP_NONE_PC34_COMPAT = 0xFFFF,
    DM1_V1_MIRROR_C545_PICKUP_C162_CANCEL_PC34_COMPAT = 162,
    DM1_V1_MIRROR_C545_PICKUP_C30_SLOT_PC34_COMPAT = 30,
    DM1_V1_MIRROR_C545_PICKUP_C37_SLOT_PC34_COMPAT = 37,
    DM1_V1_MIRROR_C545_PICKUP_C38_BOX_PC34_COMPAT = 38,
    DM1_V1_MIRROR_C545_PICKUP_C040_PANEL_PC34_COMPAT = 40,
    DM1_V1_MIRROR_C545_PICKUP_C070_MOUTH_PC34_COMPAT = 70,
    DM1_V1_MIRROR_C545_PICKUP_C537_VISIBLE_PC34_COMPAT = 537,
    DM1_V1_MIRROR_C545_PICKUP_C544_VISIBLE_PC34_COMPAT = 544,
    DM1_V1_MIRROR_C545_PICKUP_C545_ZONE_PC34_COMPAT = 545,
    DM1_V1_MIRROR_C545_PICKUP_M569_CHEST_PANEL_PC34_COMPAT = 4,
    DM1_V1_MIRROR_C545_PICKUP_M568_CANDIDATE_PANEL_PC34_COMPAT = 5,
    DM1_V1_MIRROR_C545_PICKUP_G0299_CANDIDATE_PC34_COMPAT = 299,
    DM1_V1_MIRROR_C545_PICKUP_G0425_CHEST_LIST_PC34_COMPAT = 425,
    DM1_V1_MIRROR_C545_PICKUP_G0426_OPEN_CHEST_PC34_COMPAT = 426
};

typedef struct Dm1V1MirrorC545PickupEvidencePc34Compat {
    int sourceLockedContractOnly;
    int noRealAssetBitmapParity;
    int noGameDataLoad;
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *championPutAnchor;
    const char *championRemoveAnchor;
    const char *championSlotRemoveAnchor;
    const char *championSlotAddAnchor;
    const char *championSlotDispatchAnchor;
    const char *commandDispatchAnchor;
    const char *commandPickupAnchor;
    const char *reviveOpenAnchor;
    const char *reviveClickAnchor;
    const char *panelAnchor;
    const char *utilityAnchor;
    const char *objectAnchor;
    const char *blitmaskAnchor;
    const char *defsAnchor;
    const char *contractScope;
} Dm1V1MirrorC545PickupEvidencePc34Compat;

typedef struct Dm1V1MirrorC545PickupSpecPc34Compat {
    unsigned int deterministicSeed;
    int leaderIndex;
    int partyChampionCount;
    unsigned int candidateOrdinal;
    int c545FloorThing;
    int alreadyHeldThing;
    int previousFloorThing;
    int c040PanelGraphic;
    int c040PanelContent;
    int c545Zone;
    int c162CloseCommand;
} Dm1V1MirrorC545PickupSpecPc34Compat;

typedef struct Dm1V1MirrorC545PickupStatePc34Compat {
    int sourceLockedContractOnly;
    int noRealAssetBitmapParity;
    int noGameDataLoad;
    int leaderIndex;
    int partyChampionCount;
    unsigned int candidateOrdinal;
    unsigned int g0299CandidateOrdinal;
    int leaderHandThing;
    int floorThings[DM1_V1_MIRROR_C545_PICKUP_FLOOR_CAP_PC34_COMPAT];
    int floorThingCount;
    int c040PanelOpen;
    int c040PanelContent;
    int c040LayerCookie;
    int c040ZOrderCookie;
    int c545Zone;
    int visibleC537ToC544
        [DM1_V1_MIRROR_C545_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int g0425ChestSlots
        [DM1_V1_MIRROR_C545_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0300SlotRemoveCount;
    int f0301SlotAddCount;
    int f0302SlotDispatchCount;
    int f0378DispatchCount;
    int f0380PickupFlowCount;
    int f0280ReviveOpenCount;
    int f0282ReviveClickCount;
    int f0346PanelDrawCount;
    int f0347PanelRefreshCount;
    int f0077EnableCount;
    int f0078DisableCount;
    int f0033ObjectLookupCount;
    int f0133MaskCount;
    int c040CloseCount;
    int handTransitionCount;
    int panelRejectionCount;
    int mirrorCandidateGuardCount;
    int panelFlickerCount;
    int zOrderCorruptionCount;
    int itemLostCount;
    int mirrorCandidateSideEffectCount;
} Dm1V1MirrorC545PickupStatePc34Compat;

typedef struct Dm1V1MirrorC545PickupResultPc34Compat {
    const Dm1V1MirrorC545PickupEvidencePc34Compat *evidence;
    const Dm1V1MirrorC545PickupSpecPc34Compat *spec;
    int accepted;
    int negativeRejected;
    unsigned int deterministicHash;
    unsigned int initialPanelHash;
    unsigned int afterPickupPanelHash;
    unsigned int afterClosePanelHash;
    int initialLeaderHand;
    int finalLeaderHand;
    int pickedThing;
    int initialFloorThingCount;
    int floorThingCountAfterPickup;
    int floorThingCountAfterClose;
    unsigned int initialCandidateOrdinal;
    unsigned int candidateOrdinalAfterPickup;
    unsigned int candidateOrdinalAfterClose;
    int panelOpenAfterPickup;
    int panelOpenAfterClose;
    int panelStableAfterPickup;
    int noPanelFlicker;
    int noZOrderCorruption;
    int noMirrorCandidateSideEffect;
    int noReviveTriggerOnPickup;
    int closeCleanWithHandFull;
    int c040Redraws;
    int handTransitions;
    int panelRejections;
    int mirrorCandidateGuard;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0300SlotRemoveCount;
    int f0301SlotAddCount;
    int f0302SlotDispatchCount;
    int f0378DispatchCount;
    int f0380PickupFlowCount;
    int f0280ReviveOpenCount;
    int f0282ReviveClickCount;
    int f0346PanelDrawCount;
    int f0347PanelRefreshCount;
    int f0077EnableCount;
    int f0078DisableCount;
    int f0033ObjectLookupCount;
    int f0133MaskCount;
    int negativeInitialHand;
    int negativeFinalHand;
    int negativeInitialFloorCount;
    int negativeFinalFloorCount;
    int negativePanelStable;
    unsigned int negativeCandidateOrdinal;
    int negativeItemLost;
    int rejectsNullState;
    int rejectsNullResult;
    int rejectsNonContract;
    int rejectsNoPanel;
    int rejectsNoCandidate;
    int rejectsHandFull;
    int rejectsNoFloorItem;
    int rejectsWrongZone;
} Dm1V1MirrorC545PickupResultPc34Compat;

void DM1_V1_MirrorCandidateC545PickupWhilePanelLive_InitPc34Compat(
    Dm1V1MirrorC545PickupStatePc34Compat *state);

int DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
    Dm1V1MirrorC545PickupStatePc34Compat *state,
    Dm1V1MirrorC545PickupResultPc34Compat *outResult);

const Dm1V1MirrorC545PickupEvidencePc34Compat *
DM1_V1_MirrorCandidateC545PickupWhilePanelLive_EvidencePc34Compat(void);

const Dm1V1MirrorC545PickupSpecPc34Compat *
DM1_V1_MirrorCandidateC545PickupWhilePanelLive_SpecPc34Compat(void);

const char *
DM1_V1_MirrorCandidateC545PickupWhilePanelLive_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
