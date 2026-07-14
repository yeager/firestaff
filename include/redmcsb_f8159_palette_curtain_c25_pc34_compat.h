/* ReDMCSB VIDEODRV.C F8159_PALETTE_SetCurtain, PC 3.4 C25 route. */
#ifndef FIRESTAFF_REDMCSB_F8159_PALETTE_CURTAIN_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8159_PALETTE_CURTAIN_C25_PC34_COMPAT_H

#include "redmcsb_f8156_f8157_c25_palette_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8159_BLACK_PALETTE_PC34 0U
#define REDMCSB_F8159_NORMAL_PALETTE_PC34 1U

/*
 * C25 F8159 blackens all 32 DAC RGB6 rows after VBlank; normal restores the
 * supplied full palette through F8156. The source stores every state byte,
 * including values other than the two named states.
 */
bool redmcsb_f8159_palette_set_curtain_c25_pc34_compat(
    const uint8_t full_palette[REDMCSB_F8156_C25_PALETTE_COLORS_PC34]
                              [REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34],
    RedmcsbF8156C25DacPc34Compat *dac, uint8_t *curtain_flag,
    uint8_t state, RedmcsbF8156WaitVerticalBlankPc34Compat wait_vertical_blank,
    void *context);

const char *redmcsb_f8159_palette_curtain_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
