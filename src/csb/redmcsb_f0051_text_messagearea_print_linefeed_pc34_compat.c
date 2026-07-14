#include "redmcsb_f0051_text_messagearea_print_linefeed_pc34_compat.h"

bool F0051_TEXT_MESSAGEAREA_PrintLineFeed_PC34(
    redmcsb_f0051_print_message_fn print_message,
    void *context)
{
    if (print_message == NULL) {
        return false;
    }

    print_message(context, 0, "\n");
    return true;
}
