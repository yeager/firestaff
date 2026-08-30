#include "dm1_v1_original_save_amiga_handoff.h"

#include <stdlib.h>
#include <string.h>

enum {
    DM1_AMIGA_SAVE_FORMAT_5 = 5,
    DM1_AMIGA_SAVE_PLATFORM = 3,
    DM1_AMIGA_SAVE_DUNGEON_DM = 10,
    DM1_AMIGA_SAVE_HEADER_KEY_OFFSET = 20,
    DM1_AMIGA_SAVE_HEADER_KEYS_OFFSET = 310,
    DM1_AMIGA_SAVE_HEADER_CHECKSUMS_OFFSET = 342,
    DM1_AMIGA_SAVE_MAX_ACTIVE_GROUPS = 2048,
    DM1_AMIGA_SAVE_MAX_EVENTS = 4096,
    DM1_AMIGA_SAVE_MAX_MAPS = 16
};

static const uint8_t k_thing_data_byte_counts[16] = {
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
           bytes[3];
}

static int16_t read_be_i16(const uint8_t *bytes)
{
    return (int16_t)read_be16(bytes);
}

static uint16_t header_first_half_checksum(const uint8_t *header)
{
    uint16_t checksum = 0u;
    size_t index;

    for (index = 0u; index < 32u; ++index) {
        const size_t offset = index * 8u;
        checksum = (uint16_t)(checksum + read_be16(header + offset));
        checksum = (uint16_t)(checksum ^ read_be16(header + offset + 2u));
        checksum = (uint16_t)(checksum - read_be16(header + offset + 4u));
        checksum = (uint16_t)(checksum ^ read_be16(header + offset + 6u));
    }
    return checksum;
}

static uint16_t deobfuscate_words_be(uint8_t *bytes, size_t byte_count,
                                     uint16_t key)
{
    uint16_t checksum = key;
    size_t index;
    const size_t word_count = byte_count / 2u;

    for (index = 0u; index < word_count; ++index) {
        uint16_t encrypted = read_be16(bytes + index * 2u);
        uint16_t plain;
        checksum = (uint16_t)(checksum + encrypted);
        plain = (uint16_t)(encrypted ^ key);
        bytes[index * 2u] = (uint8_t)(plain >> 8);
        bytes[index * 2u + 1u] = (uint8_t)plain;
        checksum = (uint16_t)(checksum + plain);
        key = (uint16_t)(key + (uint16_t)(word_count - index));
    }
    return checksum;
}

static int authenticate_part(const uint8_t *source, size_t source_size,
                             size_t *io_cursor, size_t byte_count,
                             uint16_t key, uint16_t expected_checksum,
                             uint16_t *out_actual_checksum)
{
    uint8_t *plain;
    uint16_t actual_checksum;

    if (!source || !io_cursor || byte_count == 0u ||
        (byte_count & 1u) != 0u || *io_cursor > source_size ||
        byte_count > source_size - *io_cursor) {
        return 0;
    }
    plain = (uint8_t *)malloc(byte_count);
    if (!plain) return 0;
    memcpy(plain, source + *io_cursor, byte_count);
    actual_checksum = deobfuscate_words_be(plain, byte_count, key);
    free(plain);
    if (out_actual_checksum) *out_actual_checksum = actual_checksum;
    if (actual_checksum != expected_checksum) return 0;
    *io_cursor += byte_count;
    return 1;
}

static uint16_t add_bytes_checksum(uint16_t checksum,
                                   const uint8_t *bytes, size_t byte_count)
{
    size_t index;
    for (index = 0u; index < byte_count; ++index) {
        checksum = (uint16_t)(checksum + bytes[index]);
    }
    return checksum;
}

static int advance_tail(size_t *io_cursor, size_t source_size, size_t byte_count)
{
    if (!io_cursor || *io_cursor > source_size ||
        byte_count > source_size - *io_cursor) {
        return 0;
    }
    *io_cursor += byte_count;
    return 1;
}

