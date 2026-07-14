#include <string.h>

#include "redmcsb_f0819_text_messagearea_print_message_as_japanese_pc34_compat.h"

void redmcsb_f0819_text_messagearea_print_message_as_japanese_pc34_compat(
    DM1_V1_TextMessageState *state,
    int16_t text_color,
    char *string)
{
    char japanese_string[100];

    japanese_string[0] = 0x1B;
    strcpy(&japanese_string[1], string);
    dm1_v1_text_print_message(state, text_color, japanese_string);
}

const char *redmcsb_f0819_text_messagearea_print_message_as_japanese_source_evidence_pc34(void)
{
    return "ReDMCSB TEXT.C:1788-1798; DEFS.H:9851-9854 for PC-98 media";
}
