#include "redmcsb_f0695_set_creature_replacement_colors_pc34_compat.h"

bool redmcsb_f0695_set_creature_replacement_colors_pc34_compat(
    RedmcsbF0695StatePc34Compat *state,
    int16_t replaced_color,
    int16_t replacement_set_index)
{
    size_t palette_index;
    size_t color_index;
    size_t set_index;

    if (state == NULL || state->replacement_sets == NULL ||
        state->palette_cache_index == NULL || replaced_color < 0 ||
        replacement_set_index < 0) {
        return false;
    }

    color_index = (size_t)replaced_color;
    set_index = (size_t)replacement_set_index;
    if (color_index >= state->color_count ||
        set_index >= state->replacement_set_count) {
        return false;
    }

    for (palette_index = 0U;
         palette_index < REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT;
         ++palette_index) {
        if (state->palette_tables[palette_index] == NULL) {
            return false;
        }
    }

    for (palette_index = 0U;
         palette_index < REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT;
         ++palette_index) {
        state->palette_tables[palette_index][color_index] =
            state->replacement_sets[
                set_index * REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT +
                palette_index];
    }
    *state->palette_cache_index = -1;
    return true;
}

const char *redmcsb_f0695_set_creature_replacement_colors_source_evidence_pc34(void)
{
    return "ReDMCSB IMAGE.C:151-155 F0695 dispatches VIDRV_12 then sets "
           "G2123_ = -1. VIDEODRV.C:3543-3564 C25_VGA loops six "
           "G8176_PaletteTable palettes and copies Red/Green/Blue from "
           "G8175_CREAT_PAL[replacement_set][palette].";
}
