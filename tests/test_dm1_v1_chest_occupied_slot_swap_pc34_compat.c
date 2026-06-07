#include "dm1_v1_chest_occupied_slot_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestOccupiedSlotSwapProbePc34 g_probe;
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

static int expect_uint_equal(const char* label,
                             unsigned int got,
                             unsigned int want,
                             const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%u anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_str(const char* label,
                      const char* got,
                      const char* want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=%s want=%s anchor=%s\n",
               label, got ? got : "(null)", want ? want : "(null)",
               redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%s anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int test_spec(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0031 =
        "ReDMCSB OBJECT.C F0031 lines 25-120";
    const DM1_V1_ChestOccupiedSlotSwapSpecPc34* spec =
        dm1_v1_chest_occupied_slot_swap_spec_pc34();
    int ok = 1;

    ok &= expect_str("contract marker", spec->contractMarker,
                     "Source-locked contract gate only; not full real-asset chest runtime parity.",
                     f0333);
    ok &= expect_int("probe contract-only marker",
                     g_probe.sourceLockedContractOnly, 1, f0333);
    ok &= expect_int("C537 slot constant", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, f0333);
    ok &= expect_int("C544 slot constant", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, f0333);
    ok &= expect_int("chest slot count", spec->chestSlotCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333);
    ok &= expect_int("case count", spec->caseCount,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CASE_COUNT, f0333);
    ok &= expect_int("backpack source index", spec->backpackSourceIndex,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_SOURCE_INDEX,
                     f0333);
    ok &= expect_int("backpack destination slot",
                     spec->backpackDestinationPc34Slot,
                     DM1_PC34_SLOT_BACKPACK_LINE1_2, f0333);
    ok &= expect_int("chest B source index", spec->chestBSourceIndex,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_SOURCE_INDEX,
                     f0333);
    ok &= expect_int("chest B destination index",
                     spec->chestBDestinationIndex,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_DEST_INDEX,
                     f0333);
    ok &= expect_int("deterministic backpack potion id",
                     spec->backpackPotion.itemType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_POTION, f0031);
    ok &= expect_int("deterministic chest A weapon id",
                     spec->chestAWeapon.itemType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_WEAPON, f0031);
    return ok;
}

static int assert_original_order(
    const char* caseName,
    const DM1_V1_ChestOccupiedSlotSwapCasePc34* c,
    int firstType,
    int visibleCount)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0031 =
        "ReDMCSB OBJECT.C F0031 lines 25-120";
    int ok = 1;
    int i;

    ok &= expect_int(caseName, c->sourceVisibleCountBefore,
                     visibleCount, f0333);
    for (i = 0; i < visibleCount; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "%s original C537-C544 order %d",
                 caseName, i);
        ok &= expect_int(label, c->originalVisibleTypes[i],
                         firstType + i, f0031);
    }
    for (i = visibleCount; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "%s original empty visible slot %d",
                 caseName, i);
        ok &= expect_int(label, c->originalVisibleTypes[i], 0, f0333);
    }
    return ok;
}

static int assert_final_order(
    const char* caseName,
    const DM1_V1_ChestOccupiedSlotSwapCasePc34* c,
    int visibleCount)
{
    const char* f0334 =
        "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0163 =
        "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    int ok = 1;
    int i;

    ok &= expect_int(caseName, c->finalVisibleCount, visibleCount, f0334);
    ok &= expect_int("visible head unchanged", c->visibleHeadUnchanged, 1,
                     f0163);
    ok &= expect_int("visible tail unchanged", c->visibleTailUnchanged, 1,
                     f0163);
    ok &= expect_int("visible head membership count",
                     c->finalHeadMembershipCount, 1, f0163);
    ok &= expect_int("visible order unchanged", c->visibleOrderUnchanged, 1,
                     f0163);
    ok &= expect_int("reopened order unchanged", c->reopenedOrderUnchanged, 1,
                     f0334);
    for (i = 0; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "%s final visible slot %d",
                 caseName, i);
        ok &= expect_int(label, c->finalVisibleTypes[i],
                         c->originalVisibleTypes[i], f0163);
        snprintf(label, sizeof(label), "%s closed visible slot %d",
                 caseName, i);
        ok &= expect_int(label, c->closedTypes[i],
                         c->originalVisibleTypes[i], f0334);
        snprintf(label, sizeof(label), "%s reopened visible slot %d",
                 caseName, i);
        ok &= expect_int(label, c->reopenedTypes[i],
                         c->originalVisibleTypes[i], f0334);
    }
    return ok;
}

