#include "dm1_v1_chest_teleport_survival_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static M11_GameView_ChestTeleportSurvivalProbePc34 g_probe;
static int g_assertions;
static int g_failures;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("ok %s contains=%s anchor=%s\n", label, needle, redmcsbAnchor);
    return 1;
}

static int expect_nonempty(const char* label,
                           const char* value,
                           const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!value || value[0] == '\0') {
        ++g_failures;
        printf("FAIL %s empty anchor=%s\n", label, redmcsbAnchor);
        return 0;
    }
    printf("ok %s=%s anchor=%s\n", label, value, redmcsbAnchor);
    return 1;
}

static int expected_slot_thing(int slot)
{
    if (slot < DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT) {
        return DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_FIRST + slot;
    }
    return DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE;
}

static int expect_slot_order(const char* label,
                             const int* got,
                             const char* redmcsbAnchor)
{
    int ok = 1;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT; ++i) {
        char slotLabel[128];

        snprintf(slotLabel, sizeof(slotLabel), "%s C%d", label,
                 DM1_PC34_SLOT_CHEST_1 + i);
        ok &= expect_int(slotLabel, got[i], expected_slot_thing(i),
                         redmcsbAnchor);
    }
    return ok;
}

static int expect_empty_slots(const char* label,
                              const int* got,
                              const char* redmcsbAnchor)
{
    int ok = 1;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT; ++i) {
        char slotLabel[128];

        snprintf(slotLabel, sizeof(slotLabel), "%s C%d", label,
                 DM1_PC34_SLOT_CHEST_1 + i);
        ok &= expect_int(slotLabel, got[i],
                         DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE,
                         redmcsbAnchor);
    }
    return ok;
}

static int test_anchor_table(void)
{
    const M11_GameView_ChestTeleportSurvivalAnchorsPc34* a =
        &g_probe.anchors;
    const char* defs =
        "ReDMCSB DEFS.H line 2088, C30, G0425, G0426, M070, M516";
    int ok = 1;

    ok &= expect_contains("source evidence F0333",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "CHEST.C F0333:31-67",
                          "ReDMCSB CHEST.C F0333 lines 31-67");
    ok &= expect_contains("source evidence F0334",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "CHEST.C F0334:113-132",
                          "ReDMCSB CHEST.C F0334 lines 113-132");
    ok &= expect_contains("source evidence F0297",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "CHAMPION.C F0297:243-298",
                          "ReDMCSB CHAMPION.C F0297 lines 243-298");
    ok &= expect_contains("source evidence F0298",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "CHAMPION.C F0298:270-298",
                          "ReDMCSB CHAMPION.C F0298 lines 270-298");
    ok &= expect_contains("source evidence F0300",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "F0300:511-515",
                          "ReDMCSB CHAMPION.C F0300 lines 511-515");
    ok &= expect_contains("source evidence F0301",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "F0301:606-614",
                          "ReDMCSB CHAMPION.C F0301 lines 606-614");
    ok &= expect_contains("source evidence F0302",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "F0302:662-710",
                          "ReDMCSB CHAMPION.C F0302 lines 662-710");
    ok &= expect_contains("source evidence F0163",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "DUNGEON.C F0163:1769-1838",
                          "ReDMCSB DUNGEON.C F0163 lines 1769-1838");
    ok &= expect_contains("source evidence map setter",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "F0173:2724-2740/F0174:2742-2756",
                          "ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2756");
    ok &= expect_contains("source evidence teleport",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "MOVESENS.C F0267:469-492",
                          "ReDMCSB MOVESENS.C F0267 lines 469-492");
    ok &= expect_contains("source evidence object",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "OBJECT.C F0033:147-212",
                          "ReDMCSB OBJECT.C F0033 lines 147-212");
    ok &= expect_contains("source evidence blit",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "BLITMASK.C F0133:30-33",
                          "ReDMCSB BLITMASK.C F0133 lines 30-33");
    ok &= expect_contains("source evidence defs",
                          M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(),
                          "DEFS.H:2088",
                          defs);

    ok &= expect_nonempty("anchor F0333",
                          a->chestF0333OpenMaterialization,
                          "ReDMCSB CHEST.C F0333 lines 31-67");
    ok &= expect_nonempty("anchor F0334",
                          a->chestF0334CloseRewrite,
                          "ReDMCSB CHEST.C F0334 lines 113-132");
    ok &= expect_nonempty("anchor F0297",
                          a->championF0297LeaderHandPut,
                          "ReDMCSB CHAMPION.C F0297 lines 243-298");
    ok &= expect_nonempty("anchor F0298",
                          a->championF0298LeaderHandRemove,
                          "ReDMCSB CHAMPION.C F0298 lines 270-298");
    ok &= expect_nonempty("anchor F0300",
                          a->championF0300ChestSlotClear,
                          "ReDMCSB CHAMPION.C F0300 lines 511-515");
    ok &= expect_nonempty("anchor F0301",
                          a->championF0301ChestSlotWrite,
                          "ReDMCSB CHAMPION.C F0301 lines 606-614");
    ok &= expect_nonempty("anchor F0302",
                          a->championF0302OccupiedSlotSwap,
                          "ReDMCSB CHAMPION.C F0302 lines 662-710");
    ok &= expect_nonempty("anchor F0163",
                          a->dungeonF0163LinkAppend,
                          "ReDMCSB DUNGEON.C F0163 lines 1769-1838");
    ok &= expect_nonempty("anchor F0173/F0174",
                          a->dungeonF0173F0174MapSet,
                          "ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2756");
    ok &= expect_nonempty("anchor F0267",
                          a->movesensF0267TeleportLevelChange,
                          "ReDMCSB MOVESENS.C F0267 lines 469-492");
    ok &= expect_nonempty("anchor F0033",
                          a->objectF0033IconIndex,
                          "ReDMCSB OBJECT.C F0033 lines 147-212");
    ok &= expect_nonempty("anchor F0133",
                          a->blitmaskF0133PresentationRoute,
                          "ReDMCSB BLITMASK.C F0133 lines 30-33");
    ok &= expect_nonempty("anchor defs",
                          a->defsSentinelsAndSlots,
                          defs);
    return ok;
}

