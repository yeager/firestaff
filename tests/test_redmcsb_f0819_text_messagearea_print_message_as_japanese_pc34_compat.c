#include <stdint.h>
#include <string.h>

#include "redmcsb_f0819_text_messagearea_print_message_as_japanese_pc34_compat.h"

int main(void)
{
    DM1_V1_TextMessageState state;
    const DM1_V1_MessageRow *row;
    char message[] = "MORNINGSTAR";

    dm1_v1_text_init(&state);
    redmcsb_f0819_text_messagearea_print_message_as_japanese_pc34_compat(
        &state, (int16_t)DM1_V1_COLOR_CYAN, message);

    row = dm1_v1_text_get_row(&state, DM1_V1_MESSAGE_AREA_ROW_COUNT - 1);
    if (row == 0 || (unsigned char)row->text[0] != 0x1BU ||
        strcmp(&row->text[1], message) != 0 ||
        row->color != DM1_V1_COLOR_CYAN ||
        strcmp(redmcsb_f0819_text_messagearea_print_message_as_japanese_source_evidence_pc34(),
               "ReDMCSB TEXT.C:1788-1798; DEFS.H:9851-9854 for PC-98 media") != 0) {
        return 1;
    }

    return 0;
}
