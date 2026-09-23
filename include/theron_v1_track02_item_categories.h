#ifndef THERON_V1_TRACK02_ITEM_CATEGORIES_H
#define THERON_V1_TRACK02_ITEM_CATEGORIES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Historical dungeon-7 analysis fixture from US Track 02 UD 0x21A046.
 * These bytes are dungeon-local type codes, not global semantic categories.
 * Production uses Theron_Track02ItemNameSource for all seven US/JP tables and
 * the thing record's own category family.  Keep this API fixture-only. */

#define THERON_TRACK02_ITEM_CATEGORY_COUNT  66u

#define THERON_ITEM_CAT_COMPASS     0x22
#define THERON_ITEM_CAT_WEAPON      0x80
#define THERON_ITEM_CAT_ARMOR       0x81
#define THERON_ITEM_CAT_CONSUMABLE  0x82
/* Neutral carried category copied from the authentic thing-record family.
 * Unlike the legacy Demon-bank values above, this is not a type-table byte. */
#define THERON_ITEM_CAT_SOURCE_MISC 0x0au

uint8_t theron_v1_track02_item_category(unsigned int index);
size_t theron_v1_track02_item_category_count(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_ITEM_CATEGORIES_H */
