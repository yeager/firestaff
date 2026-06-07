#include "dm1_v1_chest_open_with_full_leader_hand_pc34_compat.h"

#include <stdio.h>

static DM1_V1_ChestOpenFullLeaderHandProbePc34 g_probe;
static int g_assertions;

static int expect_int(const char* label, int got, int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               redmcsbAnchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int test_probe_spec_and_setup(void)
{
    const DM1_V1_ChestOpenFullLeaderHandSpecPc34* spec =
        M11_GameView_ChestOpenWithFullLeaderHandSpecPc34();
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 43,53-67";
    const char* f0302Hand = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    int ok = 1;

    ok &= expect_int("contract-only marker",
                     g_probe.sourceLockedContractOnly, 1, f0333Open);
    ok &= expect_int("party champion count",
                     g_probe.partyChampionCount, 1, f0302Hand);
    ok &= expect_int("spec party champion count",
                     spec->partyChampionCount, 1, f0302Hand);
    ok &= expect_int("C537 slot constant",
                     spec->c537Pc34Slot, DM1_PC34_SLOT_CHEST_1, f0333Open);
    ok &= expect_int("C538 slot constant",
                     spec->c538Pc34Slot, DM1_PC34_SLOT_CHEST_2, f0333Open);
    ok &= expect_int("C539 slot constant",
                     spec->c539Pc34Slot, DM1_PC34_SLOT_CHEST_3, f0333Open);
    ok &= expect_int("C540 slot constant",
                     spec->c540Pc34Slot, DM1_PC34_SLOT_CHEST_4, f0333Open);
    ok &= expect_int("C541 slot constant",
                     spec->c541Pc34Slot, DM1_PC34_SLOT_CHEST_5, f0333Open);
    ok &= expect_int("C542 slot constant",
                     spec->c542Pc34Slot, DM1_PC34_SLOT_CHEST_6, f0333Open);
    ok &= expect_int("C543 slot constant",
                     spec->c543Pc34Slot, DM1_PC34_SLOT_CHEST_7, f0333Open);
    ok &= expect_int("C544 slot constant",
                     spec->c544Pc34Slot, DM1_PC34_SLOT_CHEST_8, f0333Open);
    ok &= expect_int("chest slot count",
                     spec->chestSlotCount, DM1_PC34_CHEST_SLOT_COUNT,
                     f0333Open);
    ok &= expect_int("container base weight",
                     spec->containerBaseWeight,
                     DM1_PC34_CHEST_OPEN_FULL_HAND_CONTAINER_BASE_WEIGHT,
                     f0333Open);
    ok &= expect_int("chest A/B different x",
                     g_probe.chestACellX != g_probe.chestBCellX, 1,
                     f0333Open);
    ok &= expect_int("chest A/B different y",
                     g_probe.chestACellY != g_probe.chestBCellY, 1,
                     f0333Open);
    ok &= expect_int("leader helmet item type",
                     spec->leaderHelmet.m11ItemItemType,
                     DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Hand);
    ok &= expect_int("leader helmet allowed head plus container",
                     spec->leaderHelmet.m11ItemAllowedSlots,
                     DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER,
                     f0302Hand);
    ok &= expect_int("leader hand setup",
                     g_probe.leaderHandSetupResult, 1, f0302Hand);
    ok &= expect_int("leader hand full before chest B open",
                     g_probe.leaderHandBeforeOpenType,
                     DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Hand);
    ok &= expect_int("leader hand weight before chest B open",
                     g_probe.leaderHandBeforeOpenWeight,
                     DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_WEIGHT,
                     f0302Hand);

    return ok;
}

static int test_open_chest_b_with_full_leader_hand(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 43,53-67";
    const char* f0302Hand = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0140Weight = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    int ok = 1;
    int i;

    ok &= expect_int("chest B closed head before open",
                     g_probe.chestBClosedHeadBefore, 910, f0333Open);
    ok &= expect_int("chest B closed tail before open",
                     g_probe.chestBClosedTailBefore, 910, f0333Open);
    ok &= expect_int("chest B closed weight before open",
                     g_probe.chestBClosedWeightBefore, 61, f0140Weight);
    ok &= expect_int("chest B opens",
                     g_probe.chestBOpenResult, 1, f0333Open);
    ok &= expect_int("chest B is current open chest",
                     g_probe.chestBOpenThing, g_probe.chestBThing, f0333Open);
    ok &= expect_int("chest B C537 contains own item",
                     g_probe.chestBC537TypeAfterOpen, 910, f0333Open);
    ok &= expect_int("chest B C540 stays empty",
                     g_probe.chestBC540TypeAfterOpen, 0, f0333Open);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT; ++i) {
        const int want = i == 0 ? 910 : 0;
        ok &= expect_int("chest B open materialization",
                         g_probe.chestBOpenTypes[i], want, f0333Open);
    }
    ok &= expect_int("chest B visible weight after open",
                     g_probe.chestBVisibleWeightAfterOpen, 11, f0140Weight);
    ok &= expect_int("chest B container weight after open",
                     g_probe.chestBContainerWeightAfterOpen, 61, f0140Weight);
    ok &= expect_int("leader hand unchanged by chest B open",
                     g_probe.leaderHandAfterChestBOpenType,
                     DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Hand);
    ok &= expect_int("leader hand weight unchanged by chest B open",
                     g_probe.leaderHandAfterChestBOpenWeight,
                     DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_WEIGHT,
                     f0302Hand);
    ok &= expect_int("leader hand helmet allowed slots preserved",
                     g_probe.leaderHandAfterChestBOpenAllowedSlots,
                     DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER,
                     f0302Hand);
    ok &= expect_int("leader hand not evicted",
                     g_probe.leaderHandEvictedByChestBOpen, 0, f0302Hand);
    ok &= expect_int("leader helmet absent from chest B view",
                     g_probe.chestBContainsLeaderHelmetAfterOpen, 0,
                     f0333Open);
    ok &= expect_int("leader helmet not duplicated into C540",
                     g_probe.leaderHandDuplicatedIntoC540ByChestBOpen, 0,
                     f0333Open);
    ok &= expect_int("no chest B auto-pickup into leader hand",
                     g_probe.chestBFirstItemInLeaderHandAfterOpen, 0,
                     f0302Hand);
    ok &= expect_int("chest B closed head unchanged after open",
                     g_probe.chestBClosedHeadAfterOpen,
                     g_probe.chestBClosedHeadBefore, f0333Open);
    ok &= expect_int("chest B closed tail unchanged after open",
                     g_probe.chestBClosedTailAfterOpen,
                     g_probe.chestBClosedTailBefore, f0333Open);
    ok &= expect_int("chest B closed weight unchanged after open",
                     g_probe.chestBClosedWeightAfterOpen,
                     g_probe.chestBClosedWeightBefore, f0140Weight);
    ok &= expect_int("chest B closed source state intact",
                     g_probe.chestBClosedStateIntactAfterOpen, 1,
                     f0333Open);

    return ok;
}

