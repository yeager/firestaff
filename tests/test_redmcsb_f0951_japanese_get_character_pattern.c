#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0951_japanese_get_character_pattern.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

static void fill_pattern(uint8_t *segment, uint16_t character_index,
                         uint8_t first_value)
{
    uint16_t offset = (uint16_t)((uint32_t)character_index * 16u);
    unsigned int byte_index;

    for (byte_index = 0u;
         byte_index < REDMCSB_F0951_CHARACTER_PATTERN_BYTES;
         byte_index++) {
        segment[(uint16_t)(offset + byte_index)] =
            (uint8_t)(first_value + byte_index);
    }
}

int main(void)
{
    uint8_t ank_segment[REDMCSB_F0951_ANK_SEGMENT_BYTES] = { 0u };
    uint8_t pattern[REDMCSB_F0951_CHARACTER_PATTERN_BYTES] = { 0u };
    uint8_t expected[REDMCSB_F0951_CHARACTER_PATTERN_BYTES];
    const char *evidence;
    (void)evidence;
    unsigned int byte_index;

    fill_pattern(ank_segment, 0x0021u, 0x40u);
    redmcsb_f0951_japanese_get_character_pattern(ank_segment, 0x0021,
                                                  pattern);
    for (byte_index = 0u;
         byte_index < REDMCSB_F0951_CHARACTER_PATTERN_BYTES;
         byte_index++) {
        expected[byte_index] = (uint8_t)(0x40u + byte_index);
    }
    assert(memcmp(pattern, expected, sizeof(pattern)) == 0);

    fill_pattern(ank_segment, 0xffffu, 0xa0u);
    redmcsb_f0951_japanese_get_character_pattern(ank_segment, -1, pattern);
    for (byte_index = 0u;
         byte_index < REDMCSB_F0951_CHARACTER_PATTERN_BYTES;
         byte_index++) {
        expected[byte_index] = (uint8_t)(0xa0u + byte_index);
    }
    assert(memcmp(pattern, expected, sizeof(pattern)) == 0);

    evidence = redmcsb_f0951_japanese_get_character_pattern_source_evidence();
    assert(strstr(evidence, "JAPANESE.C:76-93") != NULL);
    assert(strstr(evidence, "F0951_JAPANESE_GetCharacterPattern") != NULL);
    assert(strstr(evidence, "CX to 0x10") != NULL);
    assert(strstr(evidence, "DS to A100h") != NULL);
    assert(strstr(evidence, "rep movsb") != NULL);
    puts("ok: ReDMCSB F0951 Japanese character-pattern copy");
    return 0;
}
