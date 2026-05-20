#ifndef FIRESTAFF_DM1_V1_FOUNTAIN_INTERACTION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_FOUNTAIN_INTERACTION_PC34_COMPAT_H

#include "dm1_v1_food_water_pc34_compat.h"
#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_FOUNTAIN_NO_LEADER (-1)
#define DM1_V1_FOUNTAIN_WATER_MAX 2048
#define DM1_V1_FOUNTAIN_FULL_CHARGES 3
#define DM1_V1_FOUNTAIN_SWALLOW_SOUND 8

#define DM1_V1_ICON_JUNK_WATER 8
#define DM1_V1_ICON_JUNK_WATERSKIN 9
#define DM1_V1_ICON_POTION_WATER_FLASK 163
#define DM1_V1_ICON_POTION_EMPTY_FLASK 195

typedef enum DM1V1FountainActionPc34Compat {
    DM1_V1_FOUNTAIN_ACTION_NONE = 0,
    DM1_V1_FOUNTAIN_ACTION_DRINK = 1,
    DM1_V1_FOUNTAIN_ACTION_FILL_JUNK_WATER = 2,
    DM1_V1_FOUNTAIN_ACTION_FILL_EMPTY_FLASK = 3
} DM1V1FountainActionPc34Compat;

typedef struct DM1V1FountainClickInputPc34Compat {
    int facingFountain;
    int leaderIndex;
    int leaderEmptyHanded;
    int leaderHandIconIndex;
    int leaderHandCharges;
    int leaderHandWeightBefore;
    int leaderHandWeightAfter;
} DM1V1FountainClickInputPc34Compat;

typedef struct DM1V1FountainResultPc34Compat {
    DM1V1FountainActionPc34Compat action;
    int championWaterAfter;
    int leaderHandIconAfter;
    int leaderHandChargesAfter;
    int leaderLoadDelta;
    int redrawObjectIcons;
    int playSoundOrdinal;
    int touchFrontWallSensor;
    const char* sourceEvidence;
} DM1V1FountainResultPc34Compat;

int DM1V1_Fountain_EvaluateFrontWallClickPc34Compat(
    DM1V1FountainClickInputPc34Compat input,
    DM1V1FountainResultPc34Compat* outResult);
int DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(M11_Item* leaderHandItem,
                                                 int facingFountain,
                                                 int weightAfter,
                                                 DM1V1FountainResultPc34Compat* outResult);
int DM1V1_Fountain_ApplyDrinkPc34Compat(M11_FoodWaterState* state,
                                        int championIndex,
                                        int facingFountain,
                                        DM1V1FountainResultPc34Compat* outResult);
const char* DM1V1_Fountain_GetSourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_FOUNTAIN_INTERACTION_PC34_COMPAT_H */
