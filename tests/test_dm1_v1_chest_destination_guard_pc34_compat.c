#include "dm1_v1_chest_destination_guard_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestDestinationGuardProbePc34 g_probe;
static int g_assertions;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_str_nonempty(const char* label,
                               const char* got,
                               const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || got[0] == '\0') {
        printf("FAIL %s empty string anchor=%s\n", label, redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%s anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int test_spec_and_evidence(void)
{
    const DM1_V1_ChestDestinationGuardSpecPc34* spec =
        M11_GameView_ChestDestinationGuardSpecPc34();
    const DM1_V1_ChestDestinationGuardEvidencePc34* evidence =
        M11_GameView_ChestDestinationGuardEvidencePc34();
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-76";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* f0172 = "ReDMCSB DUNGEON.C F0172 lines 2522-2523,2651-2692";
    const char* defs = "ReDMCSB DEFS.H lines 1001,1007-1013,1031-1035,2886";
    int ok = 1;

    ok &= expect_str_nonempty("contract marker", spec->contractMarker,
                              f0334);
    ok &= expect_int("C537 slot constant", spec->c537Pc34Slot,
                     DM1_PC34_CHEST_DESTINATION_GUARD_FIRST_SLOT, f0302);
    ok &= expect_int("C544 slot constant", spec->c544Pc34Slot,
                     DM1_PC34_CHEST_DESTINATION_GUARD_LAST_SLOT, f0302);
    ok &= expect_int("chest slot count", spec->chestSlotCount,
                     DM1_PC34_CHEST_DESTINATION_GUARD_SLOT_COUNT, f0333);
    ok &= expect_int("CM1_MAPX_NOT_ON_A_SQUARE constant",
                     spec->cm1MapXNotOnASquare,
                     DM1_PC34_CHEST_DESTINATION_GUARD_NOT_ON_SQUARE, defs);
    ok &= expect_int("teleporter element constant", spec->teleporterElement,
                     DM1_PC34_CHEST_DESTINATION_GUARD_ELEMENT_TELEPORTER,
                     defs);
    ok &= expect_int("fakewall element constant", spec->fakewallElement,
                     DM1_PC34_CHEST_DESTINATION_GUARD_ELEMENT_FAKEWALL,
                     defs);
    ok &= expect_int("corridor element constant", spec->corridorElement,
                     DM1_PC34_CHEST_DESTINATION_GUARD_ELEMENT_CORRIDOR,
                     defs);

    ok &= expect_str_nonempty("evidence contract scope",
                              evidence->contractScope, f0334);
    ok &= expect_str_nonempty("evidence chest open",
                              evidence->redmcsbChestOpen, f0333);
    ok &= expect_str_nonempty("evidence chest close",
                              evidence->redmcsbChestClose, f0334);
    ok &= expect_str_nonempty("evidence champion slot route",
                              evidence->redmcsbChampionSlotRoute, f0302);
    ok &= expect_str_nonempty("evidence dungeon link",
                              evidence->redmcsbDungeonLink, f0163);
    ok &= expect_str_nonempty("evidence dungeon aspect",
                              evidence->redmcsbDungeonAspect, f0172);
    ok &= expect_str_nonempty("evidence defs", evidence->redmcsbDefs, defs);
    ok &= expect_str_nonempty("evidence disjoint coverage",
                              evidence->disjointCoverage, f0302);
    return ok;
}

static int test_rejected_attempt(
    const char* phase,
    const DM1_V1_ChestDestinationGuardAttemptPc34* attempt,
    const char* destinationAnchor)
{
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    char label[128];
    int ok = 1;

    snprintf(label, sizeof(label), "%s rejected result", phase);
    ok &= expect_int(label, attempt->result, 0, destinationAnchor);
    snprintf(label, sizeof(label), "%s rejected flag", phase);
    ok &= expect_int(label, attempt->rejected, 1, destinationAnchor);
    snprintf(label, sizeof(label), "%s stable hash flag", phase);
    ok &= expect_int(label, attempt->stateStable, 1, f0334);
    snprintf(label, sizeof(label), "%s leader hand preserved", phase);
    ok &= expect_int(label, attempt->leaderHandAfter,
                     DM1_PC34_CHEST_DESTINATION_GUARD_ITEM, f0334);
    snprintf(label, sizeof(label), "%s chest slot remains empty", phase);
    ok &= expect_int(label, attempt->destinationSlotAfter, 0, f0334);
    snprintf(label, sizeof(label), "%s no dungeon link count", phase);
    ok &= expect_int(label, attempt->dungeonLinkCountAfter, 0, f0163);
    snprintf(label, sizeof(label), "%s square first thing untouched", phase);
    ok &= expect_int(label, attempt->squareFirstThingAfter,
                     DM1_PC34_CHEST_DESTINATION_GUARD_NONE, f0163);
    return ok;
}

static int test_probe_basics(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 31-76";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    int ok = 1;

    ok &= expect_int("contract-only marker", g_probe.contract_only, 1,
                     f0334);
    ok &= expect_int("no real asset data marker", g_probe.no_real_asset_data,
                     1, f0334);
    ok &= expect_int("open chest sentinel", g_probe.openChestThing,
                     DM1_PC34_CHEST_DESTINATION_GUARD_OPEN_CHEST, f0333);
    ok &= expect_int("leader hand sentinel", g_probe.leaderHandItem,
                     DM1_PC34_CHEST_DESTINATION_GUARD_ITEM, f0334);
    ok &= expect_int("target chest slot", g_probe.targetChestSlot,
                     DM1_PC34_CHEST_DESTINATION_GUARD_FIRST_SLOT, f0334);
    ok &= expect_int("initial dungeon link count",
                     g_probe.initialDungeonLinkCount, 0, f0334);
    ok &= expect_int("initial square first thing",
                     g_probe.initialSquareFirstThing,
                     DM1_PC34_CHEST_DESTINATION_GUARD_NONE, f0334);
    return ok;
}

static int test_guard_contract(void)
{
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* f0172 = "ReDMCSB DUNGEON.C F0172 lines 2522-2523,2651-2692";
    int ok = 1;

    ok &= test_rejected_attempt("teleporter destination",
                                &g_probe.teleporterAttempt, f0172);
    ok &= test_rejected_attempt("fakewall destination",
                                &g_probe.fakewallAttempt, f0172);
    ok &= test_rejected_attempt("corridor square destination",
                                &g_probe.corridorSquareAttempt, f0163);

    ok &= expect_int("closed chest rejected result",
                     g_probe.closedChestAttempt.result, 0, f0334);
    ok &= expect_int("closed chest rejected flag",
                     g_probe.closedChestAttempt.rejected, 1, f0334);
    ok &= expect_int("closed chest state stable",
                     g_probe.closedChestAttempt.stateStable, 1, f0334);
    ok &= expect_int("closed chest leader hand preserved",
                     g_probe.closedChestAttempt.leaderHandAfter,
                     DM1_PC34_CHEST_DESTINATION_GUARD_ITEM, f0334);
    ok &= expect_int("closed chest open sentinel stays closed",
                     g_probe.closedChestAttempt.openChestThingAfter,
                     DM1_PC34_CHEST_DESTINATION_GUARD_CLOSED_CHEST, f0334);
    ok &= expect_int("closed chest no dungeon link",
                     g_probe.closedChestAttempt.dungeonLinkCountAfter, 0,
                     f0163);

    ok &= expect_int("internal chest accepted result",
                     g_probe.internalChestAttempt.result, 1, f0302);
    ok &= expect_int("internal chest rejected flag",
                     g_probe.internalChestAttempt.rejected, 0, f0302);
    ok &= expect_int("internal chest state changed",
                     g_probe.internalChestAttempt.stateStable, 0, f0302);
    ok &= expect_int("internal chest stores leader item",
                     g_probe.internalStoredItem, 1, f0302);
    ok &= expect_int("internal chest destination slot item",
                     g_probe.internalChestAttempt.destinationSlotAfter,
                     DM1_PC34_CHEST_DESTINATION_GUARD_ITEM, f0302);
    ok &= expect_int("internal chest clears leader hand",
                     g_probe.internalLeaderHandEmpty, 1, f0302);
    ok &= expect_int("internal chest leader hand sentinel",
                     g_probe.internalChestAttempt.leaderHandAfter,
                     DM1_PC34_CHEST_DESTINATION_GUARD_NONE, f0302);
    ok &= expect_int("internal chest creates no dungeon link",
                     g_probe.internalNoDungeonLink, 1, f0163);
    ok &= expect_int("internal chest link count",
                     g_probe.internalChestAttempt.dungeonLinkCountAfter, 0,
                     f0163);
    ok &= expect_int("internal chest square first thing",
                     g_probe.internalChestAttempt.squareFirstThingAfter,
                     DM1_PC34_CHEST_DESTINATION_GUARD_NONE, f0163);
    ok &= expect_int("internal chest keeps open chest",
                     g_probe.internalOpenChestStable, 1, f0334);
    ok &= expect_int("all rejected attempts stable",
                     g_probe.rejectedAttemptsStable, 1, f0334);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestDestinationGuardEvidencePc34* evidence =
        M11_GameView_ChestDestinationGuardEvidencePc34();
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    int ok = 1;

    printf("probe=dm1_v1_chest_destination_guard_pc34_compat\n");
    printf("sourceEvidence=%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
           evidence->contractScope,
           evidence->redmcsbChestOpen,
           evidence->redmcsbChestClose,
           evidence->redmcsbChampionSlotRoute,
           evidence->redmcsbDungeonLink,
           evidence->redmcsbDungeonAspect,
           evidence->redmcsbDefs,
           evidence->disjointCoverage);

    ok &= expect_int("probe setup",
                     M11_GameView_ChestDestinationGuardRunPc34(&g_probe),
                     1, f0334);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestDestinationGuardInvariantOk=0\n");
        return 1;
    }

    ok &= test_spec_and_evidence();
    ok &= test_probe_basics();
    ok &= test_guard_contract();
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 55 ? 1 : 0, 1, f0334);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestDestinationGuardInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
