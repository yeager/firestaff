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

bool F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced(
    redmcsb_f0050_print_message_fn print_message,
    void *context)
{
    return F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced_PC34(
        print_message,
        context);
}

const char *redmcsb_f0050_text_messagearea_print_space_source_evidence_pc34(void)
{
    return "ReDMCSB TEXT.C F0050_TEXT_MESSAGEAREA_PrintSpace_Unreferenced: "
           "call F0047_TEXT_MESSAGEAREA_PrintMessage once with color C15 "
           "and a single ASCII space.";
}
