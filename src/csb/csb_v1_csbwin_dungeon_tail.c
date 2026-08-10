#include "csb_v1_csbwin_dungeon_tail.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CSB_V1_CSBWinLegacyDungeonCandidate {
    CSB_V1_DungeonData dungeon;
    CSB_V1_CSBWinDungeonTailPrefix prefix;
    CSB_V1_CSBWinDungeonTailDatabaseLayout databases;
    uint8_t *source_tail;
    size_t source_tail_size;
    uint64_t source_tail_signature;
};

/* FNV-1a is solely a stable, compact diagnostic identity.  Admission always
 * uses the retained source bytes and memcmp(), so this is not used as a
 * security or ownership decision. */
static uint64_t source_tail_signature(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t i;

    if (!bytes && size != 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        value ^= (uint64_t)bytes[i];
        value *= UINT64_C(1099511628211);
    }
    return value;
}

static uint32_t source_tail_fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t value = UINT32_C(2166136261);
    size_t i;

    if (!bytes && size != 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        value ^= (uint32_t)bytes[i];
        value *= UINT32_C(16777619);
    }
    return value;
}

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
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
    if (result == CSB_V1_CSBWIN_DUNGEON_TAIL_OK) {
        /* Retain exact source provenance.  The decoded dungeon is normalized
         * for M10, so a future atomic handoff must never infer its saved-tail
         * identity from those host-endian bytes. */
        candidate->source_tail = (uint8_t *)malloc(tail_size);
        if (!candidate->source_tail) {
            result = CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
        } else {
            memcpy(candidate->source_tail, tail, tail_size);
            candidate->source_tail_size = tail_size;
            candidate->source_tail_signature =
                source_tail_signature(tail, tail_size);
        }
    }
    if (result != CSB_V1_CSBWIN_DUNGEON_TAIL_OK) {
        csb_v1_dungeon_free(&candidate->dungeon);
        free(candidate->source_tail);
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

int csb_v1_csbwin_dungeon_tail_candidate_identity(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate,
    CSB_V1_CSBWinLegacyDungeonCandidateIdentity *out)
{
    CSB_V1_CSBWinLegacyDungeonCandidateIdentity identity;
    size_t database;

    if (!candidate || !out || !candidate->dungeon.raw_data ||
        !candidate->prefix.valid || !candidate->databases.valid ||
        !candidate->source_tail || candidate->source_tail_size == 0u) {
        return 0;
    }
    memset(&identity, 0, sizeof(identity));
    identity.valid = 1;
    identity.source_tail_size = candidate->source_tail_size;
    identity.source_tail_signature = candidate->source_tail_signature;
    identity.source_tail_checksum = candidate->databases.stored_checksum;
    identity.level_count = candidate->prefix.level_count;
    for (database = 0u; database < CSB_V1_CSBWIN_DATABASE_COUNT; ++database) {
        const uint16_t entries = candidate->prefix.database_entries[database];
        if ((uint32_t)entries > UINT32_MAX - identity.database_entry_count) {
            return 0;
        }
        identity.database_entry_count =
            identity.database_entry_count + (uint32_t)entries;
    }
    *out = identity;
    return 1;
}

int csb_v1_csbwin_dungeon_tail_candidate_matches_source_tail(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate,
    const uint8_t *tail, size_t tail_size)
{
    if (!candidate || !tail || !candidate->source_tail ||
        candidate->source_tail_size != tail_size || tail_size == 0u ||
        candidate->source_tail_signature !=
            source_tail_signature(tail, tail_size)) {
        return 0;
    }
    return memcmp(candidate->source_tail, tail, tail_size) == 0;
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

int csb_v1_csbwin_dungeon_tail_candidate_validate_resume_timers(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate,
    const CSB_V1_CSBWin512BodyReport *body)
{
    uint8_t seen[CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES] = { 0 };
    uint16_t queue_index;
    size_t timer_raw_size;
    size_t queue_raw_size;

    if (!candidate || !body || !candidate->dungeon.raw_data ||
        !body->header_valid ||
        body->timer_summary_count != body->timer_summary_total ||
        body->timer_queue_summary_count != body->timer_queue_summary_total ||
        body->max_timers != body->timer_summary_count ||
        body->timer_summary_count > CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES ||
        body->timer_queue_summary_count >
            CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES ||
        body->num_timer > body->timer_queue_summary_count ||
        body->num_timer > body->max_timers ||
        body->first_avail_timer > body->max_timers) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }
    if ((body->timer_record_size != 10u && body->timer_record_size != 12u &&
         body->timer_record_size != 16u) ||
        !multiply_size((size_t)body->max_timers,
                       (size_t)body->timer_record_size, &timer_raw_size) ||
        !multiply_size((size_t)body->max_timers, 2u, &queue_raw_size) ||
        body->timer_raw_size != timer_raw_size ||
        body->timer_queue_raw_size != queue_raw_size ||
        (body->header.byte_order != CSB_V1_CSBWIN_512_BYTE_ORDER_LITTLE_ENDIAN &&
         body->header.byte_order != CSB_V1_CSBWIN_512_BYTE_ORDER_BIG_ENDIAN)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }
    for (queue_index = 0u; queue_index < body->num_timer; ++queue_index) {
        const uint16_t timer_index = body->timer_queue[queue_index];
        const CSB_V1_CSBWin512TimerSummary *timer;
        const uint8_t *timer_raw;
        uint16_t raw_queue_index;

        if (timer_index >= body->timer_summary_count || seen[timer_index]) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
        }
        timer = &body->timers[timer_index];
        if (!timer->valid || timer->truncated ||
            timer->function == 0u || timer->source_index != timer_index) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
        }
        /* SaveGame.cpp authenticates MaxTimers fixed 10/12/16-byte TIMER
         * storage and MaxTimers 16-bit queue handles separately before it
         * calls swapTimerQue()/swapTimers().  Bind the parsed live event
         * back to those retained raw records here; this is an integrity
         * check, not an attempt to reconstruct TIMER::operator<. */
        raw_queue_index = body->header.byte_order ==
                CSB_V1_CSBWIN_512_BYTE_ORDER_BIG_ENDIAN
            ? read_be16(body->timer_queue_raw + (size_t)queue_index * 2u)
            : read_le16(body->timer_queue_raw + (size_t)queue_index * 2u);
        timer_raw = body->timer_raw +
            (size_t)timer_index * (size_t)body->timer_record_size;
        if (raw_queue_index != timer_index ||
            read_le32(timer_raw) != timer->time ||
            timer_raw[4] != timer->function ||
            timer_raw[5] != timer->ubyte5 ||
            timer_raw[6] != timer->ubyte6 ||
            timer_raw[7] != timer->ubyte7 ||
            timer_raw[8] != timer->ubyte8 ||
            timer_raw[9] != timer->ubyte9 ||
            (body->timer_record_size >= 12u &&
             read_le16(timer_raw + 10u) != timer->sequence) ||
            (body->timer_record_size >= 13u &&
             timer_raw[12] != timer->level)) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
        }
        seen[timer_index] = 1u;
    }
    return CSB_V1_CSBWIN_DUNGEON_TAIL_OK;
}

