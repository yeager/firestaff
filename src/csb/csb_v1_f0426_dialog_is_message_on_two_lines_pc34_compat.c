#include "csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat.h"

#include <string.h>

const char *csb_v1_f0426_dialog_is_message_on_two_lines_source_evidence_pc34(void)
{
    return "ReDMCSB DIALOG.C:259-280 F0426; CHANGE7_35; "
           "DIALOG.C:383-387 6-pixel centered dialog glyph geometry";
}

int csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat(
    char *message,
    char *part1,
    char *part2)
{
    unsigned int stringLength;
    unsigned int splitPosition;

    stringLength = (unsigned int)strlen(message);
    if (stringLength <= 30U) {
        return 0;
    }
    strcpy(part1, message);
    splitPosition = stringLength >> 1;
    while ((part1[splitPosition] != ' ') && splitPosition < stringLength) {
        splitPosition++;
    }
    part1[splitPosition] = '\0';
    strcpy(part2, &part1[splitPosition + 1U]);
    return 1;
}

int F0426_DIALOG_IsMessageOnTwoLines(
    char *message,
    char *part1,
    char *part2)
{
    return csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat(
        message,
        part1,
        part2);
}
