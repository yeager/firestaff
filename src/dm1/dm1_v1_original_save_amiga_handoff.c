#include "dm1_v1_original_save_amiga_handoff.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "memory_champion_lifecycle_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_group_state_bundle_pc34_compat.h"
#include "dm1_v1_inventory_slot_placement_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

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

static int32_t read_be_i32(const uint8_t *bytes)
{
    return (int32_t)read_be32(bytes);
}

static void free_unpublished_world(struct GameWorld_Compat *world)
{
    if (!world) return;
    if (world->things) {
        F0504_DUNGEON_FreeThingData_Compat(world->things);
        free(world->things);
    }
    if (world->dungeon) {
        F0500_DUNGEON_FreeDatHeader_Compat(world->dungeon);
        free(world->dungeon);
    }
    memset(world, 0, sizeof(*world));
}

static uint32_t fingerprint_bytes(const uint8_t *bytes, size_t byte_count)
{
    uint32_t hash = 2166136261u;
    size_t index;
    for (index = 0u; index < byte_count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
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
    out_global->last_random_number = read_be32(global_data + 4u);
    out_global->leader_hand_object = read_be16(global_data + 8u);
    out_global->party_champion_count = read_be16(global_data + 10u);
    out_global->party_map_x = read_be_i16(global_data + 12u);
    out_global->party_map_y = read_be_i16(global_data + 14u);
    out_global->party_direction = read_be_i16(global_data + 16u);
    out_global->party_map_index = read_be_i16(global_data + 18u);
    out_global->leader_index = read_be_i16(global_data + 20u);
    out_global->magic_caster_champion_index = read_be_i16(global_data + 22u);
    out_global->event_count = read_be16(global_data + 24u);
    out_global->first_unused_event_index = read_be16(global_data + 26u);
    out_global->event_maximum_count = read_be16(global_data + 28u);
    out_global->current_active_group_count = read_be16(global_data + 30u);
    out_global->last_creature_attack_time = read_be_i32(global_data + 32u);
    out_global->last_party_movement_time = read_be_i32(global_data + 36u);
    out_global->disabled_movement_ticks = read_be_i16(global_data + 40u);
    out_global->projectile_disabled_movement_ticks =
        read_be_i16(global_data + 42u);
    out_global->last_projectile_disabled_movement_direction =
        read_be16(global_data + 44u);
    out_global->maximum_active_group_count = read_be16(global_data + 46u);
    return DM1_V1_AMIGA_SAVE_F0435_OK;
}

int dm1_v1_original_save_amiga_f0435_party_part_bytes(
    const uint8_t *bytes, size_t size,
    uint8_t out_party[DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES],
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    size_t party_size = 0u;
    int result;

    if (!out_party) return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    result = dm1_v1_original_save_amiga_f0435_part_bytes(
        bytes, size, 2u, out_party,
        DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES, &party_size, out_receipt);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) return result;
    return party_size == DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES
        ? DM1_V1_AMIGA_SAVE_F0435_OK : DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
}

int dm1_v1_original_save_amiga_f0435_part_bytes(
    const uint8_t *bytes, size_t size, unsigned int part_index,
    uint8_t *out_part, size_t out_part_capacity, size_t *out_part_size,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435Receipt receipt;
    uint8_t header[DM1_V1_AMIGA_SAVE_F0435_HEADER_BYTES];
    uint16_t key;
    size_t part_size;
    int result;

    if (out_part_size) *out_part_size = 0u;
    if (!bytes || !out_part_size || part_index >=
        DM1_V1_AMIGA_SAVE_F0435_PART_COUNT) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    }
    memset(&receipt, 0, sizeof(receipt));
    result = dm1_v1_original_save_amiga_f0435_receipt_bytes(bytes, size,
                                                             &receipt);
    if (out_receipt) *out_receipt = receipt;
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) return result;
    part_size = receipt.part_byte_counts[part_index];
    *out_part_size = part_size;
    if (!out_part || out_part_capacity < part_size) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY;
    }
    if (receipt.part_offsets[part_index] > size ||
        part_size > size - receipt.part_offsets[part_index]) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    }
    memcpy(header, bytes, sizeof(header));
    (void)deobfuscate_words_be(header + 256u, 256u,
                                read_be16(header + DM1_AMIGA_SAVE_HEADER_KEY_OFFSET));
    key = read_be16(header + DM1_AMIGA_SAVE_HEADER_KEYS_OFFSET + part_index * 2u);
    memcpy(out_part, bytes + receipt.part_offsets[part_index], part_size);
    if (deobfuscate_words_be(out_part, part_size, key) !=
        receipt.expected_checksums[part_index]) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    }
    return DM1_V1_AMIGA_SAVE_F0435_OK;
}

