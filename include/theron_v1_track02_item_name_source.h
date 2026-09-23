#ifndef THERON_V1_TRACK02_ITEM_NAME_SOURCE_H
#define THERON_V1_TRACK02_ITEM_NAME_SOURCE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THERON_TRACK02_ITEM_SLOT_COUNT 66u
#define THERON_TRACK02_ITEM_NAME_SOURCE_MAX_COUNT 80u
#define THERON_TRACK02_ITEM_NAME_SOURCE_COUNT THERON_TRACK02_ITEM_SLOT_COUNT
#define THERON_TRACK02_ITEM_NAME_SOURCE_CAPACITY 32u
#define THERON_TRACK02_ITEM_PROPERTY_SOURCE_SIZE 6u

/* One dungeon-local name table copied from authenticated Track 02 user data.
 * US bytes are ASCII and JP bytes are Shift-JIS.  An empty source entry is
 * retained as a zero-length entry; no fallback label is generated. */
typedef struct {
    int valid;
    int variant; /* 1 = JP, 2 = US */
    unsigned int dungeon_id; /* 1..7 */
    size_t count;
    size_t source_offset;
    size_t source_span_bytes;
    uint32_t source_span_fnv1a;
    size_t type_code_source_offset;
    uint32_t type_code_source_fnv1a;
    uint8_t raw_type_codes[THERON_TRACK02_ITEM_NAME_SOURCE_MAX_COUNT];
    uint8_t raw_names[THERON_TRACK02_ITEM_NAME_SOURCE_MAX_COUNT]
                     [THERON_TRACK02_ITEM_NAME_SOURCE_CAPACITY];
    uint8_t raw_name_sizes[THERON_TRACK02_ITEM_NAME_SOURCE_MAX_COUNT];
    size_t property_source_offset;
    uint32_t property_source_fnv1a;
    uint8_t raw_properties[THERON_TRACK02_ITEM_SLOT_COUNT]
                          [THERON_TRACK02_ITEM_PROPERTY_SOURCE_SIZE];
    int object_item_index_relation_proven;
    int host_text_rendering_proven;
} Theron_Track02ItemNameSource;

int theron_v1_track02_decode_item_name_source(
    const uint8_t *user_data,
    size_t user_data_size,
    int variant,
    unsigned int dungeon_id,
    Theron_Track02ItemNameSource *out);

/* Return the dungeon's quest-artifact name from its authenticated local
 * Track 02 item bank.  No regional or fixture fallback is synthesized. */
int theron_v1_track02_quest_item_name_raw(
    const Theron_Track02ItemNameSource *source,
    const uint8_t **out_bytes,
    size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif
