#include "theron_v1_track19_jp_level_labels.h"

#include <string.h>

/* FNV-1a over the exact 15-record span, including each 0x8197 delimiter. */
#define THERON_TRACK19_JP_LEVEL_LABEL_FNV1A 0xFA2F5B6Fu

static int theron_v1_track19_jp_level_span_valid(
        const uint8_t *iso, size_t iso_size, unsigned int index,
        size_t *selected_offset) {
    uint32_t hash = 2166136261u;
    unsigned int i;

    if (!iso || iso_size < THERON_TRACK19_JP_LEVEL_LABEL_END ||
        index >= THERON_TRACK19_JP_LEVEL_LABEL_COUNT) return 0;
    for (size_t p = THERON_TRACK19_JP_LEVEL_LABEL_OFFSET;
         p < THERON_TRACK19_JP_LEVEL_LABEL_END; ++p) {
        hash ^= iso[p];
        hash *= 16777619u;
    }
    if (hash != THERON_TRACK19_JP_LEVEL_LABEL_FNV1A) return 0;
    for (i = 0u; i < THERON_TRACK19_JP_LEVEL_LABEL_COUNT; ++i) {
        size_t offset = THERON_TRACK19_JP_LEVEL_LABEL_OFFSET +
            i * (THERON_TRACK19_JP_LEVEL_LABEL_BYTES + 2u);
        if (iso[offset + THERON_TRACK19_JP_LEVEL_LABEL_BYTES] != 0x81u ||
            iso[offset + THERON_TRACK19_JP_LEVEL_LABEL_BYTES + 1u] != 0x97u) {
            return 0;
        }
        if (i == index) *selected_offset = offset;
    }
    return THERON_TRACK19_JP_LEVEL_LABEL_OFFSET +
        THERON_TRACK19_JP_LEVEL_LABEL_COUNT *
            (THERON_TRACK19_JP_LEVEL_LABEL_BYTES + 2u) ==
        THERON_TRACK19_JP_LEVEL_LABEL_END;
}

int theron_v1_track19_jp_level_label_from_iso(
        const uint8_t *iso, size_t iso_size, unsigned int index,
        uint8_t *out, size_t out_capacity, size_t *out_size) {
    size_t offset = 0u;

    if (!out || !out_size || out_capacity < THERON_TRACK19_JP_LEVEL_LABEL_BYTES ||
        !theron_v1_track19_jp_level_span_valid(
            iso, iso_size, index, &offset)) return 0;
    memcpy(out, iso + offset, THERON_TRACK19_JP_LEVEL_LABEL_BYTES);
    *out_size = THERON_TRACK19_JP_LEVEL_LABEL_BYTES;
    return 1;
}
