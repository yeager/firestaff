#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat.h"

int main(void)
{
    char shortMessage[] = "123456789012345678901234567890";
    (void)shortMessage;
    char atMidpoint[] = "0123456789ABCDEF right-side-text";
    (void)atMidpoint;
    char rightOfMidpoint[] = "0123456789ABCDEFx right-side-text";
    (void)rightOfMidpoint;
    char part1[64];
    char part2[64];

    memset(part1, 'A', sizeof(part1));
    memset(part2, 'B', sizeof(part2));
    assert(csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat(
               shortMessage, part1, part2) == 0);
    assert(part1[0] == 'A');
    assert(part2[0] == 'B');

    assert(csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat(
               atMidpoint, part1, part2) == 1);
    assert(strcmp(part1, "0123456789ABCDEF") == 0);
    assert(strcmp(part2, "right-side-text") == 0);

    assert(csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat(
               rightOfMidpoint, part1, part2) == 1);
    assert(strcmp(part1, "0123456789ABCDEFx") == 0);
    assert(strcmp(part2, "right-side-text") == 0);

    assert(strstr(csb_v1_f0426_dialog_is_message_on_two_lines_source_evidence_pc34(),
                  "F0426") != NULL);
    printf("PASS csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat\n");
    return 0;
}
