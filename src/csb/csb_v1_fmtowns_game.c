#include "csb_v1_fmtowns_game.h"

#include "redmcsb_f7061_save_header_pc34_compat.h"
#include "redmcsb_f7055_saveutil_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    CSB_V1_FMTOWNS_CHTWE_SIZE = 283936u,
    CSB_V1_FMTOWNS_CHTWJ_SIZE = 284416u,
    CSB_V1_FMTOWNS_CHTWE_FNV1A = 0x3da136f6u,
    CSB_V1_FMTOWNS_CHTWJ_FNV1A = 0xf937db45u,
    CSB_V1_FMTOWNS_UTILE_SIZE = 152387u,
    CSB_V1_FMTOWNS_UTILJ_SIZE = 152499u,
    CSB_V1_FMTOWNS_UTILE_FNV1A = 0xff240e0cu,
    CSB_V1_FMTOWNS_UTILJ_FNV1A = 0xbb3b47c2u,
    CSB_V1_FMTOWNS_CDATA_MINI_SIZE = 42776u,
    CSB_V1_FMTOWNS_CJDATA_MINI_SIZE = 43208u,
    CSB_V1_FMTOWNS_CDATA_MINI_FNV1A = 0x494999c9u,
    CSB_V1_FMTOWNS_CJDATA_MINI_FNV1A = 0x284799d1u,
    CSB_V1_FMTOWNS_SAVE_HEADER_BYTES = 512u,
    CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX = 29u,
    CSB_V1_FMTOWNS_SAVE_HEADER_USELESS_OFFSET = 0x12cu,
    CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_OFFSET = 0x12du,
    CSB_V1_FMTOWNS_SAVE_HEADER_PLATFORM_OFFSET = 0x178u,
    CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_ID_OFFSET = 0x17au,
    CSB_V1_FMTOWNS_SAVE_HEADER_KEYS_OFFSET = 0x138u,
    CSB_V1_FMTOWNS_SAVE_HEADER_CHECKSUMS_OFFSET = 0x158u,
    CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_C5 = 5u,
    CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_CSB_GAME = 13u,
    CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES = 128u,
    CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES = 16u,
    CSB_V1_FMTOWNS_CHAMPION_PARTY_BYTES = 1404u,
    CSB_V1_FMTOWNS_EVENT_BYTES = 10u,
    CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES = 2u,
    CSB_V1_FMTOWNS_GLOBAL_PARTY_CHAMPION_COUNT_OFFSET = 10u,
    CSB_V1_FMTOWNS_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET = 28u,
    CSB_V1_FMTOWNS_GLOBAL_ACTIVE_GROUP_CAPACITY_OFFSET = 46u,
    CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES = 464u,
    CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT = 4u,
    CSB_V1_FMTOWNS_DUNGEON_HEADER_BYTES = 44u,
    CSB_V1_FMTOWNS_DUNGEON_MAP_BYTES = 16u,
    CSB_V1_FMTOWNS_DUNGEON_MAP_WIDTH_OFFSET = 8u,
    CSB_V1_FMTOWNS_DUNGEON_RAW_MAP_BYTES_OFFSET = 2u,
    CSB_V1_FMTOWNS_DUNGEON_MAP_COUNT_OFFSET = 4u,
    CSB_V1_FMTOWNS_DUNGEON_TEXT_WORD_COUNT_OFFSET = 6u,
    CSB_V1_FMTOWNS_DUNGEON_SQUARE_FIRST_THING_COUNT_OFFSET = 10u,
    CSB_V1_FMTOWNS_DUNGEON_THING_COUNTS_OFFSET = 12u,
    CSB_V1_FMTOWNS_DUNGEON_TRAILER_BYTES = 2u,
    CSB_V1_FMTOWNS_UTILE_MENU_VIRTUAL_OFFSET = 0x11578u,
    CSB_V1_FMTOWNS_UTILJ_MENU_VIRTUAL_OFFSET = 0x11628u,
    CSB_V1_FMTOWNS_UTILE_MENU_BYTES = 76u,
    CSB_V1_FMTOWNS_UTILJ_MENU_BYTES = 68u,
    CSB_V1_FMTOWNS_UTILE_MENU_FNV1A = 0xfd9986bfu,
    CSB_V1_FMTOWNS_UTILJ_MENU_FNV1A = 0xdceefc60u,
    CSB_V1_FMTOWNS_UTILE_ICON_PALETTE_OFFSET = 0x17db0u,
    CSB_V1_FMTOWNS_UTILJ_ICON_PALETTE_OFFSET = 0x17e18u,
    /* The retail F31E/F31J programs carry identical 10*32*32 selector
     * tables. These offsets are from the raw verified executable image. */
    CSB_V1_FMTOWNS_CHTWE_MUSIC_TABLE_OFFSET = 271144u,
    CSB_V1_FMTOWNS_CHTWJ_MUSIC_TABLE_OFFSET = 271624u,
    CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A = 0x3faffb70u
};