int dm1_v1_original_save_amiga_f0435_party_receipt_bytes(
    const uint8_t *bytes, size_t size,
    Dm1V1AmigaSavePartyReceipt *out_party,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    uint8_t party[DM1_V1_AMIGA_SAVE_F0435_PARTY_BYTES];
    Dm1V1AmigaSaveF0435Receipt receipt;
    unsigned int champion;
    int result;

    if (!bytes || !out_party) return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    memset(&receipt, 0, sizeof(receipt));
    memset(out_party, 0, sizeof(*out_party));
    result = dm1_v1_original_save_amiga_f0435_party_part_bytes(
        bytes, size, party, &receipt);
    if (out_receipt) *out_receipt = receipt;
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) return result;
    for (champion = 0u; champion < 4u; ++champion) {
        const uint8_t *src = party + champion * DM1_V1_AMIGA_SAVE_CHAMPION_BYTES;
        Dm1V1AmigaSaveChampionReceipt *dst = &out_party->champions[champion];
        unsigned int index;
        int health_current, health_maximum;
        int stamina_current, stamina_maximum;
        int mana_current, mana_maximum;
        memcpy(dst->name, src, sizeof(dst->name));
        memcpy(dst->title, src + 8u, sizeof(dst->title));
        dst->direction = src[28u];
        dst->action_index = src[32u];
        dst->poison_dose = src[42u];
        dst->wounds = read_be16(src + 50u);
        health_current = read_be_i16(src + 52u);
        health_maximum = read_be_i16(src + 54u);
        stamina_current = read_be_i16(src + 56u);
        stamina_maximum = read_be_i16(src + 58u);
        mana_current = read_be_i16(src + 60u);
        mana_maximum = read_be_i16(src + 62u);
        dst->health_current = (uint16_t)health_current;
        dst->health_maximum = (uint16_t)health_maximum;
        dst->stamina_current = (uint16_t)stamina_current;
        dst->stamina_maximum = (uint16_t)stamina_maximum;
        dst->mana_current = (uint16_t)mana_current;
        dst->mana_maximum = (uint16_t)mana_maximum;
        dst->food = read_be_i16(src + 66u);
        dst->water = read_be_i16(src + 68u);
        if (dst->direction > 3u || health_current < 0 || health_maximum < 0 ||
            stamina_current < 0 || stamina_maximum < 0 || mana_current < 0 ||
            mana_maximum < 0 || health_current > health_maximum ||
            stamina_current > stamina_maximum || mana_current > mana_maximum) {
            return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
        }
        for (index = 0u; index < 6u; ++index) {
            const uint8_t *stat = src + 70u + (index + 1u) * 3u;
            dst->attribute_maximums[index] = stat[0];
            dst->attributes[index] = stat[1];
        }
        for (index = 0u; index < 4u; ++index) {
            dst->skill_experience[index] = read_be32(src + 92u + index * 6u + 2u);
        }
        for (index = 0u; index < 30u; ++index) {
            dst->inventory[index] = read_be16(src + 212u + index * 2u);
        }
        dst->load = read_be16(src + 272u);
    }
    {
        const uint8_t *info = party + 4u * DM1_V1_AMIGA_SAVE_CHAMPION_BYTES;
        out_party->magical_light_amount = read_be_i16(info);
        out_party->thieves_eye_count = info[2u];
        out_party->footprints_count = info[3u];
        out_party->shield_defense = read_be_i16(info + 4u);
        out_party->fire_shield_defense = read_be_i16(info + 6u);
        out_party->spell_shield_defense = read_be_i16(info + 8u);
        out_party->scent_count = info[10u];
        out_party->freeze_life_ticks = info[11u];
        out_party->first_scent_index = info[84u];
        out_party->invisibility_count = info[86u];
    }
    return DM1_V1_AMIGA_SAVE_F0435_OK;
}

