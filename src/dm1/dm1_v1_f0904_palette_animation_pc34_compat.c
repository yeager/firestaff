#include "firestaff/dm1/v1/f0904_palette_animation_pc34_compat.h"

#include <string.h>

int
dm1_v1_f0904_palette_animation_pc34(
    const uint16_t *original_palette_commands,
    size_t command_count,
    DM1_V1_F0904PaletteSetColorPc34 set_color,
    DM1_V1_F0904PaletteWaitPc34 wait_vblanks,
    void *context,
    DM1_V1_F0904PaletteAnimationResultPc34 *out_result)
{
    DM1_V1_F0904PaletteAnimationResultPc34 result;
    size_t command_index;

    memset(&result, 0, sizeof(result));
    if (!original_palette_commands || command_count == 0u) {
        if (out_result) *out_result = result;
        return 0;
    }

    for (command_index = 0u; command_index < command_count; ++command_index) {
        uint16_t command = original_palette_commands[command_index];

        result.commands_consumed++;
        if (command == 0u) {
            result.terminated = 1;
            if (out_result) *out_result = result;
            return 1;
        }
        if (command <= 7u) {
            uint16_t vblank_count = (uint16_t)(command + 1u);

            result.waits_applied++;
            result.vblanks_waited = (uint16_t)(result.vblanks_waited + vblank_count);
            if (wait_vblanks) wait_vblanks(context, vblank_count);
            continue;
        }

        result.colors_applied++;
        if (set_color) {
            set_color(context,
                      (uint8_t)(command >> 12),
                      (uint16_t)(command & 0x0fffu));
        }
    }

    if (out_result) *out_result = result;
    return 0;
}