static int csb_v1_fmtowns_game_read_span(const char *path, uint32_t offset,
                                         unsigned char *bytes, size_t size);
static uint16_t csb_v1_fmtowns_game_read_le16(const unsigned char *bytes);

static const uint8_t k_csb_v1_fmtowns_thing_data_bytes[16] = {
    4u, 6u, 4u, 8u, 16u, 4u, 4u, 4u,
    4u, 8u, 4u, 0u, 0u, 0u, 8u, 4u
};

static int csb_v1_fmtowns_game_read_sum_span(
    const char *path, uint32_t file_size, uint32_t *in_out_offset,
    uint32_t byte_count, uint16_t *in_out_checksum)
{
    unsigned char *bytes;
    uint32_t index;

    if (!path || !in_out_offset || !in_out_checksum ||
        *in_out_offset > file_size || byte_count > file_size - *in_out_offset)
        return 0;
    if (byte_count == 0u) return 1;
    bytes = (unsigned char *)malloc(byte_count);
    if (!bytes || !csb_v1_fmtowns_game_read_span(
                      path, *in_out_offset, bytes, byte_count)) {
        free(bytes);
        return 0;
    }
    for (index = 0u; index < byte_count; ++index)
        *in_out_checksum = (uint16_t)(*in_out_checksum + bytes[index]);
    free(bytes);
    *in_out_offset += byte_count;
    return 1;
}

static int csb_v1_fmtowns_game_startup_mini_dungeon_tail_open(
    const char *path, uint32_t file_size,
    CSB_V1_FmtownsGameHandoffReceipt *receipt)
{
    unsigned char header[CSB_V1_FMTOWNS_DUNGEON_HEADER_BYTES];
    unsigned char maps[255u * CSB_V1_FMTOWNS_DUNGEON_MAP_BYTES];
    uint32_t offset;
    uint32_t map_bytes;
    uint16_t checksum = 0u;
    uint16_t column_count = 0u;
    uint16_t raw_map_byte_count;
    uint16_t text_data_word_count;
    uint16_t square_first_thing_count;
    uint16_t saved_checksum;
    uint32_t map_index;
    uint32_t type;
    uint8_t map_count;

    if (!path || !receipt || file_size <
            CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES *
                CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT ||
        receipt->startup_mini_verified_save_body_offset >
            file_size - CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES *
                        CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT) return 0;
    offset = receipt->startup_mini_verified_save_body_offset +
             CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES *
                 CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT;
    if (!csb_v1_fmtowns_game_read_span(path, offset, header, sizeof(header)))
        return 0;
    for (map_index = 0u; map_index < sizeof(header); ++map_index)
        checksum = (uint16_t)(checksum + header[map_index]);
    raw_map_byte_count = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_DUNGEON_RAW_MAP_BYTES_OFFSET);
    map_count = header[CSB_V1_FMTOWNS_DUNGEON_MAP_COUNT_OFFSET];
    text_data_word_count = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_DUNGEON_TEXT_WORD_COUNT_OFFSET);
    square_first_thing_count = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_DUNGEON_SQUARE_FIRST_THING_COUNT_OFFSET);
    map_bytes = (uint32_t)map_count * CSB_V1_FMTOWNS_DUNGEON_MAP_BYTES;
    if (map_bytes > sizeof(maps) || offset > file_size - sizeof(header) ||
        map_bytes > file_size - offset - sizeof(header) ||
        !csb_v1_fmtowns_game_read_span(path, offset + sizeof(header), maps,
                                       map_bytes)) return 0;
    for (map_index = 0u; map_index < map_bytes; ++map_index)
        checksum = (uint16_t)(checksum + maps[map_index]);
    for (map_index = 0u; map_index < map_count; ++map_index) {
        uint16_t descriptor = csb_v1_fmtowns_game_read_le16(
            maps + map_index * CSB_V1_FMTOWNS_DUNGEON_MAP_BYTES +
            CSB_V1_FMTOWNS_DUNGEON_MAP_WIDTH_OFFSET);
        column_count = (uint16_t)(column_count + ((descriptor >> 6) & 0x1fu) +
                                  1u);
    }
    offset += sizeof(header) + map_bytes;
    if (!csb_v1_fmtowns_game_read_sum_span(
            path, file_size, &offset, (uint32_t)column_count * 2u, &checksum) ||
        !csb_v1_fmtowns_game_read_sum_span(
            path, file_size, &offset, (uint32_t)square_first_thing_count * 2u,
            &checksum) ||
        !csb_v1_fmtowns_game_read_sum_span(
            path, file_size, &offset, (uint32_t)text_data_word_count * 2u,
            &checksum)) return 0;
    for (type = 0u; type < 16u; ++type) {
        uint16_t count = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_DUNGEON_THING_COUNTS_OFFSET + type * 2u);
        if (!csb_v1_fmtowns_game_read_sum_span(
                path, file_size, &offset,
                (uint32_t)count * k_csb_v1_fmtowns_thing_data_bytes[type],
                &checksum)) return 0;
    }
    if (!csb_v1_fmtowns_game_read_sum_span(
            path, file_size, &offset, raw_map_byte_count, &checksum) ||
        offset > file_size - CSB_V1_FMTOWNS_DUNGEON_TRAILER_BYTES ||
        !csb_v1_fmtowns_game_read_span(path, offset, header,
                                       CSB_V1_FMTOWNS_DUNGEON_TRAILER_BYTES))
        return 0;
    saved_checksum = csb_v1_fmtowns_game_read_le16(header);
    if (saved_checksum != checksum ||
        offset + CSB_V1_FMTOWNS_DUNGEON_TRAILER_BYTES != file_size) return 0;
    receipt->startup_mini_dungeon_map_count = map_count;
    receipt->startup_mini_dungeon_column_count = column_count;
    receipt->startup_mini_dungeon_tail_checksum = checksum;
    receipt->startup_mini_first_map_offset_x = maps[6u];
    receipt->startup_mini_first_map_offset_y = maps[7u];
    receipt->startup_mini_dungeon_tail_offset =
        receipt->startup_mini_verified_save_body_offset +
        CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_BYTES *
            CSB_V1_FMTOWNS_EXTERNAL_PORTRAIT_COUNT;
    receipt->startup_mini_dungeon_tail_size = offset -
        receipt->startup_mini_dungeon_tail_offset;
    receipt->startup_mini_dungeon_tail_verified = 1;
    return 1;
}

