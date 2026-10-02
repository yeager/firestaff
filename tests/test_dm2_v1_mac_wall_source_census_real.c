#include "m11_game_view.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_actuator_event_pc34_compat.h"
#include "dm2_v1_skproject_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const DM2_V1_DungeonData *dungeon;
    unsigned counts[0x80];
    unsigned first_map[0x80];
    unsigned first_x[0x80];
    unsigned first_y[0x80];
    unsigned first_w2[0x80];
    unsigned first_w4[0x80];
    unsigned first_w6[0x80];
} Census;

static void print_chain(const DM2_V1_DungeonData *dungeon, int level,
                        int x, int y)
{
    int thing;
    unsigned steps = 0;
    thing = dm2_v1_dungeon_get_first_thing(dungeon, level, x, y);
    printf("  chain map=%d x=%d y=%d:", level, x, y);
    while (thing >= 0 && steps++ < 32u) {
        int type = -1;
        int size = 0;
        const uint8_t *record = dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)thing, &type, NULL, &size);
        if (!record || size < 2) break;
        if (type == 3 && size >= 8)
            printf(" %04x/%02x(w2=%04x,w4=%04x,w6=%04x)", thing,
                   (unsigned)(dm2_v1_dungeon_read_record_u16(dungeon, record + 2) & 0x7fu),
                   dm2_v1_dungeon_read_record_u16(dungeon, record + 2),
                   dm2_v1_dungeon_read_record_u16(dungeon, record + 4),
                   dm2_v1_dungeon_read_record_u16(dungeon, record + 6));
        else
            printf(" %04x/db%d", thing, type);
        thing = dm2_v1_dungeon_get_next_thing(dungeon, (uint16_t)thing);
    }
    putchar('\n');
}

static int exercise_authentic_mac_db1_transition(
    M11_GameViewState *state, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!state || !dungeon || !dungeon->record_graph_complete)
        return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                int square = dm2_v1_dungeon_get_square_type(dungeon, map, x, y);
                int first;
                int type = -1;
                const uint8_t *record;
                int w2;
                int w4;
                int dest_map;
                int dest_x;
                int dest_y;
                if (raw < 0 || square != 5 || (raw & 0x08) == 0)
                    continue;
                first = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
                if (first < 0 || (((unsigned)first >> 10) & 0x0fu) != 1u)
                    continue;
                record = dm2_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)first, &type, NULL, NULL);
                if (!record || type != 1)
                    continue;
                w2 = dm2_v1_dungeon_read_record_u16(dungeon, record + 2);
                w4 = dm2_v1_dungeon_read_record_u16(dungeon, record + 4);
                dest_x = w2 & 0x1f;
                dest_y = (w2 >> 5) & 0x1f;
                dest_map = (w4 >> 8) & 0xff;
                if (dest_map < 0 || dest_map >= dungeon->level_count ||
                    dest_x >= dungeon->level_widths[dest_map] ||
                    dest_y >= dungeon->level_heights[dest_map] ||
                    (w2 & 0x6000) != 0x4000)
                    continue;
                for (int dir = 0; dir < 4; ++dir) {
                    int sx = x - dx[dir];
                    int sy = y - dy[dir];
                    DM2_V1_BootRuntimeReceipt receipt;
                    if (sx < 0 || sy < 0 ||
                        sx >= dungeon->level_widths[map] ||
                        sy >= dungeon->level_heights[map] ||
                        dm2_v1_dungeon_get_square_type(
                            dungeon, map, sx, sy) == 0)
                        continue;
                    dm2_v1_runtime_set_position(map, sx, sy, dir);
                    if (dm2_v1_runtime_move(dir) != 0 ||
                        !dm2_v1_boot_runtime_capture(
                            (DM2_V1_BootProfile *)state->dm2BootProfile,
                            &receipt))
                        continue;
                    if (receipt.current_level == dest_map &&
                        receipt.party_x == dest_x && receipt.party_y == dest_y) {
                        printf("  authentic Mac DB1 transition map %d,%d,%d -> %d,%d,%d\n",
                               map, x, y, dest_map, dest_x, dest_y);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

static void print_authentic_mac_map0_specials(
    const DM2_V1_DungeonData *dungeon)
{
    if (!dungeon || dungeon->level_count <= 0) return;
    printf("  Mac map0 offsets=%d,%d size=%dx%d specials:",
           dungeon->map_offset_x[0], dungeon->map_offset_y[0],
           dungeon->level_widths[0], dungeon->level_heights[0]);
    puts("\n  Mac map0 c_map rows (x increases left-to-right):");
    for (int y = 0; y < dungeon->level_heights[0]; ++y) {
        printf("    y=%02d", y);
        for (int x = 0; x < dungeon->level_widths[0]; ++x) {
            int raw = dm2_v1_dungeon_c_map_get_tile_value(
                dungeon, 0, x, y);
            printf(" %02x", raw < 0 ? 0xffu : (unsigned)raw & 0xffu);
        }
        putchar('\n');
    }
    printf("  Mac map0 specials:");
    for (int y = 0; y < dungeon->level_heights[0]; ++y) {
        for (int x = 0; x < dungeon->level_widths[0]; ++x) {
            int raw = dm2_v1_dungeon_get_tile_raw(dungeon, 0, x, y);
            int type = dm2_v1_dungeon_get_square_type(dungeon, 0, x, y);
            if (raw >= 0 && type >= 2) {
                printf(" (%d,%d:%02x/t%d)", x, y, raw & 0xff, type);
                print_chain(dungeon, 0, x, y);
            }
        }
    }
    putchar('\n');
}

static void print_authentic_source_square_census(
    const DM2_V1_DungeonData *dungeon)
{
    unsigned counts[8] = { 0 };
    unsigned pit_raw[256] = { 0 };
    if (!dungeon) return;
    for (int map = 0; map < dungeon->level_count; ++map) {
        unsigned map_counts[8] = { 0 };
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                int type = dm2_v1_dungeon_get_square_type(dungeon, map, x, y);
                if (raw < 0 || type < 0 || type >= 8) continue;
                ++counts[type];
                ++map_counts[type];
                if (type == 2) ++pit_raw[raw & 0xff];
                if (type == 3 || type == 5)
                    printf("  source square map=%d x=%d y=%d raw=%02x class=%d\n",
                           map, x, y, raw & 0xff, type);
            }
        }
        if (map_counts[2] || map_counts[3] || map_counts[5])
            printf("  source square totals map=%d pit=%u stairs=%u tele=%u\n",
                   map, map_counts[2], map_counts[3], map_counts[5]);
    }
    printf("  source square totals all maps pit=%u stairs=%u tele=%u\n",
           counts[2], counts[3], counts[5]);
    printf("  pit raw values:");
    for (int raw = 0; raw < 256; ++raw)
        if (pit_raw[raw]) printf(" %02x=%u", raw, pit_raw[raw]);
    putchar('\n');
}

