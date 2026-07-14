/* ReDMCSB VIDEODRV.C F8156/F8157, PC 3.4 C25_VGA palette route. */
#ifndef FIRESTAFF_REDMCSB_F8156_F8157_C25_PALETTE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8156_F8157_C25_PALETTE_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F8156_C25_PALETTE_COLORS_PC34 32U
#define REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34 3U

typedef bool (*RedmcsbF8156WaitVerticalBlankPc34Compat)(void *context);

typedef struct {
    uint8_t rgb6[REDMCSB_F8156_C25_PALETTE_COLORS_PC34]
                [REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34];
} RedmcsbF8156C25DacPc34Compat;

typedef struct {
    int8_t index; /* Negative terminates, matching G8176 COLOR_DEF.Index. */
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RedmcsbF8157C25PaletteEntryPc34Compat;

/* F8156: wait for VBlank, then write all 32 RGB6 entries in index order. */
bool redmcsb_f8156_set_palette_c25_pc34_compat(
    const uint8_t full_palette[REDMCSB_F8156_C25_PALETTE_COLORS_PC34]
                              [REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34],
    RedmcsbF8156C25DacPc34Compat *dac,
    RedmcsbF8156WaitVerticalBlankPc34Compat wait_vertical_blank,
    void *context);

/*
 * F8157: apply terminated G8176 palette-table entries with indices < 32.
 * If curtain_flag is one, immediately issue F8156 using the updated table.
 */
bool redmcsb_f8157_set_multiple_colors_c25_pc34_compat(
    uint8_t full_palette[REDMCSB_F8156_C25_PALETTE_COLORS_PC34]
                        [REDMCSB_F8156_C25_PALETTE_COMPONENTS_PC34],
    const RedmcsbF8157C25PaletteEntryPc34Compat *entries,
    size_t entry_capacity, int curtain_flag,
    RedmcsbF8156C25DacPc34Compat *dac,
    RedmcsbF8156WaitVerticalBlankPc34Compat wait_vertical_blank,
    void *context);

const char *redmcsb_f8156_f8157_c25_palette_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
