#include "redmcsb_f8160_creature_palette_c25_pc34_compat.h"

bool redmcsb_f8160_set_creature_replacement_colors_c25_pc34_compat(
    RedmcsbF8160ColorPc34Compat *palette_tables
        [REDMCSB_F8160_CREATURE_REPLACEMENT_COUNT_PC34],
    size_t palette_table_entry_count,
    const RedmcsbF8160ColorPc34Compat creature_palettes
        [REDMCSB_F8160_CREATURE_COLOR_SETS_PC34]
        [REDMCSB_F8160_CREATURE_REPLACEMENT_COUNT_PC34],
    int16_t replaced_color, int16_t replacement_color_set_index)
{
    size_t table_index;

    if (palette_tables == NULL || creature_palettes == NULL ||
        replaced_color < 0 || replacement_color_set_index < 0 ||
        (size_t)replaced_color >= palette_table_entry_count ||
        (size_t)replacement_color_set_index >=
            REDMCSB_F8160_CREATURE_COLOR_SETS_PC34) {
        return false;
    }
    for (table_index = 0U;
         table_index < REDMCSB_F8160_CREATURE_REPLACEMENT_COUNT_PC34;
         ++table_index) {
        if (palette_tables[table_index] == NULL) return false;
    }
    for (table_index = 0U;
         table_index < REDMCSB_F8160_CREATURE_REPLACEMENT_COUNT_PC34;
         ++table_index) {
        RedmcsbF8160ColorPc34Compat *destination = palette_tables[table_index];
        const RedmcsbF8160ColorPc34Compat *source =
            &creature_palettes[(size_t)replacement_color_set_index][table_index];

        /* VIDEODRV.C:3552-3560 deliberately preserves COLOR_DEF.Index. */
        destination[replaced_color].red = source->red;
        destination[replaced_color].green = source->green;
        destination[replaced_color].blue = source->blue;
    }
    return true;
}

const char *redmcsb_f8160_creature_palette_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3543-3563 C25 F8160: for table rows 0..5, "
           "copy G8175_CREAT_PAL[replacement-set][row] RGB to the selected "
           "G8176 palette-table colour while retaining Index.";
}
