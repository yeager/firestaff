#include "dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    int ok;
    Dm1V1ChestInventoryC545DropToLeaderHandAlreadyOccupiedStatsPc34Compat
        stats;

    printf("probe=dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat\n");
    printf("%s\n",
           dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_source_evidence_pc34_compat());

    ok =
        run_dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat_self_test();
    stats =
        dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_last_stats_pc34_compat();

    if (!ok || stats.failures != 0) {
        printf("FAIL dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat assertions=%d failures=%d deterministic_hash=0x%08x\n",
               stats.assertions,
               stats.failures,
               (unsigned int)stats.deterministic_hash);
        return 1;
    }

    printf("PASS dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat assertions=%d failures=0 deterministic_hash=0x%08x\n",
           stats.assertions,
           (unsigned int)stats.deterministic_hash);
    return 0;
}
