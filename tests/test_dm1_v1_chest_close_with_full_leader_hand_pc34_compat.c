#include "dm1_v1_chest_close_with_full_leader_hand_pc34_compat.h"

#include <stdio.h>

static DM1_V1_ChestCloseFullLeaderHandProbePc34 g_probe;
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
    const DM1_V1_ChestCloseFullLeaderHandSpecPc34* spec =
        dm1_v1_chest_close_with_full_leader_hand_spec_pc34();
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0302Swap = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0140Weight = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    const char* f0297Load =
        "ReDMCSB CHAMPION.C F0297/F0300/F0301 lines 263-265,582-615";
    int ok = 1;

    ok &= expect_int("contract-only marker",
                     g_probe.sourceLockedContractOnly, 1, f0333Open);
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
                     spec->chestSlotCount, DM1_PC34_CHEST_SLOT_COUNT, f0333Open);
    ok &= expect_int("container base weight",
                     spec->containerBaseWeight,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_CONTAINER_BASE_WEIGHT,
                     f0140Weight);
    ok &= expect_int("leader max load constant",
                     spec->leaderMaxLoad,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_LEADER_MAX_LOAD,
                     f0297Load);
    ok &= expect_int("base backpack load set",
                     g_probe.setupBaseLoadResult, 1, f0297Load);
    ok &= expect_int("base backpack load",
                     g_probe.baseBackpackLoad,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_BASE_BACKPACK_WEIGHT,
                     f0297Load);
    ok &= expect_int("chest A/B different x",
                     g_probe.chestACellX != g_probe.chestBCellX, 1,
                     f0333Open);
    ok &= expect_int("chest A/B different y",
                     g_probe.chestACellY != g_probe.chestBCellY, 1,
                     f0333Open);
    ok &= expect_int("C544 helmet item type",
                     spec->c544Helmet.m11ItemItemType,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Swap);
    ok &= expect_int("C544 helmet allowed head plus container",
                     spec->c544Helmet.m11ItemAllowedSlots,
                     DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER,
                     f0302Swap);

    return ok;
}

