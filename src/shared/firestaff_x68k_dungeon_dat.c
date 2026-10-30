#include "firestaff_x68k_dungeon_dat.h"

#include <string.h>

/*
 * ReDMCSB X30J source contract:
 * - DEFS.H:989-998 defines DUNGEON_HEADER; DEFS.H:1049-1116 defines MAP.
 * - LOADSAVE.C:1936-1962 reads the header and map table.
 * - LOADSAVE.C:1978-2003 derives columns from Width+1 and reads column/SFT
 *   words; LOADSAVE.C:2031-2038 reads text words.
 * - DUNGEON.C:45-60 defines serialized bytes for each of 16 thing types;
 *   LOADSAVE.C:2043-2059 reads those records, then LOADSAVE.C:2076-2087
 *   reads raw maps. DUNGEON.C:1073-1075 and 2755-2761 consume the map grid
 *   followed by creature-type, wall-ornament, floor-ornament and door-
 *   ornament bytes. LOADSAVE.C:2084-2089 permits an optional trailing
 *   checksum; READWRIT.C:264-289 defines it as the byte sum of all sections.
 * X30J runs on a big-endian 68k target; this parser only exposes validated
 * section spans and does not decompress, decode maps, or start the platform.
 */
enum {
    X68K_DUNGEON_HEADER_BYTES = 44u,
    X68K_DUNGEON_MAP_BYTES = 16u,
    X68K_DUNGEON_COMPRESSED_SIGNATURE = 0x8104u
};

static const uint8_t g_x68k_thing_record_bytes[FIRESTAFF_X68K_DUNGEON_THING_TYPES] = {
    4u, 6u, 4u, 8u, 16u, 4u, 4u, 4u,
    4u, 8u, 4u, 0u, 0u, 0u, 8u, 4u
};

static uint16_t x68k_read_be16(const uint8_t *bytes) {
    return (uint16_t)(((uint16_t)bytes[0] << 8) | (uint16_t)bytes[1]);
}

static int x68k_advance_span(size_t *cursor,
                             size_t count,
                             size_t record_bytes,
                             size_t input_size,
                             FirestaffX68kDungeonDatSpan *out) {
    size_t span_bytes;
    if (!cursor || !out ||
        (record_bytes != 0u && count > SIZE_MAX / record_bytes)) {
        return -1;
    }
    span_bytes = count * record_bytes;
    if (*cursor > input_size || span_bytes > input_size - *cursor) return -1;
    out->offset = *cursor;
    out->size = span_bytes;
    *cursor += span_bytes;
    return 0;
}