static int test_chest_a_state_preserved_when_b_opens(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 43,53-67";
    const char* f0140Weight = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    int ok = 1;

    ok &= expect_int("chest A closed head before chest B open",
                     g_probe.chestAClosedHeadBefore, 810, f0333Open);
    ok &= expect_int("chest A closed tail before chest B open",
                     g_probe.chestAClosedTailBefore, 810, f0333Open);
    ok &= expect_int("chest A closed weight before chest B open",
                     g_probe.chestAClosedWeightBefore, 57, f0140Weight);
    ok &= expect_int("chest A closed head after chest B open",
                     g_probe.chestAClosedHeadAfterChestBOpen,
                     g_probe.chestAClosedHeadBefore, f0333Open);
    ok &= expect_int("chest A closed tail after chest B open",
                     g_probe.chestAClosedTailAfterChestBOpen,
                     g_probe.chestAClosedTailBefore, f0333Open);
    ok &= expect_int("chest A closed weight after chest B open",
                     g_probe.chestAClosedWeightAfterChestBOpen,
                     g_probe.chestAClosedWeightBefore, f0140Weight);
    ok &= expect_int("chest A closed state intact after chest B open",
                     g_probe.chestAClosedStateIntactAfterChestBOpen, 1,
                     f0333Open);

    return ok;
}