static int assert_world_hash_stable(
    const DM1_V1_ChestOccupiedSlotSwapCasePc34* c,
    const char* caseName)
{
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0297/F0298/F0302 lines 250-298,688-710";
    int ok = 1;

    ok &= expect_int(caseName, c->worldHashBeforeResult, 1, f0302);
    ok &= expect_int("world hash after result", c->worldHashAfterResult, 1,
                     f0302);
    ok &= expect_uint_equal("world hash unchanged value",
                            c->worldHashAfter, c->worldHashBefore, f0302);
    ok &= expect_int("world hash unchanged flag", c->worldHashUnchanged, 1,
                     f0302);
    return ok;
}

static int test_backpack_round_trip(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 =
        "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0297/F0298/F0302 lines 250-298,688-710";
    const char* f0163 =
        "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const DM1_V1_ChestOccupiedSlotSwapCasePc34* c =
        &g_probe.cases[DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_CASE];
    int ok = 1;

    ok &= expect_int("backpack source opens", c->sourceOpenResult, 1,
                     f0333);
    ok &= expect_int("backpack source open thing", c->sourceOpenThing,
                     c->sourceChestThing, f0333);
    ok &= expect_int("backpack hidden tail input", c->sourceHiddenTailInput,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_HIDDEN_TAIL,
                     f0333);
    ok &= assert_original_order("backpack case",
                                c,
                                DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_CHEST_FIRST,
                                DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT);
    ok &= expect_int("backpack leader starts with replacement",
                     c->leaderHandBefore,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_REPLACEMENT,
                     f0302);
    ok &= expect_int("backpack source swap click",
                     c->sourceSwapClickResult, 1, f0302);
    ok &= expect_int("backpack source slot receives replacement",
                     c->sourceSlotAfterSourceSwap,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_REPLACEMENT,
                     f0302);
    ok &= expect_int("backpack replacement stored at original index",
                     c->sourceReplacementStoredAtOriginalIndex, 1, f0302);
    ok &= expect_int("backpack potion moves to leader",
                     c->leaderHandAfterSourceSwap,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_POTION,
                     f0302);
    ok &= expect_int("backpack swapped object identity",
                     c->swappedObjectType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_POTION,
                     f0302);
    ok &= expect_int("backpack destination swap click",
                     c->destinationSwapClickResult, 1, f0302);
    ok &= expect_int("backpack potion stored in destination",
                     c->destinationAfterSwapType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_POTION,
                     f0302);
    ok &= expect_int("backpack destination occupant moves to leader",
                     c->leaderHandAfterDestinationSwap,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_DEST_OCCUPANT,
                     f0302);
    ok &= expect_int("backpack occupant moved flag",
                     c->destinationOccupantMovedToLeader, 1, f0302);
    ok &= expect_int("backpack return click",
                     c->destinationReturnClickResult, 1, f0302);
    ok &= expect_int("backpack destination occupant restored",
                     c->destinationAfterReturnType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_DEST_OCCUPANT,
                     f0302);
    ok &= expect_int("backpack swapped object ready for reinsert",
                     c->swappedObjectReadyForReinsert, 1, f0302);
    ok &= expect_int("backpack reinsert click",
                     c->sourceReinsertClickResult, 1, f0302);
    ok &= expect_int("backpack potion restored to source index",
                     c->sourceSlotAfterReinsert,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_BACKPACK_POTION,
                     f0302);
    ok &= expect_int("backpack replacement returned to leader",
                     c->replacementReturnedToLeader, 1, f0302);
    ok &= expect_int("backpack leader hand stable",
                     c->leaderHandStable, 1, f0302);
    ok &= expect_int("backpack close count",
                     c->sourceCloseCount,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT, f0334);
    ok &= expect_int("backpack hidden tail excluded on close",
                     c->hiddenTailClosed, 0, f0334);
    ok &= expect_int("backpack reopen after close",
                     c->sourceReopenAfterCloseResult, 1, f0333);
    ok &= expect_int("backpack hidden tail excluded on reopen",
                     c->hiddenTailReopened, 0, f0333);
    ok &= expect_int("backpack no duplicate object ids",
                     c->noDuplicateObjectIds, 1, f0163);
    ok &= expect_int("backpack no evictions",
                     c->noEvictions, 1, f0163);
    ok &= assert_final_order("backpack case", c,
                             DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT);
    ok &= assert_world_hash_stable(c, "backpack world hash before");
    return ok;
}

