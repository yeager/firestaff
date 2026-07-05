/*
 * test_dm2_v1_runtime_handoff_smoke.c
 *
 * Focused DM2 V1 runtime smoke gate.  This proves the boot/profile
 * handoff can enter the V1 runtime and advance exactly one deterministic
 * tick boundary.  It deliberately uses a synthetic verified profile and
 * does not claim real-asset launch, rendering, movement parity, or
 * end-to-end playability.
 *
 * Source-lock:
 *   DM2: SKULL.ASM T520/T560 are the documented party placement and
 *   dungeon tick anchors in dm2_v1_boot.c / dm2_v1_runtime.c.
 *   ReDMCSB GAMELOOP.C lines 55-70 shows the V1 loop boundary that
 *   advances timeline/tick work once per loop.
 *   ReDMCSB TOWNSGLB.H lines 1381-1388 documents party direction,
 *   party map position, and game-time globals used by the V1 lineage.
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_game.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_shop.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#define FS_MKDIR(path) _mkdir(path)
#define FS_RMDIR(path) _rmdir(path)
#define FS_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define FS_MKDIR(path) mkdir(path, 0700)
#define FS_RMDIR(path) rmdir(path)
#define FS_GETPID() getpid()
#endif

static int passed;
static int failed;
static uint8_t s_ceiling_pixels[16 * 8];
static uint8_t s_floor_pixels[16 * 8];
static uint8_t s_wall_pixels[16 * 8];
static uint8_t s_door_panel_pixels[16 * 8];
static uint8_t s_door_frame_pixels[16 * 8];
static uint8_t s_door_button_pixels[16 * 8];
static uint8_t s_wall_button_pixels[16 * 8];

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static void make_synthetic_verified_profile(DM2_V1_BootProfile *profile)
{
    dm2_v1_boot_profile_init(profile);
    profile->assets_verified = 1;
    snprintf(profile->asset_root, sizeof(profile->asset_root),
             "synthetic-dm2-v1-runtime-handoff");
}

static int synthetic_viewport_asset_fetch(void *user,
                                          int gdat_index,
                                          const uint8_t **out_pixels,
                                          int *out_w,
                                          int *out_h,
                                          int *out_stride)
{
    int *fetch_count = (int *)user;
    if (fetch_count) {
        ++*fetch_count;
    }
    if (gdat_index == -2) {
        if (out_pixels) *out_pixels = s_ceiling_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index == -1) {
        if (out_pixels) *out_pixels = s_floor_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <=
        DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE -
            DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST &&
        DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE - gdat_index < 0x40) {
        if (out_pixels) *out_pixels = s_wall_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <=
        DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE -
            DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT &&
        DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE - gdat_index < 0x20) {
        if (out_pixels) *out_pixels = s_door_frame_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <=
        DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE -
            DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_RELEASED &&
        DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE - gdat_index < 0x08) {
        if (out_pixels) *out_pixels = s_door_button_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <= DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE &&
        DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE - gdat_index <
            (0x100 << DM2_V1_VIEWPORT_GFX_WALL_BUTTON_INDEX_SHIFT)) {
        if (out_pixels) *out_pixels = s_wall_button_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <=
        DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE -
            DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FRONT &&
        DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - gdat_index < 0x04) {
        if (out_pixels) *out_pixels = s_door_panel_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    return -1;
}

static void put16le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static size_t build_skproject_door_fixture(uint8_t *buf, size_t cap)
{
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + 4u;
    const size_t thing_base = sft_base + 2u;
    const size_t door_base = thing_base;
    const size_t text_base = door_base + 4u;
    const size_t raw_map_base = text_base + 4u;
    uint8_t *desc;
    uint16_t door_bits;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);
    buf[4] = 1;
    put16le(buf + 10, 1);
    put16le(buf + 12, 1);
    put16le(buf + 16, 1);
    desc = buf + header_size;
    put16le(desc + 8, (uint16_t)((1u << 6) | (1u << 11)));
    put16le(buf + column_base, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x0800);
    put16le(buf + door_base, 0xfffe);
    door_bits = (uint16_t)((1u << 6) | (1u << 11) | (1u << 5) | 1u);
    put16le(buf + door_base + 2, door_bits);
    put16le(buf + text_base, 0x0000);
    put16le(buf + text_base + 2, 0x0000);
    buf[raw_map_base + 0] = 0x20;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x90;
    buf[raw_map_base + 3] = 0x20;
    return raw_map_base + 4u;
}

static size_t build_skproject_custom_wall_button_fixture(uint8_t *buf,
                                                        size_t cap)
{
    size_t size = build_skproject_door_fixture(buf, cap);
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + 4u;
    const size_t thing_base = sft_base + 2u;
    const size_t door_base = thing_base;
    const size_t text_base = door_base + 4u;
    uint16_t door_bits;

    if (size == 0) return 0;
    put16le(buf + sft_base, 0x8800);
    door_bits = (uint16_t)((1u << 5) | 1u);
    put16le(buf + door_base + 2, door_bits);
    put16le(buf + text_base, 0x0000);
    put16le(buf + text_base + 2, (uint16_t)((1u << 1) | (0x2au << 3)));
    return size;
}

static size_t build_skproject_actuator_wall_button_fixture(uint8_t *buf,
                                                          size_t cap)
{
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + 4u;
    const size_t thing_base = sft_base + 2u;
    const size_t door_base = thing_base;
    const size_t actuator_base = door_base + 4u;
    const size_t raw_map_base = actuator_base + 8u;
    uint8_t *desc;
    uint16_t door_bits;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);
    buf[4] = 1;
    put16le(buf + 10, 1);
    put16le(buf + 12, 1);
    put16le(buf + 18, 1);
    desc = buf + header_size;
    put16le(desc + 8, (uint16_t)((1u << 6) | (1u << 11)));
    put16le(buf + column_base, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x8c00);
    put16le(buf + door_base, 0xfffe);
    door_bits = (uint16_t)((1u << 5) | 1u);
    put16le(buf + door_base + 2, door_bits);
    put16le(buf + actuator_base, 0x0000);
    put16le(buf + actuator_base + 2, 0x0000);
    put16le(buf + actuator_base + 4, (uint16_t)(3u << 12));
    put16le(buf + actuator_base + 6, 0x0000);
    buf[raw_map_base + 0] = 0x20;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x90;
    buf[raw_map_base + 3] = 0x20;
    return raw_map_base + 4u;
}

static size_t build_skproject_actuator_wall_button_map_list_fixture(
    uint8_t *buf,
    size_t cap)
{
    size_t size = build_skproject_actuator_wall_button_fixture(buf, cap);
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t raw_map_base = header_size + map_desc_size +
                                4u + 2u + 4u + 8u;
    uint8_t *desc;

    if (size == 0 || cap < raw_map_base + 9u) return 0;
    desc = buf + header_size;
    put16le(desc + 10, 4);
    put16le(desc + 12, (uint16_t)(1u << 4));
    buf[raw_map_base + 4u] = 0x7e;
    buf[raw_map_base + 5u] = 0x10;
    buf[raw_map_base + 6u] = 0x20;
    buf[raw_map_base + 7u] = 0x2a;
    buf[raw_map_base + 8u] = 0x30;
    return raw_map_base + 9u;
}

static size_t build_skproject_square_actuator_fixture(uint8_t *buf,
                                                      size_t cap,
                                                      uint8_t actuator_type,
                                                      uint16_t flag)
{
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + 4u;
    const size_t actuator_base = sft_base + 2u;
    const size_t raw_map_base = actuator_base + 8u;
    uint8_t *desc;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);
    buf[4] = 1;
    put16le(buf + 10, 1);
    put16le(buf + 18, 1);
    desc = buf + header_size;
    put16le(desc + 8, (uint16_t)((1u << 6) | (1u << 11)));
    put16le(buf + column_base, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x8c00);
    put16le(buf + actuator_base, 0xfffe);
    buf[actuator_base + 2] = actuator_type;
    buf[actuator_base + 3] = 0;
    put16le(buf + actuator_base + 4, flag);
    buf[actuator_base + 6] = 0;
    buf[actuator_base + 7] = 0;
    buf[raw_map_base + 0] = 0x10;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x20;
    buf[raw_map_base + 3] = 0x20;
    return raw_map_base + 4u;
}

static void test_first_tick_after_boot_profile_handoff(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_GameState *state;
    DM2_V1_SessionState session;

    make_synthetic_verified_profile(&profile);
    CHECK(dm2_v1_boot_enter_game(&profile) == 0,
          "synthetic verified profile enters DM2 V1 game state");
    CHECK(profile.dm2_state != NULL,
          "boot handoff populates dm2_state");
    CHECK(profile.dungeon_data != NULL,
          "boot handoff owns a dungeon data handle even without real assets");
    {
        DM2_V1_DungeonData *dd = (DM2_V1_DungeonData *)profile.dungeon_data;
        dd->raw_size = 492 + 8;
        dd->raw_data = (uint8_t *)calloc(1u, (size_t)dd->raw_size);
        dd->level_count = 1;
        dd->level_widths[0] = 2;
        dd->level_heights[0] = 2;
        dd->level_offsets[0] = 0;
    }

    state = (DM2_V1_GameState *)profile.dm2_state;
    CHECK(state->party_x == 15 && state->party_y == 15 &&
          state->party_dir == 0 && state->current_level == 0,
          "boot handoff starts at the documented DM2 V1 party snapshot");

    dm2_v1_runtime_init(&profile);
    CHECK(dm2_v1_runtime_get_tick_count() == 0,
          "runtime tick counter starts at zero after handoff bind");
    CHECK(dm2_v1_runtime_has_dungeon_data() == 1,
          "runtime bind exposes the boot handoff dungeon data");
    CHECK(dm2_v1_runtime_get_party_x() == 15 &&
          dm2_v1_runtime_get_party_y() == 15 &&
          dm2_v1_runtime_get_party_dir() == 0,
          "runtime accessors read the handoff party snapshot");
    CHECK(dm2_v1_runtime_get_last_target_message() == NULL &&
          dm2_v1_runtime_get_spawn_count() == 0,
          "runtime target receipts start empty after handoff bind");
    CHECK(dm2_v1_runtime_signal_item_used(1001) == 1 &&
          dm2_v1_runtime_get_last_target_message() != NULL &&
          strstr(dm2_v1_runtime_get_last_target_message(),
                 "flickering light") != NULL,
          "runtime item-used trigger applies display-message target");
    CHECK(dm2_v1_runtime_signal_combat_ended(1) == 2 &&
          dm2_v1_runtime_get_spawn_count() == 1 &&
          dm2_v1_runtime_get_last_spawn_ai() == 10 &&
          dm2_v1_runtime_get_last_spawn_x() == 1 &&
          dm2_v1_runtime_get_last_spawn_y() == 1 &&
          dm2_v1_runtime_get_last_spawn_level() == 0,
          "runtime combat-ended trigger applies creature-spawn target");
    CHECK(dm2_v1_runtime_invoke_actuator(
              0, 0, 0, DM2_ACTUATOR_CREATURE_GENERATOR,
              DM2_AI_DRAGOTH_MINION) == 0 &&
          dm2_v1_runtime_get_last_actuator_type() ==
              DM2_ACTUATOR_CREATURE_GENERATOR &&
          dm2_v1_runtime_get_last_actuator_x() == 0 &&
          dm2_v1_runtime_get_last_actuator_y() == 0 &&
          dm2_v1_runtime_get_last_actuator_level() == 0 &&
          dm2_v1_runtime_get_spawn_count() == 2 &&
          dm2_v1_runtime_get_last_spawn_ai() == DM2_AI_DRAGOTH_MINION,
          "runtime creature-generator actuator applies spawn target");
    CHECK(dm2_v1_runtime_invoke_actuator(
              0, 0, 0, DM2_ACTUATOR_ITEM_GENERATOR, 0x1234u) == 0 &&
          dm2_v1_runtime_get_last_actuator_type() ==
              DM2_ACTUATOR_ITEM_GENERATOR &&
          dm2_v1_runtime_get_last_generated_object() == 0x1234u,
          "runtime item-generator actuator records generated object target");
    CHECK(dm2_v1_runtime_invoke_actuator(
              0, 0, 0, DM2_ACTUATOR_MISSILE_SHOOTER,
              DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL) == 0 &&
          dm2_v1_runtime_get_last_actuator_type() ==
              DM2_ACTUATOR_MISSILE_SHOOTER &&
          dm2_v1_runtime_get_last_projectile_slot() >= 0 &&
          dm2_v1_runtime_get_projectile_actuator_count() == 1,
          "runtime missile-shooter actuator dispatches a projectile target");
    {
        uint8_t fixture[128];
        size_t fixture_size = build_skproject_square_actuator_fixture(
            fixture, sizeof(fixture),
            (uint8_t)DM2_ACTUATOR_ITEM_GENERATOR, 0x4567u);
        DM2_V1_DungeonData *replacement =
            (DM2_V1_DungeonData *)calloc(1, sizeof(*replacement));
        CHECK(fixture_size > 0 && replacement != NULL,
              "runtime square-actuator fixture allocates");
        if (replacement &&
            dm2_v1_dungeon_load(replacement, fixture, (int)fixture_size) == 0) {
            DM2_V1_DungeonData *old_dd =
                (DM2_V1_DungeonData *)profile.dungeon_data;
            dm2_v1_dungeon_free(old_dd);
            free(old_dd);
            profile.dungeon_data = replacement;
            replacement = NULL;
            dm2_v1_runtime_init(&profile);
            CHECK(dm2_v1_runtime_invoke_square_actuators(0, 0, 0) == 1 &&
                  dm2_v1_runtime_get_last_actuator_type() ==
                      DM2_ACTUATOR_ITEM_GENERATOR &&
                  dm2_v1_runtime_get_last_generated_object() == 0x4567u,
                  "runtime square-first DB3 actuator invokes generated-object target");
            {
                int before_actuators = dm2_v1_runtime_get_actuator_count();
                dm2_v1_runtime_set_position(0, 0, 1, 0);
                dm2_v1_runtime_set_outdoor(1);
                CHECK(dm2_v1_runtime_move(0) == 0 &&
                      dm2_v1_runtime_get_party_x() == 0 &&
                      dm2_v1_runtime_get_party_y() == 0 &&
                      dm2_v1_runtime_get_actuator_count() ==
                          before_actuators + 1 &&
                      dm2_v1_runtime_get_last_generated_object() == 0x4567u,
                      "runtime arrival on square-first DB3 actuator invokes generated-object target");
            }
        } else {
            CHECK(0, "runtime square-actuator fixture loads");
        }
        if (replacement) {
            dm2_v1_dungeon_free(replacement);
            free(replacement);
        }
    }

    memset(&session, 0, sizeof(session));
    dm2_v1_session_new(&session);
    session.game_tick = 77;
    session.party_level = 2;
    session.party_x = 19;
    session.party_y = 12;
    session.party_dir = 3;
    session.outdoor_mode = 1;
    session.time_of_day_minutes = 1080;
    session.rain_intensity = 64;
    session.original_leader_hand_object = 0x08000034u;
    ((DM2_ChampionRecord *)session.champion_data[0])->inventory[2] =
        0x0A000033u;
    CHECK(dm2_v1_runtime_apply_session(&session) == 0,
          "runtime accepts a bounded DM2 startup session after handoff");
    CHECK(state->party_x == 19 && state->party_y == 12 &&
          state->party_dir == 3 && state->current_level == 2 &&
          state->outdoor == 1,
          "session apply updates the boot-owned DM2 game state");
    CHECK(dm2_v1_runtime_get_tick_count() == 77 &&
          dm2_v1_runtime_get_party_x() == 19 &&
          dm2_v1_runtime_get_party_y() == 12 &&
          dm2_v1_runtime_get_party_dir() == 3,
          "session apply updates runtime tick and party accessors");
    CHECK(dm2_v1_runtime_get_weather() == DM2_WEATHER_RAIN &&
          dm2_v1_runtime_get_weather_intensity() == 64,
          "session apply updates runtime weather state");
    CHECK(dm2_v1_runtime_get_leader_hand_object() ==
              session.original_leader_hand_object,
          "session apply updates runtime leader-hand object");
    CHECK(dm2_v1_runtime_get_champion_inventory_object(0, 2) ==
              0x0A000033u,
          "session apply updates runtime champion inventory ObjectIDs");
    CHECK(dm2_v1_runtime_set_champion_inventory_object(0, 2,
                                                       0x0A000044u) == 0 &&
          dm2_v1_runtime_get_champion_inventory_object(0, 2) ==
              0x0A000044u,
          "runtime champion inventory ObjectID writeback is mutable");
    dm2_v1_runtime_set_leader_hand_object(0x0A000055u);
    session.original_leader_hand_object = 0u;
    ((DM2_ChampionRecord *)session.champion_data[0])->inventory[2] = 0u;
    CHECK(dm2_v1_runtime_export_inventory_to_session(&session) == 0 &&
          session.original_leader_hand_object == 0x0A000055u &&
          ((DM2_ChampionRecord *)session.champion_data[0])->inventory[2] ==
              0x0A000044u,
          "runtime inventory export writes leader hand and champion slots into session");
    {
        char tmpdir[256];
        char slot_path[320];
        DM2_V1_SessionState restored;
        snprintf(tmpdir, sizeof(tmpdir),
                 "/tmp/firestaff_dm2_runtime_inv_%d", FS_GETPID());
        (void)remove(tmpdir);
        (void)FS_RMDIR(tmpdir);
        CHECK(FS_MKDIR(tmpdir) == 0,
              "runtime inventory export test creates save root");
        CHECK(dm2_v1_session_save_slot(tmpdir, 6, "RuntimeInv",
                                       &session) == 0,
              "runtime inventory export session saves to DM2 slot");
        memset(&restored, 0, sizeof(restored));
        CHECK(dm2_v1_session_load_slot(tmpdir, 6, &restored) == 0,
              "runtime inventory export session reloads from DM2 slot");
        CHECK(restored.original_leader_hand_object == 0x0A000055u &&
              ((DM2_ChampionRecord *)restored.champion_data[0])->inventory[2] ==
                  0x0A000044u,
              "runtime inventory export survives DM2 slot save/load");
        snprintf(slot_path, sizeof(slot_path), "%s/SKSave06.dat", tmpdir);
        (void)remove(slot_path);
        snprintf(slot_path, sizeof(slot_path), "%s/SKSave.bak", tmpdir);
        (void)remove(slot_path);
        (void)FS_RMDIR(tmpdir);
    }

    {
        uint8_t framebuffer[320 * 200];
        int fetch_count = 0;
        memset(s_ceiling_pixels, 12, sizeof(s_ceiling_pixels));
        memset(s_floor_pixels, 4, sizeof(s_floor_pixels));
        memset(s_wall_pixels, 9, sizeof(s_wall_pixels));
        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_runtime_set_outdoor(0);
        dm2_v1_runtime_set_viewport_asset_provider(
            synthetic_viewport_asset_fetch, &fetch_count);
        CHECK(dm2_v1_runtime_render_frame(
                  dm2_v1_runtime_get_party_dir(),
                  dm2_v1_runtime_get_party_x(),
                  dm2_v1_runtime_get_party_y(),
                  framebuffer, 320, 320, 200) == 0,
              "runtime renders through an injected viewport asset provider");
        CHECK(fetch_count == 12,
              "runtime viewport provider receives ceiling, floor and viewport-cell wall fetches");
        CHECK(dm2_v1_runtime_last_asset_floor_ceiling_count() == 2 &&
              dm2_v1_runtime_last_fallback_floor_ceiling_count() == 0,
              "runtime records asset-backed floor/ceiling draw counts");
        CHECK(dm2_v1_runtime_last_asset_wall_count() == 10 &&
              dm2_v1_runtime_last_fallback_wall_count() == 0,
              "runtime records asset-backed viewport-cell wall draw counts");
        CHECK(dm2_v1_runtime_last_asset_door_frame_count() == 0 &&
              dm2_v1_runtime_last_fallback_door_count() == 0,
              "runtime records no door-frame draw when no front door is visible");
        CHECK(framebuffer[0] == 1,
              "runtime asset-provider frame completes the shared viewport render pass");
        dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
    }

    state->gold = 240;
    dm2_v1_shop_reset_state();
    dm2_v1_runtime_set_position(0, 10, 6, 0);
    dm2_v1_runtime_set_outdoor(1);
    CHECK(dm2_v1_runtime_enter_shop(0, 10, 5) == 0,
          "runtime enters a catalog-backed DM2 shop by map position");
    CHECK(dm2_v1_shop_is_active() == 1 &&
          dm2_v1_shop_get_active_shop() == DM2_SHOP_ID_GENERAL,
          "runtime shop entry activates the General Store");
    CHECK(dm2_v1_shop_get_party_gold() == 240u,
          "runtime shop entry syncs party gold into shop state");
    CHECK(state->party_x == 10 && state->party_y == 6 &&
          state->party_dir == 0 && state->current_level == 0 &&
          state->outdoor == 1,
          "runtime position/outdoor setters update boot-owned game state");

    dm2_v1_runtime_set_outdoor(0);
    {
        int door_x = 0;
        int door_y = 0;
        int found_door_site = 0;
        uint8_t framebuffer[320 * 200];
        int fetch_count = 0;
        for (int y = 0; y < 63 && !found_door_site; ++y) {
            for (int x = 0; x < 64 && !found_door_site; ++x) {
                if (dm2_v1_dungeon_get_tile_raw(
                        (DM2_V1_DungeonData *)profile.dungeon_data,
                        0, x, y) >= 0 &&
                    dm2_v1_dungeon_get_tile_raw(
                        (DM2_V1_DungeonData *)profile.dungeon_data,
                        0, x, y + 1) >= 0) {
                    door_x = x;
                    door_y = y;
                    found_door_site = 1;
                }
            }
        }
        CHECK(found_door_site,
              "runtime smoke finds a valid adjacent front-door site");
        dm2_v1_runtime_set_position(0, door_x, door_y + 1, 0);
        CHECK(dm2_v1_dungeon_set_tile_raw(
                  (DM2_V1_DungeonData *)profile.dungeon_data,
                  0, door_x, door_y, 4u) == 0,
          "runtime smoke seeds a closed front door tile");
        CHECK(dm2_v1_runtime_get_door_state(0, door_x, door_y) == 4,
          "runtime door state reads closed front door");
        CHECK(dm2_v1_runtime_door_action(0, door_x, door_y, 0, 0) == 0,
          "runtime door action opens one door step");
        CHECK(dm2_v1_runtime_get_door_state(0, door_x, door_y) == -1 ||
              dm2_v1_dungeon_get_tile_raw(
                  (DM2_V1_DungeonData *)profile.dungeon_data,
                  0, door_x, door_y) == 3,
          "runtime door action writes the stepped raw tile state");
        memset(s_ceiling_pixels, 12, sizeof(s_ceiling_pixels));
        memset(s_floor_pixels, 4, sizeof(s_floor_pixels));
        memset(s_wall_pixels, 9, sizeof(s_wall_pixels));
        memset(s_door_panel_pixels, 8, sizeof(s_door_panel_pixels));
        memset(s_door_frame_pixels, 15, sizeof(s_door_frame_pixels));
        memset(s_door_button_pixels, 4, sizeof(s_door_button_pixels));
        memset(framebuffer, 0, sizeof(framebuffer));
        fetch_count = 0;
        dm2_v1_dungeon_set_tile_raw(
            (DM2_V1_DungeonData *)profile.dungeon_data,
            0, door_x, door_y, 4u);
        dm2_v1_runtime_set_viewport_asset_provider(
            synthetic_viewport_asset_fetch, &fetch_count);
        CHECK(dm2_v1_runtime_render_frame(
                  dm2_v1_runtime_get_party_dir(),
                  dm2_v1_runtime_get_party_x(),
                  dm2_v1_runtime_get_party_y(),
                  framebuffer, 320, 320, 200) == 0,
              "runtime renders a closed front door through the viewport asset provider");
        CHECK(dm2_v1_runtime_last_asset_door_panel_count() == 1 &&
              dm2_v1_runtime_last_asset_door_frame_count() == 1 &&
              dm2_v1_runtime_last_fallback_door_count() == 0,
              "runtime records asset-backed closed front door panel/frame draw counts");
        dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);

        {
            uint8_t fixture[128];
            size_t fixture_size = build_skproject_door_fixture(
                fixture, sizeof(fixture));
            DM2_V1_DungeonData *replacement =
                (DM2_V1_DungeonData *)calloc(1, sizeof(*replacement));
            CHECK(fixture_size > 0 && replacement != NULL,
                  "runtime door-record fixture allocates");
            if (replacement &&
                dm2_v1_dungeon_load(replacement, fixture, (int)fixture_size) == 0) {
                DM2_V1_DungeonData *old_dd =
                    (DM2_V1_DungeonData *)profile.dungeon_data;
                dm2_v1_dungeon_free(old_dd);
                free(old_dd);
                profile.dungeon_data = replacement;
                replacement = NULL;
                dm2_v1_runtime_set_position(0, 1, 1, 0);
                dm2_v1_runtime_set_outdoor(0);
                memset(framebuffer, 0, sizeof(framebuffer));
                fetch_count = 0;
                dm2_v1_runtime_set_viewport_asset_provider(
                    synthetic_viewport_asset_fetch, &fetch_count);
                CHECK(dm2_v1_runtime_render_frame(
                          0, 1, 1, framebuffer, 320, 320, 200) == 0,
                      "runtime renders a skproject DB0 door-record square");
                CHECK(dm2_v1_runtime_last_asset_door_panel_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_frame_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_button_count() == 1,
                      "runtime door record drives default button asset draw");
                dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
            } else {
                CHECK(0, "runtime door-record fixture loads");
            }
            if (replacement) {
                dm2_v1_dungeon_free(replacement);
                free(replacement);
            }
        }
        {
            uint8_t fixture[128];
            size_t fixture_size = build_skproject_custom_wall_button_fixture(
                fixture, sizeof(fixture));
            DM2_V1_DungeonData *replacement =
                (DM2_V1_DungeonData *)calloc(1, sizeof(*replacement));
            CHECK(fixture_size > 0 && replacement != NULL,
                  "runtime custom wall-button fixture allocates");
            if (replacement &&
                dm2_v1_dungeon_load(replacement, fixture, (int)fixture_size) == 0) {
                DM2_V1_DungeonData *old_dd =
                    (DM2_V1_DungeonData *)profile.dungeon_data;
                dm2_v1_dungeon_free(old_dd);
                free(old_dd);
                profile.dungeon_data = replacement;
                replacement = NULL;
                dm2_v1_runtime_set_position(0, 1, 1, 0);
                dm2_v1_runtime_set_outdoor(0);
                memset(framebuffer, 0, sizeof(framebuffer));
                memset(s_wall_button_pixels, 6, sizeof(s_wall_button_pixels));
                fetch_count = 0;
                dm2_v1_runtime_set_viewport_asset_provider(
                    synthetic_viewport_asset_fetch, &fetch_count);
                CHECK(dm2_v1_runtime_render_frame(
                          0, 1, 1, framebuffer, 320, 320, 200) == 0,
                      "runtime renders a skproject custom wall-button door square");
                CHECK(dm2_v1_runtime_last_asset_door_panel_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_frame_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_button_count() == 1,
                      "runtime text wall-gfx metadata drives custom button draw");
                dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
            } else {
                CHECK(0, "runtime custom wall-button fixture loads");
            }
            if (replacement) {
                dm2_v1_dungeon_free(replacement);
                free(replacement);
            }
        }
        {
            uint8_t fixture[144];
            static const uint8_t wall_gfx_list[4] = {
                0x10, 0x20, 0x2a, 0x30
            };
            size_t fixture_size = build_skproject_actuator_wall_button_fixture(
                fixture, sizeof(fixture));
            DM2_V1_DungeonData *replacement =
                (DM2_V1_DungeonData *)calloc(1, sizeof(*replacement));
            CHECK(fixture_size > 0 && replacement != NULL,
                  "runtime actuator wall-button fixture allocates");
            if (replacement &&
                dm2_v1_dungeon_load(replacement, fixture, (int)fixture_size) == 0) {
                DM2_V1_DungeonData *old_dd =
                    (DM2_V1_DungeonData *)profile.dungeon_data;
                dm2_v1_dungeon_free(old_dd);
                free(old_dd);
                profile.dungeon_data = replacement;
                replacement = NULL;
                dm2_v1_runtime_set_position(0, 1, 1, 0);
                CHECK(dm2_v1_runtime_set_map_wall_gfx_list(
                          wall_gfx_list, 4) == 0,
                      "runtime accepts a bounded map wall-gfx list");
                dm2_v1_runtime_set_outdoor(0);
                memset(framebuffer, 0, sizeof(framebuffer));
                memset(s_wall_button_pixels, 6, sizeof(s_wall_button_pixels));
                fetch_count = 0;
                dm2_v1_runtime_set_viewport_asset_provider(
                    synthetic_viewport_asset_fetch, &fetch_count);
                CHECK(dm2_v1_runtime_render_frame(
                          0, 1, 1, framebuffer, 320, 320, 200) == 0,
                      "runtime renders a skproject actuator custom-button door square");
                CHECK(dm2_v1_runtime_last_asset_door_panel_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_frame_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_button_count() == 1,
                      "runtime actuator wall-gfx list drives custom button draw");
                dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
                (void)dm2_v1_runtime_set_map_wall_gfx_list(NULL, 0);
            } else {
                CHECK(0, "runtime actuator wall-button fixture loads");
            }
            if (replacement) {
                dm2_v1_dungeon_free(replacement);
                free(replacement);
            }
        }
        {
            uint8_t fixture[160];
            size_t fixture_size =
                build_skproject_actuator_wall_button_map_list_fixture(
                    fixture, sizeof(fixture));
            DM2_V1_DungeonData *replacement =
                (DM2_V1_DungeonData *)calloc(1, sizeof(*replacement));
            CHECK(fixture_size > 0 && replacement != NULL,
                  "runtime map-list actuator wall-button fixture allocates");
            if (replacement &&
                dm2_v1_dungeon_load(replacement, fixture, (int)fixture_size) == 0) {
                DM2_V1_DungeonData *old_dd =
                    (DM2_V1_DungeonData *)profile.dungeon_data;
                dm2_v1_dungeon_free(old_dd);
                free(old_dd);
                profile.dungeon_data = replacement;
                replacement = NULL;
                dm2_v1_runtime_set_position(0, 1, 1, 0);
                dm2_v1_runtime_set_outdoor(0);
                memset(framebuffer, 0, sizeof(framebuffer));
                memset(s_wall_button_pixels, 6, sizeof(s_wall_button_pixels));
                fetch_count = 0;
                dm2_v1_runtime_set_viewport_asset_provider(
                    synthetic_viewport_asset_fetch, &fetch_count);
                CHECK(dm2_v1_runtime_render_frame(
                          0, 1, 1, framebuffer, 320, 320, 200) == 0,
                      "runtime renders map-list actuator custom-button door");
                CHECK(dm2_v1_runtime_last_asset_door_panel_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_frame_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_button_count() == 1,
                      "runtime auto-loads map wall-gfx list for actuator buttons");
                dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
            } else {
                CHECK(0, "runtime map-list actuator wall-button fixture loads");
            }
            if (replacement) {
                dm2_v1_dungeon_free(replacement);
                free(replacement);
            }
        }
    }

    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_get_tick_count() == 78,
          "first deterministic DM2 V1 runtime tick is observable");
    CHECK(dm2_v1_runtime_get_last_target_message() != NULL &&
          strstr(dm2_v1_runtime_get_last_target_message(),
                 "dungeon awakens") != NULL,
          "runtime tick applies timeline display-message target");
    CHECK(dm2_v1_runtime_get_party_x() >= 0 &&
          dm2_v1_runtime_get_party_y() >= 0 &&
          dm2_v1_runtime_get_party_dir() == 0,
          "first tick preserves the door-facing snapped party state");

    dm2_v1_boot_cleanup(&profile);
}

int main(void)
{
    printf("=== DM2 V1 Runtime Handoff Smoke Gate ===\n\n");
    test_first_tick_after_boot_profile_handoff();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: DM2 V1 boot/profile handoff reaches one deterministic runtime tick without claiming full playability");
        puts("sourceEvidence=SKULL.ASM T520/T560; ReDMCSB GAMELOOP.C lines 55-70; TOWNSGLB.H lines 1381-1388");
    }
    return failed == 0 ? 0 : 1;
}
