/* Real-data-only direct DB3 Actuator receipt test for canonical PC G1. */

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
    DM2_V1_G1RuntimeMapActuatorReceipt actuators;
    DM2_V1_G1RuntimeMapActuatorReceipt sentinel;

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
    if (!dm2_v1_dungeon_materialize_g1_runtime_map_actuators(
            &dungeon, 5, &actuators) ||
        actuators.committed != 1 || actuators.incomplete_world != 1 ||
        actuators.map != 5 || actuators.actuator_root_count != 16 ||
        actuators.actuator_record_reads != 16 ||
        actuators.generic_record_reads != 0 || actuators.blocked_record_reads != 0 ||
        actuators.actuators[0].x != 6 || actuators.actuators[0].y != 14 ||
        actuators.actuators[0].object_id != 0x4c04 ||
        actuators.actuators[0].index != 4 || actuators.actuators[0].direction != 1 ||
        actuators.actuators[0].actuator_type != 80 ||
        actuators.actuators[0].actuator_data != 65 ||
        actuators.actuators[0].graphic_number != 4 ||
        actuators.actuators[0].disabled != 0 ||
        actuators.actuators[0].delay != 3 ||
        actuators.actuators[0].sound_effect != 0 ||
        actuators.actuators[0].revert_effect != 1 ||
        actuators.actuators[0].action_type != 2 ||
        actuators.actuators[0].once_only != 0 ||
        actuators.actuators[0].active_status != 0 ||
        actuators.actuators[0].target_direction != 1 ||
        actuators.actuators[0].target_x != 18 || actuators.actuators[0].target_y != 25 ||
        actuators.actuators[15].x != 15 || actuators.actuators[15].y != 21 ||
        actuators.actuators[15].object_id != 0x0c82 ||
        actuators.actuators[15].index != 130) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: direct DB3 Actuator receipt changed canonical source fields\n",
              stderr);
        return 1;
    }
    sentinel.committed = -1;
    if (dm2_v1_dungeon_materialize_g1_runtime_map_actuators(
            &dungeon, 28, &sentinel) != 0 || sentinel.committed != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: invalid map mutated Actuator receipt\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: direct DB3 Actuator roots use only source-proven payload fields");
    return 0;
}
