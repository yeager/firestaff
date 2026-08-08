/* Canonical PC File_header must not enter the legacy G1 root census. */

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
    if (argc >= 2) return argv[1];
    root = getenv("FIRESTAFF_DM2_DATA_DIR");
    if (root && root[0]) {
        snprintf(buf, buf_size, "%s/dungeon.dat", root);
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

    if (!path) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    if (!(bytes = read_file(path, &size))) {
        fputs("FAIL: selected canonical DM2 data is unreadable\n", stderr);
        return 1;
    }
    if (bytes[0] != 0 || bytes[1] != 0 || bytes[2] != 0x47 ||
        bytes[3] != 0x31 || bytes[4] != 44 || bytes[5] != 0 ||
        bytes[6] != 28 || bytes[7] != 0 ||
        dm2_v1_dungeon_load(&dungeon, bytes, size) != 0) {
        free(bytes);
        fputs("FAIL: canonical File_header input was not accepted\n", stderr);
        return 1;
    }
    free(bytes);
    receipt.committed = -1;
    if (dm2_v1_dungeon_materialize_g1_partial_map_boot(&dungeon, &receipt) != 0 ||
        receipt.committed != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: File_header entered the legacy G1 root census\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: File_header leaves the legacy G1 root census unavailable");
    return 0;
}