int csb_v1_fmtowns_game_copy_verified_dungeon_tail(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    uint8_t *out_bytes, size_t out_size)
{
    if (!receipt || !receipt->valid || !receipt->startup_mini_verified ||
        !receipt->startup_mini_dungeon_tail_verified || !out_bytes ||
        receipt->startup_mini_dungeon_tail_size == 0u ||
        out_size != receipt->startup_mini_dungeon_tail_size) return 0;
    return csb_v1_fmtowns_game_read_span(
        receipt->startup_mini_path, receipt->startup_mini_dungeon_tail_offset,
        out_bytes, out_size);
}

static int csb_v1_fmtowns_game_startup_mini_save_parts_open(
    const char *path, uint32_t file_size, const unsigned char *header,
    CSB_V1_FmtownsGameHandoffReceipt *receipt)
{
    uint16_t keys[5];
    uint16_t checksums[5];
    uint32_t part_sizes[5];
    unsigned char global_data[CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES];
    unsigned char *part_data = NULL;
    uint32_t offset = CSB_V1_FMTOWNS_SAVE_HEADER_BYTES;
    uint32_t index;
    uint16_t event_maximum_count;
    uint16_t active_group_capacity;

    if (!path || !header || !receipt ||
        !csb_v1_fmtowns_game_read_span(path, offset, global_data,
                                       sizeof(global_data))) return 0;
    for (index = 0u; index < 5u; ++index) {
        keys[index] = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_KEYS_OFFSET + index * 2u);
        checksums[index] = csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_CHECKSUMS_OFFSET + index * 2u);
    }
    if (!redmcsb_f7057_read_save_part_with_checksum_pc34(
            global_data, sizeof(global_data), keys[0], checksums[0])) return 0;
    event_maximum_count = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_EVENT_MAXIMUM_COUNT_OFFSET);
    active_group_capacity = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_ACTIVE_GROUP_CAPACITY_OFFSET);
    if (event_maximum_count == 0u || active_group_capacity == 0u) return 0;
    part_sizes[0] = CSB_V1_FMTOWNS_GLOBAL_DATA_BYTES;
    part_sizes[1] = active_group_capacity * CSB_V1_FMTOWNS_ACTIVE_GROUP_BYTES;
    part_sizes[2] = CSB_V1_FMTOWNS_CHAMPION_PARTY_BYTES;
    part_sizes[3] = event_maximum_count * CSB_V1_FMTOWNS_EVENT_BYTES;
    part_sizes[4] = event_maximum_count * CSB_V1_FMTOWNS_TIMELINE_ENTRY_BYTES;
    offset += part_sizes[0];
    for (index = 1u; index < 5u; ++index) {
        if (offset > file_size || part_sizes[index] > file_size - offset ||
            (part_sizes[index] & 1u) != 0u ||
            !(part_data = (unsigned char *)malloc(part_sizes[index])) ||
            !csb_v1_fmtowns_game_read_span(path, offset, part_data,
                                            part_sizes[index]) ||
            !redmcsb_f7057_read_save_part_with_checksum_pc34(
                part_data, part_sizes[index], keys[index], checksums[index])) {
            free(part_data);
            return 0;
        }
        free(part_data);
        part_data = NULL;
        offset += part_sizes[index];
    }
    receipt->startup_mini_party_champion_count = csb_v1_fmtowns_game_read_le16(
        global_data + CSB_V1_FMTOWNS_GLOBAL_PARTY_CHAMPION_COUNT_OFFSET);
    receipt->startup_mini_event_maximum_count = event_maximum_count;
    receipt->startup_mini_active_group_capacity = active_group_capacity;
    receipt->startup_mini_verified_save_body_offset = offset;
    receipt->startup_mini_save_parts_verified = 1;
    return csb_v1_fmtowns_game_startup_mini_dungeon_tail_open(
        path, file_size, receipt);
}