static int test_probe_constants(void)
{
    const char* defs =
        "ReDMCSB DEFS.H line 2088, C30, G0425, G0426, M070, M516";
    int ok = 1;

    ok &= expect_int("contract only marker",
                     g_probe.sourceLockedContractOnly, 1,
                     g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_int("C0xFFFF thing none",
                     g_probe.c0xFFFFThingNone,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE, defs);
    ok &= expect_int("C0xFFFE end of list",
                     g_probe.c0xFFFEThingEndOfList,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_END_OF_LIST, defs);
    ok &= expect_int("C30 chest slot base",
                     g_probe.c30ChestSlotBase,
                     DM1_PC34_SLOT_CHEST_1, defs);
    ok &= expect_int("C37 chest slot last",
                     g_probe.c37ChestSlotLast,
                     DM1_PC34_SLOT_CHEST_8, defs);
    ok &= expect_int("G0425 slot count",
                     g_probe.g0425SlotCount,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT, defs);
    ok &= expect_int("party champion count",
                     g_probe.partyChampionCount,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT, defs);
    ok &= expect_int("all champions alive",
                     g_probe.allChampionsAlive, 1, defs);
    ok &= expect_int("leader ordinal",
                     g_probe.leaderOrdinal, 1,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("inventory champion ordinal",
                     g_probe.inventoryChampionOrdinal, 1, defs);
    ok &= expect_int("chest identity",
                     g_probe.chestThing,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A,
                     g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_int("chest map A",
                     g_probe.chestMapIndex,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A,
                     g_probe.anchors.dungeonF0173F0174MapSet);
    ok &= expect_int("teleport destination B",
                     g_probe.teleportDestinationMapIndex,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_B,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_int("initial leader hand thing",
                     g_probe.initialLeaderHandThing,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_THING,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("initial leader hand weight",
                     g_probe.initialLeaderHandWeight,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_WEIGHT,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("initial chest item 0",
                     g_probe.initialChestItems[0],
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_FIRST,
                     g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_int("initial chest item 1",
                     g_probe.initialChestItems[1],
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_FIRST + 1,
                     g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_int("initial chest item 2",
                     g_probe.initialChestItems[2],
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_FIRST + 2,
                     g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_int("initial chest weight 0",
                     g_probe.initialChestWeights[0], 2,
                     g_probe.anchors.objectF0033IconIndex);
    ok &= expect_int("initial chest weight 1",
                     g_probe.initialChestWeights[1], 3,
                     g_probe.anchors.objectF0033IconIndex);
    ok &= expect_int("initial chest weight 2",
                     g_probe.initialChestWeights[2], 4,
                     g_probe.anchors.objectF0033IconIndex);
    return ok;
}

static int test_snapshot_common(
    const M11_GameView_ChestTeleportSurvivalSnapshotPc34* s,
    int expectedMap,
    int expectedOpenChest,
    int expectedVisibleCount,
    const char* mapAnchor,
    const char* openAnchor)
{
    int ok = 1;
    int i;

    ok &= expect_nonempty("snapshot phase", s->phaseName, openAnchor);
    ok &= expect_int("snapshot current map", s->currentMapIndex,
                     expectedMap, mapAnchor);
    ok &= expect_int("snapshot party map", s->partyMapIndex,
                     expectedMap, mapAnchor);
    ok &= expect_int("snapshot open chest", s->openChestThing,
                     expectedOpenChest, openAnchor);
    ok &= expect_int("snapshot chest identity", s->chestThing,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A, openAnchor);
    ok &= expect_int("snapshot chest owning map", s->chestOwningMapIndex,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A, mapAnchor);
    ok &= expect_int("snapshot visible count", s->chestVisibleCount,
                     expectedVisibleCount, openAnchor);
    ok &= expect_int("leader hand thing", s->leaderHandThing,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_THING,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("leader hand weight", s->leaderHandWeight,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_WEIGHT,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("leader hand icon stable",
                     s->leaderHandIconIndex,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_THING &
                         0x00FF,
                     g_probe.anchors.objectF0033IconIndex);
    ok &= expect_int("leader hand name refreshed",
                     s->leaderHandNameRefreshed, 1,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("leader hand pointer stable",
                     s->leaderHandPointerStable, 1,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("leader hand remove count",
                     s->leaderHandRemoveCount, 0,
                     g_probe.anchors.championF0298LeaderHandRemove);
    ok &= expect_int("leader hand put count",
                     s->leaderHandPutCount, 1,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("leader load", s->leaderLoad,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_WEIGHT,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("alive champion count", s->aliveChampionCount,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT,
                     g_probe.anchors.defsSentinelsAndSlots);
    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "champion %d alive", i + 1);
        ok &= expect_int(label, s->championHealth[i] > 0 ? 1 : 0,
                         1, g_probe.anchors.defsSentinelsAndSlots);
    }
    return ok;
}

static int test_open_and_teleport_cycle(void)
{
    const M11_GameView_ChestTeleportSurvivalSnapshotPc34* openA =
        &g_probe.snapshots[DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_OPEN_A];
    const M11_GameView_ChestTeleportSurvivalSnapshotPc34* teleportB =
        &g_probe.snapshots[
            DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_TELEPORT_B];
    const M11_GameView_ChestTeleportSurvivalSnapshotPc34* teleportA =
        &g_probe.snapshots[
            DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_TELEPORT_A];
    int ok = 1;

    ok &= test_snapshot_common(
        openA, DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A,
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A,
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT,
        g_probe.anchors.dungeonF0173F0174MapSet,
        g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_int("open A chest resolved on current map",
                     openA->chestResolvedOnCurrentMap, 1,
                     g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_slot_order("open A G0425", openA->g0425Slots,
                            g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_slot_order("open A chest links", openA->chestLinkThings,
                            g_probe.anchors.dungeonF0163LinkAppend);
    ok &= expect_int("open A object icon lookups",
                     openA->objectIconLookupCount,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT,
                     g_probe.anchors.objectF0033IconIndex);
    ok &= expect_int("open A blit route",
                     openA->blitRouteCount, 1,
                     g_probe.anchors.blitmaskF0133PresentationRoute);

    ok &= test_snapshot_common(
        teleportB, DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_B,
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A,
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT,
        g_probe.anchors.movesensF0267TeleportLevelChange,
        g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_int("teleport B preserves G0426",
                     teleportB->openChestThing, openA->openChestThing,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_int("teleport B leaves chest foreign unresolved",
                     teleportB->chestResolvedOnCurrentMap, 0,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_slot_order("teleport B G0425", teleportB->g0425Slots,
                            g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_slot_order("teleport B chest links",
                            teleportB->chestLinkThings,
                            g_probe.anchors.dungeonF0163LinkAppend);
    ok &= expect_int("teleport B map set count",
                     teleportB->mapSetCount, 1,
                     g_probe.anchors.dungeonF0173F0174MapSet);
    ok &= expect_int("teleport B count",
                     teleportB->teleportCount, 1,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_int("teleport B party X",
                     teleportB->partyMapX, 3,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_int("teleport B party Y",
                     teleportB->partyMapY, 9,
                     g_probe.anchors.movesensF0267TeleportLevelChange);

    ok &= test_snapshot_common(
        teleportA, DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A,
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A,
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT,
        g_probe.anchors.movesensF0267TeleportLevelChange,
        g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_int("teleport A preserves G0426",
                     teleportA->openChestThing, openA->openChestThing,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_int("teleport A re-resolves chest on current map",
                     teleportA->chestResolvedOnCurrentMap, 1,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_slot_order("teleport A G0425", teleportA->g0425Slots,
                            g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_slot_order("teleport A chest links",
                            teleportA->chestLinkThings,
                            g_probe.anchors.dungeonF0163LinkAppend);
    ok &= expect_int("teleport A map set count",
                     teleportA->mapSetCount, 2,
                     g_probe.anchors.dungeonF0173F0174MapSet);
    ok &= expect_int("teleport A count",
                     teleportA->teleportCount, 2,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_int("teleport A party X",
                     teleportA->partyMapX, 11,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_int("teleport A party Y",
                     teleportA->partyMapY, 17,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    return ok;
}

static int test_close_and_reopen_cycle(void)
{
    const M11_GameView_ChestTeleportSurvivalSnapshotPc34* closeA =
        &g_probe.snapshots[DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_CLOSE_A];
    const M11_GameView_ChestTeleportSurvivalSnapshotPc34* reopenA =
        &g_probe.snapshots[
            DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_REOPEN_A];
    int ok = 1;

    ok &= test_snapshot_common(
        closeA, DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A,
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE, 0,
        g_probe.anchors.dungeonF0173F0174MapSet,
        g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_empty_slots("close A cleared G0425",
                             closeA->g0425Slots,
                             g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_slot_order("close A compacted links",
                            closeA->chestLinkThings,
                            g_probe.anchors.dungeonF0163LinkAppend);
    ok &= expect_int("close A link head",
                     closeA->chestLinkHead,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_FIRST,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_int("close A link tail",
                     closeA->chestLinkTail,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_FIRST + 2,
                     g_probe.anchors.dungeonF0163LinkAppend);
    ok &= expect_int("close A rewrite count",
                     closeA->closeRewriteCount, 1,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_int("close A recompact count",
                     closeA->closeRecompactCount,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_int("close A clears G0426",
                     closeA->closeClearedOpenChest, 1,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_int("close A no early return",
                     closeA->closeWithoutOpenEarlyReturnCount, 0,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_int("close A C30 clear/write observed",
                     closeA->c30SlotClearWriteObserved, 1,
                     g_probe.anchors.championF0300ChestSlotClear);

    ok &= test_snapshot_common(
        reopenA, DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A,
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A,
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT,
        g_probe.anchors.dungeonF0173F0174MapSet,
        g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_slot_order("reopen A G0425",
                            reopenA->g0425Slots,
                            g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_slot_order("reopen A compacted links",
                            reopenA->chestLinkThings,
                            g_probe.anchors.dungeonF0163LinkAppend);
    ok &= expect_int("reopen A G0426",
                     reopenA->openChestThing,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A,
                     g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_int("reopen A resolves chest",
                     reopenA->chestResolvedOnCurrentMap, 1,
                     g_probe.anchors.chestF0333OpenMaterialization);
    ok &= expect_int("reopen A icon lookups include both opens",
                     reopenA->objectIconLookupCount,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT * 2,
                     g_probe.anchors.objectF0033IconIndex);
    ok &= expect_int("reopen A blit count includes both opens",
                     reopenA->blitRouteCount, 2,
                     g_probe.anchors.blitmaskF0133PresentationRoute);
    ok &= expect_int("reopen A close rewrite remains one",
                     reopenA->closeRewriteCount, 1,
                     g_probe.anchors.chestF0334CloseRewrite);
    return ok;
}

static int test_negative_foreign_map_close(void)
{
    const M11_GameView_ChestTeleportSurvivalNegativePc34* n =
        &g_probe.negative;
    int ok = 1;

    ok &= expect_int("negative starts on map A",
                     n->beforeMapIndex,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A,
                     g_probe.anchors.dungeonF0173F0174MapSet);
    ok &= expect_int("negative teleports to map B",
                     n->afterMapIndex,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_B,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_int("negative close attempted on B",
                     n->closeAttemptedOnMap,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_B,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_int("negative G0426 preserved",
                     n->openChestThingAfterCloseAttempt,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_slot_order("negative preserved G0425",
                            n->g0425SlotsAfterCloseAttempt,
                            g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_slot_order("negative preserved links",
                            n->chestLinkThingsAfterCloseAttempt,
                            g_probe.anchors.dungeonF0163LinkAppend);
    ok &= expect_int("negative close rewrite count",
                     n->closeRewriteCount, 0,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_int("negative close early return count",
                     n->closeWithoutOpenEarlyReturnCount, 1,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_int("negative chest not resolved on B",
                     n->chestResolvedOnCurrentMap, 0,
                     g_probe.anchors.movesensF0267TeleportLevelChange);
    ok &= expect_int("negative leader hand thing preserved",
                     n->leaderHandThingAfterCloseAttempt,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_THING,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("negative leader hand weight preserved",
                     n->leaderHandWeightAfterCloseAttempt,
                     DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_WEIGHT,
                     g_probe.anchors.championF0297LeaderHandPut);
    ok &= expect_int("negative preserved open chest flag",
                     n->preservedOpenChestOnForeignMap, 1,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_int("negative preserved G0425 flag",
                     n->preservedG0425OnForeignMap, 1,
                     g_probe.anchors.chestF0334CloseRewrite);
    ok &= expect_int("negative preserved leader hand flag",
                     n->preservedLeaderHandOnForeignMap, 1,
                     g_probe.anchors.championF0297LeaderHandPut);
    return ok;
}

int main(void)
{
    int ok = 1;

    if (!M11_GameView_ChestTeleportSurvivalRunPc34(&g_probe)) {
        printf("FAIL probe builder returned false\n");
        return 1;
    }

    ok &= test_anchor_table();
    ok &= test_probe_constants();
    ok &= test_open_and_teleport_cycle();
    ok &= test_close_and_reopen_cycle();
    ok &= test_negative_foreign_map_close();

    if (g_assertions < 80) {
        printf("FAIL assertion budget got=%d want>=80\n", g_assertions);
        ok = 0;
        ++g_failures;
    }

    if (!ok || g_failures) {
        printf("FAIL dm1_v1_chest_teleport_survival_pc34_compat assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }

    printf("PASS dm1_v1_chest_teleport_survival_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
