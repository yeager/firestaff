#include "redmcsb_f0054_text_initialize_pc34_compat.h"

bool F0054_TEXT_Initialize_PC34(
    redmcsb_f0054_text_state *state,
    const redmcsb_f0054_text_initialize_callbacks *callbacks)
{
    int row_index;

    if (state == NULL || callbacks == NULL || callbacks->move_cursor == NULL ||
        callbacks->allocate == NULL || callbacks->load_graphic == NULL) {
        return false;
    }

    callbacks->move_cursor(callbacks->context,
                           REDMCSB_F0054_MESSAGE_CURSOR_COLUMN_PC34,
                           REDMCSB_F0054_MESSAGE_CURSOR_ROW_PC34);
    state->message_area_new_row_bitmap = callbacks->allocate(
        callbacks->context, REDMCSB_F0054_MESSAGE_AREA_LINE_BYTE_COUNT_PC34,
        REDMCSB_F0054_ALLOCATION_PERMANENT_PC34,
        REDMCSB_F0054_MESSAGE_AREA_MEMORY_REQUEST_PC34);
    state->interface_and_scrolls_font = callbacks->allocate(
        callbacks->context, REDMCSB_F0054_INTERFACE_FONT_BYTE_COUNT_PC34,
        REDMCSB_F0054_ALLOCATION_PERMANENT_PC34,
        REDMCSB_F0054_INTERFACE_FONT_MEMORY_REQUEST_PC34);
    callbacks->load_graphic(callbacks->context,
                            REDMCSB_F0054_LOAD_FONT_FLAGS_PC34,
                            REDMCSB_F0054_GRAPHIC_FONT_PC34,
                            state->interface_and_scrolls_font, 0, 0);

    for (row_index = 0;
         row_index < REDMCSB_F0054_MESSAGE_AREA_ROW_COUNT_PC34;
         row_index++) {
        state->message_area_row_expiration_time[row_index] = -1;
    }
    return true;
}