int FirestaffX68kDungeonDat_OpenRaw(const uint8_t *bytes,
                                    size_t size,
                                    FirestaffX68kDungeonDat *out) {
    FirestaffX68kDungeonDat parsed;
    size_t cursor;
    size_t columns = 0u;
    uint16_t map_index;
    uint16_t thing_type;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!bytes || size < X68K_DUNGEON_HEADER_BYTES) return -1;

    /* Do not treat a compressed-dungeon wrapper as an uncompressed header. */
    if (x68k_read_be16(bytes) == X68K_DUNGEON_COMPRESSED_SIGNATURE) return -1;

    memset(&parsed, 0, sizeof(parsed));
    parsed.bytes = bytes;
    parsed.size = size;
    parsed.ornament_random_seed = x68k_read_be16(bytes);
    parsed.raw_map_byte_count = x68k_read_be16(bytes + 2u);
    parsed.map_count = bytes[4];
    parsed.text_data_word_count = x68k_read_be16(bytes + 6u);
    parsed.initial_party_location = x68k_read_be16(bytes + 8u);
    parsed.square_first_thing_count = x68k_read_be16(bytes + 10u);
    if (parsed.map_count == 0u ||
        parsed.map_count > FIRESTAFF_X68K_DUNGEON_MAX_MAPS ||
        parsed.raw_map_byte_count == 0u) {
        return -1;
    }
    for (thing_type = 0u;
         thing_type < FIRESTAFF_X68K_DUNGEON_THING_TYPES;
         ++thing_type) {
        parsed.thing_count[thing_type] =
            x68k_read_be16(bytes + 12u + (size_t)thing_type * 2u);
    }

    cursor = X68K_DUNGEON_HEADER_BYTES;
    if (x68k_advance_span(&cursor, parsed.map_count,
                          X68K_DUNGEON_MAP_BYTES, size,
                          &parsed.map_descriptors) != 0) {
        return -1;
    }
    for (map_index = 0u; map_index < parsed.map_count; ++map_index) {
        const uint8_t *raw = bytes + parsed.map_descriptors.offset +
                             (size_t)map_index * X68K_DUNGEON_MAP_BYTES;
        const uint16_t packed = x68k_read_be16(raw + 8u);
        const uint16_t ornaments = x68k_read_be16(raw + 10u);
        const uint16_t map_counts = x68k_read_be16(raw + 12u);
        FirestaffX68kDungeonDatMap *map = &parsed.maps[map_index];
        const size_t map_columns = (size_t)((packed >> 6) & 0x1fu) + 1u;
        size_t grid_size;

        map->raw_map_offset = x68k_read_be16(raw);
        map->offset_map_x = raw[6];
        map->offset_map_y = raw[7];
        /* X30J MAP.A declarations are Height:5, Width:5, Level:6. */
        map->level = (uint8_t)(packed & 0x3fu);
        map->width = (uint8_t)map_columns;
        map->height = (uint8_t)(((packed >> 11) & 0x1fu) + 1u);
        /* X30J B/C words: low wall/door counts, next creature count, and
         * the second B nibble is the fixed floor-ornament count. */
        map->wall_ornament_count = (uint8_t)(ornaments & 0x0fu);
        map->floor_ornament_count = (uint8_t)((ornaments >> 8) & 0x0fu);
        map->door_ornament_count = (uint8_t)(map_counts & 0x0fu);
        map->allowed_creature_type_count =
            (uint8_t)((map_counts >> 4) & 0x0fu);
        grid_size = (size_t)map->width * (size_t)map->height;
        map->raw_map_span_size = grid_size +
            (size_t)map->allowed_creature_type_count +
            (size_t)map->wall_ornament_count +
            (size_t)map->floor_ornament_count +
            (size_t)map->door_ornament_count;
        if (map->level >= parsed.map_count ||
            map->raw_map_offset > parsed.raw_map_byte_count ||
            map->raw_map_span_size >
                (size_t)parsed.raw_map_byte_count - map->raw_map_offset ||
            columns > SIZE_MAX - map_columns) {
            return -1;
        }
        columns += map_columns;
    }
    if (columns > UINT16_MAX) return -1;
    parsed.column_count = (uint16_t)columns;

    if (x68k_advance_span(&cursor, columns, sizeof(uint16_t), size,
                          &parsed.column_sft_counts) != 0 ||
        x68k_advance_span(&cursor, parsed.square_first_thing_count,
                          sizeof(uint16_t), size,
                          &parsed.square_first_things) != 0 ||
        x68k_advance_span(&cursor, parsed.text_data_word_count,
                          sizeof(uint16_t), size,
                          &parsed.text_data) != 0) {
        return -1;
    }
    for (thing_type = 0u;
         thing_type < FIRESTAFF_X68K_DUNGEON_THING_TYPES;
         ++thing_type) {
        if (x68k_advance_span(&cursor, parsed.thing_count[thing_type],
                              g_x68k_thing_record_bytes[thing_type], size,
                              &parsed.thing_data[thing_type]) != 0) {
            return -1;
        }
    }
    if (x68k_advance_span(&cursor, 1u, parsed.raw_map_byte_count, size,
                          &parsed.raw_map_data) != 0) {
        return -1;
    }
    if (cursor == size) {
        parsed.has_trailing_checksum = 0u;
    } else if (cursor <= SIZE_MAX - sizeof(uint16_t) &&
               size == cursor + sizeof(uint16_t)) {
        size_t byte_index;
        uint16_t checksum = 0u;
        parsed.trailing_checksum = x68k_read_be16(bytes + cursor);
        for (byte_index = 0u; byte_index < cursor; ++byte_index) {
            checksum = (uint16_t)(checksum + bytes[byte_index]);
        }
        if (checksum != parsed.trailing_checksum) return -1;
        parsed.has_trailing_checksum = 1u;
    } else {
        return -1;
    }

    *out = parsed;
    return 0;
}