static void print_authentic_stair_routes(const DM2_V1_DungeonData *dungeon)
{
    DM2_V1_SkprojectMapDescriptor maps[DM2_V1_MAX_LEVELS];
    uint8_t cursor[DM2_V1_MAX_LEVELS + 1];
    if (!dungeon || dungeon->level_count <= 0 ||
        dungeon->level_count > DM2_V1_MAX_LEVELS) return;
    memset(maps, 0, sizeof(maps));
    for (int map = 0; map < dungeon->level_count; ++map) {
        maps[map].map_id = (uint8_t)map;
        maps[map].level = dungeon->map_level_number[map];
        maps[map].world_x = dungeon->map_offset_x[map];
        maps[map].world_y = dungeon->map_offset_y[map];
        maps[map].width = (int16_t)dungeon->level_widths[map];
        maps[map].height = (int16_t)dungeon->level_heights[map];
    }
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                int16_t tx = (int16_t)x;
                int16_t ty = (int16_t)y;
                int delta;
                int target;
                DM2_V1_SkprojectLocateOtherLevelReceipt receipt;
                if (raw < 0 || dm2_v1_dungeon_get_square_type(dungeon, map, x, y) != 3)
                    continue;
                delta = (raw & 0x04) ? -1 : 1;
                {
                    int target_level = (int)dungeon->map_level_number[map] + delta;
                    uint16_t candidate_count = 0;
                    if (target_level < 0 || target_level >= 64) continue;
                    for (int candidate = 0; candidate < dungeon->level_count; ++candidate)
                        if ((int)dungeon->map_level_number[candidate] == target_level)
                            cursor[candidate_count++] = (uint8_t)candidate;
                    cursor[candidate_count] = 0xffu;
                    memset(&receipt, 0, sizeof(receipt));
                    target = dm2_v1_skproject_locate_other_level(
                        maps, (uint16_t)dungeon->level_count, (int16_t)map,
                        (int16_t)delta, &tx, &ty, cursor,
                        (uint16_t)(candidate_count + 1u), 0, NULL, &receipt);
                }
                printf("  stair route map=%d x=%d y=%d raw=%02x dir=%+d -> map=%d x=%d y=%d found=%d target_class=%d\n",
                       map, x, y, raw & 0xff, delta, target, tx, ty,
                       receipt.found,
                       target >= 0 && tx >= 0 && ty >= 0 &&
                       tx < dungeon->level_widths[target] &&
                       ty < dungeon->level_heights[target]
                           ? dm2_v1_dungeon_get_square_type(dungeon, target, tx, ty)
                           : -1);
            }
        }
    }
}

static void print_authentic_pit_routes(const DM2_V1_DungeonData *dungeon)
{
    DM2_V1_SkprojectMapDescriptor maps[DM2_V1_MAX_LEVELS];
    uint8_t cursor[DM2_V1_MAX_LEVELS + 1];
    if (!dungeon || dungeon->level_count <= 0 ||
        dungeon->level_count > DM2_V1_MAX_LEVELS) return;
    memset(maps, 0, sizeof(maps));
    for (int map = 0; map < dungeon->level_count; ++map) {
        maps[map].map_id = (uint8_t)map;
        maps[map].level = dungeon->map_level_number[map];
        maps[map].world_x = (int16_t)dungeon->map_offset_x[map];
        maps[map].world_y = (int16_t)dungeon->map_offset_y[map];
        maps[map].width = (int16_t)dungeon->level_widths[map];
        maps[map].height = (int16_t)dungeon->level_heights[map];
    }
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                int16_t tx = (int16_t)x;
                int16_t ty = (int16_t)y;
                int target;
                if (raw < 0 || dm2_v1_dungeon_get_square_type(dungeon, map, x, y) != 2 ||
                    (raw & 0x08) == 0 || (raw & 0x01) != 0) continue;
                {
                    int target_level = (int)dungeon->map_level_number[map] + 1;
                    uint16_t candidate_count = 0;
                    if (target_level < 0 || target_level >= 64) continue;
                    for (int candidate = 0; candidate < dungeon->level_count; ++candidate)
                        if ((int)dungeon->map_level_number[candidate] == target_level)
                            cursor[candidate_count++] = (uint8_t)candidate;
                    cursor[candidate_count] = 0xffu;
                    target = dm2_v1_skproject_locate_other_level(
                        maps, (uint16_t)dungeon->level_count, (int16_t)map, 1,
                        &tx, &ty, cursor, (uint16_t)(candidate_count + 1u),
                        0, NULL, NULL);
                }
                if (target >= 0 && target != map && tx >= 0 && ty >= 0 &&
                    tx < dungeon->level_widths[target] &&
                    ty < dungeon->level_heights[target]) {
                    int target_raw = dm2_v1_dungeon_get_tile_raw(dungeon, target, tx, ty);
                    int target_type = dm2_v1_dungeon_get_square_type(dungeon, target, tx, ty);
                    printf("  pit route map=%d x=%d y=%d raw=%02x -> map=%d x=%d y=%d raw=%02x class=%d\n",
                           map, x, y, raw & 0xff, target, tx, ty,
                           target_raw & 0xff, target_type);
                }
            }
        }
    }
}

