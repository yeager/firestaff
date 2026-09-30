#ifndef TEST_THERON_V1_TRACK02_SKILL_RANK_SOURCE_H
#define TEST_THERON_V1_TRACK02_SKILL_RANK_SOURCE_H

#include <stddef.h>
#include <stdint.h>

/* Raw JP Rev. 1 Track 02 rank record bytes, excluding the NUL terminator.
 * Test-only reference data; this does not define runtime presentation. */
int theron_v1_track02_jp_skill_level_source_record(
    unsigned int index, const uint8_t **out_bytes, size_t *out_size);

#endif