int csb_v1_csbwin_dungeon_tail_prepare_legacy_resume(
    const CSB_V1_CSBWinLegacyDungeonCandidate *candidate,
    const CSB_V1_CSBWin512BodyReport *body,
    const uint8_t *source_tail, size_t source_tail_size,
    CSB_V1_CSBWinLegacyResumePrepare *out)
{
    CSB_V1_CSBWinLegacyResumePrepare receipt;
    uint16_t item16_indices[CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES];
    size_t i;

    if (!candidate || !body || !source_tail || !out) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT;
    }
    /* The verifier may retain an appended tail only while it fits its fixed
     * evidence buffer.  Refuse a partial copy rather than matching a prefix
     * of a dungeon which a later transaction could mistake for whole. */
    if (!body->header_valid || body->appended_size == 0u ||
        source_tail_size != body->appended_size ||
        body->item16_summary_count != body->item16_summary_total ||
        body->item16_summary_total > CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES ||
        !csb_v1_csbwin_dungeon_tail_candidate_matches_source_tail(
            candidate, source_tail, source_tail_size) ||
        source_tail_fnv1a32(source_tail, source_tail_size) !=
            body->appended_fnv1a ||
        (!body->appended_truncated &&
         (body->appended_preserved_size != source_tail_size ||
          memcmp(body->appended_preserved, source_tail,
                 source_tail_size) != 0))) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }
    for (i = 0u; i < body->item16_summary_total; ++i) {
        if (!body->item16[i].valid || body->item16[i].truncated) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
        }
        item16_indices[i] = body->item16[i].monster_index;
    }
    if (csb_v1_csbwin_dungeon_tail_candidate_validate_resume_shape(
            candidate, body->party_level, body->party_x, body->party_y,
            body->party_facing, item16_indices,
            body->item16_summary_total) != CSB_V1_CSBWIN_DUNGEON_TAIL_OK ||
        csb_v1_csbwin_dungeon_tail_candidate_validate_resume_timers(
            candidate, body) != CSB_V1_CSBWIN_DUNGEON_TAIL_OK) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }
    memset(&receipt, 0, sizeof(receipt));
    /* Re-read identity after zeroing the local receipt; no partially valid
     * receipt can leave this function. */
    if (!csb_v1_csbwin_dungeon_tail_candidate_identity(
            candidate, &receipt.candidate_identity)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }
    receipt.valid = 1;
    receipt.source_body_appended_fnv1a = body->appended_fnv1a;
    receipt.game_time = body->game_time;
    receipt.random_seed = body->random_seed;
    receipt.party_level = body->party_level;
    receipt.party_x = body->party_x;
    receipt.party_y = body->party_y;
    receipt.party_facing = body->party_facing;
    receipt.object_in_hand = body->object_in_hand;
    receipt.hand_char = body->hand_char;
    receipt.magic_caster = body->magic_caster;
    receipt.num_character = body->num_character;
    receipt.item16_count = body->item16_summary_total;
    receipt.max_timers = body->max_timers;
    receipt.num_timer = body->num_timer;
    receipt.first_avail_timer = body->first_avail_timer;
    receipt.timer_sequence = body->timer_sequence;
    receipt.timer_record_size = body->timer_record_size;
    receipt.timer_raw_size = body->timer_raw_size;
    receipt.timer_raw_fnv1a = body->timer_raw_fnv1a;
    receipt.timer_queue_raw_size = body->timer_queue_raw_size;
    receipt.timer_queue_raw_fnv1a = body->timer_queue_raw_fnv1a;
    *out = receipt;
    return CSB_V1_CSBWIN_DUNGEON_TAIL_OK;
}

