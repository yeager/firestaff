#include "dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char* label,
                      int got,
                      int want,
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
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_u32(const char* label,
                      unsigned int got,
                      unsigned int want,
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

static int expect_slots(const char* label,
                        const int* got,
                        int firstThing,
                        const char* anchor)
{
    int ok = 1;
    int i;

    for (i = 0; i < DM1_PC34_C545_MIDCAST_SLOT_COUNT; ++i) {
        char slotLabel[96];

        snprintf(slotLabel, sizeof(slotLabel), "%s C%d", label,
                 DM1_PC34_C545_MIDCAST_C537 + i);
        ok &= expect_int(slotLabel, got[i], firstThing + i, anchor);
    }
    return ok;
}

static int test_source_evidence(
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_source_evidence_pc34();
    int ok = 1;

    ok &= expect_int("source pointer stable",
                     evidence == spec->sourceEvidence, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_contains("source F0333", evidence, "CHEST.C F0333:30-67",
                          spec->f0333OpenAnchor);
    ok &= expect_contains("source F0334", evidence, "CHEST.C F0334:113-132",
                          spec->f0334CloseAnchor);
    ok &= expect_contains("source F0284", evidence,
                          "CHAMPION.C F0284:93-131",
                          spec->f0284Anchor);
    ok &= expect_contains("source F0297", evidence,
                          "CHAMPION.C F0297:243-298",
                          spec->f0297Anchor);
    ok &= expect_contains("source F0298", evidence,
                          "CHAMPION.C F0298:270-298",
                          spec->f0298Anchor);
    ok &= expect_contains("source F0300", evidence,
                          "CHAMPION.C F0300:511-515",
                          spec->f0300Anchor);
    ok &= expect_contains("source F0301", evidence,
                          "CHAMPION.C F0301:606-614",
                          spec->f0301Anchor);
    ok &= expect_contains("source F0302", evidence,
                          "CHAMPION.C F0302:662-714",
                          spec->f0302Anchor);
    ok &= expect_contains("source F0344", evidence, "PANEL.C F0344",
                          spec->f0344Anchor);
    ok &= expect_contains("source F0345", evidence, "F0345",
                          spec->f0345Anchor);
    ok &= expect_contains("source F0352", evidence, "PANEL.C F0352",
                          spec->f0352Anchor);
    ok &= expect_contains("source F0359", evidence,
                          "COMMAND.C F0359:1985-1990",
                          spec->f0359Anchor);
    ok &= expect_contains("source defs", evidence, "DEFS.H:2088",
                          spec->defsAnchor);
    ok &= expect_contains("source G0425", evidence, "G0425",
                          spec->defsAnchor);
    ok &= expect_contains("source C545", evidence, "C537..C545",
                          spec->defsAnchor);
    return ok;
}

static int test_constants(
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("slot count", DM1_PC34_C545_MIDCAST_SLOT_COUNT, 8,
                     spec->defsAnchor);
    ok &= expect_int("champion count",
                     DM1_PC34_C545_MIDCAST_CHAMPION_COUNT, 4,
                     spec->defsAnchor);
    ok &= expect_int("C30", DM1_PC34_C545_MIDCAST_C30, 30,
                     spec->defsAnchor);
    ok &= expect_int("C37", DM1_PC34_C545_MIDCAST_C37, 37,
                     spec->defsAnchor);
    ok &= expect_int("C537", DM1_PC34_C545_MIDCAST_C537, 537,
                     spec->defsAnchor);
    ok &= expect_int("C540", DM1_PC34_C545_MIDCAST_C540, 540,
                     spec->defsAnchor);
    ok &= expect_int("C541", DM1_PC34_C545_MIDCAST_C541, 541,
                     spec->defsAnchor);
    ok &= expect_int("C544", DM1_PC34_C545_MIDCAST_C544, 544,
                     spec->defsAnchor);
    ok &= expect_int("C545", DM1_PC34_C545_MIDCAST_C545, 545,
                     spec->defsAnchor);
    ok &= expect_int("C070", DM1_PC34_C545_MIDCAST_C070, 70,
                     spec->f0359Anchor);
    ok &= expect_int("M568", DM1_PC34_C545_MIDCAST_M568, 568,
                     spec->f0359Anchor);
    ok &= expect_int("M569", DM1_PC34_C545_MIDCAST_M569, 569,
                     spec->f0333OpenAnchor);
    ok &= expect_int("leader index", DM1_PC34_C545_MIDCAST_LEADER_INDEX, 0,
                     spec->f0284Anchor);
    ok &= expect_int("owner index", DM1_PC34_C545_MIDCAST_OWNER_INDEX, 2,
                     spec->f0284Anchor);
    ok &= expect_int("owner ordinal", DM1_PC34_C545_MIDCAST_OWNER_ORDINAL, 3,
                     spec->defsAnchor);
    return ok;
}

static int test_initial_state(
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34* probe,
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("runtime gate", probe->sourceLockedRuntimeGate, 1,
                     spec->f0302Anchor);
    ok &= expect_int("no game data load", probe->noGameDataLoad, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("party count", probe->partyChampionCount,
                     DM1_PC34_C545_MIDCAST_CHAMPION_COUNT,
                     spec->defsAnchor);
    ok &= expect_int("leader index", probe->leaderIndex,
                     DM1_PC34_C545_MIDCAST_LEADER_INDEX,
                     spec->f0284Anchor);
    ok &= expect_int("leader ordinal", probe->leaderOrdinal,
                     DM1_PC34_C545_MIDCAST_LEADER_ORDINAL,
                     spec->defsAnchor);
    ok &= expect_int("inventory champion index",
                     probe->inventoryChampionIndex,
                     DM1_PC34_C545_MIDCAST_OWNER_INDEX,
                     spec->defsAnchor);
    ok &= expect_int("inventory champion ordinal",
                     probe->inventoryChampionOrdinal,
                     DM1_PC34_C545_MIDCAST_OWNER_ORDINAL,
                     spec->defsAnchor);
    ok &= expect_int("acting champion before",
                     probe->actingChampionOrdinalBefore,
                     DM1_PC34_C545_MIDCAST_LEADER_ORDINAL,
                     spec->f0300Anchor);
    ok &= expect_int("leader mid-cast before", probe->leaderMidCastBefore, 1,
                     spec->f0300Anchor);
    ok &= expect_int("leader rune count before",
                     probe->leaderSpellRuneCountBefore, 2,
                     spec->f0300Anchor);
    ok &= expect_int("open chest before", probe->openChestThingBefore,
                     DM1_PC34_C545_MIDCAST_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_int("leader hand empty before", probe->leaderHandBefore,
                     DM1_PC34_C545_MIDCAST_NONE,
                     spec->f0298Anchor);
    ok &= expect_int("non-leader action hand before",
                     probe->nonLeaderActionHandBefore,
                     DM1_PC34_C545_MIDCAST_SCROLL,
                     spec->f0302Anchor);
    ok &= expect_int("non-leader action icon before",
                     probe->nonLeaderActionHandIconBefore,
                     DM1_PC34_C545_MIDCAST_SCROLL & 0x00FF,
                     spec->defsAnchor);
    ok &= expect_int("leader load before", probe->leaderLoadBefore, 40,
                     spec->f0297Anchor);
    ok &= expect_int("non-leader load before", probe->nonLeaderLoadBefore, 49,
                     spec->f0300Anchor);
    ok &= expect_slots("initial chest", probe->chestBefore,
                       DM1_PC34_C545_MIDCAST_CHEST_BASE,
                       spec->f0333OpenAnchor);
    return ok;
}

static int test_c545_transfer(
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34* probe,
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("C545 event zone", probe->c545EventZone,
                     DM1_PC34_C545_MIDCAST_C545, spec->defsAnchor);
    ok &= expect_int("C545 command", probe->c545Command,
                     DM1_PC34_C545_MIDCAST_C070, spec->f0359Anchor);
    ok &= expect_int("command panel before", probe->commandPanelBefore,
                     DM1_PC34_C545_MIDCAST_M569, spec->f0333OpenAnchor);
    ok &= expect_int("F0302 leader snapshot", probe->f0302LeaderSnapshot,
                     DM1_PC34_C545_MIDCAST_NONE, spec->f0302Anchor);
    ok &= expect_int("F0302 slot snapshot", probe->f0302SlotSnapshot,
                     DM1_PC34_C545_MIDCAST_SCROLL, spec->f0302Anchor);
    ok &= expect_int("F0302 champion index", probe->f0302ChampionIndex,
                     DM1_PC34_C545_MIDCAST_OWNER_INDEX, spec->defsAnchor);
    ok &= expect_int("F0302 PC34 slot", probe->f0302Pc34Slot,
                     DM1_PC34_C545_MIDCAST_C30 + 1, spec->defsAnchor);
    ok &= expect_int("F0302 allowed by slot mask",
                     probe->f0302AllowedBySlotMask, 1, spec->f0302Anchor);
    ok &= expect_int("F0302 empty-empty rejected",
                     probe->f0302EmptyEmptyRejected, 0, spec->f0302Anchor);
    ok &= expect_int("F0300 clear count", probe->f0300ClearCount, 1,
                     spec->f0300Anchor);
    ok &= expect_int("F0297 put count", probe->f0297PutCount, 1,
                     spec->f0297Anchor);
    ok &= expect_int("F0298 remove count", probe->f0298RemoveCount, 0,
                     spec->f0298Anchor);
    ok &= expect_int("F0301 write count", probe->f0301WriteCount, 0,
                     spec->f0301Anchor);
    ok &= expect_int("leader hand after", probe->leaderHandAfter,
                     DM1_PC34_C545_MIDCAST_SCROLL, spec->f0297Anchor);
    ok &= expect_int("leader hand icon after", probe->leaderHandIconAfter,
                     DM1_PC34_C545_MIDCAST_SCROLL & 0x00FF,
                     spec->defsAnchor);
    ok &= expect_int("non-leader action hand after",
                     probe->nonLeaderActionHandAfter,
                     DM1_PC34_C545_MIDCAST_NONE, spec->f0300Anchor);
    ok &= expect_int("scroll closed after F0300",
                     probe->nonLeaderActionHandClosedAfter, 1,
                     spec->f0300Anchor);
    ok &= expect_int("leader load after", probe->leaderLoadAfter, 47,
                     spec->f0297Anchor);
    ok &= expect_int("non-leader load after", probe->nonLeaderLoadAfter, 42,
                     spec->f0300Anchor);
    ok &= expect_int("leader load mask",
                     probe->leaderAttributesAfter &
                         DM1_PC34_C545_MIDCAST_LOAD_MASK,
                     DM1_PC34_C545_MIDCAST_LOAD_MASK, spec->defsAnchor);
    ok &= expect_int("non-leader load mask",
                     probe->nonLeaderAttributesAfter &
                         DM1_PC34_C545_MIDCAST_LOAD_MASK,
                     DM1_PC34_C545_MIDCAST_LOAD_MASK, spec->defsAnchor);
    ok &= expect_int("non-leader action mask",
                     probe->nonLeaderAttributesAfter &
                         DM1_PC34_C545_MIDCAST_ACTION_HAND_MASK,
                     DM1_PC34_C545_MIDCAST_ACTION_HAND_MASK,
                     spec->f0300Anchor);
    ok &= expect_int("resurrect guard blocked after pickup",
                     probe->commandF0359ResurrectBlockedAfterPickup, 1,
                     spec->f0359Anchor);
    ok &= expect_int("screen update balanced", probe->screenUpdateBalanced, 1,
                     spec->f0297Anchor);
    return ok;
}

static int test_mid_cast_and_chest_state(
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34* probe,
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("acting champion preserved",
                     probe->actingChampionOrdinalAfter,
                     DM1_PC34_C545_MIDCAST_LEADER_ORDINAL,
                     spec->f0300Anchor);
    ok &= expect_int("F0388 clear acting skipped",
                     probe->f0388ClearActingCount, 0,
                     spec->f0300Anchor);
    ok &= expect_int("leader mid-cast preserved",
                     probe->leaderMidCastAfter, 1, spec->f0300Anchor);
    ok &= expect_int("leader rune count preserved",
                     probe->leaderSpellRuneCountAfter, 2,
                     spec->f0300Anchor);
    ok &= expect_int("F0344 inactive", probe->f0344FoodBarDrawCount, 0,
                     spec->f0344Anchor);
    ok &= expect_int("F0345 inactive", probe->f0345FoodWaterPanelDrawCount, 0,
                     spec->f0345Anchor);
    ok &= expect_int("F0352 inactive", probe->f0352EyePanelDrawCount, 0,
                     spec->f0352Anchor);
    ok &= expect_int("panel redraw after C545", probe->panelRedrawAfterC545, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("open chest after C545", probe->openChestThingAfter,
                     DM1_PC34_C545_MIDCAST_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_slots("chest after C545", probe->chestAfter,
                       DM1_PC34_C545_MIDCAST_CHEST_BASE,
                       spec->f0333OpenAnchor);
    ok &= expect_int("C540 preserved", probe->c540Preserved, 1,
                     spec->defsAnchor);
    ok &= expect_int("C541 preserved", probe->c541Preserved, 1,
                     spec->defsAnchor);
    ok &= expect_int("close count", probe->closedChestCount,
                     DM1_PC34_C545_MIDCAST_SLOT_COUNT,
                     spec->f0334CloseAnchor);
    ok &= expect_slots("closed chest", probe->closedChest,
                       DM1_PC34_C545_MIDCAST_CHEST_BASE,
                       spec->f0334CloseAnchor);
    ok &= expect_int("closed order preserved", probe->chestOrderPreserved, 1,
                     spec->f0334CloseAnchor);
    return ok;
}

static int test_negative(
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34* probe,
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("negative leader busy rejected",
                     probe->negativeLeaderBusyRejected, 1,
                     spec->f0302Anchor);
    ok &= expect_int("negative leader hand preserved",
                     probe->negativeLeaderHandAfter, 0x7499,
                     spec->f0298Anchor);
    ok &= expect_int("negative non-leader hand preserved",
                     probe->negativeNonLeaderHandAfter,
                     DM1_PC34_C545_MIDCAST_SCROLL,
                     spec->f0302Anchor);
    ok &= expect_int("negative acting champion preserved",
                     probe->negativeActingChampionAfter,
                     DM1_PC34_C545_MIDCAST_LEADER_ORDINAL,
                     spec->f0300Anchor);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34* spec =
        dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_spec_pc34();
    DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34 probe;
    int ok = 1;

    printf("probe=dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_source_evidence_pc34());

    ok &= expect_int("run probe",
                     dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_run_pc34(
                         &probe),
                     1, spec->f0302Anchor);
    ok &= test_source_evidence(spec);
    ok &= test_constants(spec);
    ok &= test_initial_state(&probe, spec);
    ok &= test_c545_transfer(&probe, spec);
    ok &= test_mid_cast_and_chest_state(&probe, spec);
    ok &= test_negative(&probe, spec);
    ok &= expect_u32("deterministic hash", probe.deterministicHash,
                     0x1FDA81ADu,
                     "stable FNV-1a over C545 mid-cast payload");

    printf("assertions=%d failures=%d deterministicHash=0x%08X\n",
           g_assertions, g_failures, probe.deterministicHash);
    if (!ok || g_failures != 0 || g_assertions < 80) {
        printf("FAIL dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_pc34_compat assertions=%d failures=0 deterministicHash=0x%08X\n",
           g_assertions, probe.deterministicHash);
    return 0;
}
