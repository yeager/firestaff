#include "firestaff_dungeon_query.h"

#include <stdio.h>
#include <stdlib.h>

static int read_file(const char *path, unsigned char **out, int *out_size)
{
    FILE *file;
    long size;
    unsigned char *bytes;

    if (!path || !out || !out_size) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0) { fclose(file); return 0; }
    size = ftell(file);
    if (size <= 0 || size > 1024 * 1024 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    bytes = (unsigned char *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (int)size;
    return 1;
}

int main(int argc, char **argv)
{
    unsigned char *bytes = NULL;
    int size = 0;
    int failures = 0;

    if (argc < 2) {
        puts("SKIP: pass the extracted PC34 DUNGEON.DAT path for real-data verification");
        return 0;
    }
    if (!read_file(argv[1], &bytes, &size)) {
        fprintf(stderr, "FAIL: could not read %s\n", argv[1]);
        return 1;
    }
    if (fs_dungeon_load_dat(bytes, size) != 14) ++failures;
    if (fs_dungeon_get_width() != 18 || fs_dungeon_get_height() != 19) ++failures;
    if (fs_dungeon_get_start_x() != 1 || fs_dungeon_get_start_y() != 3 ||
        fs_dungeon_get_start_dir() != 2) ++failures;
    /* Map 0 raw square (x=1,y=2) is a 3/4-closed door in PC34. */
    if (fs_dungeon_get_square_type(1, 2) != 4 ||
        fs_dungeon_get_door_type(1, 2) != 1 ||
        fs_dungeon_get_door_state(1, 2) != 4) ++failures;
    /* Object-free cells use the real PC34 DUNGEON.DAT seed, dimensions,
     * random-ornament counts and F0170/F0171 arithmetic. Map 0's raw
     * wall at (0,12), viewed westward, selects ordinal 2; its corridor at
     * (4,2) selects floor ordinal 3. */
    if (fs_dungeon_get_wall_ornament(0, 12, 3) != 2 ||
        fs_dungeon_get_floor_ornament(4, 2) != 3) ++failures;
    free(bytes);
    if (failures) {
        fprintf(stderr, "FAIL: real DM1 DUNGEON.DAT state checks (%d)\n", failures);
        return 1;
    }
    puts("ok: real DM1 DUNGEON.DAT layout/state verified");
    return 0;
}
