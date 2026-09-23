#include "theron_v1_track02_retrieval_text_source.h"

#include <string.h>

typedef struct {
    uint16_t resource_block;
    size_t resource_offset;
    uint32_t resource_fnv1a;
    size_t offset;
    size_t bytes;
    uint32_t fnv1a;
    size_t shared_program_offset;
    uint32_t shared_program_fnv1a;
    size_t ordinal_dispatch_relative_offset;
    uint32_t ordinal_dispatch_fnv1a;
    size_t text_opcode_handler_relative_offset;
    uint32_t text_opcode_handler_fnv1a;
    size_t text_selector_relative_offset;
    uint32_t text_selector_fnv1a;
    size_t text_ordinal_advance_relative_offset;
    uint32_t text_ordinal_advance_fnv1a;
    uint16_t message_list_relative_offset;
} Theron_RetrievalTextSpan;

static const Theron_RetrievalTextSpan g_us_span = {
    0x040du, 0x277000u, 0xeeb43e74u,
    0x27713du, 331u, 0x4777d500u,
    0x264000u, 0x113c8278u,
    0x284au, 0x815cbce4u,
    0x0653u, 0xcafb5d7fu,
    0x16afu, 0x46cd7f6cu,
    0x11d2u, 0x5813b731u,
    0x013du
};
static const Theron_RetrievalTextSpan g_jp_span = {
    0x040cu, 0x276800u, 0x851c05b3u,
    0x27696du, 364u, 0xcb874921u,
    0x263800u, 0x834bede1u,
    0x284au, 0x7700654cu,
    0x0653u, 0xf7f547ccu,
    0x1729u, 0xc8086d7bu,
    0x1209u, 0x69e37389u,
    0x016du
};

static uint32_t retrieval_fnv1a(const uint8_t *bytes, size_t count) {
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0u; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int theron_v1_track02_decode_retrieval_text_source(
    const uint8_t *user_data,
    size_t user_data_size,
    int variant,
    Theron_Track02RetrievalTextSource *out) {
    const Theron_RetrievalTextSpan *span;
    size_t cursor, end;
    unsigned int record;

    if (out) memset(out, 0, sizeof(*out));
    if (!user_data || !out) return 0;
    if (variant == 2) span = &g_us_span;
    else if (variant == 1) span = &g_jp_span;
    else return 0;
    if (span->resource_offset > user_data_size ||
        2048u > user_data_size - span->resource_offset ||
        retrieval_fnv1a(user_data + span->resource_offset, 2048u) !=
            span->resource_fnv1a ||
        span->offset > user_data_size ||
        span->bytes > user_data_size - span->offset ||
        retrieval_fnv1a(user_data + span->offset, span->bytes) != span->fnv1a ||
        span->shared_program_offset > user_data_size ||
        0x8800u > user_data_size - span->shared_program_offset ||
        retrieval_fnv1a(user_data + span->shared_program_offset, 0x8800u) !=
            span->shared_program_fnv1a ||
        retrieval_fnv1a(user_data + span->shared_program_offset +
                            span->ordinal_dispatch_relative_offset,
                        0x103u) != span->ordinal_dispatch_fnv1a ||
        retrieval_fnv1a(user_data + span->shared_program_offset +
                            span->text_opcode_handler_relative_offset,
                        0x21u) != span->text_opcode_handler_fnv1a ||
        retrieval_fnv1a(user_data + span->shared_program_offset +
                            span->text_selector_relative_offset,
                        0x13au) != span->text_selector_fnv1a ||
        retrieval_fnv1a(user_data + span->shared_program_offset +
                            span->text_ordinal_advance_relative_offset,
                        0x23u) != span->text_ordinal_advance_fnv1a ||
        user_data[span->resource_offset + 4u] != 0x16u ||
        user_data[span->resource_offset + 5u] != 0x00u ||
        user_data[span->resource_offset + 0x16u] != 0xcau ||
        user_data[span->resource_offset + 0x17u] != 0x00u ||
        user_data[span->resource_offset + 0xcau] != 0x0au ||
        user_data[span->resource_offset + 0xd4u] !=
            (uint8_t)span->message_list_relative_offset ||
        user_data[span->resource_offset + 0xd5u] !=
            (uint8_t)(span->message_list_relative_offset >> 8) ||
        span->offset != span->resource_offset +
            span->message_list_relative_offset)
        return 0;

    cursor = span->offset;
    end = span->offset + span->bytes;
    for (record = 0u; record < THERON_TRACK02_RETRIEVAL_TEXT_COUNT; ++record) {
        size_t start = cursor;
        size_t size;
        if (variant == 2) {
            const uint8_t *terminator;
            if (end - cursor < 3u || user_data[cursor] != 0x05u ||
                user_data[cursor + 1u] != 0x03u)
                goto reject;
            terminator = (const uint8_t *)memchr(
                user_data + cursor, 0, end - cursor);
            if (!terminator) goto reject;
            cursor = (size_t)(terminator - user_data) + 1u;
            size = cursor - start - 1u;
        } else {
            if (end - cursor < 4u || user_data[cursor] != 0x81u ||
                user_data[cursor + 1u] != 0x96u)
                goto reject;
            cursor += 2u;
            while (end - cursor >= 2u &&
                   !(user_data[cursor] == 0x81u &&
                     user_data[cursor + 1u] == 0x97u))
                cursor += 2u;
            if (end - cursor < 2u) goto reject;
            cursor += 2u;
            size = cursor - start;
        }
        if (size == 0u || size >= THERON_TRACK02_RETRIEVAL_TEXT_CAPACITY)
            goto reject;
        memcpy(out->raw_messages[record], user_data + start, size);
        out->raw_message_sizes[record] = (uint8_t)size;
    }
    if (cursor != end) goto reject;
    out->valid = 1;
    out->variant = variant;
    out->track02_resource_block = span->resource_block;
    out->resource_offset = span->resource_offset;
    out->resource_bytes = 2048u;
    out->resource_fnv1a = span->resource_fnv1a;
    out->source_offset = span->offset;
    out->source_span_bytes = span->bytes;
    out->source_span_fnv1a = span->fnv1a;
    out->post_dungeon_shared_program_offset = span->shared_program_offset;
    out->post_dungeon_shared_program_fnv1a = span->shared_program_fnv1a;
    out->post_dungeon_ordinal_dispatch_offset =
        span->shared_program_offset + span->ordinal_dispatch_relative_offset;
    out->post_dungeon_ordinal_dispatch_fnv1a =
        span->ordinal_dispatch_fnv1a;
    out->post_dungeon_text_opcode_handler_offset =
        span->shared_program_offset +
        span->text_opcode_handler_relative_offset;
    out->post_dungeon_text_opcode_handler_fnv1a =
        span->text_opcode_handler_fnv1a;
    out->post_dungeon_text_selector_offset =
        span->shared_program_offset + span->text_selector_relative_offset;
    out->post_dungeon_text_selector_fnv1a = span->text_selector_fnv1a;
    out->post_dungeon_text_ordinal_advance_offset =
        span->shared_program_offset +
        span->text_ordinal_advance_relative_offset;
    out->post_dungeon_text_ordinal_advance_fnv1a =
        span->text_ordinal_advance_fnv1a;
    out->text_group = 2u;
    out->text_group_script_relative_offset = 0x00cau;
    out->message_list_relative_offset = span->message_list_relative_offset;
    out->resource_record_authenticated = 1;
    out->retrieval_event_relation_proven = 1;
    out->host_text_rendering_proven = 0;
    return 1;

reject:
    memset(out, 0, sizeof(*out));
    return 0;
}
