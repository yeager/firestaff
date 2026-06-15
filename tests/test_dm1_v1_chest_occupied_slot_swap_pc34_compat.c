#include "firestaff/dm1/v1/chest/occupied_slot_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char* label, int got, int want, const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_u32(const char* label,
                      uint32_t got,
                      uint32_t want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08X want=0x%08X anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=0x%08X anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || !haystack || !needle ||
        !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

static int expect_slot_array(const char* label,
                             const int* got,
                             const int* want,
                             const char* anchor)
{
    int i;
    int ok = 1;

    for (i = 0; i < DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34; ++i) {
        char slotLabel[96];

        snprintf(slotLabel, sizeof(slotLabel), "%s C%d", label,
                 DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C537_BASE_PC34 + i);
        ok &= expect_int(slotLabel, got[i], want[i], anchor);
    }
    return ok;
}

static int test_source_evidence(
    const DM1_V1_ChestOccupiedSlotSwapSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_occupied_slot_swap_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence F0333", evidence,
                          "CHEST.C F0333:30-75", spec->f0333Anchor);
    ok &= expect_contains("evidence F0334 negative", evidence,
                          "CHEST.C F0334:117-132",
                          spec->f0334NegativeAnchor);
    ok &= expect_contains("evidence F0297", evidence,
                          "CHAMPION.C F0297:243-268",
                          spec->f0297Anchor);
    ok &= expect_contains("evidence F0298", evidence,
                          "CHAMPION.C F0298:270-298",
                          spec->f0298Anchor);
    ok &= expect_contains("evidence F0300", evidence,
                          "CHAMPION.C F0300:511-515",
                          spec->f0300Anchor);
    ok &= expect_contains("evidence F0301", evidence,
                          "CHAMPION.C F0301:606-660",
                          spec->f0301Anchor);
    ok &= expect_contains("evidence F0302", evidence,
                          "CHAMPION.C F0302:662-713",
                          spec->f0302Anchor);
    ok &= expect_contains("evidence F0140", evidence,
                          "DUNGEON.C F0140:1114-1120",
                          spec->f0140Anchor);
    ok &= expect_contains("evidence F0159", evidence,
                          "F0159:1664-1681", spec->f0159Anchor);
    ok &= expect_contains("evidence F0163", evidence,
                          "F0163:1769-1838", spec->f0163Anchor);
    ok &= expect_contains("evidence F0164", evidence,
                          "F0164:1840-1905", spec->f0164Anchor);
    ok &= expect_contains("evidence OBJECT F0032", evidence,
                          "OBJECT.C F0032:121-145", spec->f0032Anchor);
    ok &= expect_contains("evidence OBJECT F0033", evidence,
                          "F0033:147-212", spec->f0033Anchor);
    ok &= expect_contains("evidence F0359", evidence,
                          "COMMAND.C F0359:1452-1662",
                          spec->f0359Anchor);
    ok &= expect_contains("evidence F0380", evidence,
                          "F0380:2045-2178", spec->f0380Anchor);
    ok &= expect_contains("evidence F0077", evidence,
                          "IO.C F0077:1113-1122", spec->f0077Anchor);
    ok &= expect_contains("evidence F0078", evidence,
                          "F0078:1102-1111", spec->f0078Anchor);
    ok &= expect_contains("evidence DEFS", evidence,
                          "DEFS.H C30/C537..C544/G0425/G0426",
                          spec->defsAnchor);
    ok &= expect_contains("evidence disjoint F0334", evidence,
                          "no F0334 close path",
                          spec->f0334NegativeAnchor);
    ok &= expect_contains("evidence disjoint eye", evidence,
                          "no C071 eye route", spec->disjointness);
    ok &= expect_contains("evidence disjoint wheel", evidence,
                          "no scroll wheel", spec->disjointness);
    return ok;
}

