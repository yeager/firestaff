#include "redmcsb_f0768_print_to_zone_with_trailing_spaces_pc34_compat.h"

void redmcsb_f0768_print_to_zone_with_trailing_spaces_pc34_compat(
    redmcsb_f0768_text_print_pc34_compat print_text,
    void *context,
    uint8_t *bitmap_destination,
    uint16_t width,
    int16_t zone_index,
    int16_t text_color,
    int16_t background_color,
    const char *source_string,
    int16_t string_length,
    int16_t height)
{
    int16_t character_index;
    char string[REDMCSB_F0768_LOCAL_STRING_CAPACITY_PC34];

    /* ReDMCSB WIP20210206 TEXT.C:1272-1315, MEDIA746 PC 3.4 route. */
    character_index = 0;
    while ((string[character_index] = *source_string++)) {
        character_index++;
    }
    while (character_index < string_length) {
        string[character_index] = ' ';
        character_index++;
    }
    string[character_index] = '\0';
    print_text(context, bitmap_destination, width, zone_index, text_color,
               background_color, string, height);
}

const char *redmcsb_f0768_print_to_zone_with_trailing_spaces_source_evidence_pc34(void)
{
    return "ReDMCSB WIP20210206 TEXT.C:1272-1315, MEDIA746 PC 3.4 route: "
           "F0768 copies the source string into a local 80-byte buffer, pads "
           "it with spaces to P0064_i_StringLength, terminates it, then calls "
           "F0040_TEXT_Print with the zone and P2756_i height.";
}
