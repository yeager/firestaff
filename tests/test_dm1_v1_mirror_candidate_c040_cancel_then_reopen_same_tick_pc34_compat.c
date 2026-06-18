#include "firestaff/dm1/v1/mirror_candidate/c040_cancel_then_reopen_same_tick_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check(int cond, const char *expr, const char *file, int line)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s\n", file, line, expr);
    }
}

#define CHECK(c) check((c), #c, __FILE__, __LINE__)

static void test_source_evidence_is_pinned(void)
{
    const char *evidence =
        dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_source_evidence_pc34();
    CHECK(evidence != 0);
    CHECK(strstr(evidence, "F0280:124-132") != 0);
    CHECK(strstr(evidence, "F0282:744-806") != 0);
    CHECK(strstr(evidence, "F0355:2299-2318") != 0);
    CHECK(strstr(evidence, "F0378:1956-1990") != 0);
    CHECK(strstr(evidence, "F0275:1502") != 0);
    CHECK(strstr(evidence, "C127") != 0);
    CHECK(strstr(evidence, "C162") != 0);
}

static void test_spec_is_stable(void)
{
    const DM1_V1_MirrorCandidateC040CancelThenReopenSameTickSpecPc34 *spec =
        dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_spec_pc34();
    CHECK(spec != 0);
    CHECK(spec->c040PanelContent == 568);
    CHECK(spec->c040PanelGraphic == 40);
    CHECK(spec->partyCountBefore == 2);
    CHECK(spec->partyCountMid == 1);
    CHECK(spec->initialCandidateOrdinal == 3);
    CHECK(spec->reopenedCandidateOrdinal == 4);
    CHECK(spec->nonOverlap != 0);
    CHECK(strstr(spec->nonOverlap, "C162") != 0);
    CHECK(strstr(spec->nonOverlap, "same tick") != 0);
}

static void test_init_clears_observability(void)
{
    DM1_V1_MirrorCandidateC040CancelThenReopenSameTickStatePc34 state;
    memset(&state, 0xab, sizeof(state));
    dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_init_pc34(
        &state);
    CHECK(state.contractOnly == 1);
    CHECK(state.f0282DispatchCount == 0);
    CHECK(state.f0280DispatchCount == 0);
    CHECK(state.f0280Rejected == 0);
    CHECK(state.f0355CallCount == 0);
    CHECK(state.partyChampionCount == 2);
    CHECK(state.panelContent == 568);
    CHECK(state.panelGraphic == 40);
    CHECK(state.candidateChampionOrdinal == 3);
    CHECK(state.leaderHandThing == 0xffff);
    CHECK(state.partyMapX == 10);
    CHECK(state.partyMapY == 10);
}

static void test_run_accepted(void)
{
    DM1_V1_MirrorCandidateC040CancelThenReopenSameTickResultPc34 r;
    int ok = dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_run_pc34(
        &r);
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 18);
    CHECK(r.partyCountBefore == 2);
    CHECK(r.partyCountMid == 1);
    CHECK(r.partyCountAfter == 2);
    CHECK(r.candidateOrdinalBefore == 3);
    CHECK(r.candidateOrdinalMid == 0);
    CHECK(r.candidateOrdinalAfter == 4);
    CHECK(r.panelContentBefore == 568);
    CHECK(r.panelContentMid == 0);
    CHECK(r.panelContentAfter == 568);
    CHECK(r.panelGraphicBefore == 40);
    CHECK(r.panelGraphicAfter == 40);
    CHECK(r.leaderHandEmptyBefore == 1);
    CHECK(r.leaderHandEmptyAfter == 1);
    CHECK(r.f0282Dispatched == 1);
    CHECK(r.f0280Dispatched == 1);
    CHECK(r.f0280NotRejected == 1);
    CHECK(r.f0355Called == 1);
    CHECK(r.partyMovedToFreshSensor == 1);
    CHECK(r.sameTick == 1);
}

static void test_run_cancels_then_reopens_in_one_tick(void)
{
    DM1_V1_MirrorCandidateC040CancelThenReopenSameTickResultPc34 r;
    int ok;
    dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick_run_pc34(&r);
    ok = (r.f0282Dispatched && r.f0280Dispatched && r.sameTick &&
          r.panelContentMid == 0 && r.panelContentAfter == 568 &&
          r.candidateOrdinalMid == 0 && r.candidateOrdinalAfter == 4);
    CHECK(ok == 1);
}

int main(void)
{
    test_source_evidence_is_pinned();
    test_spec_is_stable();
    test_init_clears_observability();
    test_run_accepted();
    test_run_cancels_then_reopens_in_one_tick();
    printf("dm1_v1_mirror_candidate_c040_cancel_then_reopen_same_tick: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
