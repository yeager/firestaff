#include <string.h>

#include "redmcsb_f0816_strstr_pc34_compat.h"

int main(void)
{
    char haystack[] = "scroll-text:scroll";
    char first[] = "scroll";
    char second[] = "text";
    char absent[] = "stone";
    char empty[] = "";

    if (redmcsb_f0816_strstr_pc34_compat(haystack, first) != haystack ||
        redmcsb_f0816_strstr_pc34_compat(haystack, second) != haystack + 7 ||
        redmcsb_f0816_strstr_pc34_compat(haystack + 1, first) != haystack + 12 ||
        redmcsb_f0816_strstr_pc34_compat(haystack, absent) != 0 ||
        redmcsb_f0816_strstr_pc34_compat(haystack, empty) != 0) {
        return 1;
    }

    if (strcmp(
            redmcsb_f0816_strstr_source_evidence_pc34(),
            "ReDMCSB STRING.C:77-108; DEFS.H:9482-9487 for PC-98 media") != 0) {
        return 1;
    }

    return 0;
}
