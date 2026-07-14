#include <stdint.h>
#include <string.h>

#include "redmcsb_f0817_substitute_first_characters_by_string_pc34_compat.h"

int main(void)
{
    char shorter[32] = "scroll-text";
    char same[32] = "scroll-text";
    char longer[32] = "scroll-text";
    char empty[32] = "scroll-text";
    char replacement_short[] = "map";
    char replacement_same[] = "panel";
    char replacement_long[] = "champion";
    char replacement_empty[] = "";

    if (redmcsb_f0817_substitute_first_characters_by_string_pc34_compat(
            shorter, 6, replacement_short) != -3 || strcmp(shorter, "map-text") != 0 ||
        redmcsb_f0817_substitute_first_characters_by_string_pc34_compat(
            same, 6, replacement_same) != 0 || strcmp(same, "panel-text") != 0 ||
        redmcsb_f0817_substitute_first_characters_by_string_pc34_compat(
            longer, 6, replacement_long) != 2 || strcmp(longer, "champion-text") != 0 ||
        redmcsb_f0817_substitute_first_characters_by_string_pc34_compat(
            empty, 6, replacement_empty) != -6 || strcmp(empty, "-text") != 0) {
        return 1;
    }

    if (strcmp(
            redmcsb_f0817_substitute_first_characters_by_string_source_evidence_pc34(),
            "ReDMCSB STRING.C:106-139; DEFS.H:9492-9496 for PC-98 media") != 0) {
        return 1;
    }

    return 0;
}
