#include "redmcsb_f0703_release_champion_icon_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0703_release_champion_icon_pc34_compat(
    const RedmcsbF0703StatePc34Compat *state)
{
    if (state == NULL || state->champion_icon_ordinal == 0U ||
        state->click_champion_icon == NULL) {
        return false;
    }

    /* ReDMCSB IO.C:3298-3304, MEDIA529 ... I34E_I34M. */
    state->click_champion_icon(state->context,
                               (uint16_t)state->mouse_pointer_champion_index);
    return true;
}

const char *redmcsb_f0703_release_champion_icon_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:3298-3304 F0703_ReleaseChampionIcon, "
           "MEDIA529_F20E_F20J_X30J_P20JA_P20JB_I34E_I34M_A36M_A31E_"
           "A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J: a nonzero G0599 "
           "dispatches F0070_MOUSE_ProcessCommands125To128_"
           "ClickOnChampionIcon with G2164.";
}