int dm1_v1_original_save_amiga_f0435_materialize_party_bytes(
    const uint8_t *bytes, size_t size,
    struct PartyState_Compat *out_party,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435GlobalData global;
    Dm1V1AmigaSavePartyReceipt source_party;
    struct PartyState_Compat candidate;
    unsigned int champion;
    int result;

    if (!bytes || !out_party) return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    memset(&global, 0, sizeof(global));
    memset(&source_party, 0, sizeof(source_party));
    result = dm1_v1_original_save_amiga_f0435_global_data_bytes(
        bytes, size, &global, out_receipt);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) return result;
    result = dm1_v1_original_save_amiga_f0435_party_receipt_bytes(
        bytes, size, &source_party, NULL);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) return result;
    memset(&candidate, 0, sizeof(candidate));
    candidate.championCount = global.party_champion_count;
    candidate.mapIndex = global.party_map_index;
    candidate.mapX = global.party_map_x;
    candidate.mapY = global.party_map_y;
    candidate.direction = global.party_direction;
    candidate.activeChampionIndex = global.leader_index;
    for (champion = 0u; champion < CHAMPION_MAX_PARTY; ++champion) {
        struct ChampionState_Compat *dst = &candidate.champions[champion];
        const Dm1V1AmigaSaveChampionReceipt *src =
            &source_party.champions[champion];
        unsigned int slot;
        F0600_CHAMPION_InitEmpty_Compat(dst);
        if (champion >= global.party_champion_count) continue;
        dst->present = 1;
        dst->portraitIndex = (int)champion;
        memcpy(dst->name, src->name, sizeof(dst->name));
        memcpy(dst->title, src->title, sizeof(dst->title));
        dst->direction = src->direction;
        dst->actionIndex = src->action_index;
        dst->poisonDose = src->poison_dose;
        dst->wounds = src->wounds;
        dst->hp.current = src->health_current;
        dst->hp.maximum = src->health_maximum;
        dst->hp.shifted = (uint16_t)(src->health_maximum << 1u);
        dst->stamina.current = src->stamina_current;
        dst->stamina.maximum = src->stamina_maximum;
        dst->stamina.shifted = (uint16_t)(src->stamina_maximum << 1u);
        dst->mana.current = src->mana_current;
        dst->mana.maximum = src->mana_maximum;
        dst->mana.shifted = (uint16_t)(src->mana_maximum << 1u);
        dst->food = src->food;
        dst->water = src->water;
        dst->load = src->load;
        for (slot = 0u; slot < CHAMPION_ATTR_COUNT; ++slot) {
            dst->attributeMaximums[slot] = src->attribute_maximums[slot];
            dst->attributes[slot] = src->attributes[slot];
        }
        for (slot = 0u; slot < CHAMPION_SKILL_COUNT; ++slot) {
            dst->skillExperience[slot] = src->skill_experience[slot];
        }
        for (slot = 0u; slot < CHAMPION_SLOT_COUNT; ++slot) {
            const int runtime_slot =
                dm1_v1_inventory_champion_slot_for_source_slot_box_pc34(
                    (int)slot + 8);
            if (runtime_slot < 0 || runtime_slot >= CHAMPION_SLOT_COUNT) {
                return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
            }
            dst->inventory[runtime_slot] = src->inventory[slot];
        }
    }
    *out_party = candidate;
    return DM1_V1_AMIGA_SAVE_F0435_OK;
}

