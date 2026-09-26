#include "dm1_v1_original_save_atari_handoff.h"

#include <stdlib.h>
#include <string.h>

static const uint8_t k_thing_record_bytes[16] = {
    4u, 6u, 4u, 8u, 16u, 4u, 4u, 4u,
    4u, 8u, 4u, 0u, 0u, 0u, 8u, 4u
};

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) |
           (uint32_t)bytes[3];
}

static uint16_t header_first_half_checksum(const uint8_t *header)
{
    uint16_t checksum = 0u;
    size_t index;

    for (index = 0u; index < 32u; ++index) {
        const uint8_t *row = header + index * 8u;
        checksum = (uint16_t)(checksum + read_be16(row));
        checksum = (uint16_t)(checksum ^ read_be16(row + 2u));
        checksum = (uint16_t)(checksum - read_be16(row + 4u));
        checksum = (uint16_t)(checksum ^ read_be16(row + 6u));
    }
    return checksum;
}

/* ReDMCSB READWRIT.C F0418/F0419 word stream. The returned checksum includes
 * each encrypted and plain word, starting with the part key. */
static uint16_t decode_part(uint8_t *bytes, size_t byte_count, uint16_t key)
{
    uint16_t checksum = key;
    const size_t word_count = byte_count / 2u;
    size_t index;

    for (index = 0u; index < word_count; ++index) {
        const uint16_t encrypted = read_be16(bytes + index * 2u);
        const uint16_t plain = (uint16_t)(encrypted ^ key);
        checksum = (uint16_t)(checksum + encrypted);
        bytes[index * 2u] = (uint8_t)(plain >> 8);
        bytes[index * 2u + 1u] = (uint8_t)plain;
        checksum = (uint16_t)(checksum + plain);
        key = (uint16_t)(key + (uint16_t)(word_count - index));
    }
    return checksum;
}

static int atari_dungeon_tail_layout(const uint8_t *tail, size_t tail_size,
                                     Dm1V1AtariSaveF0435Receipt *receipt)
{
    size_t expected_size = DM1_V1_ATARI_DUNGEON_HEADER_BYTES;
    size_t map_bytes;
    size_t column_count = 0u;
    size_t thing_bytes = 0u;
    uint16_t raw_map_bytes;
    uint16_t text_words;
    uint16_t square_words;
    uint8_t map_count;
    unsigned int map_index;
    unsigned int type;

    if (!tail || !receipt || tail_size < DM1_V1_ATARI_DUNGEON_HEADER_BYTES) {
        return 0;
    }
    raw_map_bytes = read_be16(tail + 2u);
    map_count = tail[4u];
    text_words = read_be16(tail + 6u);
    square_words = read_be16(tail + 10u);
    if (map_count == 0u || map_count > DM1_V1_ATARI_MAX_DUNGEON_MAPS) {
        return 0;
    }
    map_bytes = (size_t)map_count * DM1_V1_ATARI_DUNGEON_MAP_BYTES;
    if (map_bytes > tail_size - expected_size) return 0;
    expected_size += map_bytes;

    /* DEFS.H MEDIA016 stores Atari's DUNGEON_MAP bitfields big-endian, with
     * WidthMax in bits 6..10. F0434 allocates one cumulative entry for every
     * column (WidthMax + 1) on every map. */
    for (map_index = 0u; map_index < map_count; ++map_index) {
        const uint16_t bitfield_a = read_be16(
            tail + DM1_V1_ATARI_DUNGEON_HEADER_BYTES +
            map_index * DM1_V1_ATARI_DUNGEON_MAP_BYTES + 8u);
        column_count += ((size_t)(bitfield_a >> 6u) & 0x1fu) + 1u;
    }
    if (column_count > (SIZE_MAX - expected_size) / 2u) return 0;
    expected_size += column_count * 2u;
    if ((size_t)square_words > (SIZE_MAX - expected_size) / 2u) return 0;
    expected_size += (size_t)square_words * 2u;
    if ((size_t)text_words > (SIZE_MAX - expected_size) / 2u) return 0;
    expected_size += (size_t)text_words * 2u;

    for (type = 0u; type < 16u; ++type) {
        const size_t count = read_be16(tail + 12u + type * 2u);
        const size_t unit = k_thing_record_bytes[type];
        if (unit == 0u && count != 0u) return 0;
        if (unit != 0u && count > (SIZE_MAX - expected_size) / unit) return 0;
        thing_bytes += count * unit;
    }
    if (thing_bytes > SIZE_MAX - expected_size) return 0;
    expected_size += thing_bytes;
    if ((size_t)raw_map_bytes > SIZE_MAX - expected_size) return 0;
    expected_size += raw_map_bytes;

    /* Atari v1.0 precedes F0422's appended dungeon checksum. The complete
     * source stream must end exactly at the last raw-map byte. */
    if (expected_size != tail_size) return 0;
    receipt->dungeon_ornament_seed = read_be16(tail);
    receipt->dungeon_raw_map_byte_count = raw_map_bytes;
    receipt->dungeon_text_word_count = text_words;
    receipt->dungeon_square_first_thing_count = square_words;
    receipt->dungeon_column_count = (uint32_t)column_count;
    receipt->dungeon_map_count = map_count;
    receipt->dungeon_byte_count = (uint32_t)expected_size;
    receipt->dungeon_layout_authenticated = 1;
    return 1;
}

