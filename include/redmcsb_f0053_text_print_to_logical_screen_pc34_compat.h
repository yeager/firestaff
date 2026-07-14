#ifndef FIRESTAFF_REDMCSB_F0053_TEXT_PRINT_TO_LOGICAL_SCREEN_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0053_TEXT_PRINT_TO_LOGICAL_SCREEN_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB TEXT.C F0053_TEXT_PrintToLogicalScreen (PC 3.4).
 *
 * F0053 forwards to the text primitive with the logical screen bitmap, its
 * fixed 160-byte stride and fixed 200-pixel height. It does not render text.
 */
enum {
    REDMCSB_F0053_LOGICAL_SCREEN_BYTE_WIDTH_PC34 = 160,
    REDMCSB_F0053_LOGICAL_SCREEN_HEIGHT_PC34 = 200
};

typedef void (*redmcsb_f0053_text_print_fn)(
    void *context,
    uint8_t *destination,
    uint16_t byte_width,
    uint16_t x,
    uint16_t y,
    int16_t text_color,
    int16_t background_color,
    const char *string,
    uint16_t height);

bool F0053_TEXT_PrintToLogicalScreen_PC34(
    redmcsb_f0053_text_print_fn text_print,
    void *context,
    uint8_t *logical_screen,
    int16_t x,
    int16_t y,
    int16_t text_color,
    int16_t background_color,
    const char *string);

#ifdef __cplusplus
}
#endif

#endif
