#include "theron_v1_track19_level_labels.h"

#include <string.h>

/* Source: US Track 19 ISO (MD5 51b40a17b92a30339957ba564aa0015c),
 * MODE1/2048 byte offset 2112059. These are selector labels only; they do
 * not promote any level map, object table, or bitmap semantics. */
static const char *const g_us_level_labels[THERON_TRACK19_US_LEVEL_LABEL_COUNT] = {
    "LEVEL  1", "LEVEL  2", "LEVEL  3", "LEVEL  4", "LEVEL  5",
    "LEVEL  6", "LEVEL  7", "LEVEL  8", "LEVEL  9", "LEVEL 10",
    "LEVEL 11", "LEVEL 12", "LEVEL 13", "LEVEL 14", "LEVEL 15"
};

const char *theron_v1_track19_us_level_label(unsigned int index) {
    if (index >= THERON_TRACK19_US_LEVEL_LABEL_COUNT) return NULL;
    return g_us_level_labels[index];
}

int theron_v1_track19_us_level_label_from_iso(
        const uint8_t *iso, size_t iso_size, unsigned int index,
        char *out, size_t out_capacity) {
    size_t cursor = THERON_TRACK19_US_LEVEL_LABEL_OFFSET;
    unsigned int i;

    if (!iso || index >= THERON_TRACK19_US_LEVEL_LABEL_COUNT || !out ||
        out_capacity == 0u || cursor >= iso_size) return 0;
    out[0] = '\0';
    for (i = 0u; i < THERON_TRACK19_US_LEVEL_LABEL_COUNT; ++i) {
        const char *expected = g_us_level_labels[i];
        size_t remaining = iso_size - cursor;
        const uint8_t *terminator = memchr(iso + cursor, 0, remaining);
        size_t length;

        if (!terminator) return 0;
        length = (size_t)(terminator - (iso + cursor));
        if (length != strlen(expected) ||
            memcmp(iso + cursor, expected, length) != 0) return 0;
        if (i == index) {
            if (length + 1u > out_capacity) return 0;
            memcpy(out, iso + cursor, length);
            out[length] = '\0';
        }
        cursor += length + 1u;
    }
    return out[0] != '\0';
}