static int test_spec(
    const DM1_V1_ChestOccupiedSlotSwapSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_contains("spec marker", spec->contractMarker,
                          "leader-hand to non-leader-hand",
                          spec->f0302Anchor);
    ok &= expect_int("contractOnly", spec->contractOnly, 1,
                     spec->f0334NegativeAnchor);
    ok &= expect_int("deterministic seed", spec->deterministicSeed,
                     0xC540C002, spec->f0359Anchor);
    ok &= expect_int("party count", spec->partyChampionCount, 4,
                     spec->defsAnchor);
    ok &= expect_int("inventory ordinal", spec->inventoryChampionOrdinal, 1,
                     spec->defsAnchor);
    ok &= expect_int("leader index", spec->leaderIndex, 0,
                     spec->defsAnchor);
    ok &= expect_int("non-leader index", spec->nonLeaderIndex, 1,
                     spec->defsAnchor);
    ok &= expect_int("target index", spec->targetChestIndex, 3,
                     spec->defsAnchor);
    ok &= expect_int("target pc34 slot", spec->targetPc34Slot, 33,
                     spec->defsAnchor);
    ok &= expect_int("target zone C540", spec->targetZone, 540,
                     spec->defsAnchor);
    ok &= expect_int("non-leader slot box", spec->nonLeaderSlotBox, 2,
                     spec->f0302Anchor);
    ok &= expect_int("non-leader hand slot", spec->nonLeaderHandSlot, 0,
                     spec->defsAnchor);
    ok &= expect_int("thing none", spec->thingNone, 0xFFFF,
                     spec->f0300Anchor);
    ok &= expect_int("thing end", spec->thingEnd, 0xFFFE,
                     spec->f0163Anchor);
    ok &= expect_contains("spec F0333 anchor", spec->f0333Anchor,
                          "F0333 lines 30-75", spec->f0333Anchor);
    ok &= expect_contains("spec F0334 negative anchor",
                          spec->f0334NegativeAnchor,
                          "negative no-close", spec->f0334NegativeAnchor);
    ok &= expect_contains("spec F0297 anchor", spec->f0297Anchor,
                          "F0297 lines 243-268", spec->f0297Anchor);
    ok &= expect_contains("spec F0298 anchor", spec->f0298Anchor,
                          "F0298 lines 270-298", spec->f0298Anchor);
    ok &= expect_contains("spec F0300 anchor", spec->f0300Anchor,
                          "F0300 lines 511-515", spec->f0300Anchor);
    ok &= expect_contains("spec F0301 anchor", spec->f0301Anchor,
                          "F0301 lines 606-660", spec->f0301Anchor);
    ok &= expect_contains("spec F0302 anchor", spec->f0302Anchor,
                          "F0302 lines 662-713", spec->f0302Anchor);
    ok &= expect_contains("spec F0140 anchor", spec->f0140Anchor,
                          "F0140 lines 1114-1120", spec->f0140Anchor);
    ok &= expect_contains("spec F0159 anchor", spec->f0159Anchor,
                          "F0159 lines 1664-1681", spec->f0159Anchor);
    ok &= expect_contains("spec F0163 anchor", spec->f0163Anchor,
                          "F0163 lines 1769-1838", spec->f0163Anchor);
    ok &= expect_contains("spec F0164 anchor", spec->f0164Anchor,
                          "F0164 lines 1840-1905", spec->f0164Anchor);
    ok &= expect_contains("spec F0032 anchor", spec->f0032Anchor,
                          "F0032 lines 121-145", spec->f0032Anchor);
    ok &= expect_contains("spec F0033 anchor", spec->f0033Anchor,
                          "F0033 lines 147-212", spec->f0033Anchor);
    ok &= expect_contains("spec F0359 anchor", spec->f0359Anchor,
                          "F0359 lines 1452-1662", spec->f0359Anchor);
    ok &= expect_contains("spec F0380 anchor", spec->f0380Anchor,
                          "F0380 lines 2045-2178", spec->f0380Anchor);
    ok &= expect_contains("spec F0077 anchor", spec->f0077Anchor,
                          "F0077 lines 1113-1122", spec->f0077Anchor);
    ok &= expect_contains("spec F0078 anchor", spec->f0078Anchor,
                          "F0078 lines 1102-1111", spec->f0078Anchor);
    ok &= expect_contains("spec defs anchor", spec->defsAnchor,
                          "C30/C537..C544", spec->defsAnchor);
    ok &= expect_contains("spec disjoint", spec->disjointness,
                          "Disjoint from F0334", spec->disjointness);
    return ok;
}

