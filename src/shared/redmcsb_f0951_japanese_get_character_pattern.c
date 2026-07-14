#include "redmcsb_f0951_japanese_get_character_pattern.h"

void redmcsb_f0951_japanese_get_character_pattern(
    const uint8_t ank_segment[REDMCSB_F0951_ANK_SEGMENT_BYTES],
    int16_t character_index,
    uint8_t character_pattern[REDMCSB_F0951_CHARACTER_PATTERN_BYTES])
{
    uint16_t source_offset;
    unsigned int byte_index;

    source_offset = (uint16_t)((uint32_t)(uint16_t)character_index * 16u);
    for (byte_index = 0u;
         byte_index < REDMCSB_F0951_CHARACTER_PATTERN_BYTES;
         byte_index++) {
        character_pattern[byte_index] = ank_segment[
            (uint16_t)(source_offset + byte_index)];
    }
}

const char *redmcsb_f0951_japanese_get_character_pattern_source_evidence(void)
{
    return "ReDMCSB JAPANESE.C:76-93 F0951_JAPANESE_GetCharacterPattern "
           "sets CX to 0x10, shifts SI left four times, sets DS to A100h, "
           "and executes rep movsb into the caller's 16-byte buffer.";
}