int dm1_v1_original_save_amiga_f0435_materialize_active_groups_bytes(
    const uint8_t *bytes, size_t size,
    struct GameWorld_Compat *in_out_world,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435Receipt receipt;
    DM1_V1_SourceActiveGroupPc34Compat rows[DM1_PC34_ACTIVE_GROUP_CAPACITY];
    DM1_V1_GroupStateBundleReceiptPc34Compat bundle_receipt;
    uint8_t *source = NULL;
    size_t source_size = 0u;
    unsigned int index;
    unsigned int live_count = 0u;
    int result;

    if (!bytes || !in_out_world || !in_out_world->dungeon ||
        !in_out_world->things || !in_out_world->things->loaded) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    }
    memset(&receipt, 0, sizeof(receipt));
    result = dm1_v1_original_save_amiga_f0435_receipt_bytes(bytes, size,
                                                             &receipt);
    if (out_receipt) *out_receipt = receipt;
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) return result;
    /* ReDMCSB GROUP.C F0196 configures 60 rows for original Amiga editions.
     * The common native table has 110 slots, enough for that source shape. */
    if (receipt.maximum_active_group_count > DM1_PC34_ACTIVE_GROUP_CAPACITY ||
        receipt.current_active_group_count > receipt.maximum_active_group_count) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY;
    }
    if (receipt.maximum_active_group_count == 0u) {
        if (!dm1_v1_group_state_initialize_f0196_pc34(in_out_world,
                                                       &bundle_receipt)) {
            return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
        }
        return DM1_V1_AMIGA_SAVE_F0435_OK;
    }
    source = (uint8_t *)malloc(receipt.part_byte_counts[1u]);
    if (!source) return DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY;
    result = dm1_v1_original_save_amiga_f0435_part_bytes(
        bytes, size, 1u, source, receipt.part_byte_counts[1u],
        &source_size, NULL);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK ||
        source_size != (size_t)receipt.maximum_active_group_count *
                       DM1_V1_AMIGA_SAVE_F0435_ACTIVE_GROUP_BYTES) {
        free(source);
        return result == DM1_V1_AMIGA_SAVE_F0435_OK
            ? DM1_V1_AMIGA_SAVE_F0435_ERR_BODY : result;
    }
    memset(rows, 0, sizeof(rows));
    /* F0184 leaves a removed row's GroupThingIndex at -1 in place; C1 is
     * therefore a sparse MaximumActiveGroupCount table, not a current-count
     * prefix. Scan every original row, compact only live entries for the
     * native runtime, and reject a checksum-valid but internally inconsistent
     * C0/C1 pair before publishing any active state. */
    for (index = 0u; index < receipt.maximum_active_group_count; ++index) {
        const uint8_t *row = source +
            (size_t)index * DM1_V1_AMIGA_SAVE_F0435_ACTIVE_GROUP_BYTES;
        const int group_thing = read_be_i16(row + 0u);
        if (group_thing == -1) continue;
        if (group_thing < 0 || live_count >= receipt.current_active_group_count) {
            free(source);
            return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
        }
        rows[live_count].groupThing = group_thing;
        rows[live_count].directions = row[2u];
        rows[live_count].cells = row[3u];
        rows[live_count].lastMoveTime = row[4u];
        rows[live_count].delayFleeingFromTarget = row[5u];
        rows[live_count].targetMapX = row[6u];
        rows[live_count].targetMapY = row[7u];
        rows[live_count].priorMapX = row[8u];
        rows[live_count].priorMapY = row[9u];
        rows[live_count].homeMapX = row[10u];
        rows[live_count].homeMapY = row[11u];
        memcpy(rows[live_count].aspect, row + 12u,
               sizeof(rows[live_count].aspect));
        ++live_count;
    }
    free(source);
    if (live_count != receipt.current_active_group_count) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    }
    if (!dm1_v1_group_state_apply_save_handoff_pc34(
            in_out_world, rows, (int)receipt.current_active_group_count,
            (int)receipt.maximum_active_group_count, &bundle_receipt)) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    }
    /* GROUP.Cells is ActiveGroupIndex while a group is on the party map
     * (DEFS.H). Firestaff compacts sparse source C1 slots into its native
     * active-state array, so mirror that new index into the decoded/raw GROUP
     * owner only after the complete C1 handoff has committed. The pristine
     * Amiga tail remains separately retained and untouched. */
    for (index = 0u; index < live_count; ++index) {
        const unsigned int thing = (unsigned int)(uint16_t)rows[index].groupThing;
        const unsigned int type = (thing >> 10u) & 0x0fu;
        const unsigned int group_index = type == THING_TYPE_GROUP
            ? thing & 0x03ffu : thing;
        if (group_index >= (unsigned int)in_out_world->things->groupCount) {
            return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
        }
        in_out_world->things->groups[group_index].cells = (uint8_t)index;
        if (in_out_world->things->rawThingData[THING_TYPE_GROUP]) {
            in_out_world->things->rawThingData[THING_TYPE_GROUP]
                [(size_t)group_index * 16u + 5u] = (uint8_t)index;
        }
    }
    return DM1_V1_AMIGA_SAVE_F0435_OK;
}

