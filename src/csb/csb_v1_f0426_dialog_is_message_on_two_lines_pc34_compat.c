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
    /* The bound must be tested before the subscript. In the old order the
     * loop read part1[splitPosition] first, so a message with no space in
     * its second half exited with splitPosition == stringLength and the
     * strcpy below started at part1[stringLength + 1] -- one past the
     * terminator, and past the caller's buffer for a full-length message. */
    while (splitPosition < stringLength && part1[splitPosition] != ' ') {
        splitPosition++;
    }
    if (splitPosition >= stringLength) {
        /* No split point exists, so this is not a two-line message. */
        return 0;
    }
    part1[splitPosition] = '\0';
    strcpy(part2, &part1[splitPosition + 1U]);
    return 1;
}