static int test_chest_a_open_pickup_and_close(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334Close = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0302Swap = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0140Weight = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    const char* f0297Load =
        "ReDMCSB CHAMPION.C F0297/F0300/F0301 lines 263-265,582-615";
    int ok = 1;
    int i;

    ok &= expect_int("chest A opens",
                     g_probe.chestAOpenResult, 1, f0333Open);
    ok &= expect_int("chest A is current open chest",
                     g_probe.chestAOpenThing, g_probe.chestAThing, f0333Open);
    ok &= expect_int("chest A visible weight after open",
                     g_probe.chestAVisibleWeightAfterOpen, 44, f0140Weight);
    ok &= expect_int("chest A container weight after open",
                     g_probe.chestAContainerWeightAfterOpen, 94, f0140Weight);
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_FULL_HAND_C544_INDEX; ++i) {
        ok &= expect_int("chest A G0425 visible prefix",
                         g_probe.chestAOpenTypes[i], 800 + i, f0333Open);
        ok &= expect_int("chest A visible prefix is chest-compatible",
                         g_probe.chestAOpenAllowedSlots[i],
                         DM1_PC34_ALLOWED_CONTAINER, f0333Open);
    }
    ok &= expect_int("C544 contains helmet",
                     g_probe.c544BeforePickupType,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Swap);
    ok &= expect_int("C544 helmet is chest-slot compatible",
                     g_probe.c544HelmetCanLeaveChest, 1, f0302Swap);
    ok &= expect_int("leader hand empty before C544 click",
                     g_probe.leaderHandBeforeC544Click, 0, f0302Swap);
    ok &= expect_int("empty-hand C544 pickup succeeds",
                     g_probe.c544ClickResult, 1, f0302Swap);
    ok &= expect_int("leader hand now holds C544 helmet",
                     g_probe.leaderHandAfterC544Click,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Swap);
    ok &= expect_int("leader hand helmet keeps head plus container mask",
                     g_probe.leaderHandAfterC544ClickAllowedSlots,
                     DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER,
                     f0302Swap);
    ok &= expect_int("leader hand full after pickup",
                     g_probe.leaderHandFullAfterC544Click, 1, f0302Swap);
    ok &= expect_int("C544 becomes empty",
                     g_probe.c544AfterPickupType, 0, f0302Swap);
    ok &= expect_int("chest A visible weight after C544 pickup",
                     g_probe.chestAVisibleWeightAfterPickup, 35, f0140Weight);
    ok &= expect_int("synthetic load after C544 pickup",
                     g_probe.loadAfterC544Pickup, 48, f0297Load);
    ok &= expect_int("leader hand full during chest A close",
                     g_probe.leaderHandFullDuringChestAClose, 1, f0302Swap);
    ok &= expect_int("chest A close rewrites seven visible links",
                     g_probe.chestACloseCount, 7, f0334Close);
    ok &= expect_int("chest A close weight snapshot",
                     g_probe.chestAContainerWeightSnapshotAtClose, 85,
                     f0140Weight);
    for (i = 0; i < 7; ++i) {
        ok &= expect_int("chest A closed link prefix",
                         g_probe.chestAClosedTypes[i], 800 + i, f0334Close);
    }
    ok &= expect_int("chest A C544 closed tail remains empty",
                     g_probe.chestAClosedTypes[7], 0, f0334Close);
    ok &= expect_int("hidden tail 808 input recorded",
                     g_probe.chestAHiddenTailInputType, 808, f0333Open);
    ok &= expect_int("hidden tail excluded when chest A closes",
                     g_probe.chestAHiddenTailExcludedOnClose, 1, f0334Close);
    ok &= expect_int("leader hand still holds helmet after chest A closes",
                     g_probe.leaderHandAfterChestAClose,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Swap);
    ok &= expect_int("leader hand helmet weight after chest A closes",
                     g_probe.leaderHandWeightAfterChestAClose,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_WEIGHT,
                     f0140Weight);
    ok &= expect_int("leader hand helmet mask after chest A closes",
                     g_probe.leaderHandAllowedSlotsAfterChestAClose,
                     DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER,
                     f0302Swap);
    ok &= expect_int("ready hand remains empty after chest A closes",
                     g_probe.readyHandAfterChestAClose, 0, f0302Swap);
    ok &= expect_int("action hand remains empty after chest A closes",
                     g_probe.actionHandAfterChestAClose, 0, f0302Swap);
    ok &= expect_int("backpack item survives chest A close",
                     g_probe.backpackAfterChestAClose,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_BACKPACK_ITEM, f0334Close);
    ok &= expect_int("closed chest A no longer contributes to load",
                     g_probe.loadAfterChestAClose,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_BASE_BACKPACK_WEIGHT,
                     f0297Load);
    ok &= expect_int("closed chest A slot read is rejected",
                     g_probe.chestAReadSlotAfterCloseResult, 0, f0334Close);
    ok &= expect_int("no chest open after chest A closes",
                     g_probe.chestAOpenThingAfterClose, 0, f0334Close);

    return ok;
}

