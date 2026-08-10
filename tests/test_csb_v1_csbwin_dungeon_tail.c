#include "csb_v1_csbwin_dungeon_tail.h"
#include "csb_v1_csbwin_512_xor_pad_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(expr) do { if (!(expr)) { ++failures; \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); } } while (0)

static void put_be16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8u);
    p[1] = (uint8_t)value;
}

static void put_le32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8u);
    p[2] = (uint8_t)(value >> 16u);
    p[3] = (uint8_t)(value >> 24u);
}

static size_t build_database_tail(
    uint8_t *tail,
    size_t capacity,
    uint8_t version,
    uint8_t flags,
    uint16_t legacy_cell_flag_bytes,
    uint32_t extended_cell_flag_bytes)
{
    static const uint16_t base_entry_bytes[CSB_V1_CSBWIN_DATABASE_COUNT] = {
        4u, 6u, 4u, 10u, 16u, 4u, 4u, 6u,
        4u, 8u, 4u, 256u, 2u, 2u, 8u, 4u
    };
    CSB_V1_CSBWinDungeonTailPrefix prefix;
    size_t offset;
    size_t i;
    size_t cell_bytes = extended_cell_flag_bytes != 0u
        ? (size_t)extended_cell_flag_bytes
        : (size_t)legacy_cell_flag_bytes;
    uint16_t checksum = 0u;

    memset(tail, 0, capacity);
    put_be16(tail + 0u, 13u);
    put_be16(tail + 2u, legacy_cell_flag_bytes);
    put_be16(tail + 4u, 0x0100u);
    for (i = 0u; i < CSB_V1_CSBWIN_DATABASE_COUNT; ++i) {
        put_be16(tail + 12u + i * 2u, 1u);
    }
    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(
              tail, capacity, flags, &prefix) ==
          CSB_V1_CSBWIN_DUNGEON_TAIL_OK);
    offset = prefix.next_database_offset;
    for (i = 0u; i < CSB_V1_CSBWIN_DATABASE_COUNT; ++i) {
        size_t entry_bytes = base_entry_bytes[i];
        size_t j;
        if (i == 3u &&
            (flags & CSB_V1_CSBWIN_EXTENDED_FLAG_BIG_ACTUATORS) == 0u) {
            entry_bytes = 8u;
        }
        if (i == 7u && version < (uint8_t)'B') entry_bytes = 4u;
        CHECK(offset + entry_bytes <= capacity);
        for (j = 0u; j < entry_bytes && offset + j < capacity; ++j) {
            tail[offset + j] = (uint8_t)(i + 1u);
        }
        offset += entry_bytes;
    }
    CHECK(offset + cell_bytes + 2u <= capacity);
    for (i = 0u; i < cell_bytes && offset + i < capacity; ++i) {
        tail[offset + i] = (uint8_t)(0xa0u + i);
    }
    offset += cell_bytes;
    for (i = 0u; i < offset; ++i) checksum = (uint16_t)(checksum + tail[i]);
    put_be16(tail + offset, checksum);
    return offset + 2u;
}

