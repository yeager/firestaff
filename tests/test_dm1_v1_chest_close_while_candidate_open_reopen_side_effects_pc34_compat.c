/* ReDMCSB CHEST.C F0333:30-67 and F0334:117-132; CHAMPION.C
 * F0297:243-268, F0298:270-298, F0300:511-515, F0301:606-614,
 * F0302:662-714, and F0284:93-131; PANEL.C F0344:1390-1406,
 * F0345, F0352, and F0354:2307-2344; UTAMSCR.C F0077:147-151
 * and F0078:141-145; OBJECT.C F0033:147-212; BLITMASK.C
 * F0133:30-33; DEFS.H C10, C30..C36, C38, C040, M568/M569,
 * C537..C544, G0299, and G0425/G0426. */
#include "dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int assertions;
    int failures;
} TestCountersPc34Compat;

static void expect_true_pc34_compat(TestCountersPc34Compat* counters,
                                    int condition,
                                    const char* label,
                                    const char* anchor)
{
    ++counters->assertions;
    if (!condition) {
        ++counters->failures;
        printf("FAIL %s anchor=%s\n", label, anchor ? anchor : "(null)");
    }
    assert(condition);
}

static void expect_int_pc34_compat(TestCountersPc34Compat* counters,
                                   const char* label,
                                   int got,
                                   int want,
                                   const char* anchor)
{
    ++counters->assertions;
    if (got != want) {
        ++counters->failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor ? anchor : "(null)");
    }
    assert(got == want);
}

static void expect_u64_pc34_compat(TestCountersPc34Compat* counters,
                                   const char* label,
                                   uint64_t got,
                                   uint64_t want,
                                   const char* anchor)
{
    ++counters->assertions;
    if (got != want) {
        ++counters->failures;
        printf("FAIL %s got=0x%llx want=0x%llx anchor=%s\n",
               label,
               (unsigned long long)got,
               (unsigned long long)want,
               anchor ? anchor : "(null)");
    }
    assert(got == want);
}