static int test_chest_b_round_trip(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 =
        "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0297/F0298/F0302 lines 250-298,688-710";
    const char* f0163 =
        "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const DM1_V1_ChestOccupiedSlotSwapCasePc34* c =
        &g_probe.cases[DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_CASE];
    int ok = 1;

    ok &= expect_int("chest B source opens", c->sourceOpenResult, 1,
                     f0333);
    ok &= expect_int("chest B source open thing", c->sourceOpenThing,
                     c->sourceChestThing, f0333);
    ok &= expect_int("chest B hidden tail input", c->sourceHiddenTailInput,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_HIDDEN_TAIL,
                     f0333);
    ok &= assert_original_order("chest B case",
                                c,
                                DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_FIRST,
                                3);
    ok &= expect_int("chest B leader starts with replacement",
                     c->leaderHandBefore,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_REPLACEMENT,
                     f0302);
    ok &= expect_int("chest B source swap click",
                     c->sourceSwapClickResult, 1, f0302);
    ok &= expect_int("chest B source slot receives replacement",
                     c->sourceSlotAfterSourceSwap,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_REPLACEMENT,
                     f0302);
    ok &= expect_int("chest B weapon moves to leader",
                     c->leaderHandAfterSourceSwap,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_WEAPON,
                     f0302);
    ok &= expect_int("chest B swapped object identity",
                     c->swappedObjectType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_WEAPON,
                     f0302);
    ok &= expect_int("chest A closes before chest B opens",
                     c->sourceReopenResult, 1, f0334);
    ok &= expect_int("chest B destination swap click",
                     c->destinationSwapClickResult, 1, f0302);
    ok &= expect_int("chest B weapon stored in destination",
                     c->destinationAfterSwapType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_WEAPON,
                     f0302);
    ok &= expect_int("chest B destination occupant moves to leader",
                     c->leaderHandAfterDestinationSwap,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_DEST_OCCUPANT,
                     f0302);
    ok &= expect_int("chest B return click",
                     c->destinationReturnClickResult, 1, f0302);
    ok &= expect_int("chest B destination occupant restored",
                     c->destinationAfterReturnType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_DEST_OCCUPANT,
                     f0302);
    ok &= expect_int("chest B swapped object ready for reinsert",
                     c->swappedObjectReadyForReinsert, 1, f0302);
    ok &= expect_int("chest B close count while reopening source",
                     c->destinationCloseCount, 1, f0334);
    ok &= expect_int("chest B closed occupant restored",
                     c->destinationClosedTypes[0],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_B_DEST_OCCUPANT,
                     f0334);
    ok &= expect_int("chest B reinsert click",
                     c->sourceReinsertClickResult, 1, f0302);
    ok &= expect_int("chest B weapon restored to source index",
                     c->sourceSlotAfterReinsert,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_CHEST_A_WEAPON,
                     f0302);
    ok &= expect_int("chest B replacement returned to leader",
                     c->replacementReturnedToLeader, 1, f0302);
    ok &= expect_int("chest B leader hand stable",
                     c->leaderHandStable, 1, f0302);
    ok &= expect_int("chest B source close count",
                     c->sourceCloseCount, 3, f0334);
    ok &= expect_int("chest B hidden tail excluded on close",
                     c->hiddenTailClosed, 0, f0334);
    ok &= expect_int("chest B reopen after close",
                     c->sourceReopenAfterCloseResult, 1, f0333);
    ok &= expect_int("chest B hidden tail excluded on reopen",
                     c->hiddenTailReopened, 0, f0333);
    ok &= expect_int("chest B no duplicate object ids",
                     c->noDuplicateObjectIds, 1, f0163);
    ok &= expect_int("chest B no evictions",
                     c->noEvictions, 1, f0163);
    ok &= assert_final_order("chest B case", c, 3);
    ok &= assert_world_hash_stable(c, "chest B world hash before");
    return ok;
}

int main(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    int ok = 1;

    printf("probe=dm1_v1_chest_occupied_slot_swap_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_occupied_slot_swap_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_occupied_slot_swap_pc34(&g_probe),
                     1, f0333);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestOccupiedSlotSwapInvariantOk=0\n");
        return 1;
    }

    ok &= test_spec();
    ok &= test_backpack_round_trip();
    ok &= test_chest_b_round_trip();

    ok &= expect_int("minimum assertion count",
                     g_assertions >= 60 ? 1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestOccupiedSlotSwapInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
