#ifndef FIRESTAFF_REDMCSB_F0054_TEXT_INITIALIZE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0054_TEXT_INITIALIZE_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB TEXT.C F0054_TEXT_Initialize, PC 3.4 (F20E).
 *
 * The source positions the message cursor, allocates the message-row and
 * interface-font buffers, loads Graphic 557 into the latter, and clears the
 * four row-expiration times. Buffer contents remain owned by the callbacks.
 */
enum {
    REDMCSB_F0054_MESSAGE_AREA_ROW_COUNT_PC34 = 4,
    REDMCSB_F0054_MESSAGE_CURSOR_COLUMN_PC34 = 0,
    REDMCSB_F0054_MESSAGE_CURSOR_ROW_PC34 =
        REDMCSB_F0054_MESSAGE_AREA_ROW_COUNT_PC34 - 1,
    REDMCSB_F0054_MESSAGE_AREA_LINE_BYTE_COUNT_PC34 = 1120,
    REDMCSB_F0054_INTERFACE_FONT_BYTE_COUNT_PC34 = 3072,
    REDMCSB_F0054_ALLOCATION_PERMANENT_PC34 = 1,
    REDMCSB_F0054_MESSAGE_AREA_MEMORY_REQUEST_PC34 = 0x0008,
    REDMCSB_F0054_INTERFACE_FONT_MEMORY_REQUEST_PC34 = 0x0400,
    REDMCSB_F0054_LOAD_FONT_FLAGS_PC34 = 0xc000,
    REDMCSB_F0054_GRAPHIC_FONT_PC34 = 557
};

typedef void (*redmcsb_f0054_move_cursor_fn)(
    void *context,
    int16_t column,
    int16_t row);

typedef uint8_t *(*redmcsb_f0054_memory_allocate_fn)(
    void *context,
    size_t byte_count,
    int16_t allocation_kind,
    uint16_t memory_request);

typedef void (*redmcsb_f0054_load_graphic_fn)(
    void *context,
    uint16_t load_flags,
    uint16_t graphic_index,
    uint8_t *destination,
    int16_t width,
    int16_t height);

typedef struct {
    void *context;
    redmcsb_f0054_move_cursor_fn move_cursor;
    redmcsb_f0054_memory_allocate_fn allocate;
    redmcsb_f0054_load_graphic_fn load_graphic;
} redmcsb_f0054_text_initialize_callbacks;

typedef struct {
    uint8_t *message_area_new_row_bitmap;
    uint8_t *interface_and_scrolls_font;
    int32_t message_area_row_expiration_time[
        REDMCSB_F0054_MESSAGE_AREA_ROW_COUNT_PC34];
} redmcsb_f0054_text_state;

bool F0054_TEXT_Initialize_PC34(
    redmcsb_f0054_text_state *state,
    const redmcsb_f0054_text_initialize_callbacks *callbacks);

#ifdef __cplusplus
}
#endif

#endif
