#include "theron_v1_track19_record_window.h"
#include "theron_v1_track02_item_properties.h"
#include <string.h>

int theron_v1_track19_item_property_table_validate(
        const uint8_t *iso, size_t iso_size, int japanese_variant,
        size_t *out_offset, size_t *out_bytes) {
    const size_t offset = japanese_variant
        ? THERON_TRACK19_ITEM_PROPERTY_TABLE_JP_OFFSET
        : THERON_TRACK19_ITEM_PROPERTY_TABLE_US_OFFSET;
    unsigned int i;

    if (!iso || iso_size < offset + THERON_TRACK19_ITEM_PROPERTY_TABLE_BYTES)
        return 0;
    for (i = 0u; i < THERON_TRACK19_ITEM_PROPERTY_TABLE_COUNT; ++i) {
        const Theron_ItemPropertyRecord *record =
            theron_v1_track02_item_property(i);
        if (!record || memcmp(iso + offset +
                              i * THERON_TRACK19_ITEM_PROPERTY_RECORD_BYTES,
                              record,
                              THERON_TRACK19_ITEM_PROPERTY_RECORD_BYTES) != 0)
            return 0;
    }
    if (out_offset) *out_offset = offset;
    if (out_bytes) *out_bytes = THERON_TRACK19_ITEM_PROPERTY_TABLE_BYTES;
    return 1;
}

int theron_v1_track19_item_property_from_iso(
        const uint8_t *iso, size_t iso_size, int japanese_variant,
        unsigned int index, Theron_ItemPropertyRecord *out) {
    size_t offset;
    size_t bytes;

    if (!out || index >= THERON_TRACK19_ITEM_PROPERTY_TABLE_COUNT ||
        !theron_v1_track19_item_property_table_validate(
            iso, iso_size, japanese_variant, &offset, &bytes) ||
        bytes != THERON_TRACK19_ITEM_PROPERTY_TABLE_BYTES) {
        return 0;
    }
    memcpy(out, iso + offset +
           index * THERON_TRACK19_ITEM_PROPERTY_RECORD_BYTES,
           THERON_TRACK19_ITEM_PROPERTY_RECORD_BYTES);
    return 1;
}

/* FNV-1a over the identical 502-byte US/JP window in the real ISO corpus.
 * It intentionally overlaps 395 bytes of the authenticated property table;
 * the unclassified remainder is not promoted to object/map semantics. */
#define THERON_TRACK19_OPAQUE_RECORD_WINDOW_FNV1A 0xC48424F2u

int theron_v1_track19_opaque_record_window_validate(
        const uint8_t *iso, size_t iso_size, int japanese_variant,
        size_t *out_offset, size_t *out_bytes) {
    const size_t offset = japanese_variant
        ? THERON_TRACK19_OPAQUE_RECORD_WINDOW_JP_OFFSET
        : THERON_TRACK19_OPAQUE_RECORD_WINDOW_US_OFFSET;
    uint32_t hash = 2166136261u;

    if (!iso || iso_size < offset + THERON_TRACK19_OPAQUE_RECORD_WINDOW_BYTES) {
        return 0;
    }
    for (size_t i = 0u; i < THERON_TRACK19_OPAQUE_RECORD_WINDOW_BYTES; ++i) {
        hash ^= iso[offset + i];
        hash *= 16777619u;
    }
    if (hash != THERON_TRACK19_OPAQUE_RECORD_WINDOW_FNV1A) return 0;
    if (out_offset) *out_offset = offset;
    if (out_bytes) *out_bytes = THERON_TRACK19_OPAQUE_RECORD_WINDOW_BYTES;
    return 1;
}

int theron_v1_track19_startup_level_envelope_validate(
        const uint8_t *iso, size_t iso_size, size_t *out_offset,
        size_t *out_bytes, uint32_t *out_fnv1a) {
    static const uint8_t header[THERON_TRACK19_STARTUP_LEVEL_HEADER_BYTES] = {
        0x00u, 0x20u, 0x00u, 0x1Bu, 0x01u, 0x08u,
        0xE9u, 0x38u, 0x00u, 0x26u, 0x01u, 0x03u
    };
    uint32_t hash = 2166136261u;
    size_t i;

    if (!iso || iso_size < THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET +
            THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_BYTES ||
        memcmp(iso + THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET, header,
               sizeof(header)) != 0) {
        return 0;
    }
    for (i = 0u; i < THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_BYTES; ++i) {
        hash ^= iso[THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET + i];
        hash *= 16777619u;
    }
    if (hash != THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_FNV1A) return 0;
    if (out_offset) *out_offset = THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET;
    if (out_bytes) *out_bytes = THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_BYTES;
    if (out_fnv1a) *out_fnv1a = hash;
    return 1;
}
