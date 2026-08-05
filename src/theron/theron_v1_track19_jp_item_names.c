#include "theron_v1_track19_jp_item_names.h"

#include <string.h>

/* FNV-1a over the exact 69-entry span in the real JP Track 19 ISO.  The
 * complete file is separately MD5-authenticated by track19_inventory_file;
 * this span check keeps the byte-level accessor fail-closed when used alone. */
#define THERON_TRACK19_JP_ITEM_NAME_FNV1A 0x1020AC88u

static int theron_v1_track19_jp_item_span_valid(
        const uint8_t *iso, size_t iso_size, size_t *selected_offset,
        size_t *selected_size, unsigned int index) {
    size_t cursor = THERON_TRACK19_JP_ITEM_NAME_OFFSET;
    size_t end = THERON_TRACK19_JP_ITEM_NAME_END;
    unsigned int i;
    uint32_t hash = 2166136261u;

    if (!iso || iso_size < end || index >= THERON_TRACK19_JP_ITEM_NAME_COUNT) {
        return 0;
    }
    for (size_t p = cursor; p < end; ++p) {
        hash ^= iso[p];
        hash *= 16777619u;
    }
    if (hash != THERON_TRACK19_JP_ITEM_NAME_FNV1A) return 0;

    for (i = 0u; i < THERON_TRACK19_JP_ITEM_NAME_COUNT; ++i) {
        size_t terminator = cursor;
        while (terminator < end && iso[terminator] != 0u) ++terminator;
        if (terminator == cursor || terminator >= end) return 0;
        if (i == index) {
            *selected_offset = cursor;
            *selected_size = terminator - cursor;
        }
        cursor = terminator + 1u;
    }
    return cursor == end;
}

int theron_v1_track19_jp_item_name_from_iso(
        const uint8_t *iso, size_t iso_size, unsigned int index,
        uint8_t *out, size_t out_capacity, size_t *out_size) {
    size_t offset = 0u;
    size_t size = 0u;

    if (!out || !out_size || out_capacity == 0u ||
        !theron_v1_track19_jp_item_span_valid(
            iso, iso_size, &offset, &size, index) || size > out_capacity) {
        return 0;
    }
    memcpy(out, iso + offset, size);
    *out_size = size;
    return 1;
}