static int authenticate_dungeon_tail(const uint8_t *bytes, size_t size,
                                     Dm1V1AmigaSaveF0435Receipt *receipt)
{
    const size_t portraits = (size_t)DM1_V1_AMIGA_SAVE_PORTRAIT_BYTES *
                             DM1_V1_AMIGA_SAVE_PORTRAIT_COUNT;
    const size_t map_bytes = DM1_V1_AMIGA_SAVE_DUNGEON_MAP_BYTES;
    size_t dungeon_start = receipt->authenticated_body_end_offset;
    size_t cursor;
    size_t map_table_offset;
    size_t raw_data_offset;
    size_t thing_bytes = 0u;
    uint16_t checksum = 0u;
    uint16_t raw_map_bytes;
    uint16_t text_words;
    uint16_t square_first_things;
    uint8_t map_count;
    unsigned int columns = 0u;
    unsigned int type;
    unsigned int map_index;

    if (!advance_tail(&dungeon_start, size, portraits) ||
        !advance_tail(&dungeon_start, size,
                      DM1_V1_AMIGA_SAVE_DUNGEON_HEADER_BYTES)) {
        return 0;
    }
    /* The real save carries four raw 32x29 Amiga portraits immediately before
     * F0434.  DEFS.H:658 specifies 464 bytes per portrait. */
    receipt->portrait_byte_count = (uint32_t)portraits;
    receipt->dungeon_offset = (uint32_t)(dungeon_start -
        DM1_V1_AMIGA_SAVE_DUNGEON_HEADER_BYTES);
    cursor = receipt->dungeon_offset;
    checksum = add_bytes_checksum(checksum, bytes + cursor,
                                  DM1_V1_AMIGA_SAVE_DUNGEON_HEADER_BYTES);
    raw_map_bytes = read_be16(bytes + cursor + 2u);
    map_count = bytes[cursor + 4u];
    text_words = read_be16(bytes + cursor + 6u);
    square_first_things = read_be16(bytes + cursor + 10u);
    if (map_count == 0u || map_count > DM1_AMIGA_SAVE_MAX_MAPS) return 0;
    receipt->dungeon_raw_map_byte_count = raw_map_bytes;
    receipt->dungeon_map_count = map_count;
    receipt->dungeon_text_word_count = text_words;
    receipt->dungeon_square_first_thing_count = square_first_things;
    map_table_offset = cursor + DM1_V1_AMIGA_SAVE_DUNGEON_HEADER_BYTES;
    if (!advance_tail(&cursor, size, DM1_V1_AMIGA_SAVE_DUNGEON_HEADER_BYTES) ||
        !advance_tail(&cursor, size, (size_t)map_count * map_bytes)) {
        return 0;
    }
    checksum = add_bytes_checksum(checksum, bytes + map_table_offset,
                                  (size_t)map_count * map_bytes);
    for (map_index = 0u; map_index < map_count; ++map_index) {
        const uint8_t *map = bytes + map_table_offset + map_index * map_bytes;
        const uint16_t bitfield_a = read_be16(map + 8u);
        /* A20's original on-disk struct retains the MEGAMAX/PC-compatible
         * low-bit-first field order (DEFS.H MEDIA016, !THINK_C): level 0..5,
         * width 6..10, height 11..15.  The 16-bit container itself is read
         * big-endian from the Amiga disk. */
        const unsigned int width_max = ((unsigned int)(bitfield_a >> 6u) & 0x1fu);
        const unsigned int height_max = ((unsigned int)(bitfield_a >> 11u) & 0x1fu);
        /* F0434 only uses the A.Width values here to size the cumulative
         * column table.  Raw-map offsets are consumed later by map selection
         * and are deliberately not used as an alternate tail-size oracle. */
        if (width_max > 31u || height_max > 31u ||
            columns > 0xffffu - (width_max + 1u)) {
            return 0;
        }
        columns += width_max + 1u;
    }
    receipt->dungeon_column_count = (uint16_t)columns;
    receipt->dungeon_byte_count = (uint32_t)(cursor - receipt->dungeon_offset);
    if (!advance_tail(&cursor, size, columns * 2u) ||
        !advance_tail(&cursor, size, (size_t)square_first_things * 2u) ||
        !advance_tail(&cursor, size, (size_t)text_words * 2u)) {
        return 0;
    }
    checksum = add_bytes_checksum(checksum, bytes + map_table_offset +
                                  (size_t)map_count * map_bytes,
                                  cursor - (map_table_offset + (size_t)map_count * map_bytes));
    receipt->dungeon_byte_count = (uint32_t)(cursor - receipt->dungeon_offset);
    for (type = 0u; type < 16u; ++type) {
        const size_t count = read_be16(bytes + receipt->dungeon_offset + 12u + type * 2u);
        if (k_thing_data_byte_counts[type] == 0u && count != 0u) {
            return 0;
        }
        if (k_thing_data_byte_counts[type] != 0u &&
            count > SIZE_MAX / k_thing_data_byte_counts[type]) return 0;
        thing_bytes = count * k_thing_data_byte_counts[type];
        if (!advance_tail(&cursor, size, thing_bytes)) return 0;
        checksum = add_bytes_checksum(checksum, bytes + cursor - thing_bytes,
                                      thing_bytes);
        receipt->dungeon_byte_count = (uint32_t)(cursor - receipt->dungeon_offset);
    }
    raw_data_offset = cursor;
    if (!advance_tail(&cursor, size, raw_map_bytes) ||
        !advance_tail(&cursor, size, 2u) || cursor != size) {
        return 0;
    }
    checksum = add_bytes_checksum(checksum, bytes + raw_data_offset, raw_map_bytes);
    receipt->dungeon_checksum_offset = (uint32_t)(cursor - 2u);
    receipt->dungeon_expected_checksum = read_be16(bytes + cursor - 2u);
    receipt->dungeon_actual_checksum = checksum;
    receipt->dungeon_byte_count = (uint32_t)(cursor - receipt->dungeon_offset);
    if (receipt->dungeon_expected_checksum != checksum) return 0;
    receipt->tail_authenticated = 1;
    return 1;
}

