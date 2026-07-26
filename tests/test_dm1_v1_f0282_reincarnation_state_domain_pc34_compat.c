/* ReDMCSB REVIVE.C F0282:832-835 applies twelve M002_RANDOM(7) values as
 * direct statistic indexes after halving the candidate's vital state. */
#include "dm1_v1_resurrection_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_valid_reincarnation_state_plan(void)
{
    uint8_t draws[12] = {0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4};
    ReincarnationResult_Compat result =
        F0864_RESURRECTION_ComputeReincarnation_Compat(
    (void)result;
            200, 120, 180, 100, 80, 60, draws);

    assert(result.valid == 1);
    assert(result.newMaxHealth == 100);
    assert(result.newCurrentHealth == 60);
    assert(result.newMaxStamina == 90);
    assert(result.newCurrentStamina == 50);
    assert(result.newMaxMana == 40);
    assert(result.newCurrentMana == 30);
    assert(result.statIncrements[0] == 2);
    assert(result.statIncrements[1] == 2);
    assert(result.statIncrements[2] == 2);
    assert(result.statIncrements[3] == 2);
    assert(result.statIncrements[4] == 2);
    assert(result.statIncrements[5] == 1);
    assert(result.statIncrements[6] == 1);
}

static void test_invalid_draw_has_no_reincarnation_mutation(void)
{
    uint8_t draws[12] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3};
    ReincarnationResult_Compat result =
        F0864_RESURRECTION_ComputeReincarnation_Compat(
    (void)result;
            200, 120, 180, 100, 80, 60, draws);
    int index;

    assert(result.valid == 0);
    assert(result.newMaxHealth == 200);
    assert(result.newCurrentHealth == 120);
    assert(result.newMaxStamina == 180);
    assert(result.newCurrentStamina == 100);
    assert(result.newMaxMana == 80);
    assert(result.newCurrentMana == 60);
    for (index = 0; index < 7; ++index) {
        assert(result.statIncrements[index] == 0);
    }
}

int main(void)
{
    test_valid_reincarnation_state_plan();
    test_invalid_draw_has_no_reincarnation_mutation();
    puts("PASS dm1_v1_f0282_reincarnation_state_domain_pc34_compat");
    return 0;
}
