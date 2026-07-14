#include "redmcsb_f0052_text_print_to_viewport_pc34_compat.h"

#include <stddef.h>

bool F0052_TEXT_PrintToViewport_PC34(
    redmcsb_f0052_text_print_fn text_print,
    void *context,
    uint8_t *viewport_bitmap,
    int16_t x,
    int16_t y,
    int16_t text_color,
    const char *string)
{
    if (text_print == NULL) {
        return false;
    }

    text_print(context, viewport_bitmap,
               REDMCSB_F0052_VIEWPORT_BYTE_WIDTH_PC34,
               (uint16_t)x, (uint16_t)y, text_color,
               REDMCSB_F0052_VIEWPORT_BACKGROUND_COLOR_PC34, string);
    return true;
}
