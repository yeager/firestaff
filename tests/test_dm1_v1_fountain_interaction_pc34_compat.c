#include "dm1_v1_fountain_interaction_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_int(const char* label, int actual, int expected) {
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s got %d expected %d\n", label, actual, expected);
        return 0;
    }
    return 1;
}

static int expect_evidence(const char* evidence) {
    return evidence &&
           strstr(evidence, "CLIKVIEW.C:419-422") &&
           strstr(evidence, "CLIKVIEW.C:480-496") &&
           strstr(evidence, "DUNGEON.C:1350-1363");
}

int main(void) {
    int ok = 1;
    M11_Item item;
    M11_FoodWaterState foodWater;
    DM1V1FountainClickInputPc34Compat input;
    DM1V1FountainResultPc34Compat result;

    printf("probe=dm1_v1_fountain_interaction_pc34_compat\n");
    printf("sourceEvidence=%s\n", DM1V1_Fountain_GetSourceEvidencePc34Compat());

    memset(&input, 0, sizeof(input));
    input.facingFountain = 1;
    input.leaderIndex = 0;
    input.leaderEmptyHanded = 1;
    ok &= DM1V1_Fountain_EvaluateFrontWallClickPc34Compat(input, &result);
    ok &= expect_int("empty hand drink action", result.action, DM1_V1_FOUNTAIN_ACTION_DRINK);
    ok &= expect_int("empty hand water max", result.championWaterAfter, DM1_V1_FOUNTAIN_WATER_MAX);
    ok &= expect_int("empty hand swallow sound", result.playSoundOrdinal, DM1_V1_FOUNTAIN_SWALLOW_SOUND);
    ok &= expect_int("empty hand still routes wall sensor", result.touchFrontWallSensor, 1);
    ok &= expect_evidence(result.sourceEvidence);

    input.leaderIndex = DM1_V1_FOUNTAIN_NO_LEADER;
    ok &= DM1V1_Fountain_EvaluateFrontWallClickPc34Compat(input, &result);
    ok &= expect_int("no leader no drink", result.action, DM1_V1_FOUNTAIN_ACTION_NONE);
    ok &= expect_int("no leader still routes wall sensor", result.touchFrontWallSensor, 1);

    memset(&item, 0, sizeof(item));
    item.itemType = DM1_V1_ICON_JUNK_WATERSKIN;
    item.charges = 0;
    item.weight = 2;
    ok &= DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(&item, 1, 5, &result);
    ok &= expect_int("waterskin fill action", result.action, DM1_V1_FOUNTAIN_ACTION_FILL_JUNK_WATER);
    ok &= expect_int("waterskin icon unchanged", item.itemType, DM1_V1_ICON_JUNK_WATERSKIN);
    ok &= expect_int("waterskin charge full", item.charges, DM1_V1_FOUNTAIN_FULL_CHARGES);
    ok &= expect_int("waterskin weight after", item.weight, 5);
    ok &= expect_int("waterskin load delta", result.leaderLoadDelta, 3);
    ok &= expect_int("waterskin redraw", result.redrawObjectIcons, 1);

    memset(&item, 0, sizeof(item));
    item.itemType = DM1_V1_ICON_POTION_EMPTY_FLASK;
    item.charges = 7;
    item.weight = 1;
    ok &= DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(&item, 1, 3, &result);
    ok &= expect_int("empty flask fill action", result.action, DM1_V1_FOUNTAIN_ACTION_FILL_EMPTY_FLASK);
    ok &= expect_int("empty flask becomes water flask", item.itemType, DM1_V1_ICON_POTION_WATER_FLASK);
    ok &= expect_int("empty flask keeps charge field", item.charges, 7);
    ok &= expect_int("empty flask weight after", item.weight, 3);
    ok &= expect_int("empty flask load delta", result.leaderLoadDelta, 2);

    memset(&item, 0, sizeof(item));
    item.itemType = 42;
    item.charges = 1;
    item.weight = 4;
    ok &= DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(&item, 1, 9, &result);
    ok &= expect_int("non-fillable item no action", result.action, DM1_V1_FOUNTAIN_ACTION_NONE);
    ok &= expect_int("non-fillable item unchanged", item.itemType, 42);
    ok &= expect_int("non-fillable weight unchanged", item.weight, 4);
    ok &= expect_int("non-fillable still routes wall sensor", result.touchFrontWallSensor, 1);

    memset(&item, 0, sizeof(item));
    item.itemType = DM1_V1_ICON_POTION_EMPTY_FLASK;
    item.charges = 0;
    item.weight = 1;
    ok &= DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(&item, 0, 3, &result);
    ok &= expect_int("not facing fountain no action", result.action, DM1_V1_FOUNTAIN_ACTION_NONE);
    ok &= expect_int("not facing fountain item unchanged", item.itemType, DM1_V1_ICON_POTION_EMPTY_FLASK);

    m11_fw_init(&foodWater, 1);
    foodWater.champions[0].water = 11;
    foodWater.champions[0].thirsty = 1;
    ok &= DM1V1_Fountain_ApplyDrinkPc34Compat(&foodWater, 0, 1, &result);
    ok &= expect_int("drink apply water", foodWater.champions[0].water, DM1_V1_FOUNTAIN_WATER_MAX);
    ok &= expect_int("drink apply clears thirsty", foodWater.champions[0].thirsty, 0);

    printf("fountainInteractionInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
