/* Opt-in real-media Amiga NEW GAME/runtime handoff regression. */

#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int exercise_authentic_pit(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!profile || !dungeon || !dungeon->record_graph_complete) return 0;
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
                    dm2_v1_runtime_set_outdoor(
                        dm2_v1_dungeon_is_outdoor(dungeon, map));
                    dm2_v1_runtime_tick();
                    memset(&receipt, 0, sizeof(receipt));
                    if (dm2_v1_runtime_move(dir) != 0 ||
                        !dm2_v1_boot_runtime_capture(profile, &receipt) ||
                        receipt.current_level == map)
                    {
                        continue;
                    }
                    printf("  authentic Amiga pit transition map %d,%d,%d -> %d,%d,%d\n",
                           map, px, py, receipt.current_level,
                           receipt.party_x, receipt.party_y);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int exercise_authentic_stairs(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!profile || !dungeon || !dungeon->record_graph_complete) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                if (raw < 0 || dm2_v1_dungeon_get_square_type(
                        dungeon, map, x, y) != 3)
                    continue;
                for (int dir = 0; dir < 4; ++dir) {
                    int px = x - dx[dir], py = y - dy[dir];
                    DM2_V1_BootRuntimeReceipt receipt;
                    if (px < 0 || py < 0 ||
                        px >= dungeon->level_widths[map] ||
                        py >= dungeon->level_heights[map] ||
                        dm2_v1_dungeon_get_square_type(
                            dungeon, map, px, py) == 0)
                        continue;
                    dm2_v1_runtime_set_position(map, px, py, dir);
                    dm2_v1_runtime_set_outdoor(
                        dm2_v1_dungeon_is_outdoor(dungeon, map));
                    dm2_v1_runtime_tick();
                    memset(&receipt, 0, sizeof(receipt));
                    if (dm2_v1_runtime_move(dir) == 0 &&
                        dm2_v1_boot_runtime_capture(profile, &receipt) &&
                        receipt.current_level != map) {
                        printf("  authentic Amiga stairs transition map %d,%d,%d -> %d,%d,%d\n",
                               map, x, y, receipt.current_level,
                               receipt.party_x, receipt.party_y);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

static int exercise_authentic_active_creature(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    unsigned char framebuffer[320 * 200];
    if (!profile || !dungeon) return 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        DM2_V1_G1CreatureMapChipRuntimeReceipt materials;
        int base_x = dungeon->level_widths[map] > 1 ? 1 : 0;
        int base_y = dungeon->level_heights[map] > 1 ? 1 : 0;
        dm2_v1_runtime_set_outdoor(dm2_v1_dungeon_is_outdoor(dungeon, map));
        dm2_v1_runtime_set_position(map, base_x, base_y, 0);
        memset(&materials, 0, sizeof(materials));
        if (!dm2_v1_runtime_g1_creature_map_chip_receipt(&materials) ||
            !materials.valid) continue;
        for (int i = 0; i < materials.material_count; ++i) {
            const DM2_V1_G1CreatureMapChipMaterial *material =
                &materials.materials[i];
            for (int dir = 0; dir < 4; ++dir) {
                int px = material->x - dx[dir];
                int py = material->y - dy[dir];
                DM2_V1_BootRuntimeRenderReceipt render;
                DM2_V1_RuntimeCreatureRenderReceipt creature;
                if (px < 0 || py < 0 || px >= dungeon->level_widths[map] ||
                    py >= dungeon->level_heights[map]) continue;
                dm2_v1_runtime_set_outdoor(dm2_v1_dungeon_is_outdoor(dungeon, map));
                dm2_v1_runtime_set_position(map, px, py, dir);
                memset(framebuffer, 0, sizeof(framebuffer));
                memset(&render, 0, sizeof(render));
                memset(&creature, 0, sizeof(creature));
                (void)dm2_v1_boot_runtime_render_frame(
                    profile, framebuffer, 320, 320, 200, NULL, NULL, &render);
                if (!dm2_v1_runtime_last_creature_render_receipt(&creature))
                    continue;
                if (render.render_result == 0 && render.v1_succeeded &&
                    creature.valid && creature.source_kind == 2 &&
                    creature.thing_handle == material->object_id &&
                    creature.asset_blit_ready && !creature.fallback_drawn &&
                    creature.gdat_index != 0) {
                    DM2_V1_CreatureScheduleReceipt schedule;
                    memset(&schedule, 0, sizeof(schedule));
                    if (dm2_v1_runtime_schedule_creature_at(
                            map, material->x, material->y, &schedule) &&
                        schedule.valid)
                        dm2_v1_runtime_tick();
                    printf("  active Amiga creature DB4/F9 map %d,%d,%d type %d GDAT %d\n",
                           map, material->x, material->y,
                           creature.creature_type, creature.gdat_index);
                    printf("  Amiga dynamic path attempts %d admissions %d\n",
                           dm2_v1_runtime_dynamic_path_attempts(),
                           dm2_v1_runtime_dynamic_path_admissions());
                    printf("  Amiga dynamic move queues %d\n",
                           dm2_v1_runtime_dynamic_move_queue_admissions());
                    for (int tick = 0; tick < 4; ++tick)
                        dm2_v1_runtime_tick();
                    printf("  Amiga dynamic move timers %d successes %d\n",
                           dm2_v1_runtime_dynamic_move_timer_consumptions(),
                           dm2_v1_runtime_dynamic_move_successes());
                    printf("  Amiga dynamic move last failure %d\n",
                           dm2_v1_runtime_dynamic_move_last_failure());
                    return dm2_v1_runtime_dynamic_path_admissions() > 0 &&
                           dm2_v1_runtime_dynamic_move_queue_admissions() > 0 &&
                           dm2_v1_runtime_dynamic_move_timer_consumptions() > 0 &&
                           dm2_v1_runtime_dynamic_move_successes() > 0;
                }
            }
        }
    }
    return 0;
}

static int exercise_authentic_db1(
    DM2_V1_BootProfile *profile, const DM2_V1_DungeonData *dungeon)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    if (!profile || !dungeon || !dungeon->record_graph_complete) return 0;
    unsigned candidates = 0;
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int y = 0; y < dungeon->level_heights[map]; ++y) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                int first, type = -1;
                const uint8_t *record;
                int w2, w4, dest_map, dest_x, dest_y, rotation, rotation_type;
                if (raw < 0 || dm2_v1_dungeon_get_square_type(dungeon, map, x, y) != 5 ||
                    (raw & 0x08) == 0)
                    continue;
                first = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
                if (first < 0 || (((unsigned)first >> 10) & 0xfu) != 1u)
                    continue;
                record = dm2_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)first, &type, NULL, NULL);
                if (!record || type != 1) continue;
                w2 = dm2_v1_dungeon_read_record_u16(dungeon, record + 2);
                w4 = dm2_v1_dungeon_read_record_u16(dungeon, record + 4);
                dest_x = w2 & 0x1f; dest_y = (w2 >> 5) & 0x1f;
                dest_map = (w4 >> 8) & 0xff;
                rotation = (w2 >> 10) & 3; rotation_type = (w2 >> 12) & 1;
                if (dest_map < 0 || dest_map >= dungeon->level_count ||
                    dest_x >= dungeon->level_widths[dest_map] ||
                    dest_y >= dungeon->level_heights[dest_map] ||
                    (w2 & 0x6000) != 0x4000)
                    continue;
                ++candidates;
                for (int dir = 0; dir < 4; ++dir) {
                    int px = x - dx[dir], py = y - dy[dir];
                    DM2_V1_BootRuntimeReceipt receipt;
                    if (px < 0 || py < 0 || px >= dungeon->level_widths[map] ||
                        py >= dungeon->level_heights[map] ||
                        dm2_v1_dungeon_get_square_type(dungeon, map, px, py) == 0)
                        continue;
                    dm2_v1_runtime_set_position(map, px, py, dir);
                    dm2_v1_runtime_set_outdoor(dm2_v1_dungeon_is_outdoor(dungeon, map));
                    dm2_v1_runtime_tick();
                    memset(&receipt, 0, sizeof(receipt));
                    if (dm2_v1_runtime_move(dir) != 0 ||
                        !dm2_v1_boot_runtime_capture(profile, &receipt)) continue;
                    if (receipt.current_level == dest_map && receipt.party_x == dest_x &&
                        receipt.party_y == dest_y && receipt.party_dir ==
                            (rotation_type ? rotation : ((dir + rotation) & 3))) {
                        printf("  authentic Amiga DB1 transition map %d,%d,%d -> %d,%d,%d\n",
                               map, x, y, dest_map, dest_x, dest_y);
                        return 1;
                    }
                }
            }
        }
    }
    printf("  Amiga DB1 candidates=%u (no positive runtime route)\n", candidates);
    return candidates ? -1 : 0;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_AMIGA_ROOT");
    DM2_V1_BootStartupLaunch launch;
    DM2_V1_BootProfile *profile;
    DM2_V1_BootRuntimeReceipt runtime;

    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_DM2_AMIGA_ROOT is not set");
        return 77;
    }
    memset(&launch, 0, sizeof(launch));
    expect(dm2_v1_boot_startup_launch_alloc(root, &launch) == 1 &&
               launch.profile && launch.profile->assets_verified &&
               launch.profile->platform == DM2_PLATFORM_AMIGA_EN,
           "authentic Amiga installer is admitted");
    profile = launch.profile;
    if (!profile || !profile->assets_verified) {
        dm2_v1_boot_startup_launch_cleanup(&launch);
        return 1;
    }
    expect(dm2_v1_boot_prepare_new_game_world(profile) == 1,
           "Amiga materializes the source-owned NEW GAME candidate");
    expect(dm2_v1_boot_new_game_runtime_candidate_readonly(profile) != NULL,
           "Amiga retains the private runtime candidate after STARTEND");
    memset(&runtime, 0, sizeof(runtime));
    expect(dm2_v1_boot_commit_new_game_session(profile) == 1,
           "Amiga commits the source-owned NEW GAME session");
    expect(dm2_v1_boot_runtime_capture(profile, &runtime) == 1 &&
               runtime.runtime_ready && runtime.party_x == 1 &&
               runtime.party_y == 8 && runtime.party_dir == 0 &&
               runtime.leader_hand_object == 0xffffu,
           "Amiga exposes the committed source party to runtime");
    expect(dm2_v1_boot_runtime_move(profile, runtime.party_dir, &runtime) == 1,
           "Amiga accepts a source-owned movement command");
    expect(exercise_authentic_pit(
               profile, (const DM2_V1_DungeonData *)profile->dungeon_data),
           "Amiga commits an authentic open-pit transition");
    expect(exercise_authentic_stairs(
               profile, (const DM2_V1_DungeonData *)profile->dungeon_data),
           "Amiga commits an authentic stairs transition");
    expect(exercise_authentic_active_creature(
               profile, (const DM2_V1_DungeonData *)profile->dungeon_data),
           "Amiga renders an authentic active DB4 creature through F9");
    (void)exercise_authentic_db1(
        profile, (const DM2_V1_DungeonData *)profile->dungeon_data);
    dm2_v1_boot_startup_launch_cleanup(&launch);
    if (failures != 0) return 1;
    puts("PASS: authentic DM2 Amiga NEW GAME reaches runtime");
    return 0;
}
