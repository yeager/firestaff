#ifndef THERON_V1_TRACK02_CLASS_BASE_STATS_H
#define THERON_V1_TRACK02_CLASS_BASE_STATS_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TRACK02_PREFIX_WORD_COUNT 16u

const uint16_t *theron_v1_track02_prefix_words(void);
size_t theron_v1_track02_prefix_word_count(void);

#endif
