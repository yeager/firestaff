#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int source_level;
    int source_x;
    int source_y;
    int destination_level;
    int destination_x;
    int destination_y;
    unsigned short thing;
} OriginalRoute;

static unsigned short read_u16(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

static int square_type(const CSB_V1_DungeonData *dungeon,
                       int level, int x, int y)
{
    int raw = csb_v1_dungeon_get_raw_square(dungeon, level, x, y);
    return raw < 0 ? -1 : ((raw >> 5) & 7);
}

static int square_contains_thing(const CSB_V1_DungeonData *dungeon,
                                 int level, int x, int y,
                                 unsigned short target)
{
    int thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
    int guard;

    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
         ++guard) {
        if (((unsigned short)thing & 0x3fffu) == (target & 0x3fffu)) return 1;
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
            dungeon, (unsigned short)thing);
    }
    return 0;
}

static int square_has_only_sensor_type(const CSB_V1_DungeonData *dungeon,
                                       int level, int x, int y,
                                       int expected_sensor_type,
                                       int *out_object_type,
                                       int *out_sensor_cell)
{
    int thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
    int guard;
    int found = 0;

    if (out_object_type) *out_object_type = -1;
    if (out_sensor_cell) *out_sensor_cell = -1;
    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
         ++guard) {
        const unsigned char *record;
        int type;
        int size;

        record = csb_v1_dungeon_get_thing_record(
            dungeon, (unsigned short)thing, &type, NULL, &size);
        if (!record) return 0;
        if (type == 3 && size >= 8) {
            unsigned short type_data = read_u16(record + 2);
            unsigned short flags = read_u16(record + 4);
            int sensor_type = type_data & 0x007f;

            /* A remote non-Revert sensor would publish F0272/F0268 if its
             * family admitted the object.  Keeping the chain sensor-only
             * makes each negative result unambiguous. */
            if (sensor_type != expected_sensor_type ||
                (flags & ((1u << 5) | (1u << 11))) != 0u) {
                return 0;
            }
            found = 1;
            if (out_object_type) *out_object_type = type_data >> 7;
            if (out_sensor_cell) *out_sensor_cell = ((unsigned short)thing >> 14) & 3;
        } else if (type < 4) {
            return 0;
        }
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
            dungeon, (unsigned short)thing);
    }
    return found;
}

static int square_is_sensor_free_corridor(const CSB_V1_DungeonData *dungeon,
                                          int level, int x, int y)
{
    int thing;
    int guard;

    if (square_type(dungeon, level, x, y) != 1) return 0;
    thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
         ++guard) {
        int type;

        if (!csb_v1_dungeon_get_thing_record(
                dungeon, (unsigned short)thing, &type, NULL, NULL) ||
            type == 3) {
            return 0;
        }
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
            dungeon, (unsigned short)thing);
    }
    return 1;
}

static int find_isolated_source_object(const CSB_V1_DungeonData *dungeon,
                                       int object_type,
                                       int required_cell,
                                       OriginalRoute *route)
{
    int level;

    for (level = 0; level < dungeon->level_count; ++level) {
        int x;
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int thing;
                int guard;

                if (!square_is_sensor_free_corridor(dungeon, level, x, y)) continue;
                thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
                for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
                     ++guard) {
                    const unsigned char *record;
                    int type;
                    int size;

                    record = csb_v1_dungeon_get_thing_record(
                        dungeon, (unsigned short)thing, &type, NULL, &size);
                    if (!record) break;
                    if (type > 4 && type < 14 && size >= 4 &&
                        (object_type < 0 ||
                         (read_u16(record + 2) & 0x007f) == object_type) &&
                        (required_cell < 0 ||
                         (((unsigned short)thing >> 14) & 3) == required_cell)) {
                        route->source_level = level;
                        route->source_x = x;
                        route->source_y = y;
                        route->thing = (unsigned short)thing;
                        return 1;
                    }
                    thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
                        dungeon, (unsigned short)thing);
                }
            }
        }
    }
    return 0;
}

