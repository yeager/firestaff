#include "csb_v1_csbwin_dungeon_tail.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t read_le32(const uint8_t *p)
{
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static int add_size(size_t *value, size_t add, size_t limit)
{
    if (!value || *value > limit || add > limit - *value) return 0;
    *value += add;
    return 1;
}

static int multiply_size(size_t left, size_t right, size_t *out)
{
    if (!out || (left != 0u && right > (size_t)-1 / left)) return 0;
    *out = left * right;
    return 1;
}

int csb_v1_csbwin_dungeon_tail_parse_prefix(
    const uint8_t *tail,
    size_t tail_size,
    uint8_t extended_flags,
    CSB_V1_CSBWinDungeonTailPrefix *out)
{
    CSB_V1_CSBWinDungeonTailPrefix report;
    size_t offset;
    size_t level_bytes;
    size_t i;
    size_t text_bytes;

    if (!tail || !out) return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT;
    memset(&report, 0, sizeof(report));
    if (tail_size < CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }

    report.dungeon_index_offset = 0u;
    report.sentinel = read_be16(tail + 0u);
    report.legacy_cell_flag_bytes = read_be16(tail + 2u);
    report.level_count = (uint8_t)(read_be16(tail + 4u) >> 8u);
    report.text_word_count = read_be16(tail + 6u);
    report.object_list_length = read_be16(tail + 10u);
    for (i = 0u; i < CSB_V1_CSBWIN_DATABASE_COUNT; ++i) {
        report.database_entries[i] = read_be16(tail + 12u + i * 2u);
    }
    if (report.level_count == 0u ||
        report.level_count > CSB_V1_CSBWIN_MAX_SAVE_LEVELS) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LEVEL_COUNT;
    }

    offset = CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES;
    report.level_descriptors_offset = offset;
    level_bytes = (size_t)report.level_count *
                  CSB_V1_CSBWIN_LEVEL_DESC_BYTES;
    if (!add_size(&offset, level_bytes, tail_size)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    for (i = 0u; i < report.level_count; ++i) {
        const uint16_t word8 = read_be16(
            tail + report.level_descriptors_offset +
            i * CSB_V1_CSBWIN_LEVEL_DESC_BYTES + 8u);
        const uint16_t columns = (uint16_t)(((word8 >> 6u) & 0x1fu) + 1u);
        if (columns > UINT16_MAX - report.column_pointer_count) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_OVERFLOW;
        }
        report.level_last_column[i] = (uint16_t)(columns - 1u);
        report.column_pointer_count =
            (uint16_t)(report.column_pointer_count + columns);
    }

    report.object_list_index_offset = offset;
    if (!add_size(&offset, (size_t)report.column_pointer_count * 2u,
                  tail_size)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    report.object_list_offset = offset;
    if (!add_size(&offset, (size_t)report.object_list_length * 2u,
                  tail_size)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }

    report.text_offset = offset;
    report.indirect_text =
        (extended_flags & CSB_V1_CSBWIN_EXTENDED_FLAG_INDIRECT_TEXT) != 0u;
    if (!multiply_size((size_t)report.text_word_count,
                       report.indirect_text ? 4u : 2u, &text_bytes)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_OVERFLOW;
    }
    if (!add_size(&offset, text_bytes, tail_size)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    if (report.indirect_text) {
        report.compressed_text_size_offset = offset;
        if (!add_size(&offset, 4u, tail_size)) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
        }
        report.compressed_text_word_count =
            read_le32(tail + report.compressed_text_size_offset);
        if (report.compressed_text_word_count >
            CSB_V1_CSBWIN_MAX_COMPRESSED_TEXT_WORDS) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TEXT_SIZE;
        }
        report.compressed_text_offset = offset;
        if (!multiply_size((size_t)report.compressed_text_word_count,
                           2u, &text_bytes)) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_OVERFLOW;
        }
        if (!add_size(&offset, text_bytes, tail_size)) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
        }
    }
    report.next_database_offset = offset;
    report.valid = 1;
    *out = report;
    return CSB_V1_CSBWIN_DUNGEON_TAIL_OK;
}

