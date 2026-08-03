#ifndef THERON_V1_TRACK02_ITEM_CATEGORIES_H
#define THERON_V1_TRACK02_ITEM_CATEGORIES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN, UD 0x21A046.
 * 66-byte category/type table, one byte per primary item name.
 * Values: 0x22 = COMPASS (unique), 0x80 = weapon, 0x81 = armor, 0x82 = consumable. */

#define THERON_TRACK02_ITEM_CATEGORY_COUNT  66u

#define THERON_ITEM_CAT_COMPASS     0x22
#define THERON_ITEM_CAT_WEAPON      0x80
#define THERON_ITEM_CAT_ARMOR       0x81
#define THERON_ITEM_CAT_CONSUMABLE  0x82

uint8_t theron_v1_track02_item_category(unsigned int index);
size_t theron_v1_track02_item_category_count(void);

int theron_v1_track02_resolve_drop_item(uint8_t synthetic_id, unsigned int seed);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_ITEM_CATEGORIES_H */
