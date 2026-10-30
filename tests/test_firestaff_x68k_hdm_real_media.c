#include "firestaff_7z_extract.h"
#include "firestaff_x68k_hdm.h"
#include "firestaff_x68k_graphics_dat.h"
#include "firestaff_x68k_dungeon_dat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *name;
    uint32_t size;
    uint16_t first_cluster;
    unsigned seen;
} ExpectedRootFile;

static ExpectedRootFile g_expected[] = {
    {"HUMAN.SYS", 44660u, 2u, 0u},
    {"SYS", 0u, 46u, 0u},
    {"SWOOSH", 9784u, 63u, 0u},
    {"DM.X", 8014u, 73u, 0u},
    {"CONFIG.SYS", 118u, 81u, 0u},
    {"DUNGEON.DAT", 33990u, 82u, 0u},
    {"GRAPHICS.DAT", 332785u, 116u, 0u},
    {"FTLCODE", 156564u, 441u, 0u},
    {"AUTOEXEC.BAT", 3u, 594u, 0u},
    {"COMMAND.X", 24736u, 595u, 0u}
};

static int file_exists(const char *path) {
    FILE *file;
    if (!path || !path[0]) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    fclose(file);
    return 1;
}

static const char *configured_or_home_path(const char *environment_name,
                                           const char *suffix,
                                           char *buffer,
                                           size_t capacity) {
    const char *configured = getenv(environment_name);
    const char *home = getenv("HOME");
    if (configured && configured[0]) return configured;
    if (!home || !home[0] || !suffix || !buffer || capacity == 0u) return NULL;
    if (snprintf(buffer, capacity, "%s/%s", home, suffix) >= (int)capacity) {
        return NULL;
    }
    return buffer;
}

static int compare_reference_file(const char *path,
                                  const uint8_t *bytes,
                                  size_t byte_count) {
    uint8_t *reference = NULL;
    FILE *file;
    long file_size;
    int result = -1;
    file = fopen(path, "rb");
    if (!file) return -1;
    if (fseek(file, 0, SEEK_END) != 0 || (file_size = ftell(file)) < 0 ||
        (size_t)file_size != byte_count || fseek(file, 0, SEEK_SET) != 0) {
        goto done;
    }
    reference = (uint8_t *)malloc(byte_count ? byte_count : 1u);
    if (!reference || fread(reference, 1u, byte_count, file) != byte_count) {
        goto done;
    }
    result = memcmp(reference, bytes, byte_count) == 0 ? 0 : -1;
done:
    free(reference);
    fclose(file);
    return result;
}

static ExpectedRootFile *expected_file(const char *name) {
    size_t i;
    for (i = 0u; i < sizeof(g_expected) / sizeof(g_expected[0]); ++i) {
        if (strcmp(name, g_expected[i].name) == 0) return &g_expected[i];
    }
    return NULL;
}

