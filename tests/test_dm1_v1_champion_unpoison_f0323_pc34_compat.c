#include "firestaff/dm1/v1/champion_unpoison_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_source_named_boundary_clears_poison_dose(void)
{
    DM1_V1_ChampionPoisonStatePc34Compat poisonState = {0x1234u};

    F0323_CHAMPION_Unpoison(&poisonState);

    assert(poisonState.poisonDose == 0u);
}

static void test_compat_boundary_delegates_to_source_named_boundary(void)
{
    DM1_V1_ChampionPoisonStatePc34Compat poisonState = {0xffffu};

    F0323_CHAMPION_Unpoison_Compat(&poisonState);

    assert(poisonState.poisonDose == 0u);
}

static void test_null_state_is_ignored(void)
{
    F0323_CHAMPION_Unpoison(NULL);
    F0323_CHAMPION_Unpoison_Compat(NULL);
}

static void test_source_evidence_names_redmcsb_symbol(void)
{
    const char* evidence =
        F0323_CHAMPION_Unpoison_SourceEvidencePc34Compat();
    (void)evidence;

    assert(evidence != NULL);
    assert(strstr(evidence, "CHAMPION.C") != NULL);
    assert(strstr(evidence, "F0323_CHAMPION_Unpoison") != NULL);
}

int main(void)
{
    test_source_named_boundary_clears_poison_dose();
    test_compat_boundary_delegates_to_source_named_boundary();
    test_null_state_is_ignored();
    test_source_evidence_names_redmcsb_symbol();

    puts("ok: DM1 F0323 champion unpoison callable");
    return 0;
}