int csb_v1_csbwin_dungeon_tail_parse_databases(
    const uint8_t *tail,
    size_t tail_size,
    const CSB_V1_CSBWinDungeonTailPrefix *prefix,
    uint8_t extended_features_version,
    uint8_t extended_flags,
    uint32_t extended_cell_flag_bytes,
    CSB_V1_CSBWinDungeonTailDatabaseLayout *out)
{
    static const uint16_t source_entry_bytes[CSB_V1_CSBWIN_DATABASE_COUNT] = {
        4u, 6u, 4u, 10u, 16u, 4u, 4u, 6u,
        4u, 8u, 4u, 256u, 2u, 2u, 8u, 4u
    };
    CSB_V1_CSBWinDungeonTailDatabaseLayout report;
    size_t offset;
    size_t bytes;
    size_t i;
    int checksum_result;

    if (!tail || !prefix || !out) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT;
    }
    if (!prefix->valid || prefix->next_database_offset > tail_size) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_PREFIX;
    }

    memset(&report, 0, sizeof(report));
    report.extended_features_version = extended_features_version;
    report.extended_flags = extended_flags;
    report.big_actuators =
        (extended_flags & CSB_V1_CSBWIN_EXTENDED_FLAG_BIG_ACTUATORS) != 0u;
    report.legacy_scroll_records = extended_features_version < (uint8_t)'B';
    report.cell_flag_size_from_extended_features =
        extended_cell_flag_bytes != 0u;
    report.database_offset = prefix->next_database_offset;
    offset = report.database_offset;

    for (i = 0u; i < CSB_V1_CSBWIN_DATABASE_COUNT; ++i) {
        uint16_t entry_bytes = source_entry_bytes[i];
        CSB_V1_CSBWinDungeonTailDatabaseSpan *span = &report.database[i];

        if (i == 3u && !report.big_actuators) entry_bytes = 8u;
        if (i == 7u && report.legacy_scroll_records) entry_bytes = 4u;
        if (!multiply_size((size_t)prefix->database_entries[i],
                           (size_t)entry_bytes, &bytes)) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_OVERFLOW;
        }
        span->database_number = (uint8_t)i;
        span->entry_count = prefix->database_entries[i];
        span->source_entry_bytes = entry_bytes;
        span->offset = offset;
        span->byte_count = bytes;
        if (!add_size(&offset, bytes, tail_size)) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
        }
    }
    report.database_bytes = offset - report.database_offset;
    report.cell_flags_offset = offset;
    report.cell_flag_bytes = extended_cell_flag_bytes != 0u
        ? extended_cell_flag_bytes
        : (uint32_t)prefix->legacy_cell_flag_bytes;
    if (!add_size(&offset, (size_t)report.cell_flag_bytes, tail_size)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    report.checksum_offset = offset;
    if (!add_size(&offset, 2u, tail_size)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    if (offset != tail_size) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }

    checksum_result = csb_v1_csbwin_dungeon_tail_validate_checksum(
        tail, tail_size, &report.computed_checksum, &report.stored_checksum);
    if (checksum_result != 1) {
        return checksum_result == 0
            ? CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_CHECKSUM
            : CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT;
    }
    report.valid = 1;
    *out = report;
    return CSB_V1_CSBWIN_DUNGEON_TAIL_OK;
}

int csb_v1_csbwin_dungeon_tail_validate_checksum(
    const uint8_t *tail,
    size_t tail_size,
    uint16_t *out_computed,
    uint16_t *out_stored)
{
    uint16_t computed = 0u;
    uint16_t stored;
    size_t i;

    if (!tail || tail_size < 2u) return -1;
    for (i = 0u; i + 2u < tail_size; ++i) {
        computed = (uint16_t)(computed + tail[i]);
    }
    stored = read_be16(tail + tail_size - 2u);
    if (out_computed) *out_computed = computed;
    if (out_stored) *out_stored = stored;
    return computed == stored ? 1 : 0;
}

const char *csb_v1_csbwin_dungeon_tail_source_evidence(void)
{
    return "CSBWin SaveGame.cpp:1236-1337,2285-2411,2536-2859; "
           "CSBWin CSB.h:DUNGEONDATINDEX,LEVELDESC,DB0-DB15; "
           "CSBWin data.cpp:395-413";
}
