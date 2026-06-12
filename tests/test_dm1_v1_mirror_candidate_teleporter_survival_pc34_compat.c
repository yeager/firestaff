#include "firestaff/dm1/v1/mirror_candidate/dm1_v1_mirror_candidate_teleporter_survival_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const DM1_V1_MirrorCandidateTeleporterSurvivalSelfTestResultPc34 *result;
    int ok;

    ok = run_dm1_v1_mirror_candidate_teleporter_survival_self_test();
    result =
        dm1_v1_mirror_candidate_teleporter_survival_last_self_test_result_pc34();
    if (!result || !ok || result->failures != 0 ||
        result->assertions < 40 || result->deterministic_hash == 0u ||
        result->teleporter_activations != 1 ||
        result->candidate_index_persists < 2 ||
        result->panel_redraws < 2 ||
        result->g0426_state_preserved < 2 ||
        result->mutation_rejections < 2 ||
        result->resurrect_reincarnate_selection_persists < 2 ||
        result->champion_chain_preserved < 2 ||
        result->creature_scope_party_transition_rejected != 1 ||
        result->party_audible_teleporter_buzzes != 1) {
        printf("FAIL dm1_v1_mirror_candidate_teleporter_survival_pc34_compat "
               "assertions=%d failures=%d hash=0x%08X "
               "teleporters=%d candidate_persist=%d redraws=%d "
               "g0426=%d rejects=%d creature_scope=%d\n",
               result ? result->assertions : -1,
               result ? result->failures : -1,
               result ? result->deterministic_hash : 0u,
               result ? result->teleporter_activations : -1,
               result ? result->candidate_index_persists : -1,
               result ? result->panel_redraws : -1,
               result ? result->g0426_state_preserved : -1,
               result ? result->mutation_rejections : -1,
               result ? result->creature_scope_party_transition_rejected : -1);
        return 1;
    }

    printf("PASS dm1_v1_mirror_candidate_teleporter_survival_pc34_compat "
           "assertions=%d failures=0 hash=0x%08X "
           "teleporters=%d candidate_persist=%d redraws=%d g0426=%d "
           "rejects=%d audible=%d\n",
           result->assertions,
           result->deterministic_hash,
           result->teleporter_activations,
           result->candidate_index_persists,
           result->panel_redraws,
           result->g0426_state_preserved,
           result->mutation_rejections,
           result->party_audible_teleporter_buzzes);
    return 0;
}
