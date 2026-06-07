#ifndef FIRESTAFF_DM1_V1_CHEST_RUNTIME_ENCUMBRANCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_RUNTIME_ENCUMBRANCE_PC34_COMPAT_H

#include "dm1_v1_inventory_chest_load_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_RUNTIME_ENCUMBRANCE_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT
};

typedef struct {
    int leaderBaseSetResult;
    int bystanderBaseSetResult;
    int leaderBaseLoad;
    int bystanderLoadBeforeOpen;
    int openResult;
    int openChestThingAfterOpen;
    int visibleContentsWeight;
    int openContainerWeight;
    int leaderLoadAfterOpen;
    int closeCount;
    int closeContainerWeightSnapshot;
    int closeContainerWeightAfter;
    int openChestThingAfterClose;
    int leaderLoadAfterClose;
    int bystanderLoadAfterClose;
    int closedItemTypes[DM1_PC34_CHEST_RUNTIME_ENCUMBRANCE_SLOT_COUNT];
    int closedItemWeights[DM1_PC34_CHEST_RUNTIME_ENCUMBRANCE_SLOT_COUNT];
} DM1_V1_ChestRuntimeEncumbranceProbePc34;

const char* dm1_v1_chest_runtime_encumbrance_source_evidence_pc34(void);
int dm1_v1_chest_runtime_encumbrance_run_pc34(
    DM1_V1_ChestRuntimeEncumbranceProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_RUNTIME_ENCUMBRANCE_PC34_COMPAT_H */