int dm1_v1_original_save_atari_f0435_receipt_bytes(
    const uint8_t *bytes, size_t size,
    Dm1V1AtariSaveF0435Receipt *out_receipt)
{
    Dm1V1AtariSaveF0435Receipt staged;
    uint8_t header[DM1_V1_ATARI_SAVE_F0435_HEADER_BYTES];
    uint8_t global[DM1_V1_ATARI_SAVE_GLOBAL_BYTES];
    uint16_t key;
    uint16_t header_sum;
    uint16_t actual_header_sum = 0u;
    size_t part_sizes[DM1_V1_ATARI_SAVE_PART_COUNT];
    size_t cursor = DM1_V1_ATARI_SAVE_F0435_HEADER_BYTES;
    size_t index;

    if (!bytes || !out_receipt) return DM1_V1_ATARI_SAVE_ERR_ARGUMENT;
    memset(&staged, 0, sizeof(staged));
    if (!dm1_v1_original_save_classify_bytes(bytes, size, &staged.classify)) {
        return DM1_V1_ATARI_SAVE_ERR_ARGUMENT;
    }
    if (staged.classify.shape != DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1 ||
        staged.classify.format_id != 1u) {
        *out_receipt = staged;
        return DM1_V1_ATARI_SAVE_ERR_NOT_FORMAT1;
    }
    if (size < sizeof(header)) {
        *out_receipt = staged;
        return DM1_V1_ATARI_SAVE_ERR_HEADER;
    }
    memcpy(header, bytes, sizeof(header));
    header_sum = header_first_half_checksum(header);
    key = read_be16(header + 20u);
    actual_header_sum = decode_part(header + 256u, 256u, key);
    /* F0429's header sum covers decoded second-half words, not F0419's
     * running part checksum. */
    actual_header_sum = 0u;
    for (index = 0u; index < 128u; ++index) {
        actual_header_sum = (uint16_t)(actual_header_sum +
                                       read_be16(header + 256u + index * 2u));
    }
    if (header_sum != actual_header_sum || !staged.classify.header_checksum_ok) {
        *out_receipt = staged;
        return DM1_V1_ATARI_SAVE_ERR_HEADER;
    }
    staged.header_authenticated = 1;
    for (index = 0u; index < DM1_V1_ATARI_SAVE_PART_COUNT; ++index) {
        staged.expected_checksums[index] = read_be16(header + 342u + index * 2u);
    }

    if (size - cursor < sizeof(global)) {
        *out_receipt = staged;
        return DM1_V1_ATARI_SAVE_ERR_PART;
    }
    memcpy(global, bytes + cursor, sizeof(global));
    staged.part_offsets[0] = (uint32_t)cursor;
    staged.part_byte_counts[0] = sizeof(global);
    staged.actual_checksums[0] = decode_part(
        global, sizeof(global), read_be16(header + 310u));
    if (staged.actual_checksums[0] != staged.expected_checksums[0]) {
        *out_receipt = staged;
        return DM1_V1_ATARI_SAVE_ERR_PART;
    }
    ++staged.parts_authenticated;
    staged.party_champion_count = read_be16(global + 10u);
    staged.event_count = read_be16(global + 24u);
    staged.event_capacity = read_be16(global + 28u);
    staged.current_active_group_count = read_be16(global + 30u);
    staged.maximum_active_group_count = read_be16(global + 46u);
    if (staged.party_champion_count == 0u || staged.party_champion_count > 4u ||
        read_be16(global + 12u) >= 32u || read_be16(global + 14u) >= 32u ||
        read_be16(global + 16u) > 3u || read_be16(global + 18u) >= 32u ||
        read_be16(global + 20u) >= staged.party_champion_count ||
        staged.event_count > staged.event_capacity ||
        staged.event_capacity > DM1_V1_ATARI_MAX_EVENTS ||
        staged.current_active_group_count > staged.maximum_active_group_count ||
        staged.maximum_active_group_count > DM1_V1_ATARI_MAX_ACTIVE_GROUPS) {
        *out_receipt = staged;
        return DM1_V1_ATARI_SAVE_ERR_COUNTS;
    }

    part_sizes[0] = sizeof(global);
    part_sizes[1] = (size_t)staged.maximum_active_group_count *
                     DM1_V1_ATARI_SAVE_ACTIVE_GROUP_BYTES;
    part_sizes[2] = DM1_V1_ATARI_SAVE_PARTY_BYTES;
    part_sizes[3] = (size_t)staged.event_capacity * DM1_V1_ATARI_SAVE_EVENT_BYTES;
    part_sizes[4] = (size_t)staged.event_capacity * DM1_V1_ATARI_SAVE_TIMELINE_BYTES;
    cursor += part_sizes[0];
    for (index = 1u; index < DM1_V1_ATARI_SAVE_PART_COUNT; ++index) {
        uint8_t *part;
        if ((part_sizes[index] & 1u) != 0u ||
            part_sizes[index] > UINT32_MAX || cursor > UINT32_MAX ||
            cursor > size ||
            part_sizes[index] > size - cursor) {
            *out_receipt = staged;
            return DM1_V1_ATARI_SAVE_ERR_PART;
        }
        part = (uint8_t *)malloc(part_sizes[index] ? part_sizes[index] : 1u);
        if (!part) {
            *out_receipt = staged;
            return DM1_V1_ATARI_SAVE_ERR_PART;
        }
        memcpy(part, bytes + cursor, part_sizes[index]);
        staged.part_offsets[index] = (uint32_t)cursor;
        staged.part_byte_counts[index] = (uint32_t)part_sizes[index];
        staged.actual_checksums[index] = decode_part(
            part, part_sizes[index], read_be16(header + 310u + index * 2u));
        free(part);
        if (staged.actual_checksums[index] != staged.expected_checksums[index]) {
            *out_receipt = staged;
            return DM1_V1_ATARI_SAVE_ERR_PART;
        }
        cursor += part_sizes[index];
        ++staged.parts_authenticated;
    }
    staged.body_authenticated = 1;
    staged.dungeon_offset = (uint32_t)cursor;
    if (!atari_dungeon_tail_layout(bytes + cursor, size - cursor, &staged)) {
        *out_receipt = staged;
        return DM1_V1_ATARI_SAVE_ERR_DUNGEON;
    }
    if (cursor > UINT32_MAX || size - cursor > UINT32_MAX ||
        read_be16(global + 18u) >= staged.dungeon_map_count) {
        staged.dungeon_layout_authenticated = 0;
        *out_receipt = staged;
        return DM1_V1_ATARI_SAVE_ERR_DUNGEON;
    }
    {
        const uint8_t *map = bytes + cursor +
            DM1_V1_ATARI_DUNGEON_HEADER_BYTES +
            (size_t)read_be16(global + 18u) * DM1_V1_ATARI_DUNGEON_MAP_BYTES;
        const uint16_t bitfield_a = read_be16(map + 8u);
        const unsigned int width =
            ((unsigned int)(bitfield_a >> 6u) & 0x1fu) + 1u;
        const unsigned int height =
            ((unsigned int)(bitfield_a >> 11u) & 0x1fu) + 1u;
        if (read_be16(global + 12u) >= width ||
            read_be16(global + 14u) >= height) {
            staged.dungeon_layout_authenticated = 0;
            *out_receipt = staged;
            return DM1_V1_ATARI_SAVE_ERR_DUNGEON;
        }
    }
    /* FormatID 1 carries neither Platform nor DungeonID. Match the original
     * LOADSAVE.C legacy DM detector so this adapter cannot admit another
     * campaign's tail as Dungeon Master. */
    if (staged.dungeon_map_count != 14u || staged.dungeon_ornament_seed != 99u) {
        staged.dungeon_layout_authenticated = 0;
        *out_receipt = staged;
        return DM1_V1_ATARI_SAVE_ERR_DUNGEON;
    }
    *out_receipt = staged;
    return DM1_V1_ATARI_SAVE_OK;
}

const char *dm1_v1_original_save_atari_f0435_result_name(int result)
{
    switch (result) {
    case DM1_V1_ATARI_SAVE_OK: return "ok";
    case DM1_V1_ATARI_SAVE_ERR_ARGUMENT: return "argument";
    case DM1_V1_ATARI_SAVE_ERR_NOT_FORMAT1: return "not-format1";
    case DM1_V1_ATARI_SAVE_ERR_HEADER: return "header";
    case DM1_V1_ATARI_SAVE_ERR_PART: return "part";
    case DM1_V1_ATARI_SAVE_ERR_COUNTS: return "counts";
    case DM1_V1_ATARI_SAVE_ERR_DUNGEON: return "dungeon";
    default: return "unknown";
    }
}
