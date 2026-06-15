#include "dm1/dm1_v1_inventory_champion_switch_hand_carry_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;

#define CHECK_REDMCSB(cond, label, anchor) do { \
    ++g_assertions; \
    if (!(anchor) || (anchor)[0] == '\0') { \
        printf("FAIL %s missing ReDMCSB anchor\n", (label)); \
        return 0; \
    } \
    if (!(cond)) { \
        printf("FAIL %s anchor=%s\n", (label), (anchor)); \
        return 0; \
    } \
    printf("ok %s anchor=%s\n", (label), (anchor)); \
} while (0)

static int check_equal_int(const char* label, int got, int want,
                           const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               anchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int test_evidence(void)
{
    const DM1_V1_InventoryChampionSwitchHandCarryEvidencePc34* e =
        DM1_V1_InventoryChampionSwitchHandCarry_EvidencePc34();

    CHECK_REDMCSB(e != 0, "evidence exists",
                  "ReDMCSB PANEL.C:2267-2447 F0354");
    CHECK_REDMCSB(strstr(e->f0354EntryGuard, "PANEL.C:2267-2285") != 0,
                  "entry guard cites F0354 dead/mouth-eye lines",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(strstr(e->f0354OldInventoryClose, "PANEL.C:2299-2322") != 0,
                  "old inventory close cites F0354 clear/close path",
                  e->f0354OldInventoryClose);
    CHECK_REDMCSB(strstr(e->f0354CloseBranch, "PANEL.C:2335-2352") != 0,
                  "close branch cites F0354 movement return",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(strstr(e->f0354NewInventoryDraw, "PANEL.C:2363-2433") != 0,
                  "new inventory draw cites F0354 slot redraw path",
                  e->f0354NewInventoryDraw);
    CHECK_REDMCSB(strstr(e->f0354MouseInputRefresh, "PANEL.C:2437-2447") != 0,
                  "mouse/input refresh cites F0354 tail",
                  e->f0354MouseInputRefresh);
    CHECK_REDMCSB(strstr(e->f0334ChestClose, "CHEST.C:113-132") != 0,
                  "chest close cites F0334 rewrite/clear path",
                  e->f0334ChestClose);
    CHECK_REDMCSB(strstr(e->f0334ChestClose, "skipping empty G0425 slots") != 0,
                  "chest close cites visible-slot compaction",
                  e->f0334ChestClose);
    CHECK_REDMCSB(strstr(e->f0352F0353LeaderHandDraw, "PANEL.C:2153-2158") != 0,
                  "leader hand draw contrast cites F0352",
                  e->f0352F0353LeaderHandDraw);
    CHECK_REDMCSB(strstr(e->f0352F0353LeaderHandDraw, "PANEL.C:2183-2190") != 0,
                  "leader hand name contrast cites F0353",
                  e->f0352F0353LeaderHandDraw);
    CHECK_REDMCSB(strstr(e->nonOverlapScope, "Non-overlap") != 0,
                  "non-overlap scope is explicit",
                  e->nonOverlapScope);
    return 1;
}

static int test_switch_to_different_champion_preserves_hand(void)
{
    DM1_V1_InventoryChampionSwitchHandCarryStatePc34 state;
    DM1_V1_InventoryChampionSwitchHandCarryResultPc34 r;
    const DM1_V1_InventoryChampionSwitchHandCarryEvidencePc34* e =
        DM1_V1_InventoryChampionSwitchHandCarry_EvidencePc34();

    DM1_V1_InventoryChampionSwitchHandCarry_InitPc34(
        &state, 0, DM1_V1_ICSWHC_OPEN_CHEST_THING_PC34);

    CHECK_REDMCSB(DM1_V1_InventoryChampionSwitchHandCarry_OpenPc34(
                      &state, 1, &r) == 1,
                  "switch to different champion is accepted",
                  e->f0354NewInventoryDraw);
    CHECK_REDMCSB(r.acceptedSwitch == 1,
                  "result records accepted switch",
                  e->f0354NewInventoryDraw);
    CHECK_REDMCSB(r.acceptedClose == 0,
                  "different champion switch is not close branch",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.rejectedDeadChampion == 0,
                  "healthy target avoids dead guard",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.rejectedMouthEyePress == 0,
                  "no mouth/eye press avoids guard",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.oldInventoryOrdinalBefore == 1,
                  "old inventory ordinal starts at champion 0",
                  e->f0354OldInventoryClose);
    CHECK_REDMCSB(r.targetOrdinalAfter == 2,
                  "new inventory ordinal becomes champion 1",
                  e->f0354NewInventoryDraw);
    CHECK_REDMCSB(r.leaderHandThingBefore ==
                      DM1_V1_ICSWHC_LEADER_HAND_THING_PC34,
                  "leader hand begins with carried thing",
                  e->f0352F0353LeaderHandDraw);
    CHECK_REDMCSB(r.leaderHandThingAfter ==
                      DM1_V1_ICSWHC_LEADER_HAND_THING_PC34,
                  "leader hand still carries same thing after switch",
                  e->f0352F0353LeaderHandDraw);
    CHECK_REDMCSB(r.leaderHandPreserved == 1,
                  "leader hand preservation flag is set",
                  e->f0352F0353LeaderHandDraw);
    CHECK_REDMCSB(r.openChestThingBefore ==
                      DM1_V1_ICSWHC_OPEN_CHEST_THING_PC34,
                  "switch starts with open chest",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.openChestThingAfter ==
                      DM1_V1_ICSWHC_THING_NONE_PC34,
                  "switch closes old open chest",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.chestClosed == 1,
                  "chest closed flag is set",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.chestCloseCountAfter == 3,
                  "switch close compacts three visible chest items",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.closedChestTypes[0] ==
                      DM1_V1_ICSWHC_CHEST_ITEM_FIRST_PC34,
                  "switch close keeps first visible chest item",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.closedChestTypes[1] ==
                      DM1_V1_ICSWHC_CHEST_ITEM_THIRD_PC34,
                  "switch close skips empty C538 before third item",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.closedChestTypes[2] ==
                      DM1_V1_ICSWHC_CHEST_ITEM_EIGHTH_PC34,
                  "switch close preserves late C544 item after gaps",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.chestSlotsClearedAfterClose == 1,
                  "switch close clears the G0425 chest window",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.oldStatusDrawDelta == 1,
                  "old champion status redraws once",
                  e->f0354OldInventoryClose);
    CHECK_REDMCSB(r.newStatusDrawDelta == 1,
                  "new champion status redraws once",
                  e->f0354NewInventoryDraw);
    CHECK_REDMCSB(r.slotDrawDelta == DM1_V1_ICSWHC_SLOT_DRAW_COUNT_PC34,
                  "new inventory draws C00..C29 slots",
                  e->f0354NewInventoryDraw);
    CHECK_REDMCSB(r.mousePointerBitmapUpdatedDelta == 1,
                  "mouse pointer bitmap updates for inventory switch",
                  e->f0354MouseInputRefresh);
    CHECK_REDMCSB(r.secondaryInputInventoryAfter == 1,
                  "secondary input switches to champion inventory",
                  e->f0354MouseInputRefresh);
    CHECK_REDMCSB(r.secondaryInputMovementAfter == 0,
                  "movement secondary input is not selected",
                  e->f0354MouseInputRefresh);
    CHECK_REDMCSB(r.discardInputDelta == 1,
                  "input queue is discarded once",
                  e->f0354MouseInputRefresh);
    CHECK_REDMCSB(r.stopWaitingForPlayerInputAfter == 1,
                  "stop-waiting flag is set after accepted switch",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(state.f0334CloseChestCount == 1,
                  "F0334 close count increments once",
                  e->f0334ChestClose);
    return 1;
}

static int test_same_champion_closes_inventory_preserves_hand(void)
{
    DM1_V1_InventoryChampionSwitchHandCarryStatePc34 state;
    DM1_V1_InventoryChampionSwitchHandCarryResultPc34 r;
    const DM1_V1_InventoryChampionSwitchHandCarryEvidencePc34* e =
        DM1_V1_InventoryChampionSwitchHandCarry_EvidencePc34();

    DM1_V1_InventoryChampionSwitchHandCarry_InitPc34(
        &state, 0, DM1_V1_ICSWHC_OPEN_CHEST_THING_PC34);

    CHECK_REDMCSB(DM1_V1_InventoryChampionSwitchHandCarry_OpenPc34(
                      &state, 0, &r) == 1,
                  "same champion request is accepted as close",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.acceptedClose == 1,
                  "result records close branch",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.acceptedSwitch == 0,
                  "close branch is not a champion switch",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.targetOrdinalAfter ==
                      DM1_V1_ICSWHC_NO_INVENTORY_ORDINAL_PC34,
                  "close branch clears inventory ordinal",
                  e->f0354OldInventoryClose);
    CHECK_REDMCSB(r.leaderHandPreserved == 1,
                  "close branch preserves carried leader hand item",
                  e->f0352F0353LeaderHandDraw);
    CHECK_REDMCSB(r.chestClosed == 1,
                  "close branch closes old chest",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.chestCloseCountAfter == 3,
                  "close branch compacts three visible chest items",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.closedChestTypes[0] ==
                      DM1_V1_ICSWHC_CHEST_ITEM_FIRST_PC34,
                  "close branch keeps first visible chest item",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.closedChestTypes[1] ==
                      DM1_V1_ICSWHC_CHEST_ITEM_THIRD_PC34,
                  "close branch skips empty C538 before third item",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.closedChestTypes[2] ==
                      DM1_V1_ICSWHC_CHEST_ITEM_EIGHTH_PC34,
                  "close branch preserves late C544 item after gaps",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.chestSlotsClearedAfterClose == 1,
                  "close branch clears the G0425 chest window",
                  e->f0334ChestClose);
    CHECK_REDMCSB(r.oldStatusDrawDelta == 1,
                  "close branch redraws old champion state",
                  e->f0354OldInventoryClose);
    CHECK_REDMCSB(r.newStatusDrawDelta == 0,
                  "close branch does not draw a new inventory champion",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.slotDrawDelta == 0,
                  "close branch does not redraw C00..C29 slots",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.movementArrowsDrawDelta == 1,
                  "close branch redraws movement arrows",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.floorCeilingDrawDelta == 1,
                  "close branch redraws floor and ceiling",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.refreshMousePointerDelta == 1,
                  "close branch requests mouse pointer refresh",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.secondaryInputInventoryAfter == 0,
                  "close branch leaves inventory secondary input",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.secondaryInputMovementAfter == 1,
                  "close branch restores movement secondary input",
                  e->f0354CloseBranch);
    CHECK_REDMCSB(r.discardInputDelta == 1,
                  "close branch discards input once",
                  e->f0354CloseBranch);
    return 1;
}

static int test_entry_guards_preserve_state(void)
{
    DM1_V1_InventoryChampionSwitchHandCarryStatePc34 state;
    DM1_V1_InventoryChampionSwitchHandCarryResultPc34 r;
    const DM1_V1_InventoryChampionSwitchHandCarryEvidencePc34* e =
        DM1_V1_InventoryChampionSwitchHandCarry_EvidencePc34();

    DM1_V1_InventoryChampionSwitchHandCarry_InitPc34(
        &state, 0, DM1_V1_ICSWHC_OPEN_CHEST_THING_PC34);
    state.championHealth[1] = 0;
    CHECK_REDMCSB(DM1_V1_InventoryChampionSwitchHandCarry_OpenPc34(
                      &state, 1, &r) == 0,
                  "dead target champion is rejected",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.rejectedDeadChampion == 1,
                  "dead target rejection flag is set",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.targetOrdinalAfter == 1,
                  "dead target leaves inventory ordinal unchanged",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.leaderHandPreserved == 1,
                  "dead target leaves leader hand unchanged",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.openChestThingAfter ==
                      DM1_V1_ICSWHC_OPEN_CHEST_THING_PC34,
                  "dead target leaves open chest unchanged",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.stopWaitingForPlayerInputAfter == 0,
                  "dead target returns before stop-waiting flag",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(state.f0334CloseChestCount == 0,
                  "dead target does not close chest",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.chestCloseCountAfter == 0,
                  "dead target leaves compacted close count at zero",
                  e->f0354EntryGuard);

    DM1_V1_InventoryChampionSwitchHandCarry_InitPc34(
        &state, 0, DM1_V1_ICSWHC_OPEN_CHEST_THING_PC34);
    state.pressingMouthOrEye = 1;
    CHECK_REDMCSB(DM1_V1_InventoryChampionSwitchHandCarry_OpenPc34(
                      &state, 1, &r) == 0,
                  "mouth/eye press rejects champion switch",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.rejectedMouthEyePress == 1,
                  "mouth/eye rejection flag is set",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.targetOrdinalAfter == 1,
                  "mouth/eye press leaves inventory ordinal unchanged",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.leaderHandPreserved == 1,
                  "mouth/eye press leaves leader hand unchanged",
                  e->f0352F0353LeaderHandDraw);
    CHECK_REDMCSB(r.openChestThingAfter ==
                      DM1_V1_ICSWHC_OPEN_CHEST_THING_PC34,
                  "mouth/eye press leaves open chest unchanged",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.discardInputDelta == 0,
                  "mouth/eye press returns before input discard",
                  e->f0354EntryGuard);
    CHECK_REDMCSB(r.chestCloseCountAfter == 0,
                  "mouth/eye press leaves compacted close count at zero",
                  e->f0354EntryGuard);
    return 1;
}

int main(void)
{
    int ok = 1;

    printf("probe=dm1_v1_inventory_champion_switch_hand_carry_pc34_compat\n");
    ok &= test_evidence();
    ok &= test_switch_to_different_champion_preserves_hand();
    ok &= test_same_champion_closes_inventory_preserves_hand();
    ok &= test_entry_guards_preserve_state();
    ok &= check_equal_int("assertion count", g_assertions + 1, 74,
                          "ReDMCSB PANEL.C:2267-2447 F0354");

    printf("dm1V1InventoryChampionSwitchHandCarryOk=%d assertions=%d\n",
           ok ? 1 : 0, g_assertions);
    return ok ? 0 : 1;
}
