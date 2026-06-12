#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C045_FOOD_WATER_CLOSE_NO_CANDIDATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C045_FOOD_WATER_CLOSE_NO_CANDIDATE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_C045_FW_SLOT_COUNT_PC34 8
#define DM1_V1_MC_C045_FW_NONE_PC34 0xffffu
#define DM1_V1_MC_C045_FW_END_PC34 0xfffeu

typedef struct {
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *championAnchor;
    const char *panelFoodWaterAnchor;
    const char *panelCloseAnchor;
    const char *reviveOpenAnchor;
    const char *reviveC040Anchor;
    const char *commandAnchor;
    const char *defsAnchor;
    const char *disjointness;
} Dm1V1MirrorCandidateC045FoodWaterCloseEvidencePc34Compat;

typedef struct {
    int contractOnly;
    int inventoryChampionOrdinal;
    int leaderIndex;
    int sourceChestSlotIndex;
    uint16_t openChestThing;
    uint16_t sourceChestThing;
    uint16_t foodThing;
    uint16_t waterThing;
    uint16_t sourceChestChain[DM1_V1_MC_C045_FW_SLOT_COUNT_PC34];
    uint16_t g0425Slots[DM1_V1_MC_C045_FW_SLOT_COUNT_PC34];
    uint16_t championSwitchC30Thing;
    int championSwitchSourceSlot;
    int c144EyeDispatches;
    int c503CloseDispatches;
    int c018CloseCommand;
    int panelContent;
    int panelGraphic;
    int panelOpen;
    int c040ResurrectPendingOrdinal;
    int c040PanelOpened;
    int f0282Entered;
    int f0280CandidateGateChecked;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300RemoveSlotCount;
    int f0301AddSlotCount;
    int f0302SlotCommandCount;
    int f0344BarReadCount;
    int f0345FoodWaterDrawCount;
    int f0354CloseCount;
    int f0359C040DispatchCount;
    int closeReleasedC30ToChestChain;
    int preservedFoodBeforeClose;
    int preservedWaterBeforeClose;
    int consumedFoodAfterClose;
    int consumedWaterAfterClose;
    uint32_t preHash;
    uint32_t openHash;
    uint32_t closeHash;
    uint32_t consumeHash;
} Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat;

typedef struct {
    int accepted;
    int openedByC144Eye;
    int openedC045FoodWater;
    int noC040OnOpen;
    int noF0282OnC045Close;
    int closeFromChestBoundState;
    int closeCommandC503C018;
    int releasedC30BackToSourceSlot;
    int sourceChainRestored;
    int g0425SlotsClearedAfterClose;
    int foodWaterPanelStatePreserved;
    int consumptionReadPreservedFood;
    int consumptionReadPreservedWater;
    int foodAfterConsumption;
    int waterAfterConsumption;
    int c040PendingStillPending;
    int c040PanelStillClosed;
    int chestClosedBeforeFoodWaterDraw;
    int championSwitchSlotWasTransient;
    int disjointFromC040CandidatePath;
    int guardRejectsInvalidChest;
    int guardRejectsNoFoodThing;
    int guardRejectsWrongPanel;
    uint16_t restoredThing;
    uint16_t restoredChain[DM1_V1_MC_C045_FW_SLOT_COUNT_PC34];
    uint16_t g0425AfterClose[DM1_V1_MC_C045_FW_SLOT_COUNT_PC34];
    uint32_t hash;
} Dm1V1MirrorCandidateC045FoodWaterCloseResultPc34Compat;

void dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_init_pc34(
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *state);

int dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_run_pc34(
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *state,
    Dm1V1MirrorCandidateC045FoodWaterCloseResultPc34Compat *result);

const Dm1V1MirrorCandidateC045FoodWaterCloseEvidencePc34Compat *
dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_evidence_pc34(void);

const char *
dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
