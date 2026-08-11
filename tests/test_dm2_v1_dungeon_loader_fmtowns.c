/*
 * test_dm2_v1_dungeon_loader_fmtowns.c
 *
 * Validates DM2 dungeon loader against real FM Towns DUNGEON.DAT.
 * Source: ~/.firestaff/data/dm2/fmtowns_iso/DATA/DUNGEON.DAT
 *
 * The FM Towns DUNGEON.DAT uses magic 0x3094 at offset 2 instead of
 * PC's 0x3147 ('G1'), but has the same header layout, map definition
 * format, and byte-square tile data.
 */

#include "dm2_v1_dungeon_loader.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *data;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    data = malloc((size_t)sz);
    if (!data) { fclose(f); return NULL; }
    if (fread(data, 1, (size_t)sz, f) != (size_t)sz) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return data;
}

static void test_fmtowns_load(const char *path) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_DungeonData dungeon;
    int result;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read FM Towns DUNGEON.DAT at %s\n", path);
        return;
    }

    printf("  FM Towns DUNGEON.DAT: %zu bytes\n", data_size);
    assert(data_size == 37954);

    /* Magic should be 0x3094 */
    assert(data[2] == 0x94 && data[3] == 0x30);

    result = dm2_v1_dungeon_load(&dungeon, data, (int)data_size);
    assert(result == 0);
    printf("  PASS: FM Towns DUNGEON.DAT loaded successfully\n");
    {
        int mirrors = 0;
        DM2_V1_G1ChampionMirrorReceipt receipt;
        for (int i = 0; i < dungeon.thing_type_counts[3]; ++i) {
            int type = -1;
            int size = 0;
            const uint8_t *record = dm2_v1_dungeon_get_thing_record(
                &dungeon, (uint16_t)((3u << 10) | (unsigned)i),
                &type, NULL, &size);
            if (record && type == 3 && size >= 8 &&
                ((record[2] | ((uint16_t)record[3] << 8)) & 0x7fu) == 0x7eu) {
                ++mirrors;
            }
        }
        assert(mirrors == 16);
        assert(dm2_v1_dungeon_collect_g1_champion_mirrors(&dungeon, &receipt));
        assert(receipt.committed && receipt.mirror_count == 16);
        printf("  PASS: 16 FM Towns champion mirror records\n");
    }

    /* File_header.nMaps is the byte at offset 4 (SKWIN/DME.h:93-101,
     * mirrored by docs/dm2_save_format.md's section order). Both the PC and
     * FM Towns releases store 0x2c = 44 there. The old expectation of 28 was
     * reading offset 6, which is cwTextData -- verified against the real
     * files: map descriptors 0..43 are all well-formed with monotonically
     * increasing map-data offsets (0x0000..0x2ff0 inside the 12615-byte
     * cbMapData region), and the 45th 16-byte slot is already the column
     * index table. */
    assert(dungeon.level_count == 44);
    printf("  PASS: 44 maps\n");

    /* Byte-sized squares */
    assert(dungeon.square_bytes == 1);
    printf("  PASS: byte squares\n");

    /* File_header.nRecords[16] starts at offset 12 (SKWIN/DME.h:101), so
     * thing_type_counts[0] is the first DB pool. The old expectations began
     * at 209, which is nRecords[1]: they were written against the shifted
     * header offsets in dm2_v1_try_load_pc_g1_byte_layout, which reads the
     * pool table from offset 14. The real FM Towns header holds
     * 53, 209, 448, 1020, 280, 169 at offsets 12, 14, 16, 18, 20, 22. */
    assert(dungeon.thing_type_counts[0] == 53);
    assert(dungeon.thing_type_counts[1] == 209);
    assert(dungeon.thing_type_counts[2] == 448);
    assert(dungeon.thing_type_counts[3] == 1020);
    assert(dungeon.thing_type_counts[4] == 280);
    assert(dungeon.thing_type_counts[5] == 169);
    printf("  PASS: thing type counts match header\n");

    /* Verify some map data was parsed — raw_map_data_base should be set */
    assert(dungeon.raw_map_data_base >= 0);
    printf("  PASS: raw map data base set (%d)\n", dungeon.raw_map_data_base);

    /* Column index should be set */
    assert(dungeon.column_index_base >= 0);
    printf("  PASS: column index base set (%d)\n", dungeon.column_index_base);

    printf("  PASS: FM Towns dungeon loader\n");
    free(data);
}

static void test_pc_still_works(const char *path) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_DungeonData dungeon;
    int result;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read PC DUNGEON.DAT at %s\n", path);
        return;
    }

    /* Magic should be 0x3147 = 'G1' */
    assert(data[2] == 0x47 && data[3] == 0x31);

    result = dm2_v1_dungeon_load(&dungeon, data, (int)data_size);
    assert(result == 0);
    assert(dungeon.level_count == 28);
    printf("  PASS: PC DUNGEON.DAT still loads\n");

    free(data);
}

int main(void) {
    const char *home;
    char fm_path[512], pc_path[512];

    printf("DM2 FM Towns dungeon loader tests:\n");

    home = getenv("HOME");
    if (!home) {
        printf("  SKIP: HOME not set\n");
        return 0;
    }

    snprintf(fm_path, sizeof(fm_path),
             "%s/.firestaff/data/dm2/fmtowns_iso/DATA/DUNGEON.DAT", home);
    test_fmtowns_load(fm_path);

    snprintf(pc_path, sizeof(pc_path),
             "%s/.firestaff/data/dm2/DUNGEON.DAT", home);
    test_pc_still_works(pc_path);

    printf("\nAll FM Towns dungeon loader tests passed.\n");
    return 0;
}
