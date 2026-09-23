#include "theron_v1_track19_level_labels.h"

#include <string.h>

#define THERON_TRACK19_US_LEVEL_LABEL_END 0x203AC2u
#define THERON_TRACK19_US_LEVEL_LABEL_FNV1A 0x7F7D9F67u

int theron_v1_track19_us_level_label_from_iso(
        const uint8_t *iso, size_t iso_size, unsigned int index,
        char *out, size_t out_capacity) {
    size_t cursor = THERON_TRACK19_US_LEVEL_LABEL_OFFSET;
    size_t selected_offset = 0u;
    size_t selected_size = 0u;
    uint32_t hash = 2166136261u;
    unsigned int i;

    if (!iso || !out || out_capacity == 0u ||
        iso_size < THERON_TRACK19_US_LEVEL_LABEL_END ||
        index >= THERON_TRACK19_US_LEVEL_LABEL_COUNT) return 0;
    for (size_t p = cursor; p < THERON_TRACK19_US_LEVEL_LABEL_END; ++p) {
        hash ^= iso[p];
        hash *= 16777619u;
    }
    if (hash != THERON_TRACK19_US_LEVEL_LABEL_FNV1A) return 0;

    for (i = 0u; i < THERON_TRACK19_US_LEVEL_LABEL_COUNT; ++i) {
        size_t terminator = cursor;
        while (terminator < THERON_TRACK19_US_LEVEL_LABEL_END &&
               iso[terminator] != 0u) ++terminator;
        if (terminator == cursor ||
            terminator >= THERON_TRACK19_US_LEVEL_LABEL_END) return 0;
        if (i == index) {
            selected_offset = cursor;
            selected_size = terminator - cursor;
        }
        cursor = terminator + 1u;
    }
    if (cursor != THERON_TRACK19_US_LEVEL_LABEL_END ||
        selected_size + 1u > out_capacity) return 0;
    memcpy(out, iso + selected_offset, selected_size);
    out[selected_size] = '\0';
    return 1;
}
