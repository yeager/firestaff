#ifndef FIRESTAFF_DM1_V1_F0904_PALETTE_ANIMATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0904_PALETTE_ANIMATION_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/*
 * ReDMCSB SWSH.C F0904_PaletteAnimation consumes the original packed
 * palette-command stream.  This adapter deliberately owns no palette data:
 * callers provide the original words and receive each source operation.
 */

typedef void (*DM1_V1_F0904PaletteSetColorPc34)(void *context,
                                                 uint8_t color_index,
                                                 uint16_t rgb12);
typedef void (*DM1_V1_F0904PaletteWaitPc34)(void *context,
                                            uint16_t vblank_count);

typedef struct DM1_V1_F0904PaletteAnimationResultPc34 {
    size_t commands_consumed;
    size_t colors_applied;
    size_t waits_applied;
    uint16_t vblanks_waited;
    int terminated;
} DM1_V1_F0904PaletteAnimationResultPc34;

/*
 * Process caller-provided original SWSH command words.  A zero word ends the
 * sequence; words 1..7 request N + 1 VBL waits (the source DBF loop); all
 * other words carry a
 * palette index in bits 12..15 and an RGB12 value in bits 0..11.  Returns 1
 * only when a terminating zero appears within command_count.
 */
int dm1_v1_f0904_palette_animation_pc34(
    const uint16_t *original_palette_commands,
    size_t command_count,
    DM1_V1_F0904PaletteSetColorPc34 set_color,
    DM1_V1_F0904PaletteWaitPc34 wait_vblanks,
    void *context,
    DM1_V1_F0904PaletteAnimationResultPc34 *out_result);

#endif /* FIRESTAFF_DM1_V1_F0904_PALETTE_ANIMATION_PC34_COMPAT_H */