int dm1_v1_original_save_amiga_f0435_receipt_bytes(
    const uint8_t *bytes,
    size_t size,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435Receipt staged;
    uint8_t header[DM1_V1_AMIGA_SAVE_F0435_HEADER_BYTES];
    uint8_t global_data[DM1_V1_AMIGA_SAVE_F0435_GLOBAL_DATA_BYTES];
    uint16_t header_sum;
    uint16_t second_sum = 0u;
    uint16_t keys[DM1_V1_AMIGA_SAVE_F0435_PART_COUNT];
    size_t cursor = DM1_V1_AMIGA_SAVE_F0435_HEADER_BYTES;
    size_t part_sizes[DM1_V1_AMIGA_SAVE_F0435_PART_COUNT];
    size_t index;

    if (!bytes || !out_receipt) return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    memset(&staged, 0, sizeof(staged));
    if (!dm1_v1_original_save_classify_bytes(bytes, size, &staged.classify)) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    }
    if (staged.classify.shape != DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_COMPAT_FAMILY ||
        staged.classify.format_id != DM1_AMIGA_SAVE_FORMAT_5 ||
        staged.classify.platform != DM1_AMIGA_SAVE_PLATFORM ||
        staged.classify.dungeon_id != DM1_AMIGA_SAVE_DUNGEON_DM) {
        *out_receipt = staged;
        return DM1_V1_AMIGA_SAVE_F0435_ERR_NOT_AMIGA_FORMAT5;
    }
    if (size < sizeof(header)) {
        *out_receipt = staged;
        return DM1_V1_AMIGA_SAVE_F0435_ERR_HEADER;
    }
    memcpy(header, bytes, sizeof(header));
    header_sum = header_first_half_checksum(header);
    (void)deobfuscate_words_be(header + 256u, 256u,
                                read_be16(header + DM1_AMIGA_SAVE_HEADER_KEY_OFFSET));
    for (index = 0u; index < 128u; ++index) {
        second_sum = (uint16_t)(second_sum + read_be16(header + 256u + index * 2u));
    }
    if (header_sum != second_sum || !staged.classify.header_checksum_ok) {
        *out_receipt = staged;
        return DM1_V1_AMIGA_SAVE_F0435_ERR_HEADER;
    }
    staged.header_authenticated = 1;
    for (index = 0u; index < DM1_V1_AMIGA_SAVE_F0435_PART_COUNT; ++index) {
        keys[index] = read_be16(header + DM1_AMIGA_SAVE_HEADER_KEYS_OFFSET + index * 2u);
        staged.expected_checksums[index] =
            read_be16(header + DM1_AMIGA_SAVE_HEADER_CHECKSUMS_OFFSET + index * 2u);
    }

    if (cursor > size || sizeof(global_data) > size - cursor) {
        *out_receipt = staged;
        return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    }
    memcpy(global_data, bytes + cursor, sizeof(global_data));
    staged.part_offsets[0] = (uint32_t)cursor;
    staged.part_byte_counts[0] = sizeof(global_data);
    staged.actual_checksums[0] = deobfuscate_words_be(global_data,
                                                       sizeof(global_data), keys[0]);
    if (staged.actual_checksums[0] != staged.expected_checksums[0]) {
        *out_receipt = staged;
        return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    }
    staged.parts_authenticated = 1u;
    cursor += sizeof(global_data);

    staged.game_time = read_be32(global_data + 0u);
    staged.party_champion_count = read_be16(global_data + 10u);
    staged.party_map_x = read_be_i16(global_data + 12u);
    staged.party_map_y = read_be_i16(global_data + 14u);
    staged.party_direction = read_be_i16(global_data + 16u);
    staged.party_map_index = read_be_i16(global_data + 18u);
    staged.leader_index = read_be_i16(global_data + 20u);
    staged.event_count = read_be16(global_data + 24u);
    staged.first_unused_event_index = read_be16(global_data + 26u);
    staged.event_maximum_count = read_be16(global_data + 28u);
    staged.current_active_group_count = read_be16(global_data + 30u);
    staged.maximum_active_group_count = read_be16(global_data + 46u);
    if (staged.party_champion_count > 4u || staged.party_direction < 0 ||
        staged.party_direction > 3 || staged.party_map_index < 0 ||
        staged.party_map_x < 0 || staged.party_map_y < 0 ||
        staged.leader_index < -1 ||
        (staged.party_champion_count == 0u && staged.leader_index > 0) ||
        (staged.party_champion_count != 0u &&
         staged.leader_index >= (int16_t)staged.party_champion_count) ||
        staged.maximum_active_group_count > DM1_AMIGA_SAVE_MAX_ACTIVE_GROUPS ||
        staged.current_active_group_count > staged.maximum_active_group_count ||
        staged.event_maximum_count > DM1_AMIGA_SAVE_MAX_EVENTS ||
        staged.event_count > staged.event_maximum_count) {
        *out_receipt = staged;
        return DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY;
    }

    part_sizes[0] = sizeof(global_data);
    part_sizes[1] = (size_t)staged.maximum_active_group_count *
                    DM1_V1_AMIGA_SAVE_F0435_ACTIVE_GROUP_BYTES;
    part_sizes[2] = DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES;
    part_sizes[3] = (size_t)staged.event_maximum_count *
                    DM1_V1_AMIGA_SAVE_F0435_EVENT_BYTES;
    part_sizes[4] = (size_t)staged.event_maximum_count * 2u;
    for (index = 1u; index < DM1_V1_AMIGA_SAVE_F0435_PART_COUNT; ++index) {
        staged.part_offsets[index] = (uint32_t)cursor;
        staged.part_byte_counts[index] = (uint32_t)part_sizes[index];
        if (!authenticate_part(bytes, size, &cursor, part_sizes[index], keys[index],
                               staged.expected_checksums[index],
                               &staged.actual_checksums[index])) {
            *out_receipt = staged;
            return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
        }
        ++staged.parts_authenticated;
    }
    staged.authenticated_body_end_offset = (uint32_t)cursor;
    staged.trailing_source_byte_count = (uint32_t)(size - cursor);
    staged.body_authenticated = 1;
    if (!authenticate_dungeon_tail(bytes, size, &staged)) {
        *out_receipt = staged;
        return DM1_V1_AMIGA_SAVE_F0435_ERR_TAIL;
    }
    *out_receipt = staged;
    return DM1_V1_AMIGA_SAVE_F0435_OK;
}