static int csb_v1_fmtowns_game_startup_mini_header_open(
    const char *path, uint16_t expected_platform,
    CSB_V1_FmtownsGameHandoffReceipt *receipt)
{
    unsigned char header[CSB_V1_FMTOWNS_SAVE_HEADER_BYTES];
    uint16_t key;

    if (!path || !receipt ||
        !csb_v1_fmtowns_game_read_span(path, 0u, header, sizeof(header))) {
        return 0;
    }
    key = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX * 2u);
    /* ReDMCSB CEDTINCD.C F7051 lines 211-255 selects the CSB word-29
     * header route for F31E/F31J. DEFS.H defines C5 as the family including
     * FM Towns CSB. F7061 validates and deobfuscates the second 256 bytes. */
    if (!redmcsb_f7061_is_read_save_header_successful_pc34(
            header, sizeof(header),
            CSB_V1_FMTOWNS_CSB_HEADER_KEY_WORD_INDEX) ||
        header[CSB_V1_FMTOWNS_SAVE_HEADER_USELESS_OFFSET] != 1u ||
        header[CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_OFFSET] !=
            CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_C5 ||
        csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_PLATFORM_OFFSET) !=
            expected_platform ||
        csb_v1_fmtowns_game_read_le16(
            header + CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_ID_OFFSET) !=
            CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_CSB_GAME) {
        return 0;
    }
    receipt->startup_mini_header_key = key;
    receipt->startup_mini_header_format_id =
        header[CSB_V1_FMTOWNS_SAVE_HEADER_FORMAT_OFFSET];
    receipt->startup_mini_header_platform = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_SAVE_HEADER_PLATFORM_OFFSET);
    receipt->startup_mini_header_dungeon_id = csb_v1_fmtowns_game_read_le16(
        header + CSB_V1_FMTOWNS_SAVE_HEADER_DUNGEON_ID_OFFSET);
    receipt->startup_mini_header_verified = 1;
    return csb_v1_fmtowns_game_startup_mini_save_parts_open(
        path, receipt->startup_mini_size, header, receipt);
}

static int csb_v1_fmtowns_utility_icon_palette_open(
    const char *path, uint32_t file_offset,
    CSB_V1_FmtownsUtilityHandoffReceipt *receipt)
{
    uint8_t source[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_RECORD_BYTES];
    uint32_t index;

    if (!path || !receipt ||
        !csb_v1_fmtowns_game_read_span(path, file_offset, source,
                                       sizeof(source))) return 0;
    for (index = 0u;
         index < CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT;
         ++index) {
        const uint8_t *entry = source + index * 4u;
        if (entry[0] != index || entry[1] > 0x3fu || entry[2] > 0x3fu ||
            entry[3] > 0x3fu) return 0;
        receipt->icon_palette_rgb6[index][0] = entry[1];
        receipt->icon_palette_rgb6[index][1] = entry[2];
        receipt->icon_palette_rgb6[index][2] = entry[3];
    }
    if (source[64] != 0xffu || source[65] != 0u || source[66] != 0u ||
        source[67] != 0u) return 0;
    receipt->icon_palette_file_offset = file_offset;
    receipt->icon_palette_verified = 1;
    return 1;
}

int csb_v1_fmtowns_utility_icon_palette_rgb6(
    const CSB_V1_FmtownsUtilityMenuReceipt *receipt,
    uint8_t out_rgb6[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3])
{
    if (!receipt || !receipt->valid || !receipt->icon_palette_verified ||
        !out_rgb6) return 0;
    memcpy(out_rgb6, receipt->icon_palette_rgb6, sizeof(receipt->icon_palette_rgb6));
    return 1;
}

