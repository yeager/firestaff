/* ReDMCSB MOVESENS.C F0275 C012 -> F0167 -> F0272/F0268 corpus route. */
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int level;
    int x;
    int y;
    int cell;
    int target_x;
    int target_y;
} C012OriginalRoute;

static unsigned short read_u16(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

static uint32_t fnv1a_bytes(const unsigned char *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t index;

    for (index = 0; index < size; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static int find_c012_generator_route(const CSB_V1_DungeonData *dungeon,
                                     C012OriginalRoute *route)
{
    int level;

    for (level = 0; level < dungeon->level_count; ++level) {
        int x;
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int raw_square = csb_v1_dungeon_get_raw_square(
                    dungeon, level, x, y);
                int first_thing;
                int thing;
                int guard;

                if (raw_square < 0 || ((raw_square >> 5) & 7) != 0) {
                    continue;
                }
                first_thing = csb_v1_dungeon_get_first_thing(
                    dungeon, level, x, y);
                if (first_thing < 0 || first_thing == 0xfffe ||
                    first_thing == 0xffff) {
                    continue;
                }
                thing = first_thing;
                for (guard = 0;
                     guard < 128 && thing != 0xfffe && thing != 0xffff;
                     ++guard) {
                    const unsigned char *record;
                    int type;
                    int size;

                    record = csb_v1_dungeon_get_thing_record(
                        dungeon, (unsigned short)thing, &type, NULL, &size);
                    if (!record) break;
                    if (type == 3 && size >= 8) {
                        unsigned short type_data = read_u16(record + 2);
                        unsigned short flags = read_u16(record + 4);
                        unsigned short target = read_u16(record + 6);
                        int target_x = (target >> 6) & 0x1f;
                        int target_y = (target >> 11) & 0x1f;
                        int target_raw = csb_v1_dungeon_get_raw_square(
                            dungeon, level, target_x, target_y);
                        int cell = thing & 3;
                        int scan;
                        int scan_guard;
                        int same_cell_sensors = 0;

                        if ((type_data & 0x007f) !=
                                DM1_SENSOR_WALL_OBJECT_GENERATOR_ROTATE ||
                            ((flags >> 3) & 0x03) != DM1_EFFECT_SET ||
                            (flags & ((1u << 2) | (1u << 5) | (1u << 11))) != 0u ||
                            target_raw < 0 || ((target_raw >> 5) & 7) != 6 ||
                            (target_raw & 0x04) != 0) {
                            thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
                                dungeon, (unsigned short)thing);
                            continue;
                        }

                        /* F0275 admits C012 only when it is the last sensor
                         * in its source cell.  Reject ambiguous source cells. */
                        scan = first_thing;
                        for (scan_guard = 0;
                             scan_guard < 128 && scan != 0xfffe && scan != 0xffff;
                             ++scan_guard) {
                            int scan_type;
                            const unsigned char *scan_record =
                                csb_v1_dungeon_get_thing_record(
                                    dungeon, (unsigned short)scan,
                                    &scan_type, NULL, NULL);
                            if (!scan_record) break;
                            if (scan_type == 3 && (scan & 3) == cell) {
                                ++same_cell_sensors;
                            }
                            scan = csb_v1_dungeon_f0159_get_next_thing_pc34(
                                dungeon, (unsigned short)scan);
                        }
                        if (same_cell_sensors != 1) {
                            thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
                                dungeon, (unsigned short)thing);
                            continue;
                        }
                        route->level = level;
                        route->x = x;
                        route->y = y;
                        route->cell = cell;
                        route->target_x = target_x;
                        route->target_y = target_y;
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

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_DUNGEON_DAT");
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    C012OriginalRoute route;
    const unsigned char *generated_record;
    uint32_t raw_before;
    int generated_type;
    int generated_size;
    int target_raw;

    if (!path || !*path) {
        puts("SKIP: FIRESTAFF_CSB_DUNGEON_DAT is not configured");
        return 0;
    }
    if (csb_v1_dungeon_load_from_file(&dungeon, path) != 0 ||
        dungeon.square_bytes != 1) {
        fprintf(stderr, "FAIL: cannot load original PC34 Dungeon.dat: %s\n", path);
        return 1;
    }
    if (!find_c012_generator_route(&dungeon, &route)) {
        puts("SKIP: original Dungeon.dat has no isolated C012 generator route");
        csb_v1_dungeon_free(&dungeon);
        return 0;
    }

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.current_level = route.level;
    profile.party_state_valid = 1;
    profile.party_state.LeaderHandThing = 0xffffu;
    profile.csbwin_gameblock2_summary_valid = 1;
    profile.csbwin_object_in_hand = 0xffffu;
    csb_v1_dungeon_set_current_level(route.level);
    raw_before = fnv1a_bytes(dungeon.raw_data, dungeon.raw_size);

    if (csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
            &profile, route.x, route.y, route.cell) != 1) {
        fputs("FAIL: original C012 did not enter F0275 runtime hand route\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }
    generated_record = csb_v1_dungeon_get_thing_record(
        &dungeon, profile.party_state.LeaderHandThing,
        &generated_type, NULL, &generated_size);
    if (profile.party_state.LeaderHandThing == 0xffffu ||
        profile.party_state.LeaderHandThing == 0xfffeu ||
        profile.csbwin_object_in_hand != profile.party_state.LeaderHandThing ||
        !generated_record || generated_type < 5 || generated_type >= 14 ||
        generated_size < 2 || read_u16(generated_record) == 0xffffu ||
        raw_before == fnv1a_bytes(dungeon.raw_data, dungeon.raw_size) ||
        profile.timeline_queue.eventCount != 1) {
        fputs("FAIL: original C012 did not allocate source object and publish F0268\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }

    if (csb_v1_runtime_tick_v1(&profile) != 1) {
        fputs("FAIL: original C012 event did not reach F0261 consumer tick\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }
    target_raw = csb_v1_dungeon_get_raw_square(
        &dungeon, route.level, route.target_x, route.target_y);
    if (profile.timeline_queue.eventCount != 0 || target_raw < 0 ||
        (target_raw & 0x04) == 0) {
        fputs("FAIL: original C012 did not apply source F0261 fakewall SET\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }

    puts("ok: original C012 allocates through F0167 and reaches F0261");
    csb_v1_dungeon_free(&dungeon);
    return 0;
}
