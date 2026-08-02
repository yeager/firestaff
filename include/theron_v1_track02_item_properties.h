#ifndef THERON_V1_TRACK02_ITEM_PROPERTIES_H
#define THERON_V1_TRACK02_ITEM_PROPERTIES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * 66 item property records from UD 0x099825, 6 bytes each (396 bytes total).
 * One record per item matching the category table at UD 0x21A046 and
 * the item names at UD 0x21A08E. */

#define THERON_TRACK02_ITEM_PROPERTY_COUNT  66u
#define THERON_TRACK02_ITEM_PROPERTY_SIZE    6u

typedef struct {
    uint8_t b0;  /* category/flags */
    uint8_t b1;  /* primary stat */
    uint8_t b2;  /* secondary stat */
    uint8_t b3;  /* tertiary stat */
    uint8_t b4;  /* quaternary stat */
    uint8_t b5;  /* quinary stat */
} Theron_ItemPropertyRecord;

const Theron_ItemPropertyRecord *theron_v1_track02_item_property(unsigned int index);
size_t theron_v1_track02_item_property_count(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_ITEM_PROPERTIES_H */