static void check_staged_real_save(void)
{
    const char *path = getenv("FIRESTAFF_CSBWIN_REAL_SAVE");
    FILE *file;
    long length;
    uint8_t *bytes;
    CSB_V1_CSBWin512BodyReport body;
    CSB_V1_CSBWinDungeonTailPrefix prefix;
    CSB_V1_CSBWinDungeonTailDatabaseLayout databases;
    CSB_V1_DungeonData dungeon;
    CSB_V1_DungeonData rejected;
    CSB_V1_CSBWinLegacyDungeonCandidate *candidate = NULL;
    const CSB_V1_DungeonData *current_before = csb_v1_dungeon_get_current();
    uint16_t *item16_indices = NULL;
    uint8_t *before = NULL;
    uint16_t computed;
    uint16_t stored;

    if (!path || path[0] == '\0') {
        puts("SKIP: FIRESTAFF_CSBWIN_REAL_SAVE is not staged");
        return;
    }
    file = fopen(path, "rb");
    CHECK(file != NULL);
    if (!file) return;
    CHECK(fseek(file, 0L, SEEK_END) == 0);
    length = ftell(file);
    CHECK(length > 0L);
    CHECK(fseek(file, 0L, SEEK_SET) == 0);
    if (length <= 0L || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return;
    }
    bytes = (uint8_t *)malloc((size_t)length);
    CHECK(bytes != NULL);
    if (!bytes) {
        fclose(file);
        return;
    }
    CHECK(fread(bytes, 1u, (size_t)length, file) == (size_t)length);
    fclose(file);
    memset(&body, 0, sizeof(body));
    CHECK(csb_v1_csbwin_512_verify_save_body(bytes, (size_t)length, 10u, &body) ==
          CSB_V1_CSBWIN_512_OK);
    CHECK(body.appended_size > CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(
              bytes + body.appended_offset, body.appended_size, 0u, &prefix) == 0);
    CHECK(prefix.valid && prefix.level_count > 0u);
    CHECK(prefix.next_database_offset < body.appended_size);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_databases(
              bytes + body.appended_offset, body.appended_size, &prefix,
              CSB_V1_CSBWIN_LEGACY_FEATURE_VERSION, 0u, 0u, &databases) ==
          CSB_V1_CSBWIN_DUNGEON_TAIL_OK);
    CHECK(databases.valid);
    CHECK(databases.database[0].entry_count == prefix.database_entries[0]);
    CHECK(databases.database[15].entry_count == prefix.database_entries[15]);
    CHECK(databases.checksum_offset + 2u == body.appended_size);
    CHECK(csb_v1_csbwin_dungeon_tail_validate_checksum(
              bytes + body.appended_offset,
              body.appended_size, &computed, &stored) == 1);
    CHECK(computed == stored);
    before = (uint8_t *)malloc(body.appended_size);
    CHECK(before != NULL);
    if (before) {
        memcpy(before, bytes + body.appended_offset,
               body.appended_size);
        memset(&dungeon, 0, sizeof(dungeon));
        CHECK(csb_v1_csbwin_dungeon_tail_load_legacy_source_dungeon(
                  bytes + body.appended_offset,
                  body.appended_size, &prefix, &databases, &dungeon) ==
              CSB_V1_CSBWIN_DUNGEON_TAIL_OK);
        CHECK(dungeon.raw_data != NULL && dungeon.level_count == prefix.level_count &&
              dungeon.square_bytes == 1);
        CHECK(memcmp(before,
                     bytes + body.appended_offset,
                     body.appended_size) == 0);
        csb_v1_dungeon_free(&dungeon);
        CHECK(csb_v1_csbwin_dungeon_tail_prepare_legacy_candidate(
                  bytes + body.appended_offset, body.appended_size,
                  &candidate) == CSB_V1_CSBWIN_DUNGEON_TAIL_OK);
        CHECK(candidate != NULL &&
              csb_v1_csbwin_dungeon_tail_candidate_dungeon(candidate) != NULL &&
              csb_v1_csbwin_dungeon_tail_candidate_dungeon(candidate)->level_count ==
                  prefix.level_count &&
              csb_v1_csbwin_dungeon_tail_candidate_prefix(candidate)->valid &&
              csb_v1_csbwin_dungeon_tail_candidate_databases(candidate)->valid &&
              memcmp(before, bytes + body.appended_offset,
                     body.appended_size) == 0);
        CHECK(body.item16_summary_count == body.item16_summary_total);
        if (body.item16_summary_total != 0u &&
            body.item16_summary_count == body.item16_summary_total) {
            size_t i;
            item16_indices = (uint16_t *)malloc(
                (size_t)body.item16_summary_total * sizeof(*item16_indices));
            CHECK(item16_indices != NULL);
            if (item16_indices) {
                for (i = 0u; i < body.item16_summary_total; ++i) {
                    item16_indices[i] = body.item16[i].monster_index;
                }
            }
        }
        if (body.item16_summary_count == body.item16_summary_total &&
            (body.item16_summary_total == 0u || item16_indices != NULL)) {
            CHECK(csb_v1_csbwin_dungeon_tail_candidate_validate_resume_shape(
                      candidate, body.party_level, body.party_x, body.party_y,
                      body.party_facing, item16_indices,
                      body.item16_summary_total) ==
                  CSB_V1_CSBWIN_DUNGEON_TAIL_OK);
            CHECK(csb_v1_csbwin_dungeon_tail_candidate_validate_resume_shape(
                      candidate, body.party_level, body.party_x, body.party_y,
                      4u, item16_indices, body.item16_summary_total) ==
                  CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT);
            CHECK(csb_v1_csbwin_dungeon_tail_candidate_validate_resume_timers(
                      candidate, &body) == CSB_V1_CSBWIN_DUNGEON_TAIL_OK);
            if (body.num_timer > 1u) {
                CSB_V1_CSBWin512BodyReport malformed = body;
                malformed.timer_queue[1] = malformed.timer_queue[0];
                CHECK(csb_v1_csbwin_dungeon_tail_candidate_validate_resume_timers(
                          candidate, &malformed) ==
                      CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT);
            }
            if (body.num_timer > 0u) {
                CSB_V1_CSBWin512BodyReport malformed = body;
                const uint16_t timer_index = malformed.timer_queue[0];
                malformed.timer_raw[(size_t)timer_index *
                    malformed.timer_record_size + 4u] ^= 1u;
                CHECK(csb_v1_csbwin_dungeon_tail_candidate_validate_resume_timers(
                          candidate, &malformed) ==
                      CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT);
            }
        }
        CHECK(csb_v1_dungeon_get_current() == current_before &&
              csb_v1_dungeon_get_current_mutable() == current_before);
        csb_v1_csbwin_dungeon_tail_discard_legacy_candidate(candidate);
        candidate = NULL;
        free(item16_indices);
        item16_indices = NULL;
        memset(&rejected, 0xa5, sizeof(rejected));
        before[body.appended_size - 1u] ^= 1u;
        CHECK(csb_v1_csbwin_dungeon_tail_load_legacy_source_dungeon(
                  before, body.appended_size, &prefix, &databases,
                  &rejected) == CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT);
        CHECK(rejected.raw_data == NULL && rejected.level_count == 0);
        free(before);
    }
    csb_v1_csbwin_dungeon_tail_discard_legacy_candidate(candidate);
    free(item16_indices);
    free(bytes);
}

