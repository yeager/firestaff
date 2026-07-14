#include "redmcsb_f0049_text_messagearea_print_integer_unreferenced_pc34_compat.h"

bool F0049_TEXT_MESSAGEAREA_PrintInteger_Unreferenced_PC34(
    redmcsb_f0049_print_message_fn print_message,
    void *context,
    int16_t text_color,
    uint16_t integer)
{
    unsigned int character_index;
    char string[8];

    if (print_message == NULL) {
        return false;
    }

    character_index = 7;
    string[7] = '\0';
    do {
        string[--character_index] = (char)('0' + (integer % 10U));
    } while ((integer /= 10U) != 0U);

    print_message(context, text_color, &string[character_index]);
    return true;
}