int main(void) {
    static const char *const archive_suffix =
        ".firestaff/data/dm1/Game,Dungeon_Master,JP,X68000,Software.7z";
    static const char *const cracked_member =
        "Dungeon Master for X68000 3.0 (Japanese) Cracked.hdm";
    static const char *const swoosh_reference_suffix =
        "ReDMCSB/Reference/Original/X30J/SWOOSH";
    char archive_buffer[2048];
    char reference_buffer[2048];
    const char *archive_path;
    const char *reference_path;
    uint8_t *image = NULL;
    size_t image_size = 0u;
    uint8_t *swoosh = NULL;
    uint8_t *ftlcode = NULL;
    uint8_t *graphics = NULL;
    uint8_t *dungeon = NULL;
    FirestaffX68kHdm hdm;
    FirestaffX68kGraphicsDat graphics_dat;
    FirestaffX68kDungeonDat dungeon_dat;
    uint16_t cursor = 0u;
    size_t entries = 0u;
    size_t total_thing_records = 0u;
    size_t square_list_count = 0u;
    size_t square_list_thing_count = 0u;
    size_t ownership_list_count = 0u;
    size_t ownership_thing_count = 0u;
    size_t i;
    int result = 1;

    archive_path = configured_or_home_path(
        "FIRESTAFF_X68K_HDM_ARCHIVE", archive_suffix,
        archive_buffer, sizeof(archive_buffer));
    reference_path = configured_or_home_path(
        "FIRESTAFF_X68K_SWOOSH_REFERENCE", swoosh_reference_suffix,
        reference_buffer, sizeof(reference_buffer));
    if (!file_exists(archive_path) || !file_exists(reference_path)) {
        puts("SKIP: authentic X68000 archive or ReDMCSB SWOOSH reference is unavailable");
        result = 77;
        goto done;
    }
    if (!firestaff_7z_read_member(archive_path, cracked_member,
                                  &image, &image_size)) {
        fputs("FAIL: could not read the named cracked HDM member from the original 7z\n",
              stderr);
        goto done;
    }
    if (FirestaffX68kHdm_Open(image, image_size, &hdm) != 0) {
        fputs("FAIL: cracked HDM did not match the bounded DM1 X68000 layout\n",
              stderr);
        goto done;
    }

    for (;;) {
        FirestaffX68kHdmRootEntry entry;
        ExpectedRootFile *expected;
        uint8_t *contents;
        int next = FirestaffX68kHdm_NextRootEntry(&hdm, &cursor, &entry);
        if (next == 1) break;
        if (next != 0) {
            fputs("FAIL: malformed X68000 root directory\n", stderr);
            goto done;
        }
        ++entries;
        expected = expected_file(entry.name);
        if (!expected || expected->seen || entry.size != expected->size ||
            entry.first_cluster != expected->first_cluster) {
            fprintf(stderr, "FAIL: unexpected X68000 root entry %s\n", entry.name);
            goto done;
        }
        expected->seen = 1u;
        if ((entry.attributes & 0x10u) != 0u) continue;
        contents = (uint8_t *)malloc(entry.size ? entry.size : 1u);
        if (!contents || FirestaffX68kHdm_ReadRootFile(
                &hdm, &entry, contents, entry.size) != 0) {
            free(contents);
            fprintf(stderr, "FAIL: invalid FAT chain for %s\n", entry.name);
            goto done;
        }
        if (strcmp(entry.name, "SWOOSH") == 0) swoosh = contents;
        else if (strcmp(entry.name, "FTLCODE") == 0) ftlcode = contents;
        else if (strcmp(entry.name, "GRAPHICS.DAT") == 0) graphics = contents;
        else if (strcmp(entry.name, "DUNGEON.DAT") == 0) dungeon = contents;
        else free(contents);
    }
    if (entries != sizeof(g_expected) / sizeof(g_expected[0])) {
        fputs("FAIL: X68000 root directory did not contain the expected file set\n",
              stderr);
        goto done;
    }
    for (i = 0u; i < sizeof(g_expected) / sizeof(g_expected[0]); ++i) {
        if (!g_expected[i].seen) {
            fprintf(stderr, "FAIL: missing X68000 root entry %s\n", g_expected[i].name);
            goto done;
        }
    }
    if (!swoosh || !ftlcode || !graphics || !dungeon ||
        memcmp(image + 0x12000u, swoosh, 9784u) != 0 ||
        memcmp(image + 0x70800u, ftlcode, 156564u) != 0 ||
        graphics[0] != 0x80u || graphics[1] != 0x01u ||
        graphics[2] != 0x02u || graphics[3] != 0x32u) {
        fputs("FAIL: extracted X68000 files did not match raw-file offsets or GRAPHICS.DAT header\n",
              stderr);
        goto done;
    }
    if (compare_reference_file(reference_path, swoosh, 9784u) != 0) {
        fputs("FAIL: extracted SWOOSH differs from ReDMCSB X30J reference bytes\n",
              stderr);
        goto done;
    }
    if (FirestaffX68kGraphicsDat_Open(graphics, 332785u,
                                      &graphics_dat) != 0 ||
        graphics_dat.count != 562u ||
        graphics_dat.payload_offset != 4500u) {
        fputs("FAIL: X68000 GRAPHICS.DAT did not match ReDMCSB format-1 header\n",
              stderr);
        goto done;
    }
    {
        size_t expected_offset = graphics_dat.payload_offset;
        uint16_t index;
        for (index = 0u; index < graphics_dat.count; ++index) {
            FirestaffX68kGraphicsDatItem item;
            if (FirestaffX68kGraphicsDat_GetItem(&graphics_dat, index,
                                                 &item) != 0 ||
                item.payload_offset != expected_offset ||
                item.compressed_size > graphics_dat.size - expected_offset) {
                fprintf(stderr,
                        "FAIL: X68000 GRAPHICS.DAT item %u has invalid metadata span\n",
                        (unsigned)index);
                goto done;
            }
            expected_offset += item.compressed_size;
        }
        if (expected_offset != graphics_dat.size) {
            fputs("FAIL: X68000 GRAPHICS.DAT payload table did not consume the file\n",
                  stderr);
            goto done;
        }
    }
    {
        /* FNV-1a of IMG1 indexes, dimensions, and decoded palette indices,
         * computed from Greatstone SCK's extracted dm_x68k_gd.map BMP set. */
        const uint64_t fnv_offset = UINT64_C(14695981039346656037);
        const uint64_t fnv_prime = UINT64_C(1099511628211);
        const uint64_t expected_pixels_hash = UINT64_C(0x8c1193b353972be5);
        uint64_t pixels_hash = fnv_offset;
        uint8_t *pixels = (uint8_t *)malloc(640u * 400u);
        unsigned image_count = 0u;
        unsigned non_image_count = 0u;
        uint16_t index;
        if (!pixels) {
            fputs("FAIL: could not allocate X68000 indexed graphics buffer\n",
                  stderr);
            goto done;
        }
        for (index = 0u; index < graphics_dat.count; ++index) {
            uint16_t width = 0u;
            uint16_t height = 0u;
            size_t pixel_index;
            if (FirestaffX68kGraphicsDat_DecodeImageItem(
                    &graphics_dat, index, pixels, 640u * 400u,
                    &width, &height) != 0) {
                ++non_image_count;
                continue;
            }
            ++image_count;
            pixels_hash = (pixels_hash ^ (uint8_t)index) * fnv_prime;
            pixels_hash = (pixels_hash ^ (uint8_t)width) * fnv_prime;
            pixels_hash = (pixels_hash ^ (uint8_t)height) * fnv_prime;
            for (pixel_index = 0u;
                 pixel_index < (size_t)width * (size_t)height;
                 ++pixel_index) {
                pixels_hash = (pixels_hash ^ pixels[pixel_index]) * fnv_prime;
            }
        }
        free(pixels);
        if (image_count != 539u || non_image_count != 23u ||
            pixels_hash != expected_pixels_hash) {
            fprintf(stderr,
                    "FAIL: X68000 IMG1 decode corpus changed (images=%u non-images=%u hash=%016llx)\n",
                    image_count, non_image_count,
                    (unsigned long long)pixels_hash);
            goto done;
        }
    }
    if (FirestaffX68kDungeonDat_OpenRaw(dungeon, 33990u, &dungeon_dat) != 0 ||
        dungeon_dat.size != 33990u ||
        dungeon_dat.ornament_random_seed != 99u ||
        dungeon_dat.raw_map_byte_count != 12366u ||
        dungeon_dat.map_count != 14u ||
        dungeon_dat.text_data_word_count != 2023u ||
        dungeon_dat.initial_party_location != 0x0861u ||
        dungeon_dat.square_first_thing_count != 1682u ||
        dungeon_dat.column_count != 412u ||
        dungeon_dat.map_descriptors.offset != 44u ||
        dungeon_dat.map_descriptors.size != 224u ||
        dungeon_dat.column_sft_counts.offset != 268u ||
        dungeon_dat.column_sft_counts.size != 824u ||
        dungeon_dat.square_first_things.offset != 1092u ||
        dungeon_dat.square_first_things.size != 3364u ||
        dungeon_dat.text_data.offset != 4456u ||
        dungeon_dat.text_data.size != 4046u ||
        dungeon_dat.thing_data[0].offset != 8502u ||
        dungeon_dat.thing_data[10].offset != 20504u ||
        dungeon_dat.thing_data[10].size != 1120u ||
        dungeon_dat.raw_map_data.offset != 21624u ||
        dungeon_dat.raw_map_data.size != 12366u ||
        dungeon_dat.has_trailing_checksum != 0u ||
        dungeon_dat.raw_map_data.offset + dungeon_dat.raw_map_data.size !=
            dungeon_dat.size) {
        fputs("FAIL: X68000 DUNGEON.DAT metadata spans did not match ReDMCSB X30J layout\n",
              stderr);
        goto done;
    }
    {
        static const uint8_t expected_widths[14] = {
            19u, 32u, 32u, 32u, 30u, 32u, 30u,
            32u, 31u, 31u, 31u, 32u, 22u, 26u
        };
        static const uint8_t expected_heights[14] = {
            19u, 32u, 32u, 32u, 31u, 32u, 31u,
            32u, 32u, 32u, 32u, 31u, 20u, 13u
        };
        static const uint16_t expected_thing_counts[16] = {
            170u, 179u, 125u, 683u, 182u, 107u, 121u, 35u,
            56u, 12u, 280u, 0u, 0u, 0u, 0u, 0u
        };
        uint16_t expected_columns = 0u;
        uint16_t map_index;
        for (map_index = 0u; map_index < 14u; ++map_index) {
            const FirestaffX68kDungeonDatMap *map =
                &dungeon_dat.maps[map_index];
            if (map->level != map_index ||
                map->width != expected_widths[map_index] ||
                map->height != expected_heights[map_index]) {
                fprintf(stderr,
                        "FAIL: X68000 DUNGEON.DAT map %u metadata changed\n",
                        (unsigned)map_index);
                goto done;
            }
            if ((size_t)map->raw_map_offset > dungeon_dat.raw_map_byte_count ||
                map->raw_map_span_size > (size_t)dungeon_dat.raw_map_byte_count -
                                             map->raw_map_offset ||
                (map_index + 1u < dungeon_dat.map_count &&
                 (size_t)map->raw_map_offset + map->raw_map_span_size !=
                     dungeon_dat.maps[map_index + 1u].raw_map_offset) ||
                (map_index + 1u == dungeon_dat.map_count &&
                 (size_t)map->raw_map_offset + map->raw_map_span_size !=
                     dungeon_dat.raw_map_byte_count)) {
                fprintf(stderr,
                        "FAIL: X68000 DUNGEON.DAT map %u raw-data span is not bounded\n",
                        (unsigned)map_index);
                goto done;
            }
            expected_columns = (uint16_t)(expected_columns + map->width);
        }
        if (expected_columns != dungeon_dat.column_count) {
            fputs("FAIL: X68000 DUNGEON.DAT column count did not match map records\n",
                  stderr);
            goto done;
        }
        for (map_index = 0u; map_index < 16u; ++map_index) {
            if (dungeon_dat.thing_count[map_index] !=
                expected_thing_counts[map_index]) {
                fprintf(stderr,
                        "FAIL: X68000 DUNGEON.DAT thing count %u changed\n",
                        (unsigned)map_index);
                goto done;
            }
        }
    }
    {
        size_t typed_thing_bytes = 0u;
        size_t expected_offset = 8502u;
        uint16_t thing_type;
        for (thing_type = 0u; thing_type < 16u; ++thing_type) {
            uint16_t thing_index;
            const size_t record_bytes[] = {
                4u, 6u, 4u, 8u, 16u, 4u, 4u, 4u,
                4u, 8u, 4u, 0u, 0u, 0u, 8u, 4u
            };
            if (dungeon_dat.thing_data[thing_type].offset != expected_offset) {
                fprintf(stderr,
                        "FAIL: X68000 DUNGEON.DAT thing section %u offset changed\n",
                        (unsigned)thing_type);
                goto done;
            }
            if (dungeon_dat.thing_data[thing_type].size > SIZE_MAX -
                    typed_thing_bytes) {
                fputs("FAIL: typed thing spans overflowed size_t\n", stderr);
                goto done;
            }
            typed_thing_bytes += dungeon_dat.thing_data[thing_type].size;
            expected_offset += dungeon_dat.thing_data[thing_type].size;
            total_thing_records += dungeon_dat.thing_count[thing_type];
            for (thing_index = 0u;
                 thing_index < dungeon_dat.thing_count[thing_type];
                 ++thing_index) {
                uint16_t thing = (uint16_t)((thing_type << 10) | thing_index);
                uint16_t actual_next = 0u;
                const size_t record_offset = dungeon_dat.thing_data[thing_type].offset +
                    (size_t)thing_index * record_bytes[thing_type];
                const uint16_t raw_next =
                    (uint16_t)(((uint16_t)dungeon[record_offset] << 8) |
                               dungeon[record_offset + 1u]);
                if (FirestaffX68kDungeonDat_GetThingNext(
                        &dungeon_dat, thing, &actual_next) != 0 ||
                    actual_next != raw_next) {
                    fprintf(stderr,
                            "FAIL: X68000 type %u thing %u Next differs from raw GENERIC field\n",
                            (unsigned)thing_type, (unsigned)thing_index);
                    goto done;
                }
                if (actual_next != FIRESTAFF_X68K_DUNGEON_THING_END_OF_LIST &&
                    actual_next != FIRESTAFF_X68K_DUNGEON_THING_NONE) {
                    const uint8_t next_type =
                        (uint8_t)((actual_next & 0x3c00u) >> 10);
                    const uint16_t next_index =
                        (uint16_t)(actual_next & 0x03ffu);
                    if (next_index >= dungeon_dat.thing_count[next_type]) {
                        fprintf(stderr,
                                "FAIL: X68000 type %u thing %u has invalid Next ref 0x%04x\n",
                                (unsigned)thing_type, (unsigned)thing_index,
                                (unsigned)actual_next);
                        goto done;
                    }
                }
                if (thing_type == 4u || thing_type == 9u) {
                    uint16_t slot = 0u;
                    const uint16_t raw_slot =
                        (uint16_t)(((uint16_t)dungeon[record_offset + 2u] << 8) |
                                   dungeon[record_offset + 3u]);
                    if (FirestaffX68kDungeonDat_GetThingSlot(
                            &dungeon_dat, thing, &slot) != 0 || slot != raw_slot) {
                        fprintf(stderr,
                                "FAIL: X68000 type %u thing %u Slot differs from raw record\n",
                                (unsigned)thing_type, (unsigned)thing_index);
                        goto done;
                    }
                    /* An unused record has no owned contents. */
                    if (actual_next != FIRESTAFF_X68K_DUNGEON_THING_NONE &&
                        slot != FIRESTAFF_X68K_DUNGEON_THING_END_OF_LIST) {
                        uint16_t current = slot;
                        size_t traversed = 0u;
                        ++ownership_list_count;
                        while (current !=
                                   FIRESTAFF_X68K_DUNGEON_THING_END_OF_LIST) {
                            uint16_t next = 0u;
                            const uint8_t current_type =
                                (uint8_t)((current & 0x3c00u) >> 10);
                            const uint16_t current_index =
                                (uint16_t)(current & 0x03ffu);
                            if (current == FIRESTAFF_X68K_DUNGEON_THING_NONE ||
                                traversed++ >= total_thing_records ||
                                current_index >=
                                    dungeon_dat.thing_count[current_type] ||
                                FirestaffX68kDungeonDat_GetThingNext(
                                    &dungeon_dat, current, &next) != 0) {
                                fprintf(stderr,
                                        "FAIL: X68000 type %u thing %u has invalid or cyclic owned Slot list\n",
                                        (unsigned)thing_type,
                                        (unsigned)thing_index);
                                goto done;
                            }
                            ++ownership_thing_count;
                            current = next;
                        }
                    }
                }
            }
        }
        if (typed_thing_bytes != 13122u ||
            expected_offset != dungeon_dat.raw_map_data.offset) {
            fputs("FAIL: X68000 DUNGEON.DAT typed thing span total changed\n",
                  stderr);
            goto done;
        }
    }
    {
        uint8_t map_x = 0u;
        uint8_t map_y = 0u;
        uint8_t direction = 0u;
        uint8_t start_square = 0u;
        uint16_t start_thing = 0u;
        size_t expected_column_base = 0u;
        uint16_t map_index;
        if (FirestaffX68kDungeonDat_GetInitialPartyLocation(
                &dungeon_dat, &map_x, &map_y, &direction) != 0 ||
            map_x != 1u || map_y != 3u || direction != 2u ||
            FirestaffX68kDungeonDat_GetSquare(
                &dungeon_dat, 0u, map_x, map_y, &start_square) != 0 ||
            start_square != 0xb0u ||
            FirestaffX68kDungeonDat_GetSquareFirstThing(
                &dungeon_dat, 0u, map_x, map_y, &start_thing) != 0) {
            fputs("FAIL: X68000 initial party location did not resolve to the source map grid\n",
                  stderr);
            goto done;
        }
        for (map_index = 0u; map_index < dungeon_dat.map_count; ++map_index) {
            const FirestaffX68kDungeonDatMap *map =
                &dungeon_dat.maps[map_index];
            uint8_t map_x_index;
            uint8_t map_y_index;
            for (map_x_index = 0u; map_x_index < map->width; ++map_x_index) {
                for (map_y_index = 0u; map_y_index < map->height;
                     ++map_y_index) {
                    uint8_t square = 0u;
                    FirestaffX68kDungeonSquare decoded_square;
                    uint8_t prior_y;
                    const size_t source_index =
                        dungeon_dat.raw_map_data.offset +
                        map->raw_map_offset +
                        (size_t)map_x_index * map->height + map_y_index;
                    if (FirestaffX68kDungeonDat_GetSquare(
                            &dungeon_dat, (uint8_t)map_index,
                            map_x_index, map_y_index, &square) != 0 ||
                        FirestaffX68kDungeonDat_DecodeSquare(
                            square, &decoded_square) != 0 ||
                        decoded_square.type != (uint8_t)(square >> 5) ||
                        decoded_square.flags != (uint8_t)(square & 0x1fu) ||
                        square != dungeon[source_index]) {
                        fprintf(stderr,
                                "FAIL: X68000 map %u square (%u,%u) did not follow ReDMCSB column-major layout\n",
                                (unsigned)map_index, (unsigned)map_x_index,
                                (unsigned)map_y_index);
                        goto done;
                    }
                    {
                        uint16_t first_thing = 0u;
                        const int has_list = (square & 0x10u) != 0u;
                        if (FirestaffX68kDungeonDat_GetSquareFirstThing(
                                &dungeon_dat, (uint8_t)map_index,
                                map_x_index, map_y_index, &first_thing) != 0) {
                            fprintf(stderr,
                                    "FAIL: X68000 map %u square (%u,%u) SFT accessor rejected authentic data\n",
                                    (unsigned)map_index, (unsigned)map_x_index,
                                    (unsigned)map_y_index);
                            goto done;
                        }
                        if (!has_list) {
                            if (first_thing !=
                                    FIRESTAFF_X68K_DUNGEON_THING_END_OF_LIST) {
                                fprintf(stderr,
                                        "FAIL: X68000 empty square (%u,%u) did not return end-of-list\n",
                                        (unsigned)map_x_index,
                                        (unsigned)map_y_index);
                                goto done;
                            }
                        } else {
                            ++square_list_count;
                            size_t raw_column_index = expected_column_base +
                                                      map_x_index;
                            size_t expected_sft =
                                ((size_t)dungeon[dungeon_dat.column_sft_counts.offset +
                                                raw_column_index * 2u] << 8) |
                                dungeon[dungeon_dat.column_sft_counts.offset +
                                        raw_column_index * 2u + 1u];
                            size_t raw_sft_offset;
                            uint16_t expected_thing;
                            for (prior_y = 0u; prior_y < map_y_index; ++prior_y) {
                                const size_t prior_square_offset =
                                    dungeon_dat.raw_map_data.offset +
                                    map->raw_map_offset +
                                    (size_t)map_x_index * map->height + prior_y;
                                if ((dungeon[prior_square_offset] & 0x10u) != 0u) {
                                    ++expected_sft;
                                }
                            }
                            raw_sft_offset = dungeon_dat.square_first_things.offset +
                                             expected_sft * 2u;
                            if (expected_sft >= dungeon_dat.square_first_thing_count ||
                                raw_sft_offset + 2u >
                                    dungeon_dat.square_first_things.offset +
                                    dungeon_dat.square_first_things.size) {
                                fprintf(stderr,
                                        "FAIL: X68000 map %u square (%u,%u) expected SFT index is out of range\n",
                                        (unsigned)map_index, (unsigned)map_x_index,
                                        (unsigned)map_y_index);
                                goto done;
                            }
                            expected_thing =
                                (uint16_t)(((uint16_t)dungeon[raw_sft_offset] << 8) |
                                           dungeon[raw_sft_offset + 1u]);
                            if (first_thing != expected_thing) {
                                size_t found_index;
                                for (found_index = 0u;
                                     found_index < dungeon_dat.square_first_thing_count;
                                     ++found_index) {
                                    size_t found_offset =
                                        dungeon_dat.square_first_things.offset +
                                        found_index * 2u;
                                    if ((((uint16_t)dungeon[found_offset] << 8) |
                                         dungeon[found_offset + 1u]) == first_thing) {
                                        break;
                                    }
                                }
                                fprintf(stderr,
                                        "FAIL: X68000 map %u square (%u,%u) F0160/F0161 THING mismatch (got=0x%04x expected=0x%04x index=%lu found=%lu)\n",
                                        (unsigned)map_index, (unsigned)map_x_index,
                                        (unsigned)map_y_index, (unsigned)first_thing,
                                        (unsigned)expected_thing,
                                        (unsigned long)expected_sft,
                                        (unsigned long)found_index);
                                goto done;
                            }
                            if (first_thing != 0xfffeu &&
                                first_thing != 0xffffu) {
                                const uint8_t thing_type =
                                    (uint8_t)((first_thing & 0x3c00u) >> 10);
                                const uint16_t thing_index =
                                    (uint16_t)(first_thing & 0x03ffu);
                                if (thing_type >= FIRESTAFF_X68K_DUNGEON_THING_TYPES ||
                                    thing_index >= dungeon_dat.thing_count[thing_type]) {
                                    fprintf(stderr,
                                            "FAIL: X68000 map %u square (%u,%u) has out-of-range THING ref 0x%04x\n",
                                            (unsigned)map_index,
                                            (unsigned)map_x_index,
                                            (unsigned)map_y_index,
                                            (unsigned)first_thing);
                                    goto done;
                                }
                            }
                            {
                                uint16_t current = first_thing;
                                size_t traversed = 0u;
                                while (current !=
                                           FIRESTAFF_X68K_DUNGEON_THING_END_OF_LIST) {
                                    uint16_t next = 0u;
                                    uint8_t current_type;
                                    uint16_t current_index;
                                    if (current == FIRESTAFF_X68K_DUNGEON_THING_NONE ||
                                        traversed++ >= total_thing_records) {
                                        fprintf(stderr,
                                                "FAIL: X68000 map %u square (%u,%u) has unterminated or cyclic THING list\n",
                                                (unsigned)map_index,
                                                (unsigned)map_x_index,
                                                (unsigned)map_y_index);
                                        goto done;
                                    }
                                    ++square_list_thing_count;
                                    current_type =
                                        (uint8_t)((current & 0x3c00u) >> 10);
                                    current_index =
                                        (uint16_t)(current & 0x03ffu);
                                    if (current_index >=
                                            dungeon_dat.thing_count[current_type] ||
                                        FirestaffX68kDungeonDat_GetThingNext(
                                            &dungeon_dat, current, &next) != 0) {
                                        fprintf(stderr,
                                                "FAIL: X68000 map %u square (%u,%u) list references invalid THING 0x%04x\n",
                                                (unsigned)map_index,
                                                (unsigned)map_x_index,
                                                (unsigned)map_y_index,
                                                (unsigned)current);
                                        goto done;
                                    }
                                    current = next;
                                }
                            }
                        }
                    }
                }
            }
            expected_column_base += map->width;
        }
        if (FirestaffX68kDungeonDat_GetSquare(
                &dungeon_dat, 0u, dungeon_dat.maps[0].width, 0u,
                &start_square) == 0 ||
            FirestaffX68kDungeonDat_GetSquare(
                &dungeon_dat, 0u, 0u, dungeon_dat.maps[0].height,
                &start_square) == 0) {
            fputs("FAIL: X68000 dungeon reader accepted an out-of-map coordinate\n",
                  stderr);
            goto done;
        }
        {
            static const FirestaffX68kDungeonDatMapMetadataKind kinds[] = {
                FIRESTAFF_X68K_DUNGEON_MAP_CREATURE_TYPE,
                FIRESTAFF_X68K_DUNGEON_MAP_WALL_ORNAMENT,
                FIRESTAFF_X68K_DUNGEON_MAP_FLOOR_ORNAMENT,
                FIRESTAFF_X68K_DUNGEON_MAP_DOOR_ORNAMENT
            };
            for (map_index = 0u; map_index < dungeon_dat.map_count;
                 ++map_index) {
                const FirestaffX68kDungeonDatMap *map =
                    &dungeon_dat.maps[map_index];
                const uint8_t counts[] = {
                    map->allowed_creature_type_count,
                    map->wall_ornament_count,
                    map->floor_ornament_count,
                    map->door_ornament_count
                };
                size_t kind_index;
                size_t metadata_offset = dungeon_dat.raw_map_data.offset +
                    map->raw_map_offset +
                    (size_t)map->width * map->height;
                for (kind_index = 0u; kind_index < 4u; ++kind_index) {
                    uint8_t item_index;
                    for (item_index = 0u; item_index < counts[kind_index];
                         ++item_index) {
                        uint8_t value = 0u;
                        if (FirestaffX68kDungeonDat_GetMapMetadata(
                                &dungeon_dat, (uint8_t)map_index,
                                kinds[kind_index], item_index, &value) != 0 ||
                            value != dungeon[metadata_offset + item_index]) {
                            fprintf(stderr,
                                    "FAIL: X68000 map %u metadata kind %u item %u changed source order\n",
                                    (unsigned)map_index, (unsigned)kind_index,
                                    (unsigned)item_index);
                            goto done;
                        }
                    }
                    metadata_offset += counts[kind_index];
                }
                if (FirestaffX68kDungeonDat_GetMapMetadata(
                        &dungeon_dat, (uint8_t)map_index,
                        FIRESTAFF_X68K_DUNGEON_MAP_DOOR_ORNAMENT,
                        map->door_ornament_count, &start_square) == 0) {
                    fputs("FAIL: X68000 dungeon reader accepted out-of-range map metadata\n",
                          stderr);
                    goto done;
                }
            }
        }
        printf("PASS: X68000 start location map=0 (%u,%u) dir=%u square=0x%02x first-thing=0x%04x; verified %lu square lists/%lu links, %lu owner lists/%lu items, all map cells and metadata\n",
               (unsigned)map_x, (unsigned)map_y, (unsigned)direction,
               (unsigned)start_square, (unsigned)start_thing,
               (unsigned long)square_list_count,
               (unsigned long)square_list_thing_count,
               (unsigned long)ownership_list_count,
               (unsigned long)ownership_thing_count);
    }
    printf("PASS: read-only X68000 HDM import authenticated %lu root entries; "
           "SWOOSH matches ReDMCSB; all 562 X30J graphics spans and the "
           "complete raw dungeon metadata layout match ReDMCSB\n",
           (unsigned long)entries);
    result = 0;

done:
    free(graphics);
    free(dungeon);
    free(ftlcode);
    free(swoosh);
    free(image);
    return result;
}
