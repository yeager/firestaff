#ifndef FIRESTAFF_DM1_V1_CHEST_REOPEN_CONTENTS_ORDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_REOPEN_CONTENTS_ORDER_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_COUNT = 5,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT =
        DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_MAX_INPUT = 9,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_ONE = 0,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_THREE = 1,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_FULL = 2,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_HIDDEN_TAIL = 3,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_LEADER_HELMET = 4,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_WEAPON = 31,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_POTION = 153,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_JUNK = 137,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_HIDDEN_TAIL = 808,
    DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_HELMET = 0x0040
};

typedef struct {
    int itemType;
    int weight;
    int allowedSlots;
} M11_GameView_ChestReopenContentsOrderItemPc34;

typedef struct {
    int inputCount;
    int openResult;
    int closeCount;
    int reopenResult;
    int reopenedVisibleCount;

    int inputTypes[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_MAX_INPUT];
    int openedTypes[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT];
    int closedTypes[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT];
    int reopenedTypes[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_SLOT_COUNT];

    int originalHead;
    int originalMiddle;
    int originalTail;
    int closedHead;
    int closedMiddle;
    int closedTail;
    int reopenedHead;
    int reopenedMiddle;
    int reopenedTail;

    int closedContainsHiddenTail;
    int reopenedContainsHiddenTail;
    int reopenedContainsEveryVisibleInput;
    int reopenedUniqueVisibleCount;
    int noDroppedOrDuplicatedVisibleItems;

    int leaderHandBeforeOpen;
    int leaderHandAfterOpen;
    int leaderHandAfterClose;
    int leaderHandAfterReopen;
    int leaderHandUnchangedAcrossCycle;
} M11_GameView_ChestReopenContentsOrderCasePc34;

typedef struct {
    int sourceLockedContractOnly;
    int c537Pc34Slot;
    int c544Pc34Slot;
    int chestSlotCount;
    M11_GameView_ChestReopenContentsOrderCasePc34
        cases[DM1_PC34_CHEST_REOPEN_CONTENTS_ORDER_CASE_COUNT];
} M11_GameView_ChestReopenContentsOrderProbePc34;

const char* M11_GameView_ChestReopenContentsOrderSourceEvidencePc34(void);
int M11_GameView_ChestReopenContentsOrderRunPc34(
    M11_GameView_ChestReopenContentsOrderProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_REOPEN_CONTENTS_ORDER_PC34_COMPAT_H */