int dm1_v1_original_save_amiga_f0435_global_data_bytes(
    const uint8_t *bytes, size_t size,
    Dm1V1AmigaSaveF0435GlobalData *out_global,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435Receipt receipt;
    uint8_t global_data[DM1_V1_AMIGA_SAVE_F0435_GLOBAL_DATA_BYTES];
    uint8_t header[DM1_V1_AMIGA_SAVE_F0435_HEADER_BYTES];
    uint16_t key;
    int result;

    if (!bytes || !out_global) return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    memset(&receipt, 0, sizeof(receipt));
    result = dm1_v1_original_save_amiga_f0435_receipt_bytes(
        bytes, size, &receipt);
    if (out_receipt) *out_receipt = receipt;
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) return result;
    if (receipt.part_offsets[0] != DM1_V1_AMIGA_SAVE_F0435_HEADER_BYTES ||
        receipt.part_byte_counts[0] != sizeof(global_data) ||
        size < sizeof(header)) return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    memcpy(header, bytes, sizeof(header));
    (void)deobfuscate_words_be(header + 256u, 256u,
                                read_be16(header + DM1_AMIGA_SAVE_HEADER_KEY_OFFSET));
    key = read_be16(header + DM1_AMIGA_SAVE_HEADER_KEYS_OFFSET);
    memcpy(global_data, bytes + receipt.part_offsets[0], sizeof(global_data));
    if (deobfuscate_words_be(global_data, sizeof(global_data), key) !=
        receipt.expected_checksums[0]) return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    out_global->game_time = read_be32(global_data + 0u);
    out_global->party_champion_count = read_be16(global_data + 10u);
    out_global->party_map_x = read_be_i16(global_data + 12u);
    out_global->party_map_y = read_be_i16(global_data + 14u);
    out_global->party_direction = read_be_i16(global_data + 16u);
    out_global->party_map_index = read_be_i16(global_data + 18u);
    out_global->leader_index = read_be_i16(global_data + 20u);
    out_global->event_count = read_be16(global_data + 24u);
    out_global->first_unused_event_index = read_be16(global_data + 26u);
    out_global->event_maximum_count = read_be16(global_data + 28u);
    out_global->current_active_group_count = read_be16(global_data + 30u);
    out_global->maximum_active_group_count = read_be16(global_data + 46u);
    return DM1_V1_AMIGA_SAVE_F0435_OK;
}

