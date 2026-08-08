/*
 * Real-data-only c_map tile accessor gate.
 *
 * Covers skproject/SKWIN/xxx.cpp:
 *   GET_ADDRESS_OF_TILE_RECORD
 *   IS_TILE_SOLID
 *   GET_TILE_VALUE
 *
 * The test accepts only canonical PC File_header DUNGEON.DAT. It builds no fixture,
 * follows no GenericRecord::w0 chain, and does not synthesize map roots.
 */

#include "dm2_v1_dungeon_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t fnv1a(const unsigned char *data, int size)
{
    uint32_t hash = 2166136261u;
    int i;

    for (i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

static unsigned char *read_file(const char *path, int *out_size)
{
    FILE *file = fopen(path, "rb");
    long size;
    unsigned char *bytes = NULL;

    *out_size = 0;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) != 39437L || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (int)size;
    return bytes;
}

int main(int argc, char **argv)
{
    const char *path = NULL;
    unsigned char *bytes = NULL;
    int size = 0;
    DM2_V1_DungeonData dungeon;
    DM2_V1_SkprojectTileValueReceipt value;
    DM2_V1_SkprojectTileSolidReceipt solid;
    DM2_V1_SkprojectTileRecordAddressReceipt address;
    int x;
    int y;
    int root_count = 0;
    int solid_count = 0;
    int passage_count = 0;
    int first_root_x = -1;
    int first_root_y = -1;
    int first_plain_x = -1;
    int first_plain_y = -1;

    if (argc == 2) path = argv[1];
    if (!path) path = getenv("FIRESTAFF_DM2_DUNGEON_DAT");
    if (!path) {
        fputs("SKIP: FIRESTAFF_DM2_DUNGEON_DAT or argv[1] is required\n",
              stderr);
        return 77;
    }

    bytes = read_file(path, &size);
    if (!bytes || bytes[0] != 0 || bytes[1] != 0 ||
        bytes[2] != 0x47 || bytes[3] != 0x31 ||
        bytes[4] != 44 || bytes[5] != 0 || bytes[6] != 28 || bytes[7] != 0 ||
        bytes[8] != 1 || bytes[9] != 1 || fnv1a(bytes, size) == 0u ||
        dm2_v1_dungeon_load(&dungeon, bytes, size) != 0) {
        free(bytes);
        fputs("FAIL: canonical File_header DUNGEON.DAT was not accepted\n", stderr);
        return 1;
    }
    free(bytes);

    if (dungeon.level_count != 44 || dungeon.level_widths[0] != 7 ||
        dungeon.level_heights[0] != 10 || dungeon.raw_map_data_base != 26820 ||
        dungeon.level_offsets[0] != 0 || dungeon.g1_extension_base != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: canonical map-0 source span changed\n", stderr);
        return 1;
    }

    for (x = 0; x < dungeon.level_widths[0]; ++x) {
        for (y = 0; y < dungeon.level_heights[0]; ++y) {
            int raw = dm2_v1_dungeon_get_tile_raw(&dungeon, 0, x, y);
            if (!dm2_v1_skproject_get_tile_value(&dungeon, 0, x, y, &value) ||
                !value.valid ||
                value.raw_tile != (uint16_t)raw ||
                value.tile_value != (uint8_t)(((uint8_t)raw >> 5) & 0x07u) ||
                strcmp(value.source_symbol, "DM2_GET_TILE_VALUE") != 0) {
                dm2_v1_dungeon_free(&dungeon);
                fputs("FAIL: GET_TILE_VALUE lost source byte semantics\n",
                      stderr);
                return 1;
            }
            if (!dm2_v1_skproject_is_tile_solid(&dungeon, 0, x, y, &solid) ||
                !solid.valid ||
                solid.raw_tile != (uint16_t)raw ||
                strcmp(solid.source_symbol, "DM2_IS_TILE_SOLID") != 0) {
                dm2_v1_dungeon_free(&dungeon);
                fputs("FAIL: IS_TILE_SOLID receipt was not source-named\n",
                      stderr);
                return 1;
            }
            if (solid.is_solid) ++solid_count;
            else ++passage_count;
            if (((uint8_t)raw & 0x10u) != 0u) {
                ++root_count;
                if (first_root_x < 0) {
                    first_root_x = x;
                    first_root_y = y;
                }
            } else if (first_plain_x < 0) {
                first_plain_x = x;
                first_plain_y = y;
            }
        }
    }

    if (root_count != 23 || solid_count != 37 || passage_count != 33 ||
        first_root_x < 0 || first_plain_x < 0) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: canonical map-0 tile/root census changed\n", stderr);
        return 1;
    }

    if (!dm2_v1_skproject_get_address_of_tile_record(
            &dungeon, 0, first_root_x, first_root_y, &address) ||
        !address.valid ||
        address.direct_or_proven_extension_address != 1 ||
        address.record_size <= 0 ||
        strcmp(address.source_symbol,
               "DM2_GET_ADDRESS_OF_TILE_RECORD") != 0) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: GET_ADDRESS_OF_TILE_RECORD did not bind a real root\n",
              stderr);
        return 1;
    }

    memset(&address, 0, sizeof(address));
    if (dm2_v1_skproject_get_address_of_tile_record(
            &dungeon, 0, first_plain_x, first_plain_y, &address) ||
        !address.blocked_no_tile_record_link ||
        address.blocked_missing_record) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: plain tile promoted a synthetic record address\n",
              stderr);
        return 1;
    }

    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: real G1 c_map tile access uses source map bytes and roots");
    return 0;
}
