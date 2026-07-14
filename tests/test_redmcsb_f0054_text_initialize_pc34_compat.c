#include "redmcsb_f0054_text_initialize_pc34_compat.h"

#include <stdio.h>

enum event_kind {
    EVENT_MOVE_CURSOR,
    EVENT_ALLOCATE_MESSAGE_AREA,
    EVENT_ALLOCATE_FONT,
    EVENT_LOAD_FONT
};

struct capture {
    enum event_kind events[4];
    int event_count;
    int16_t cursor_column;
    int16_t cursor_row;
    size_t allocation_bytes[2];
    int16_t allocation_kinds[2];
    uint16_t allocation_requests[2];
    uint16_t load_flags;
    uint16_t graphic_index;
    uint8_t *load_destination;
    int16_t load_width;
    int16_t load_height;
    uint8_t message_area[1];
    uint8_t font[1];
};

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

static void capture_move_cursor(void *context, int16_t column, int16_t row)
{
    struct capture *capture = context;

    capture->events[capture->event_count++] = EVENT_MOVE_CURSOR;
    capture->cursor_column = column;
    capture->cursor_row = row;
}

static uint8_t *capture_allocate(void *context, size_t byte_count,
                                 int16_t allocation_kind,
                                 uint16_t memory_request)
{
    struct capture *capture = context;
    int allocation_index = capture->event_count - 1;

    capture->events[capture->event_count++] =
        allocation_index == 0 ? EVENT_ALLOCATE_MESSAGE_AREA : EVENT_ALLOCATE_FONT;
    capture->allocation_bytes[allocation_index] = byte_count;
    capture->allocation_kinds[allocation_index] = allocation_kind;
    capture->allocation_requests[allocation_index] = memory_request;
    return allocation_index == 0 ? capture->message_area : capture->font;
}

static void capture_load_graphic(void *context, uint16_t load_flags,
                                 uint16_t graphic_index, uint8_t *destination,
                                 int16_t width, int16_t height)
{
    struct capture *capture = context;

    capture->events[capture->event_count++] = EVENT_LOAD_FONT;
    capture->load_flags = load_flags;
    capture->graphic_index = graphic_index;
    capture->load_destination = destination;
    capture->load_width = width;
    capture->load_height = height;
}

int main(void)
{
    struct capture capture = { 0 };
    redmcsb_f0054_text_state state = { 0, 0, { 1, 2, 3, 4 } };
    redmcsb_f0054_text_initialize_callbacks callbacks = {
        &capture,
        capture_move_cursor,
        capture_allocate,
        capture_load_graphic
    };
    int ok = 1;
    int row_index;

    ok &= check(F0054_TEXT_Initialize_PC34(&state, &callbacks),
                "accepts the source-proven initialization callbacks");
    ok &= check(capture.event_count == 4 &&
                    capture.events[0] == EVENT_MOVE_CURSOR &&
                    capture.events[1] == EVENT_ALLOCATE_MESSAGE_AREA &&
                    capture.events[2] == EVENT_ALLOCATE_FONT &&
                    capture.events[3] == EVENT_LOAD_FONT,
                "preserves TEXT.C F0054 cursor, allocation, and load order");
    ok &= check(capture.cursor_column ==
                    REDMCSB_F0054_MESSAGE_CURSOR_COLUMN_PC34 &&
                    capture.cursor_row == REDMCSB_F0054_MESSAGE_CURSOR_ROW_PC34,
                "moves the message cursor to the first column of the bottom row");
    ok &= check(capture.allocation_bytes[0] ==
                    REDMCSB_F0054_MESSAGE_AREA_LINE_BYTE_COUNT_PC34 &&
                    capture.allocation_kinds[0] ==
                        REDMCSB_F0054_ALLOCATION_PERMANENT_PC34 &&
                    capture.allocation_requests[0] ==
                        REDMCSB_F0054_MESSAGE_AREA_MEMORY_REQUEST_PC34 &&
                    state.message_area_new_row_bitmap == capture.message_area,
                "stores the source-defined message-row allocation");
    ok &= check(capture.allocation_bytes[1] ==
                    REDMCSB_F0054_INTERFACE_FONT_BYTE_COUNT_PC34 &&
                    capture.allocation_kinds[1] ==
                        REDMCSB_F0054_ALLOCATION_PERMANENT_PC34 &&
                    capture.allocation_requests[1] ==
                        REDMCSB_F0054_INTERFACE_FONT_MEMORY_REQUEST_PC34 &&
                    state.interface_and_scrolls_font == capture.font,
                "stores the source-defined interface-font allocation");
    ok &= check(capture.load_flags == REDMCSB_F0054_LOAD_FONT_FLAGS_PC34 &&
                    capture.graphic_index == REDMCSB_F0054_GRAPHIC_FONT_PC34 &&
                    capture.load_destination == capture.font &&
                    capture.load_width == 0 && capture.load_height == 0,
                "forwards the PC 3.4 font load without inventing font data");
    for (row_index = 0;
         row_index < REDMCSB_F0054_MESSAGE_AREA_ROW_COUNT_PC34;
         row_index++) {
        ok &= check(state.message_area_row_expiration_time[row_index] == -1,
                    "clears each message-row expiration time");
    }

    callbacks.load_graphic = NULL;
    ok &= check(!F0054_TEXT_Initialize_PC34(&state, &callbacks),
                "rejects a missing required callback");
    ok &= check(capture.event_count == 4,
                "a rejected adapter call has no source-side effects");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0054_text_initialize_pc34_compat");
    return 0;
}
