/*
 * ReDMCSB CHEST.C F0333 lines 30-32: returns when G0426_T_OpenChest already
 * equals the requested chest, before the close/re-materialize path can run.
 * ReDMCSB CHEST.C F0333 lines 53-76: first open copies linked contents into
 * G0425_aT_ChestSlots and fills empty visible slots with THING_NONE.
 * ReDMCSB CHAMPION.C F0302 lines 688-710: C30+ slot-box clicks swap the
 * leader hand with the current G0425_aT_ChestSlots entry.
 */
#ifndef FIRESTAFF_DM1_V1_CHEST_SAME_OPEN_NOOP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SAME_OPEN_NOOP_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_SAME_OPEN_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CHEST_SAME_OPEN_THING = 0x6A10,
    DM1_PC34_CHEST_SAME_OPEN_ITEM_A1 = 0x6101,
    DM1_PC34_CHEST_SAME_OPEN_ITEM_B1 = 0x7101
};

typedef struct {
    int setupResult;
    int firstOpenResult;
    int openThingAfterFirstOpen;
    int firstOpenSlotTypes[DM1_PC34_CHEST_SAME_OPEN_SLOT_COUNT];
    int firstOpenVisibleWeight;
    int pickupC538Result;
    int leaderHandAfterPickup;
    int c538AfterPickup;
    int visibleWeightAfterPickup;
    int loadAfterPickup;
    int sameOpenResult;
    int openThingAfterSameOpen;
    int leaderHandAfterSameOpen;
    int c537AfterSameOpen;
    int c538AfterSameOpen;
    int c539AfterSameOpen;
    int c540AfterSameOpen;
    int panelContentBeforeReplacingSameOpen;
    int replacingSameOpenResult;
    int panelContentAfterReplacingSameOpen;
    int openThingAfterReplacingSameOpen;
    int c538AfterReplacingSameOpen;
    int bItemsLeakedAfterSameOpen;
    int visibleWeightAfterSameOpen;
    int loadAfterSameOpen;
    int closeCountAfterSameOpen;
    int closedItemTypes[DM1_PC34_CHEST_SAME_OPEN_SLOT_COUNT];
} DM1_V1_ChestSameOpenNoopProbePc34;

const char* dm1_v1_chest_same_open_noop_source_evidence_pc34(void);
int dm1_v1_chest_same_open_noop_run_pc34(
    DM1_V1_ChestSameOpenNoopProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_SAME_OPEN_NOOP_PC34_COMPAT_H */