static int test_setup(
    const DM1_V1_ChestOccupiedSlotSwapSpecPc34* spec,
    const DM1_V1_ChestOccupiedSlotSwapProbePc34* p)
{
    const int beforeTypes[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34] = {
        0x7300, 0x7301, 0x7302,
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_WEAPON_PC34,
        0x7304, 0x7305, 0x7306, 0x7307
    };
    const int beforeNext[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34] = {
        0x7301, 0x7302, DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_WEAPON_PC34,
        0x7304, 0x7305, 0x7306, 0x7307,
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_END_PC34
    };
    int ok = 1;

    ok &= expect_int("setup result", p->setupResult, 1, spec->f0333Anchor);
    ok &= expect_int("probe contract-only", p->contractOnly, 1,
                     spec->f0334NegativeAnchor);
    ok &= expect_int("probe seed", p->deterministicSeed,
                     spec->deterministicSeed, spec->f0359Anchor);
    ok &= expect_int("party count", p->partyChampionCount,
                     spec->partyChampionCount, spec->defsAnchor);
    ok &= expect_int("inventory ordinal", p->inventoryChampionOrdinal,
                     spec->inventoryChampionOrdinal, spec->defsAnchor);
    ok &= expect_int("candidate ordinal", p->candidateChampionOrdinal, 0,
                     spec->f0302Anchor);
    ok &= expect_int("leader index", p->leaderIndex, spec->leaderIndex,
                     spec->defsAnchor);
    ok &= expect_int("non-leader index", p->nonLeaderIndex,
                     spec->nonLeaderIndex, spec->defsAnchor);
    ok &= expect_int("leader current health", p->leaderCurrentHealth, 100,
                     spec->f0302Anchor);
    ok &= expect_int("non-leader current health",
                     p->nonLeaderCurrentHealth, 100, spec->f0302Anchor);
    ok &= expect_int("open chest before", p->openChestBefore,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_OPEN_CHEST_PC34,
                     spec->f0333Anchor);
    ok &= expect_int("panel chest before", p->panelWasChestBefore, 1,
                     spec->f0333Anchor);
    ok &= expect_int("leader hand before", p->leaderHandBefore,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_WEAPON_PC34,
                     spec->f0297Anchor);
    ok &= expect_int("non-leader hand before", p->nonLeaderHandBefore,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34,
                     spec->f0302Anchor);
    ok &= expect_int("C540 before", p->targetChestBefore,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_WEAPON_PC34,
                     spec->f0300Anchor);
    ok &= expect_slot_array("before G0425 types", p->beforeChestTypes,
                            beforeTypes, spec->f0333Anchor);
    ok &= expect_slot_array("before open chain next", p->beforeChestNext,
                            beforeNext, spec->f0159Anchor);
    return ok;
}

static int test_queue(
    const DM1_V1_ChestOccupiedSlotSwapSpecPc34* spec,
    const DM1_V1_ChestOccupiedSlotSwapProbePc34* p)
{
    int ok = 1;

    ok &= expect_int("queued count", p->queuedCount, 2, spec->f0359Anchor);
    ok &= expect_int("write order first C540", p->queueWriteOrder[0], 61,
                     spec->f0359Anchor);
    ok &= expect_int("write order second non-leader",
                     p->queueWriteOrder[1], 20, spec->f0359Anchor);
    ok &= expect_int("drain order first non-leader",
                     p->queueDrainOrder[0], 20, spec->f0380Anchor);
    ok &= expect_int("drain order second C540",
                     p->queueDrainOrder[1], 61, spec->f0380Anchor);
    ok &= expect_int("command0 is C540", p->commands[0].command, 61,
                     spec->f0359Anchor);
    ok &= expect_int("command0 zone", p->commands[0].zone,
                     spec->targetZone, spec->defsAnchor);
    ok &= expect_int("command0 pc34 slot", p->commands[0].pc34Slot,
                     spec->targetPc34Slot, spec->defsAnchor);
    ok &= expect_int("command0 champion resolves leader inventory",
                     p->commands[0].resolvedChampion, spec->leaderIndex,
                     spec->f0302Anchor);
    ok &= expect_int("command0 no hand slot", p->commands[0].resolvedHandSlot,
                     -1, spec->f0302Anchor);
    ok &= expect_int("command1 is non-leader ready",
                     p->commands[1].command, 20, spec->f0359Anchor);
    ok &= expect_int("command1 slotbox", p->commands[1].zone,
                     spec->nonLeaderSlotBox, spec->defsAnchor);
    ok &= expect_int("command1 pc34 hand", p->commands[1].pc34Slot,
                     spec->nonLeaderHandSlot, spec->defsAnchor);
    ok &= expect_int("command1 champion resolves non-leader",
                     p->commands[1].resolvedChampion, spec->nonLeaderIndex,
                     spec->f0302Anchor);
    ok &= expect_int("command1 hand slot",
                     p->commands[1].resolvedHandSlot,
                     spec->nonLeaderHandSlot, spec->f0302Anchor);
    ok &= expect_int("F0359 queued C540", p->f0359QueuedChestC540, 1,
                     spec->f0359Anchor);
    ok &= expect_int("F0359 queued non-leader",
                     p->f0359QueuedNonLeaderHand, 1, spec->f0359Anchor);
    ok &= expect_int("F0380 drains non-leader first",
                     p->f0380DrainedNonLeaderFirst, 1, spec->f0380Anchor);
    ok &= expect_int("F0380 drains C540 second",
                     p->f0380DrainedChestSecond, 1, spec->f0380Anchor);
    return ok;
}

