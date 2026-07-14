#include "redmcsb_f0053_text_print_to_logical_screen_pc34_compat.h"

#include <stddef.h>

bool F0053_TEXT_PrintToLogicalScreen_PC34(
    redmcsb_f0053_text_print_fn text_print,
    void *context,
    uint8_t *logical_screen,
    int16_t x,
    int16_t y,
    int16_t text_color,
    int16_t background_color,
    const char *string)
{
    if (text_print == NULL) {
        return false;
    }

    text_print(context, logical_screen,
               REDMCSB_F0053_LOGICAL_SCREEN_BYTE_WIDTH_PC34,
               (uint16_t)x, (uint16_t)y, text_color, background_color, string,
               REDMCSB_F0053_LOGICAL_SCREEN_HEIGHT_PC34);
    return true;
}