int dm1_v1_original_save_amiga_f0435_runtime_queue_receipt_bytes(
    const uint8_t *bytes, size_t size,
    Dm1V1AmigaSaveRuntimeQueueReceipt *out_queue,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435Receipt receipt;
    uint8_t *active_groups = NULL;
    uint8_t *events = NULL;
    uint8_t *heap = NULL;
    size_t active_group_size = 0u, event_size = 0u, heap_size = 0u;
    unsigned int index;
    int result;

    if (!bytes || !out_queue) return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    memset(out_queue, 0, sizeof(*out_queue));
    memset(&receipt, 0, sizeof(receipt));
    result = dm1_v1_original_save_amiga_f0435_receipt_bytes(bytes, size,
                                                             &receipt);
    if (out_receipt) *out_receipt = receipt;
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) return result;
    active_group_size = receipt.part_byte_counts[1u];
    event_size = receipt.part_byte_counts[3u];
    heap_size = receipt.part_byte_counts[4u];
    /* A source-valid fresh session may have zero active groups or no event
     * capacity.  Keep the pointer admission independent of implementation-
     * defined malloc(0), while retaining the authoritative zero lengths. */
    active_groups = (uint8_t *)malloc(active_group_size ? active_group_size : 1u);
    events = (uint8_t *)malloc(event_size ? event_size : 1u);
    heap = (uint8_t *)malloc(heap_size ? heap_size : 1u);
    if (!active_groups || !events || !heap) {
        free(active_groups); free(events); free(heap);
        return DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY;
    }
    if (dm1_v1_original_save_amiga_f0435_part_bytes(
            bytes, size, 1u, active_groups, active_group_size,
            &active_group_size, NULL) != DM1_V1_AMIGA_SAVE_F0435_OK ||
        dm1_v1_original_save_amiga_f0435_part_bytes(
            bytes, size, 3u, events, event_size, &event_size, NULL) !=
            DM1_V1_AMIGA_SAVE_F0435_OK ||
        dm1_v1_original_save_amiga_f0435_part_bytes(
            bytes, size, 4u, heap, heap_size, &heap_size, NULL) !=
            DM1_V1_AMIGA_SAVE_F0435_OK) {
        free(active_groups); free(events); free(heap);
        return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    }
    if (active_group_size != (size_t)receipt.maximum_active_group_count * 16u ||
        event_size != (size_t)receipt.event_maximum_count * 10u ||
        heap_size != (size_t)receipt.event_maximum_count * 2u ||
        receipt.event_count > receipt.event_maximum_count) {
        free(active_groups); free(events); free(heap);
        return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    }
    out_queue->active_group_capacity = receipt.maximum_active_group_count;
    out_queue->active_group_active_count = receipt.current_active_group_count;
    out_queue->event_capacity = receipt.event_maximum_count;
    out_queue->scheduled_event_count = receipt.event_count;
    out_queue->first_unused_event_index = receipt.first_unused_event_index;
    out_queue->active_group_fingerprint = fingerprint_bytes(active_groups,
                                                             active_group_size);
    out_queue->event_fingerprint = fingerprint_bytes(events, event_size);
    out_queue->heap_fingerprint = fingerprint_bytes(heap, heap_size);
    out_queue->timeline_membership_valid = 1;
    for (index = 0u; index < receipt.event_count; ++index) {
        const uint16_t event_index = read_be16(heap + index * 2u);
        unsigned int earlier;
        if (event_index >= receipt.event_maximum_count ||
            events[(size_t)event_index * 10u + 4u] == 0u) {
            out_queue->timeline_membership_valid = 0;
            break;
        }
        for (earlier = 0u; earlier < index; ++earlier) {
            if (read_be16(heap + earlier * 2u) == event_index) {
                out_queue->timeline_membership_valid = 0;
                break;
            }
        }
        if (!out_queue->timeline_membership_valid) break;
        if (index == 0u) {
            const uint8_t *event = events + (size_t)event_index * 10u;
            out_queue->first_scheduled_map_time = read_be32(event);
            out_queue->first_scheduled_type = event[4u];
            out_queue->first_scheduled_priority = event[5u];
            out_queue->first_scheduled_map_x = event[6u];
            out_queue->first_scheduled_map_y = event[7u];
            out_queue->first_scheduled_cell = event[8u];
            out_queue->first_scheduled_effect = event[9u];
        }
    }
    if (out_queue->first_unused_event_index < receipt.event_maximum_count &&
        events[(size_t)out_queue->first_unused_event_index * 10u + 4u] != 0u) {
        out_queue->timeline_membership_valid = 0;
    }
    free(active_groups); free(events); free(heap);
    return out_queue->timeline_membership_valid
        ? DM1_V1_AMIGA_SAVE_F0435_OK : DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
}

