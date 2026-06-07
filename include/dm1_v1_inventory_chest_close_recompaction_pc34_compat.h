#ifndef FIRESTAFF_DM1_V1_INVENTORY_CHEST_CLOSE_RECOMPACTION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_CHEST_CLOSE_RECOMPACTION_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT = 8,
    DM1_PC34_CHEST_CLOSE_RECOMPACTION_CHAIN_CAPACITY = 10,
    DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST = 0xFFFE,
    DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE = 0xFFFF,
    DM1_PC34_CHEST_CLOSE_RECOMPACTION_CONTAINER_BASE_WEIGHT = 50
};

typedef struct {
    int inputSlots[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT];
    int processFirstBefore[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT];
    int processFirstAfter[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT];
    int containerSlotBeforeClear;
    int containerSlotAfterEndMarker;
    int containerSlotAfterFirstHead;
    int openChestAfterReset;
    int chainHead;
    int chainCount;
    int chain[DM1_PC34_CHEST_CLOSE_RECOMPACTION_CHAIN_CAPACITY];
    int chainTerminator;
    int chainContainsNone;
    int firstThing;
    int firstThingNextBefore;
    int firstThingNextAfterSentinel;
    int firstThingNextAfterClose;
    int processFirstFalseSlot;
    int endMarkerEvent;
    int openChestResetEvent;
    int processFirstFalseEvent;
    int firstNextSentinelEvent;
    int headAssignEvent;
    int firstLinkCallEvent;
    int linkCallCount;
    int linkThingArgs[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT];
    int linkPreviousArgs[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT];
    int previousBeforeLink[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT];
    int previousAfterLink[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT];
    int linkCallEvents[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT];
    int nestedThing;
    int nestedThingNextAfterClose;
    int nestedThingInChain;
    int secondCloseNoOp;
    int secondCloseLinkCallDelta;
    int openChestAfterSecondClose;
    int containerSlotAfterSecondClose;
} DM1_V1_InventoryChestCloseRecompactionTracePc34;

typedef struct {
    DM1_V1_InventoryChestCloseRecompactionTracePc34 fullOrder;
    DM1_V1_InventoryChestCloseRecompactionTracePc34 sparseOrder;
    DM1_V1_InventoryChestCloseRecompactionTracePc34 guardOrder;
    DM1_V1_InventoryChestCloseRecompactionTracePc34 overfullOrder;
    DM1_V1_InventoryChestCloseRecompactionTracePc34 emptyOrder;

    int baseWeightCloseCount;
    int baseWeightParentContainerWeight;
    int baseWeightFirstChildWeight;
    int baseWeightSecondChildWeight;
    int baseWeightContentsOnly;
    int baseWeightExpectedChampionLoad;
    int baseWeightChampionLoadAfterF0301;
    int baseWeightF0301Calls;
    int baseWeightF0140Calls;
} DM1_V1_InventoryChestCloseRecompactionProbePc34;

const char* dm1_inventory_chest_close_recompaction_source_evidence_pc34(void);
int m11_inventory_pc34_probe_chest_close_recompaction(
    DM1_V1_InventoryChestCloseRecompactionProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_CHEST_CLOSE_RECOMPACTION_PC34_COMPAT_H */