static int test_receive_step(
    const DM1_V1_ChestOccupiedSlotSwapSpecPc34* spec,
    const DM1_V1_ChestOccupiedSlotSwapProbePc34* p)
{
    int ok = 1;

    ok &= expect_int("receive resolves non-leader",
                     p->f0302ResolvedNonLeaderHand, 1, spec->f0302Anchor);
    ok &= expect_int("receive removed leader hand",
                     p->f0298RemovedLeaderWeaponForReceive, 1,
                     spec->f0298Anchor);
    ok &= expect_int("receive wrote non-leader hand",
                     p->f0301WroteLeaderWeaponToNonLeaderHand, 1,
                     spec->f0301Anchor);
    ok &= expect_int("leader hand after receive",
                     p->leaderHandAfterReceive,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34,
                     spec->f0298Anchor);
    ok &= expect_int("non-leader hand after receive",
                     p->nonLeaderHandAfterReceive,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_WEAPON_PC34,
                     spec->f0301Anchor);
    ok &= expect_int("C540 still occupied after receive",
                     p->targetChestAfterReceive,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_WEAPON_PC34,
                     spec->f0300Anchor);
    ok &= expect_int("open chest after receive", p->openChestAfterReceive,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_OPEN_CHEST_PC34,
                     spec->f0333Anchor);
    ok &= expect_slot_array("receive G0425 types",
                            p->afterReceiveChestTypes,
                            p->beforeChestTypes, spec->f0333Anchor);
    ok &= expect_slot_array("receive chain next",
                            p->afterReceiveChestNext,
                            p->beforeChestNext, spec->f0159Anchor);
    return ok;
}

