#include "firestaff/dm1/v1/chest/dm1_v1_chest_teleporter_survival_open_g0426_pc34_compat.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    const DM1_V1_ChestTeleporterSurvivalOpenG0426SelfTestResultPc34* first;
    uint64_t first_hash;

    if (!run_dm1_v1_chest_teleporter_survival_open_g0426_self_test()) {
        first =
            dm1_v1_chest_teleporter_survival_open_g0426_last_self_test_result_pc34();
        printf("FAIL dm1_v1_chest_teleporter_survival_open_g0426_pc34_compat "
               "assertions=%d failures=%d hash=0x%016" PRIx64 "\n",
               first ? first->assertions : 0,
               first ? first->failures : 1,
               first ? first->deterministic_hash : UINT64_C(0));
        return 1;
    }

    first =
        dm1_v1_chest_teleporter_survival_open_g0426_last_self_test_result_pc34();
    if (!first || first->assertions < 40 || first->failures != 0 ||
        !first->g0426_kept_open || !first->leader_hand_preserved ||
        !first->chest_slot_count_preserved ||
        first->teleporter_activations != 1 ||
        first->mutation_rejections != 1 ||
        first->deterministic_hash == UINT64_C(0)) {
        printf("FAIL dm1_v1_chest_teleporter_survival_open_g0426_pc34_compat "
               "assertions=%d failures=%d hash=0x%016" PRIx64 "\n",
               first ? first->assertions : 0,
               first ? first->failures : 1,
               first ? first->deterministic_hash : UINT64_C(0));
        return 1;
    }

    first_hash = first->deterministic_hash;
    if (!run_dm1_v1_chest_teleporter_survival_open_g0426_self_test()) {
        return 1;
    }
    first =
        dm1_v1_chest_teleporter_survival_open_g0426_last_self_test_result_pc34();
    if (!first || first->deterministic_hash != first_hash) {
        printf("FAIL dm1_v1_chest_teleporter_survival_open_g0426_pc34_compat "
               "deterministic hash changed\n");
        return 1;
    }

    printf("dm1_v1_chest_teleporter_survival_open_g0426_pc34_compat: "
           "assertions=%d failures=%d hash=0x%016" PRIx64
           " g0426=%d leader=%d slots=%d activations=%d rejections=%d\n",
           first->assertions, first->failures, first->deterministic_hash,
           first->g0426_kept_open, first->leader_hand_preserved,
           first->chest_slot_count_preserved, first->teleporter_activations,
           first->mutation_rejections);
    return 0;
}