static int test_open_a_then_open_b_with_full_leader_hand(void)
{
    const char* f0333Replace =
        "ReDMCSB CHEST.C F0333 lines 31-38,43,53-67";
    const char* f0334Close = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302Hand = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0140Weight = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    int ok = 1;
    int i;

    ok &= expect_int("case2 leader hand setup",
                     g_probe.case2LeaderHandSetupResult, 1, f0302Hand);
    ok &= expect_int("case2 leader before chest A open",
                     g_probe.case2LeaderBeforeChestAOpen,
                     DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Hand);
    ok &= expect_int("case2 chest A opens",
                     g_probe.case2ChestAOpenResult, 1, f0333Replace);
    ok &= expect_int("case2 chest A current open chest",
                     g_probe.case2ChestAOpenThing, g_probe.chestAThing,
                     f0333Replace);
    ok &= expect_int("case2 leader preserved after chest A open",
                     g_probe.case2LeaderAfterChestAOpen,
                     DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Hand);
    ok &= expect_int("case2 chest A C537 item",
                     g_probe.case2ChestAC537TypeAfterOpen, 810,
                     f0333Replace);
    ok &= expect_int("case2 chest A C538 item",
                     g_probe.case2ChestAC538TypeAfterOpen, 811,
                     f0333Replace);
    ok &= expect_int("case2 leader helmet absent from chest A",
                     g_probe.case2ChestAContainsLeaderHelmetAfterOpen, 0,
                     f0333Replace);
    ok &= expect_int("case2 chest A visible weight",
                     g_probe.case2ChestAVisibleWeightAfterOpen, 17,
                     f0140Weight);
    ok &= expect_int("case2 chest A container weight",
                     g_probe.case2ChestAContainerWeightAfterOpen, 67,
                     f0140Weight);
    ok &= expect_int("case2 open chest B closes previous A count",
                     g_probe.case2OpenChestBReplacingReturn, 2, f0334Close);
    ok &= expect_int("case2 chest B current open chest",
                     g_probe.case2ChestBOpenThing, g_probe.chestBThing,
                     f0333Replace);
    ok &= expect_int("case2 leader preserved after chest B open",
                     g_probe.case2LeaderAfterChestBOpen,
                     DM1_PC34_CHEST_OPEN_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Hand);
    ok &= expect_int("case2 previous chest A closed count",
                     g_probe.case2PreviousChestAClosedCount, 2, f0334Close);
    ok &= expect_int("case2 previous chest A closed head",
                     g_probe.case2PreviousChestAClosedHead, 810, f0334Close);
    ok &= expect_int("case2 previous chest A closed tail",
                     g_probe.case2PreviousChestAClosedTail, 811, f0334Close);
    ok &= expect_int("case2 previous chest A closed weight",
                     g_probe.case2PreviousChestAClosedWeight, 67,
                     f0140Weight);
    ok &= expect_int("case2 chest B C537 item",
                     g_probe.case2ChestBC537TypeAfterOpen, 910,
                     f0333Replace);
    ok &= expect_int("case2 chest B C538 empty",
                     g_probe.case2ChestBC538TypeAfterOpen, 0, f0333Replace);
    ok &= expect_int("case2 chest B C540 empty",
                     g_probe.case2ChestBC540TypeAfterOpen, 0, f0333Replace);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_FULL_HAND_SLOT_COUNT; ++i) {
        const int want = i == 0 ? 910 : 0;
        ok &= expect_int("case2 chest B open materialization",
                         g_probe.case2ChestBOpenTypes[i], want,
                         f0333Replace);
    }
    ok &= expect_int("case2 chest B visible weight",
                     g_probe.case2ChestBVisibleWeightAfterOpen, 11,
                     f0140Weight);
    ok &= expect_int("case2 chest B container weight",
                     g_probe.case2ChestBContainerWeightAfterOpen, 61,
                     f0140Weight);
    ok &= expect_int("case2 no chest A item in chest B view",
                     g_probe.case2ChestBContainsChestAItem, 0, f0333Replace);
    ok &= expect_int("case2 leader helmet absent from chest B",
                     g_probe.case2ChestBContainsLeaderHelmet, 0, f0333Replace);
    ok &= expect_int("case2 no chest B auto-pickup into leader hand",
                     g_probe.case2ChestBFirstItemInLeaderHand, 0, f0302Hand);
    ok &= expect_int("case2 leader helmet not duplicated into C540",
                     g_probe.case2LeaderDuplicatedIntoC540, 0,
                     f0333Replace);

    return ok;
}

int main(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 43,53-67";
    int ok = 1;

    printf("probe=dm1_v1_chest_open_with_full_leader_hand_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           M11_GameView_ChestOpenWithFullLeaderHandSourceEvidencePc34());

    ok &= expect_int(
        "probe setup",
        M11_GameView_ChestOpenWithFullLeaderHandRuntimeGatePc34(&g_probe),
        1, f0333Open);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestOpenWithFullLeaderHandInvariantOk=0\n");
        return 1;
    }

    ok &= test_probe_spec_and_setup();
    ok &= test_open_chest_b_with_full_leader_hand();
    ok &= test_chest_a_state_preserved_when_b_opens();
    ok &= test_open_a_then_open_b_with_full_leader_hand();

    printf("assertionCount=%d\n", g_assertions);
    printf("chestOpenWithFullLeaderHandInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
