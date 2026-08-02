#ifndef THERON_V1_TRACK02_FULL_ITEM_NAMES_H
#define THERON_V1_TRACK02_FULL_ITEM_NAMES_H

#include <stddef.h>

/* Track 02 per-bank item name table (the complete runtime table).
 * Source: US Track 02 BIN UD 0x099517, null-separated ASCII.
 * Full 80-item name table starting with COMPASS. */

#define THERON_TRACK02_FULL_ITEM_COUNT 80u

const char *theron_v1_track02_us_full_item_name(unsigned int index);
size_t theron_v1_track02_us_full_item_count(void);

#endif
