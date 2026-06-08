#ifndef FIRESTAFF_DM1_V1_CHEST_NINTH_ITEM_HIDDEN_TAIL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_NINTH_ITEM_HIDDEN_TAIL_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_MAX_LINKED = 9,
    DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_VISIBLE_FIRST = 800,
    DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT = 808,
    DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_CHEST_THING = 0x7A80,
    DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_REOPEN_THING = 0x7A81
};

typedef struct {
    const char* contractMarker;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int chestSlotCount;
    int maxLinkedInputCount;
    int visibleFirstItemType;
    int hiddenTailItemType;
} DM1_V1_ChestNinthItemHiddenTailSpecPc34;

typedef struct {
    int sourceLockedContractOnly;
    int chestThing;
    int reopenedChestThing;

    int linkedInputCount;
    int openResult;
    int openThing;
    int openedVisibleCount;
    int openedTypes[DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT];
    int openedHiddenTailVisible;
    int openedHiddenTailType;
    int openedHiddenTailPreserved;
    int openedOrderMatchesInput;

    int leaderHandBeforePut;
    int leaderHandCanEnterContainer;
    int hiddenTailPutResult;
    int hiddenTailChainCountAfterPut;
    int hiddenTailTypeAfterPut;
    int hiddenTailVisibleAfterPut;
    int visibleCountAfterPut;
    int afterPutTypes[DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT];
    int afterPutOrderMatchesInput;
    int leaderHandAfterPut;

    int closeCount;
    int closedTypes[DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT];
    int hiddenTailReturnedByVisibleClose;
    int leaderHandAfterClose;

    int reopenResult;
    int reopenedVisibleCount;
    int reopenedTypes[DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT];
    int hiddenTailVisibleAfterReopen;
    int hiddenTailChainCountAfterReopen;
    int hiddenTailTypeAfterReopen;
    int reopenedOrderMatchesInput;
    int leaderHandAfterReopen;
} DM1_V1_ChestNinthItemHiddenTailProbePc34;

extern const DM1_V1_ChestNinthItemHiddenTailSpecPc34
    dm1_v1_chest_ninth_item_hidden_tail_pc34_spec;

const char* dm1_v1_chest_ninth_item_hidden_tail_source_evidence_pc34(void);
const DM1_V1_ChestNinthItemHiddenTailSpecPc34*
dm1_v1_chest_ninth_item_hidden_tail_spec_pc34(void);
int dm1_v1_chest_ninth_item_hidden_tail_pc34(
    DM1_V1_ChestNinthItemHiddenTailProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_NINTH_ITEM_HIDDEN_TAIL_PC34_COMPAT_H */
