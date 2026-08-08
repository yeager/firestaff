/* Real-data-only champion-mirror census for canonical PC-DOS DM2 File_header. */

#include "dm2_v1_dungeon_loader.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *read_file(const char *path, int *out_size)
{
    FILE *file = fopen(path, "rb");
    long size;
    unsigned char *bytes;

    *out_size = 0;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || size > INT_MAX ||
        fseek(file, 0, SEEK_SET) != 0) {
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

    if (!path) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    bytes = read_file(path, &size);
    if (!bytes) {
        fprintf(stderr, "FAIL: selected canonical DM2 data is unreadable: %s\n",
                path);
        return 1;
    }
    if (bytes[2] != 0x47 || bytes[3] != 0x31 || bytes[4] != 44 ||
        bytes[6] != 28 ||
        dm2_v1_dungeon_load(&dungeon, bytes, size) != 0) {
        free(bytes);
        fputs("FAIL: canonical G1 input was not accepted\n", stderr);
        return 1;
    }
    free(bytes);
    if (dungeon.level_count != 44 || !dungeon.initial_party_pose_valid ||
        dungeon.initial_party_x != 1 || dungeon.initial_party_y != 8 ||
        dungeon.initial_party_dir != 0) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: canonical File_header did not retain its 44-map boot state\n",
              stderr);
        return 1;
    }
    memset(&mirrors, 0, sizeof(mirrors));
    if (!dm2_v1_dungeon_collect_g1_champion_mirrors(&dungeon, &mirrors) ||
        !mirrors.committed || !mirrors.incomplete_world ||
        mirrors.mirror_count != 16 || mirrors.actuator_record_reads <= 0) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: canonical File_header mirror roots were not retained\n",
              stderr);
        return 1;
    }
    {
        unsigned int type_mask = 0u;
        int i;

        for (i = 0; i < mirrors.mirror_count; ++i) {
            const DM2_V1_G1ChampionMirrorRoot *mirror = &mirrors.mirrors[i];
            if (mirror->map != 0 || mirror->dynamic_hero_type > 15u ||
                mirror->actuator_data != mirror->dynamic_hero_type ||
                mirror->dynamic_load_id !=
                    (0x1600ffffu +
                     ((unsigned int)mirror->dynamic_hero_type << 16))) {
                dm2_v1_dungeon_free(&dungeon);
                fputs("FAIL: mirror root diverges from c_hero/c_loadlevel fields\n",
                      stderr);
                return 1;
            }
            type_mask |= 1u << mirror->dynamic_hero_type;
        }
        if (type_mask != 0xffffu ||
            mirrors.mirrors[0].x != 0 || mirrors.mirrors[0].y != 0 ||
            mirrors.mirrors[0].dynamic_hero_type != 14u ||
            mirrors.mirrors[15].x != 6 || mirrors.mirrors[15].y != 8 ||
            mirrors.mirrors[15].dynamic_hero_type != 12u) {
            dm2_v1_dungeon_free(&dungeon);
            fputs("FAIL: canonical mirror census changed unexpectedly\n", stderr);
            return 1;
        }
    }
    puts("PASS: canonical 44-map File_header retains all 16 direct champion mirrors");
    dm2_v1_dungeon_free(&dungeon);
    return 0;
}