static int exercise_authentic_mac_stairs(
    M11_GameViewState *state, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!state || !dungeon || !dungeon->record_graph_complete) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                if (raw < 0 || dm2_v1_dungeon_get_square_type(dungeon, map, x, y) != 3)
                    continue;
                for (int dir = 0; dir < 4; ++dir) {
                    int px = x - dx[dir];
                    int py = y - dy[dir];
                    DM2_V1_BootRuntimeReceipt receipt;
                    if (px < 0 || py < 0 || px >= dungeon->level_widths[map] ||
                        py >= dungeon->level_heights[map] ||
                        dm2_v1_dungeon_get_square_type(dungeon, map, px, py) == 0)
                        continue;
                    dm2_v1_runtime_set_position(map, px, py, dir);
                    dm2_v1_runtime_tick();
                    if (dm2_v1_runtime_move(dir) != 0) continue;
                    memset(&receipt, 0, sizeof(receipt));
                    if (!dm2_v1_boot_runtime_capture(
                            (DM2_V1_BootProfile *)state->dm2BootProfile,
                            &receipt))
                        continue;
                    if (receipt.current_level < 0 ||
                        receipt.current_level >= dungeon->level_count ||
                        receipt.current_level == map || receipt.party_x < 0 ||
                        receipt.party_y < 0 ||
                        receipt.party_x >= dungeon->level_widths[receipt.current_level] ||
                        receipt.party_y >= dungeon->level_heights[receipt.current_level])
                        continue;
                    printf("  authentic Mac stairs transition map %d,%d,%d -> %d,%d,%d\n",
                           map, px, py, receipt.current_level,
                           receipt.party_x, receipt.party_y);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int exercise_authentic_mac_pit(
    M11_GameViewState *state, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!state || !dungeon || !dungeon->record_graph_complete) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                if (raw < 0 || dm2_v1_dungeon_get_square_type(dungeon, map, x, y) != 2 ||
                    (raw & 0x08) == 0 || (raw & 0x01) != 0)
                    continue;
                for (int dir = 0; dir < 4; ++dir) {
                    int px = x - dx[dir];
                    int py = y - dy[dir];
                    DM2_V1_BootRuntimeReceipt receipt;
                    if (px < 0 || py < 0 || px >= dungeon->level_widths[map] ||
                        py >= dungeon->level_heights[map] ||
                        dm2_v1_dungeon_get_square_type(dungeon, map, px, py) == 0)
                        continue;
                    dm2_v1_runtime_set_position(map, px, py, dir);
                    dm2_v1_runtime_tick();
                    if (dm2_v1_runtime_move(dir) != 0) continue;
                    memset(&receipt, 0, sizeof(receipt));
                    if (!dm2_v1_boot_runtime_capture(
                            (DM2_V1_BootProfile *)state->dm2BootProfile,
                            &receipt) || receipt.current_level == map ||
                        receipt.current_level < 0 ||
                        receipt.current_level >= dungeon->level_count ||
                        receipt.party_x < 0 || receipt.party_y < 0 ||
                        receipt.party_x >= dungeon->level_widths[receipt.current_level] ||
                        receipt.party_y >= dungeon->level_heights[receipt.current_level])
                        continue;
                    printf("  authentic Mac pit transition map %d,%d,%d -> %d,%d,%d\n",
                           map, px, py, receipt.current_level,
                           receipt.party_x, receipt.party_y);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int exercise_authentic_mac_door_action(
    M11_GameViewState *state, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!state || !dungeon || !dungeon->record_graph_complete)
        return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                int type = -1;
                int first;
                if (raw < 0 || ((unsigned)raw >> 5) != 4u)
                    continue;
                first = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
                if (first < 0 || !dm2_v1_dungeon_get_thing_record(
                        dungeon, (uint16_t)first, &type, NULL, NULL) ||
                    type != 0)
                    continue;
                int party_x = -1;
                int party_y = -1;
                for (int dir = 0; dir < 4; ++dir) {
                    int px = x + dx[dir];
                    int py = y + dy[dir];
                    int neighbor = dm2_v1_dungeon_get_tile_raw(
                        dungeon, map, px, py);
                    if (neighbor < 0 ||
                        dm2_v1_dungeon_get_square_type(
                            dungeon, map, px, py) == 0)
                        continue;
                    party_x = px;
                    party_y = py;
                    break;
                }
                if (party_x < 0) continue;
                dm2_v1_runtime_set_position(map, party_x, party_y, 0);
                if (dm2_v1_runtime_door_action(map, x, y, 0, 0) != 0)
                    continue;
                int before = dm2_v1_runtime_get_door_state(map, x, y);
                DM2_V1_RuntimeActuatorTileReceipt actuator;
                for (int tick = 0; tick < 6; ++tick)
                    dm2_v1_runtime_tick();
                memset(&actuator, 0, sizeof(actuator));
                dm2_v1_runtime_actuator_tile_receipt(&actuator);
                DM2_V1_RuntimeDoorStepReceipt step;
                memset(&step, 0, sizeof(step));
                dm2_v1_runtime_door_step_receipt(&step);
                if (actuator.door <= 0 || step.mutations <= 0 ||
                    dm2_v1_runtime_get_door_state(map, x, y) != 0)
                    continue;
                printf("  authentic Mac door action map %d,%d,%d state %d->%d\n",
                       map, x, y, before,
                       dm2_v1_runtime_get_door_state(map, x, y));
                return 1;
            }
        }
    }
    return 0;
}

