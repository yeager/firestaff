#include "dm1_v1_champion_panel_wound_handling_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *label, int actual, int expected)
{
    ++g_assertions;
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s got %d expected %d\n",
                label, actual, expected);
        ++g_failures;
    }
}

static void expect_valid_model(const char *label,
                               int championIndex,
                               int handIndex,
                               int woundLevel,
                               int isActingChampion,
                               int expectedGraphic,
                               int expectedBranch)
{
    DM1_ChampionPanel_WoundHandSlotBoxModel model;
    const int expectedSlotBoxIndex =
        (championIndex *
         DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_HAND_COUNT_PC34) +
        handIndex;
    const int expectedX =
        (championIndex *
         DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_STATUS_BOX_SPACING_PC34) +
        ((handIndex ==
          DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_ACTION_HAND_PC34)
             ? 24
             : 4);
    const int isActionHand =
        handIndex == DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_ACTION_HAND_PC34;
    const int isWoundAffected = woundLevel > 0;

    expect_int(label,
               DM1_ChampionPanel_BuildWoundHandSlotBoxModel(
                   championIndex, handIndex, woundLevel, isActingChampion,
                   &model),
               1);
    expect_int("championIndex", model.championIndex, championIndex);
    expect_int("handIndex", model.handIndex, handIndex);
    expect_int("bodySlotIndex", model.bodySlotIndex, handIndex);
    expect_int("slotBoxIndex", model.slotBoxIndex, expectedSlotBoxIndex);
    expect_int("zoneId", model.zoneId,
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_FIRST_HAND_ZONE_PC34 +
                   expectedSlotBoxIndex);
    expect_int("woundLevel", model.woundLevel, woundLevel);
    expect_int("isWoundAffected", model.isWoundAffected,
               isWoundAffected ? 1 : 0);
    expect_int("isActingChampion", model.isActingChampion,
               isActingChampion ? 1 : 0);
    expect_int("isActionHand", model.isActionHand, isActionHand ? 1 : 0);
    expect_int("x", model.x, expectedX);
    expect_int("y", model.y, 10);
    expect_int("width", model.width,
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_SLOT_BOX_SIZE_PC34);
    expect_int("height", model.height,
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_SLOT_BOX_SIZE_PC34);
    expect_int("graphicId", model.graphicId, expectedGraphic);
    expect_int("branch", model.branch, expectedBranch);
    expect_int("woundBranchSuppressedByActing",
               model.woundBranchSuppressedByActing,
               (isActionHand && isActingChampion && isWoundAffected) ? 1 : 0);
}

static void test_constants(void)
{
    expect_int("champion count",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_CHAMPION_COUNT_PC34, 4);
    expect_int("hand count",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_HAND_COUNT_PC34, 2);
    expect_int("ready hand",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_READY_HAND_PC34, 0);
    expect_int("action hand",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_ACTION_HAND_PC34, 1);
    expect_int("C195",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_FIRST_BAR_GRAPH_ZONE_PC34,
               195);
    expect_int("C211",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_FIRST_HAND_ZONE_PC34,
               211);
    expect_int("C218",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_LAST_HAND_ZONE_PC34,
               218);
    expect_int("slot size",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_SLOT_BOX_SIZE_PC34, 18);
    expect_int("C033",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_NORMAL_PC34,
               33);
    expect_int("C034",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_WOUNDED_PC34,
               34);
    expect_int("C035",
               DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_ACTING_PC34,
               35);
}

static void test_invalid_inputs(void)
{
    DM1_ChampionPanel_WoundHandSlotBoxModel model;

    expect_int("reject null model",
               DM1_ChampionPanel_BuildWoundHandSlotBoxModel(0, 0, 0, 0,
                                                            NULL),
               0);
    expect_int("reject negative champion",
               DM1_ChampionPanel_BuildWoundHandSlotBoxModel(-1, 0, 0, 0,
                                                            &model),
               0);
    expect_int("reject champion 4",
               DM1_ChampionPanel_BuildWoundHandSlotBoxModel(4, 0, 0, 0,
                                                            &model),
               0);
    expect_int("reject negative hand",
               DM1_ChampionPanel_BuildWoundHandSlotBoxModel(0, -1, 0, 0,
                                                            &model),
               0);
    expect_int("reject hand 2",
               DM1_ChampionPanel_BuildWoundHandSlotBoxModel(0, 2, 0, 0,
                                                            &model),
               0);
    expect_int("reject negative wound",
               DM1_ChampionPanel_BuildWoundHandSlotBoxModel(0, 0, -1, 0,
                                                            &model),
               0);
}

static void test_full_hand_grid(void)
{
    int champion;
    int hand;

    for (champion = 0;
         champion <
         DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_CHAMPION_COUNT_PC34;
         ++champion) {
        for (hand = 0;
             hand < DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_HAND_COUNT_PC34;
             ++hand) {
            char label[80];

            snprintf(label, sizeof(label), "normal champ%d hand%d",
                     champion, hand);
            expect_valid_model(
                label, champion, hand, 0, 0,
                DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_NORMAL_PC34,
                DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_NORMAL_PC34);

            snprintf(label, sizeof(label), "wounded champ%d hand%d",
                     champion, hand);
            expect_valid_model(
                label, champion, hand, 1, 0,
                DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_WOUNDED_PC34,
                DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_WOUNDED_PC34);
        }
    }
}

static void test_acting_priority_and_positive_wound_levels(void)
{
    int champion;

    for (champion = 0;
         champion <
         DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_CHAMPION_COUNT_PC34;
         ++champion) {
        char label[80];

        snprintf(label, sizeof(label), "acting ready wounded champ%d",
                 champion);
        expect_valid_model(
            label, champion,
            DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_READY_HAND_PC34, 7, 1,
            DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_WOUNDED_PC34,
            DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_WOUNDED_PC34);

        snprintf(label, sizeof(label), "acting action normal champ%d",
                 champion);
        expect_valid_model(
            label, champion,
            DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_ACTION_HAND_PC34, 0, 1,
            DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_ACTING_PC34,
            DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_ACTING_PC34);

        snprintf(label, sizeof(label), "acting action wounded champ%d",
                 champion);
        expect_valid_model(
            label, champion,
            DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_ACTION_HAND_PC34, 32767, 1,
            DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_ACTING_PC34,
            DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_ACTING_PC34);
    }
}

int main(void)
{
    printf("== DM1 V1 Champion Panel wound hand slot-box source lock ==\n");
    printf("ReDMCSB anchors: CHAMDRAW.C F0291:632-646/F0296:1185-1262; "
           "DEFS.H C033-C035/M070/C211-C218; CHAMPION.C F0297/F0298/F0302; "
           "PANEL.C F0354/F0355\n");

    test_constants();
    test_invalid_inputs();
    test_full_hand_grid();
    test_acting_priority_and_positive_wound_levels();

    if (g_failures == 0) {
        printf("PASS: %d wound hand slot-box assertions passed.\n",
               g_assertions);
        return EXIT_SUCCESS;
    }

    printf("FAIL: %d of %d wound hand slot-box assertions failed.\n",
           g_failures, g_assertions);
    return EXIT_FAILURE;
}
