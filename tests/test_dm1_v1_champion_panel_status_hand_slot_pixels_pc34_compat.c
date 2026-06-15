/*
 * CTest gate: DM1 V1 Champion Panel status hand slot box pixel slice.
 *
 * Source-locks the CHAMDRAW.C F0291 18x18 hand-slot box layout across
 * the full 4-champion x 2-hand grid. The runtime pixel probe and the
 * status-states pixel probe both pixel-match the on-screen slot-box
 * blit through real M11 draw stack, but neither pins the source-locked
 * (champIdx*69 + 4/24, 10) origin for every one of the 8 status hand
 * slots in a focused unit test.
 *
 * ReDMCSB references:
 *   CHAMDRAW.C F0291 lines 632-646: body slots 0..5 render an 18x18
 *     slot-box graphic; ready hand (slot 0) and action hand (slot 1)
 *     both enter this branch.
 *   CHAMDRAW.C F0291 lines 648-651: acting-hand override requires
 *     (slotIndex == C01_SLOT_ACTION_HAND) && isActingChampion, so the
 *     ready hand never gets the C035 acting graphic even when its
 *     champion is acting.
 *   DEFS.H:2186-2188: C033/C034/C035 slot-box graphic IDs.
 *   layout-696 C211..C218: 8 hand-slot zone anchors at
 *     (champIdx * 69 + 4/24, 10) with size 18x18.
 */

#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>

static int expect_int(const char *label, int actual, int expected)
{
    if (actual == expected) {
        return 0;
    }
    fprintf(stderr, "FAIL: %s got %d expected %d\n",
            label, actual, expected);
    return 1;
}

static int expect_hand_slot(const char *label,
                            int championIndex, int handIndex,
                            int isActingChampion,
                            int expectedChampion,
                            int expectedHand,
                            int expectedIsActionHand,
                            int expectedX,
                            int expectedGraphic)
{
    DM1_ChampionPanel_StatusHandSlotBoxModel model;
    int failures = 0;

    if (!DM1_ChampionPanel_BuildStatusHandSlotBoxModel(
            championIndex, handIndex, isActingChampion, &model)) {
        fprintf(stderr, "FAIL: %s model build rejected valid fixture\n", label);
        return 1;
    }

    /* ReDMCSB CHAMDRAW.C F0291 line ~536-548: status hand box is parent
     * status box + (4/24, 10) for ready/action; the parent status box
     * left is champIdx * 69. */
    failures += expect_int("championIndex", model.championIndex, expectedChampion);
    failures += expect_int("handIndex", model.handIndex, expectedHand);
    failures += expect_int("isActionHand", model.isActionHand, expectedIsActionHand);
    failures += expect_int("isActingChampion",
                           model.isActingChampion, isActingChampion ? 1 : 0);
    failures += expect_int("x", model.x, expectedX);
    failures += expect_int("y", model.y, 10);
    failures += expect_int("width", model.width, DM1_SLOT_BOX_SIZE);
    failures += expect_int("height", model.height, DM1_SLOT_BOX_SIZE);
    failures += expect_int("graphic", model.graphicId, expectedGraphic);
    return failures;
}

