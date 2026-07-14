#ifndef FIRESTAFF_REDMCSB_F0019_MAIN_DISPLAY_ERROR_AND_STOP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0019_MAIN_DISPLAY_ERROR_AND_STOP_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB BASE.C F0019_MAIN_DisplayErrorAndStop, PC 3.4.
 *
 * The PC 3.4 path writes a red "\n\033SYSTEM ERROR " prefix, writes the
 * decimal tens and ones separately, waits for input, and enters F0666_endgame.
 * This adapter records that terminal transition and delegates each external
 * action; it deliberately owns neither UI rendering nor host termination.
 */
enum {
    REDMCSB_F0019_ERROR_TEXT_COLOR_PC34 = 8
};

typedef void (*redmcsb_f0019_print_message_fn)(
    void *context,
    int16_t text_color,
    const char *message);

typedef void (*redmcsb_f0019_print_character_fn)(
    void *context,
    int16_t text_color,
    char character);

typedef void (*redmcsb_f0019_wait_for_input_fn)(void *context);

typedef void (*redmcsb_f0019_endgame_fn)(void *context);

typedef struct {
    void *context;
    redmcsb_f0019_print_message_fn print_message;
    redmcsb_f0019_print_character_fn print_character;
    redmcsb_f0019_wait_for_input_fn wait_for_input;
    redmcsb_f0019_endgame_fn enter_endgame;
} redmcsb_f0019_display_error_and_stop_callbacks;

typedef struct {
    bool terminal_error_requested;
    int16_t error_number;
} redmcsb_f0019_display_error_and_stop_state;

bool F0019_MAIN_DisplayErrorAndStop_PC34(
    redmcsb_f0019_display_error_and_stop_state *state,
    const redmcsb_f0019_display_error_and_stop_callbacks *callbacks,
    int16_t error_number);

#ifdef __cplusplus
}
#endif

#endif