int dm1_v1_original_save_amiga_f0435_materialize_event_queue_bytes(
    const uint8_t *bytes, size_t size,
    struct DM1_EventQueue_V1 *out_queue,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435Receipt receipt;
    Dm1V1AmigaSaveRuntimeQueueReceipt queue_receipt;
    struct DM1_EventQueue_V1 candidate;
    uint8_t *events = NULL;
    uint8_t *heap = NULL;
    size_t event_size = 0u, heap_size = 0u;
    unsigned int index;
    int result;

    if (!bytes || !out_queue) return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    memset(&receipt, 0, sizeof(receipt));
    memset(&queue_receipt, 0, sizeof(queue_receipt));
    result = dm1_v1_original_save_amiga_f0435_runtime_queue_receipt_bytes(
        bytes, size, &queue_receipt, &receipt);
    if (out_receipt) *out_receipt = receipt;
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK ||
        receipt.event_maximum_count > DM1_EVENT_MAX_COUNT) {
        return result == DM1_V1_AMIGA_SAVE_F0435_OK
            ? DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY : result;
    }
    event_size = receipt.part_byte_counts[3u];
    heap_size = receipt.part_byte_counts[4u];
    events = (uint8_t *)malloc(event_size ? event_size : 1u);
    heap = (uint8_t *)malloc(heap_size ? heap_size : 1u);
    if (!events || !heap) {
        free(events); free(heap);
        return DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY;
    }
    if (dm1_v1_original_save_amiga_f0435_part_bytes(
            bytes, size, 3u, events, event_size, &event_size, NULL) !=
            DM1_V1_AMIGA_SAVE_F0435_OK ||
        dm1_v1_original_save_amiga_f0435_part_bytes(
            bytes, size, 4u, heap, heap_size, &heap_size, NULL) !=
            DM1_V1_AMIGA_SAVE_F0435_OK) {
        free(events); free(heap);
        return DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
    }
    if (!dm1v1_event_queue_init(&candidate, receipt.game_time)) {
        free(events); free(heap);
        return DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY;
    }
    candidate.eventCount = receipt.event_count;
    candidate.firstUnusedIndex = receipt.first_unused_event_index;
    for (index = 0u; index < receipt.event_maximum_count; ++index) {
        const uint8_t *event = events + (size_t)index * 10u;
        candidate.events[index].map_time = read_be32(event);
        candidate.events[index].type = event[4u];
        candidate.events[index].priority = event[5u];
        candidate.events[index].b_mapX = event[6u];
        candidate.events[index].b_mapY = event[7u];
        candidate.events[index].c_cell = event[8u];
        candidate.events[index].c_effect = event[9u];
    }
    for (index = 0u; index < receipt.event_count; ++index) {
        candidate.timeline[index] = read_be16(heap + index * 2u);
    }
    free(events); free(heap);
    *out_queue = candidate;
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

int dm1_v1_original_save_amiga_f0435_materialize_dungeon_world_bytes(
    const uint8_t *bytes, size_t size,
    struct GameWorld_Compat *out_world,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435Receipt receipt;
    struct DungeonDatState_Compat *dungeon = NULL;
    struct DungeonThings_Compat *things = NULL;
    uint8_t *tail = NULL;
    size_t tail_size = 0u;
    int result;

    if (!bytes || !out_world || out_world->dungeon || out_world->things ||
        out_world->ownsDungeon) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    }
    memset(&receipt, 0, sizeof(receipt));
    result = dm1_v1_original_save_amiga_f0435_receipt_bytes(bytes, size,
                                                             &receipt);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) {
        if (out_receipt) *out_receipt = receipt;
        return result;
    }
    if (!receipt.tail_authenticated || receipt.dungeon_byte_count == 0u ||
        (size_t)receipt.dungeon_byte_count > (size_t)0x7fffffff) {
        if (out_receipt) *out_receipt = receipt;
        return DM1_V1_AMIGA_SAVE_F0435_ERR_TAIL;
    }
    tail = (uint8_t *)malloc(receipt.dungeon_byte_count);
    dungeon = (struct DungeonDatState_Compat *)calloc(1u, sizeof(*dungeon));
    things = (struct DungeonThings_Compat *)calloc(1u, sizeof(*things));
    if (!tail || !dungeon || !things) {
        free(tail);
        free(dungeon);
        free(things);
        if (out_receipt) *out_receipt = receipt;
        return DM1_V1_AMIGA_SAVE_F0435_ERR_CAPACITY;
    }
    result = dm1_v1_original_save_amiga_f0435_dungeon_tail_bytes(
        bytes, size, tail, receipt.dungeon_byte_count, &tail_size, NULL);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK ||
        tail_size != receipt.dungeon_byte_count ||
        !F0505_DUNGEON_LoadTailBufferAmigaBE_Compat(
            tail, (int)tail_size, dungeon, things)) {
        F0504_DUNGEON_FreeThingData_Compat(things);
        F0500_DUNGEON_FreeDatHeader_Compat(dungeon);
        free(things);
        free(dungeon);
        free(tail);
        if (out_receipt) *out_receipt = receipt;
        return DM1_V1_AMIGA_SAVE_F0435_ERR_TAIL;
    }
    /* F0505 receives a transient read-only copy. Retain the exact source
     * tail separately, including its original big-endian checksum word. */
    dungeon->originalSaveTailBytes = tail;
    dungeon->originalSaveTailByteCount = (int)tail_size;
    dungeon->originalSaveTailPristine = 1;
    out_world->dungeon = dungeon;
    out_world->things = things;
    out_world->ownsDungeon = 1;
    out_world->dungeonFingerprint =
        ((uint32_t)receipt.dungeon_actual_checksum << 16) ^ tail_size;
    if (out_receipt) *out_receipt = receipt;
    return DM1_V1_AMIGA_SAVE_F0435_OK;
}

