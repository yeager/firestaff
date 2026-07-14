#ifndef FIRESTAFF_REDMCSB_F0694_SET_MULTIPLE_COLORS_IN_PALETTE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0694_SET_MULTIPLE_COLORS_IN_PALETTE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB: IMAGE.C F0694, routed for PC 3.4 through
 * VIDEODRV.C F8157_VIDRV_08_SetMultipleColorsInPalette,
 * EXETYPE == C25_VGA, lines 3395-3452.
 *
 * A source palette is terminated by an entry whose signed Index is negative.
 * RGB components are the six-bit DAC values held by G8183_aac_FullPalette.
 */
typedef struct {
    int8_t index;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} redmcsb_f0694_palette_entry_pc34_compat;

typedef struct {
    const redmcsb_f0694_palette_entry_pc34_compat *entries;
    size_t entry_count;
} redmcsb_f0694_palette_definition_pc34_compat;

/* F8156_SetPalette equivalent.  The source uploads all 32 DAC entries after
 * an update only while G4094_CURTAIN_FLAG is one. */
typedef void (*redmcsb_f0694_palette_upload_pc34_compat)(
    void *context,
    const uint8_t full_palette[32][3]);

/* Returns 1 after applying the selected source palette and 0 for an invalid
 * table/index or a non-terminated palette. Entries with Index >= 32 are
 * skipped exactly as in the C25 VGA source route. */
int redmcsb_f0694_set_multiple_colors_in_palette_pc34_compat(
    const redmcsb_f0694_palette_definition_pc34_compat *palette_table,
    size_t palette_table_count,
    int16_t palette_index,
    uint8_t full_palette[32][3],
    int curtain_flag,
    redmcsb_f0694_palette_upload_pc34_compat upload,
    void *upload_context);

#ifdef __cplusplus
}
#endif

#endif
