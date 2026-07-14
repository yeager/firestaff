#ifndef FIRESTAFF_REDMCSB_F0052_TEXT_PRINT_TO_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0052_TEXT_PRINT_TO_VIEWPORT_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB TEXT.C F0052_TEXT_PrintToViewport (PC 3.4).
 *
 * F0052 is only a fixed-argument forwarding wrapper for F0040_TEXT_Print:
 * the viewport bitmap is passed as the destination, its row stride and height
 * are 112 bytes and 136 pixels, and every glyph background uses
 * C12_COLOR_DARKEST_GRAY (12).
 * This adapter deliberately delegates F0040; it does not render text.
 */
enum {
    REDMCSB_F0052_VIEWPORT_BYTE_WIDTH_PC34 = 112,
    REDMCSB_F0052_VIEWPORT_HEIGHT_PC34 = 136,
    REDMCSB_F0052_VIEWPORT_BACKGROUND_COLOR_PC34 = 12
};

typedef void (*redmcsb_f0052_text_print_fn)(
    void *context,
    uint8_t *destination,
    uint16_t byte_width,
    uint16_t x,
    uint16_t y,
    int16_t text_color,
    int16_t background_color,
    const char *string,
    uint16_t height);

bool F0052_TEXT_PrintToViewport_PC34(
    redmcsb_f0052_text_print_fn text_print,
    void *context,
    uint8_t *viewport_bitmap,
    int16_t x,
    int16_t y,
    int16_t text_color,
    const char *string);

#ifdef __cplusplus
}
#endif

#endif