int main(void)
{
    int failures = 0;
    int champion;
    int hand;
    /* Row 0: all four champions, ready hand (handIndex=0, isActingChampion=0).
     *   Per F0291 lines 632-646: ready hand is slot 0, not the action hand,
     *   so the acting-hand override is rejected; without wounds we expect
     *   C033_GRAPHIC_SLOT_BOX_NORMAL for every champion.
     *   Per layout-696 C211/C213/C215/C217: ready hand X = champIdx*69+4. */
    static const int kReadyHandX[DM1_CHAMPION_COUNT] = { 4, 73, 142, 211 };
    /* Row 1: all four champions, action hand (handIndex=1, isActingChampion=0).
     *   Per F0291 lines 648-651: action hand is slot 1, but the acting
     *   override is gated by isActingChampion; without wounds and with
     *   isActingChampion=0 we still expect C033_GRAPHIC_SLOT_BOX_NORMAL.
     *   Per layout-696 C212/C214/C216/C218: action hand X = champIdx*69+24. */
    static const int kActionHandX[DM1_CHAMPION_COUNT] = { 24, 93, 162, 231 };
    /* Row 2: action hand for the acting champion. Per F0291 lines 648-651
     *   the acting-hand override is gated by both slot 1 and
     *   isActingChampion, so only the action hand of the acting champion
     *   flips to C035_GRAPHIC_SLOT_BOX_ACTING_HAND. */
    static const int kActingChampionIndex = 2;

    printf("== DM1 V1 Champion Panel status hand slot box pixel slice ==\n");

    for (champion = 0; champion < DM1_CHAMPION_COUNT; ++champion) {
        char label[64];

        /* Ready hand, not acting: every champion slot gets the normal
         * C033 slot-box graphic at (champIdx*69+4, 10). */
        snprintf(label, sizeof(label), "ready hand champ%d normal", champion);
        failures += expect_hand_slot(label,
            champion, DM1_SLOT_READY_HAND, 0,
            champion, DM1_SLOT_READY_HAND, 0,
            kReadyHandX[champion], DM1_GFX_SLOT_NORMAL);

        /* Action hand, not acting: the acting-hand override is rejected
         * (isActingChampion=0), so the action hand keeps the normal
         * C033 slot-box graphic at (champIdx*69+24, 10). */
        snprintf(label, sizeof(label), "action hand champ%d normal", champion);
        failures += expect_hand_slot(label,
            champion, DM1_SLOT_ACTION_HAND, 0,
            champion, DM1_SLOT_ACTION_HAND, 1,
            kActionHandX[champion], DM1_GFX_SLOT_NORMAL);
    }

    /* Acting-champion hand-slot cascade for the chosen acting champion.
     * Per F0291 lines 648-651 only the action hand of the acting
     * champion gets the C035 override; the ready hand of the acting
     * champion still uses C033 because slot 0 is not the action hand. */
    {
        char readyLabel[64];
        char actionLabel[64];
        snprintf(readyLabel, sizeof(readyLabel),
                 "acting champ%d ready hand stays normal", kActingChampionIndex);
        snprintf(actionLabel, sizeof(actionLabel),
                 "acting champ%d action hand flips to acting", kActingChampionIndex);
        failures += expect_hand_slot(readyLabel,
            kActingChampionIndex, DM1_SLOT_READY_HAND, 1,
            kActingChampionIndex, DM1_SLOT_READY_HAND, 0,
            kReadyHandX[kActingChampionIndex], DM1_GFX_SLOT_NORMAL);
        failures += expect_hand_slot(actionLabel,
            kActingChampionIndex, DM1_SLOT_ACTION_HAND, 1,
            kActingChampionIndex, DM1_SLOT_ACTION_HAND, 1,
            kActionHandX[kActingChampionIndex], DM1_GFX_SLOT_ACTING);
    }

    /* ReDMCSB CHAMDRAW.C F0291 line ~536: every champion gets exactly
     * one 18x18 ready hand box and one 18x18 action hand box. Walk the
     * full 8-zone grid and confirm the championIndex/handIndex fields
     * round-trip to the same origin coordinates the layout-696
     * C211..C218 anchors claim. */
    {
        int seenReady[DM1_CHAMPION_COUNT] = { 0, 0, 0, 0 };
        int seenAction[DM1_CHAMPION_COUNT] = { 0, 0, 0, 0 };
        for (champion = 0; champion < DM1_CHAMPION_COUNT; ++champion) {
            for (hand = 0; hand < 2; ++hand) {
                DM1_ChampionPanel_StatusHandSlotBoxModel model;
                if (!DM1_ChampionPanel_BuildStatusHandSlotBoxModel(
                        champion, hand, 0, &model)) {
                    fprintf(stderr,
                            "FAIL: 8-zone grid rejected valid (champ=%d hand=%d)\n",
                            champion, hand);
                    failures++;
                    continue;
                }
                if (hand == 0) {
                    if (model.x != kReadyHandX[champion] || model.y != 10) {
                        fprintf(stderr,
                                "FAIL: 8-zone grid ready champ=%d got (%d,%d) want (%d,10)\n",
                                champion, model.x, model.y,
                                kReadyHandX[champion]);
                        failures++;
                    }
                    seenReady[champion]++;
                } else {
                    if (model.x != kActionHandX[champion] || model.y != 10) {
                        fprintf(stderr,
                                "FAIL: 8-zone grid action champ=%d got (%d,%d) want (%d,10)\n",
                                champion, model.x, model.y,
                                kActionHandX[champion]);
                        failures++;
                    }
                    seenAction[champion]++;
                }
                if (model.width != DM1_SLOT_BOX_SIZE ||
                    model.height != DM1_SLOT_BOX_SIZE ||
                    model.graphicId != DM1_GFX_SLOT_NORMAL) {
                    fprintf(stderr,
                            "FAIL: 8-zone grid champ=%d hand=%d got w/h/gfx=%d/%d/%d\n",
                            champion, hand, model.width, model.height,
                            model.graphicId);
                    failures++;
                }
            }
        }
        for (champion = 0; champion < DM1_CHAMPION_COUNT; ++champion) {
            if (seenReady[champion] != 1 || seenAction[champion] != 1) {
                fprintf(stderr,
                        "FAIL: 8-zone grid coverage champ=%d ready=%d action=%d\n",
                        champion, seenReady[champion], seenAction[champion]);
                failures++;
            }
        }
    }

    /* ReDMCSB F0291: model builder must reject champions outside 0..3,
     * handIndex outside 0..1, and a NULL output. */
    {
        DM1_ChampionPanel_StatusHandSlotBoxModel model;
        if (DM1_ChampionPanel_BuildStatusHandSlotBoxModel(4, 0, 0, &model) ||
            DM1_ChampionPanel_BuildStatusHandSlotBoxModel(0, 2, 0, &model) ||
            DM1_ChampionPanel_BuildStatusHandSlotBoxModel(0, 0, 0, NULL) ||
            DM1_ChampionPanel_BuildStatusHandSlotBoxModel(-1, 0, 0, &model)) {
            fprintf(stderr, "FAIL: F0291 status hand slot model accepted invalid input\n");
            failures++;
        }
    }

    if (failures == 0) {
        printf("PASS: status hand slot box pixel-slice assertions passed.\n");
    } else {
        printf("FAIL: %d assertion(s) failed.\n", failures);
    }
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
