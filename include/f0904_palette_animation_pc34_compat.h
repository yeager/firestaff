#ifndef F0904_PALETTE_ANIMATION_PC34_COMPAT_H
#define F0904_PALETTE_ANIMATION_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB SWSH.C F0904 uses 27 two-word records: a non-negative first word
 * updates one palette entry; -1 applies the current black palette and waits. */
typedef struct {
    int16_t color_index;
    uint16_t value_or_vblank_count;
} F0904PaletteAnimationStepPc34Compat;

typedef void (*F0904PaletteSetColorPc34Compat)(
    void *context, unsigned int color_index, uint16_t value);
typedef void (*F0904PaletteApplyBlackPc34Compat)(void *context);
typedef void (*F0904PaletteWaitVblankPc34Compat)(
    void *context, unsigned int vblank_count);

typedef struct {
    size_t colors_written;
    size_t black_palette_applies;
    size_t vblanks_waited;
} F0904PaletteAnimationReceiptPc34Compat;

int f0904_palette_animation_pc34_compat(
    const F0904PaletteAnimationStepPc34Compat *steps,
    size_t step_count,
    F0904PaletteSetColorPc34Compat set_color,
    F0904PaletteApplyBlackPc34Compat apply_black,
    F0904PaletteWaitVblankPc34Compat wait_vblank,
    void *context,
    F0904PaletteAnimationReceiptPc34Compat *out_receipt);

#endif
