#include "redmcsb_f0019_main_display_error_and_stop_pc34_compat.h"

#include <stddef.h>

bool F0019_MAIN_DisplayErrorAndStop_PC34(
    redmcsb_f0019_display_error_and_stop_state *state,
    const redmcsb_f0019_display_error_and_stop_callbacks *callbacks,
    int16_t error_number)
{
    if (state == NULL || callbacks == NULL || callbacks->print_message == NULL ||
        callbacks->print_character == NULL || callbacks->wait_for_input == NULL ||
        callbacks->enter_endgame == NULL) {
        return false;
    }

    state->terminal_error_requested = true;
    state->error_number = error_number;
    callbacks->print_message(callbacks->context,
                             REDMCSB_F0019_ERROR_TEXT_COLOR_PC34,
                             "\n\033SYSTEM ERROR ");
    callbacks->print_character(callbacks->context,
                               REDMCSB_F0019_ERROR_TEXT_COLOR_PC34,
                               (char)((error_number / 10) + '0'));
    callbacks->print_character(callbacks->context,
                               REDMCSB_F0019_ERROR_TEXT_COLOR_PC34,
                               (char)((error_number % 10) + '0'));
    callbacks->wait_for_input(callbacks->context);
    callbacks->enter_endgame(callbacks->context);
    return true;
}