int csb_v1_csbwin_dungeon_tail_prepare_legacy_resume_file(
    const char *path, size_t max_size,
    CSB_V1_CSBWinLegacyDungeonCandidate **out_candidate,
    CSB_V1_CSBWinLegacyResumePrepare *out_receipt)
{
    enum { DEFAULT_MAX_BYTES = 4 * 1024 * 1024 };
    FILE *file;
    long file_size_long;
    size_t file_size;
    uint8_t *bytes = NULL;
    CSB_V1_CSBWinExtendedFeaturesReport features;
    CSB_V1_CSBWinExtendedDSAReport dsa;
    CSB_V1_CSBWinExtendedTailReport extended_tail;
    CSB_V1_CSBWin512BodyReport body;
    CSB_V1_CSBWinLegacyDungeonCandidate *candidate = NULL;
    CSB_V1_CSBWinLegacyResumePrepare receipt;
    int rc;

    if (!path || !path[0] || !out_candidate || !out_receipt) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT;
    }
    if (max_size == 0u) max_size = (size_t)DEFAULT_MAX_BYTES;
    file = fopen(path, "rb");
    if (!file) return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_IO;
    if (fseek(file, 0L, SEEK_END) != 0 ||
        (file_size_long = ftell(file)) < 0 ||
        (file_size = (size_t)file_size_long) > max_size ||
        file_size == 0u || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    bytes = (uint8_t *)malloc(file_size);
    if (!bytes) {
        fclose(file);
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_OVERFLOW;
    }
    if (fread(bytes, 1u, file_size, file) != file_size) {
        fclose(file);
        free(bytes);
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    fclose(file);

    memset(&features, 0, sizeof(features));
    memset(&dsa, 0, sizeof(dsa));
    memset(&extended_tail, 0, sizeof(extended_tail));
    /* This adapter is specifically for the old CSBGAME2-style save body.
     * Any extended header has its own DSA/global ownership and must wait for
     * a separate complete transaction rather than being treated as legacy. */
    if (csb_v1_csbwin_512_inspect_extended_tail(
            bytes, file_size, &extended_tail, &dsa, &features) !=
        CSB_V1_CSBWIN_EXTENDED_ABSENT) {
        free(bytes);
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }
    memset(&body, 0, sizeof(body));
    if (csb_v1_csbwin_512_verify_save_body_legacy_layouts(
            bytes, file_size, &body) != CSB_V1_CSBWIN_512_OK ||
        csb_v1_csbwin_512_validate_appended_expool_tail(&body) ||
        body.appended_size == 0u || body.appended_offset > file_size ||
        body.appended_size > file_size - body.appended_offset) {
        free(bytes);
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT;
    }
    rc = csb_v1_csbwin_dungeon_tail_prepare_legacy_candidate(
        bytes + body.appended_offset, body.appended_size, &candidate);
    if (rc == CSB_V1_CSBWIN_DUNGEON_TAIL_OK) {
        rc = csb_v1_csbwin_dungeon_tail_prepare_legacy_resume(
            candidate, &body, bytes + body.appended_offset,
            body.appended_size, &receipt);
    }
    free(bytes);
    if (rc != CSB_V1_CSBWIN_DUNGEON_TAIL_OK) {
        csb_v1_csbwin_dungeon_tail_discard_legacy_candidate(candidate);
        return rc;
    }
    *out_candidate = candidate;
    *out_receipt = receipt;
    return CSB_V1_CSBWIN_DUNGEON_TAIL_OK;
}

void csb_v1_csbwin_dungeon_tail_discard_legacy_candidate(
    CSB_V1_CSBWinLegacyDungeonCandidate *candidate)
{
    if (!candidate) return;
    csb_v1_dungeon_free(&candidate->dungeon);
    free(candidate->source_tail);
    free(candidate);
}

const char *csb_v1_csbwin_dungeon_tail_source_evidence(void)
{
    return "CSBWin SaveGame.cpp:1236-1337,2285-2411,2536-2896; "
           "CSBWin SaveGame.cpp:533-545,619-632; "
           "CSBWin CSB.h:DUNGEONDATINDEX,LEVELDESC,DB0-DB15; "
           "CSBWin data.cpp:1166-1185,1287-1495,1778-1790";
}
