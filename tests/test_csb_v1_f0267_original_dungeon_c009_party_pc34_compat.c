/* ReDMCSB MOVESENS.C F0267/F0276 C009 -> F0272/F0268 -> F0261 corpus route. */
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int source_level;
    int source_x;
    int source_y;
    int destination_x;
    int destination_y;
    int direction;
    int target_x;
    int target_y;
} C009OriginalRoute;

static const char *const kOriginalCsbDungeonMd5 =
    "6695d2acebce49f95db1d8f3a5c733de";

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

static int has_object_or_group(const CSB_V1_DungeonData *dungeon,
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
        if (type >= 4 && type < 14) return 1;
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
            dungeon, (unsigned short)thing);
    }
    return 0;
}

static int find_remote_c009_set_fakewall(const CSB_V1_DungeonData *dungeon,
                                         int level, int x, int y,
                                         int *target_x, int *target_y)
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
            unsigned short target = read_u16(record + 6);
            int sensor_type = type_data & 0x007f;
            int sensor_data = type_data >> 7;
            int sensor_effect = (flags >> 3) & 0x03;
            int remote_x = (target >> 6) & 0x1f;
            int remote_y = (target >> 11) & 0x1f;
            int raw_target = csb_v1_dungeon_get_raw_square(
                dungeon, level, remote_x, remote_y);

            /* Require exactly one original PC34 C009 whose existing F0261
             * fakewall consumer can be observed after the normal F0268 tick. */
            if (sensor_type == DM1_SENSOR_FLOOR_VERSION_CHECKER &&
                sensor_data <= 34 && sensor_effect == DM1_EFFECT_SET &&
                (flags & ((1u << 5) | (1u << 11))) == 0u &&
                raw_target >= 0 && ((raw_target >> 5) & 7) == 6 &&
                (raw_target & 0x04) == 0) {
                /* A corpus may contain unrelated C009 sensors before the
                 * usable route.  Keep scanning, but reject two candidates
                 * rather than selecting an arbitrary original path. */
                if (found) return 0;
                *target_x = remote_x;
                *target_y = remote_y;
                found = 1;
            }
        } else if (type < 4) {
            return 0;
        }
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
            dungeon, (unsigned short)thing);
    }
    return found;
}

static int find_c009_party_route(const CSB_V1_DungeonData *dungeon,
                                 C009OriginalRoute *route)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    int level;

    for (level = 0; level < dungeon->level_count; ++level) {
        int x;
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int target_x;
                int target_y;
                int direction;

                if (square_type(dungeon, level, x, y) != 1 ||
                    has_object_or_group(dungeon, level, x, y) ||
                    !find_remote_c009_set_fakewall(
                        dungeon, level, x, y, &target_x, &target_y)) {
                    continue;
                }
                for (direction = 0; direction < 4; ++direction) {
                    int source_x = x - dx[direction];
                    int source_y = y - dy[direction];

                    if (square_type(dungeon, level, source_x, source_y) != 1 ||
                        has_object_or_group(dungeon, level, source_x, source_y)) {
                        continue;
                    }
                    route->source_level = level;
                    route->source_x = source_x;
                    route->source_y = source_y;
                    route->destination_x = x;
                    route->destination_y = y;
                    route->direction = direction;
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
    C009OriginalRoute route;
    int raw_target;

    if (!path || !*path) {
        puts("SKIP: FIRESTAFF_CSB_DUNGEON_DAT is not configured");
        return 0;
    }
    if (!asset_file_matches_md5(path, kOriginalCsbDungeonMd5)) {
        fprintf(stderr,
                "FAIL: FIRESTAFF_CSB_DUNGEON_DAT is not hash-verified original CSB data: %s\n",
                path);
        return 1;
    }
    if (csb_v1_dungeon_load_from_file(&dungeon, path) != 0 ||
        dungeon.square_bytes != 1) {
        fprintf(stderr, "FAIL: cannot load original PC34 Dungeon.dat: %s\n", path);
        return 1;
    }
    if (!find_c009_party_route(&dungeon, &route)) {
        puts("SKIP: original Dungeon.dat has no isolated C009 fakewall route");
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
        result.sensor_last_type != DM1_SENSOR_FLOOR_VERSION_CHECKER ||
        result.sensor_last_data > 34 ||
        result.sensor_event_count != 1 ||
        result.sensor_last_event_type != DM1_EVENT_FAKEWALL ||
        profile.party_x != route.destination_x ||
        profile.party_y != route.destination_y ||
        profile.timeline_queue.eventCount != 1) {
        fputs("FAIL: original C009 did not publish F0276 through F0272/F0268\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }

    if (csb_v1_runtime_tick_v1(&profile) != 1) {
        fputs("FAIL: original C009 event did not reach F0261 consumer tick\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }
    raw_target = csb_v1_dungeon_get_raw_square(
        &dungeon, route.source_level, route.target_x, route.target_y);
    if (profile.timeline_queue.eventCount != 0 || raw_target < 0 ||
        (raw_target & 0x04) == 0) {
        fputs("FAIL: original C009 did not apply the F0261 fakewall SET\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }

    puts("ok: original C009 party route publishes F0276/F0272/F0268 then F0261");
    csb_v1_dungeon_free(&dungeon);
    return 0;
}