int main(void)
{
    uint8_t tail[512];
    const uint8_t checksum_good[] = { 1u, 2u, 3u, 0u, 6u };
    const uint8_t checksum_bad[] = { 1u, 2u, 3u, 0u, 7u };
    CSB_V1_CSBWinDungeonTailPrefix report;
    CSB_V1_CSBWinDungeonTailPrefix database_prefix;
    CSB_V1_CSBWinDungeonTailDatabaseLayout databases;
    CSB_V1_CSBWinDungeonTailDatabaseLayout unchanged;
    CSB_V1_CSBWinLegacyDungeonCandidate *candidate =
        (CSB_V1_CSBWinLegacyDungeonCandidate *)(uintptr_t)1u;
    const size_t levels = 2u;
    const size_t descriptors = CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES +
        levels * CSB_V1_CSBWIN_LEVEL_DESC_BYTES;
    const size_t indirect_text_offset = descriptors + 12u + 10u;
    size_t database_tail_size;
    size_t legacy_database_tail_size;

    memset(tail, 0, sizeof(tail));
    put_be16(tail + 0u, 13u);
    put_be16(tail + 2u, 128u);
    put_be16(tail + 4u, 0x0200u);
    put_be16(tail + 6u, 3u);
    put_be16(tail + 10u, 5u);
    put_be16(tail + 12u, 4u);
    put_be16(tail + descriptors - 32u + 8u, (uint16_t)(3u << 6u));
    put_be16(tail + descriptors - 16u + 8u, (uint16_t)(1u << 6u));
    put_le32(tail + indirect_text_offset + 12u, 2u);

    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(
              tail, sizeof(tail),
              CSB_V1_CSBWIN_EXTENDED_FLAG_INDIRECT_TEXT, &report) == 0);
    CHECK(report.valid);
    CHECK(report.sentinel == 13u);
    CHECK(report.level_count == 2u);
    CHECK(report.text_word_count == 3u);
    CHECK(report.object_list_length == 5u);
    CHECK(report.database_entries[0] == 4u);
    CHECK(report.level_last_column[0] == 3u);
    CHECK(report.level_last_column[1] == 1u);
    CHECK(report.column_pointer_count == 6u);
    CHECK(report.level_descriptors_offset == 44u);
    CHECK(report.object_list_index_offset == descriptors);
    CHECK(report.object_list_offset == descriptors + 12u);
    CHECK(report.text_offset == descriptors + 12u + 10u);
    CHECK(report.compressed_text_size_offset == indirect_text_offset + 12u);
    CHECK(report.compressed_text_word_count == 2u);
    CHECK(report.compressed_text_offset == indirect_text_offset + 16u);
    CHECK(report.next_database_offset == indirect_text_offset + 20u);
    CHECK(report.indirect_text);
    put_le32(tail + indirect_text_offset + 12u,
             CSB_V1_CSBWIN_MAX_COMPRESSED_TEXT_WORDS + 1u);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(
              tail, sizeof(tail),
              CSB_V1_CSBWIN_EXTENDED_FLAG_INDIRECT_TEXT, &report) ==
          CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TEXT_SIZE);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(tail, 43u, 0u, &report) ==
          CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED);
    CHECK(csb_v1_csbwin_dungeon_tail_validate_checksum(
              checksum_good, sizeof(checksum_good), NULL, NULL) == 1);
    CHECK(csb_v1_csbwin_dungeon_tail_validate_checksum(
              checksum_bad, sizeof(checksum_bad), NULL, NULL) == 0);
    CHECK(csb_v1_csbwin_dungeon_tail_validate_checksum(NULL, 0u, NULL, NULL) == -1);
    CHECK(csb_v1_csbwin_dungeon_tail_prepare_legacy_candidate(
              NULL, 0u, &candidate) == CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT &&
          candidate == (CSB_V1_CSBWinLegacyDungeonCandidate *)(uintptr_t)1u);
    put_be16(tail + 4u, 0u);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(tail, sizeof(tail), 0u,
              &report) == CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LEVEL_COUNT);

    database_tail_size = build_database_tail(
        tail, sizeof(tail), (uint8_t)'B',
        CSB_V1_CSBWIN_EXTENDED_FLAG_BIG_ACTUATORS, 7u, 3u);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(
              tail, database_tail_size,
              CSB_V1_CSBWIN_EXTENDED_FLAG_BIG_ACTUATORS,
              &database_prefix) == CSB_V1_CSBWIN_DUNGEON_TAIL_OK);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_databases(
              tail, database_tail_size, &database_prefix, (uint8_t)'B',
              CSB_V1_CSBWIN_EXTENDED_FLAG_BIG_ACTUATORS, 3u,
              &databases) == CSB_V1_CSBWIN_DUNGEON_TAIL_OK);
    CHECK(databases.valid);
    CHECK(databases.big_actuators);
    CHECK(!databases.legacy_scroll_records);
    CHECK(databases.cell_flag_size_from_extended_features);
    CHECK(databases.database[3].source_entry_bytes == 10u);
    CHECK(databases.database[7].source_entry_bytes == 6u);
    CHECK(databases.database[11].source_entry_bytes == 256u);
    CHECK(databases.database[0].offset == database_prefix.next_database_offset);
    CHECK(databases.database[15].offset +
          databases.database[15].byte_count == databases.cell_flags_offset);
    CHECK(databases.cell_flag_bytes == 3u);
    CHECK(databases.checksum_offset + 2u == database_tail_size);
    CHECK(databases.computed_checksum == databases.stored_checksum);

    unchanged = databases;
    unchanged.valid = 77;
    tail[database_tail_size - 1u] ^= 1u;
    CHECK(csb_v1_csbwin_dungeon_tail_parse_databases(
              tail, database_tail_size, &database_prefix, (uint8_t)'B',
              CSB_V1_CSBWIN_EXTENDED_FLAG_BIG_ACTUATORS, 3u,
              &unchanged) == CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_CHECKSUM);
    CHECK(unchanged.valid == 77);
    tail[database_tail_size - 1u] ^= 1u;
    CHECK(csb_v1_csbwin_dungeon_tail_parse_databases(
              tail, database_tail_size - 1u, &database_prefix, (uint8_t)'B',
              CSB_V1_CSBWIN_EXTENDED_FLAG_BIG_ACTUATORS, 3u,
              &unchanged) == CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED);
    CHECK(unchanged.valid == 77);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_databases(
              tail, database_tail_size + 1u, &database_prefix, (uint8_t)'B',
              CSB_V1_CSBWIN_EXTENDED_FLAG_BIG_ACTUATORS, 3u,
              &unchanged) == CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT);
    CHECK(unchanged.valid == 77);

    legacy_database_tail_size = build_database_tail(
        tail, sizeof(tail), CSB_V1_CSBWIN_LEGACY_FEATURE_VERSION,
        0u, 5u, 0u);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_prefix(
              tail, legacy_database_tail_size, 0u,
              &database_prefix) == CSB_V1_CSBWIN_DUNGEON_TAIL_OK);
    CHECK(csb_v1_csbwin_dungeon_tail_parse_databases(
              tail, legacy_database_tail_size, &database_prefix,
              CSB_V1_CSBWIN_LEGACY_FEATURE_VERSION, 0u, 0u,
              &databases) == CSB_V1_CSBWIN_DUNGEON_TAIL_OK);
    CHECK(!databases.big_actuators);
    CHECK(databases.legacy_scroll_records);
    CHECK(!databases.cell_flag_size_from_extended_features);
    CHECK(databases.database[3].source_entry_bytes == 8u);
    CHECK(databases.database[7].source_entry_bytes == 4u);
    CHECK(databases.cell_flag_bytes == 5u);

    check_staged_real_save();

    if (failures) return 1;
    puts("PASS: CSBWin dungeon-tail DB0-DB15 metadata framing");
    return 0;
}