static int exercise_authentic_mac_mirror_not_wall_button(
    M11_GameViewState *state, const DM2_V1_DungeonData *dungeon)
{
    uint8_t frame[320u * 200u];
    DM2_V1_RuntimeMacWallButtonReceipt action;
    const uint8_t *mirror;
    int type = -1;
    int thing;

    /* The retail D1C wall at (3,2) owns a Mac BE DB3 subtype-0x7e
     * champion mirror. Its raw bytes must never be interpreted as a
     * little-endian wall switch. The adjacent source floor is (3,3). */
    thing = dm2_v1_dungeon_get_first_thing(dungeon, 0, 3, 2);
    mirror = thing >= 0 ? dm2_v1_dungeon_get_thing_record(
        dungeon, (uint16_t)thing, &type, NULL, NULL) : NULL;
    if (!state || !mirror || type != 3 ||
        (dm2_v1_dungeon_read_record_u16(dungeon, mirror + 2) & 0x7fu) != 0x7eu ||
        dm2_v1_dungeon_get_square_type(dungeon, 0, 3, 3) != 1)
        return 0;
    dm2_v1_runtime_set_position(0, 3, 3, 0);
    memset(frame, 0, sizeof(frame));
    M11_GameView_Draw(state, frame, 320, 200);
    memset(&action, 0, sizeof(action));
    return !dm2_v1_runtime_activate_mac_wall_button(1, &action) &&
           !action.accepted;
}

static int authentic_mac_false_switch_word_is_source_type(
    const DM2_V1_DungeonData *dungeon)
{
    DM2_V1_G1RuntimeMapActuatorReceipt source;
    int found = 0;

    /* The BE DB3 word 0x1888 on map-2 wall (7,8) is an original subtype
     * 0x08 mechanism. A PC-order read yields false subtype 0x18, which
     * the Mac wall-control fallback would incorrectly admit as a switch. */
    memset(&source, 0, sizeof(source));
    if (!dm2_v1_dungeon_collect_file_header_runtime_map_actuators(
            dungeon, 2, &source) || !source.committed)
        return 0;
    for (int i = 0; i < source.actuator_root_count; ++i) {
        const DM2_V1_G1DirectActuatorRoot *record = &source.actuators[i];
        if (record->x != 7 || record->y != 8 ||
            record->object_id != 0x8c72u)
            continue;
        if (record->attributes != 0x1888u ||
            record->actuator_type != 0x08u ||
            dm2_v1_dungeon_get_square_type(dungeon, 2, 7, 8) != 0)
            return 0;
        ++found;
    }
    return found == 1;
}

static int census_thing(void *user, uint16_t thing, int type, int index,
                        const uint8_t *record, int record_size,
                        int level, int x, int y)
{
    Census *c = (Census *)user;
    unsigned cls;
    (void)thing;
    (void)index;
    if (!c || type != 3 || !record || record_size < 8)
        return 0;
    cls = (unsigned)(dm2_v1_dungeon_read_record_u16(c->dungeon,
                                                     record + 2) & 0x7fu);
    if (cls >= 0x80u)
        return 0;
    ++c->counts[cls];
    if (cls == 0x05u || cls == 0x17u || cls == 0x18u || cls == 0x1au ||
        cls == 0x27u || cls == 0x46u)
        printf("  actuator type=%02x object=%04x map=%d x=%d y=%d w2=%04x w4=%04x w6=%04x\n",
               cls, thing, level, x, y,
               dm2_v1_dungeon_read_record_u16(c->dungeon, record + 2),
               dm2_v1_dungeon_read_record_u16(c->dungeon, record + 4),
               dm2_v1_dungeon_read_record_u16(c->dungeon, record + 6));
    if (c->counts[cls] == 1u) {
        c->first_map[cls] = (unsigned)level;
        c->first_x[cls] = (unsigned)x;
        c->first_y[cls] = (unsigned)y;
        c->first_w2[cls] = dm2_v1_dungeon_read_record_u16(c->dungeon, record + 2);
        c->first_w4[cls] = dm2_v1_dungeon_read_record_u16(c->dungeon, record + 4);
        c->first_w6[cls] = dm2_v1_dungeon_read_record_u16(c->dungeon, record + 6);
    }
    return 0;
}

