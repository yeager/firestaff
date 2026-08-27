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
    uint8_t cursor[DM2_V1_MAX_LEVELS];
    if (!dungeon || dungeon->level_count <= 0 ||
        dungeon->level_count > DM2_V1_MAX_LEVELS) return;
    memset(maps, 0, sizeof(maps));
    for (int map = 0; map < dungeon->level_count; ++map) {
        maps[map].map_id = (uint8_t)map;
        maps[map].world_x = dungeon->map_offset_x[map];
        maps[map].world_y = dungeon->map_offset_y[map];
        maps[map].width = (int16_t)dungeon->level_widths[map];
        maps[map].height = (int16_t)dungeon->level_heights[map];
        cursor[map] = (uint8_t)map;
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
                memset(&receipt, 0, sizeof(receipt));
                target = dm2_v1_skproject_locate_other_level(
                    maps, (uint16_t)dungeon->level_count, (int16_t)map,
                    (int16_t)delta, &tx, &ty, cursor,
                    (uint16_t)dungeon->level_count, 0, NULL, &receipt);
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
    uint8_t cursor[DM2_V1_MAX_LEVELS];
    if (!dungeon || dungeon->level_count <= 0 ||
        dungeon->level_count > DM2_V1_MAX_LEVELS) return;
    memset(maps, 0, sizeof(maps));
    for (int map = 0; map < dungeon->level_count; ++map) {
        maps[map].map_id = (uint8_t)map;
        maps[map].world_x = (int16_t)dungeon->map_offset_x[map];
        maps[map].world_y = (int16_t)dungeon->map_offset_y[map];
        maps[map].width = (int16_t)dungeon->level_widths[map];
        maps[map].height = (int16_t)dungeon->level_heights[map];
        cursor[map] = (uint8_t)map;
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
                cursor[map] = 0xffu;
                target = dm2_v1_skproject_locate_other_level(
                    maps, (uint16_t)dungeon->level_count, (int16_t)map, 1,
                    &tx, &ty, cursor, (uint16_t)dungeon->level_count,
                    0, NULL, NULL);
                cursor[map] = (uint8_t)map;
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
    cls = (unsigned)dm2_actu_type(record);
    if (cls >= 0x80u)
        return 0;
    ++c->counts[cls];
    if (cls == 0x05u || cls == 0x17u || cls == 0x18u || cls == 0x1au ||
        cls == 0x27u || cls == 0x46u)
        printf("  actuator type=%02x object=%04x map=%d x=%d y=%d w2=%04x w4=%04x w6=%04x\n",
               cls, thing, level, x, y,
               dm2_actu_w2(record), dm2_actu_w4(record), dm2_actu_w6(record));
    if (c->counts[cls] == 1u) {
        c->first_map[cls] = (unsigned)level;
        c->first_x[cls] = (unsigned)x;
        c->first_y[cls] = (unsigned)y;
        c->first_w2[cls] = record[2] | ((unsigned)record[3] << 8);
        c->first_w4[cls] = record[4] | ((unsigned)record[5] << 8);
        c->first_w6[cls] = record[6] | ((unsigned)record[7] << 8);
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
    {
        unsigned char framebuffer[320u * 200u];
        memset(framebuffer, 0, sizeof(framebuffer));
        while (state.dm2MacMovieActive)
            M11_GameView_Draw(&state, framebuffer, 320, 200);
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) ==
            M11_GAME_INPUT_IGNORED ||
        state.dm2State.startup_menu_active ||
        !state.dm2State.level_loaded) {
        fprintf(stderr, "Mac census New Game failed: %s\n", source_id);
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
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW || !state.inventoryPanelActive) {
        fprintf(stderr, "Mac authenticated keyboard item transaction unavailable: %s\n",
                source_id);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    puts("  authenticated Mac keyboard item transaction accepted");
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK) !=
            M11_GAME_INPUT_REDRAW || state.inventoryPanelActive) {
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
