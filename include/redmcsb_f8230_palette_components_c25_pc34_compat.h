/* ReDMCSB VIDEODRV.C F8230 single C25 palette component update. */
#ifndef FIRESTAFF_REDMCSB_F8230_PALETTE_COMPONENTS_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8230_PALETTE_COMPONENTS_C25_PC34_COMPAT_H

#include "redmcsb_f8156_f8157_c25_palette_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8230_COMPONENT_MAX_PC34 15U

/*
 * C25 F8230 maps each original four-bit component to RGB6 as `(value << 2)+3`.
 * When curtain equals one, it immediately republishes the real full palette
 * through F8156; otherwise only the logical palette changes.
 */
bool redmcsb_f8230_set_single_color_components_c25_pc34_compat(
    uint8_t full_palette[REDMCSB_F8156_C25_PALETTE_COLORS_PC34]
                        [REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34],
    RedmcsbF8156C25DacPc34Compat *dac, uint8_t curtain_flag,
    int16_t color_index, uint8_t red, uint8_t green, uint8_t blue,
    RedmcsbF8156WaitVerticalBlankPc34Compat wait_vertical_blank,
    void *context);

const char *redmcsb_f8230_palette_components_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
