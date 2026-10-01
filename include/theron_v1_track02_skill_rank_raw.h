#ifndef THERON_V1_TRACK02_SKILL_RANK_RAW_H
#define THERON_V1_TRACK02_SKILL_RANK_RAW_H

#include "theron_v1_track02_champion_strings.h"

#include <stdint.h>

/* Raw authentic Track 02 rank record bytes, excluding the NUL terminator.
 * The US UD 0x1C9B6B and JP Rev. 1 UD 0x89333 records are independently
 * authenticated. These data-only accessors preserve custom glyph bytes
 * without assigning them rendering semantics. Outputs are cleared on failure. */
int theron_v1_track02_us_skill_level_source_record(
    unsigned int index, const uint8_t **out_bytes, size_t *out_size);
int theron_v1_track02_jp_skill_level_source_record(
    unsigned int index, const uint8_t **out_bytes, size_t *out_size);

#endif
