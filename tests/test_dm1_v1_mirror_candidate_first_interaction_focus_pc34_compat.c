#include "firestaff/dm1/v1/mirror_candidate/first_interaction_focus_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

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
        dm1_v1_mirror_candidate_first_interaction_focus_source_evidence_pc34();
    CHECK(evidence != 0);
    CHECK(strstr(evidence, "MOVESENS.C:1501-1503") != 0);
    CHECK(strstr(evidence, "F0280:124-132") != 0);
    CHECK(strstr(evidence, "F0280:272-276") != 0);
    CHECK(strstr(evidence, "F0280:276-283") != 0);
    CHECK(strstr(evidence, "F0280:353-354") != 0);
    CHECK(strstr(evidence, "F0380:2159-2182") != 0);
}

static void test_spec_is_stable(void)
{
    const DM1_V1_MirrorCandidateFirstInteractionFocusSpecPc34 *spec =
        dm1_v1_mirror_candidate_first_interaction_focus_spec_pc34();
    CHECK(spec != 0);
    CHECK(spec->candidatePanelContent == 568);
    CHECK(spec->candidatePanelGraphic == 40);
    CHECK(strstr(spec->movesensC127Anchor, "MOVESENS.C:1501-1503") != 0);
    CHECK(strstr(spec->reviveF0280PublishAnchor, "F0280:272-276") != 0);
    CHECK(strstr(spec->reviveF0280InventoryAnchor, "F0280:353-354") != 0);
    CHECK(strstr(spec->commandF0380FocusAnchor, "F0380:2159-2182") != 0);
    CHECK(strstr(spec->nonOverlap, "zero-party first C127") != 0);
}

static void test_init_zero_party_focus_defaults(void)
{
    DM1_V1_MirrorCandidateFirstInteractionFocusStatePc34 state;
    memset(&state, 0xab, sizeof(state));
    dm1_v1_mirror_candidate_first_interaction_focus_init_pc34(&state);
    CHECK(state.contractOnly == 1);
    CHECK(state.leaderHandEmpty == 1);
    CHECK(state.partyChampionCount == 0);
    CHECK(state.candidateChampionOrdinal == 0);
    CHECK(state.leaderIndex == -1);
    CHECK(state.magicCasterChampionIndex == -1);
    CHECK(state.inventoryChampionOrdinal == 0);
    CHECK(state.panelContent == 0);
    CHECK(state.menusDisabled == 0);
}

static void test_run_first_candidate_owns_focus(void)
{
    DM1_V1_MirrorCandidateFirstInteractionFocusResultPc34 r;
    int ok = dm1_v1_mirror_candidate_first_interaction_focus_run_pc34(&r);
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 21);
    CHECK(r.partyCountBefore == 0);
    CHECK(r.candidateOrdinalBefore == 0);
    CHECK(r.leaderIndexBefore == -1);
    CHECK(r.inventoryOrdinalBefore == 0);
    CHECK(r.partyCountAfter == 1);
    CHECK(r.candidateOrdinalAfter == 1);
    CHECK(r.leaderIndexAfter == 0);
    CHECK(r.magicCasterChampionIndexAfter == 0);
    CHECK(r.inventoryChampionOrdinalAfter == 1);
    CHECK(r.panelContentAfter == 568);
    CHECK(r.panelGraphicAfter == 40);
    CHECK(r.menusDisabledAfter == 1);
    CHECK(r.f0280CallCount == 1);
    CHECK(r.f0355InventoryToggleCount == 1);
    CHECK(r.f0368SetLeaderCount == 1);
    CHECK(r.f0394SetMagicCasterCount == 1);
    CHECK(r.focusOwnedByCandidate == 1);
}

static void test_g0299_blocks_sibling_input_focus(void)
{
    DM1_V1_MirrorCandidateFirstInteractionFocusResultPc34 r;
    int ok;
    memset(&r, 0, sizeof(r));
    ok = dm1_v1_mirror_candidate_first_interaction_focus_run_pc34(&r);
    CHECK(ok == 1);
    CHECK(r.candidateOrdinalAfter == 1);
    CHECK(r.blockedStatusBoxCount == 1);
    CHECK(r.blockedInventoryToggleCount == 1);
    CHECK(r.blockedSpellAreaCount == 1);
    CHECK(r.blockedActionAreaCount == 1);
}

static void test_f0280_publication_guards_reject_unpublishable_state(void)
{
    DM1_V1_MirrorCandidateFirstInteractionFocusStatePc34 state;
    DM1_V1_MirrorCandidateFirstInteractionFocusResultPc34 r;
    int ok;

    dm1_v1_mirror_candidate_first_interaction_focus_init_pc34(&state);
    state.leaderHandEmpty = 0;
    ok = dm1_v1_mirror_candidate_first_interaction_focus_try_pc34(&state,
                                                                  &r);
    CHECK(ok == 0);
    CHECK(r.accepted == 0);
    CHECK(r.partyCountBefore == 0);
    CHECK(r.partyCountAfter == 0);
    CHECK(r.candidateOrdinalAfter == 0);
    CHECK(r.inventoryChampionOrdinalAfter == 0);
    CHECK(r.panelContentAfter == 0);
    CHECK(r.menusDisabledAfter == 0);
    CHECK(r.f0280CallCount == 0);
    CHECK(r.f0355InventoryToggleCount == 0);
    CHECK(r.blockedStatusBoxCount == 0);
    CHECK(r.focusOwnedByCandidate == 0);

    dm1_v1_mirror_candidate_first_interaction_focus_init_pc34(&state);
    state.partyChampionCount = 4;
    ok = dm1_v1_mirror_candidate_first_interaction_focus_try_pc34(&state,
                                                                  &r);
    CHECK(ok == 0);
    CHECK(r.accepted == 0);
    CHECK(r.partyCountBefore == 4);
    CHECK(r.partyCountAfter == 4);
    CHECK(r.candidateOrdinalAfter == 0);
    CHECK(r.inventoryChampionOrdinalAfter == 0);
    CHECK(r.panelContentAfter == 0);
    CHECK(r.menusDisabledAfter == 0);
    CHECK(r.f0280CallCount == 0);
    CHECK(r.f0355InventoryToggleCount == 0);
    CHECK(r.blockedStatusBoxCount == 0);
    CHECK(r.focusOwnedByCandidate == 0);
}

int main(void)
{
    test_source_evidence_is_pinned();
    test_spec_is_stable();
    test_init_zero_party_focus_defaults();
    test_run_first_candidate_owns_focus();
    test_g0299_blocks_sibling_input_focus();
    test_f0280_publication_guards_reject_unpublishable_state();
    printf("dm1_v1_mirror_candidate_first_interaction_focus: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
