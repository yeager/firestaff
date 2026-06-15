#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_TELEPORTER_SURVIVAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_TELEPORTER_SURVIVAL_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_MirrorCandidateTeleporterSurvivalSelfTestResultPc34 {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int teleporter_activations;
    int candidate_index_persists;
    int panel_redraws;
    int g0426_state_preserved;
    int mutation_rejections;
    int resurrect_reincarnate_selection_persists;
    int champion_chain_preserved;
    int creature_scope_party_transition_rejected;
    int party_audible_teleporter_buzzes;
} DM1_V1_MirrorCandidateTeleporterSurvivalSelfTestResultPc34;

int run_dm1_v1_mirror_candidate_teleporter_survival_self_test(void);

const DM1_V1_MirrorCandidateTeleporterSurvivalSelfTestResultPc34 *
dm1_v1_mirror_candidate_teleporter_survival_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
