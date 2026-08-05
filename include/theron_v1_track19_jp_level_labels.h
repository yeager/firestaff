#ifndef THERON_V1_TRACK19_JP_LEVEL_LABELS_H
#define THERON_V1_TRACK19_JP_LEVEL_LABELS_H

#include <stddef.h>
#include <stdint.h>

/* Japanese Track 19 level-selector labels.
 * Source: TQJP19.iso (MD5 f9f069a5e489b91207f3156059b756f1)
 * Each of the 15 records is a fixed 16-byte Shift-JIS payload followed by
 * the source text delimiter 0x8197. */
#define THERON_TRACK19_JP_LEVEL_LABEL_COUNT 15u
#define THERON_TRACK19_JP_LEVEL_LABEL_OFFSET 0x203A7Eu
#define THERON_TRACK19_JP_LEVEL_LABEL_END 0x203B8Cu
#define THERON_TRACK19_JP_LEVEL_LABEL_BYTES 16u

/* Validate the complete source-owned table and copy one raw Shift-JIS label.
 * No label is returned when the authenticated table span is changed. */
int theron_v1_track19_jp_level_label_from_iso(
    const uint8_t *iso, size_t iso_size, unsigned int index,
    uint8_t *out, size_t out_capacity, size_t *out_size);

#endif
