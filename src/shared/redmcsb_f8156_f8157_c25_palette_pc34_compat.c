#include "redmcsb_f8156_f8157_c25_palette_pc34_compat.h"

#include <string.h>

bool redmcsb_f8156_set_palette_c25_pc34_compat(
    const uint8_t full_palette[REDMCSB_F8156_C25_PALETTE_COLORS_PC34]
                              [REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34],
    RedmcsbF8156C25DacPc34Compat *dac,
    RedmcsbF8156WaitVerticalBlankPc34Compat wait_vertical_blank,
    void *context)
{
    if (full_palette == NULL || dac == NULL || wait_vertical_blank == NULL ||
        !wait_vertical_blank(context)) {
        return false;
    }

    /* VIDEODRV.C:3371-3387 selects DAC index zero then writes RGB for 0..31. */
    memcpy(dac->rgb6, full_palette, sizeof(dac->rgb6));
    return true;
}

bool redmcsb_f8157_set_multiple_colors_c25_pc34_compat(
    uint8_t full_palette[REDMCSB_F8156_C25_PALETTE_COLORS_PC34]
                        [REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34],
    const RedmcsbF8157C25PaletteEntryPc34Compat *entries,
    size_t entry_capacity, int curtain_flag,
    RedmcsbF8156C25DacPc34Compat *dac,
    RedmcsbF8156WaitVerticalBlankPc34Compat wait_vertical_blank,
    void *context)
{
    size_t index;
    bool terminated = false;

    if (full_palette == NULL || entries == NULL) {
        return false;
    }
    /* The original walks until negative Index; require that terminator first. */
    for (index = 0U; index < entry_capacity; ++index) {
        if (entries[index].index < 0) {
            terminated = true;
            break;
        }
    }
    if (!terminated) {
        return false;
    }

    for (index = 0U; index < entry_capacity && entries[index].index >= 0; ++index) {
        uint8_t palette_index = (uint8_t)entries[index].index;
        if (palette_index < REDMCSB_F8156_C25_PALETTE_COLORS_PC34) {
            full_palette[palette_index][0] = entries[index].red;
            full_palette[palette_index][1] = entries[index].green;
            full_palette[palette_index][2] = entries[index].blue;
        }
    }

    if (curtain_flag == 1) {
        return redmcsb_f8156_set_palette_c25_pc34_compat(
            (const uint8_t (*)[REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34])full_palette,
            dac, wait_vertical_blank, context);
    }
    return true;
}

const char *redmcsb_f8156_f8157_c25_palette_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3302-3389 F8156 writes RGB bytes for all 32 "
           "C25 DAC entries after F8153; :3395-3453 F8157 applies terminated "
           "G8176 entries with Index < 32 and calls F8156 only if curtain is 1.";
}
