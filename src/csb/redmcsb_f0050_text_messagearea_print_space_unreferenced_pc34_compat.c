#include "redmcsb_f0050_text_messagearea_print_space_unreferenced_pc34_compat.h"

bool F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced_PC34(
    redmcsb_f0050_print_message_fn print_message,
    void *context)
{
    if (print_message == NULL) {
        return false;
    }

    print_message(context, 15, " ");
    return true;
}
