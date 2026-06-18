#include "firestaff/dm1/v1/mirror_candidate/c040_spell_area_click_while_panel_live_pc34_compat.h"

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
        dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_source_evidence_pc34();
    CHECK(evidence != 0);
    CHECK(strstr(evidence, "F0380:2303-2306") != 0);
    CHECK(strstr(evidence, "F0370:2482-2520") != 0);
    CHECK(strstr(evidence, "F0280:124-132") != 0);
    CHECK(strstr(evidence, "F0282:744-806") != 0);
    CHECK(strstr(evidence, "C100") != 0);
    CHECK(strstr(evidence, "G0514") != 0);
}

static void test_spec_is_stable(void)
{
    const DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveSpecPc34 *spec =
        dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_spec_pc34();
    CHECK(spec != 0);
    CHECK(spec->c040PanelContent == 568);
    CHECK(spec->c040PanelGraphic == 40);
    CHECK(spec->spellAreaClick == 100);
    CHECK(spec->nonOverlap != 0);
    CHECK(strstr(spec->nonOverlap, "spell-area-click-while-c040-live") != 0);
    CHECK(strstr(spec->nonOverlap, "C111 action-area") != 0);
    CHECK(strstr(spec->nonOverlap, "Disjoint from pass785") != 0);
}

static void test_init_clears_observability(void)
{
    DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveStatePc34 state;
    memset(&state, 0xab, sizeof(state));
    dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_init_pc34(
        &state);
    CHECK(state.contractOnly == 1);
    CHECK(state.f0282DispatchCount == 0);
    CHECK(state.f0370CallCount == 0);
    CHECK(state.rejectedWhileLiveCount == 0);
    CHECK(state.rejectedNoCasterCount == 0);
    CHECK(state.partyChampionCount == 2);
    CHECK(state.leaderIndex == 0);
    CHECK(state.magicCasterChampionIndex == 0);
    CHECK(state.leaderHandThing == 0xffff);
    CHECK(state.panelContent == 568);
    CHECK(state.panelGraphic == 40);
    CHECK(state.candidateChampionOrdinal == 3);
}

static void test_run_accepted(void)
{
    DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveResultPc34 r;
    int ok =
        dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_run_pc34(
            &r);
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 16);
    CHECK(r.partyCountBefore == 2);
    CHECK(r.magicCasterBefore == 0);
    CHECK(r.panelContentBefore == 568);
    CHECK(r.candidateOrdinalBefore == 3);
    CHECK(r.f0370CallsWhileLive == 0);
    CHECK(r.rejectedWhileLive == 3);
    CHECK(r.panelContentAfterLiveToggles == 568);
    CHECK(r.candidateOrdinalAfterLiveToggles == 3);
    CHECK(r.f0282Dispatched == 1);
    CHECK(r.f0370CallsAfterClear == 1);
    CHECK(r.panelContentAfterClear == 0);
    CHECK(r.candidateOrdinalAfterClear == 0);
    CHECK(r.magicCasterAfterClear == 0);
    CHECK(r.f0370CallsAfterCasterDrop == 1);
    CHECK(r.rejectedNoCaster == 1);
}

static void test_run_gates_spell_area_while_panel_live(void)
{
    DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveResultPc34 r;
    int ok;
    dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_run_pc34(
        &r);
    ok = (r.f0370CallsWhileLive == 0 &&
          r.rejectedWhileLive == 3 &&
          r.panelContentAfterLiveToggles == 568);
    CHECK(ok == 1);
}

static void test_run_unlocks_after_cancel_with_caster(void)
{
    DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveResultPc34 r;
    int ok;
    dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_run_pc34(
        &r);
    ok = (r.f0282Dispatched &&
          r.f0370CallsAfterClear == 1 &&
          r.candidateOrdinalAfterClear == 0 &&
          r.panelContentAfterClear == 0 &&
          r.magicCasterAfterClear == 0);
    CHECK(ok == 1);
}

static void test_run_drops_after_caster_clear(void)
{
    DM1_V1_MirrorCandidateC040SpellAreaClickWhilePanelLiveResultPc34 r;
    int ok;
    dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live_run_pc34(
        &r);
    ok = (r.f0370CallsAfterCasterDrop == 1 &&
          r.rejectedNoCaster == 1);
    CHECK(ok == 1);
}

int main(void)
{
    test_source_evidence_is_pinned();
    test_spec_is_stable();
    test_init_clears_observability();
    test_run_accepted();
    test_run_gates_spell_area_while_panel_live();
    test_run_unlocks_after_cancel_with_caster();
    test_run_drops_after_caster_clear();
    printf("dm1_v1_mirror_candidate_c040_spell_area_click_while_panel_live: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
