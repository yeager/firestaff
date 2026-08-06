/* Real-data-only champion mirror receipt for canonical PC DM2 G1. */

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
    const char *path = resolve_dungeon_dat_path(argc, argv, path_buf,
                                                sizeof(path_buf));
    unsigned char *bytes;
    int size;
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1ChampionMirrorReceipt mirrors;

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
    if (!dm2_v1_dungeon_collect_g1_champion_mirrors(&dungeon, &mirrors) ||
        !mirrors.committed || !mirrors.incomplete_world ||
        mirrors.mirror_count != 16 || mirrors.actuator_record_reads <= 0) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: source G1 champion mirrors were not retained\n", stderr);
        return 1;
    }
    for (int i = 0; i < mirrors.mirror_count; ++i) {
        const DM2_V1_G1ChampionMirrorRoot *mirror = &mirrors.mirrors[i];
        if (mirror->map < 0 || mirror->map >= dungeon.level_count ||
            mirror->x < 0 || mirror->x >= dungeon.level_widths[mirror->map] ||
            mirror->y < 0 || mirror->y >= dungeon.level_heights[mirror->map] ||
            mirror->actuator_data != 0x1ffu ||
            mirror->dynamic_hero_type != 0xffu ||
            mirror->dynamic_load_id != 0x16ffffffu) {
            dm2_v1_dungeon_free(&dungeon);
            fputs("FAIL: source champion mirror lost its dynamic-load key\n",
                  stderr);
            return 1;
        }
    }
    printf("PASS: retained %d source G1 champion-mirror marker roots\n",
           mirrors.mirror_count);
    dm2_v1_dungeon_free(&dungeon);
    return 0;
}
