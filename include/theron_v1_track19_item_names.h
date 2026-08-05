#ifndef THERON_V1_TRACK19_ITEM_NAMES_H
#define THERON_V1_TRACK19_ITEM_NAMES_H

#include <stddef.h>
#include <stdint.h>

/* US Track 19 ISO runtime item name table.
 * Source: TQUS19.iso (MD5 51b40a17b92a30339957ba564aa0015c)
 * Offset: 0x0E9271 (MODE1/2048 user data)
 * This table is loaded during dungeon gameplay and includes
 * dungeon-specific items not present in Track 02's table. */

#define THERON_TRACK19_US_ITEM_NAME_COUNT 69u
#define THERON_TRACK19_US_ITEM_NAME_OFFSET 0x0E9271u

const char *theron_v1_track19_us_item_name(unsigned int index);
size_t theron_v1_track19_us_item_name_count(void);

/* Validate and read one name from the source-bound US Track 19 ISO span.
 * The complete 69-entry table must match before any name is returned. */
int theron_v1_track19_us_item_name_from_iso(
    const uint8_t *iso, size_t iso_size, unsigned int index,
    char *out, size_t out_capacity);

#endif
