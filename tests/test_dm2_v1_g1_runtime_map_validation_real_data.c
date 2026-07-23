/* Real-data-only c_map admission test for the canonical PC G1 DUNGEON.DAT. */

#include "dm2_v1_dungeon_loader.h"

#include <stdio.h>
#include <stdlib.h>

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
    DM2_V1_G1RuntimeMapValidationReceipt map0;
    DM2_V1_G1RuntimeMapValidationReceipt map16;
    DM2_V1_G1RuntimeMapValidationReceipt sentinel;

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
    if (!dm2_v1_dungeon_validate_g1_runtime_map(&dungeon, 0, &map0) ||
        map0.committed != 1 || map0.incomplete_world != 1 ||
        map0.width != 7 || map0.height != 10 || map0.map_data_base != 31667 ||
        map0.map_data_offset != 0 || map0.map_data_byte_count != 70u ||
        map0.map_data_hash == 0u || map0.root_count != 22 ||
        map0.direct_root_count != 22 || map0.db3_root_count != 0 ||
        map0.db4_root_count != 0 || map0.blocked_root_count != 0 ||
        map0.generic_record_reads != 0 || map0.blocked_record_reads != 0) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: map 0 runtime admission changed G1 provenance\n", stderr);
        return 1;
    }
    if (!dungeon.initial_party_pose_valid || dungeon.initial_party_x != 3 ||
        dungeon.initial_party_y != 5 || dungeon.initial_party_dir != 2 ||
        dm2_v1_dungeon_get_tile_raw(&dungeon, 0, 3, 5) < 0) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: canonical G1 start pose is not map-owned\n", stderr);
        return 1;
    }
    if (!dm2_v1_dungeon_validate_g1_runtime_map(&dungeon, 16, &map16) ||
        map16.committed != 1 || map16.incomplete_world != 1 ||
        map16.map != 16 || map16.map_data_hash == 0u ||
        map16.db3_root_count != 11 || map16.db4_root_count != 2 ||
        map16.blocked_root_count != 3 || map16.generic_record_reads != 0 ||
        map16.blocked_record_reads != 0) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: map 16 did not preserve its blocked G1 roots\n", stderr);
        return 1;
    }
    sentinel.committed = -1;
    if (dm2_v1_dungeon_validate_g1_runtime_map(&dungeon, 28, &sentinel) != 0 ||
        sentinel.committed != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: invalid map mutated the runtime admission receipt\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: real G1 maps validate root provenance without w0 traversal");
    return 0;
}
