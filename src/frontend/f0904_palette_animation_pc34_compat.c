#include "f0904_palette_animation_pc34_compat.h"

#include <string.h>

int f0904_palette_animation_pc34_compat(
    const F0904PaletteAnimationStepPc34Compat *steps,
    size_t step_count,
    F0904PaletteSetColorPc34Compat set_color,
    F0904PaletteApplyBlackPc34Compat apply_black,
    F0904PaletteWaitVblankPc34Compat wait_vblank,
    void *context,
    F0904PaletteAnimationReceiptPc34Compat *out_receipt)
{
    F0904PaletteAnimationReceiptPc34Compat receipt;
    size_t step_index;

    memset(&receipt, 0, sizeof(receipt));
    if (out_receipt) *out_receipt = receipt;
    if (!steps || step_count != 27u) return 0;

    for (step_index = 0u; step_index < step_count; ++step_index) {
        const F0904PaletteAnimationStepPc34Compat *step = &steps[step_index];
        if (step->color_index == -1) {
            if (apply_black) apply_black(context);
            if (wait_vblank) wait_vblank(context, step->value_or_vblank_count);
            ++receipt.black_palette_applies;
            receipt.vblanks_waited += step->value_or_vblank_count;
        } else if (step->color_index >= 0 && step->color_index < 16) {
            if (set_color) {
                set_color(context, (unsigned int)step->color_index,
                          step->value_or_vblank_count);
            }
            ++receipt.colors_written;
        } else {
            return 0;
        }
    }
    if (out_receipt) *out_receipt = receipt;
    return 1;
}