static int run_one(const char *zip, const char *source_id)
{
    M11_GameViewState state;
    M11_GameLaunchSpec spec;
    DM2_V1_BootProfile *profile;
    DM2_V1_DungeonData *dungeon;
    Census census;
    unsigned level;

    memset(&state, 0, sizeof(state));
    memset(&spec, 0, sizeof(spec));
    memset(&census, 0, sizeof(census));
    spec.title = "Dungeon Master II Macintosh";
    spec.gameId = "dm2";
    spec.dataDir = zip;
    spec.sourceId = source_id;
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    spec.launcherOptionsBound = 1;
    M11_GameView_Init(&state);
    if (!M11_GameView_Start(&state, &spec)) {
        fprintf(stderr, "Mac census launch failed: %s\n", source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    /* Advance the original QuickTime title through M11's bounded probe
     * cadence.  This changes no source state; it prevents the census from
     * being coupled to host wall-clock scheduling. */
    M11_GameView_SetBootProbeMode(&state, 1);
    {
        unsigned char framebuffer[320u * 200u];
        memset(framebuffer, 0, sizeof(framebuffer));
        while (state.dm2MacMovieActive)
            M11_GameView_Draw(&state, framebuffer, 320, 200);
    }
    /* Retail Mac New Game is a two-stage source path.  The menu command
     * opens GAME_LOAD's preselection mirror; the following source-owned
     * viewport click commits the selected candidate and only then closes the
     * startup menu.  Do not turn the first command into a synthetic direct
     * runtime start. */
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        !state.dm2State.startup_menu_active ||
        state.dm2State.level_loaded) {
        fprintf(stderr, "Mac census New Game preselection failed: %s\n", source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (M11_GameView_HandlePointerButton(
            &state, 100, 60, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        state.dm2State.startup_menu_active ||
        !state.dm2State.level_loaded) {
        fprintf(stderr, "Mac census New Game confirmation failed: %s\n", source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        unsigned char inventory_frame[320u * 200u];
        if (M11_GameView_HandleInput(
                &state, M12_MENU_INPUT_INVENTORY_TOGGLE) !=
                M11_GAME_INPUT_REDRAW || !state.inventoryPanelActive) {
            fprintf(stderr, "Mac authenticated CHARSHEET inventory did not open: %s\n",
                    source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (M11_GameView_HandlePointerButton(
                &state, 0, 0, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                M11_GAME_INPUT_IGNORED || !state.inventoryPanelActive) {
            fprintf(stderr,
                    "Mac CHARSHEET pointer leaked into gameplay route: %s\n",
                    source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        memset(inventory_frame, 0, sizeof(inventory_frame));
        M11_GameView_Draw(&state, inventory_frame, 320, 200);
        {
            DM2_V1_BootExpandedRectReceipt slot_rect;
            DM2_V1_BootProfile *mac_profile =
                (DM2_V1_BootProfile *)state.dm2BootProfile;
            uint32_t slot_before =
                dm2_v1_runtime_get_champion_inventory_object(0u, 4u);
            uint32_t hand_before = dm2_v1_runtime_get_leader_hand_object();
            /* Retail view-8 object 55 emits event 0x20 through Rect 0x81ff.
             * CODE(8)+0x1ae4 passes event-20=12 to CODE(10)+0x1e3c,
             * which removes eight: this is champion inventory slot 4. */
            if (!mac_profile ||
                !dm2_v1_boot_query_expanded_rect_receipt(
                    mac_profile, 0x01ffu, &slot_rect) ||
                !slot_rect.valid || slot_rect.rect.w != 16 ||
                slot_rect.rect.h != 16 || slot_before != 0u ||
                hand_before != 0xffffu ||
                M11_GameView_HandlePointerButton(
                    &state, slot_rect.rect.x + slot_rect.rect.w / 2,
                    slot_rect.rect.y + slot_rect.rect.h / 2,
                    DM1_V1_MOUSE_MASK_LEFT_PC34) != M11_GAME_INPUT_REDRAW ||
                state.inventorySelectedSlot != 4 ||
                dm2_v1_runtime_get_champion_inventory_object(0u, 4u) !=
                    slot_before ||
                dm2_v1_runtime_get_leader_hand_object() != hand_before) {
                fprintf(stderr,
                        "Mac CHARSHEET event 0x20 did not select slot 4: %s\n",
                        source_id);
                M11_GameView_Shutdown(&state);
                return 1;
            }
        }
        if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK) !=
                M11_GAME_INPUT_REDRAW || state.inventoryPanelActive) {
            fprintf(stderr, "Mac CHARSHEET inventory did not close: %s\n",
                    source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        puts("  authenticated Mac CHARSHEET inventory frame accepted");
    }
    if (M11_GameView_HandleInput(
            &state, M12_MENU_INPUT_CHAMPION_1_INVENTORY) !=
        M11_GAME_INPUT_REDRAW || !state.inventoryPanelActive ||
        state.world.party.activeChampionIndex != 0) {
        fprintf(stderr, "Mac F1 champion inventory owner did not open: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        DM2_V1_BootExpandedRectReceipt slot_rect;
        DM2_V1_BootProfile *mac_profile =
            (DM2_V1_BootProfile *)state.dm2BootProfile;
        uint32_t slot_before =
            dm2_v1_runtime_get_champion_inventory_object(0u, 4u);
        uint32_t hand_before = dm2_v1_runtime_get_leader_hand_object();
        if (dm2_v1_runtime_get_inventory_eye_champion_index() != 0 ||
            hand_before != 0xffffu || slot_before != 0u ||
            !dm2_v1_boot_query_expanded_rect_receipt(
                mac_profile, 0x01ffu, &slot_rect) ||
            M11_GameView_HandlePointerButton(
                &state, slot_rect.rect.x + slot_rect.rect.w / 2,
                slot_rect.rect.y + slot_rect.rect.h / 2,
                DM1_V1_MOUSE_MASK_LEFT_PC34) != M11_GAME_INPUT_REDRAW ||
            state.inventorySelectedSlot != 4 ||
            dm2_v1_runtime_get_champion_inventory_object(0u, 4u) !=
                slot_before ||
            dm2_v1_runtime_get_leader_hand_object() != 0u) {
            fprintf(stderr,
                    "Mac F1 CHARSHEET source owner/slot 4 pointer failed: %s\n",
                    source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW || !state.inventoryPanelActive) {
        fprintf(stderr, "Mac authenticated keyboard item transaction unavailable: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    puts("  authenticated Mac keyboard item transaction accepted");
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK) !=
            M11_GAME_INPUT_REDRAW || state.inventoryPanelActive ||
        dm2_v1_runtime_get_inventory_eye_champion_index() != -1) {
        fprintf(stderr, "Mac F1 champion inventory owner did not close: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    puts("  authenticated Mac F1 champion inventory command accepted");
    profile = (DM2_V1_BootProfile *)state.dm2BootProfile;
    dungeon = profile ? (DM2_V1_DungeonData *)profile->dungeon_data : NULL;
    if (!dungeon || !dungeon->record_graph_complete) {
        fprintf(stderr, "Mac census has no complete dungeon graph: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        const uint8_t *pixels = NULL;
        const uint8_t *record;
        int record_type = -1;
        int width = 0, height = 0, stride = 0;
        int item = dm2_v1_dungeon_get_first_thing(dungeon, 9, 1, 6);
        int image = dm2_v1_viewport_item_graphic_index(0x15, 0x2c, 0);
        record = dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)item, &record_type, NULL, NULL);
        /* SKProject skcore.cpp::DRAW_ITEM selects Misc::ItemType from the
         * authenticated DB10 record, then category 0x15/Image field 0. */
        if (item != 0x2831 || !record || record_type != 10 ||
            (dm2_v1_dungeon_read_record_u16(dungeon, record + 2) & 0x7fu) !=
                0x2cu ||
            dm2_v1_boot_viewport_asset_fetch(
                profile, image, &pixels, &width, &height, &stride) != 0 ||
            !pixels || width != 34 || height != 13 || stride < width) {
            fprintf(stderr, "Mac retail DB10 source image address failed: %s\n",
                    source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    census.dungeon = dungeon;
    print_authentic_mac_map0_specials(dungeon);
    print_authentic_source_square_census(dungeon);
    print_authentic_stair_routes(dungeon);
    print_authentic_pit_routes(dungeon);
    for (level = 0; level < (unsigned)dungeon->level_count; ++level) {
        int y;
        for (y = 0; y < dungeon->level_heights[level]; ++y) {
            int x;
            for (x = 0; x < dungeon->level_widths[level]; ++x) {
                (void)dm2_v1_dungeon_walk_square_things(
                    dungeon, (int)level, x, y, 256, census_thing, &census);
            }
        }
    }
    {
        const unsigned classes[] = { 0x05u, 0x17u, 0x18u, 0x1au, 0x27u };
        size_t i;
        for (i = 0; i < sizeof(classes) / sizeof(classes[0]); ++i) {
            unsigned cls = classes[i];
            if (census.counts[cls]) {
                int tx = (int)((census.first_w6[cls] >> 6) & 0x1fu);
                int ty = (int)((census.first_w6[cls] >> 11) & 0x1fu);
                int first = dm2_v1_dungeon_get_first_thing(
                    dungeon, (int)census.first_map[cls], tx, ty);
                int type = -1;
                (void)dm2_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)first, &type, NULL, NULL);
                printf("  type=%02x target=%u,%d,%d tile=%d first_type=%d\n",
                       cls, census.first_map[cls], tx, ty,
                       dm2_v1_dungeon_get_square_type(
                           dungeon, (int)census.first_map[cls], tx, ty), type);
                print_chain(dungeon, (int)census.first_map[cls],
                            (int)census.first_x[cls],
                            (int)census.first_y[cls]);
            }
        }
    }
    if (!exercise_authentic_mac_mirror_not_wall_button(&state, dungeon)) {
        fprintf(stderr, "Mac retail mirror published as wall button: %s\n", source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!authentic_mac_false_switch_word_is_source_type(dungeon)) {
        fprintf(stderr, "Mac retail DB3 source word decoded as false switch: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!exercise_authentic_mac_db1_transition(&state, dungeon)) {
        fprintf(stderr, "Mac authentic DB1 transition fixture did not commit: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!exercise_authentic_mac_stairs(&state, dungeon)) {
        fprintf(stderr, "Mac authentic stairs transition did not commit: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!exercise_authentic_mac_pit(&state, dungeon)) {
        fprintf(stderr, "Mac authentic pit transition did not commit: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!exercise_authentic_mac_door_action(&state, dungeon)) {
        fprintf(stderr, "Mac authentic DB0 door action did not dispatch: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    printf("%s: 0x17=%u", source_id, census.counts[0x17]);
    if (census.counts[0x17])
        printf(" @%u,%u,%u w2=%04x w4=%04x w6=%04x", census.first_map[0x17],
               census.first_x[0x17], census.first_y[0x17], census.first_w2[0x17],
               census.first_w4[0x17], census.first_w6[0x17]);
    printf("; 0x18=%u", census.counts[0x18]);
    if (census.counts[0x18])
        printf(" @%u,%u,%u w2=%04x w4=%04x w6=%04x", census.first_map[0x18],
               census.first_x[0x18], census.first_y[0x18], census.first_w2[0x18],
               census.first_w4[0x18], census.first_w6[0x18]);
    printf("; 0x1a=%u", census.counts[0x1a]);
    if (census.counts[0x1a])
        printf(" @%u,%u,%u w2=%04x w4=%04x w6=%04x", census.first_map[0x1a],
               census.first_x[0x1a], census.first_y[0x1a], census.first_w2[0x1a],
               census.first_w4[0x1a], census.first_w6[0x1a]);
    printf("; 0x46=%u", census.counts[0x46]);
    if (census.counts[0x46])
        printf(" @%u,%u,%u w2=%04x w4=%04x w6=%04x", census.first_map[0x46],
               census.first_x[0x46], census.first_y[0x46], census.first_w2[0x46],
               census.first_w4[0x46], census.first_w6[0x46]);
    putchar('\n');
    {
        uint8_t frame[320u * 200u];
        DM2_V1_RuntimeItemRenderReceipt weapon_render;
        const uint8_t *weapon;
        int record_type = -1;
        /* Retail Mac File_header has a direct DB5 weapon root on the
         * corridor square immediately ahead of this diagnostic pose. */
        weapon = dm2_v1_dungeon_get_thing_record(
            dungeon, 0xd407u, &record_type, NULL, NULL);
        if (dm2_v1_dungeon_get_square_type(dungeon, 11, 11, 3) != 1 ||
            dm2_v1_dungeon_get_square_type(dungeon, 11, 10, 3) != 1 ||
            dm2_v1_dungeon_get_first_thing(dungeon, 11, 10, 3) != 0xd407 ||
            !weapon || record_type != 5 ||
            dm2_v1_dungeon_read_record_u16(dungeon, weapon + 2) != 0x3c85u) {
            fprintf(stderr, "Mac retail DB5 source pose invalid: %s\n", source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        dm2_v1_runtime_set_position(11, 11, 3, 3);
        memset(frame, 0, sizeof(frame));
        M11_GameView_Draw(&state, frame, 320, 200);
        memset(&weapon_render, 0, sizeof(weapon_render));
        if (dm2_v1_runtime_last_asset_item_count() < 1 ||
            !dm2_v1_runtime_last_item_render_receipt(&weapon_render) ||
            !weapon_render.valid || !weapon_render.asset_blit_ready ||
            weapon_render.object_id != 0xd407u ||
            weapon_render.item_category != 0x10 ||
            weapon_render.item_type != 0x05) {
            fprintf(stderr,
                    "Mac retail DB5 viewport missing: %s last=%04x cat=%x type=%x blit=%d items=%d\n",
                    source_id, weapon_render.object_id,
                    weapon_render.item_category, weapon_render.item_type,
                    weapon_render.asset_blit_ready,
                    dm2_v1_runtime_last_asset_item_count());
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    {
        static const struct {
            int map, item_x, item_y, pose_x, pose_y, dir;
            uint16_t object_id;
            uint16_t w2, w4;
            uint8_t record_type, category, item_type;
        } source_items[] = {
            { 17, 3, 8, 3, 7, 2, 0xa037u, 0x93ffu, 0u, 8u, 0x13u, 0x13u },
            { 14, 6, 13, 6, 12, 2, 0x240eu, 0x1476u, 0x6000u, 9u, 0x14u, 3u }
        };
        for (size_t item = 0; item < sizeof(source_items) / sizeof(source_items[0]); ++item) {
            uint8_t frame[320u * 200u];
            DM2_V1_RuntimeItemRenderReceipt render;
            const uint8_t *record;
            int record_type = -1;
            const int map = source_items[item].map;
            const int x = source_items[item].item_x;
            const int y = source_items[item].item_y;
            record = dm2_v1_dungeon_get_thing_record(
                dungeon, source_items[item].object_id, &record_type, NULL, NULL);
            if (!record || record_type != source_items[item].record_type ||
                dm2_v1_dungeon_read_record_u16(dungeon, record + 2) !=
                    source_items[item].w2 ||
                (record_type == 9 &&
                 dm2_v1_dungeon_read_record_u16(dungeon, record + 4) !=
                    source_items[item].w4) ||
                dm2_v1_dungeon_get_square_type(dungeon, map, x, y) != 1 ||
                dm2_v1_dungeon_get_square_type(
                    dungeon, map, source_items[item].pose_x,
                    source_items[item].pose_y) != 1) {
                fprintf(stderr, "Mac retail DB8-9 source pose invalid: %s db=%d\n",
                        source_id, record_type);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            dm2_v1_runtime_set_position(map, source_items[item].pose_x,
                                        source_items[item].pose_y,
                                        source_items[item].dir);
            memset(frame, 0, sizeof(frame));
            M11_GameView_Draw(&state, frame, 320, 200);
            memset(&render, 0, sizeof(render));
            if (dm2_v1_runtime_last_asset_item_count() != 1 ||
                !dm2_v1_runtime_last_item_render_receipt(&render) ||
                !render.valid || !render.asset_blit_ready ||
                render.object_id != source_items[item].object_id ||
                render.item_category != source_items[item].category ||
                render.item_type != source_items[item].item_type ||
                render.frame_index != 0) {
                fprintf(stderr,
                        "Mac retail DB%d bitmap missing: %s last=%04x cat=%x type=%x field=%x count=%d\n",
                        record_type, source_id, render.object_id,
                        render.item_category, render.item_type,
                        render.frame_index,
                        dm2_v1_runtime_last_asset_item_count());
                M11_GameView_Shutdown(&state);
                return 1;
            }
        }
    }
    {
        uint8_t frame[320u * 200u];
        DM2_V1_RuntimeItemRenderReceipt item_render;
        /* The authentic retail DB6 record 0x5880 is on outdoor floor map
         * 15 (16,6). The previous corridor fixture used a wall square
         * exposed as floor by the old map-byte offset. */
        {
            int db6_type = -1;
            const uint8_t *db6 = dm2_v1_dungeon_get_thing_record(
                dungeon, 0x5880u, &db6_type, NULL, NULL);
            if (dm2_v1_dungeon_get_square_type(dungeon, 15, 16, 5) != 1 ||
                dm2_v1_dungeon_get_square_type(dungeon, 15, 16, 6) != 1 ||
                !db6 || db6_type != 6 ||
                dm2_v1_dungeon_read_record_u16(dungeon, db6 + 2) != 0x0099u) {
                fprintf(stderr, "Mac retail DB6 floor source invalid: %s\n", source_id);
                M11_GameView_Shutdown(&state);
                return 1;
            }
        }
        dm2_v1_runtime_set_position(15, 16, 5, 2);
        memset(frame, 0, sizeof(frame));
        M11_GameView_Draw(&state, frame, 320, 200);
        memset(&item_render, 0, sizeof(item_render));
        if (!dm2_v1_runtime_last_item_render_receipt(&item_render) ||
            !item_render.valid || !item_render.asset_blit_ready ||
            item_render.object_id != 0x5880u ||
            item_render.item_category != 0x11) {
            fprintf(stderr,
                    "Mac retail DB6 bitmap missing: %s last=%04x cat=%x type=%x blit=%d\n",
                    source_id, item_render.object_id,
                    item_render.item_category, item_render.item_type,
                    item_render.asset_blit_ready);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    {
        uint8_t frame[320u * 200u];
        DM2_V1_RuntimeItemRenderReceipt item_render;
        /* Diagnostic pose: the retail DB10 square is one step in front of
         * this source floor square. This does not assert a New Game route. */
        if (dm2_v1_dungeon_get_square_type(dungeon, 9, 1, 7) != 1 ||
            dm2_v1_dungeon_get_first_thing(dungeon, 9, 1, 6) != 0x2831) {
            fprintf(stderr, "Mac retail DB10 source pose invalid: %s\n", source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        dm2_v1_runtime_set_position(9, 1, 7, 0);
        memset(frame, 0, sizeof(frame));
        M11_GameView_Draw(&state, frame, 320, 200);
        memset(&item_render, 0, sizeof(item_render));
        if (!dm2_v1_runtime_last_item_render_receipt(&item_render) ||
            !item_render.valid || !item_render.asset_blit_ready ||
            item_render.item_category != 0x15 ||
            item_render.item_type != 0x2c ||
            item_render.asset_src_w != 34 ||
            item_render.asset_src_h != 13 ||
            dm2_v1_runtime_last_asset_item_count() < 1) {
            fprintf(stderr, "Mac retail DB10 static object was not drawn: %s "
                    "valid=%d blit=%d cat=%x type=%x size=%dx%d count=%d\n",
                    source_id, item_render.valid, item_render.asset_blit_ready,
                    item_render.item_category, item_render.item_type,
                    item_render.asset_src_w, item_render.asset_src_h,
                    dm2_v1_runtime_last_asset_item_count());
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    {
        uint8_t frame[320u * 200u];
        DM2_V1_RuntimeItemRenderReceipt linked_render;
        if (dm2_v1_dungeon_get_square_type(dungeon, 10, 3, 9) != 1 ||
            dm2_v1_dungeon_get_first_thing(dungeon, 10, 4, 9) != 0xe80f ||
            dm2_v1_dungeon_get_next_thing(dungeon, 0xe80fu) != 0x2810) {
            fprintf(stderr, "Mac linked DB10 diagnostic pose invalid: %s\n", source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        dm2_v1_runtime_set_position(10, 3, 9, 1);
        memset(frame, 0, sizeof(frame));
        M11_GameView_Draw(&state, frame, 320, 200);
        memset(&linked_render, 0, sizeof(linked_render));
        if (dm2_v1_runtime_last_asset_item_count() != 5 ||
            !dm2_v1_runtime_last_item_render_receipt(&linked_render) ||
            !linked_render.asset_blit_ready ||
            linked_render.object_id != 0xe813u ||
            linked_render.source_static_object_draw_slot != 1 ||
            linked_render.source_static_object_record_ordinal != 5) {
            fprintf(stderr,
                    "Mac linked DB10 viewport missing: %s last=%04x slot=%d ordinal=%d asset_items=%d\n",
                    source_id, linked_render.object_id,
                    linked_render.source_static_object_draw_slot,
                    linked_render.source_static_object_record_ordinal,
                    dm2_v1_runtime_last_asset_item_count());
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    {
        uint8_t frame[320u * 200u];
        DM2_V1_RuntimeItemRenderReceipt outdoor_item;
        /* Original Mac retail map 15: two linked DB10 records are on the
         * floor square directly ahead of this outdoor party pose. */
        if (dm2_v1_dungeon_get_square_type(dungeon, 15, 10, 7) != 1 ||
            dm2_v1_dungeon_get_square_type(dungeon, 15, 10, 6) != 1 ||
            dm2_v1_dungeon_get_first_thing(dungeon, 15, 10, 6) != 0x2848 ||
            dm2_v1_dungeon_get_next_thing(dungeon, 0x2848u) != 0x6849) {
            fprintf(stderr, "Mac outdoor DB10 source pose invalid: %s\n", source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        dm2_v1_runtime_set_position(15, 10, 7, 0);
        memset(frame, 0, sizeof(frame));
        M11_GameView_Draw(&state, frame, 320, 200);
        memset(&outdoor_item, 0, sizeof(outdoor_item));
        if (dm2_v1_runtime_last_asset_item_count() != 2 ||
            !dm2_v1_runtime_last_item_render_receipt(&outdoor_item) ||
            !outdoor_item.valid || !outdoor_item.asset_blit_ready ||
            outdoor_item.item_category != 0x15 ||
            outdoor_item.object_id != 0x6849u ||
            outdoor_item.source_static_object_draw_slot != 0 ||
            outdoor_item.source_static_object_record_ordinal != 2) {
            fprintf(stderr,
                    "Mac outdoor DB10 viewport missing: %s last=%04x blit=%d items=%d\n",
                    source_id, outdoor_item.object_id,
                    outdoor_item.asset_blit_ready,
                    dm2_v1_runtime_last_asset_item_count());
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    {
        uint8_t frame[320u * 200u];
        DM2_V1_ViewportRect rect;
        DM2_V1_RuntimeViewportClickReceipt on_image, above_image;
        DM2_V1_BootExpandedRectReceipt rect7;
        /* Diagnostic pose only: retail map-5 floor (2,2) faces source
         * DB3 wall switch 0x4fa3 at (1,2). No New Game route is inferred. */
        if (dm2_v1_dungeon_get_square_type(dungeon, 5, 2, 2) != 1 ||
            dm2_v1_dungeon_get_square_type(dungeon, 5, 1, 2) != 0 ||
            !dm2_v1_boot_query_expanded_rect_receipt(profile, 7u, &rect7) ||
            !rect7.valid || rect7.rect.x != 0 || rect7.rect.y != 40) {
            fprintf(stderr, "Mac retail wall-target source pose invalid: %s\n", source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        dm2_v1_runtime_set_position(5, 2, 2, 3);
        memset(frame, 0, sizeof(frame));
        M11_GameView_Draw(&state, frame, 320, 200);
        memset(&on_image, 0, sizeof(on_image));
        memset(&above_image, 0, sizeof(above_image));
        if (!dm2_v1_viewport_wall_frame_rect_for_square(DM2_SQ_D0C, &rect) ||
            !dm2_v1_runtime_route_viewport_click(
                rect.x + rect.w / 2,
                rect.y + rect.h - 5 + rect7.rect.y, &on_image) ||
            !on_image.accepted || on_image.target_kind != 4u ||
            on_image.object_id != 0x4fa3 ||
            on_image.rect.y != rect.y + rect7.rect.y ||
            dm2_v1_runtime_route_viewport_click(
                rect.x + rect.w / 2, rect.y + 20, &above_image)) {
            fprintf(stderr, "Mac retail RECT_7 wall hitbox is misaligned: %s\n",
                    source_id);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    M11_GameView_Shutdown(&state);
    return 0;
}

int main(void)
{
    const char *retail = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    if (!retail) {
        puts("SKIP: authentic DM2 Mac ZIP environment is not set");
        return 0;
    }
    if (retail && run_one(retail, "mac-en-retail") != 0)
        return 1;
    return 0;
}
