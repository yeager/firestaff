#include "csb_v1_csbwin_dungeon_tail.h"

#include <stdlib.h>
#include <string.h>

struct CSB_V1_CSBWinLegacyDungeonCandidate {
    CSB_V1_DungeonData dungeon;
    CSB_V1_CSBWinDungeonTailPrefix prefix;
    CSB_V1_CSBWinDungeonTailDatabaseLayout databases;
};

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

static void swap16(uint8_t *p)
{
    uint8_t byte;
    byte = p[0];
    p[0] = p[1];
    p[1] = byte;
}

static void swap32(uint8_t *p)
{
    uint8_t byte;
    byte = p[0]; p[0] = p[3]; p[3] = byte;
    byte = p[1]; p[1] = p[2]; p[2] = byte;
}

static void swap_record_words(uint8_t *record, const size_t *offsets,
                              size_t offset_count)
{
    size_t i;
    if (!record || !offsets) return;
    for (i = 0u; i < offset_count; ++i) swap16(record + offsets[i]);
}

static int swap_legacy_database(uint8_t *bytes,
                                const CSB_V1_CSBWinDungeonTailDatabaseSpan *span)
{
    static const size_t db0[] = { 0u, 2u };
    static const size_t db1[] = { 0u, 2u, 4u };
    static const size_t db2[] = { 0u, 2u };
    static const size_t db3[] = { 0u, 2u, 4u, 6u };
    static const size_t db4[] = { 0u, 2u, 6u, 8u, 10u, 12u, 14u };
    static const size_t db5[] = { 0u, 2u };
    static const size_t db7[] = { 0u, 2u };
    static const size_t db9[] = { 0u, 2u, 4u };
    static const size_t db14[] = { 0u, 2u, 6u };
    const size_t *offsets = NULL;
    size_t offset_count = 0u;
    size_t entry;

    if (!bytes || !span || span->source_entry_bytes == 0u) return 0;
    switch (span->database_number) {
    case 0u: offsets = db0; offset_count = sizeof(db0) / sizeof(db0[0]); break;
    case 1u: offsets = db1; offset_count = sizeof(db1) / sizeof(db1[0]); break;
    case 2u: offsets = db2; offset_count = sizeof(db2) / sizeof(db2[0]); break;
    case 3u: offsets = db3; offset_count = sizeof(db3) / sizeof(db3[0]); break;
    case 4u: offsets = db4; offset_count = sizeof(db4) / sizeof(db4[0]); break;
    case 5u:
    case 6u:
    case 8u:
    case 10u:
    case 15u: offsets = db5; offset_count = sizeof(db5) / sizeof(db5[0]); break;
    case 7u: offsets = db7; offset_count = sizeof(db7) / sizeof(db7[0]); break;
    case 9u: offsets = db9; offset_count = sizeof(db9) / sizeof(db9[0]); break;
    case 11u:
        if (span->source_entry_bytes != 256u) return 0;
        for (entry = 0u; entry < span->entry_count; ++entry) {
            uint8_t *record = bytes + span->offset +
                entry * span->source_entry_bytes;
            size_t word;
            swap16(record + 0u);
            swap16(record + 2u);
            for (word = 4u; word < 256u; word += 4u) swap32(record + word);
        }
        return 1;
    case 12u:
    case 13u:
        return span->source_entry_bytes == 2u;
    case 14u: offsets = db14; offset_count = sizeof(db14) / sizeof(db14[0]); break;
    default: return 0;
    }
    for (entry = 0u; entry < span->entry_count; ++entry) {
        uint8_t *record = bytes + span->offset + entry * span->source_entry_bytes;
        size_t index;
        for (index = 0u; index < offset_count; ++index) {
            if (offsets[index] + 2u > span->source_entry_bytes) return 0;
        }
        swap_record_words(record, offsets, offset_count);
    }
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

int csb_v1_csbwin_dungeon_tail_load_legacy_source_dungeon(
    const uint8_t *tail,
    size_t tail_size,
    const CSB_V1_CSBWinDungeonTailPrefix *prefix,
    const CSB_V1_CSBWinDungeonTailDatabaseLayout *databases,
    CSB_V1_DungeonData *out)
{
    uint8_t *normalized;
    size_t level;
    size_t column;
    size_t object;
    size_t database;
    int rc;

    if (!tail || !prefix || !databases || !out || !prefix->valid ||
        !databases->valid || prefix->indirect_text ||
        databases->extended_features_version !=
            CSB_V1_CSBWIN_LEGACY_FEATURE_VERSION ||
        databases->big_actuators || databases->legacy_scroll_records == 0 ||
        databases->checksum_offset + 2u != tail_size ||
        databases->checksum_offset > tail_size ||
        databases->cell_flags_offset > databases->checksum_offset ||
        csb_v1_csbwin_dungeon_tail_validate_checksum(tail, tail_size,
                                                      NULL, NULL) != 1) {
        if (out) memset(out, 0, sizeof(*out));
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }
    if (prefix->level_count > CSB_V1_MAX_LEVELS ||
        prefix->object_list_index_offset > databases->database_offset ||
        prefix->object_list_offset > databases->database_offset ||
        prefix->text_offset > databases->database_offset) {
        memset(out, 0, sizeof(*out));
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }

    /* Do not copy the terminal WriteAndChecksum word into DUNGEON.DAT.
     * CSBWin ReadDatabases() consumes it after all source structures. */
    normalized = (uint8_t *)malloc(databases->checksum_offset);
    if (!normalized) {
        memset(out, 0, sizeof(*out));
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    memcpy(normalized, tail, databases->checksum_offset);

    /* DUNGEONDATINDEX::Swap() (data.cpp:1778-1790). */
    /* word4's high byte is NumLevel.  ReDMCSB/Firestaff's compressed-source
     * ingress deliberately leaves bytes 4..5 unswapped for that byte owner;
     * mirror csb_swap_big_endian_dungeon_words(), rather than treating every
     * DUNGEONDATINDEX field as a host u16. */
    for (column = 0u; column < CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES;
         column += 2u) {
        if (column == 4u) continue;
        swap16(normalized + column);
    }
    /* SaveGame.cpp swapLevelDescriptors() only converts words 0/8/10/12/14;
     * bytes 4..7 remain byte fields in LEVELDESC. */
    for (level = 0u; level < prefix->level_count; ++level) {
        uint8_t *descriptor = normalized + prefix->level_descriptors_offset +
            level * CSB_V1_CSBWIN_LEVEL_DESC_BYTES;
        swap16(descriptor + 0u);
        swap16(descriptor + 8u);
        swap16(descriptor + 10u);
        swap16(descriptor + 12u);
        swap16(descriptor + 14u);
    }
    /* SaveGame.cpp swapPointer10454()/swapPRN10464(). */
    for (column = 0u; column < prefix->column_pointer_count; ++column) {
        swap16(normalized + prefix->object_list_index_offset + column * 2u);
    }
    for (object = 0u; object < prefix->object_list_length; ++object) {
        swap16(normalized + prefix->object_list_offset + object * 2u);
    }
    /* Legacy text is a packed byte stream. ReadDatabases() does not swap it
     * before ConvertToIndirectText(), so neither do we. */
    for (database = 0u; database < CSB_V1_CSBWIN_DATABASE_COUNT; ++database) {
        if (!swap_legacy_database(normalized, &databases->database[database])) {
            free(normalized);
            memset(out, 0, sizeof(*out));
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
        }
    }

    rc = csb_v1_dungeon_load_source_bytes(out, normalized,
                                           (int)databases->checksum_offset);
    free(normalized);
    if (rc != 0) {
        memset(out, 0, sizeof(*out));
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }
    return CSB_V1_CSBWIN_DUNGEON_TAIL_OK;
}

int csb_v1_csbwin_dungeon_tail_prepare_legacy_candidate(
    const uint8_t *tail, size_t tail_size,
    CSB_V1_CSBWinLegacyDungeonCandidate **out_candidate)
{
    CSB_V1_CSBWinLegacyDungeonCandidate *candidate;
    int result;

    if (!tail || !out_candidate) return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT;
    *out_candidate = NULL;
    candidate = (CSB_V1_CSBWinLegacyDungeonCandidate *)calloc(
        1u, sizeof(*candidate));
    if (!candidate) return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    result = csb_v1_csbwin_dungeon_tail_parse_prefix(
        tail, tail_size, 0u, &candidate->prefix);
    if (result == CSB_V1_CSBWIN_DUNGEON_TAIL_OK) {
        result = csb_v1_csbwin_dungeon_tail_parse_databases(
            tail, tail_size, &candidate->prefix,
            CSB_V1_CSBWIN_LEGACY_FEATURE_VERSION, 0u, 0u,
            &candidate->databases);
    }
    if (result == CSB_V1_CSBWIN_DUNGEON_TAIL_OK) {
        result = csb_v1_csbwin_dungeon_tail_load_legacy_source_dungeon(
            tail, tail_size, &candidate->prefix, &candidate->databases,
            &candidate->dungeon);
    }
    if (result != CSB_V1_CSBWIN_DUNGEON_TAIL_OK) {
        csb_v1_dungeon_free(&candidate->dungeon);
        free(candidate);
        return result;
    }
    *out_candidate = candidate;
    return CSB_V1_CSBWIN_DUNGEON_TAIL_OK;
}

const CSB_V1_DungeonData *csb_v1_csbwin_dungeon_tail_candidate_dungeon(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate)
{
    return candidate ? &candidate->dungeon : NULL;
}

const CSB_V1_CSBWinDungeonTailPrefix
    *csb_v1_csbwin_dungeon_tail_candidate_prefix(
        const CSB_V1_CSBWinLegacyDungeonCandidate *candidate)
{
    return candidate ? &candidate->prefix : NULL;
}

const CSB_V1_CSBWinDungeonTailDatabaseLayout
    *csb_v1_csbwin_dungeon_tail_candidate_databases(
        const CSB_V1_CSBWinLegacyDungeonCandidate *candidate)
{
    return candidate ? &candidate->databases : NULL;
}

int csb_v1_csbwin_dungeon_tail_candidate_validate_resume_shape(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate,
    uint16_t party_level, uint16_t party_x, uint16_t party_y,
    uint16_t party_facing, const uint16_t *item16_monster_indices,
    size_t item16_count)
{
    const CSB_V1_DungeonData *dungeon;
    size_t i;

    if (!candidate || (item16_count != 0u && !item16_monster_indices)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT;
    }
    dungeon = &candidate->dungeon;
    if (!dungeon->raw_data || party_level >= (uint16_t)dungeon->level_count ||
        party_x >= (uint16_t)dungeon->level_widths[party_level] ||
        party_y >= (uint16_t)dungeon->level_heights[party_level] ||
        party_facing > 3u || dungeon->thing_type_counts[4] < 0) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }
    for (i = 0u; i < item16_count; ++i) {
        /* CSBWin represents an unused ITEM16::word0 as signed -1. */
        if (item16_monster_indices[i] == UINT16_MAX) continue;
        if (item16_monster_indices[i] >=
            (uint16_t)dungeon->thing_type_counts[4]) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
        }
    }
    return CSB_V1_CSBWIN_DUNGEON_TAIL_OK;
}

void csb_v1_csbwin_dungeon_tail_discard_legacy_candidate(
    CSB_V1_CSBWinLegacyDungeonCandidate *candidate)
{
    if (!candidate) return;
    csb_v1_dungeon_free(&candidate->dungeon);
    free(candidate);
}

const char *csb_v1_csbwin_dungeon_tail_source_evidence(void)
{
    return "CSBWin SaveGame.cpp:1236-1337,2285-2411,2536-2896; "
           "CSBWin SaveGame.cpp:533-545,619-632; "
           "CSBWin CSB.h:DUNGEONDATINDEX,LEVELDESC,DB0-DB15; "
           "CSBWin data.cpp:1166-1185,1287-1495,1778-1790";
}