static void expect_contains_pc34_compat(TestCountersPc34Compat* counters,
                                        const char* label,
                                        const char* haystack,
                                        const char* needle,
                                        const char* anchor)
{
    int ok = haystack && needle && strstr(haystack, needle);

    ++counters->assertions;
    if (!ok) {
        ++counters->failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
    assert(ok);
}

static void test_evidence_pc34_compat(TestCountersPc34Compat* counters)
{
    const char* evidence =
        dm1_v1_chest_close_while_candidate_open_reopen_side_effects_source_evidence_pc34_compat();
    const DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsSpecPc34Compat*
        spec =
            dm1_v1_chest_close_while_candidate_open_reopen_side_effects_spec_pc34_compat();

    expect_contains_pc34_compat(counters, "evidence F0333", evidence,
                                "CHEST.C F0333:30-67",
                                "ReDMCSB CHEST.C F0333 lines 30-67");
    expect_contains_pc34_compat(counters, "evidence F0334", evidence,
                                "CHEST.C F0334:117-132",
                                "ReDMCSB CHEST.C F0334 lines 117-132");
    expect_contains_pc34_compat(counters, "evidence F0297", evidence,
                                "CHAMPION.C F0297:243-268",
                                "ReDMCSB CHAMPION.C F0297 lines 243-268");
    expect_contains_pc34_compat(counters, "evidence F0298", evidence,
                                "CHAMPION.C F0298:270-298",
                                "ReDMCSB CHAMPION.C F0298 lines 270-298");
    expect_contains_pc34_compat(counters, "evidence F0300", evidence,
                                "CHAMPION.C F0300:511-515",
                                "ReDMCSB CHAMPION.C F0300 lines 511-515");
    expect_contains_pc34_compat(counters, "evidence F0301", evidence,
                                "CHAMPION.C F0301:606-614",
                                "ReDMCSB CHAMPION.C F0301 lines 606-614");
    expect_contains_pc34_compat(counters, "evidence F0302", evidence,
                                "CHAMPION.C F0302:662-714",
                                "ReDMCSB CHAMPION.C F0302 lines 662-714");
    expect_contains_pc34_compat(counters, "evidence F0284", evidence,
                                "CHAMPION.C F0284:93-131",
                                "ReDMCSB CHAMPION.C F0284 lines 93-131");
    expect_contains_pc34_compat(counters, "evidence F0344", evidence,
                                "PANEL.C F0344:1390-1406",
                                "ReDMCSB PANEL.C F0344 lines 1390-1406");
    expect_contains_pc34_compat(counters, "evidence F0352", evidence,
                                "PANEL.C F0352",
                                "ReDMCSB PANEL.C F0352");
    expect_contains_pc34_compat(counters, "evidence F0354", evidence,
                                "PANEL.C F0354:2307-2344",
                                "ReDMCSB PANEL.C F0354 lines 2307-2344");
    expect_contains_pc34_compat(counters, "evidence F0077", evidence,
                                "UTAMSCR.C F0077:147-151",
                                "ReDMCSB UTAMSCR.C F0077 lines 147-151");
    expect_contains_pc34_compat(counters, "evidence F0078", evidence,
                                "F0078:141-145",
                                "ReDMCSB UTAMSCR.C F0078 lines 141-145");
    expect_contains_pc34_compat(counters, "evidence F0033", evidence,
                                "OBJECT.C F0033:147-212",
                                "ReDMCSB OBJECT.C F0033 lines 147-212");
    expect_contains_pc34_compat(counters, "evidence F0133", evidence,
                                "BLITMASK.C F0133:30-33",
                                "ReDMCSB BLITMASK.C F0133 lines 30-33");
    expect_contains_pc34_compat(counters, "evidence DEFS", evidence,
                                "3906-3913 C537..C544",
                                "ReDMCSB DEFS.H lines 3906-3913");
    expect_contains_pc34_compat(counters, "spec marker",
                                spec->contractMarker, "close G0426",
                                "ReDMCSB CHEST.C F0334 lines 117-132");
    expect_int_pc34_compat(counters, "spec seed",
                           (int)spec->deterministicSeed,
                           (int)0xC537C040u,
                           "deterministic seed");
    expect_int_pc34_compat(counters, "spec C537", spec->c537Ordinal,
                           DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C537_FIRST,
                           "ReDMCSB DEFS.H lines 3906-3913");
    expect_int_pc34_compat(counters, "spec C544", spec->c544Ordinal,
                           DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C544_LAST,
                           "ReDMCSB DEFS.H lines 3906-3913");
}

static void test_probe_pc34_compat(TestCountersPc34Compat* counters)
{
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsProbePc34Compat probe;
    int i;
    int ok =
        dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat(
            &probe);

    printf("deterministicHash=0x%llx\n",
           (unsigned long long)probe.deterministicHash);
    expect_int_pc34_compat(counters, "runtime ok", ok, 1,
                           "ReDMCSB CHEST.C F0333/F0334");
    expect_int_pc34_compat(counters, "module failures",
                           probe.failedAssertions, 0,
                           "module contract assertions");
    expect_int_pc34_compat(counters, "module assertion count",
                           probe.totalAssertions,
                           DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_EXPECTED_ASSERTIONS,
                           "module contract assertions");
    expect_int_pc34_compat(counters, "module accounting",
                           probe.passedAssertions + probe.failedAssertions,
                           probe.totalAssertions,
                           "module contract assertions");
    expect_int_pc34_compat(counters, "first close count",
                           probe.firstClosedCount, 3,
                           "ReDMCSB CHEST.C F0334 lines 117-132");
    expect_int_pc34_compat(counters, "second close count",
                           probe.secondClosedCount, 4,
                           "ReDMCSB CHEST.C F0333 lines 34-39");
    expect_int_pc34_compat(counters, "reopened count",
                           probe.reopenedVisibleCount, 3,
                           "ReDMCSB CHEST.C F0333 lines 53-67");
    expect_int_pc34_compat(counters, "first close cleared G0426",
                           probe.firstCloseClearedG0426, 1,
                           "ReDMCSB CHEST.C F0334 lines 113-117");
    expect_int_pc34_compat(counters, "first compact",
                           probe.firstCloseCompactedVisibleChain, 1,
                           "ReDMCSB CHEST.C F0334 lines 117-132");
    expect_int_pc34_compat(counters, "second compact",
                           probe.secondCloseCompactedVisibleChain, 1,
                           "ReDMCSB CHEST.C F0334 lines 117-132");
    expect_int_pc34_compat(counters, "reopen on different champion",
                           probe.reopenedFirstChestOnDifferentChampion, 1,
                           "ReDMCSB CHAMPION.C F0284 lines 93-131");
    expect_int_pc34_compat(counters, "champion zero clear",
                           probe.championZeroVisibleCleared, 1,
                           "ReDMCSB CHEST.C F0334 lines 117-132");
    expect_int_pc34_compat(counters, "no first leak into second",
                           probe.noFirstLeakIntoSecondClosedChain, 1,
                           "ReDMCSB CHEST.C F0333/F0334");
    expect_int_pc34_compat(counters, "no second leak into reopened",
                           probe.noSecondLeakIntoReopenedFirstChain, 1,
                           "ReDMCSB CHEST.C F0333/F0334");
    expect_int_pc34_compat(counters, "C537 chain rebound",
                           probe.c537ToC544ReboundToReopenedChest, 1,
                           "ReDMCSB DEFS.H lines 3906-3913");
    expect_int_pc34_compat(counters, "candidate after close",
                           probe.candidateAfterClose,
                           DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0299_ORDINAL,
                           "ReDMCSB DEFS.H line 5694 G0299");
    expect_int_pc34_compat(counters, "candidate after reopen",
                           probe.candidateAfterReopen,
                           DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0299_ORDINAL,
                           "ReDMCSB DEFS.H line 5694 G0299");
    expect_int_pc34_compat(counters, "C040 redraw count",
                           probe.c040RedrawOnCloseCount, 1,
                           "ReDMCSB PANEL.C F0352");
    expect_int_pc34_compat(counters, "C025 redraw count",
                           probe.c025RedrawOnReopenCount, 1,
                           "ReDMCSB PANEL.C F0354 lines 2307-2344");
    expect_int_pc34_compat(counters, "panel after close",
                           probe.panelAfterClose,
                           DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_M568_CANDIDATE,
                           "ReDMCSB DEFS.H lines 3001-3008");
    expect_int_pc34_compat(counters, "panel after reopen",
                           probe.panelAfterReopen,
                           DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_M569_CHEST,
                           "ReDMCSB DEFS.H lines 3001-3008");
    expect_int_pc34_compat(counters, "first compact slot0",
                           probe.firstClosedTypes[0],
                           probe.firstBeforeTypes[0],
                           "ReDMCSB CHEST.C F0334 lines 123-132");
    expect_int_pc34_compat(counters, "first compact slot1",
                           probe.firstClosedTypes[1],
                           probe.firstBeforeTypes[2],
                           "ReDMCSB CHEST.C F0334 lines 123-132");
    expect_int_pc34_compat(counters, "first compact slot2",
                           probe.firstClosedTypes[2],
                           probe.firstBeforeTypes[4],
                           "ReDMCSB CHEST.C F0334 lines 123-132");
    expect_int_pc34_compat(counters, "second compact slot0",
                           probe.secondClosedTypes[0],
                           probe.secondBeforeTypes[0],
                           "ReDMCSB CHEST.C F0334 lines 123-132");
    expect_int_pc34_compat(counters, "second compact slot1",
                           probe.secondClosedTypes[1],
                           probe.secondBeforeTypes[1],
                           "ReDMCSB CHEST.C F0334 lines 123-132");
    expect_int_pc34_compat(counters, "second compact slot2",
                           probe.secondClosedTypes[2],
                           probe.secondBeforeTypes[3],
                           "ReDMCSB CHEST.C F0334 lines 123-132");
    expect_int_pc34_compat(counters, "second compact slot3",
                           probe.secondClosedTypes[3],
                           probe.secondBeforeTypes[5],
                           "ReDMCSB CHEST.C F0334 lines 123-132");
    expect_int_pc34_compat(counters, "reopened slot0",
                           probe.reopenedTypes[0],
                           probe.firstClosedTypes[0],
                           "ReDMCSB CHEST.C F0333 lines 53-67");
    expect_int_pc34_compat(counters, "reopened slot1",
                           probe.reopenedTypes[1],
                           probe.firstClosedTypes[1],
                           "ReDMCSB CHEST.C F0333 lines 53-67");
    expect_int_pc34_compat(counters, "reopened slot2",
                           probe.reopenedTypes[2],
                           probe.firstClosedTypes[2],
                           "ReDMCSB CHEST.C F0333 lines 53-67");
    for (i = 3;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        expect_int_pc34_compat(counters, "reopened tail empty",
                               probe.reopenedTypes[i], 0,
                               "ReDMCSB CHEST.C F0333 lines 68-76");
    }
    for (i = 0;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        expect_int_pc34_compat(
            counters, "reopened zone ordinal",
            probe.reopenedZoneOrdinals[i],
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C537_FIRST + i,
            "ReDMCSB DEFS.H lines 3906-3913");
    }
    expect_u64_pc34_compat(
        counters, "deterministic FNV-1a hash", probe.deterministicHash,
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_EXPECTED_HASH,
        "pinned deterministic contract hash");
}

int main(void)
{
    TestCountersPc34Compat counters;

    memset(&counters, 0, sizeof(counters));
    printf("probe=dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_close_while_candidate_open_reopen_side_effects_source_evidence_pc34_compat());
    test_evidence_pc34_compat(&counters);
    test_probe_pc34_compat(&counters);
    expect_true_pc34_compat(&counters,
                            counters.assertions >=
                                DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_EXPECTED_ASSERTIONS,
                            "minimum assertion count",
                            "ctest regression coverage");
    printf("assertions=%d failures=%d\n", counters.assertions,
           counters.failures);
    return counters.failures == 0 ? 0 : 1;
}
