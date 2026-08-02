#include "theron_v1_track02_class_base_stats.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * UD offset 0x1DA870, 16 LE u16 words immediately preceding the
 * experience threshold table at UD 0x1DA890.
 * Likely encodes class/skill base parameters and growth rates. */
static const uint16_t g_words[THERON_TRACK02_CLASS_BASE_WORD_COUNT] = {
    0, 0, 60, 50, 256, 256, 256, 256,
    3, 3,  3,  3,   0,  10,  54,  90,
};

const uint16_t *theron_v1_track02_class_base_words(void) {
    return g_words;
}

size_t theron_v1_track02_class_base_word_count(void) {
    return THERON_TRACK02_CLASS_BASE_WORD_COUNT;
}
