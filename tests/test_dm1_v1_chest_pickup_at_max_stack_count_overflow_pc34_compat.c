/*
 * DM1 V1 chest pickup at max stack-count overflow source-lock regression.
 *
 * Required ReDMCSB anchors:
 * - CHEST.C F0333:30-67 opens/materializes G0425 chest slots; F0334:117-132
 *   closes by rewiring non-empty G0425 slots.
 * - CHAMPION.C F0297:243-268, F0298:270-298, F0300:511-584,
 *   F0301:606-660, and F0302:662-713 define leader hand and C30+ slot
 *   pickup state.
 * - COMMAND.C F0378:1973-1983 and F0380:2045-2156 route panel/queue clicks.
 * - PANEL.C F0354:2307-2344 plus F0346/F0347:1619-1657 redraw/close panels.
 * - UTAMSCR.C F0077:147-151 and F0078:141-145 bracket screen updates.
 * - OBJECT.C F0033:147-212 supplies icon/count identity.
 * - BLITMASK.C F0133:30-33 anchors mask/clip redraw.
 * - DEFS.H C30..C37:810-817, C38:1876-1878, C537..C544:3906-3913.
 * - Local DEFS.H has no C160 stack-count cap: C160 command/zone are
 *   DEFS.H:338 and 3788; the count cap used here is 4-bit ChargeCount at
 *   DEFS.H:1387/1394 and 1421/1428, cap=15.
 */

