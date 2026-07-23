/* Real-data-only direct-root family census for canonical PC G1. */

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

int main(int argc, char **argv)
{
    char path_buf[1024];
    const char *path = resolve_dungeon_dat_path(argc, argv, path_buf, sizeof(path_buf));
    unsigned char *bytes = NULL;
    int size;
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1PartialMapBootReceipt receipt;

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
    if (!dm2_v1_dungeon_materialize_g1_partial_map_boot(&dungeon, &receipt) ||
        receipt.committed != 1 || receipt.incomplete != 1 ||
        receipt.direct_root_count != 676 ||
        receipt.direct_root_count_by_type[0] != 14 ||
        receipt.direct_root_count_by_type[1] != 357 ||
        receipt.direct_root_count_by_type[2] != 204 ||
        receipt.direct_root_count_by_type[3] != 64 ||
        receipt.direct_root_count_by_type[4] != 33 ||
        receipt.direct_root_count_by_type[5] != 3 ||
        receipt.direct_root_count_by_type[6] != 0 ||
        receipt.direct_root_count_by_type[7] != 0 ||
        receipt.direct_root_count_by_type[8] != 0 ||
        receipt.direct_root_count_by_type[9] != 1 ||
        receipt.direct_root_count_by_type[10] != 0 ||
        receipt.direct_root_count_by_type[11] != 0 ||
        receipt.direct_root_count_by_type[12] != 0 ||
        receipt.direct_root_count_by_type[13] != 0 ||
        receipt.direct_root_count_by_type[14] != 0 ||
        receipt.direct_root_count_by_type[15] != 0 ||
        receipt.blocked_root_count != 5 ||
        receipt.blocked_root_count_by_type[8] != 1 ||
        receipt.blocked_root_count_by_type[10] != 4) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: direct/blocked G1 root-family census changed\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: canonical G1 has no direct record family after DB9");
    return 0;
}