static int test_chest_b_open_close_without_leader_contamination(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334Close = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0302Swap = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0140Weight = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    int ok = 1;
    int i;

    ok &= expect_int("leader hand helmet before chest B open",
                     g_probe.leaderHandBeforeChestBOpen,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Swap);
    ok &= expect_int("chest B opens",
                     g_probe.chestBOpenResult, 1, f0333Open);
    ok &= expect_int("chest B is current open chest",
                     g_probe.chestBOpenThing, g_probe.chestBThing, f0333Open);
    ok &= expect_int("leader hand unchanged by chest B open",
                     g_probe.leaderHandAfterChestBOpen,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_ITEM_TYPE,
                     f0333Open);
    ok &= expect_int("leader hand helmet absent from chest B G0425",
                     g_probe.chestBContainsLeaderHelmet, 0, f0333Open);
    ok &= expect_int("chest B visible weight after open",
                     g_probe.chestBVisibleWeightAfterOpen, 116, f0140Weight);
    ok &= expect_int("chest B container weight after open",
                     g_probe.chestBContainerWeightAfterOpen, 166, f0140Weight);
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT; ++i) {
        ok &= expect_int("chest B G0425 visible item",
                         g_probe.chestBOpenTypes[i], 900 + i, f0333Open);
    }
    ok &= expect_int("chest B C538 contains own item",
                     g_probe.chestBC538Type, 901, f0333Open);
    ok &= expect_int("chest B C538 is not leader helmet",
                     g_probe.chestBC538IsOwnItem, 1, f0333Open);
    ok &= expect_int("leader hand full during chest B close",
                     g_probe.leaderHandFullDuringChestBClose, 1, f0302Swap);
    ok &= expect_int("chest B close rewrites eight links",
                     g_probe.chestBCloseCount,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT, f0334Close);
    ok &= expect_int("chest B close weight snapshot",
                     g_probe.chestBContainerWeightSnapshotAtClose, 166,
                     f0140Weight);
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_FULL_HAND_SLOT_COUNT; ++i) {
        ok &= expect_int("chest B closed link",
                         g_probe.chestBClosedTypes[i], 900 + i, f0334Close);
    }
    ok &= expect_int("leader hand still holds helmet after chest B closes",
                     g_probe.leaderHandAfterChestBClose,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_ITEM_TYPE,
                     f0302Swap);
    ok &= expect_int("leader hand helmet weight after chest B closes",
                     g_probe.leaderHandWeightAfterChestBClose,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_HELMET_WEIGHT,
                     f0140Weight);
    ok &= expect_int("leader hand helmet mask after chest B closes",
                     g_probe.leaderHandAllowedSlotsAfterChestBClose,
                     DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER,
                     f0302Swap);
    ok &= expect_int("ready hand remains empty after chest B closes",
                     g_probe.readyHandAfterChestBClose, 0, f0302Swap);
    ok &= expect_int("action hand remains empty after chest B closes",
                     g_probe.actionHandAfterChestBClose, 0, f0302Swap);
    ok &= expect_int("backpack item survives chest B close",
                     g_probe.backpackAfterChestBClose,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_BACKPACK_ITEM, f0334Close);
    ok &= expect_int("closed chest B no longer contributes to load",
                     g_probe.loadAfterChestBClose,
                     DM1_PC34_CHEST_CLOSE_FULL_HAND_BASE_BACKPACK_WEIGHT,
                     f0140Weight);
    ok &= expect_int("closed chest B slot read is rejected",
                     g_probe.chestBReadSlotAfterCloseResult, 0, f0334Close);
    ok &= expect_int("no chest open after chest B closes",
                     g_probe.chestBOpenThingAfterClose, 0, f0334Close);

    return ok;
}

static int test_chest_a_state_and_container_weights_preserved(void)
{
    const char* f0334Close = "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0140Weight = "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    int ok = 1;
    int i;

    for (i = 0; i < 7; ++i) {
        ok &= expect_int("chest A closed prefix after chest B close",
                         g_probe.chestAAfterChestBCloseTypes[i], 800 + i,
                         f0334Close);
    }
    ok &= expect_int("chest A C544 still empty after chest B close",
                     g_probe.chestAAfterChestBCloseTypes[7], 0, f0334Close);
    ok &= expect_int("chest B close did not mutate chest A",
                     g_probe.chestAChangedByChestBClose, 0, f0334Close);
    ok &= expect_int("chest A state intact after chest B close",
                     g_probe.chestAStateIntactAfterChestBClose, 1, f0334Close);
    ok &= expect_int("chest A container weight after first close",
                     g_probe.containerAWeightAfterFirstClose, 85, f0140Weight);
    ok &= expect_int("chest A container weight after chest B close",
                     g_probe.containerAWeightAfterChestBClose, 85, f0140Weight);
    ok &= expect_int("container A base weight preserved flag",
                     g_probe.containerAContainerBaseWeightPreserved, 1,
                     f0140Weight);
    ok &= expect_int("container B closed-link weight",
                     g_probe.containerBWeightFromClosedLinks, 166,
                     f0140Weight);
    ok &= expect_int("container B base weight computed flag",
                     g_probe.containerBContainerBaseWeightComputed, 1,
                     f0140Weight);

    return ok;
}

int main(void)
{
    const char* f0333Open = "ReDMCSB CHEST.C F0333 lines 53-67";
    int ok = 1;

    printf("probe=dm1_v1_chest_close_with_full_leader_hand_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_close_with_full_leader_hand_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_close_with_full_leader_hand_pc34(&g_probe),
                     1, f0333Open);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestCloseWithFullLeaderHandInvariantOk=0\n");
        return 1;
    }

    ok &= test_probe_spec_and_setup();
    ok &= test_chest_a_open_pickup_and_close();
    ok &= test_chest_b_open_close_without_leader_contamination();
    ok &= test_chest_a_state_and_container_weights_preserved();

    printf("assertionCount=%d\n", g_assertions);
    printf("chestCloseWithFullLeaderHandInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
