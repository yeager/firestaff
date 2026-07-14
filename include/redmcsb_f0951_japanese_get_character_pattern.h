/*
 * ReDMCSB JAPANESE.C F0951_JAPANESE_GetCharacterPattern.
 *
 * The original reads sixteen bytes from the PC-98 ANK character-pattern
 * segment (A100h). The caller supplies that 64 KiB segment on the host.
 */
#ifndef FIRESTAFF_REDMCSB_F0951_JAPANESE_GET_CHARACTER_PATTERN_H
#define FIRESTAFF_REDMCSB_F0951_JAPANESE_GET_CHARACTER_PATTERN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0951_ANK_SEGMENT_BYTES = 65536u,
    REDMCSB_F0951_CHARACTER_PATTERN_BYTES = 16u
};

/*
 * Copies the pattern at (uint16_t)(character_index << 4) from the supplied
 * A100h segment. As in the original routine, the copy is forward and has no
 * bounds or null-pointer handling.
 */
void redmcsb_f0951_japanese_get_character_pattern(
    const uint8_t ank_segment[REDMCSB_F0951_ANK_SEGMENT_BYTES],
    int16_t character_index,
    uint8_t character_pattern[REDMCSB_F0951_CHARACTER_PATTERN_BYTES]);

const char *redmcsb_f0951_japanese_get_character_pattern_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0951_JAPANESE_GET_CHARACTER_PATTERN_H */
