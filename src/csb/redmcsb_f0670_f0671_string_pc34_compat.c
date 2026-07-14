#include "redmcsb_f0670_f0671_string_pc34_compat.h"

#include <string.h>

int redmcsb_f0670_replace_character_by_string_pc34_compat(
    const char *source_string,
    const char *replacement_string,
    char character,
    char *destination_string)
{
    const char *replacement_position;
    size_t prefix_length;

    if (source_string == NULL || replacement_string == NULL ||
        destination_string == NULL) {
        return -1;
    }
    replacement_position = strchr(source_string, character);
    if (replacement_position == NULL) {
        strcpy(destination_string, source_string);
        return 0;
    }
    prefix_length = (size_t)(replacement_position - source_string);
    memcpy(destination_string, source_string, prefix_length);
    destination_string[prefix_length] = '\0';
    strcat(destination_string, replacement_string);
    strcat(destination_string, replacement_position + 1);
    return 1;
}

void redmcsb_f0671_convert_long_to_string_pc34_compat(
    int32_t value,
    char *destination_string)
{
    char local_string[14];
    uint32_t magnitude;
    unsigned int character_index = 13U;
    int is_negative = value < 0;

    magnitude = is_negative ? (uint32_t)(-(int64_t)value) : (uint32_t)value;
    local_string[character_index] = '\0';
    do {
        local_string[--character_index] =
            (char)('0' + (magnitude % UINT32_C(10)));
        magnitude /= UINT32_C(10);
    } while (magnitude != 0U);
    if (is_negative) local_string[--character_index] = '-';
    strcpy(destination_string, &local_string[character_index]);
}

const char *redmcsb_f0670_f0671_string_source_evidence_pc34(void)
{
    return "ReDMCSB STRING.C F0670_ReplaceCharacterByString (143-162); "
           "F0671_ConvertLongToString (164-187), I34E/I34M route";
}
