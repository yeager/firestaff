#include "theron_v1_track02_level_labels.h"
#include <stddef.h>

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * UD offset 0x274230, 16 entries × 9 bytes (8-char fixed-width + null).
 * Entry 0 is 8 spaces (blank/cleared indicator).
 * Entries 1-15 are "LEVEL  1" through "LEVEL 15".
 * Used by the HUD depth indicator during dungeon exploration. */
static const char *const g_labels[THERON_TRACK02_LEVEL_LABEL_COUNT] = {
    "        ",  /* 0 — UD 0x274232: blank (8 spaces) */
    "LEVEL  1",  /* 1 — UD 0x27423B */
    "LEVEL  2",  /* 2 — UD 0x274244 */
    "LEVEL  3",  /* 3 — UD 0x27424D */
    "LEVEL  4",  /* 4 — UD 0x274256 */
    "LEVEL  5",  /* 5 — UD 0x27425F */
    "LEVEL  6",  /* 6 — UD 0x274268 */
    "LEVEL  7",  /* 7 — UD 0x274271 */
    "LEVEL  8",  /* 8 — UD 0x27427A */
    "LEVEL  9",  /* 9 — UD 0x274283 */
    "LEVEL 10",  /* 10 — UD 0x27428C */
    "LEVEL 11",  /* 11 — UD 0x274295 */
    "LEVEL 12",  /* 12 — UD 0x27429E */
    "LEVEL 13",  /* 13 — UD 0x2742A7 */
    "LEVEL 14",  /* 14 — UD 0x2742B0 */
    "LEVEL 15",  /* 15 — UD 0x2742B9 */
};

const char *theron_v1_track02_us_level_label(unsigned int index) {
    if (index >= THERON_TRACK02_LEVEL_LABEL_COUNT) return NULL;
    return g_labels[index];
}

size_t theron_v1_track02_us_level_label_count(void) {
    return THERON_TRACK02_LEVEL_LABEL_COUNT;
}
