#include "dm1_v1_fountain_interaction_pc34_compat.h"
#include "dm1_v1_inventory_consumables_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* ReDMCSB source lock for the round-trip fill→drink regression cases
 * appended to this file:
 *  - CLIKVIEW.C:419-422   empty-handed leader drink: champion Water = 2048,
 *                         plays C08_SOUND_SWALLOW (8) on the front-wall click.
 *  - CLIKVIEW.C:480-496   non-empty hand: junk water/waterskin charges = 3,
 *                         empty flask C195 → C15_POTION_WATER_FLASK
 *                         (icon 163, type 15), F0296_CHAMPION_DrawChangedObjectIcons,
 *                         Load += (new weight - old weight), then routes to
 *                         F0372 front-wall sensor touch.
 *  - PANEL.C:1832-1836    F0349_INVENTORY_ProcessCommand70_ClickOnMouth:
 *                         waterskin in hand with ChargeCount > 0 adds 800 to
 *                         Water (capped at 2048), decrements ChargeCount, keeps
 *                         the junk in the leader hand (RemoveObject = FALSE).
 *  - PANEL.C:1860-1945    F0349 potion branch: Water Flask (C15) adds 1600 to
 *                         Water (capped at 2048) and converts the potion to
 *                         C20_POTION_EMPTY_FLASK while preserving Power; plays
 *                         C08_SOUND_SWALLOW but does NOT remove the leader hand
 *                         object.
 *  - DEFS.H:68,1479,1481,1891-1892,1945,1950
 *                         sound 8 swallow, water flask type 15, empty flask
 *                         type 20, junk icons 8/9, potion icons 163/195.
 */

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
    foodWater.champions[0].food = 777;
    foodWater.champions[0].water = 11;
    foodWater.champions[0].thirsty = 1;
    ok &= DM1V1_Fountain_ApplyDrinkPc34Compat(&foodWater, 0, 1, &result);
    ok &= expect_int("drink apply water", foodWater.champions[0].water, DM1_V1_FOUNTAIN_WATER_MAX);
    ok &= expect_int("drink preserves food", foodWater.champions[0].food, 777);
    ok &= expect_int("drink apply clears thirsty", foodWater.champions[0].thirsty, 0);
    ok &= expect_int("drink apply action", result.action, DM1_V1_FOUNTAIN_ACTION_DRINK);
    ok &= expect_int("drink apply sound", result.playSoundOrdinal, DM1_V1_FOUNTAIN_SWALLOW_SOUND);

    foodWater.champions[0].food = 333;
    foodWater.champions[0].water = 2300;
    foodWater.champions[0].thirsty = 1;
    ok &= DM1V1_Fountain_ApplyDrinkPc34Compat(&foodWater, 0, 1, &result);
    ok &= expect_int("drink forces overfull water to source max",
                     foodWater.champions[0].water,
                     DM1_V1_FOUNTAIN_WATER_MAX);
    ok &= expect_int("drink overfull preserves food", foodWater.champions[0].food, 333);
    ok &= expect_int("drink overfull clears thirsty", foodWater.champions[0].thirsty, 0);

    foodWater.champions[0].food = 444;
    foodWater.champions[0].water = 123;
    foodWater.champions[0].thirsty = 1;
    ok &= DM1V1_Fountain_ApplyDrinkPc34Compat(&foodWater, 0, 0, &result);
    ok &= expect_int("non-fountain drink no water mutation", foodWater.champions[0].water, 123);
    ok &= expect_int("non-fountain drink preserves food", foodWater.champions[0].food, 444);
    ok &= expect_int("non-fountain drink keeps thirsty", foodWater.champions[0].thirsty, 1);
    ok &= expect_int("non-fountain drink no action", result.action, DM1_V1_FOUNTAIN_ACTION_NONE);

    /* ── Round-trip: fountain fill → waterskin drink (CLIKVIEW.C:480-496 →
     * PANEL.C:1832-1836). The fountain writes 3 charges into the waterskin
     * junk; F0349 then spends one charge at a time, adding 800 to champion
     * Water and capping at the 2048 ceiling. */
    {
        M11_Item waterskin;
        DM1ConsumableResultPc34 drinkResult;
        DM1ConsumableChampionPc34 champ;
        memset(&champ, 0, sizeof(champ));
        champ.water = 200;
        memset(&waterskin, 0, sizeof(waterskin));
        waterskin.itemType = DM1_V1_ICON_JUNK_WATERSKIN;
        waterskin.charges = 0;
        waterskin.weight = 2;
        /* Source: waterskin weight = base + (charges << 1). After fill
         * (charges = 3) the source weight becomes 2 + (3 << 1) = 8. The
         * fountain helper writes the leader-hand item and returns the
         * weight delta for caller commit. */
        ok &= DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(&waterskin, 1, 8, &result);
        ok &= expect_int("round-trip waterskin fill action",
                         result.action, DM1_V1_FOUNTAIN_ACTION_FILL_JUNK_WATER);
        ok &= expect_int("round-trip waterskin fill charges",
                         waterskin.charges, DM1_V1_FOUNTAIN_FULL_CHARGES);
        ok &= expect_int("round-trip waterskin fill load delta",
                         result.leaderLoadDelta, 6);

        ok &= dm1_inventory_consume_water_junk_pc34(&champ, waterskin.itemType,
                                                    waterskin.charges,
                                                    &drinkResult);
        ok &= expect_int("round-trip waterskin first drink adds 800",
                         champ.water, 1000);
        ok &= expect_int("round-trip waterskin first drink charges 2",
                         drinkResult.chargeCountAfter, 2);
        ok &= expect_int("round-trip waterskin first drink swallow flag",
                         drinkResult.playSwallowSound, 1);
        ok &= expect_int("round-trip waterskin first drink stays in hand",
                         drinkResult.removeLeaderHandObject, 0);

        /* Second and third drinks keep the source's 800/cap-2048 arithmetic. */
        ok &= dm1_inventory_consume_water_junk_pc34(&champ, waterskin.itemType, 2, &drinkResult);
        ok &= expect_int("round-trip waterskin second drink",
                         champ.water, 1800);
        ok &= expect_int("round-trip waterskin second drink charges",
                         drinkResult.chargeCountAfter, 1);
        ok &= dm1_inventory_consume_water_junk_pc34(&champ, waterskin.itemType, 1, &drinkResult);
        ok &= expect_int("round-trip waterskin third drink",
                         champ.water, DM1_V1_FOUNTAIN_WATER_MAX);
        ok &= expect_int("round-trip waterskin third drink charges",
                         drinkResult.chargeCountAfter, 0);
        /* A fourth attempt with zero charges must be a no-op per source. */
        {
            int drinkRc = dm1_inventory_consume_water_junk_pc34(&champ, waterskin.itemType,
                                                                0, &drinkResult);
            ok &= expect_int("round-trip waterskin dry no-op", drinkRc, 0);
            ok &= expect_int("round-trip waterskin dry keeps water",
                             champ.water, DM1_V1_FOUNTAIN_WATER_MAX);
        }
    }

    /* ── Round-trip: fountain fill empty flask → drink as water flask
     * (CLIKVIEW.C:480-496 mutates Type → C15_POTION_WATER_FLASK; PANEL.C:1860
     * then adds 1600 and re-converts to C20_POTION_EMPTY_FLASK with Power
     * preserved). */
    {
        M11_Item flask;
        DM1ConsumableResultPc34 drinkResult;
        DM1ConsumableChampionPc34 champ;
        memset(&champ, 0, sizeof(champ));
        champ.water = 100;
        memset(&flask, 0, sizeof(flask));
        flask.itemType = DM1_V1_ICON_POTION_EMPTY_FLASK;
        flask.charges = 0; /* surrogate for Power in the compact M11_Item */
        flask.weight = 1;
        /* Source: empty flask weight = 1, water flask weight = 3 → +2. */
        ok &= DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(&flask, 1, 3, &result);
        ok &= expect_int("round-trip flask fill action",
                         result.action, DM1_V1_FOUNTAIN_ACTION_FILL_EMPTY_FLASK);
        ok &= expect_int("round-trip flask becomes water flask icon",
                         flask.itemType, DM1_V1_ICON_POTION_WATER_FLASK);
        ok &= expect_int("round-trip flask fill load delta",
                         result.leaderLoadDelta, 2);

        ok &= dm1_inventory_consume_potion_pc34(&champ, /*potionType=*/15,
                                                /*potionPower=*/42, NULL, 0,
                                                &drinkResult);
        ok &= expect_int("round-trip flask drink adds 1600",
                         champ.water, 1700);
        ok &= expect_int("round-trip flask drink result kind",
                         drinkResult.kind, DM1_CONSUMABLE_RESULT_POTION);
        ok &= expect_int("round-trip flask drink converts to empty flask",
                         drinkResult.potionTypeAfter,
                         DM1_CONSUMABLE_POTION_EMPTY_FLASK_PC34);
        ok &= expect_int("round-trip flask drink preserves power",
                         drinkResult.potionPowerAfter, 42);
        ok &= expect_int("round-trip flask drink stays in hand",
                         drinkResult.removeLeaderHandObject, 0);
        ok &= expect_int("round-trip flask drink swallow flag",
                         drinkResult.playSwallowSound, 1);
    }

    /* ── Round-trip: empty-flask fill path is type-changed in place. Re-filling
     * the same leader-hand item must operate on the *post-fill* state, not the
     * pre-fill icon. This locks the contract that the fountain never observes
     * a stale empty-flask icon after a successful fill. */
    {
        M11_Item flask;
        memset(&flask, 0, sizeof(flask));
        flask.itemType = DM1_V1_ICON_POTION_EMPTY_FLASK;
        flask.weight = 1;
        ok &= DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(&flask, 1, 3, &result);
        ok &= expect_int("refill flask first fill",
                         result.action, DM1_V1_FOUNTAIN_ACTION_FILL_EMPTY_FLASK);
        ok &= expect_int("refill flask first fill icon",
                         flask.itemType, DM1_V1_ICON_POTION_WATER_FLASK);
        /* Now re-evaluate with the *new* icon: water flask (163) is not in
         * the 8..9 junk range and is not C195 empty flask, so the fountain
         * must NOT change it. It must still route to the front-wall sensor. */
        ok &= DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(&flask, 1, 3, &result);
        ok &= expect_int("refill flask second click no-op action",
                         result.action, DM1_V1_FOUNTAIN_ACTION_NONE);
        ok &= expect_int("refill flask second click icon preserved",
                         flask.itemType, DM1_V1_ICON_POTION_WATER_FLASK);
        ok &= expect_int("refill flask second click still routes wall sensor",
                         result.touchFrontWallSensor, 1);
    }

    /* ── Round-trip: an already-full waterskin (charges = 3) refilled at the
     * fountain remains at 3 charges, no spurious second load delta, and still
     * routes to the front-wall sensor. The source unconditionally writes 3,
     * which our evaluator matches. */
    {
        M11_Item waterskin;
        memset(&waterskin, 0, sizeof(waterskin));
        waterskin.itemType = DM1_V1_ICON_JUNK_WATERSKIN;
        waterskin.charges = DM1_V1_FOUNTAIN_FULL_CHARGES;
        waterskin.weight = 8;
        ok &= DM1V1_Fountain_ApplyLeaderHandItemPc34Compat(&waterskin, 1, 8, &result);
        ok &= expect_int("full waterskin refill action",
                         result.action, DM1_V1_FOUNTAIN_ACTION_FILL_JUNK_WATER);
        ok &= expect_int("full waterskin refill charges stay 3",
                         waterskin.charges, DM1_V1_FOUNTAIN_FULL_CHARGES);
        ok &= expect_int("full waterskin refill zero load delta",
                         result.leaderLoadDelta, 0);
        ok &= expect_int("full waterskin refill routes wall sensor",
                         result.touchFrontWallSensor, 1);
    }

    /* ── Multi-champion isolation: the drink path mutates only the leader
     * champion. F0349_INVENTORY_ProcessCommand70_ClickOnMouth operates on
     * G0423_i_InventoryChampionOrdinal (the inventory panel champion), not
     * the whole party; same source-locked invariant must hold here. */
    {
        M11_FoodWaterState party;
        m11_fw_init(&party, 4);
        party.champions[0].water = 500;
        party.champions[0].thirsty = 1;
        party.champions[1].water = 1500;
        party.champions[1].thirsty = 1;
        party.champions[2].water = 1500;
        party.champions[2].thirsty = 1;
        party.champions[3].water = 1500;
        party.champions[3].thirsty = 1;
        ok &= DM1V1_Fountain_ApplyDrinkPc34Compat(&party, 0, 1, &result);
        ok &= expect_int("party drink leader only water",
                         party.champions[0].water, DM1_V1_FOUNTAIN_WATER_MAX);
        ok &= expect_int("party drink champ1 untouched",
                         party.champions[1].water, 1500);
        ok &= expect_int("party drink champ2 untouched",
                         party.champions[2].water, 1500);
        ok &= expect_int("party drink champ3 untouched",
                         party.champions[3].water, 1500);
        ok &= expect_int("party drink champ1 still thirsty",
                         party.champions[1].thirsty, 1);
        ok &= expect_int("party drink champ2 still thirsty",
                         party.champions[2].thirsty, 1);
        ok &= expect_int("party drink champ3 still thirsty",
                         party.champions[3].thirsty, 1);
        ok &= expect_int("party drink leader no longer thirsty",
                         party.champions[0].thirsty, 0);
    }

    /* ── Re-entrant evaluation: the front-wall click must be idempotent in
     * its routing side-effect. A second call against the same input must
     * produce the same action and the same wall-sensor touch. */
    {
        memset(&input, 0, sizeof(input));
        input.facingFountain = 1;
        input.leaderIndex = 0;
        input.leaderEmptyHanded = 1;
        ok &= DM1V1_Fountain_EvaluateFrontWallClickPc34Compat(input, &result);
        ok &= expect_int("reentrant first call action",
                         result.action, DM1_V1_FOUNTAIN_ACTION_DRINK);
        ok &= DM1V1_Fountain_EvaluateFrontWallClickPc34Compat(input, &result);
        ok &= expect_int("reentrant second call action",
                         result.action, DM1_V1_FOUNTAIN_ACTION_DRINK);
        ok &= expect_int("reentrant second call still routes wall sensor",
                         result.touchFrontWallSensor, 1);
        ok &= expect_int("reentrant second call still plays swallow",
                         result.playSoundOrdinal, DM1_V1_FOUNTAIN_SWALLOW_SOUND);
    }

    printf("fountainInteractionInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
