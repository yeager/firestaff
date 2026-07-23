/* Real-data-only direct DB4 Creature receipt test for canonical PC G1. */

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
    DM2_V1_G1RuntimeMapCreatureReceipt creatures;
    DM2_V1_G1RuntimeMapCreatureReceipt sentinel;
    const unsigned char *first_record;
    const unsigned char *fourth_record;

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
    if (!dm2_v1_dungeon_materialize_g1_runtime_map_creatures(
            &dungeon, 17, &creatures) ||
        creatures.committed != 1 || creatures.incomplete_world != 1 ||
        creatures.map != 17 || creatures.creature_root_count != 4 ||
        creatures.creature_record_reads != 4 ||
        creatures.generic_record_reads != 0 || creatures.blocked_record_reads != 0 ||
        creatures.creatures[0].x != 4 || creatures.creatures[0].y != 4 ||
        creatures.creatures[0].object_id != 0x1098 ||
        creatures.creatures[0].index != 152 ||
        creatures.creatures[0].creature_type != 10 ||
        creatures.creatures[0].hit_points_1 != 0 ||
        creatures.creatures[3].x != 5 || creatures.creatures[3].y != 5 ||
        creatures.creatures[3].object_id != 0x10a8 ||
        creatures.creatures[3].index != 168 ||
        creatures.creatures[3].creature_type != 1 ||
        creatures.creatures[3].hit_points_1 != 0) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: direct DB4 Creature receipt changed canonical source fields\n",
              stderr);
        return 1;
    }
    first_record = dm2_v1_dungeon_get_thing_record(
        &dungeon, creatures.creatures[0].object_id, NULL, NULL, NULL);
    fourth_record = dm2_v1_dungeon_get_thing_record(
        &dungeon, creatures.creatures[3].object_id, NULL, NULL, NULL);
    if (!first_record || !fourth_record ||
        creatures.creatures[0].direction != (first_record[15] & 3u) ||
        creatures.creatures[3].direction != (fourth_record[15] & 3u)) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: direct DB4 Creature receipt did not use b15 facing\n",
              stderr);
        return 1;
    }
    sentinel.committed = -1;
    if (dm2_v1_dungeon_materialize_g1_runtime_map_creatures(
            &dungeon, 28, &sentinel) != 0 || sentinel.committed != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: invalid map mutated Creature receipt\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: direct DB4 Creature roots use only source-proven payload fields");
    return 0;
}
