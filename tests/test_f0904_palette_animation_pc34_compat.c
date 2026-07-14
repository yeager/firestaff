#include <stdio.h>
#include <string.h>

#include "f0904_palette_animation_pc34_compat.h"

typedef struct {
    unsigned int color_index;
    uint16_t color_value;
    unsigned int set_count;
    unsigned int apply_count;
    unsigned int waited;
} Trace;

static void set_color(void *context, unsigned int index, uint16_t value)
{
    Trace *trace = context;
    trace->color_index = index;
    trace->color_value = value;
    ++trace->set_count;
}

static void apply_black(void *context) { ++((Trace *)context)->apply_count; }
static void wait_vblank(void *context, unsigned int count)
{
    ((Trace *)context)->waited += count;
}

int main(void)
{
    F0904PaletteAnimationStepPc34Compat steps[27];
    F0904PaletteAnimationReceiptPc34Compat receipt;
    Trace trace;

    memset(steps, 0, sizeof(steps));
    memset(&trace, 0, sizeof(trace));
    steps[0].color_index = 3;
    steps[0].value_or_vblank_count = 0x0abcu;
    steps[1].color_index = -1;
    steps[1].value_or_vblank_count = 4u;
    if (!f0904_palette_animation_pc34_compat(steps, 27u, set_color, apply_black,
                                              wait_vblank, &trace, &receipt) ||
        receipt.colors_written != 26u || receipt.black_palette_applies != 1u ||
        receipt.vblanks_waited != 4u || trace.set_count != 26u ||
        trace.apply_count != 1u || trace.waited != 4u || trace.color_index != 0u ||
        trace.color_value != 0u) {
        return 1;
    }
    steps[4].color_index = 16;
    if (f0904_palette_animation_pc34_compat(steps, 27u, set_color, apply_black,
                                             wait_vblank, &trace, &receipt)) {
        return 1;
    }
    if (f0904_palette_animation_pc34_compat(steps, 26u, set_color, apply_black,
                                             wait_vblank, &trace, &receipt)) {
        return 1;
    }
    puts("PASS f0904_palette_animation_pc34_compat");
    return 0;
}
