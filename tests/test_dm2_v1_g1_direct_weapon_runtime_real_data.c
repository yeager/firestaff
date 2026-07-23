/* Real-data-only direct DB5 Weapon receipt test for canonical PC G1. */

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
    DM2_V1_G1RuntimeMapWeaponReceipt weapons;
    DM2_V1_G1RuntimeMapWeaponReceipt sentinel;

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
    if (!dm2_v1_dungeon_materialize_g1_runtime_map_weapons(
            &dungeon, 17, &weapons) ||
        weapons.committed != 1 || weapons.incomplete_world != 1 ||
        weapons.map != 17 || weapons.weapon_root_count != 2 ||
        weapons.weapon_record_reads != 2 ||
        weapons.generic_record_reads != 0 || weapons.blocked_record_reads != 0 ||
        weapons.weapons[0].x != 5 || weapons.weapons[0].y != 8 ||
        weapons.weapons[0].object_id != 0xd407 ||
        weapons.weapons[0].index != 7 || weapons.weapons[0].direction != 3 ||
        weapons.weapons[0].item_type != 126 ||
        weapons.weapons[0].important != 1 || weapons.weapons[0].charges != 15 ||
        weapons.weapons[1].x != 6 || weapons.weapons[1].y != 1 ||
        weapons.weapons[1].object_id != 0xd407 ||
        weapons.weapons[1].index != 7 || weapons.weapons[1].direction != 3 ||
        weapons.weapons[1].item_type != 126 ||
        weapons.weapons[1].important != 1 || weapons.weapons[1].charges != 15) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: direct DB5 Weapon receipt changed canonical source fields\n",
              stderr);
        return 1;
    }
    sentinel.committed = -1;
    if (dm2_v1_dungeon_materialize_g1_runtime_map_weapons(
            &dungeon, 28, &sentinel) != 0 || sentinel.committed != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: invalid map mutated Weapon receipt\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: direct DB5 Weapon roots use only source-proven payload fields");
    return 0;
}