static int find_route(const CSB_V1_DungeonData *dungeon,
                      int destination_square_type,
                      int sensor_type,
                      int require_object_match,
                      OriginalRoute *route)
{
    int level;

    memset(route, 0, sizeof(*route));
    for (level = 0; level < dungeon->level_count; ++level) {
        int x;
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int object_type;
                int sensor_cell;

                if (square_type(dungeon, level, x, y) != destination_square_type ||
                    !square_has_only_sensor_type(dungeon, level, x, y,
                                                  sensor_type, &object_type,
                                                  &sensor_cell) ||
                    !find_isolated_source_object(
                        dungeon,
                        require_object_match ? object_type : -1,
                        destination_square_type == 0 ? sensor_cell : -1,
                        route)) {
                    continue;
                }
                route->destination_level = level;
                route->destination_x = x;
                route->destination_y = y;
                return 1;
            }
        }
    }
    return 0;
}

static unsigned int original_corpus_fnv1a(const CSB_V1_DungeonData *dungeon)
{
    unsigned int hash = 2166136261u;
    int index;

    for (index = 0; dungeon && dungeon->raw_data && index < dungeon->raw_size;
         ++index) {
        hash ^= dungeon->raw_data[index];
        hash *= 16777619u;
    }
    return hash;
}

static int run_route(const char *path,
                     unsigned int expected_corpus_fnv1a,
                     int expected_corpus_size,
                     int destination_square_type,
                     int sensor_type,
                     int require_object_match,
                     int expect_event,
                     const char *label)
{
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    OriginalRoute route;
    int result = 0;

    if (csb_v1_dungeon_load_from_file(&dungeon, path) != 0 ||
        dungeon.square_bytes != 1) {
        fprintf(stderr, "FAIL: cannot load original PC34 Dungeon.dat: %s\n", path);
        return 1;
    }
    if (dungeon.raw_size != expected_corpus_size ||
        original_corpus_fnv1a(&dungeon) != expected_corpus_fnv1a) {
        fputs("FAIL: route did not load the requested original Dungeon.dat corpus\n",
              stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }
    if (!find_route(&dungeon, destination_square_type, sensor_type,
                    require_object_match, &route)) {
        printf("SKIP: no isolated original %s route\n", label);
        csb_v1_dungeon_free(&dungeon);
        return 0;
    }

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    if (!csb_v1_runtime_f0267_move_original_object(
            &profile, route.thing, route.source_level, route.source_x,
            route.source_y, route.destination_level, route.destination_x,
            route.destination_y)) {
        fprintf(stderr, "FAIL: original %s route was rejected\n", label);
        result = 1;
    } else if (square_contains_thing(&dungeon, route.source_level,
                                     route.source_x, route.source_y,
                                     route.thing) ||
               !square_contains_thing(&dungeon, route.destination_level,
                                      route.destination_x, route.destination_y,
                                      route.thing) ||
               (expect_event && profile.timeline_queue.eventCount == 0) ||
               (!expect_event && profile.timeline_queue.eventCount != 0)) {
        fprintf(stderr, "FAIL: original %s admission result is wrong\n", label);
        result = 1;
    } else {
        printf("ok: original %s route\n", label);
    }
    csb_v1_dungeon_free(&dungeon);
    return result;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_DUNGEON_DAT");
    CSB_V1_DungeonData corpus;
    unsigned int corpus_fnv1a;
    int corpus_size;
    int failed = 0;

    if (!path || !*path) {
        puts("SKIP: FIRESTAFF_CSB_DUNGEON_DAT is not configured");
        return 0;
    }
    if (csb_v1_dungeon_load_from_file(&corpus, path) != 0 ||
        corpus.square_bytes != 1) {
        fprintf(stderr, "FAIL: cannot load original PC34 Dungeon.dat: %s\n", path);
        return 1;
    }
    corpus_fnv1a = original_corpus_fnv1a(&corpus);
    corpus_size = corpus.raw_size;
    csb_v1_dungeon_free(&corpus);

    failed |= run_route(path, corpus_fnv1a, corpus_size,
                        0, 2, 1, 1, "C002 wall/object admission");
    failed |= run_route(path, corpus_fnv1a, corpus_size,
                        1, 2, 0, 0, "C002 floor/object rejection");
    failed |= run_route(path, corpus_fnv1a, corpus_size,
                        1, 7, 0, 0, "C007 floor/object rejection");
    return failed ? 1 : 0;
}
