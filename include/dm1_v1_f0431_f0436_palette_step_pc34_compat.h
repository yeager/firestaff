#ifndef FIRESTAFF_DM1_V1_F0431_F0436_PALETTE_STEP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0431_F0436_PALETTE_STEP_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_PALETTE_ENTRY_COUNT_PC34 = 16,
    DM1_V1_PALETTE_BYTE_COUNT_PC34 = 32
};

/* A palette is admitted only as its original big-endian PC34 0x0RGB words. */
typedef struct DM1_V1_RawPalettePc34 {
    const uint8_t *bytes;
    size_t byte_count;
    int original_pc34_palette;
    int source_bytes_verified;
    int no_synthetic_palette;
} DM1_V1_RawPalettePc34;

typedef struct DM1_V1_F0431DarkenedColorReceiptPc34 {
    int accepted;
    uint16_t source_color;
    uint16_t darkened_color;
    int source_palette_bound;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0431DarkenedColorReceiptPc34;

typedef struct DM1_V1_F0436FadeStepReceiptPc34 {
    int accepted;
    uint8_t next_palette[DM1_V1_PALETTE_BYTE_COUNT_PC34];
    int source_palette_bound;
    int target_palette_bound;
    int vertical_blank_authorized;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0436FadeStepReceiptPc34;

/* DARKCOLR.C F0431: darken one authenticated raw PC34 palette entry. */
int dm1_v1_f0431_get_darkened_color_pc34(
    const DM1_V1_RawPalettePc34 *palette,
    uint8_t color_index,
    DM1_V1_F0431DarkenedColorReceiptPc34 *out_receipt);

/* PALETTE.C F0436 MEDIA108: emit exactly one source-defined fade step.
 * This helper owns neither title nor entrance routing and never invents a
 * palette or an interpolated frame when either raw PC34 input is absent. */
int dm1_v1_f0436_fade_to_palette_step_pc34(
    const DM1_V1_RawPalettePc34 *current_palette,
    const DM1_V1_RawPalettePc34 *target_palette,
    int vertical_blank_authorized,
    DM1_V1_F0436FadeStepReceiptPc34 *out_receipt);

const char *dm1_v1_f0431_f0436_palette_step_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0431_F0436_PALETTE_STEP_PC34_COMPAT_H */
