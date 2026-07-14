#include <string.h>

#include "redmcsb_f0817_substitute_first_characters_by_string_pc34_compat.h"

int16_t redmcsb_f0817_substitute_first_characters_by_string_pc34_compat(
    char *string,
    int16_t character_count,
    char *replacement_string)
{
    char *copy_source;
    char *copy_destination;
    int16_t remaining_character_count;

    while ((character_count > 0) && (*replacement_string != '\0')) {
        *string++ = *replacement_string++;
        character_count--;
    }
    if (*replacement_string != '\0') {
        character_count = (int16_t)strlen(replacement_string);
        remaining_character_count = (int16_t)strlen(string);
        copy_source = string + remaining_character_count;
        copy_destination = copy_source + character_count;
        while (remaining_character_count-- >= 0) {
            *copy_destination-- = *copy_source--;
        }
        while (*replacement_string != '\0') {
            *string++ = *replacement_string++;
        }
    } else if (character_count > 0) {
        replacement_string = string + character_count;
        while ((*string++ = *replacement_string++) != '\0') {
        }
        character_count = (int16_t)-character_count;
    }
    return character_count;
}

const char *redmcsb_f0817_substitute_first_characters_by_string_source_evidence_pc34(void)
{
    return "ReDMCSB STRING.C:106-139; DEFS.H:9492-9496 for PC-98 media";
}
