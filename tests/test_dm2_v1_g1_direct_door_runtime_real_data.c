/* Real-data-only direct DB0 Door receipt test for canonical PC G1. */

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
    DM2_V1_G1RuntimeMapDoorReceipt doors;
    DM2_V1_G1RuntimeMapDoorReceipt sentinel;
    const DM2_V1_G1DirectDoorRoot *front_door = NULL;
    uint8_t door_ornates[16];
    int door_ornate_count;
    int ornate_map = -1;

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
    for (int map = 0; map < dungeon.level_count; ++map) {
        door_ornate_count = dm2_v1_dungeon_get_map_door_ornate_list(
            &dungeon, map, door_ornates, (int)sizeof(door_ornates));
        if (door_ornate_count > 0) {
            ornate_map = map;
            break;
        }
    }
    if (door_ornate_count <= 0 || door_ornate_count > (int)sizeof(door_ornates) ||
        ornate_map < 0 || dungeon.map_door_ornate_count[ornate_map] != door_ornate_count ||
        door_ornates[0] == 0) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: canonical G1 map door-ornate list was not source-owned\n",
              stderr);
        return 1;
    }
    if (!dm2_v1_dungeon_materialize_g1_runtime_map_doors(
            &dungeon, 9, &doors) ||
        doors.committed != 1 || doors.incomplete_world != 1 || doors.map != 9 ||
        doors.door_root_count != 3 || doors.door_record_reads != 3 ||
        doors.generic_record_reads != 0 || doors.blocked_record_reads != 0 ||
        doors.doors[0].x != 1 || doors.doors[0].y != 5 ||
        doors.doors[0].object_id != 0x000c || doors.doors[0].index != 12 ||
        doors.doors[0].button != 0 || doors.doors[0].opening_dir != 1 ||
        doors.doors[1].x != 7 || doors.doors[1].y != 3 ||
        doors.doors[1].object_id != 0x0006 || doors.doors[1].index != 6 ||
        doors.doors[1].button != 1 || doors.doors[1].button_state != 1 ||
        doors.doors[1].opening_dir != 1 || doors.doors[1].door_type != 0 ||
        doors.doors[2].x != 16 || doors.doors[2].y != 3 ||
        doors.doors[2].object_id != 0x001e || doors.doors[2].index != 30 ||
        doors.doors[2].button != 0 || doors.doors[2].opening_dir != 1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: direct DB0 Door receipt changed canonical source fields\n",
              stderr);
        return 1;
    }
    if (!dm2_v1_g1_runtime_map_door_at(&doors, 7, 3, &front_door) ||
        !front_door || front_door != &doors.doors[1] ||
        front_door->button != 1 || front_door->door_type != 0 ||
        front_door->opening_dir != 1 ||
        dm2_v1_g1_runtime_map_door_at(&doors, 0, 0, &front_door) != 0 ||
        front_door != NULL) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: direct DB0 door lookup reopened or invented a record\n",
              stderr);
        return 1;
    }
    sentinel.committed = -1;
    if (dm2_v1_dungeon_materialize_g1_runtime_map_doors(
            &dungeon, 28, &sentinel) != 0 || sentinel.committed != -1) {
        dm2_v1_dungeon_free(&dungeon);
        fputs("FAIL: invalid map mutated Door receipt\n", stderr);
        return 1;
    }
    dm2_v1_dungeon_free(&dungeon);
    puts("PASS: direct DB0 Door roots and map-local ornate list use source fields");
    return 0;
}
