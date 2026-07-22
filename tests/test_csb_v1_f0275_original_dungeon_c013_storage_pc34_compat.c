/* ReDMCSB MOVESENS.C F0275: real C013 wall storage through boot handoff. */
#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int level;
    int wall_x;
    int wall_y;
    int party_x;
    int party_y;
    int party_dir;
    int cell;
    uint16_t stored_thing;
    int target_x;
    int target_y;
} C013OriginalRoute;

static uint16_t read_u16(const uint8_t *bytes)
{
    return (uint16_t)(bytes[0] | ((uint16_t)bytes[1] << 8));
}

static uint32_t fnv1a_bytes(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t index;
    for (index = 0; index < size; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static int thing_object_type(const CSB_V1_DungeonData *dungeon, uint16_t thing)
{
    const uint8_t *record;
    int type;
    int size;
    record = csb_v1_dungeon_get_thing_record(dungeon, thing, &type, NULL, &size);
    if (!record || type <= 4 || type >= 14 || size < 4) return -1;
    return (int)(read_u16(record + 2) & 0x007fu);
}

static int square_chain_contains(const CSB_V1_DungeonData *dungeon,
                                 int level, int x, int y, uint16_t wanted)
{
    int thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
    int guard;
    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff; ++guard) {
        const uint8_t *record = csb_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)thing, NULL, NULL, NULL);
        if (!record) return 0;
        if ((uint16_t)thing == wanted) return 1;
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(dungeon, (uint16_t)thing);
    }
    return 0;
}

