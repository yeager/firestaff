#ifndef FIRESTAFF_DM1_V1_CHEST_CLOSE_REWIRE_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_CLOSE_REWIRE_RUNTIME_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_CLOSE_REWIRE_TEST_STAFF_OF_CLAWS_OBJECT_INFO = 27,
    DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT
};

typedef struct {
    int baseLoadSetResult;
    int baseLoad;

    int openResult;
    int openVisibleWeight;
    int openContainerWeight;
    int loadAfterOpen;

    int incompatibleSlotMask;
    int incompatibleStaffCanEquip;
    int incompatibleClickResult;
    int incompatibleLeaderHandBefore;
    int incompatibleLeaderHandAfter;
    int incompatibleSlotBefore;
    int incompatibleSlotAfter;
    int loadAfterIncompatibleAttempt;
    int visibleBeforeRejectTypes[DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT];
    int visibleAfterRejectTypes[DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT];

    int replacementCanEquip;
    int replacementClickResult;
    int leaderHandAfterReplacement;
    int finalSlotAfterReplacement;
    int replacementVisibleWeight;
    int replacementContainerWeight;
    int loadAfterReplacement;

    int hiddenTailInput;
    int closeCount;
    int closeContainerWeightSnapshot;
    int closeContainerWeightAfter;
    int loadAfterClose;
    int hiddenTailClosed;
    int closedTypes[DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT];

    int reopenResult;
    int reopenVisibleCount;
    int reopenVisibleWeight;
    int reopenContainerWeight;
    int loadAfterReopen;
    int hiddenTailReopened;
    int reopenedTypes[DM1_PC34_CHEST_CLOSE_REWIRE_TEST_SLOT_COUNT];
} DM1_V1_ChestCloseRewireRuntimeProbePc34;

const char* dm1_v1_chest_close_rewire_runtime_source_evidence_pc34(void);
int m11_inventory_pc34_probe_chest_close_rewire_runtime(
    DM1_V1_ChestCloseRewireRuntimeProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_CLOSE_REWIRE_RUNTIME_PC34_COMPAT_H */
