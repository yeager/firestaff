#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned short read_u16(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

static int contains_thing(const CSB_V1_DungeonData *dungeon,
                          int level, int map_x, int map_y,
                          unsigned short target)
{
    int thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    int guard;

    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
         ++guard) {
        if (((unsigned short)thing & 0x3fffu) == (target & 0x3fffu)) return 1;
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
            dungeon, (unsigned short)thing);
    }
    return 0;
}

static int square_has_matching_object(const CSB_V1_DungeonData *dungeon,
                                      int level, int map_x, int map_y,
                                      int object_type)
{
    int thing = csb_v1_dungeon_get_first_thing(dungeon, level, map_x, map_y);
    int guard;

    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
         ++guard) {
        const unsigned char *record;
        int type;
        int size;

        record = csb_v1_dungeon_get_thing_record(
            dungeon, (unsigned short)thing, &type, NULL, &size);
        if (!record) return 0;
        if (type > 4 && type < 14 && size >= 4 &&
            (read_u16(record + 2) & 0x007f) == object_type) {
            return 1;
        }
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
            dungeon, (unsigned short)thing);
    }
    return 0;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_DUNGEON_DAT");
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    int sensor_level = -1;
    int sensor_x = -1;
    int sensor_y = -1;
    int source_level = -1;
    int source_x = -1;
    int source_y = -1;
    unsigned short source_thing = 0xffffu;
    int wanted_object_type = -1;
    int level;

    if (!path || !*path) {
        puts("SKIP: FIRESTAFF_CSB_DUNGEON_DAT is not configured");
        return 0;
    }
    if (csb_v1_dungeon_load_from_file(&dungeon, path) != 0 ||
        dungeon.square_bytes != 1) {
        fprintf(stderr, "FAIL: cannot load original PC34 Dungeon.dat: %s\n", path);
        return 1;
    }

    /* Locate a real C004 object sensor on an ordinary floor square, then an
     * independently stored original object with the source-matching type.
     * No test dungeon, event, or replacement object is constructed. */
    for (level = 0; level < dungeon.level_count && sensor_level < 0; ++level) {
        int x;
        for (x = 0; x < dungeon.level_widths[level] && sensor_level < 0; ++x) {
            int y;
            for (y = 0; y < dungeon.level_heights[level]; ++y) {
                int raw_square = csb_v1_dungeon_get_raw_square(&dungeon, level, x, y);
                int thing;
                int guard;

                if (raw_square < 0 || ((raw_square >> 5) & 7) != 1) continue;
                thing = csb_v1_dungeon_get_first_thing(&dungeon, level, x, y);
                for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
                     ++guard) {
                    const unsigned char *record;
                    int type;
                    int size;
                    unsigned short type_data;
                    unsigned short flags;

                    record = csb_v1_dungeon_get_thing_record(
                        &dungeon, (unsigned short)thing, &type, NULL, &size);
                    if (!record) break;
                    if (type == 3 && size >= 8) {
                        type_data = read_u16(record + 2);
                        flags = read_u16(record + 4);
                        if ((type_data & 0x007f) == DM1_SENSOR_FLOOR_OBJECT &&
                            (flags & (1u << 11)) == 0) {
                            sensor_level = level;
                            sensor_x = x;
                            sensor_y = y;
                            wanted_object_type = (int)(type_data >> 7);
                            break;
                        }
                    }
                    thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
                        &dungeon, (unsigned short)thing);
                }
            }
        }
    }

    if (sensor_level >= 0) {
        for (level = 0; level < dungeon.level_count && source_level < 0; ++level) {
            int x;
            for (x = 0; x < dungeon.level_widths[level] && source_level < 0; ++x) {
                int y;
                for (y = 0; y < dungeon.level_heights[level] && source_level < 0; ++y) {
                    int thing;
                    int guard;

                    if (level == sensor_level && x == sensor_x && y == sensor_y) continue;
                    thing = csb_v1_dungeon_get_first_thing(&dungeon, level, x, y);
                    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
                         ++guard) {
                        const unsigned char *record;
                        int type;
                        int size;

                        record = csb_v1_dungeon_get_thing_record(
                            &dungeon, (unsigned short)thing, &type, NULL, &size);
                        if (!record) break;
                        if (type > 4 && type < 14 && size >= 4 &&
                            (read_u16(record + 2) & 0x007f) == wanted_object_type) {
                            source_level = level;
                            source_x = x;
                            source_y = y;
                            source_thing = (unsigned short)thing;
                            break;
                        }
                        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
                            &dungeon, (unsigned short)thing);
                    }
                }
            }
        }
    }

    if (sensor_level < 0 || source_level < 0 ||
        square_has_matching_object(&dungeon, sensor_level, sensor_x, sensor_y,
                                   wanted_object_type)) {
        puts("SKIP: original Dungeon.dat has no isolated C004/object route");
        csb_v1_dungeon_free(&dungeon);
        return 0;
    }

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    if (!csb_v1_runtime_f0267_move_original_object(
            &profile, source_thing, source_level, source_x, source_y,
            sensor_level, sensor_x, sensor_y)) {
        fputs("FAIL: original F0267 object route was rejected\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }
    if (contains_thing(&dungeon, source_level, source_x, source_y, source_thing) ||
        !contains_thing(&dungeon, sensor_level, sensor_x, sensor_y, source_thing) ||
        profile.timeline_queue.eventCount == 0) {
        fputs("FAIL: original C004 route did not mutate source/destination/event state\n",
              stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }

    puts("ok: original Dungeon.dat C004 object route mutates chain and publishes F0272/F0268");
    csb_v1_dungeon_free(&dungeon);
    return 0;
}