int dm1_v1_original_save_amiga_f0435_materialize_session_bytes(
    const uint8_t *bytes, size_t size,
    struct GameWorld_Compat *out_world,
    struct DM1_EventQueue_V1 *out_event_queue,
    Dm1V1AmigaSaveF0435Receipt *out_receipt)
{
    Dm1V1AmigaSaveF0435Receipt receipt;
    Dm1V1AmigaSaveF0435GlobalData source_global;
    Dm1V1AmigaSavePartyReceipt source_party;
    struct GameWorld_Compat candidate_world;
    struct DM1_EventQueue_V1 candidate_queue;
    unsigned int index;
    int result;

    if (!bytes || !out_world || !out_event_queue || out_world->dungeon ||
        out_world->things || out_world->ownsDungeon) {
        return DM1_V1_AMIGA_SAVE_F0435_ERR_ARGUMENT;
    }
    memset(&receipt, 0, sizeof(receipt));
    memset(&source_global, 0, sizeof(source_global));
    memset(&source_party, 0, sizeof(source_party));
    memset(&candidate_world, 0, sizeof(candidate_world));
    memset(&candidate_queue, 0, sizeof(candidate_queue));
    result = dm1_v1_original_save_amiga_f0435_receipt_bytes(bytes, size,
                                                             &receipt);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) goto done;
    if (receipt.event_count > TIMELINE_QUEUE_CAPACITY) {
        result = DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
        goto done;
    }
    result = dm1_v1_original_save_amiga_f0435_materialize_dungeon_world_bytes(
        bytes, size, &candidate_world, NULL);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) goto done;
    result = dm1_v1_original_save_amiga_f0435_materialize_party_bytes(
        bytes, size, &candidate_world.party, NULL);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) goto done;
    /* C1 rows are live on the saved party map. Publish that source value
     * before the transactional F0145/F0146/F0147/F0196 owner handoff. */
    candidate_world.partyMapIndex = candidate_world.party.mapIndex;
    candidate_world.newPartyMapIndex = candidate_world.party.mapIndex;
    result = dm1_v1_original_save_amiga_f0435_global_data_bytes(
        bytes, size, &source_global, NULL);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) goto done;
    /* A20 C2's 128-byte PARTY_INFO is an authenticated, big-endian source
     * record.  Bind every named status scalar directly from that record;
     * these are not PC34 defaults and must not be cleared on resume.  The
     * scent arrays, rest state, RNG, and last-attack time remain unavailable
     * here and therefore stay fail-closed rather than being reconstructed. */
    result = dm1_v1_original_save_amiga_f0435_party_receipt_bytes(
        bytes, size, &source_party, NULL);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) goto done;
    candidate_world.magic.magicalLightAmount = source_party.magical_light_amount;
    candidate_world.magic.event73CountThievesEye = source_party.thieves_eye_count;
    candidate_world.magic.event79CountFootprints = source_party.footprints_count;
    candidate_world.magic.event71CountInvisibility = source_party.invisibility_count;
    candidate_world.magic.magicFootprintsActive =
        source_party.footprints_count > 0u;
    candidate_world.magic.partyShieldDefense = source_party.shield_defense;
    candidate_world.magic.fireShieldDefense = source_party.fire_shield_defense;
    candidate_world.magic.spellShieldDefense = source_party.spell_shield_defense;
    candidate_world.magic.scentCount = source_party.scent_count;
    candidate_world.magic.freezeLifeTicks = source_party.freeze_life_ticks;
    candidate_world.magic.firstScentIndex = source_party.first_scent_index;
    candidate_world.freezeLifeTicks = source_party.freeze_life_ticks;
    /* GLOBAL_DATA.LastRandomNumber is G0349's exact LCG state (BASE.C),
     * not a host-selected seed. Preserve it before any M10 consumer can
     * request the next random value. */
    if (!F0730_COMBAT_RngInit_Compat(&candidate_world.masterRng,
                                     source_global.last_random_number)) {
        result = DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
        goto done;
    }
    candidate_world.disabledMovementTicks = source_global.disabled_movement_ticks;
    candidate_world.projectileDisabledMovementTicks =
        source_global.projectile_disabled_movement_ticks;
    candidate_world.lastProjectileDisabledMovementDirection =
        source_global.last_projectile_disabled_movement_direction;
    if (candidate_world.party.mapIndex < 0 ||
        candidate_world.party.mapIndex >= candidate_world.dungeon->header.mapCount ||
        candidate_world.party.mapX < 0 || candidate_world.party.mapY < 0 ||
        candidate_world.party.mapX >=
            candidate_world.dungeon->maps[candidate_world.party.mapIndex].width ||
        candidate_world.party.mapY >=
            candidate_world.dungeon->maps[candidate_world.party.mapIndex].height ||
        candidate_world.party.direction < 0 || candidate_world.party.direction > 3 ||
        candidate_world.party.championCount < 0 ||
        candidate_world.party.championCount > CHAMPION_MAX_PARTY ||
        candidate_world.party.activeChampionIndex < -1 ||
        candidate_world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) {
        result = DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
        goto done;
    }
    result = dm1_v1_original_save_amiga_f0435_materialize_active_groups_bytes(
        bytes, size, &candidate_world, NULL);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) goto done;
    result = dm1_v1_original_save_amiga_f0435_materialize_event_queue_bytes(
        bytes, size, &candidate_queue, NULL);
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) goto done;
    if (!F0720_TIMELINE_Init_Compat(&candidate_world.timeline,
                                    receipt.game_time)) {
        result = DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
        goto done;
    }
    for (index = 0u; index < candidate_queue.eventCount; ++index) {
        const uint16_t event_index = candidate_queue.timeline[index];
        const struct DM1_Event_V1 *source;
        struct TimelineEvent_Compat event;
        if (event_index >= candidate_queue.maxEvents) {
            result = DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
            goto done;
        }
        source = &candidate_queue.events[event_index];
        /* ReDMCSB TIMELINE.C F0256 C53 uses only Type and low 24 bits of
         * Map_Time.  Its otherwise populated B/C bytes in this real save
         * are not adopted as invented watchdog state. */
        if (source->type != DM1_EVENT_WATCHDOG ||
            (source->map_time >> 24) != 0u) {
            result = DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
            goto done;
        }
        memset(&event, 0, sizeof(event));
        event.kind = TIMELINE_EVENT_WATCHDOG;
        event.fireAtTick = source->map_time & 0x00ffffffu;
        event.aux0 = DM1_EVENT_WATCHDOG;
        event.aux2 = DM1_EVENT_WATCHDOG;
        if (!F0721_TIMELINE_Schedule_Compat(&candidate_world.timeline,
                                            &event)) {
            result = DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
            goto done;
        }
    }
    candidate_world.gameTick = receipt.game_time;
    /* F0859 derives per-champion lifecycle data only from the restored C2
     * champion records.  Re-apply the independent PARTY_INFO status words
     * after that initializer, exactly as separate source data rather than
     * treating its zeroed status structure as saved state. */
    if (!F0859_LIFECYCLE_Init_Compat(&candidate_world.lifecycle,
                                     &candidate_world.party)) {
        result = DM1_V1_AMIGA_SAVE_F0435_ERR_BODY;
        goto done;
    }
    candidate_world.lifecycle.gameTime = receipt.game_time;
    candidate_world.lifecycle.lastCreatureAttackTime =
        (uint32_t)source_global.last_creature_attack_time;
    candidate_world.lifecycle.status.partyShieldDefense = source_party.shield_defense;
    candidate_world.lifecycle.status.partyFireShieldDefense =
        source_party.fire_shield_defense;
    candidate_world.lifecycle.status.partySpellShieldDefense =
        source_party.spell_shield_defense;
    candidate_world.lifecycle.status.invisibilityCount =
        source_party.invisibility_count;
    candidate_world.lifecycle.status.thievesEyeCount =
        source_party.thieves_eye_count;
    candidate_world.lifecycle.status.footprintsCount =
        source_party.footprints_count;
    *out_world = candidate_world;
    *out_event_queue = candidate_queue;
    memset(&candidate_world, 0, sizeof(candidate_world));
    result = DM1_V1_AMIGA_SAVE_F0435_OK;

done:
    if (out_receipt) *out_receipt = receipt;
    if (result != DM1_V1_AMIGA_SAVE_F0435_OK) {
        free_unpublished_world(&candidate_world);
    }
    return result;
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
