#include "firestaff/dm1/v1/champion_unpoison_pc34_compat.h"

static const char s_source_evidence[] =
    "ReDMCSB CHAMPION.C F0319_CHAMPION_Kill:1651-1652 calls "
    "F0323_CHAMPION_Unpoison after setting CurrentHealth to zero; "
    "F0323 owns the champion poison reset.  Firestaff represents that "
    "bounded state as the accumulated poison dose, without coupling this "
    "callable to M11_GameViewState.";

void F0323_CHAMPION_Unpoison_Compat(
    DM1_V1_ChampionPoisonStatePc34Compat* poisonState)
{
    if (!poisonState) {
        return;
    }

    poisonState->poisonDose = 0;
}

const char* F0323_CHAMPION_Unpoison_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}