#include "dm1/dm1_v1_chest_pickup_at_max_stack_count_overflow_pc34_compat.h"

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

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* anchor)
{
    ++g_assertions;
    if (!anchor || !haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

static int test_source_evidence(void)
{
    const char* evidence =
        dm1_v1_chest_pickup_at_max_stack_count_overflow_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence CHEST F0333", evidence,
                          "CHEST.C F0333:30-67",
                          "ReDMCSB CHEST.C F0333:30-67");
    ok &= expect_contains("evidence CHEST F0334", evidence,
                          "CHEST.C F0334:117-132",
                          "ReDMCSB CHEST.C F0334:117-132");
    ok &= expect_contains("evidence CHAMPION F0297", evidence,
                          "CHAMPION.C F0297:243-268",
                          "ReDMCSB CHAMPION.C F0297:243-268");
    ok &= expect_contains("evidence CHAMPION F0298", evidence,
                          "F0298:270-298",
                          "ReDMCSB CHAMPION.C F0298:270-298");
    ok &= expect_contains("evidence CHAMPION F0300", evidence,
                          "F0300:511-584",
                          "ReDMCSB CHAMPION.C F0300:511-584");
    ok &= expect_contains("evidence CHAMPION F0301", evidence,
                          "F0301:606-660",
                          "ReDMCSB CHAMPION.C F0301:606-660");
    ok &= expect_contains("evidence CHAMPION F0302", evidence,
                          "F0302:662-713",
                          "ReDMCSB CHAMPION.C F0302:662-713");
    ok &= expect_contains("evidence COMMAND F0378", evidence,
                          "COMMAND.C F0378:1973-1983",
                          "ReDMCSB COMMAND.C F0378:1973-1983");
    ok &= expect_contains("evidence COMMAND F0380", evidence,
                          "F0380:2045-2156",
                          "ReDMCSB COMMAND.C F0380:2045-2156");
    ok &= expect_contains("evidence PANEL F0354", evidence,
                          "PANEL.C F0354:2307-2344",
                          "ReDMCSB PANEL.C F0354:2307-2344");
    ok &= expect_contains("evidence PANEL F0346/F0347", evidence,
                          "F0346/F0347:1619-1657",
                          "ReDMCSB PANEL.C F0346/F0347:1619-1657");
    ok &= expect_contains("evidence UTAMSCR F0077", evidence,
                          "UTAMSCR.C F0077:147-151",
                          "ReDMCSB UTAMSCR.C F0077:147-151");
    ok &= expect_contains("evidence UTAMSCR F0078", evidence,
                          "F0078:141-145",
                          "ReDMCSB UTAMSCR.C F0078:141-145");
    ok &= expect_contains("evidence OBJECT F0033", evidence,
                          "OBJECT.C F0033:147-212",
                          "ReDMCSB OBJECT.C F0033:147-212");
    ok &= expect_contains("evidence BLITMASK F0133", evidence,
                          "BLITMASK.C F0133:30-33",
                          "ReDMCSB BLITMASK.C F0133:30-33");
    ok &= expect_contains("evidence DEFS C30", evidence,
                          "C30..C37:810-817",
                          "ReDMCSB DEFS.H:810-817");
    ok &= expect_contains("evidence DEFS C38", evidence, "C38:1876-1878",
                          "ReDMCSB DEFS.H:1876-1878");
    ok &= expect_contains("evidence DEFS C537", evidence,
                          "C537..C544:3906-3913",
                          "ReDMCSB DEFS.H:3906-3913");
    ok &= expect_contains("evidence no C160 cap", evidence,
                          "no C160 stack-count cap exists",
                          "ReDMCSB DEFS.H:338/3788");
    ok &= expect_contains("evidence C160 command", evidence,
                          "C160 command is 338",
                          "ReDMCSB DEFS.H:338");
    ok &= expect_contains("evidence ChargeCount cap", evidence,
                          "DEFS.H:1387/1394 and 1421/1428",
                          "ReDMCSB DEFS.H:1387/1394/1421/1428");
    ok &= expect_contains("evidence cap value", evidence, "cap=15",
                          "ReDMCSB DEFS.H ChargeCount 4-bit cap");
    ok &= expect_contains("evidence policy", evidence,
                          "cap-1 + 1 merges",
                          "ReDMCSB CHAMPION.C F0302:662-713");
    ok &= expect_contains("evidence overflow policy", evidence,
                          "cap + 1 stays at cap",
                          "ReDMCSB CHAMPION.C F0301:606-660");
    ok &= expect_contains("evidence no allocation", evidence,
                          "no new C537..C544 slot",
                          "ReDMCSB CHEST.C F0334:117-132");
    return ok;
}

static int test_probe_setup(
    const DM1_V1_ChestPickupAtMaxStackCountOverflowProbePc34* probe)
{
    int ok = 1;

    ok &= expect_int("setup result", probe->setupResult, 1,
                     "ReDMCSB CHEST.C F0333:30-67");
    ok &= expect_int("open result", probe->openResult, 1,
                     "ReDMCSB CHEST.C F0333:30-67");
    ok &= expect_int("open chest thing", probe->openChestThing,
                     DM1_PC34_CHEST_MAX_STACK_OVERFLOW_CHEST_THING,
                     "ReDMCSB CHEST.C F0333:30-67");
    ok &= expect_int("leader index", probe->leaderIndex,
                     DM1_PC34_CHEST_MAX_STACK_OVERFLOW_LEADER,
                     "ReDMCSB CHAMPION.C F0302:662-713");
    ok &= expect_int("party champion count", probe->partyChampionCount, 1,
                     "ReDMCSB COMMAND.C F0380:2045-2156");
    ok &= expect_int("stack cap", probe->stackCap,
                     DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP,
                     "ReDMCSB DEFS.H:1387/1394");
    ok &= expect_int("stack cap bits", probe->stackCapFromDefsBits, 4,
                     "ReDMCSB DEFS.H:1387/1394");
    ok &= expect_int("C160 not stack cap", probe->c160IsStackCap, 0,
                     "ReDMCSB DEFS.H:338/3788");
    ok &= expect_int("C160 command line", probe->c160CommandLine, 338,
                     "ReDMCSB DEFS.H:338");
    ok &= expect_int("C160 zone line", probe->c160ZoneLine, 3788,
                     "ReDMCSB DEFS.H:3788");
    ok &= expect_int("weapon ChargeCount line", probe->chargeCountWeaponLine,
                     1387, "ReDMCSB DEFS.H:1387");
    ok &= expect_int("armour ChargeCount line", probe->chargeCountArmourLine,
                     1421, "ReDMCSB DEFS.H:1421");
    ok &= expect_int("initial panel content", probe->initialPanelContent,
                     DM1_PC34_CHEST_MAX_STACK_OVERFLOW_PANEL_CHEST,
                     "ReDMCSB PANEL.C F0346/F0347:1619-1657");
    ok &= expect_int("initial redraw generation",
                     probe->initialPanelRedrawGeneration, 0,
                     "ReDMCSB PANEL.C F0354:2307-2344");
    ok &= expect_int("initial party direction", probe->initialPartyDirection,
                     3, "ReDMCSB COMMAND.C F0380:2045-2156");
    ok &= expect_int("initial rotate ticks", probe->initialRotateTicks, 2,
                     "ReDMCSB COMMAND.C F0380:2045-2156");
    ok &= expect_int("initial command queue locked",
                     probe->initialCommandQueueLocked, 1,
                     "ReDMCSB COMMAND.C F0380:2045-2156");
    ok &= expect_int("source pc34 slot", probe->sourcePc34Slot,
                     DM1_PC34_SLOT_CHEST_1, "ReDMCSB DEFS.H:810");
    ok &= expect_int("source zone", probe->sourceZone,
                     DM1_PC34_CHEST_MAX_STACK_OVERFLOW_C537_ZONE,
                     "ReDMCSB DEFS.H:3906");
    ok &= expect_int("free pc34 slot", probe->freePc34Slot,
                     DM1_PC34_SLOT_CHEST_2, "ReDMCSB DEFS.H:811");
    return ok;
}

static int test_event(
    const DM1_V1_ChestPickupAtMaxStackCountOverflowEventPc34* event,
    int handBefore,
    int chestBefore,
    int handAfter,
    int chestAfter,
    int overflowAttempted,
    const char* prefix)
{
    char label[128];
    int ok = 1;

#define EXPECT_FIELD(field, want, anchor)                                      \
    do {                                                                       \
        snprintf(label, sizeof(label), "%s.%s", prefix, #field);             \
        ok &= expect_int(label, event->field, (want), (anchor));               \
    } while (0)

    EXPECT_FIELD(result, 1, "ReDMCSB CHAMPION.C F0302:662-713");
    EXPECT_FIELD(sourceSlotIndex, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SOURCE_SLOT,
                 "ReDMCSB DEFS.H:810");
    EXPECT_FIELD(sourceZone, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_C537_ZONE,
                 "ReDMCSB DEFS.H:3906");
    EXPECT_FIELD(leaderHandTypeBefore,
                 DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM,
                 "ReDMCSB CHAMPION.C F0297:243-268");
    EXPECT_FIELD(leaderHandCountBefore, handBefore,
                 "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(chestTypeBefore, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM,
                 "ReDMCSB CHEST.C F0333:30-67");
    EXPECT_FIELD(chestCountBefore, chestBefore,
                 "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(freeSlotTypeBefore, 0, "ReDMCSB CHEST.C F0333:53-67");
    EXPECT_FIELD(freeSlotCountBefore, 0, "ReDMCSB CHEST.C F0333:53-67");
    EXPECT_FIELD(candidateMergedCount, handBefore + chestBefore,
                 "ReDMCSB CHAMPION.C F0302:662-713");
    EXPECT_FIELD(stackCap, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP,
                 "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(saturatedCount, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP,
                 "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(rolloverCountIfUnguarded,
                 (handBefore + chestBefore) & 0x0F,
                 "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(leaderHandTypeAfter,
                 DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM,
                 "ReDMCSB CHAMPION.C F0297:243-268");
    EXPECT_FIELD(leaderHandCountAfter, handAfter,
                 "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(chestTypeAfter,
                 chestAfter > 0 ?
                     DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM : 0,
                 "ReDMCSB CHEST.C F0334:117-132");
    EXPECT_FIELD(chestCountAfter, chestAfter,
                 "ReDMCSB CHEST.C F0334:117-132");
    EXPECT_FIELD(freeSlotTypeAfter, 0, "ReDMCSB CHEST.C F0334:117-132");
    EXPECT_FIELD(freeSlotCountAfter, 0, "ReDMCSB CHEST.C F0334:117-132");
    EXPECT_FIELD(mergedIntoExistingHand, 1,
                 "ReDMCSB CHAMPION.C F0301:606-660");
    EXPECT_FIELD(createdNewChestSlot, 0,
                 "ReDMCSB CHAMPION.C F0301:606-660");
    EXPECT_FIELD(overflowRemainderPreserved, chestAfter > 0 ? 1 : 0,
                 "ReDMCSB CHEST.C F0334:117-132");
    EXPECT_FIELD(overflowPrevented, 1, "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(negativeCountPrevented, 1, "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(crashGuardOk, 1, "ReDMCSB COMMAND.C F0380:2045-2156");
    EXPECT_FIELD(totalCountBefore, handBefore + chestBefore,
                 "ReDMCSB OBJECT.C F0033:147-212");
    EXPECT_FIELD(totalCountAfter, handAfter + chestAfter,
                 "ReDMCSB OBJECT.C F0033:147-212");
    EXPECT_FIELD(totalCountPreserved, 1, "ReDMCSB OBJECT.C F0033:147-212");
    EXPECT_FIELD(panelRedrawRequested, 1,
                 "ReDMCSB PANEL.C F0354:2307-2344");
    EXPECT_FIELD(panelRedrawCountAfter, event->panelRedrawCountBefore + 1,
                 "ReDMCSB PANEL.C F0354:2307-2344");
    EXPECT_FIELD(partyDirectionAfter, event->partyDirectionBefore,
                 "ReDMCSB COMMAND.C F0380:2045-2156");
    EXPECT_FIELD(rotateTicksAfter, event->rotateTicksBefore,
                 "ReDMCSB COMMAND.C F0380:2045-2156");
    EXPECT_FIELD(partyRotateStatePreserved, 1,
                 "ReDMCSB COMMAND.C F0380:2045-2156");
    EXPECT_FIELD(commandQueueLockedAfter, event->commandQueueLockedBefore,
                 "ReDMCSB COMMAND.C F0380:2045-2156");
    EXPECT_FIELD(handOverflowAttempted, overflowAttempted,
                 "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(handOverflowPrevented, overflowAttempted,
                 "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(panel.redrawGeneration, event->panelRedrawCountAfter,
                 "ReDMCSB PANEL.C F0354:2307-2344");
    EXPECT_FIELD(panel.panelContent,
                 DM1_PC34_CHEST_MAX_STACK_OVERFLOW_PANEL_CHEST,
                 "ReDMCSB PANEL.C F0346/F0347:1619-1657");
    EXPECT_FIELD(panel.sourceZone, DM1_PC34_CHEST_MAX_STACK_OVERFLOW_C537_ZONE,
                 "ReDMCSB DEFS.H:3906");
    EXPECT_FIELD(panel.sourceSlotIndex,
                 DM1_PC34_CHEST_MAX_STACK_OVERFLOW_SOURCE_SLOT,
                 "ReDMCSB DEFS.H:810");
    EXPECT_FIELD(panel.handItemType,
                 DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM,
                 "ReDMCSB OBJECT.C F0033:147-212");
    EXPECT_FIELD(panel.handStackCount, handAfter,
                 "ReDMCSB DEFS.H:1387/1394");
    EXPECT_FIELD(panel.chestSlot0ItemType,
                 chestAfter > 0 ?
                     DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_ITEM : 0,
                 "ReDMCSB CHEST.C F0334:117-132");
    EXPECT_FIELD(panel.chestSlot0StackCount, chestAfter,
                 "ReDMCSB CHEST.C F0334:117-132");
    EXPECT_FIELD(panel.chestSlot1ItemType, 0,
                 "ReDMCSB CHAMPION.C F0301:606-660");
    EXPECT_FIELD(panel.chestSlot1StackCount, 0,
                 "ReDMCSB CHAMPION.C F0301:606-660");
    EXPECT_FIELD(panel.displayedSaturatedCount, 1,
                 "ReDMCSB PANEL.C F0354:2307-2344");
    EXPECT_FIELD(panel.displayedNegativeCount, 0,
                 "ReDMCSB PANEL.C F0354:2307-2344");
    EXPECT_FIELD(panel.displayedOverflowCount, 0,
                 "ReDMCSB PANEL.C F0354:2307-2344");
    EXPECT_FIELD(panel.maskClipApplied, 1,
                 "ReDMCSB BLITMASK.C F0133:30-33");
    EXPECT_FIELD(panel.screenUpdateBalanced, 1,
                 "ReDMCSB UTAMSCR.C F0077:147-151/F0078:141-145");

#undef EXPECT_FIELD

    return ok;
}

int main(void)
{
    DM1_V1_ChestPickupAtMaxStackCountOverflowProbePc34 probe;
    int ok = 1;

    printf("probe=dm1_v1_chest_pickup_at_max_stack_count_overflow_pc34_compat\n");
    ok &= expect_int(
        "run probe",
        dm1_v1_chest_pickup_at_max_stack_count_overflow_run_pc34(&probe),
        1, "ReDMCSB CHEST.C F0333:30-67");
    if (ok) {
        ok &= test_source_evidence();
        ok &= test_probe_setup(&probe);
        ok &= test_event(&probe.capMinusOnePlusOne, 1,
                         DM1_PC34_CHEST_MAX_STACK_OVERFLOW_CAP_MINUS_ONE,
                         DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP, 0, 0,
                         "cap_minus_one_plus_one");
        ok &= test_event(&probe.alreadyCapPlusOne,
                         DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP, 1,
                         DM1_PC34_CHEST_MAX_STACK_OVERFLOW_STACK_CAP, 1, 1,
                         "already_cap_plus_one");
        ok &= expect_int("deterministic hash", (int)probe.deterministicHash,
                         (int)0x465F9261u,
                         "stable FNV-1a over source-locked probe fields");
        ok &= expect_int("assertion count lower bound",
                         g_assertions >= 90 ? 1 : 0, 1,
                         "regression requires at least 90 assertions");
    }

    printf("assertionCount=%d failureCount=%d deterministicHash=0x%08X\n",
           g_assertions, g_failures, probe.deterministicHash);
    printf("chestPickupAtMaxStackCountOverflowOk=%d\n",
           ok && g_failures == 0 ? 1 : 0);
    if (ok && g_failures == 0) {
        printf("PASS dm1_v1_chest_pickup_at_max_stack_count_overflow_pc34_compat assertions=%d failures=%d deterministicHash=0x%08X\n",
               g_assertions, g_failures, probe.deterministicHash);
        return 0;
    }
    return 1;
}
