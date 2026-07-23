/* Real-data c_map -> c_record direct-address test for canonical PC G1. */

#include "dm2_v1_dungeon_loader.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, int *out_size)
{
    FILE *file = fopen(path, "rb");
    long size;
    unsigned char *bytes;

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

static const char *resolve_dungeon_dat_path(int argc, char **argv,
                                            char *buf, size_t buf_size)
{
    const char *root;
    const char *home;
    if (argc >= 2) return argv[1];
    root = getenv("FIRESTAFF_DM2_DATA_DIR");
    if (root && root[0]) {
        snprintf(buf, buf_size, "%s/dungeon.dat", root);
        return buf;
    }
    home = getenv("HOME");
    if (home && home[0]) {
        snprintf(buf, buf_size, "%s/.firestaff/data/dm2/data/dungeon.dat", home);
        return buf;
    }
    return NULL;
}

static int expect_address(const DM2_V1_DungeonData *d, int level, int x, int y,
                          uint16_t object_id, int type, int index,
                          int offset, int size)
{
    DM2_V1_G1DirectRootRecordAddressReceipt receipt;

    if (!dm2_v1_dungeon_resolve_g1_direct_root_record(
            d, level, x, y, &receipt) ||
        receipt.committed != 1 || receipt.incomplete_world != 1 ||
        receipt.level != level || receipt.x != x || receipt.y != y ||
        receipt.object_id != object_id || receipt.type != type ||
        receipt.index != index || receipt.record_offset != offset ||
        receipt.record_size != size) {
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    char path_buf[1024];
    const char *path = resolve_dungeon_dat_path(argc, argv, path_buf, sizeof(path_buf));
    unsigned char *bytes = NULL;
    int size;
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1DirectRootRecordAddressReceipt sentinel;

    if (!path || !(bytes = read_file(path, &size))) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    if (bytes[2] != 0x47 || bytes[3] != 0x31 || bytes[6] != 28 ||
        dm2_v1_dungeon_load(&dungeon, bytes, size) != 0) {
        free(bytes);
        fputs("FAIL: canonical G1 input was not accepted\n", stderr);
        return 1;
    }
    free(bytes);
    if (!expect_address(&dungeon, 9, 7, 3, 0x0006, 0, 6, 6966, 4) ||
        !expect_address(&dungeon, 0, 0, 4, 0x04a5, 1, 165, 8800, 6) ||
        !expect_address(&dungeon, 5, 9, 13, 0x084a, 2, 74, 11562, 4) ||
        !expect_address(&dungeon, 5, 6, 14, 0x4c04, 3, 4, 15378, 8) ||
        !expect_address(&dungeon, 5, 10, 5, 0x1003, 4, 3, 17786, 16) ||
        !expect_address(&dungeon, 17, 5, 8, 0xd407, 5, 7, 20534, 4) ||
        !expect_address(&dungeon, 9, 11, 19, 0xe408, 9, 8, 21842, 8)) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: direct G1 c_map/c_record address changed\n", stderr);
        return 1;
    }
    sentinel.committed = -1;
    if (dm2_v1_dungeon_resolve_g1_direct_root_record(
            &dungeon, 16, 0, 0, &sentinel) != 0 || sentinel.committed != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: non-direct root mutated direct address receipt\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: direct G1 tile roots resolve to bounded c_record addresses");
    return 0;
}
