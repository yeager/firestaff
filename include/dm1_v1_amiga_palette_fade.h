#ifndef FIRESTAFF_DM1_V1_AMIGA_PALETTE_FADE_H
#define FIRESTAFF_DM1_V1_AMIGA_PALETTE_FADE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_AMIGA_RGB4_COLOR_COUNT = 16,
    DM1_V1_AMIGA_RGB4_FADE_FRAMES = 8
};

/* Caller-owned RGB4 data. Firestaff accepts this only when it remains bound
 * to a verified original Amiga source.  The producer never supplies colors
 * of its own and therefore cannot turn a missing gameplay palette into a
 * plausible-looking synthetic one. */
typedef struct DM1_V1_AmigaRgb4Palette {
    const uint16_t *colors;
    size_t color_count;
    int original_amiga_palette;
    int source_bytes_verified;
    int no_synthetic_palette;
} DM1_V1_AmigaRgb4Palette;

typedef struct DM1_V1_AmigaPaletteFade {
    uint16_t current[DM1_V1_AMIGA_RGB4_COLOR_COUNT];
    uint16_t target[DM1_V1_AMIGA_RGB4_COLOR_COUNT];
    uint8_t frame_count;
    int accepted;
} DM1_V1_AmigaPaletteFade;

typedef struct DM1_V1_AmigaPaletteFadeReceipt {
    int accepted;
    int source_palette_bound;
    int target_palette_bound;
    int suppress_synthetic_fallback;
    uint8_t frame_index;
    uint16_t rgb4[DM1_V1_AMIGA_RGB4_COLOR_COUNT];
    const char *source_evidence;
} DM1_V1_AmigaPaletteFadeReceipt;

/* Starts the exact eight-iteration producer recovered from the supplied
 * English Amiga v2.0 `dm` program at file offset 0x14306. */
int dm1_v1_amiga_palette_fade_begin(
    DM1_V1_AmigaPaletteFade *state,
    const DM1_V1_AmigaRgb4Palette *source,
    const DM1_V1_AmigaRgb4Palette *target);

/* Emits one original RGB4 producer iteration. Each component advances by
 * two when it is more than one nibble from its target, otherwise by one.
 * Returns zero after the original eight iterations have been emitted. */
int dm1_v1_amiga_palette_fade_step(
    DM1_V1_AmigaPaletteFade *state,
    DM1_V1_AmigaPaletteFadeReceipt *out_receipt);

const char *dm1_v1_amiga_palette_fade_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_AMIGA_PALETTE_FADE_H */
