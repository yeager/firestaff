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

static int passed;
static int failed;
static uint8_t s_ceiling_pixels[16 * 8];
static uint8_t s_floor_pixels[16 * 8];
static uint8_t s_wall_pixels[16 * 8];
static uint8_t s_door_panel_pixels[16 * 8];
static uint8_t s_door_frame_pixels[16 * 8];
static uint8_t s_door_button_pixels[16 * 8];

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
    const size_t raw_map_base = thing_base + 4u;
    uint8_t *desc;
    uint16_t door_bits;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);
    buf[4] = 1;
    put16le(buf + 10, 1);
    put16le(buf + 12, 1);
    desc = buf + header_size;
    put16le(desc + 8, (uint16_t)((1u << 6) | (1u << 11)));
    put16le(buf + column_base, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x0000);
    put16le(buf + thing_base, 0xfffe);
    door_bits = (uint16_t)((1u << 6) | (1u << 11) | (1u << 5) | 1u);
    put16le(buf + thing_base + 2, door_bits);
    buf[raw_map_base + 0] = 0x20;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x90;
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
    }

    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_get_tick_count() == 78,
          "first deterministic DM2 V1 runtime tick is observable");
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