static uint32_t csb_v1_fmtowns_game_bytes_fnv1a(const unsigned char *bytes,
                                                  size_t size)
{
    uint32_t hash = 2166136261u;
    size_t index;

    if (!bytes) return 0u;
    for (index = 0u; index < size; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static uint16_t csb_v1_fmtowns_game_read_le16(const unsigned char *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

static uint32_t csb_v1_fmtowns_game_read_le32(const unsigned char *bytes)
{
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static int csb_v1_fmtowns_game_read_span(const char *path, uint32_t offset,
                                         unsigned char *bytes, size_t size)
{
    FILE *file;

    if (!path || !path[0] || !bytes || size == 0u) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, (long)offset, SEEK_SET) != 0 ||
        fread(bytes, 1u, size, file) != size || ferror(file)) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

static uint32_t csb_v1_fmtowns_game_file_fnv1a(const char *path,
                                                 uint32_t *out_size)
{
    FILE *file;
    unsigned char buffer[4096];
    size_t count;
    uint32_t hash = 2166136261u;
    uint32_t size = 0u;

    if (out_size) *out_size = 0u;
    if (!path || !path[0]) return 0u;
    file = fopen(path, "rb");
    if (!file) return 0u;
    while ((count = fread(buffer, 1u, sizeof(buffer), file)) != 0u) {
        size_t index;
        if (size > UINT32_MAX - (uint32_t)count) {
            fclose(file);
            return 0u;
        }
        for (index = 0u; index < count; ++index) {
            hash ^= buffer[index];
            hash *= 16777619u;
        }
        size += (uint32_t)count;
    }
    if (ferror(file)) {
        fclose(file);
        return 0u;
    }
    fclose(file);
    if (out_size) *out_size = size;
    return hash;
}

static int csb_v1_fmtowns_utility_p3_header_open(
    const char *path, uint32_t expected_file_size,
    CSB_V1_FmtownsUtilityHandoffReceipt *receipt)
{
    unsigned char header[0x78];
    uint32_t header_size;
    uint32_t declared_file_size;
    uint32_t runtime_offset;
    uint32_t runtime_size;
    uint32_t load_offset;
    uint32_t load_size;
    uint32_t memory_size;
    uint32_t initial_eip;

    if (!path || !receipt ||
        !csb_v1_fmtowns_game_read_span(path, 0u, header, sizeof(header)) ||
        header[0] != 'P' || header[1] != '3' ||
        csb_v1_fmtowns_game_read_le16(header + 2u) != 1u) return 0;
    header_size = csb_v1_fmtowns_game_read_le16(header + 4u);
    declared_file_size = csb_v1_fmtowns_game_read_le32(header + 6u);
    runtime_offset = csb_v1_fmtowns_game_read_le32(header + 0x0cu);
    runtime_size = csb_v1_fmtowns_game_read_le32(header + 0x10u);
    load_offset = csb_v1_fmtowns_game_read_le32(header + 0x26u);
    load_size = csb_v1_fmtowns_game_read_le32(header + 0x2au);
    initial_eip = csb_v1_fmtowns_game_read_le32(header + 0x68u);
    memory_size = csb_v1_fmtowns_game_read_le32(header + 0x74u);
    if (header_size < 0x80u || header_size > expected_file_size ||
        declared_file_size != expected_file_size ||
        runtime_offset < header_size || runtime_offset > expected_file_size ||
        runtime_size > expected_file_size - runtime_offset ||
        load_offset < header_size || load_offset > expected_file_size ||
        load_size > expected_file_size - load_offset ||
        memory_size < load_size || initial_eip >= memory_size) return 0;
    receipt->p3_header_verified = 1;
    receipt->p3_header_size = header_size;
    receipt->p3_load_image_offset = load_offset;
    receipt->p3_load_image_size = load_size;
    receipt->p3_initial_eip = initial_eip;
    return 1;
}

int csb_v1_fmtowns_game_handoff_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsGameHandoffReceipt *out_receipt)
{
    const char *name;
    uint32_t expected_size;
    uint32_t expected_hash;
    uint32_t actual_size;
    uint32_t actual_hash;
    uint32_t music_table_offset;
    const char *mini_name;
    uint32_t mini_expected_size;
    uint32_t mini_expected_hash;
    uint32_t mini_actual_size;
    uint32_t mini_actual_hash;
    uint16_t mini_expected_platform;
    unsigned char music_table[CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_BYTES];
    CSB_V1_VariantId expected_variant;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->assets_verified || !profile->graphics_verified ||
        !profile->dungeon_verified || !profile->asset_root[0]) return 0;

    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        name = "CHTWE.EXP";
        expected_size = CSB_V1_FMTOWNS_CHTWE_SIZE;
        expected_hash = CSB_V1_FMTOWNS_CHTWE_FNV1A;
        music_table_offset = CSB_V1_FMTOWNS_CHTWE_MUSIC_TABLE_OFFSET;
        mini_name = "CDATA/MINI.DAT";
        mini_expected_size = CSB_V1_FMTOWNS_CDATA_MINI_SIZE;
        mini_expected_hash = CSB_V1_FMTOWNS_CDATA_MINI_FNV1A;
        mini_expected_platform = 7u;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_EN;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        name = "CHTWJ.EXP";
        expected_size = CSB_V1_FMTOWNS_CHTWJ_SIZE;
        expected_hash = CSB_V1_FMTOWNS_CHTWJ_FNV1A;
        music_table_offset = CSB_V1_FMTOWNS_CHTWJ_MUSIC_TABLE_OFFSET;
        mini_name = "CJDATA/MINI.DAT";
        mini_expected_size = CSB_V1_FMTOWNS_CJDATA_MINI_SIZE;
        mini_expected_hash = CSB_V1_FMTOWNS_CJDATA_MINI_FNV1A;
        mini_expected_platform = 8u;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_JA;
    } else {
        return 0;
    }
    if (profile->variant_id != expected_variant ||
        snprintf(out_receipt->executable_path,
                 sizeof(out_receipt->executable_path), "%s/%s",
                 profile->asset_root, name) < 0 ||
        strlen(out_receipt->executable_path) >=
            sizeof(out_receipt->executable_path)) return 0;

    actual_hash = csb_v1_fmtowns_game_file_fnv1a(
        out_receipt->executable_path, &actual_size);
    if (actual_size != expected_size || actual_hash != expected_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* ReDMCSB MUSIC.C G4099 (line 6) is indexed in F0743 at lines 632-646.
     * Bind that exact 10*32*32 payload from the already authenticated F31
     * executable, rather than recreating a coordinate-to-music table. */
    if (!csb_v1_fmtowns_game_read_span(out_receipt->executable_path,
                                       music_table_offset, music_table,
                                       sizeof(music_table)) ||
        csb_v1_fmtowns_game_bytes_fnv1a(music_table,
                                        sizeof(music_table)) !=
            CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->language = language;
    out_receipt->variant_id = profile->variant_id;
    out_receipt->executable_size = actual_size;
    out_receipt->executable_fnv1a = actual_hash;
    out_receipt->executable_verified = 1;
    out_receipt->language_matches_profile = 1;
    out_receipt->game_program_is_c03_game = 1;
    snprintf(out_receipt->executable_name, sizeof(out_receipt->executable_name),
             "%s", name);
    snprintf(out_receipt->graphics_md5, sizeof(out_receipt->graphics_md5),
             "%s", profile->graphics_md5);
    snprintf(out_receipt->dungeon_md5, sizeof(out_receipt->dungeon_md5),
             "%s", profile->dungeon_md5);
    /* ReDMCSB CEDTDATA.C G2297 lines 380-387 selects CDATA/CJDATA MINI.DAT
     * for F31E/F31J. F0435 then reads its native 512-byte save header. The
     * shipped seed is recorded by exact bytes here, never decoded as the
     * unrelated big-endian Atari/Amiga GAMEBLOCK layout. */
    if (snprintf(out_receipt->startup_mini_path,
                 sizeof(out_receipt->startup_mini_path), "%s/%s",
                 profile->asset_root, mini_name) >= 0 &&
        strlen(out_receipt->startup_mini_path) <
            sizeof(out_receipt->startup_mini_path)) {
        mini_actual_hash = csb_v1_fmtowns_game_file_fnv1a(
            out_receipt->startup_mini_path, &mini_actual_size);
        out_receipt->startup_mini_size = mini_actual_size;
        out_receipt->startup_mini_fnv1a = mini_actual_hash;
        out_receipt->startup_mini_verified =
            mini_actual_size == mini_expected_size &&
            mini_actual_hash == mini_expected_hash;
        if (!out_receipt->startup_mini_verified ||
            !csb_v1_fmtowns_game_startup_mini_header_open(
                out_receipt->startup_mini_path, mini_expected_platform,
                out_receipt)) {
            memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
    }
    out_receipt->music_table_verified = 1;
    out_receipt->music_table_source_offset = music_table_offset;
    out_receipt->music_table_size = sizeof(music_table);
    out_receipt->music_table_fnv1a = CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A;
    out_receipt->source_evidence =
        "ReDMCSB COMPILE.H EXEID 60/61 lines 367-375; "
        "STARTUP1.C F0435 line 163; CEDTDATA.C G2297 lines 380-387/F7051 "
        "lines 211-255; CEDTINC6.C F7061/F7057/F7059; CEDTINCA.C F7063; "
        "CEDTINCT.C F7054; "
        "DEFS.H C5/F7/F8/C13; "
        "ENTRANCE.C F0807 line 85; "
        "MUSIC.C G4099 line 6/F0743 lines 632-646";
    out_receipt->valid = 1;
    return 1;
}

int csb_v1_fmtowns_utility_handoff_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityHandoffReceipt *out_receipt)
{
    const char *name;
    uint32_t expected_size;
    uint32_t expected_hash;
    uint32_t icon_palette_offset;
    uint32_t actual_size;
    uint32_t actual_hash;
    CSB_V1_VariantId expected_variant;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!profile || !profile->assets_verified || !profile->graphics_verified ||
        !profile->dungeon_verified || !profile->asset_root[0]) return 0;
    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        name = "UTILE.EXP";
        expected_size = CSB_V1_FMTOWNS_UTILE_SIZE;
        expected_hash = CSB_V1_FMTOWNS_UTILE_FNV1A;
        icon_palette_offset = CSB_V1_FMTOWNS_UTILE_ICON_PALETTE_OFFSET;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_EN;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        name = "UTILJ.EXP";
        expected_size = CSB_V1_FMTOWNS_UTILJ_SIZE;
        expected_hash = CSB_V1_FMTOWNS_UTILJ_FNV1A;
        icon_palette_offset = CSB_V1_FMTOWNS_UTILJ_ICON_PALETTE_OFFSET;
        expected_variant = CSB_V1_VARIANT_FMTOWNS_JA;
    } else return 0;
    if (profile->variant_id != expected_variant ||
        snprintf(out_receipt->executable_path,
                 sizeof(out_receipt->executable_path), "%s/%s",
                 profile->asset_root, name) < 0 ||
        strlen(out_receipt->executable_path) >=
            sizeof(out_receipt->executable_path)) return 0;
    actual_hash = csb_v1_fmtowns_game_file_fnv1a(
        out_receipt->executable_path, &actual_size);
    if (actual_size != expected_size || actual_hash != expected_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* COMPILE.H EXEID 63/64 lines 379-385 identifies these P3 executables
     * as separate C06_CEDT programs. Bind their native entry envelopes before
     * any future TBIOS/CEDT decoder consumes a menu or save command. */
    if (!csb_v1_fmtowns_utility_p3_header_open(out_receipt->executable_path,
                                                actual_size, out_receipt)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    /* ReDMCSB CEDT027.C:45-62 declares C09_ICON.  The exact indexed RGB6
     * sequence is present in each verified F31 C06 image, at a different
     * raw offset because the English and Japanese P3 layouts differ. */
    if (!csb_v1_fmtowns_utility_icon_palette_open(out_receipt->executable_path,
                                                  icon_palette_offset,
                                                  out_receipt)) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->executable_verified = 1;
    out_receipt->language_matches_profile = 1;
    out_receipt->utility_program_is_c06_cedt = 1;
    out_receipt->language = language;
    out_receipt->variant_id = expected_variant;
    out_receipt->executable_size = actual_size;
    out_receipt->executable_fnv1a = actual_hash;
    snprintf(out_receipt->executable_name, sizeof(out_receipt->executable_name),
             "%s", name);
    out_receipt->source_evidence =
        "ReDMCSB SWITCH.C F2279; AUTOEXEC.BAT exits 2/5; "
        "COMPILE.H EXEID 63/64 lines 379-385 C06_CEDT";
    return 1;
}

int csb_v1_fmtowns_utility_menu_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityMenuReceipt *out_receipt)
{
    CSB_V1_FmtownsUtilityHandoffReceipt handoff;
    uint32_t virtual_offset;
    uint32_t byte_count;
    uint32_t expected_hash;
    static const uint16_t k_english_offsets[
        CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT] = {0u, 16u, 32u, 52u, 60u, 68u};
    static const uint16_t k_japanese_offsets[
        CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT] = {0u, 12u, 28u, 44u, 52u, 60u};
    const uint16_t *offsets;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!csb_v1_fmtowns_utility_handoff_open(profile, language, &handoff))
        return 0;
    if (language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        virtual_offset = CSB_V1_FMTOWNS_UTILE_MENU_VIRTUAL_OFFSET;
        byte_count = CSB_V1_FMTOWNS_UTILE_MENU_BYTES;
        expected_hash = CSB_V1_FMTOWNS_UTILE_MENU_FNV1A;
        offsets = k_english_offsets;
    } else if (language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        virtual_offset = CSB_V1_FMTOWNS_UTILJ_MENU_VIRTUAL_OFFSET;
        byte_count = CSB_V1_FMTOWNS_UTILJ_MENU_BYTES;
        expected_hash = CSB_V1_FMTOWNS_UTILJ_MENU_FNV1A;
        offsets = k_japanese_offsets;
    } else return 0;
    if (byte_count > sizeof(out_receipt->source_bytes) ||
        virtual_offset > UINT32_MAX - handoff.p3_load_image_offset ||
        !csb_v1_fmtowns_game_read_span(
            handoff.executable_path,
            handoff.p3_load_image_offset + virtual_offset,
            out_receipt->source_bytes, byte_count) ||
        csb_v1_fmtowns_game_bytes_fnv1a(out_receipt->source_bytes,
                                         byte_count) != expected_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    memcpy(out_receipt->label_offsets, offsets,
           sizeof(out_receipt->label_offsets));
    out_receipt->icon_palette_verified = handoff.icon_palette_verified;
    out_receipt->icon_palette_file_offset = handoff.icon_palette_file_offset;
    memcpy(out_receipt->icon_palette_rgb6, handoff.icon_palette_rgb6,
           sizeof(out_receipt->icon_palette_rgb6));
    out_receipt->valid = 1;
    out_receipt->language = language;
    out_receipt->variant_id = handoff.variant_id;
    out_receipt->source_virtual_offset = virtual_offset;
    out_receipt->source_file_offset = handoff.p3_load_image_offset + virtual_offset;
    out_receipt->source_size = byte_count;
    out_receipt->source_fnv1a = expected_hash;
    out_receipt->source_evidence =
        "UTILE/UTILJ Phar Lap P3 disassembly: C06 menu label pool; "
        "ReDMCSB COMPILE.H EXEID 63/64 C06_CEDT";
    return 1;
}

int csb_v1_fmtowns_utility_menu_action_at(
    const CSB_V1_FmtownsUtilityMenuReceipt *receipt,
    int16_t source_x, int16_t source_y,
    CSB_V1_FmtownsUtilityMenuHitBox *out_hit_box)
{
    static const CSB_V1_FmtownsUtilityMenuHitBox k_english_hits[
        CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT] = {
        { CSB_V1_FMTOWNS_UTILITY_ACTION_LOAD_CHAMPIONS, 2, 92, 186, 194 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_SAVE_CHAMPIONS, 102, 192, 186, 194 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_MAKE_NEW_ADVENTURE, 202, 316, 186, 194 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_REVERT, 156, 196, 159, 167 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_UNDO, 225, 253, 159, 167 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_QUIT, 288, 316, 5, 13 }
    };
    static const CSB_V1_FmtownsUtilityMenuHitBox k_japanese_hits[
        CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT] = {
        { CSB_V1_FMTOWNS_UTILITY_ACTION_LOAD_CHAMPIONS, 2, 92, 179, 196 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_SAVE_CHAMPIONS, 98, 197, 179, 196 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_MAKE_NEW_ADVENTURE, 203, 317, 179, 196 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_REVERT, 156, 196, 154, 171 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_UNDO, 213, 253, 154, 171 },
        { CSB_V1_FMTOWNS_UTILITY_ACTION_QUIT, 266, 317, 6, 23 }
    };
    const CSB_V1_FmtownsUtilityMenuHitBox *hits;
    uint32_t index;

    if (out_hit_box) memset(out_hit_box, 0, sizeof(*out_hit_box));
    if (!receipt || !out_hit_box || !receipt->valid) return 0;
    if (receipt->language == CSB_FMTOWNS_SWITCH_ENGLISH) {
        hits = k_english_hits;
    } else if (receipt->language == CSB_FMTOWNS_SWITCH_JAPANESE) {
        hits = k_japanese_hits;
    } else return 0;

    /* ReDMCSB CEDTDATA.C G2272_MouseInputs keeps both box boundaries. */
    for (index = 0u; index < CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT;
         ++index) {
        if (source_x >= hits[index].left && source_x <= hits[index].right &&
            source_y >= hits[index].top && source_y <= hits[index].bottom) {
            *out_hit_box = hits[index];
            return 1;
        }
    }
    return 0;
}

int csb_v1_fmtowns_game_music_track_at(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    uint32_t map_index,
    uint32_t map_x,
    uint32_t map_y,
    uint8_t *out_track)
{
    unsigned char track;
    uint32_t table_index;

    if (out_track) *out_track = 0u;
    if (!receipt || !out_track || !receipt->valid ||
        !receipt->executable_verified || !receipt->music_table_verified ||
        receipt->music_table_size != CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_BYTES ||
        receipt->music_table_fnv1a != CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_FNV1A ||
        map_index >= CSB_V1_FMTOWNS_GAME_MUSIC_MAP_COUNT ||
        map_x >= CSB_V1_FMTOWNS_GAME_MUSIC_MAP_WIDTH ||
        map_y >= CSB_V1_FMTOWNS_GAME_MUSIC_MAP_HEIGHT) return 0;
    table_index = map_index * CSB_V1_FMTOWNS_GAME_MUSIC_MAP_WIDTH *
                  CSB_V1_FMTOWNS_GAME_MUSIC_MAP_HEIGHT +
                  map_y * CSB_V1_FMTOWNS_GAME_MUSIC_MAP_WIDTH + map_x;
    if (!csb_v1_fmtowns_game_read_span(receipt->executable_path,
                                       receipt->music_table_source_offset +
                                           table_index,
                                       &track, sizeof(track))) return 0;
    *out_track = track;
    return 1;
}
