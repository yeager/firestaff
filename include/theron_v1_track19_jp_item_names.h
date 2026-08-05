#ifndef THERON_V1_TRACK19_JP_ITEM_NAMES_H
#define THERON_V1_TRACK19_JP_ITEM_NAMES_H

#include <stddef.h>
#include <stdint.h>

/* Japanese Track 19 ISO item-name table.
 * Source: TQJP19.iso (MD5 f9f069a5e489b91207f3156059b756f1)
 * The table is Shift-JIS payload, not host UTF-8 text.  Keep the bytes
 * intact so the original game's font/decoder remains the owner of display
 * semantics. */
#define THERON_TRACK19_JP_ITEM_NAME_COUNT 69u
#define THERON_TRACK19_JP_ITEM_NAME_OFFSET 0x0E92B1u
#define THERON_TRACK19_JP_ITEM_NAME_END 0x0E955Eu

/* Validate the complete source-owned table and copy one raw Shift-JIS name.
 * No name is returned when the authenticated table span is changed. */
int theron_v1_track19_jp_item_name_from_iso(
    const uint8_t *iso, size_t iso_size, unsigned int index,
    uint8_t *out, size_t out_capacity, size_t *out_size);

#endif
