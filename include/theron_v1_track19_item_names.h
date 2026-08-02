#ifndef THERON_V1_TRACK19_ITEM_NAMES_H
#define THERON_V1_TRACK19_ITEM_NAMES_H

#include <stddef.h>

/* US Track 19 ISO runtime item name table.
 * Source: TQUS19.iso (MD5 51b40a17b92a30339957ba564aa0015c)
 * Offset: 0x0E9271 (MODE1/2048 user data)
 * This table is loaded during dungeon gameplay and includes
 * dungeon-specific items not present in Track 02's table. */

#define THERON_TRACK19_US_ITEM_NAME_COUNT 69u

const char *theron_v1_track19_us_item_name(unsigned int index);
size_t theron_v1_track19_us_item_name_count(void);

#endif
