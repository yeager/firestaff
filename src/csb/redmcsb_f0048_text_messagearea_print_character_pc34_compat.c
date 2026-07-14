#include "redmcsb_f0048_text_messagearea_print_character_pc34_compat.h"

bool F0048_TEXT_MESSAGEAREA_PrintCharacter_PC34(
    redmcsb_f0048_print_message_fn print_message,
    void *context,
    int16_t text_color,
    char character)
{
    char string[2];

    if (print_message == NULL) {
        return false;
    }

    string[0] = character;
    string[1] = '\0';
    print_message(context, text_color, string);
    return true;
}
