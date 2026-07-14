#include "redmcsb_f0019_main_display_error_and_stop_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

typedef struct {
    char calls[6];
    size_t call_count;
    size_t character_count;
    int16_t colors[3];
    char prefix[32];
    char digits[2];
} callback_log;

static void record_message(void *context, int16_t color, const char *message)
{
    callback_log *log = context;

    log->calls[log->call_count++] = 'M';
    log->colors[0] = color;
    snprintf(log->prefix, sizeof(log->prefix), "%s", message);
}

static void record_character(void *context, int16_t color, char character)
{
    callback_log *log = context;
    size_t digit_index = log->character_count++;

    log->calls[log->call_count++] = 'C';
    log->colors[digit_index + 1] = color;
    log->digits[digit_index] = character;
}

static void record_wait(void *context)
{
    callback_log *log = context;

    log->calls[log->call_count++] = 'W';
}

static void record_endgame(void *context)
{
    callback_log *log = context;

    log->calls[log->call_count++] = 'E';
}

int main(void)
{
    redmcsb_f0019_display_error_and_stop_state state = { false, 0 };
    callback_log log = { { 0 }, 0, 0, { 0 }, { 0 }, { 0 } };
    redmcsb_f0019_display_error_and_stop_callbacks callbacks = {
        &log, record_message, record_character, record_wait, record_endgame
    };

    check(F0019_MAIN_DisplayErrorAndStop_PC34(&state, &callbacks, 45),
          "the adapter accepts complete callbacks");
    check(state.terminal_error_requested && state.error_number == 45,
          "the terminal error state retains the source error number");
    check(log.call_count == 5 && memcmp(log.calls, "MCCWE", 5) == 0,
          "PC 3.4 prints, waits, then enters endgame in source order");
    check(strcmp(log.prefix, "\n\033SYSTEM ERROR ") == 0 &&
              log.colors[0] == REDMCSB_F0019_ERROR_TEXT_COLOR_PC34 &&
              log.colors[1] == REDMCSB_F0019_ERROR_TEXT_COLOR_PC34 &&
              log.colors[2] == REDMCSB_F0019_ERROR_TEXT_COLOR_PC34 &&
              log.digits[0] == '4' && log.digits[1] == '5',
          "the message prefix and two decimal digits match PC 3.4");

    state.terminal_error_requested = false;
    check(!F0019_MAIN_DisplayErrorAndStop_PC34(&state, NULL, 50) &&
              !state.terminal_error_requested,
          "missing callbacks perform no state transition");

    return failures ? 1 : 0;
}
