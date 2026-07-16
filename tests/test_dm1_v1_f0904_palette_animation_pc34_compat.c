#include <stdio.h>
#include <string.h>

#include "firestaff/dm1/v1/f0904_palette_animation_pc34_compat.h"

typedef struct PaletteTrace {
    uint8_t color_indices[2];
    uint16_t colors[2];
    uint16_t waits[2];
    size_t color_count;
    size_t wait_count;
} PaletteTrace;

static void set_color(void *context, uint8_t color_index, uint16_t rgb12)
{
    PaletteTrace *trace = (PaletteTrace *)context;
    trace->color_indices[trace->color_count] = color_index;
    trace->colors[trace->color_count++] = rgb12;
}

static void wait_vblanks(void *context, uint16_t vblank_count)
{
    PaletteTrace *trace = (PaletteTrace *)context;
    trace->waits[trace->wait_count++] = vblank_count;
}

int main(void)
{
    static const uint16_t original_palette_commands[] = {
        0x1777u, 0x0001u, 0x2770u, 0x0003u, 0x0000u
    };
    static const uint16_t unterminated_palette_commands[] = {0x3abc};
    PaletteTrace trace;
    DM1_V1_F0904PaletteAnimationResultPc34 result;

    memset(&trace, 0, sizeof(trace));
    if (!dm1_v1_f0904_palette_animation_pc34(original_palette_commands,
                                              sizeof(original_palette_commands) /
                                                  sizeof(original_palette_commands[0]),
                                              set_color,
                                              wait_vblanks,
                                              &trace,
                                              &result)) {
        return 1;
    }
    if (result.commands_consumed != 5u || result.colors_applied != 2u ||
        result.waits_applied != 2u || result.vblanks_waited != 6u ||
        !result.terminated || trace.color_count != 2u || trace.wait_count != 2u ||
        trace.color_indices[0] != 1u || trace.colors[0] != 0x0777u ||
        trace.color_indices[1] != 2u || trace.colors[1] != 0x0770u ||
        trace.waits[0] != 2u || trace.waits[1] != 4u) {
        return 1;
    }

    if (dm1_v1_f0904_palette_animation_pc34(unterminated_palette_commands,
                                              sizeof(unterminated_palette_commands) /
                                                  sizeof(unterminated_palette_commands[0]),
                                              NULL,
                                              NULL,
                                              NULL,
                                              &result) ||
        result.commands_consumed != 1u || result.terminated) {
        return 1;
    }
    if (dm1_v1_f0904_palette_animation_pc34(NULL, 0u, NULL, NULL, NULL, &result) ||
        result.commands_consumed != 0u || result.colors_applied != 0u ||
        result.waits_applied != 0u || result.terminated) {
        return 1;
    }

    puts("ok");
    return 0;
}
