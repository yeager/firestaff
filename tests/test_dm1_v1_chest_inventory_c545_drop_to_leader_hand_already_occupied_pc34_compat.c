#include "dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    int ok;
    DM1_V1_ChestInventoryC545DropToLeaderHandAlreadyOccupiedStatsPc34
        stats;

    printf("probe=dm1_v1_chest_inventory_c545_drop_to_leader_hand_already_occupied_pc34_compat\n");
    printf("%s\n",
           DM1_V1_ChestInventoryC545DropToLeaderHandAlreadyOccupied_SourceEvidencePc34());

    ok =
        DM1_V1_ChestInventoryC545DropToLeaderHandAlreadyOccupied_RunSelfTestPc34();
    stats =
        DM1_V1_ChestInventoryC545DropToLeaderHandAlreadyOccupied_LastStatsPc34();

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