static int find_c013_storage_route(const CSB_V1_DungeonData *dungeon,
                                   C013OriginalRoute *route)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    int level;
    for (level = 0; level < dungeon->level_count; ++level) {
        int x;
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int raw_square = csb_v1_dungeon_get_raw_square(dungeon, level, x, y);
                int first_thing;
                int thing;
                int guard;
                if (raw_square < 0 || ((raw_square >> 5) & 7) != 0) continue;
                first_thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
                if (first_thing < 0 || first_thing == 0xfffe || first_thing == 0xffff) continue;
                thing = first_thing;
                for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff; ++guard) {
                    const uint8_t *record;
                    int type;
                    int size;
                    record = csb_v1_dungeon_get_thing_record(dungeon, (uint16_t)thing,
                                                               &type, NULL, &size);
                    if (!record) break;
                    if (type == 3 && size >= 8) {
                        uint16_t type_data = read_u16(record + 2);
                        uint16_t flags = read_u16(record + 4);
                        uint16_t target = read_u16(record + 6);
                        int target_x = (target >> 6) & 0x1f;
                        int target_y = (target >> 11) & 0x1f;
                        int target_raw = csb_v1_dungeon_get_raw_square(
                            dungeon, level, target_x, target_y);
                        int cell = thing & 3;
                        int desired_type = type_data >> 7;
                        int stored = first_thing;
                        int stored_guard;
                        int stored_count = 0;
                        uint16_t stored_thing = 0xffffu;
                        int dir;
                        if ((type_data & 0x007f) != DM1_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE ||
                            ((flags >> 3) & 0x03) != DM1_EFFECT_SET ||
                            (flags & ((1u << 2) | (1u << 5) | (1u << 11))) != 0u ||
                            target_raw < 0 || ((target_raw >> 5) & 7) != 6 ||
                            (target_raw & 0x04) != 0) goto next_thing;
                        for (stored_guard = 0; stored_guard < 128 && stored != 0xfffe &&
                             stored != 0xffff; ++stored_guard) {
                            if ((stored & 3) == cell &&
                                thing_object_type(dungeon, (uint16_t)stored) == desired_type) {
                                ++stored_count;
                                stored_thing = (uint16_t)stored;
                            }
                            stored = csb_v1_dungeon_f0159_get_next_thing_pc34(
                                dungeon, (uint16_t)stored);
                        }
                        if (stored_count != 1) goto next_thing;
                        for (dir = 0; dir < 4; ++dir) {
                            int party_x = x - dx[dir];
                            int party_y = y - dy[dir];
                            int party_raw = csb_v1_dungeon_get_raw_square(
                                dungeon, level, party_x, party_y);
                            if (party_raw < 0 || ((party_raw >> 5) & 7) != 1) continue;
                            route->level = level;
                            route->wall_x = x;
                            route->wall_y = y;
                            route->party_x = party_x;
                            route->party_y = party_y;
                            route->party_dir = dir;
                            route->cell = cell;
                            route->stored_thing = stored_thing;
                            route->target_x = target_x;
                            route->target_y = target_y;
                            return 1;
                        }
                    }
next_thing:
                    thing = csb_v1_dungeon_f0159_get_next_thing_pc34(dungeon, (uint16_t)thing);
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
    CSB_V1_BootProfile boot;
    C013OriginalRoute route;
    unsigned short hand_after = 0xffffu;
    uint32_t raw_before;
    int target_raw;
    if (!path || !*path) {
        puts("SKIP: FIRESTAFF_CSB_DUNGEON_DAT is not configured");
        return 0;
    }
    if (csb_v1_dungeon_load_from_file(&dungeon, path) != 0 || dungeon.square_bytes != 1) {
        fprintf(stderr, "FAIL: cannot load original PC34 Dungeon.dat: %s\n", path);
        return 1;
    }
    csb_v1_dungeon_set_current(&dungeon);
    /* The production bridge must refuse a stale global-level context before
     * it derives or mutates a front-wall chain. This uses the original corpus,
     * but deliberately mismatches only the live level owner. */
    if (dungeon.level_count > 1) {
        csb_v1_boot_profile_init(&boot);
        boot.runtime.dungeon_handle = &dungeon;
        boot.runtime.current_level = 0;
        csb_v1_dungeon_set_current_level(1);
        if (csb_v1_boot_runtime_trigger_front_wall_ornament_click_pc34(
                &boot, 0xffffu, &hand_after) != 0) {
            fputs("FAIL: C013 bridge accepted a stale global dungeon level\n", stderr);
            csb_v1_dungeon_unload();
            return 1;
        }
    }
    if (!find_c013_storage_route(&dungeon, &route)) {
        puts("SKIP: original Dungeon.dat has no isolated C013 storage route");
        csb_v1_dungeon_unload();
        return 0;
    }
    csb_v1_boot_profile_init(&boot);
    boot.runtime.chaos_magic.magic_initialized = 1;
    boot.runtime.dungeon_handle = &dungeon;
    boot.runtime.current_level = route.level;
    boot.runtime.party_x = route.party_x;
    boot.runtime.party_y = route.party_y;
    boot.runtime.party_dir = route.party_dir;
    boot.runtime.party_state_valid = 1;
    boot.runtime.party_state.LeaderHandThing = 0xffffu;
    boot.runtime.csbwin_gameblock2_summary_valid = 1;
    boot.runtime.csbwin_object_in_hand = 0xffffu;
    csb_v1_dungeon_set_current(&dungeon);
    csb_v1_dungeon_set_current_level(route.level);
    raw_before = fnv1a_bytes(dungeon.raw_data, dungeon.raw_size);
    if (csb_v1_boot_runtime_trigger_front_wall_ornament_click_pc34(&boot, 0xffffu,
                                                                     &hand_after) != 1) {
        fputs("FAIL: original C013 did not enter source-owned front-wall route\n", stderr);
        csb_v1_dungeon_unload();
        return 1;
    }
    if (hand_after != route.stored_thing ||
        boot.runtime.party_state.LeaderHandThing != route.stored_thing ||
        boot.runtime.csbwin_object_in_hand != route.stored_thing ||
        square_chain_contains(&dungeon, route.level, route.wall_x, route.wall_y,
                              route.stored_thing) ||
        raw_before == fnv1a_bytes(dungeon.raw_data, dungeon.raw_size) ||
        boot.runtime.timeline_queue.eventCount != 1) {
        fputs("FAIL: C013 pickup did not mutate its original source/hand state\n", stderr);
        csb_v1_dungeon_unload();
        return 1;
    }
    if (csb_v1_runtime_tick_v1(&boot.runtime) != 1) {
        fputs("FAIL: original C013 event did not reach F0261 consumer tick\n", stderr);
        csb_v1_dungeon_unload();
        return 1;
    }
    target_raw = csb_v1_dungeon_get_raw_square(&dungeon, route.level,
                                                route.target_x, route.target_y);
    if (boot.runtime.timeline_queue.eventCount != 0 || target_raw < 0 ||
        (target_raw & 0x04) == 0) {
        fputs("FAIL: original C013 did not apply the F0261 fakewall SET\n", stderr);
        csb_v1_dungeon_unload();
        return 1;
    }
    puts("ok: original C013 front-wall pickup reaches F0275/F0272/F0261");
    csb_v1_dungeon_unload();
    return 0;
}
