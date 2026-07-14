#include "redmcsb_f8159_palette_curtain_c25_pc34_compat.h"

#include <string.h>

bool redmcsb_f8159_palette_set_curtain_c25_pc34_compat(
    const uint8_t full_palette[REDMCSB_F8156_C25_PALETTE_COLORS_PC34]
                              [REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34],
    RedmcsbF8156C25DacPc34Compat *dac, uint8_t *curtain_flag,
    uint8_t state, RedmcsbF8156WaitVerticalBlankPc34Compat wait_vertical_blank,
    void *context)
{
    if (full_palette == NULL || dac == NULL || curtain_flag == NULL ||
        wait_vertical_blank == NULL) {
        return false;
    }

    if (state == REDMCSB_F8159_BLACK_PALETTE_PC34) {
        /* VIDEODRV.C:3513-3528: wait, select DAC zero, write 32 RGB triples. */
        if (!wait_vertical_blank(context)) {
            return false;
        }
        memset(dac->rgb6, 0, sizeof(dac->rgb6));
    } else if (state == REDMCSB_F8159_NORMAL_PALETTE_PC34) {
        if (!redmcsb_f8156_set_palette_c25_pc34_compat(
                full_palette, dac, wait_vertical_blank, context)) {
            return false;
        }
    }

    /* VIDEODRV.C:3539 assigns G4094 after either named route or no-op. */
    *curtain_flag = state;
    return true;
}

const char *redmcsb_f8159_palette_curtain_source_evidence_pc34(void)
{
    return "ReDMCSB VIDEODRV.C:3487-3541 C25 F8159: black waits then writes "
           "32 zero RGB triples, normal calls F8156, then G4094 stores state.";
}