int dm1_v1_original_save_amiga_f0435_party_part_bytes(
    const uint8_t *bytes, size_t size,
    uint8_t out_party[DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES],
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435Receipt receipt;
    uint8_t header[DM1_V1_AMIGA_SAVE_F0435_HEADER_BYTES];
    uint16_t key;
    int result;

    if (!bytes || !out_party) return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    memset(&receipt, 0, sizeof(receipt));
    result = dm1_v1_original_save_amiga_f0435_receipt_bytes(
        bytes, size, &receipt);
    if (out_receipt) *out_receipt = receipt;
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) return result;
    if (receipt.part_byte_counts[2] !=
            DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES ||
        receipt.part_offsets[2] > size ||
        DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES >
            size - receipt.part_offsets[2] ||
        size < sizeof(header)) return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    memcpy(header, bytes, sizeof(header));
    (void)deobfuscate_words_be(header + 256u, 256u,
                                read_be16(header + DM1_AMIGA_SAVE_HEADER_KEY_OFFSET));
    key = read_be16(header + DM1_AMIGA_SAVE_HEADER_KEYS_OFFSET + 4u);
    memcpy(out_party, bytes + receipt.part_offsets[2],
           DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES);
    if (deobfuscate_words_be(out_party,
                             DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES, key) !=
        receipt.expected_checksums[2]) return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    return DM1_V1_AMIGA_SAVE_F0435_OK;
}

int dm1_v1_original_save_amiga_f0435_dungeon_tail_bytes(
    const uint8_t *bytes, size_t size,
    uint8_t *out_tail, size_t out_tail_capacity, size_t *out_tail_size,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435Receipt receipt;
    size_t tail_size;
    int result;

    if (out_tail_size) *out_tail_size = 0u;
    if (!bytes || !out_tail_size) return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    memset(&receipt, 0, sizeof(receipt));
    result = dm1_v1_original_save_amiga_f0435_receipt_bytes(
        bytes, size, &receipt);
    if (out_receipt) *out_receipt = receipt;
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) return result;
    if (receipt.dungeon_offset > size ||
        receipt.dungeon_byte_count > size - receipt.dungeon_offset) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_TAIL;
    }
    tail_size = (size_t)receipt.dungeon_byte_count;
    *out_tail_size = tail_size;
    if (!out_tail || out_tail_capacity < tail_size) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY;
    }
    memcpy(out_tail, bytes + receipt.dungeon_offset, tail_size);
    return DM1_V1_AMIGA_SAVE_F0435_OK;
}

const char *dm1_v1_original_save_amiga_f0435_result_name(int result)
{
    switch (result) {
    case DM1_V1_AMIGA_SAVE_F0435_OK: return "ok";
    case DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT: return "argument";
    case DM1_V1_AMIGA_SAVE_F0435_ERR_NOT_AMIGA_FORMAT5: return "not-amiga-format5";
    case DM1_V1_AMIGA_SAVE_F0435_ERR_HEADER: return "header";
    case DM1_V1_AMIGA_SAVE_F0435_ERR_BODY: return "body";
    case DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY: return "capacity";
    case DM1_V1_AMIGA_SAVE_F0435_ERR_TAIL: return "tail";
    default: return "unknown";
    }
}