static int test_chest_step(
    const DM1_V1_ChestOccupiedSlotSwapSpecPc34* spec,
    const DM1_V1_ChestOccupiedSlotSwapProbePc34* p)
{
    const int finalTypes[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34] = {
        0x7300, 0x7301, 0x7302,
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34,
        0x7304, 0x7305, 0x7306, 0x7307
    };
    const int finalNext[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34] = {
        0x7301, 0x7302, 0x7304,
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34,
        0x7305, 0x7306, 0x7307,
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_END_PC34
    };
    int ok = 1;

    ok &= expect_int("exercise result", p->exerciseResult, 1,
                     spec->f0380Anchor);
    ok &= expect_int("chest resolves C540", p->f0302ResolvedChestC540, 1,
                     spec->f0302Anchor);
    ok &= expect_int("F0300 cleared C540", p->f0300ClearedC540, 1,
                     spec->f0300Anchor);
    ok &= expect_int("F0297 put C540 in leader hand",
                     p->f0297PutC540WeaponInLeaderHand, 1,
                     spec->f0297Anchor);
    ok &= expect_int("F0164 detached old C540",
                     p->f0164DetachedOldC540FromOpenChain, 1,
                     spec->f0164Anchor);
    ok &= expect_int("leader hand after C540 click",
                     p->leaderHandAfterChestClick,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_WEAPON_PC34,
                     spec->f0297Anchor);
    ok &= expect_int("non-leader hand after C540 click",
                     p->nonLeaderHandAfterChestClick,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_WEAPON_PC34,
                     spec->f0301Anchor);
    ok &= expect_int("C540 after click", p->targetChestAfterChestClick,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34,
                     spec->f0300Anchor);
    ok &= expect_int("open chest after C540 click",
                     p->openChestAfterChestClick,
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_OPEN_CHEST_PC34,
                     spec->f0333Anchor);
    ok &= expect_int("panel still chest", p->panelStillChestAfter, 1,
                     spec->f0333Anchor);
    ok &= expect_slot_array("final G0425 types", p->afterChestTypes,
                            finalTypes, spec->f0300Anchor);
    ok &= expect_slot_array("final chain next", p->afterChestNext,
                            finalNext, spec->f0164Anchor);
    ok &= expect_slot_array("expected final types",
                            p->expectedAfterChestTypes, finalTypes,
                            spec->f0164Anchor);
    ok &= expect_slot_array("expected final next",
                            p->expectedAfterChestNext, finalNext,
                            spec->f0164Anchor);
    ok &= expect_int("C537 stable", p->c537Stable, 1, spec->f0333Anchor);
    ok &= expect_int("C538 stable", p->c538Stable, 1, spec->f0333Anchor);
    ok &= expect_int("C539 stable", p->c539Stable, 1, spec->f0333Anchor);
    ok &= expect_int("C541 stable", p->c541Stable, 1, spec->f0333Anchor);
    ok &= expect_int("C542 stable", p->c542Stable, 1, spec->f0333Anchor);
    ok &= expect_int("C543 stable", p->c543Stable, 1, spec->f0333Anchor);
    ok &= expect_int("C544 stable", p->c544Stable, 1, spec->f0333Anchor);
    ok &= expect_int("C540 cleared", p->c540Cleared, 1,
                     spec->f0300Anchor);
    ok &= expect_int("no duplicate leader weapon",
                     p->noDuplicateLeaderWeapon, 1, spec->f0301Anchor);
    ok &= expect_int("no duplicate C540 weapon", p->noDuplicateC540Weapon,
                     1, spec->f0297Anchor);
    ok &= expect_int("open chain skips detached C540",
                     p->noThingNoneInsideNonEmptyPrefix, 1,
                     spec->f0164Anchor);
    ok &= expect_int("old leader moved to non-leader",
                     p->oldLeaderWeaponMovedToNonLeader, 1,
                     spec->f0301Anchor);
    ok &= expect_int("old C540 moved to leader",
                     p->oldC540WeaponMovedToLeader, 1,
                     spec->f0297Anchor);
    ok &= expect_int("chest stayed open", p->chestStayedOpen, 1,
                     spec->f0333Anchor);
    ok &= expect_int("close path not used", p->closePathNotUsed, 1,
                     spec->f0334NegativeAnchor);
    return ok;
}

static int test_negative_and_hash(
    const DM1_V1_ChestOccupiedSlotSwapSpecPc34* spec,
    const DM1_V1_ChestOccupiedSlotSwapProbePc34* p)
{
    int ok = 1;

    ok &= expect_int("F0334 close call count", p->f0334CloseCallCount, 0,
                     spec->f0334NegativeAnchor);
    ok &= expect_int("F0333 open call count", p->f0333OpenCallCount, 1,
                     spec->f0333Anchor);
    ok &= expect_int("F0077 call count", p->f0077CallCount, 2,
                     spec->f0077Anchor);
    ok &= expect_int("F0078 call count", p->f0078CallCount, 2,
                     spec->f0078Anchor);
    ok &= expect_u32("deterministic hash", p->hash, 0xDFF9738Fu,
                     spec->f0380Anchor);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestOccupiedSlotSwapSpecPc34* spec =
        dm1_v1_chest_occupied_slot_swap_spec_pc34();
    DM1_V1_ChestOccupiedSlotSwapProbePc34 probe;
    int ok = 1;

    memset(&probe, 0, sizeof(probe));
    printf("probe=pass_dm1_v1_chest_occupied_slot_swap_pc34_compat\n");
    ok &= expect_int("run result",
                     dm1_v1_chest_occupied_slot_swap_pc34(&probe), 1,
                     spec->f0380Anchor);
    ok &= test_source_evidence(spec);
    ok &= test_spec(spec);
    ok &= test_setup(spec, &probe);
    ok &= test_queue(spec, &probe);
    ok &= test_receive_step(spec, &probe);
    ok &= test_chest_step(spec, &probe);
    ok &= test_negative_and_hash(spec, &probe);

    printf("assertions=%d failures=%d hash=0x%08X\n",
           g_assertions, g_failures, probe.hash);
    return (ok && g_failures == 0) ? 0 : 1;
}
