#include "firestaff_dungeon_query.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void w16(unsigned char *p, unsigned short value)
{
    p[0] = (unsigned char)(value & 0xffu);
    p[1] = (unsigned char)(value >> 8);
}

static int sensor_wall_cell_regression(void)
{
    unsigned char data[73];
    const size_t map = 44;
    const size_t column_sft = 60;
    const size_t square_first_thing = 62;
    const size_t sensor = 64;
    const size_t raw_map = 72;

    memset(data, 0, sizeof(data));
    w16(data + 2, 1);                 /* raw map bytes */
    data[4] = 1;                      /* map count */
    w16(data + 10, 1);                /* SquareFirstThing count */
    w16(data + 18, 1);                /* C03 sensor count */
    w16(data + map + 8, 0);           /* 1x1 map dimensions */
    w16(data + column_sft, 0);        /* x=0 starts at SFT index 0 */
    w16(data + square_first_thing, (unsigned short)((3u << 10) | (2u << 14)));
    w16(data + sensor, 0xffffu);      /* C03 next Thing */
    w16(data + sensor + 4, (unsigned short)(5u << 12));
    data[raw_map] = 0x10u;            /* wall with a Thing list */

    if (fs_dungeon_load_dat(data, (int)sizeof(data)) != 1) return 0;
    /* North view sees wall cell 2, so only this C03 supplies ordinal 5. */
    if (fs_dungeon_get_wall_ornament(0, 0, 0) != 5) return 0;
    w16(data + square_first_thing, (unsigned short)((3u << 10) | (1u << 14)));
    if (fs_dungeon_load_dat(data, (int)sizeof(data)) != 1) return 0;
    return fs_dungeon_get_wall_ornament(0, 0, 0) == 0;
}

int main(int argc, char **argv)
{
    unsigned char *bytes = NULL;
    int size = 0;
    int failures = 0;

    if (!sensor_wall_cell_regression()) {
        fprintf(stderr, "FAIL: C03 wall sensor cell selection\n");
        return 1;
    }

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
    /* Compact F0510/F0511 lookup reaches a source sensor in map 1 instead
     * of substituting the random floor ordinal. */
    fs_dungeon_set_level(1);
    if (fs_dungeon_get_floor_ornament(2, 15) != 3) ++failures;
    free(bytes);
    if (failures) {
        fprintf(stderr, "FAIL: real DM1 DUNGEON.DAT state checks (%d)\n", failures);
        return 1;
    }
    puts("ok: real DM1 DUNGEON.DAT layout/state verified");
    return 0;
}
