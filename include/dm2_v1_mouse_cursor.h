#ifndef DM2_V1_MOUSE_CURSOR_H
#define DM2_V1_MOUSE_CURSOR_H

#include <stddef.h>
#include <stdint.h>

#define DM2_V1_MOUSE_QUEUE_CAPACITY 11
#define DM2_V1_CURSOR_PATTERN_CAPACITY 1024

typedef struct DM2_V1_MouseEvent {
    uint16_t x;
    uint16_t y;
    uint16_t button;
} DM2_V1_MouseEvent;

typedef struct DM2_V1_CursorPattern {
    uint8_t hot_x;
    uint8_t hot_y;
    uint8_t width;
    uint8_t height;
    uint8_t transparent_color;
    uint8_t pixels[DM2_V1_CURSOR_PATTERN_CAPACITY];
    uint16_t pixel_count;
    int valid;
} DM2_V1_CursorPattern;

typedef struct DM2_V1_MouseCursorState {
    int mouse_visibility;
    int ibmio_hide_depth;
    int queue_locked;
    int queued_count;
    int write_index;
    int right_button_latched;
    int deferred_pending;
    DM2_V1_MouseEvent deferred;
    DM2_V1_MouseEvent events[DM2_V1_MOUSE_QUEUE_CAPACITY];
    DM2_V1_CursorPattern patterns[4];
} DM2_V1_MouseCursorState;

typedef struct DM2_V1_MouseCursorReceipt {
    int handled;
    int source_locked;
    int admitted;
    int deferred;
    int queue_count;
    int copied_pixels;
    int blocked;
    const char* symbol;
    const char* source_path;
} DM2_V1_MouseCursorReceipt;

void dm2_v1_mouse_cursor_state_init(DM2_V1_MouseCursorState* state);
void dm2_v1_mouse_cursor_receipt_clear(DM2_V1_MouseCursorReceipt* receipt);

int dm2_v1_FIRE_HIDE_MOUSE_CURSOR(
    DM2_V1_MouseCursorState* state,
    DM2_V1_MouseCursorReceipt* out_receipt);

int dm2_v1_FIRE_QUEUE_MOUSE_EVENT(
    DM2_V1_MouseCursorState* state,
    uint16_t x,
    uint16_t y,
    uint16_t button,
    DM2_V1_MouseCursorReceipt* out_receipt);

/* 4bpp cursor patterns require the active original 16-entry palette. The
 * function rejects a missing or short palette rather than inventing an index
 * mapping; 8bpp cursor images carry their own physical indices. */
int dm2_v1_IBMIO_SET_CURSOR_PATTERN(
    DM2_V1_MouseCursorState* state,
    int index,
    const uint8_t* src,
    size_t src_size,
    uint8_t hot_x,
    uint8_t hot_y,
    uint16_t src_width,
    uint16_t src_height,
    uint16_t src_bits,
    const uint8_t* local_pal,
    size_t local_pal_size,
    uint16_t colorkey,
    DM2_V1_MouseCursorReceipt* out_receipt);

const char* dm2_v1_mouse_cursor_source_evidence(void);

#endif
