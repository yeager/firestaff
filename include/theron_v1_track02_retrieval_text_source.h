#ifndef THERON_V1_TRACK02_RETRIEVAL_TEXT_SOURCE_H
#define THERON_V1_TRACK02_RETRIEVAL_TEXT_SOURCE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THERON_TRACK02_RETRIEVAL_TEXT_COUNT 7u
#define THERON_TRACK02_RETRIEVAL_TEXT_CAPACITY 96u

typedef struct {
    int valid;
    int variant; /* 1 = JP, 2 = US */
    uint16_t track02_resource_block;
    size_t resource_offset;
    size_t resource_bytes;
    uint32_t resource_fnv1a;
    size_t source_offset;
    size_t source_span_bytes;
    uint32_t source_span_fnv1a;
    size_t post_dungeon_shared_program_offset;
    uint32_t post_dungeon_shared_program_fnv1a;
    size_t post_dungeon_ordinal_dispatch_offset;
    uint32_t post_dungeon_ordinal_dispatch_fnv1a;
    size_t post_dungeon_text_opcode_handler_offset;
    uint32_t post_dungeon_text_opcode_handler_fnv1a;
    size_t post_dungeon_text_selector_offset;
    uint32_t post_dungeon_text_selector_fnv1a;
    size_t post_dungeon_text_ordinal_advance_offset;
    uint32_t post_dungeon_text_ordinal_advance_fnv1a;
    uint8_t text_group;
    uint16_t text_group_script_relative_offset;
    uint16_t message_list_relative_offset;
    uint8_t raw_messages[THERON_TRACK02_RETRIEVAL_TEXT_COUNT]
                        [THERON_TRACK02_RETRIEVAL_TEXT_CAPACITY];
    uint8_t raw_message_sizes[THERON_TRACK02_RETRIEVAL_TEXT_COUNT];
    int resource_record_authenticated;
    int retrieval_event_relation_proven;
    int host_text_rendering_proven;
} Theron_Track02RetrievalTextSource;

/* Decode the seven regional retrieval-message records and authenticate the
 * original post-dungeon ordinal-to-record selector that consumes them. */
int theron_v1_track02_decode_retrieval_text_source(
    const uint8_t *user_data,
    size_t user_data_size,
    int variant,
    Theron_Track02RetrievalTextSource *out);

#ifdef __cplusplus
}
#endif

#endif
