/* ReDMCSB CLIKVIEW.C F0377: real roomDOOR C05 button -> C10 at T+1. */
#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int level;
    int door_x;
    int door_y;
    int party_x;
    int party_y;
    int party_dir;
} DoorButtonRoute;

static uint16_t read_u16(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static int find_route(const CSB_V1_DungeonData *d, DoorButtonRoute *out)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int level;

    for (level = 0; level < d->level_count; ++level) {
        int x;
        for (x = 0; x < d->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < d->level_heights[level]; ++y) {
                const uint8_t *record;
                int raw = csb_v1_dungeon_get_raw_square(d, level, x, y);
                int thing;
                int type;
                int size;
                int dir;

                if (raw < 0 || ((raw >> 5) & 7) != 4 || (raw & 7) == 5)
                    continue;
                thing = csb_v1_dungeon_get_first_thing(d, level, x, y);
                if (thing < 0 || thing == 0xfffe || thing == 0xffff)
                    continue;
                record = csb_v1_dungeon_get_thing_record(
                    d, (uint16_t)thing, &type, NULL, &size);
                if (!record || type != 0 || size < 4 ||
                    (read_u16(record + 2) & 0x0040u) == 0u)
                    continue;
                for (dir = 0; dir < 4; ++dir) {
                    int px = x - dx[dir];
                    int py = y - dy[dir];
                    int party_raw = csb_v1_dungeon_get_raw_square(
                        d, level, px, py);
                    if (party_raw < 0 || ((party_raw >> 5) & 7) == 0)
                        continue;
                    out->level = level;
                    out->door_x = x;
                    out->door_y = y;
                    out->party_x = px;
                    out->party_y = py;
                    out->party_dir = dir;
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
    CSB_V1_BootProfile boot;
    DoorButtonRoute route;
    const struct DM1_Event_V1 *event;

    if (!path || !*path) {
        puts("SKIP: FIRESTAFF_CSB_DUNGEON_DAT is not configured");
        return 0;
    }
    if (csb_v1_dungeon_load_from_file(&dungeon, path) != 0 ||
        dungeon.square_bytes != 1) {
        fprintf(stderr, "FAIL: cannot load original CSB Dungeon.dat: %s\n", path);
        return 1;
    }
    if (!find_route(&dungeon, &route)) {
        puts("SKIP: original Dungeon.dat has no front-facing button door");
        csb_v1_dungeon_unload();
        return 0;
    }

    csb_v1_boot_profile_init(&boot);
    boot.runtime.dungeon_handle = &dungeon;
    boot.runtime.current_level = route.level;
    boot.runtime.party_x = route.party_x;
    boot.runtime.party_y = route.party_y;
    boot.runtime.party_dir = route.party_dir;
    boot.runtime.party_state_valid = 1;
    boot.runtime.party_state.ChampionCount = 1;
    boot.runtime.party_state.LeaderIndex = 0;
    boot.runtime.party_state.LeaderHandThing = 0xffffu;
    boot.runtime.champion_count = 1;
    boot.runtime.leader_index = 0;
    boot.runtime.game_time = 41u;
    csb_v1_dungeon_set_current(&dungeon);
    csb_v1_dungeon_set_current_level(route.level);

    /* The source bridge refuses the M11-side hand: F0377's door branch is
     * only taken for G0415_ui_LeaderEmptyHanded. */
    if (csb_v1_boot_runtime_trigger_front_door_button_click_pc34(
            &boot, 0x1234u) != 0 || boot.runtime.timeline_queue.eventCount != 0) {
        fputs("FAIL: occupied hand reached original door-button path\n", stderr);
        csb_v1_dungeon_unload();
        return 1;
    }
    if (csb_v1_boot_runtime_trigger_front_door_button_click_pc34(
            &boot, 0xffffu) != 1 || boot.runtime.timeline_queue.eventCount != 1) {
        fputs("FAIL: real button door did not queue C10\n", stderr);
        csb_v1_dungeon_unload();
        return 1;
    }
    event = &boot.runtime.timeline_queue.events[
        boot.runtime.timeline_queue.timeline[0]];
    if (event->type != DM1_EVENT_DOOR || event->b_mapX != route.door_x ||
        event->b_mapY != route.door_y || event->c_effect != DM1_EFFECT_TOGGLE ||
        DM1_MAP_TIME_MAP(event->map_time) != (uint32_t)route.level ||
        DM1_MAP_TIME_TIME(event->map_time) != 42u ||
        boot.runtime.audio_runtime.totalRequests != 1u ||
        boot.runtime.audio_runtime.pendingSoundIndex != CSB_V1_SOUND_SWITCH) {
        fputs("FAIL: C05 door path did not preserve switch/C10/T+1\n", stderr);
        csb_v1_dungeon_unload();
        return 1;
    }
    /* C10 is consumed on the next runtime tick and begins the source door
     * state sequence; the click itself never mutates the square directly. */
    if (csb_v1_runtime_tick_v1(&boot.runtime) != 1 ||
        boot.runtime.timeline_queue.eventCount == 0) {
        fputs("FAIL: queued C10 did not enter source door animation\n", stderr);
        csb_v1_dungeon_unload();
        return 1;
    }
    puts("ok: original CSB button door uses C05/F0377 switch and C10 at T+1");
    csb_v1_dungeon_unload();
    return 0;
}
