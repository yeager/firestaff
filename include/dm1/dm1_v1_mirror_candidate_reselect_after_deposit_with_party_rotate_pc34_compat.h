#ifndef DM1_V1_MIRROR_CANDIDATE_RESELECT_AFTER_DEPOSIT_WITH_PARTY_ROTATE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RESELECT_AFTER_DEPOSIT_WITH_PARTY_ROTATE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_MCRADPR_CHAMPION_COUNT_PC34 = 4,
    DM1_V1_MCRADPR_CHEST_SLOT_COUNT_PC34 = 8,
    DM1_V1_MCRADPR_THING_NONE_PC34 = 0xFFFF,
    DM1_V1_MCRADPR_M568_PANEL_PC34 = 568,
    DM1_V1_MCRADPR_C040_GRAPHIC_PC34 = 40,
    DM1_V1_MCRADPR_C160_RESURRECT_PC34 = 160,
    DM1_V1_MCRADPR_C162_CANCEL_PC34 = 162,
    DM1_V1_MCRADPR_C001_TURN_LEFT_PC34 = 1,
    DM1_V1_MCRADPR_C002_TURN_RIGHT_PC34 = 2
};

typedef struct Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateEvidencePc34Compat {
    int contractOnly;
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *championPutAnchor;
    const char *championRemoveAnchor;
    const char *championSlotRemoveAnchor;
    const char *championSlotAddAnchor;
    const char *championSlotClickAnchor;
    const char *commandPanelAnchor;
    const char *commandQueueAnchor;
    const char *reviveOpenAnchor;
    const char *reviveClickAnchor;
    const char *panelRedrawAnchor;
    const char *mouseAnchor;
    const char *objectAnchor;
    const char *blitmaskAnchor;
    const char *defsAnchor;
    const char *partyRotateAnchor;
    const char *nonOverlapScope;
} Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat {
    int contractOnly;
    int partyChampionCount;
    int leaderIndex;
    int partyDirection;
    int candidateChampionIndex;
    int candidateOrdinal;
    int championCell[DM1_V1_MCRADPR_CHAMPION_COUNT_PC34];
    int championDirection[DM1_V1_MCRADPR_CHAMPION_COUNT_PC34];
    unsigned int leaderHandThing;
    int leaderHandEmpty;
    unsigned int pendingDepositThing;
    int depositPending;
    unsigned int candidateSlotThing;
    int candidateSlotFilled;
    unsigned int depositedThing;
    int g0299CandidateOrdinal;
    int c040PanelOpen;
    int panelContent;
    int c040Graphic;
    int candidateReferenceDirection;
    int reopenedReferenceDirection;
    int openChestThing;
    int chestSlots[DM1_V1_MCRADPR_CHEST_SLOT_COUNT_PC34];
    int f0280OpenCount;
    int f0282ClickCount;
    int f0282CancelCount;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0300RemoveCount;
    int f0301AddCount;
    int f0302SlotClickCount;
    int f0378PanelDispatchCount;
    int f0380QueueDispatchCount;
    int f0346PanelDrawCount;
    int f0347PanelRedrawCount;
    int f0077EnableCount;
    int f0078DisableCount;
    int f0033IconCount;
    int f0133BlitmaskCount;
    int f0284SetPartyDirectionCount;
    int f0365TurnPartyCount;
    int objectTransferCount;
    int depositFireCount;
    int closeNoopCount;
    int doubleFireCount;
    int staleDepositRejectCount;
    int noCrashGuard;
} Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat;

typedef struct Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat {
    const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateEvidencePc34Compat *evidence;
    int openedInitialCandidate;
    int putDepositInLeaderHand;
    int depositFired;
    int mirrorClosedAfterDeposit;
    int objectTransferred;
    int g0425ResetAfterDeposit;
    int g0426ResetAfterDeposit;
    int g0299ResetAfterDeposit;
    int c040ResetAfterDeposit;
    int leaderHandEmptyAfterDeposit;
    int noPendingDepositAfterDeposit;
    int partyRotated;
    int partyDirectionBeforeRotate;
    int partyDirectionAfterRotate;
    int championCellsRotated;
    int championDirectionsRotated;
    int reopenedCandidate;
    int reopenUsesNewDirection;
    int reopenHasCleanDepositState;
    int reopenHasCleanPanelState;
    int reopenG0299CandidateOrdinal;
    int noLeftoverC040BeforeReopen;
    int noLeftoverChestBeforeReopen;
    int noCrash;
    int closeAfterReopen;
    int closeAfterReopenNoDoubleFire;
    int secondCloseNoop;
    int doubleFireCountAfterClose;
    int f0280OpenCountAfterReopen;
    int f0282ClickCountAfterDeposit;
    int f0334CloseCountAfterDeposit;
    int f0378DispatchCountAfterDeposit;
    int f0380DispatchCountAfterRotate;
    int f0346PanelDrawCountAfterReopen;
    int f0347PanelRedrawCountAfterReopen;
    unsigned int leaderHandThingBeforeDeposit;
    unsigned int leaderHandThingAfterDeposit;
    unsigned int depositedThing;
    int candidateReferenceDirectionBeforeDeposit;
    int candidateReferenceDirectionAfterReopen;
    int contractOnly;
    int noAssetsOrPixelParity;
    unsigned int hash;
    int ok;
} Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat;

void DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_InitPc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state);

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_OpenCandidatePc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state);

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_PutDepositInLeaderHandPc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    unsigned int thing);

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_DepositViaMirrorPc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state);

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_RotatePartyPc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    int turnCommand);

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_CloseCandidatePc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state);

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_RunPc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat *outResult);

const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateEvidencePc34Compat *
DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_EvidencePc34Compat(void);

unsigned int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_HashPc34Compat(
    const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat *result);

#ifdef __cplusplus
}
#endif

#endif
