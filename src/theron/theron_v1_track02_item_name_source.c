#include "theron_v1_track02_item_name_source.h"

#include <string.h>

typedef struct {
    size_t offset;
    size_t count;
    size_t bytes;
    uint32_t fnv1a;
    uint32_t type_fnv1a;
    size_t property_offset;
    uint32_t property_fnv1a;
} Theron_Track02ItemNameSpan;

static const Theron_Track02ItemNameSpan g_us_spans[7] = {
    {0x099517u, 80u, 782u, 0xda9c2c2au, 0xf6a52f0au, 0x099825u, 0xb97787efu},
    {0x0d9b32u, 65u, 659u, 0xba87d012u, 0x18598643u, 0x0d9dc5u, 0xb97787efu},
    {0x11a22bu, 69u, 681u, 0xa0532734u, 0xb9d60ae1u, 0x11a4d4u, 0xb97787efu},
    {0x159a71u, 69u, 684u, 0x7dd92f3fu, 0x21533bb5u, 0x159d1du, 0xb97787efu},
    {0x19a397u, 67u, 695u, 0x7d653b8au, 0x2e80b74au, 0x19a64eu, 0xb97787efu},
    {0x1d9737u, 63u, 616u, 0x2a875409u, 0x47c0f5d8u, 0x1d999fu, 0xb97787efu},
    {0x21a08eu, 66u, 671u, 0x4c218d40u, 0x17209737u, 0x21a32du, 0xb97787efu}
};

static const Theron_Track02ItemNameSpan g_jp_spans[7] = {
    {0x098d77u, 80u, 811u, 0x586b1dddu, 0xad48a7c7u, 0x0990a2u, 0xb97787efu},
    {0x0d938au, 65u, 652u, 0x810c3cccu, 0x0f10e982u, 0x0d9616u, 0x6c4d1386u},
    {0x119a97u, 69u, 694u, 0xe8d0c9afu, 0xda37b625u, 0x119d4du, 0xb97787efu},
    {0x1592b1u, 69u, 684u, 0xbc7c9658u, 0xf9c3eabbu, 0x15955du, 0xb97787efu},
    {0x199bfbu, 67u, 694u, 0x7f1bd484u, 0x4d8a2ff9u, 0x199eb1u, 0xb97787efu},
    {0x1d8f7fu, 63u, 602u, 0xda4399b5u, 0x78e10802u, 0x1d91d9u, 0xb97787efu},
    {0x21988eu, 66u, 645u, 0x9855519au, 0xa8136977u, 0x219b13u, 0xb97787efu}
};

/* Proven by matching the seven quest-artifact labels inside each authentic
 * dungeon-local Track 02 name bank.  These are item-table indices, not object
 * record offsets. */
static const uint8_t g_quest_item_indices[7] = {
    41u, 63u, 45u, 43u, 44u, 43u, 7u
};

static uint32_t theron_track02_item_name_fnv1a(
    const uint8_t *bytes, size_t count) {
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int theron_v1_track02_decode_item_name_source(
    const uint8_t *user_data,
    size_t user_data_size,
    int variant,
    unsigned int dungeon_id,
    Theron_Track02ItemNameSource *out) {
    const Theron_Track02ItemNameSpan *span;
    size_t cursor, end;
    unsigned int i;

    if (out) memset(out, 0, sizeof(*out));
    if (!out || !user_data || dungeon_id < 1u || dungeon_id > 7u)
        return 0;
    if (variant == 2) span = &g_us_spans[dungeon_id - 1u];
    else if (variant == 1) span = &g_jp_spans[dungeon_id - 1u];
    else return 0;
    if (span->count > THERON_TRACK02_ITEM_NAME_SOURCE_MAX_COUNT ||
        span->offset < span->count + 6u || span->offset > user_data_size ||
        span->bytes > user_data_size - span->offset ||
        theron_track02_item_name_fnv1a(
            user_data + span->offset - span->count - 6u,
            span->count) != span->type_fnv1a ||
        theron_track02_item_name_fnv1a(
            user_data + span->offset, span->bytes) != span->fnv1a ||
        span->property_offset != span->offset + span->bytes ||
        span->property_offset > user_data_size ||
        THERON_TRACK02_ITEM_SLOT_COUNT *
                THERON_TRACK02_ITEM_PROPERTY_SOURCE_SIZE >
            user_data_size - span->property_offset ||
        theron_track02_item_name_fnv1a(
            user_data + span->property_offset,
            THERON_TRACK02_ITEM_SLOT_COUNT *
                THERON_TRACK02_ITEM_PROPERTY_SOURCE_SIZE) !=
            span->property_fnv1a)
        return 0;

    cursor = span->offset;
    end = span->property_offset + 1u;
    for (i = 0u; i < span->count; ++i) {
        const uint8_t *terminator;
        size_t length;
        if (cursor >= end) return 0;
        terminator = (const uint8_t *)memchr(user_data + cursor, 0,
                                             end - cursor);
        if (!terminator) return 0;
        length = (size_t)(terminator - (user_data + cursor));
        if (length >= THERON_TRACK02_ITEM_NAME_SOURCE_CAPACITY)
            return 0;
        if (length != 0u)
            memcpy(out->raw_names[i], user_data + cursor, length);
        out->raw_name_sizes[i] = (uint8_t)length;
        cursor += length + 1u;
    }
    if (cursor != end) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    out->valid = 1;
    out->variant = variant;
    out->dungeon_id = dungeon_id;
    out->count = span->count;
    out->source_offset = span->offset;
    out->source_span_bytes = span->bytes;
    out->source_span_fnv1a = span->fnv1a;
    out->type_code_source_offset = span->offset - span->count - 6u;
    out->type_code_source_fnv1a = span->type_fnv1a;
    memcpy(out->raw_type_codes,
           user_data + span->offset - span->count - 6u, span->count);
    out->property_source_offset = span->property_offset;
    out->property_source_fnv1a = span->property_fnv1a;
    memcpy(out->raw_properties, user_data + span->property_offset,
           sizeof(out->raw_properties));
    out->object_item_index_relation_proven = 1;
    out->host_text_rendering_proven = 0;
    return 1;
}

int theron_v1_track02_quest_item_name_raw(
    const Theron_Track02ItemNameSource *source,
    const uint8_t **out_bytes,
    size_t *out_size) {
    unsigned int item_index;
    if (out_bytes) *out_bytes = NULL;
    if (out_size) *out_size = 0u;
    if (!source || !out_bytes || !out_size || !source->valid ||
        (source->variant != 1 && source->variant != 2) ||
        source->dungeon_id < 1u || source->dungeon_id > 7u ||
        !source->object_item_index_relation_proven ||
        source->host_text_rendering_proven)
        return 0;
    item_index = g_quest_item_indices[source->dungeon_id - 1u];
    if (item_index >= source->count ||
        source->raw_name_sizes[item_index] == 0u)
        return 0;
    *out_bytes = source->raw_names[item_index];
    *out_size = source->raw_name_sizes[item_index];
    return 1;
}
