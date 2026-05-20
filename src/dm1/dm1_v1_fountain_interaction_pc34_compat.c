#include "dm1_v1_fountain_interaction_pc34_compat.h"

#include <string.h>

static const char* kFountainSourceEvidence =
    "ReDMCSB source lock: DUNVIEW.C:2668-2690 maps wall ornament graphic 35 into "
    "G0268_ai_CurrentMapFountainOrnamentIndices; DUNGEON.C:1350-1363 F0601 tests "
    "whether a wall ornament is a fountain; DUNVIEW.C:3735-3744 and 7784-7788 set "
    "G0288_B_FacingFountain while drawing/resetting wall view state; CLIKVIEW.C:419-422 "
    "sets the inventory champion Water to 2048 and plays C08_SOUND_SWALLOW when an "
    "empty-handed leader clicks a fountain; CLIKVIEW.C:480-496 fills junk water/waterskin "
    "charges to 3 or changes empty flask C195 to water flask C163, redraws changed object "
    "icons, applies the object-weight delta to champion load, then routes to F0372 front-wall "
    "sensor touch; DEFS.H:68,1479,1518,1891-1892,1945,1950 define the sound, potion type, "
    "junk type, and icon constants; DUNGEON.C:80-103 maps water flask and empty flask object "
    "info rows.";

static void fountain_clear_result(DM1V1FountainResultPc34Compat* outResult) {
    if (!outResult) return;
    memset(outResult, 0, sizeof(*outResult));
    outResult->championWaterAfter = -1;
    outResult->leaderHandIconAfter = -1;
    outResult->leaderHandChargesAfter = -1;
    outResult->playSoundOrdinal = -1;
    outResult->sourceEvidence = kFountainSourceEvidence;
}

int DM1V1_Fountain_EvaluateFrontWallClickPc34Compat(
    DM1V1FountainClickInputPc34Compat input,
    DM1V1FountainResultPc34Compat* outResult) {
    if (!outResult) return 0;
    fountain_clear_result(outResult);
    outResult->touchFrontWallSensor = 1;

    if (!input.facingFountain) {
        return 1;
    }

    if (input.leaderEmptyHanded) {
        if (input.leaderIndex != DM1_V1_FOUNTAIN_NO_LEADER) {
            outResult->action = DM1_V1_FOUNTAIN_ACTION_DRINK;
            outResult->championWaterAfter = DM1_V1_FOUNTAIN_WATER_MAX;
            outResult->playSoundOrdinal = DM1_V1_FOUNTAIN_SWALLOW_SOUND;
        }
        return 1;
    }

    outResult->leaderHandIconAfter = input.leaderHandIconIndex;
    outResult->leaderHandChargesAfter = input.leaderHandCharges;
    if (input.leaderHandIconIndex >= DM1_V1_ICON_JUNK_WATER &&
        input.leaderHandIconIndex <= DM1_V1_ICON_JUNK_WATERSKIN) {
        outResult->action = DM1_V1_FOUNTAIN_ACTION_FILL_JUNK_WATER;
        outResult->leaderHandChargesAfter = DM1_V1_FOUNTAIN_FULL_CHARGES;
    } else if (input.leaderHandIconIndex == DM1_V1_ICON_POTION_EMPTY_FLASK) {
        outResult->action = DM1_V1_FOUNTAIN_ACTION_FILL_EMPTY_FLASK;
        outResult->leaderHandIconAfter = DM1_V1_ICON_POTION_WATER_FLASK;
    } else {
        return 1;
    }

    outResult->redrawObjectIcons = 1;
    outResult->leaderLoadDelta = input.leaderHandWeightAfter - input.leaderHandWeightBefore;
    return 1;
}

int DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(M11_Item* leaderHandItem,
                                                 int facingFountain,
                                                 int weightAfter,
                                                 DM1V1FountainResultPc34Compat* outResult) {
    DM1V1FountainClickInputPc34Compat input;
    DM1V1FountainResultPc34Compat localResult;
    if (!leaderHandItem) return 0;
    memset(&input, 0, sizeof(input));
    input.facingFountain = facingFountain;
    input.leaderIndex = 0;
    input.leaderEmptyHanded = 0;
    input.leaderHandIconIndex = leaderHandItem->itemType;
    input.leaderHandCharges = leaderHandItem->charges;
    input.leaderHandWeightBefore = leaderHandItem->weight;
    input.leaderHandWeightAfter = weightAfter;
    if (!DM1V1_Fountain_EvaluateFrontWallClickPc34Compat(input, &localResult)) return 0;
    if (localResult.action == DM1_V1_FOUNTAIN_ACTION_FILL_JUNK_WATER ||
        localResult.action == DM1_V1_FOUNTAIN_ACTION_FILL_EMPTY_FLASK) {
        leaderHandItem->itemType = localResult.leaderHandIconAfter;
        leaderHandItem->charges = localResult.leaderHandChargesAfter;
        leaderHandItem->weight = weightAfter;
    }
    if (outResult) *outResult = localResult;
    return 1;
}

int DM1V1_Fountain_ApplyDrinkPc34Compat(M11_FoodWaterState* state,
                                        int championIndex,
                                        int facingFountain,
                                        DM1V1FountainResultPc34Compat* outResult) {
    DM1V1FountainClickInputPc34Compat input;
    DM1V1FountainResultPc34Compat localResult;
    if (!state || championIndex < 0 || championIndex >= state->count) return 0;
    memset(&input, 0, sizeof(input));
    input.facingFountain = facingFountain;
    input.leaderIndex = championIndex;
    input.leaderEmptyHanded = 1;
    if (!DM1V1_Fountain_EvaluateFrontWallClickPc34Compat(input, &localResult)) return 0;
    if (localResult.action == DM1_V1_FOUNTAIN_ACTION_DRINK) {
        state->champions[championIndex].water = localResult.championWaterAfter;
        state->champions[championIndex].thirsty = 0;
    }
    if (outResult) *outResult = localResult;
    return 1;
}

const char* DM1V1_Fountain_GetSourceEvidencePc34Compat(void) {
    return kFountainSourceEvidence;
}
