#include "redmcsb_f0816_strstr_pc34_compat.h"

char *redmcsb_f0816_strstr_pc34_compat(char *string_haystack, char *string_needle)
{
    char *candidate;
    char *needle_cursor;
    char haystack_character;
    char needle_character;

    if ((needle_character = *string_needle) != '\0') {
        while ((haystack_character = *string_haystack) != '\0') {
            if (haystack_character == needle_character) {
                candidate = string_haystack + 1;
                needle_cursor = string_needle + 1;
                while ((*needle_cursor != '\0') && (*candidate == *needle_cursor)) {
                    candidate++;
                    needle_cursor++;
                }
                if (*needle_cursor == '\0') {
                    return string_haystack;
                }
            }
            string_haystack++;
        }
    }
    return 0;
}

const char *redmcsb_f0816_strstr_source_evidence_pc34(void)
{
    return "ReDMCSB STRING.C:77-108; DEFS.H:9482-9487 for PC-98 media";
}
