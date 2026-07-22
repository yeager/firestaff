/* ReDMCSB MOVESENS.C F0267/F0276 C005 -> CLIKMENU.C F0364 corpus route. */
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int source_level;
    int source_x;
    int source_y;
    int destination_level;
    int destination_x;
    int destination_y;
    int direction;
    int target_level;
    int target_x;
    int target_y;
} C005OriginalRoute;

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

static int has_group(const CSB_V1_DungeonData *dungeon,
                     int level, int x, int y)
{
    int thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
    int guard;

    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
         ++guard) {
        int type;

        if (!csb_v1_dungeon_get_thing_record(
                dungeon, (unsigned short)thing, &type, NULL, NULL)) {
            return 1;
        }
        if (type == 4) return 1;
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
            dungeon, (unsigned short)thing);
    }
    return 0;
}

static int is_remote_c005_only(const CSB_V1_DungeonData *dungeon,
                               int level, int x, int y)
{
    int thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
    int guard;
    int found = 0;

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

            /* Require exactly one non-Revert, non-local C005: if F0276
             * admits it, its F0272/F0268 side effect is observable without
             * inventing a timer or accepting another sensor family. */
            if ((type_data & 0x007f) != DM1_SENSOR_FLOOR_PARTY_ON_STAIRS ||
                (flags & ((1u << 5) | (1u << 11))) != 0u || found) {
                return 0;
            }
            found = 1;
        } else if (type < 4) {
            return 0;
        }
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
            dungeon, (unsigned short)thing);
    }
    return found;
}

static int find_c005_stairs_route(const CSB_V1_DungeonData *dungeon,
                                  C005OriginalRoute *route)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    int level;

    for (level = 0; level < dungeon->level_count; ++level) {
        int x;
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int raw_stairs = csb_v1_dungeon_get_raw_square(dungeon, level, x, y);
                int target_level;
                int target_x;
                int target_y;
                int direction;

                if (square_type(dungeon, level, x, y) != 3 ||
                    !is_remote_c005_only(dungeon, level, x, y) ||
                    has_group(dungeon, level, x, y)) {
                    continue;
                }
                target_x = x;
                target_y = y;
                target_level = csb_v1_dungeon_f0154_get_location_after_level_change_pc34(
                    dungeon, level, (raw_stairs & 0x04) ? -1 : 1,
                    &target_x, &target_y);
                if (target_level < 0 ||
                    csb_v1_dungeon_f0155_get_stairs_exit_direction_pc34(
                        dungeon, target_level, target_x, target_y) < 0) {
                    continue;
                }
                for (direction = 0; direction < 4; ++direction) {
                    int source_x = x - dx[direction];
                    int source_y = y - dy[direction];

                    if (square_type(dungeon, level, source_x, source_y) != 1 ||
                        has_group(dungeon, level, source_x, source_y)) {
                        continue;
                    }
                    route->source_level = level;
                    route->source_x = source_x;
                    route->source_y = source_y;
                    route->destination_level = level;
                    route->destination_x = x;
                    route->destination_y = y;
                    route->direction = direction;
                    route->target_level = target_level;
                    route->target_x = target_x;
                    route->target_y = target_y;
                    return 1;
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
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;
    C005OriginalRoute route;

    if (!path || !*path) {
        puts("SKIP: FIRESTAFF_CSB_DUNGEON_DAT is not configured");
        return 0;
    }
    if (csb_v1_dungeon_load_from_file(&dungeon, path) != 0 ||
        dungeon.square_bytes != 1) {
        fprintf(stderr, "FAIL: cannot load original PC34 Dungeon.dat: %s\n", path);
        return 1;
    }
    if (!find_c005_stairs_route(&dungeon, &route)) {
        puts("SKIP: original Dungeon.dat has no isolated C005 stairs route");
        csb_v1_dungeon_free(&dungeon);
        return 0;
    }

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.current_level = route.source_level;
    profile.party_x = route.source_x;
    profile.party_y = route.source_y;
    profile.party_dir = route.direction;
    profile.champion_count = 1;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 1;
    profile.party_state.Champions[0].CurrentHealth = 100;
    profile.party_state.PartyMapX = route.source_x;
    profile.party_state.PartyMapY = route.source_y;
    profile.party_state.PartyDirection = route.direction;
    csb_v1_dungeon_set_current_level(route.source_level);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    if (!DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
            &queue, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0) ||
        csb_v1_runtime_process_input_queue(
            &profile, &queue, 0, 0, 0, &result) != 1) {
        fputs("FAIL: original neighboring MOVE_FORWARD did not enter F0267\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }

    if (!result.movement_step_applied ||
        !result.sensor_destination_add_checked ||
        result.sensor_trigger_count != 1 ||
        result.sensor_last_type != DM1_SENSOR_FLOOR_PARTY_ON_STAIRS ||
        result.sensor_event_count != 1 ||
        !result.stair_transition_applied ||
        profile.current_level != route.target_level ||
        profile.party_x != route.target_x || profile.party_y != route.target_y ||
        profile.timeline_queue.eventCount != 1) {
        fputs("FAIL: original C005 did not publish F0276 before F0364\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }

    puts("ok: original C005 stairs route publishes F0276 before F0364 transition");
    csb_v1_dungeon_free(&dungeon);
    return 0;
}
