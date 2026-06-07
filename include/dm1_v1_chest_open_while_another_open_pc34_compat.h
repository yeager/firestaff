#ifndef FIRESTAFF_DM1_V1_CHEST_OPEN_WHILE_ANOTHER_OPEN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_OPEN_WHILE_ANOTHER_OPEN_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT = 8,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT = 3,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_B_COUNT = 8,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE = 0xFFFF,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST = 0xFFFE,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A = 0x0710,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_B = 0x0720,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1 = 4100,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1 = 5100,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_LEADER_HAND = 0x4C09,
    DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_LEADER_HAND_WEIGHT = 19
};

typedef struct {
    int contractOnly;
    int thingNone;
    int thingEndOfList;
    int slotCount;
    int chestAThing;
    int chestBThing;

    int initResult;
    int openAResult;
    int openBResult;
    int g0426BeforeOpenA;
    int g0426AfterOpenA;
    int g0426BeforeOpenB;
    int g0426AfterCloseReset;
    int g0426AfterOpenB;

    int mediaGuardFamilyPresent;
    int sameChestGuardBypassed;
    int anotherChestGuardTriggered;
    int closeCalledBeforeOpenB;
    int closeCallCount;
    int closeProcessedChest;
    int closeNonEmptyCount;
    int closeFirstNonEmptySlot;
    int closeClearedSlotsCount;
    int closeLinkThingToListCalls;
    int closeUnlinkThingFromListCalls;
    int candidateUnlinkThingFromListCalls;
    int processFirstFalseSlot;

    int openAItemCount;
    int openASlots[DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT];
    int chestASlotBeforeClose;
    int chestASlotAfterClose;
    int chestAClosedCount;
    int chestAClosedChain[DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT];
    int chestANextAfterClose[DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT];
    int chestAAllItemsStillLinked;
    int chestAOrderPreserved;

    int linkThingArgs[DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT];
    int linkPreviousArgs[DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT];
    int g0425AfterCloseBeforeOpenB[DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT];

    int openBItemCount;
    int openBPanelBlitCount;
    int openBPanelBlitThing;
    int openBPanelFullyPopulated;
    int openBSlots[DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT];
    int chestBSlotBeforeOpen;
    int chestBSlotAfterOpen;
    int chestBOrderPreserved;
    int chestBOrderLeakedA;

    int leaderHandBeforeThing;
    int leaderHandAfterThing;
    int leaderHandBeforeWeight;
    int leaderHandAfterWeight;
    int leaderEmptyBefore;
    int leaderEmptyAfter;

    int openAAssignEvent;
    int openAPanelBlitEvent;
    int openAMaterializeEvent;
    int closeGuardEvent;
    int closeOpenResetEvent;
    int closeContainerEndEvent;
    int firstHeadAssignEvent;
    int firstLinkCallEvent;
    int openBAssignEvent;
    int openBPanelBlitEvent;
    int openBMaterializeEvent;
} DM1_V1_ChestOpenWhileAnotherOpenStatePc34;

const char*
dm1_v1_chest_open_while_another_open_source_evidence_pc34_compat(void);
void dm1_v1_chest_open_while_another_open_init_pc34_compat(void);
int dm1_v1_chest_open_while_another_open_chest_a_then_b_pc34_compat(void);
const DM1_V1_ChestOpenWhileAnotherOpenStatePc34*
dm1_v1_chest_open_while_another_open_state_after_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_OPEN_WHILE_ANOTHER_OPEN_PC34_COMPAT_H */
