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
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_game.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_world_model.h"
#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_session_fixture.h"
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
static uint8_t s_wall_pixels[320 * 200];
static uint8_t s_door_panel_pixels[16 * 8];
static uint8_t s_door_overlay_pixels[16 * 8];
static uint8_t s_door_frame_pixels[16 * 8];
static uint8_t s_door_button_pixels[16 * 8];
static uint8_t s_wall_button_pixels[16 * 8];
static uint8_t s_creature_pixels[16 * 8];
static uint8_t s_creature_wide_pixels[32 * 8];
static int s_creature_asset_w = 16;
static uint8_t s_item_pixels[16 * 8];
static uint8_t s_projectile_pixels[16 * 8];
static uint8_t s_hud_core_pixels[16 * 8];
static uint8_t s_hud_portrait_pixels[16 * 8];
static int s_last_door_panel_index;

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
    int scene_material_index = 0;
    int scene_material_field = 0;
    int wall_graphicsset_index = 0;
    int wall_field = 0;
    if (fetch_count) {
        ++*fetch_count;
    }
    if (dm2_v1_viewport_scene_material_graphic_address(
            gdat_index, &scene_material_index, &scene_material_field) &&
        scene_material_field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING) {
        if (out_pixels) *out_pixels = s_ceiling_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (dm2_v1_viewport_scene_material_graphic_address(
            gdat_index, &scene_material_index, &scene_material_field) &&
        scene_material_field == DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR) {
        if (out_pixels) *out_pixels = s_floor_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (dm2_v1_viewport_wall_graphic_address(
            gdat_index, &wall_graphicsset_index, &wall_field)) {
        if (out_pixels) *out_pixels = s_wall_pixels;
        if (out_w) *out_w = 320;
        if (out_h) *out_h = 200;
        if (out_stride) *out_stride = 320;
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
    if (gdat_index <= DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE &&
        (((DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index) >>
          DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff) >= 0x10 &&
        (((DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index) >>
          DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff) <= 0x15) {
        if (out_pixels) *out_pixels = s_item_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <= DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE &&
        DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE - gdat_index <
            (0x100 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT)) {
        if (out_pixels) {
            *out_pixels =
                s_creature_asset_w == 32 ? s_creature_wide_pixels :
                                            s_creature_pixels;
        }
        if (out_w) *out_w = s_creature_asset_w;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = s_creature_asset_w;
        return 0;
    }
    if (gdat_index <= DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE &&
        (((DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - gdat_index) >>
          DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) & 0xff) >= 0x0d &&
        (((DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - gdat_index) >>
          DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) & 0xff) <= 0x10) {
        if (out_pixels) *out_pixels = s_projectile_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <=
        DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE &&
        gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE) {
        if (out_pixels) *out_pixels = s_door_overlay_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <=
            DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE &&
        DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE - gdat_index <
            (0x100 << DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT)) {
        if (out_pixels) *out_pixels = s_door_overlay_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <=
        DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE &&
        DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE - gdat_index <
            (0x100 << DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT)) {
        s_last_door_panel_index = gdat_index;
        if (out_pixels) *out_pixels = s_door_panel_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <=
        DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE -
            DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FRONT &&
        DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - gdat_index < 0x04) {
        s_last_door_panel_index = gdat_index;
        if (out_pixels) *out_pixels = s_door_panel_pixels;
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
    if (gdat_index <= DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE &&
        DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE - gdat_index <
            (0x100 << DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_INDEX_SHIFT)) {
        if (out_pixels) *out_pixels = s_hud_portrait_pixels;
        if (out_w) *out_w = 16;
        if (out_h) *out_h = 8;
        if (out_stride) *out_stride = 16;
        return 0;
    }
    if (gdat_index <= DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE &&
        DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE - gdat_index < 0x100) {
        if (out_pixels) *out_pixels = s_hud_core_pixels;
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

static int set_door_state_preserve_tile(DM2_V1_DungeonData *d,
                                        int level,
                                        int x,
                                        int y,
                                        int state)
{
    int raw = dm2_v1_dungeon_get_tile_raw(d, level, x, y);
    if (raw < 0) return -1;
    return dm2_v1_dungeon_set_tile_raw(
        d, level, x, y, (uint16_t)((raw & ~0x0007u) | (state & 0x0007)));
}

static void clear_creature_pool_for_door_runtime_test(void)
{
    for (int i = 0; i < DM2_MAX_CREATURE_INSTANCES; ++i) {
        (void)dm2_v1_creature_deal_damage(i, 999999);
    }
    dm2_v1_runtime_tick();
    dm2_v1_creature_reset_ccm_tick_observer();
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

    if (cap < raw_map_base + 6u) return 0;
    memset(buf, 0, cap);
    buf[4] = 1;
    put16le(buf + 10, 1);
    put16le(buf + 12, 1);
    put16le(buf + 16, 1);
    desc = buf + header_size;
    put16le(desc + 2, (uint16_t)(1u << 8));
    put16le(desc + 8, (uint16_t)((1u << 6) | (1u << 11)));
    put16le(desc + 12, 2u);
    put16le(desc + 14, (uint16_t)((3u << 8) | (7u << 12)));
    put16le(buf + column_base, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x0800);
    put16le(buf + door_base, 0xfffe);
    door_bits = (uint16_t)((1u << 6) | (1u << 11) | (1u << 5) |
                           (2u << 1) | 1u);
    put16le(buf + door_base + 2, door_bits);
    put16le(buf + text_base, 0x0000);
    put16le(buf + text_base + 2, 0x0000);
    buf[raw_map_base + 0] = 0x20;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x90;
    buf[raw_map_base + 3] = 0x20;
    /* DME.h DoorDecorationGraphics() is the map-local byte list consumed
     * by DRAW_DOOR after its one-based DB0 OrnamentIndex. */
    buf[raw_map_base + 4] = 0x01;
    buf[raw_map_base + 5] = 0x0du;
    return raw_map_base + 6u;
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

static size_t build_skproject_creature_possession_fixture(uint8_t *buf,
                                                         size_t cap)
{
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + 4u;
    const size_t thing_base = sft_base + 2u;
    const size_t creature_base = thing_base;
    const size_t weapon_base = creature_base + 16u;
    const size_t junk_base = weapon_base + 4u;
    const size_t raw_map_base = junk_base + 4u;
    uint8_t *desc;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);
    buf[4] = 1;
    put16le(buf + 10, 1);
    put16le(buf + 20, 1);
    put16le(buf + 22, 1);
    put16le(buf + 32, 1);
    desc = buf + header_size;
    put16le(desc + 8, (uint16_t)((1u << 6) | (1u << 11)));
    put16le(buf + column_base, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x1000);
    put16le(buf + creature_base, 0xfffe);
    put16le(buf + creature_base + 2, 0x5400);
    buf[creature_base + 4] = 0x06;
    put16le(buf + weapon_base, 0xa800);
    put16le(buf + weapon_base + 2, 0x0022);
    put16le(buf + junk_base, 0xfffe);
    put16le(buf + junk_base + 2, 0x0033);
    buf[raw_map_base + 0] = 0x20;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x90;
    buf[raw_map_base + 3] = 0x20;
    return raw_map_base + 4u;
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
                                                      uint16_t flag,
                                                      uint8_t target_level,
                                                      uint8_t target_x,
                                                      uint8_t target_y)
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
    buf[actuator_base + 3] = target_level;
    put16le(buf + actuator_base + 4, flag);
    buf[actuator_base + 6] = target_x;
    buf[actuator_base + 7] = target_y;
    buf[raw_map_base + 0] = 0x10;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x20;
    buf[raw_map_base + 3] = 0x20;
    return raw_map_base + 4u;
}

static void test_first_tick_after_boot_profile_handoff(void)
{
    DM2_V1_BootViewportAssetEvidence evidence;
    DM2_V1_BootProfile profile;
    DM2_V1_GameState *state;
    DM2_V1_SessionState session;

    CHECK(dm2_v1_boot_viewport_asset_evidence(
              NULL, DM2_V1_VIEWPORT_GFX_FLOOR, &evidence) == 0 &&
              evidence.category == DM2_GDAT_CATEGORY_GRAPHICSSET &&
              evidence.entry_index == 0 &&
              evidence.field == DM2_GDAT_GFXSET_FLOOR,
          "viewport floor material resolves to its raw GDAT address before decode");
    CHECK(dm2_v1_boot_viewport_asset_evidence(
              NULL,
              dm2_v1_viewport_scene_material_graphic_index(
                  3, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR),
              &evidence) == 0 &&
              evidence.category == DM2_GDAT_CATEGORY_GRAPHICSSET &&
              evidence.entry_index == 3 &&
              evidence.field == DM2_GDAT_GFXSET_FLOOR,
          "active map graphics style selects its own GDAT floor material");
    {
        int graphicsset_index = -1;
        int wall_field = -1;
        int wall_gdat_index = dm2_v1_viewport_wall_graphic_index_for_graphicsset(
            3, DM2_SQ_D0L);
        int wall_address_ready = dm2_v1_viewport_wall_graphic_address(
            wall_gdat_index, &graphicsset_index, &wall_field);
        CHECK(wall_address_ready == 1 &&
                  graphicsset_index == 3 &&
                  wall_field == dm2_v1_viewport_wall_field_for_square(
                      DM2_SQ_D0L),
              "active map graphics style selects its own GDAT wall material");
    }
    CHECK(dm2_v1_boot_viewport_asset_evidence(
              NULL,
              dm2_v1_viewport_door_panel_graphic_index_for_record(
                  DM2_SQ_D0C, 7, 1),
              &evidence) == 0 &&
              evidence.category == DM2_GDAT_CATEGORY_DOORS &&
              evidence.entry_index == 7 && evidence.field == 0,
          "viewport door panel resolves map-local GDAT record before decode");
    CHECK(dm2_v1_boot_viewport_asset_evidence(
              NULL, dm2_v1_viewport_creature_graphic_index(0x12, 0),
              &evidence) == 0 &&
              evidence.category == DM2_GDAT_CATEGORY_CREATURES &&
              evidence.entry_index == 0x12 &&
              evidence.field == DM2_GDAT_IMG_MAP_CHIP,
          "viewport creature sprite resolves map-chip GDAT record before decode");
    CHECK(dm2_v1_boot_viewport_asset_evidence(
              NULL,
              dm2_v1_viewport_door_ornate_graphic_index(2, DM2_SQ_D0C),
              &evidence) == 0 &&
              evidence.category == DM2_GDAT_CATEGORY_DOOR_GFX &&
              evidence.entry_index == 2 && evidence.field == 0,
          "viewport door ornament resolves its GDAT overlay record before decode");
    CHECK(dm2_v1_boot_viewport_asset_evidence(
              NULL,
              dm2_v1_viewport_door_destroyed_mask_graphic_index(
                  7, DM2_SQ_D0C),
              &evidence) == 0 &&
              evidence.category == DM2_GDAT_CATEGORY_DOORS &&
              evidence.entry_index == 7 && evidence.field == 0,
          "viewport destroyed-door mask resolves its GDAT overlay record before decode");

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
    CHECK(state->party_x == 0 && state->party_y == 0 &&
          state->party_dir == 0 && state->current_level == 0,
          "fixture-only boot handoff does not invent a party snapshot");

    CHECK(dm2_v1_runtime_bind_boot_profile(NULL) == 0,
          "runtime boot-profile bind rejects NULL");
    {
        DM2_V1_StartupHostReceipt bind_receipt;
        CHECK(dm2_v1_runtime_bind_boot_profile_with_receipt(
                  NULL,
                  &bind_receipt) == 0 &&
                  bind_receipt.status_scope &&
                  strcmp(bind_receipt.status_scope, "BOOT") == 0 &&
                  bind_receipt.status &&
                  strcmp(bind_receipt.status,
                         "DM2 RUNTIME BIND FAILED") == 0,
              "runtime boot-profile bind receipt reports M11-ready failure");
    }
    CHECK(dm2_v1_runtime_bind_boot_profile(&profile) == 1,
          "runtime boot-profile bind initializes runtime state");
    {
        DM2_V1_StartupHostReceipt bind_receipt;
        CHECK(dm2_v1_runtime_bind_boot_profile_with_receipt(
                  &profile,
                  &bind_receipt) == 1 &&
                  bind_receipt.status_scope &&
                  strcmp(bind_receipt.status_scope, "BOOT") == 0 &&
                  bind_receipt.status &&
                  strcmp(bind_receipt.status, "DM2 RUNTIME READY") == 0,
              "runtime boot-profile bind receipt reports M11-ready success");
    }
    CHECK(dm2_v1_runtime_get_tick_count() == 0,
          "runtime tick counter starts at zero after handoff bind");
    CHECK(dm2_v1_runtime_has_dungeon_data() == 1,
          "runtime bind exposes the boot handoff dungeon data");
    CHECK(dm2_v1_runtime_get_party_x() == 0 &&
          dm2_v1_runtime_get_party_y() == 0 &&
          dm2_v1_runtime_get_party_dir() == 0,
          "runtime accessors preserve the unowned fixture party state");
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
              DM2_AI_DRAGOTH_MINION) < 0 &&
          dm2_v1_runtime_get_spawn_count() == 0,
          "runtime creature-generator rejects an unowned actuator payload");
    CHECK(dm2_v1_runtime_invoke_actuator(
              0, 0, 0, DM2_ACTUATOR_ITEM_GENERATOR, 0x1234u) < 0 &&
          dm2_v1_runtime_get_last_generated_object() == 0u,
          "runtime item-generator rejects an unowned actuator payload");
    CHECK(dm2_v1_runtime_invoke_actuator(
              0, 0, 0, DM2_ACTUATOR_MISSILE_SHOOTER,
              DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL) == -1 &&
          dm2_v1_runtime_get_last_projectile_slot() < 0 &&
          dm2_v1_runtime_get_projectile_actuator_count() == 0,
          "runtime missile-shooter rejects synthetic projectile state");
    {
        uint8_t fixture[128];
        size_t fixture_size = build_skproject_square_actuator_fixture(
            fixture, sizeof(fixture),
            (uint8_t)DM2_ACTUATOR_ITEM_GENERATOR, 0x4567u,
            0, 1, 1);
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
            CHECK(dm2_v1_runtime_invoke_square_actuators(0, 0, 0) == 0 &&
                  dm2_v1_runtime_get_actuator_count() == 0 &&
                  dm2_v1_runtime_get_last_generated_object() == 0u,
                  "runtime square-local DB3 fixture cannot infer an actuator transition");
            {
                int before_actuators = dm2_v1_runtime_get_actuator_count();
                dm2_v1_runtime_set_position(0, 0, 1, 0);
                dm2_v1_runtime_set_outdoor(1);
                CHECK(dm2_v1_runtime_move(0) == 0 &&
                      dm2_v1_runtime_get_party_x() == 0 &&
                      dm2_v1_runtime_get_party_y() == 0 &&
                      dm2_v1_runtime_get_actuator_count() ==
                          before_actuators &&
                      dm2_v1_runtime_get_last_generated_object() == 0u,
                      "runtime arrival cannot infer an actuator transition from a DB3 fixture");
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
    dm2_v1_test_session_fixture_new(&session);
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
        DM2_V1_QuicksaveReceipt quicksave;
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
        dm2_v1_boot_set_save_root(&profile, tmpdir);
        memset(&quicksave, 0, sizeof(quicksave));
        CHECK(dm2_v1_runtime_quicksave_boot_profile_with_receipt(
                  &profile,
                  &quicksave) == 0,
              "runtime quicksave refuses the unproven original writer");
        CHECK(quicksave.result == DM2_V1_QUICKSAVE_ORIGINAL_WRITER_REQUIRED &&
              !quicksave.session_valid && quicksave.save_path[0] == '\0',
              "runtime quicksave receipt exposes no Firestaff SKSave substitute");
        snprintf(slot_path, sizeof(slot_path), "%s/SKSave.dat", tmpdir);
        {
            FILE *blocked_save = fopen(slot_path, "rb");
            CHECK(blocked_save == NULL,
                  "runtime quicksave cannot create an unproven SKSave.dat");
            if (blocked_save) fclose(blocked_save);
        }
        snprintf(slot_path, sizeof(slot_path), "%s/SKSave.dat", tmpdir);
        (void)remove(slot_path);
        snprintf(slot_path, sizeof(slot_path), "%s/SKSave06.dat", tmpdir);
        (void)remove(slot_path);
        snprintf(slot_path, sizeof(slot_path), "%s/SKSave.bak", tmpdir);
        (void)remove(slot_path);
        (void)FS_RMDIR(tmpdir);
    }

    {
        uint8_t framebuffer[320 * 200];
        int fetch_count = 0;
        DM2_V1_RuntimeFrameOwnershipReceipt ownership;
        memset(s_ceiling_pixels, 12, sizeof(s_ceiling_pixels));
        memset(s_floor_pixels, 4, sizeof(s_floor_pixels));
        memset(s_wall_pixels, 9, sizeof(s_wall_pixels));
        memset(s_item_pixels, 6, sizeof(s_item_pixels));
        memset(s_hud_core_pixels, 12, sizeof(s_hud_core_pixels));
        memset(s_hud_portrait_pixels, 14, sizeof(s_hud_portrait_pixels));
        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_runtime_set_outdoor(0);
        dm2_v1_runtime_set_leader_hand_object(0u);
        dm2_v1_runtime_set_viewport_asset_provider(
            synthetic_viewport_asset_fetch, &fetch_count);
        CHECK(dm2_v1_runtime_render_frame(
                  dm2_v1_runtime_get_party_dir(),
                  dm2_v1_runtime_get_party_x(),
                  dm2_v1_runtime_get_party_y(),
                  framebuffer, 320, 320, 200) == 0,
              "runtime renders through an injected viewport asset provider");
        CHECK(fetch_count == 25,
              "runtime viewport provider receives ceiling, floor, wall and full HUD GDAT fetches");
        CHECK(dm2_v1_runtime_last_asset_floor_ceiling_count() == 2 &&
              dm2_v1_runtime_last_fallback_floor_ceiling_count() == 0,
              "runtime records asset-backed floor/ceiling draw counts");
        CHECK(dm2_v1_runtime_last_asset_wall_count() == 10 &&
              dm2_v1_runtime_last_fallback_wall_count() == 0,
              "runtime records asset-backed viewport-cell wall draw counts");
        CHECK(dm2_v1_runtime_last_asset_door_frame_count() == 0 &&
              dm2_v1_runtime_last_fallback_door_count() == 0,
              "runtime records no door-frame draw when no front door is visible");
        CHECK(dm2_v1_runtime_last_asset_carried_item_count() == 0 &&
              dm2_v1_runtime_last_fallback_carried_item_count() == 0,
              "runtime records no carried-item draw when leader hand is empty");
        CHECK(dm2_v1_runtime_last_asset_hud_portrait_count() == 4 &&
              dm2_v1_runtime_last_fallback_hud_portrait_count() == 0,
              "runtime records asset-backed HUD portrait draws");
        CHECK(dm2_v1_runtime_last_frame_ownership(&ownership) &&
              ownership.runtime_frame_owned &&
              ownership.gdat_provider_bound &&
              ownership.floor_ceiling_gdat_blits == 2 &&
              ownership.wall_gdat_blits == 10 &&
              ownership.gdat_wall_material_plan_consumed == 0 &&
              ownership.hud_core_gdat_blits == 9 &&
              ownership.hud_gdat_blits == 13 &&
              ownership.door_gdat_blits == 0 &&
              ownership.creature_gdat_blits == 0 &&
              ownership.item_gdat_blits == 0 &&
              ownership.projectile_gdat_blits == 0 &&
              ownership.total_runtime_gdat_blits == 25 &&
              ownership.total_runtime_fallback_draws == 0 &&
              ownership.full_gdat_frame_valid == 1 &&
              ownership.real_gdat_evidence_valid == 0 &&
              ownership.gdat_scene_control_ready == 0 &&
              ownership.gdat_scene_control_consumed == 0,
              "indoor runtime frame keeps provider-backed wall fetches distinct from boot-owned GDAT plan consumption");
        CHECK(framebuffer[0] != 0,
              "runtime asset-provider frame completes the shared viewport render pass");
        dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
    }

    {
        uint8_t framebuffer[320 * 200];
        int fetch_count = 0;
        DM2_V1_RuntimeFrameOwnershipReceipt ownership;

        memset(s_ceiling_pixels, 12, sizeof(s_ceiling_pixels));
        memset(s_floor_pixels, 4, sizeof(s_floor_pixels));
        memset(s_hud_core_pixels, 12, sizeof(s_hud_core_pixels));
        memset(s_hud_portrait_pixels, 14, sizeof(s_hud_portrait_pixels));
        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_runtime_set_outdoor(1);
        dm2_v1_runtime_set_leader_hand_object(0u);
        dm2_v1_runtime_set_viewport_asset_provider(
            synthetic_viewport_asset_fetch, &fetch_count);
        CHECK(dm2_v1_runtime_render_frame(
                  dm2_v1_runtime_get_party_dir(),
                  dm2_v1_runtime_get_party_x(),
                  dm2_v1_runtime_get_party_y(),
                  framebuffer, 320, 320, 200) == 0,
              "outdoor runtime routes active GRAPHICSSET sky and ground GDAT materials");
        CHECK(fetch_count >= 2 &&
              dm2_v1_runtime_last_asset_floor_ceiling_count() == 2 &&
              dm2_v1_runtime_last_fallback_floor_ceiling_count() == 0,
              "outdoor viewport fetches both real material planes without a generated fallback");
        CHECK(dm2_v1_runtime_last_frame_ownership(&ownership) &&
              ownership.is_outdoor == 1 &&
              ownership.outdoor_sky_gdat_blits == 1 &&
              ownership.outdoor_ground_gdat_blits == 1 &&
              ownership.gdat_scene_material_index == 0 &&
              ownership.gdat_scene_material_consumed == 2 &&
              ownership.wall_gdat_blits == 0 &&
              ownership.outdoor_gdat_frame_valid == 1 &&
              ownership.full_gdat_frame_valid == 1 &&
              ownership.valid == 1,
              "outdoor GDAT material route reaches the runtime host receipt");
        CHECK(framebuffer[40 * 320] == 12 && framebuffer[140 * 320] == 4,
              "outdoor scene pixels retain their GDAT material palette entries");
        dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
        dm2_v1_runtime_set_outdoor(0);
    }

    {
        uint8_t framebuffer[320 * 200];
        int fetch_count = 0;
        DM2_V1_RuntimeItemRenderReceipt item_receipt;
        DM2_V1_ViewportM11FrameReceipt m11_receipt;
        memset(s_ceiling_pixels, 12, sizeof(s_ceiling_pixels));
        memset(s_floor_pixels, 4, sizeof(s_floor_pixels));
        memset(s_wall_pixels, 9, sizeof(s_wall_pixels));
        memset(s_item_pixels, 6, sizeof(s_item_pixels));
        memset(s_projectile_pixels, 13, sizeof(s_projectile_pixels));
        memset(s_hud_portrait_pixels, 14, sizeof(s_hud_portrait_pixels));
        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_runtime_set_outdoor(0);
        dm2_v1_runtime_set_leader_hand_object(dm2_db_make_handle(10, 0x0055));
        dm2_v1_runtime_set_viewport_asset_provider(
            synthetic_viewport_asset_fetch, &fetch_count);
        CHECK(dm2_v1_runtime_render_frame(
                  dm2_v1_runtime_get_party_dir(),
                  dm2_v1_runtime_get_party_x(),
                  dm2_v1_runtime_get_party_y(),
                  framebuffer, 320, 320, 200) == 0,
              "runtime keeps the fixture provider viewport renderable");
        CHECK(fetch_count == 25,
              "runtime does not fetch an invented leader-hand map-chip frame");
        CHECK(dm2_v1_runtime_last_asset_carried_item_count() == 0 &&
              dm2_v1_runtime_last_fallback_carried_item_count() == 0,
              "runtime blocks the carried item without its exact source field");
        CHECK(dm2_v1_runtime_last_item_render_receipt(&item_receipt) == 0,
              "runtime publishes no carried-item receipt from fixture pixels");
        memset(&m11_receipt, 0, sizeof(m11_receipt));
        (void)dm2_v1_runtime_last_m11_frame_receipt(&m11_receipt);
        CHECK(m11_receipt.item_material_plan_required == 0 &&
                  m11_receipt.item_material_plan_consumed == 0 &&
                  m11_receipt.item_material_plan_command_count == 0 &&
                  m11_receipt.item_material_plan_hash == 0u,
              "runtime gives M11 no carried-item plan from fixture pixels");
        dm2_v1_runtime_set_leader_hand_object(0u);
        dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
    }

    {
        uint8_t framebuffer[320 * 200];
        int fetch_count = 0;
        int slot;
        DM2_V1_RuntimeCreatureRenderReceipt receipt;

        clear_creature_pool_for_door_runtime_test();
        dm2_v1_runtime_set_position(0, 1, 1, 0);
        dm2_v1_runtime_set_outdoor(0);
        slot = dm2_v1_creature_spawn(DM2_AI_CAVE_BAT, 1, 0, 0, 1, 8);
        dm2_v1_runtime_tick();
        memset(s_creature_pixels, 10, sizeof(s_creature_pixels));
        memset(s_creature_wide_pixels, 10, sizeof(s_creature_wide_pixels));
        memset(framebuffer, 0, sizeof(framebuffer));
        s_creature_asset_w = 32;
        dm2_v1_runtime_set_viewport_asset_provider(
            synthetic_viewport_asset_fetch, &fetch_count);
        CHECK(slot >= 0 &&
              dm2_v1_runtime_render_frame(
                  0, 1, 1, framebuffer, 320, 320, 200) == 0,
              "runtime advances an active CCM creature without a viewport failure");
        CHECK(dm2_v1_runtime_last_asset_creature_count() == 0 &&
              dm2_v1_runtime_last_fallback_creature_count() == 0 &&
              dm2_v1_runtime_last_creature_render_receipt(&receipt) == 0,
              "runtime suppresses a CCM creature without a source-owned DB4 record");
        CHECK(dm2_v1_creature_deal_damage(slot, 0x7fff) == 0,
              "runtime live creature can despawn through damage writeback");
        dm2_v1_runtime_tick();
        memset(framebuffer, 0, sizeof(framebuffer));
        CHECK(dm2_v1_runtime_render_frame(
                  0, 1, 1, framebuffer, 320, 320, 200) == 0 &&
              dm2_v1_runtime_last_asset_creature_count() == 0 &&
              dm2_v1_runtime_last_fallback_creature_count() == 0 &&
              dm2_v1_runtime_last_creature_render_receipt(&receipt) == 0,
              "runtime despawn removes the live creature from the GDAT render plan");
        s_creature_asset_w = 16;
        dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
        clear_creature_pool_for_door_runtime_test();
    }

    {
        uint8_t framebuffer[320 * 200];
        int slot;
        DM2_V1_RuntimeCreatureRenderReceipt receipt;

        clear_creature_pool_for_door_runtime_test();
        dm2_v1_runtime_set_position(0, 1, 1, 0);
        dm2_v1_runtime_set_outdoor(0);
        slot = dm2_v1_creature_spawn(DM2_AI_WOLF, 1, 0, 0, 2, 12);
        dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
        memset(framebuffer, 0, sizeof(framebuffer));
        CHECK(slot >= 0 &&
              dm2_v1_runtime_render_frame(
                  0, 1, 1, framebuffer, 320, 320, 200) == 0,
              "runtime advances active CCM creature without a GDAT provider");
        CHECK(dm2_v1_runtime_last_asset_creature_count() == 0 &&
              dm2_v1_runtime_last_fallback_creature_count() == 0 &&
              dm2_v1_runtime_last_creature_render_receipt(&receipt) == 0,
              "runtime never converts an unowned CCM creature into fallback art");
        clear_creature_pool_for_door_runtime_test();
    }

    {
        uint8_t fixture[144];
        size_t fixture_size = build_skproject_creature_possession_fixture(
            fixture, sizeof(fixture));
        DM2_V1_DungeonData *replacement =
            (DM2_V1_DungeonData *)calloc(1, sizeof(*replacement));
        uint8_t framebuffer[320 * 200];
        int fetch_count = 0;
        DM2_V1_RuntimeCreatureRenderReceipt receipt;
        DM2_V1_RuntimeItemRenderReceipt item_receipt;

        CHECK(fixture_size > 0 && replacement != NULL,
              "runtime creature-possession fixture allocates");
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
            memset(s_creature_pixels, 12, sizeof(s_creature_pixels));
            memset(s_item_pixels, 7, sizeof(s_item_pixels));
            memset(framebuffer, 0, sizeof(framebuffer));
            dm2_v1_runtime_set_viewport_asset_provider(
                synthetic_viewport_asset_fetch, &fetch_count);
            CHECK(dm2_v1_runtime_render_frame(
                      0, 1, 1, framebuffer, 320, 320, 200) == 0,
                  "runtime renders skproject creature possession chain");
            CHECK(fetch_count >= 14,
                  "runtime creature possession chain reaches item-map-chip fetch path");
            CHECK(dm2_v1_runtime_last_asset_creature_count() == 0 &&
                  dm2_v1_runtime_last_fallback_creature_count() == 0,
                  "runtime does not draw fixture DB4 through an unowned record chain");
            CHECK(dm2_v1_runtime_last_creature_render_receipt(&receipt) == 0,
                  "runtime withholds DB4 atlas receipt without direct G1 material ownership");
            CHECK(dm2_v1_runtime_last_asset_creature_possession_item_count() == 2 &&
                  dm2_v1_runtime_last_fallback_creature_possession_item_count() == 0,
                  "runtime records asset-backed creature possession item draws");
            CHECK(dm2_v1_runtime_last_item_render_receipt(&item_receipt) == 1 &&
                  item_receipt.source_kind == 2 &&
                  item_receipt.draw_order == 1 &&
                  item_receipt.frame_index == 0 &&
                  item_receipt.asset_blit_ready == 1 &&
                  item_receipt.fallback_drawn == 0 &&
                  item_receipt.asset_src_w == 16 &&
                  item_receipt.asset_src_h == 8 &&
                  item_receipt.asset_frame_count == 2 &&
                  item_receipt.atlas_frame_w == 8 &&
                  item_receipt.atlas_frame_h == 8 &&
                  item_receipt.asset_dst_rect.w > 0 &&
                  item_receipt.asset_dst_rect.h > 0,
                  "runtime creature-possession receipt exposes final item map-chip blit");
            dm2_v1_runtime_set_viewport_asset_provider(NULL, NULL);
        } else {
            CHECK(0, "runtime creature-possession fixture loads");
        }
        if (replacement) {
            dm2_v1_dungeon_free(replacement);
            free(replacement);
        }
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
        {
            int slot;
            DM2_V1_CreatureCCMTickObserver obs;
            const DM2_V1_CreatureInstance *inst;

            CHECK(dm2_v1_dungeon_set_tile_raw(
                      (DM2_V1_DungeonData *)profile.dungeon_data,
                      0, door_x, door_y, 4u) == 0,
                  "runtime seeds closed door for creature field tick");
            slot = dm2_v1_creature_spawn(0, door_x, door_y + 1, 0, 0, 8);
            dm2_v1_creature_reset_ccm_tick_observer();
            dm2_v1_runtime_tick();
            CHECK(slot >= 0 &&
                  dm2_v1_creature_last_ccm_tick(&obs) == 1 &&
                  obs.instance_id == slot &&
                  obs.field_door_valid == 1 &&
                  obs.field_blocks_movement == 1 &&
                  obs.field_moved == 0 &&
                  obs.field_door_open_pct == 0,
                  "runtime creature tick reads closed dungeon door and blocks writeback");
            inst = dm2_v1_creature_get_instance(slot);
            CHECK(inst != NULL &&
                  inst->world_x == door_x &&
                  inst->world_y == door_y + 1,
                  "runtime blocked creature remains before the door");

            CHECK(dm2_v1_dungeon_set_tile_raw(
                      (DM2_V1_DungeonData *)profile.dungeon_data,
                      0, door_x, door_y, 2u) == 0,
                  "runtime seeds half-open door for creature field tick");
            dm2_v1_creature_reset_ccm_tick_observer();
            dm2_v1_runtime_tick();
            CHECK(dm2_v1_creature_last_ccm_tick(&obs) == 1 &&
                  obs.instance_id == slot &&
                  obs.field_door_valid == 1 &&
                  obs.field_blocks_movement == 0 &&
                  obs.field_moved == 1 &&
                  obs.field_door_open_pct == 50,
                  "runtime creature tick writes through passable door and records render pct");
            inst = dm2_v1_creature_get_instance(slot);
            CHECK(inst != NULL &&
                  inst->world_x == door_x &&
                  inst->world_y == door_y,
                  "runtime creature world position writes back after passable door");
        }
        memset(s_ceiling_pixels, 12, sizeof(s_ceiling_pixels));
        memset(s_floor_pixels, 4, sizeof(s_floor_pixels));
        memset(s_wall_pixels, 9, sizeof(s_wall_pixels));
        memset(s_door_panel_pixels, 8, sizeof(s_door_panel_pixels));
        memset(s_door_overlay_pixels, 11, sizeof(s_door_overlay_pixels));
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
                DM2_V1_RuntimeDoorRenderReceipt door_receipt;
                dm2_v1_dungeon_free(old_dd);
                free(old_dd);
                profile.dungeon_data = replacement;
                replacement = NULL;
                dm2_v1_runtime_set_position(0, 1, 1, 0);
                dm2_v1_runtime_set_outdoor(0);
                CHECK(dm2_v1_dungeon_set_tile_raw(
                          (DM2_V1_DungeonData *)profile.dungeon_data,
                          0, 1, 0, 4u) == 0,
                      "runtime DB0 closed door seeds panel GDAT receipt");
                memset(framebuffer, 0, sizeof(framebuffer));
                fetch_count = 0;
                s_last_door_panel_index = 0;
                dm2_v1_runtime_set_viewport_asset_provider(
                    synthetic_viewport_asset_fetch, &fetch_count);
                CHECK(dm2_v1_runtime_render_frame(
                          0, 1, 1, framebuffer, 320, 320, 200) == 0,
                      "runtime renders a skproject DB0 door-record square");
                CHECK(dm2_v1_runtime_last_asset_door_panel_count() >= 0 &&
                      dm2_v1_runtime_last_asset_door_panel_count() <= 1 &&
                      dm2_v1_runtime_last_asset_door_frame_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_button_count() == 1,
                      "runtime door record drives default button asset draw");
                CHECK(s_last_door_panel_index ==
                          dm2_v1_viewport_door_panel_graphic_index_for_record(
                              DM2_SQ_D0C, 7, 1),
                      "runtime DB0 door type selects level door-set GDAT panel");
                CHECK(dm2_v1_runtime_last_door_render_receipt(&door_receipt) == 1 &&
                      door_receipt.view_square == DM2_SQ_D0C &&
                      door_receipt.skproject_cell == 0 &&
                      door_receipt.door_record_type == 1 &&
                      door_receipt.door_gfx_index == 7 &&
                      door_receipt.door_gfx_admitted == 1 &&
                      door_receipt.door_opening_dir == 1 &&
                      door_receipt.ornament_index == 2 &&
                      door_receipt.door_ornate_gfx_index == 0x0d &&
                      door_receipt.door_button == 1 &&
                      door_receipt.door_button_state == 1 &&
                      door_receipt.door_state == 4 &&
                      door_receipt.door_open_pct == 0 &&
                      door_receipt.panel_gdat_index ==
                          dm2_v1_viewport_door_panel_graphic_index_for_record(
                              DM2_SQ_D0C, 7, 1) &&
                      door_receipt.ornate_gdat_index ==
                          dm2_v1_viewport_door_ornate_graphic_index(0x0d, DM2_SQ_D0C) &&
                      door_receipt.destroyed_mask_gdat_index == 0 &&
                      door_receipt.frame_gdat_index ==
                          dm2_v1_viewport_door_frame_graphic_index_for_square(
                              DM2_SQ_D0C) &&
                      door_receipt.button_gdat_index ==
                          dm2_v1_viewport_door_button_graphic_index_for_state(1) &&
                      door_receipt.button_source_kind == 1 &&
                      door_receipt.wall_button_index == 0 &&
                      door_receipt.wall_button_field == 0 &&
                      door_receipt.panel_blit_ready == 1 &&
                      door_receipt.ornate_blit_ready == 1 &&
                      door_receipt.destroyed_mask_blit_ready == 0 &&
                      door_receipt.frame_blit_ready == 1 &&
                      door_receipt.button_blit_ready == 1 &&
                      door_receipt.skproject_material_expected_count == 4 &&
                      door_receipt.skproject_material_ready_count == 4 &&
                      door_receipt.skproject_material_drawn_count == 4 &&
                      door_receipt.skproject_material_chain_ready == 1 &&
                      door_receipt.skproject_material_chain_drawn == 1 &&
                      door_receipt.skproject_material_chain_hash != 0u &&
                      door_receipt.panel_rect.w > 0 &&
                      door_receipt.panel_rect.h > 0 &&
                      door_receipt.panel_visible_rect.w > 0 &&
                      door_receipt.panel_visible_rect.h > 0 &&
                      door_receipt.panel_asset_drawn == 1 &&
                      door_receipt.ornate_asset_drawn == 1 &&
                      door_receipt.destroyed_mask_asset_drawn == 0 &&
                      door_receipt.frame_asset_drawn == 1 &&
                      door_receipt.button_asset_drawn == 1 &&
                      door_receipt.panel_asset_src_w == 16 &&
                      door_receipt.panel_asset_src_h == 8 &&
                      door_receipt.ornate_asset_src_w == 16 &&
                      door_receipt.ornate_asset_src_h == 8 &&
                      door_receipt.frame_asset_src_w == 16 &&
                      door_receipt.frame_asset_src_h == 8 &&
                      door_receipt.button_asset_src_w == 16 &&
                      door_receipt.button_asset_src_h == 8 &&
                      door_receipt.panel_asset_dst_rect.w > 0 &&
                      door_receipt.panel_asset_dst_rect.h > 0 &&
                      door_receipt.ornate_asset_dst_rect.w ==
                          door_receipt.panel_rect.w &&
                      door_receipt.ornate_asset_dst_rect.h ==
                          door_receipt.panel_rect.h &&
                      door_receipt.overlay_rect.w == door_receipt.panel_rect.w &&
                      door_receipt.overlay_rect.h == door_receipt.panel_rect.h &&
                      door_receipt.frame_rect.w > 0 &&
                      door_receipt.frame_rect.h > 0 &&
                      door_receipt.button_rect.w > 0 &&
                      door_receipt.button_rect.h > 0,
                      "runtime DB0 closed door render receipt exposes GDAT table row and blit intent");
                CHECK(dm2_v1_runtime_last_asset_door_overlay_count() == 1,
                      "runtime DB0 ornate draws through door overlay asset");
                CHECK(set_door_state_preserve_tile(
                          (DM2_V1_DungeonData *)profile.dungeon_data,
                          0, 1, 0, 1) == 0 &&
                      dm2_v1_runtime_is_passable(0, 1, 0) == 1,
                      "runtime DB0 door state 1 remains party-passable");
                CHECK(set_door_state_preserve_tile(
                          (DM2_V1_DungeonData *)profile.dungeon_data,
                          0, 1, 0, 2) == 0 &&
                      dm2_v1_runtime_is_passable(0, 1, 0) == 0,
                      "runtime DB0 door state 2 blocks party movement");
                CHECK(set_door_state_preserve_tile(
                          (DM2_V1_DungeonData *)profile.dungeon_data,
                          0, 1, 0, 5) == 0 &&
                      dm2_v1_runtime_is_passable(0, 1, 0) == 1,
                      "runtime DB0 destroyed door state remains passable");
                {
                    int slot;
                    DM2_V1_CreatureCCMTickObserver obs;
                    const DM2_V1_CreatureInstance *inst;

                    clear_creature_pool_for_door_runtime_test();
                    CHECK(set_door_state_preserve_tile(
                              (DM2_V1_DungeonData *)profile.dungeon_data,
                              0, 1, 0, 3) == 0,
                          "runtime DB0 door state 3 seeds creature block");
                    slot = dm2_v1_creature_spawn(0, 1, 1, 0, 0, 8);
                    dm2_v1_creature_reset_ccm_tick_observer();
                    dm2_v1_runtime_tick();
                    CHECK(slot >= 0 &&
                          dm2_v1_creature_last_ccm_tick(&obs) == 1 &&
                          obs.instance_id == slot &&
                          obs.field_door_valid == 1 &&
                          obs.field_door_state == 3 &&
                          obs.field_blocks_movement == 1 &&
                          obs.field_door_open_pct == 25,
                          "runtime DB0 state 3 blocks creature and reports 25 pct open");
                    inst = dm2_v1_creature_get_instance(slot);
                    CHECK(inst != NULL &&
                          inst->world_x == 1 &&
                          inst->world_y == 1,
                          "runtime DB0 state 3 keeps creature before door");

                    CHECK(set_door_state_preserve_tile(
                              (DM2_V1_DungeonData *)profile.dungeon_data,
                              0, 1, 0, 1) == 0,
                          "runtime DB0 door state 1 seeds creature pass");
                    dm2_v1_creature_reset_ccm_tick_observer();
                    dm2_v1_runtime_tick();
                    CHECK(dm2_v1_creature_last_ccm_tick(&obs) == 1 &&
                          obs.instance_id == slot &&
                          obs.field_door_state == 1 &&
                          obs.field_blocks_movement == 0 &&
                          obs.field_moved == 1 &&
                          obs.field_door_open_pct == 75,
                          "runtime DB0 state 1 passes creature and reports 75 pct open");

                    CHECK(set_door_state_preserve_tile(
                              (DM2_V1_DungeonData *)profile.dungeon_data,
                              0, 1, 0, 5) == 0,
                          "runtime DB0 door state 5 seeds destroyed pass");
                    clear_creature_pool_for_door_runtime_test();
                    slot = dm2_v1_creature_spawn(0, 1, 1, 0, 0, 8);
                    dm2_v1_creature_reset_ccm_tick_observer();
                    dm2_v1_runtime_tick();
                    CHECK(slot >= 0 &&
                          dm2_v1_creature_last_ccm_tick(&obs) == 1 &&
                          obs.instance_id == slot &&
                          obs.field_door_state == 5 &&
                          obs.field_blocks_movement == 0 &&
                          obs.field_moved == 1 &&
                          obs.field_door_open_pct == 100,
                          "runtime DB0 destroyed door passes creature and reports fully open");
                }
                memset(framebuffer, 0, sizeof(framebuffer));
                fetch_count = 0;
                CHECK(set_door_state_preserve_tile(
                          (DM2_V1_DungeonData *)profile.dungeon_data,
                          0, 1, 0, 5) == 0,
                      "runtime DB0 destroyed door render state seeded");
                CHECK(dm2_v1_runtime_render_frame(
                          0, 1, 1, framebuffer, 320, 320, 200) == 0,
                      "runtime renders DB0 destroyed door state");
                CHECK(dm2_v1_runtime_last_asset_door_panel_count() == 0 &&
                      dm2_v1_runtime_last_asset_door_overlay_count() == 2 &&
                      dm2_v1_runtime_last_asset_door_frame_count() == 1,
                      "runtime DB0 destroyed door skips panel and draws overlay masks");
                CHECK(dm2_v1_runtime_last_door_render_receipt(&door_receipt) == 1 &&
                      door_receipt.door_state == 5 &&
                      door_receipt.door_open_pct == 100 &&
                      door_receipt.ornate_gdat_index ==
                          dm2_v1_viewport_door_ornate_graphic_index(0x0d, DM2_SQ_D0C) &&
                      door_receipt.destroyed_mask_gdat_index ==
                          dm2_v1_viewport_door_destroyed_mask_graphic_index(
                              7, DM2_SQ_D0C) &&
                      door_receipt.panel_blit_ready == 0 &&
                      door_receipt.ornate_blit_ready == 1 &&
                      door_receipt.destroyed_mask_blit_ready == 1 &&
                      door_receipt.frame_blit_ready == 1 &&
                      door_receipt.button_blit_ready == 1 &&
                      door_receipt.skproject_material_expected_count == 5 &&
                      door_receipt.skproject_material_ready_count == 4 &&
                      door_receipt.skproject_material_drawn_count == 4 &&
                      door_receipt.skproject_material_chain_ready == 0 &&
                      door_receipt.skproject_material_chain_drawn == 0 &&
                      door_receipt.skproject_material_chain_hash != 0u &&
                      door_receipt.panel_asset_drawn == 0 &&
                      door_receipt.ornate_asset_drawn == 1 &&
                      door_receipt.destroyed_mask_asset_drawn == 1 &&
                      door_receipt.frame_asset_drawn == 1 &&
                      door_receipt.button_asset_drawn == 1 &&
                      door_receipt.ornate_asset_src_w == 16 &&
                      door_receipt.ornate_asset_src_h == 8 &&
                      door_receipt.destroyed_mask_asset_src_w == 16 &&
                      door_receipt.destroyed_mask_asset_src_h == 8 &&
                      door_receipt.destroyed_mask_asset_dst_rect.w ==
                          door_receipt.panel_rect.w &&
                      door_receipt.destroyed_mask_asset_dst_rect.h ==
                          door_receipt.panel_rect.h &&
                      door_receipt.panel_visible_rect.h == 0 &&
                      door_receipt.overlay_rect.w == door_receipt.panel_rect.w &&
                      door_receipt.overlay_rect.h == door_receipt.panel_rect.h,
                      "runtime DB0 destroyed door render receipt exposes mask row and blit intent");
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
                CHECK(dm2_v1_runtime_last_asset_door_panel_count() >= 0 &&
                      dm2_v1_runtime_last_asset_door_panel_count() <= 1 &&
                      dm2_v1_runtime_last_asset_door_frame_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_button_count() == 1,
                      "runtime text wall-gfx metadata drives custom button draw");
                {
                    DM2_V1_RuntimeDoorRenderReceipt door_receipt;
                    CHECK(dm2_v1_runtime_last_door_render_receipt(
                              &door_receipt) == 1 &&
                          door_receipt.button_source_kind == 2 &&
                          door_receipt.wall_button_index == 0x2a &&
                          door_receipt.wall_button_field > 0 &&
                          door_receipt.button_gdat_index ==
                              dm2_v1_viewport_wall_button_graphic_index(
                                  door_receipt.wall_button_index,
                                  door_receipt.wall_button_field) &&
                          door_receipt.button_asset_drawn == 1,
                          "runtime custom wall-button receipt exposes wall-gfx source row");
                }
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
                CHECK(dm2_v1_runtime_last_asset_door_panel_count() >= 0 &&
                      dm2_v1_runtime_last_asset_door_panel_count() <= 1 &&
                      dm2_v1_runtime_last_asset_door_frame_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_button_count() == 1,
                      "runtime actuator wall-gfx list drives custom button draw");
                {
                    DM2_V1_RuntimeDoorRenderReceipt door_receipt;
                    CHECK(dm2_v1_runtime_last_door_render_receipt(
                              &door_receipt) == 1 &&
                          door_receipt.button_source_kind == 2 &&
                          door_receipt.wall_button_index == 0x2a &&
                          door_receipt.wall_button_field > 0 &&
                          door_receipt.button_gdat_index ==
                              dm2_v1_viewport_wall_button_graphic_index(
                                  door_receipt.wall_button_index,
                                  door_receipt.wall_button_field) &&
                          door_receipt.button_asset_drawn == 1,
                          "runtime actuator custom-button receipt exposes wall-gfx list row");
                }
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
                CHECK(dm2_v1_runtime_last_asset_door_panel_count() >= 0 &&
                      dm2_v1_runtime_last_asset_door_panel_count() <= 1 &&
                      dm2_v1_runtime_last_asset_door_frame_count() == 1 &&
                      dm2_v1_runtime_last_asset_door_button_count() == 1,
                      "runtime auto-loads map wall-gfx list for actuator buttons");
                {
                    DM2_V1_RuntimeDoorRenderReceipt door_receipt;
                    CHECK(dm2_v1_runtime_last_door_render_receipt(
                              &door_receipt) == 1 &&
                          door_receipt.button_source_kind == 2 &&
                          door_receipt.wall_button_index == 0x2a &&
                          door_receipt.wall_button_field > 0 &&
                          door_receipt.button_gdat_index ==
                              dm2_v1_viewport_wall_button_graphic_index(
                                  door_receipt.wall_button_index,
                                  door_receipt.wall_button_field) &&
                          door_receipt.button_asset_drawn == 1,
                          "runtime map-list custom-button receipt exposes auto-loaded wall-gfx row");
                }
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

    {
        int tick_before = dm2_v1_runtime_get_tick_count();
        const char *message_before = dm2_v1_runtime_get_last_target_message();
        char message_before_copy[160];
        if (message_before) {
            snprintf(message_before_copy, sizeof(message_before_copy), "%s",
                     message_before);
        } else {
            message_before_copy[0] = '\0';
        }
        dm2_v1_runtime_tick();
        CHECK(dm2_v1_runtime_get_tick_count() == tick_before + 1,
              "deterministic DM2 V1 runtime tick advances by one");
        CHECK((message_before == NULL &&
               dm2_v1_runtime_get_last_target_message() == NULL) ||
              (message_before != NULL &&
               dm2_v1_runtime_get_last_target_message() != NULL &&
               strcmp(dm2_v1_runtime_get_last_target_message(),
                      message_before_copy) == 0),
              "runtime tick does not fabricate a display message");
    }
    CHECK(dm2_v1_runtime_get_party_x() >= 0 &&
          dm2_v1_runtime_get_party_y() >= 0 &&
          dm2_v1_runtime_get_party_dir() == 0,
          "first tick preserves the door-facing snapped party state");

    {
        uint8_t framebuffer[320 * 200];
        DM2_V1_DoorRender direct_door;
        DM2_V1_DoorAssetBlit direct_door_blit;
        DM2_V1_CreatureRender direct_creature;
        DM2_V1_CreatureAssetBlit direct_creature_blit;
        DM2_V1_ItemRender direct_item;
        DM2_V1_ItemAssetBlit direct_item_blit;
        DM2_V1_ProjectileRender direct_projectile;
        DM2_V1_ProjectileAssetBlit direct_blit;
        uint32_t direct_seed = 0x12345678u;
        memset(s_ceiling_pixels, 12, sizeof(s_ceiling_pixels));
        memset(s_floor_pixels, 4, sizeof(s_floor_pixels));
        memset(s_wall_pixels, 9, sizeof(s_wall_pixels));
        memset(s_projectile_pixels, 13, sizeof(s_projectile_pixels));
        memset(framebuffer, 0, sizeof(framebuffer));
        memset(&direct_door, 0, sizeof(direct_door));
        memset(&direct_door_blit, 0, sizeof(direct_door_blit));
        memset(&direct_creature, 0, sizeof(direct_creature));
        memset(&direct_creature_blit, 0, sizeof(direct_creature_blit));
        memset(&direct_item, 0, sizeof(direct_item));
        memset(&direct_item_blit, 0, sizeof(direct_item_blit));
        memset(&direct_projectile, 0, sizeof(direct_projectile));
        memset(&direct_blit, 0, sizeof(direct_blit));
        direct_door.panel_gdat_index =
            dm2_v1_viewport_door_panel_graphic_index_for_square(DM2_SQ_D0C);
        direct_door.frame_gdat_index =
            dm2_v1_viewport_door_frame_graphic_index_for_square(DM2_SQ_D1C);
        direct_door.button_gdat_index =
            dm2_v1_viewport_door_button_graphic_index_for_state(1);
        direct_door.panel_rect =
            (DM2_V1_ViewportRect){ 80, 0, 160, 135 };
        direct_door.panel_visible_rect =
            (DM2_V1_ViewportRect){ 80, 33, 160, 102 };
        direct_door.frame_rect =
            (DM2_V1_ViewportRect){ 60, 9, 104, 110 };
        direct_door.button_rect =
            (DM2_V1_ViewportRect){ 142, 57, 12, 14 };
        CHECK(dm2_v1_viewport_door_panel_asset_blit(&direct_door,
                                                    64,
                                                    64,
                                                    80,
                                                    &direct_door_blit) == 1,
              "door panel asset blit contract builds");
        CHECK(direct_door_blit.src_rect.x == 0 &&
              direct_door_blit.src_rect.y == 15 &&
              direct_door_blit.src_rect.w == 64 &&
              direct_door_blit.src_rect.h == 49 &&
              direct_door_blit.dst_rect.y == 33 &&
              direct_door_blit.dst_rect.h == 102,
              "door panel asset blit owns open-percent source clipping");
        CHECK(direct_door_blit.src_stride == 80 &&
              direct_door_blit.transparent_color == 10,
              "door panel asset blit owns material and stride");
        CHECK(dm2_v1_viewport_door_frame_asset_blit(&direct_door,
                                                    40,
                                                    48,
                                                    0,
                                                    &direct_door_blit) == 1,
              "door frame asset blit contract builds");
        CHECK(direct_door_blit.src_rect.w == 40 &&
              direct_door_blit.src_rect.h == 48 &&
              direct_door_blit.dst_rect.x == 60 &&
              direct_door_blit.dst_rect.y == 9 &&
              direct_door_blit.dst_rect.w == 104 &&
              direct_door_blit.dst_rect.h == 110 &&
              direct_door_blit.src_stride == 40,
              "door frame asset blit owns full-source destination");
        CHECK(dm2_v1_viewport_door_button_asset_blit(&direct_door,
                                                     11,
                                                     13,
                                                     0,
                                                     &direct_door_blit) == 1,
              "door button asset blit contract builds");
        CHECK(direct_door_blit.src_rect.w == 11 &&
              direct_door_blit.src_rect.h == 13 &&
              direct_door_blit.dst_rect.x == 142 &&
              direct_door_blit.dst_rect.y == 57 &&
              direct_door_blit.dst_rect.w == 12 &&
              direct_door_blit.dst_rect.h == 14,
              "door button asset blit owns full-source destination");
        direct_creature.gdat_index = dm2_v1_viewport_creature_graphic_index(
            0x12, 0);
        direct_creature.frame_index = 1;
        direct_creature.direction = 1;
        direct_creature.depth = 1;
        direct_creature.center_x = 144;
        direct_creature.center_y = 92;
        CHECK(dm2_v1_viewport_creature_asset_blit(&direct_creature,
                                                  32,
                                                  8,
                                                  32,
                                                  0,
                                                  &direct_creature_blit) == 1,
              "creature asset blit contract builds");
        CHECK(direct_creature_blit.dst_rect.x == 140 &&
              direct_creature_blit.dst_rect.y == 88 &&
              direct_creature_blit.dst_rect.w == 8 &&
              direct_creature_blit.dst_rect.h == 8,
              "creature asset blit owns scaled destination");
        CHECK(direct_creature_blit.frame_x == 8 &&
              direct_creature_blit.frame_y == 0 &&
              direct_creature_blit.frame_w == 8 &&
              direct_creature_blit.frame_h == 8,
              "creature asset blit owns directional source frame");
        CHECK(direct_creature_blit.transparent_color == 10 &&
              direct_creature_blit.render_frame == 1,
              "creature asset blit owns material and render frame");
        direct_item.gdat_index =
            dm2_v1_viewport_item_graphic_index(0x15, 0x22, 1);
        direct_item.frame_index = 1;
        direct_item.depth = 2;
        direct_item.center_x = 120;
        direct_item.center_y = 90;
        direct_item.flip_mirror = 1;
        CHECK(dm2_v1_viewport_item_asset_blit(&direct_item,
                                              24,
                                              8,
                                              24,
                                              0,
                                              4,
                                              32,
                                              &direct_item_blit) == 1,
              "item asset blit contract builds");
        CHECK(direct_item_blit.dst_rect.x == 118 &&
              direct_item_blit.dst_rect.y == 88 &&
              direct_item_blit.dst_rect.w == 5 &&
              direct_item_blit.dst_rect.h == 5,
              "item asset blit owns scaled destination");
        CHECK(direct_item_blit.frame_x == 8 &&
              direct_item_blit.frame_y == 0 &&
              direct_item_blit.frame_w == 8 &&
              direct_item_blit.frame_h == 8,
              "item asset blit owns source frame");
        CHECK(direct_item_blit.transparent_color == 10 &&
              direct_item_blit.flip_mirror == 1 &&
              direct_item_blit.render_frame == 1,
              "item asset blit owns material and flip");
        direct_projectile.gdat_index =
            dm2_v1_viewport_projectile_graphic_index(
                0x0d, DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL, 0);
        direct_projectile.frame_index = 0;
        direct_projectile.direction = 0;
        direct_projectile.object_direction = 0;
        direct_projectile.frame_class =
            DM2_V1_PROJECTILE_FRAME_CLASS_FRONT_ONLY;
        direct_projectile.render_kind = DM2_V1_PROJECTILE_RENDER_MISSILE;
        direct_projectile.depth = 1;
        direct_projectile.center_x = 112;
        direct_projectile.center_y = 84;
        CHECK(dm2_v1_viewport_projectile_asset_blit(
                  &direct_projectile,
                  16, 8, 16, 0, 77, &direct_seed, &direct_blit) == 1,
              "projectile asset blit contract builds");
        CHECK(direct_blit.dst_rect.x == 109 &&
              direct_blit.dst_rect.y == 81 &&
              direct_blit.dst_rect.w == 6 &&
              direct_blit.dst_rect.h == 6,
              "projectile asset blit owns scaled destination");
        CHECK(direct_blit.frame_x == 0 &&
              direct_blit.frame_y == 0 &&
              direct_blit.frame_w == 8 &&
              direct_blit.frame_h == 8,
              "projectile asset blit owns source frame");
        CHECK(direct_blit.transparent_color == 10 &&
              direct_blit.flip_mirror == 0,
              "projectile asset blit owns material and flip");
    }

    dm2_v1_boot_cleanup(&profile);
}

/*
 * test_door_step_timer_wiring — DM2-003 follow-up round 24.
 *
 * Proves the source-order 0x01 timer (DM2_STEP_DOOR) reaches the runtime
 * dispatcher, mutates the dungeon grid one state per tick, and re-queues
 * subsequent steps until the door reaches OPEN or CLOSED.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4041 (0x01 dispatch)
 *         skproject/SKULLWIN/c_tim_proc.cpp:127+   (DM2_STEP_DOOR)
 *         ReDMCSB TIMELINE.C:750-810               (door state transitions)
 */
static void test_door_step_timer_wiring(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_DungeonData *old_dd;
    DM2_V1_SourceTimer timer;
    DM2_V1_RuntimeDoorStepReceipt receipt;
    uint8_t fixture[128];
    size_t fixture_size;

    make_synthetic_verified_profile(&profile);
    CHECK(dm2_v1_boot_enter_game(&profile) == 0,
          "door-step wiring synthetic profile enters game");

    fixture_size = build_skproject_door_fixture(fixture, sizeof(fixture));
    dungeon = (DM2_V1_DungeonData *)calloc(1, sizeof(*dungeon));
    CHECK(fixture_size > 0 && dungeon != NULL,
          "door-step wiring fixture allocates");

    if (dungeon == NULL ||
        dm2_v1_dungeon_load(dungeon, fixture, (int)fixture_size) != 0) {
        CHECK(0, "door-step wiring fixture loads");
        free(dungeon);
        dm2_v1_boot_cleanup(&profile);
        return;
    }

    old_dd = (DM2_V1_DungeonData *)profile.dungeon_data;
    dm2_v1_dungeon_free(old_dd);
    free(old_dd);
    profile.dungeon_data = dungeon;

    dm2_v1_runtime_init(&profile);
    dm2_v1_runtime_set_position(0, 1, 1, 0);
    dm2_v1_runtime_set_outdoor(0);

    CHECK(dm2_v1_dungeon_set_tile_raw(dungeon, 0, 1, 0,
                                      DM2_DOOR_STATE_CLOSED) == 0,
          "door-step wiring seeds CLOSED door at (1,0)");
    CHECK(dm2_v1_runtime_get_door_state(0, 1, 0) == DM2_DOOR_STATE_CLOSED,
          "door-step wiring reads seeded CLOSED state");

    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = 0; /* due at tick 0, map 0 */
    timer.type = DM2_V1_TIMER_STEP_DOOR;
    timer.value_a = (int16_t)((1 & 0xff) | ((0 & 0xff) << 8));
    timer.value_b = DM2_DOOR_TOGGLE_DIR_OPEN;

    CHECK(dm2_v1_runtime_enqueue_source_timer(&timer, 0) ==
              DM2_V1_SOURCE_TIMER_OK,
          "door-step wiring enqueues 0x01 OPEN timer");

    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_get_door_state(0, 1, 0) ==
              DM2_DOOR_STATE_CLOSED_THREE_QUARTER &&
          dm2_v1_runtime_door_step_receipt(&receipt) == 1 &&
          receipt.valid && receipt.timers == 1 &&
          receipt.mutations == 1 && receipt.requeues == 1,
          "first tick steps CLOSED door toward OPEN and requeues");

    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_get_door_state(0, 1, 0) ==
              DM2_DOOR_STATE_CLOSED_HALF &&
          dm2_v1_runtime_door_step_receipt(&receipt) == 1 &&
          receipt.valid && receipt.timers == 2 &&
          receipt.mutations == 2 && receipt.requeues == 2,
          "second tick steps door to HALF and requeues");

    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_get_door_state(0, 1, 0) ==
              DM2_DOOR_STATE_CLOSED_ONE_FOURTH &&
          dm2_v1_runtime_door_step_receipt(&receipt) == 1 &&
          receipt.valid && receipt.timers == 3 &&
          receipt.mutations == 3 && receipt.requeues == 3,
          "third tick steps door to ONE_FOURTH and requeues");

    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_get_door_state(0, 1, 0) == DM2_DOOR_STATE_OPEN &&
          dm2_v1_runtime_door_step_receipt(&receipt) == 1 &&
          receipt.valid && receipt.timers == 4 &&
          receipt.mutations == 4 && receipt.requeues == 3,
          "fourth tick reaches OPEN without requeue");

    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_get_door_state(0, 1, 0) == DM2_DOOR_STATE_OPEN &&
          dm2_v1_runtime_door_step_receipt(&receipt) == 1 &&
          receipt.valid && receipt.timers == 4 &&
          receipt.mutations == 4 && receipt.requeues == 3,
          "no further steps once OPEN and no pending timer");

    dm2_v1_boot_cleanup(&profile);
}

static void test_actuator_tile_subdispatch_wiring(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_DungeonData *old_dd;
    DM2_V1_SourceTimer timer;
    DM2_V1_RuntimeActuatorTileReceipt receipt;
    uint8_t fixture[128];
    size_t fixture_size;

    make_synthetic_verified_profile(&profile);
    CHECK(dm2_v1_boot_enter_game(&profile) == 0,
          "actuator-tile wiring synthetic profile enters game");

    fixture_size = build_skproject_door_fixture(fixture, sizeof(fixture));
    dungeon = (DM2_V1_DungeonData *)calloc(1, sizeof(*dungeon));
    CHECK(fixture_size > 0 && dungeon != NULL,
          "actuator-tile wiring fixture allocates");

    if (dungeon == NULL ||
        dm2_v1_dungeon_load(dungeon, fixture, (int)fixture_size) != 0) {
        CHECK(0, "actuator-tile wiring fixture loads");
        free(dungeon);
        dm2_v1_boot_cleanup(&profile);
        return;
    }

    old_dd = (DM2_V1_DungeonData *)profile.dungeon_data;
    dm2_v1_dungeon_free(old_dd);
    free(old_dd);
    profile.dungeon_data = dungeon;

    dm2_v1_runtime_init(&profile);
    dm2_v1_runtime_set_position(0, 1, 1, 0);
    dm2_v1_runtime_set_outdoor(0);

    /* Seed a class-2 pitfall cell at (0,0) starting as FLOOR. */
    CHECK(dm2_v1_dungeon_set_tile_raw(dungeon, 0, 0, 0, 0x40) == 0,
          "actuator-tile wiring seeds class-2 FLOOR pitfall at (0,0)");
    /* Seed a class-4 door cell at (1,0) starting CLOSED. */
    CHECK(dm2_v1_dungeon_set_tile_raw(dungeon, 0, 1, 0, 0x84) == 0,
          "actuator-tile wiring seeds class-4 CLOSED door at (1,0)");

    /* Enqueue 0x04 timer to open the pit at (0,0). */
    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = 0;
    timer.type = DM2_V1_TIMER_ACTUATE_TILE;
    timer.value_a = (int16_t)((0 & 0xff) | ((0 & 0xff) << 8));
    timer.value_b = 1; /* open pit */
    CHECK(dm2_v1_runtime_enqueue_source_timer(&timer, 0) ==
              DM2_V1_SOURCE_TIMER_OK,
          "actuator-tile wiring enqueues class-2 OPEN pit timer");

    /* Enqueue 0x04 timer to open the door at (1,0). */
    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = 0;
    timer.type = DM2_V1_TIMER_ACTUATE_TILE;
    timer.value_a = (int16_t)((1 & 0xff) | ((0 & 0xff) << 8));
    timer.value_b = DM2_DOOR_TOGGLE_DIR_OPEN;
    CHECK(dm2_v1_runtime_enqueue_source_timer(&timer, 1) ==
              DM2_V1_SOURCE_TIMER_OK,
          "actuator-tile wiring enqueues class-4 OPEN door timer");

    dm2_v1_runtime_tick();

    CHECK((dm2_v1_dungeon_get_tile_raw(dungeon, 0, 0, 0) & 0x1f) ==
              DM2_SQUARE_PIT,
          "class-2 pitfall timer toggles FLOOR to PIT");
    /* For byte-square maps the class lives in bits 5-7 and the door state
     * in bits 0-2.  After one OPEN step from CLOSED the raw byte is
     * 0x80 | CLOSED_THREE_QUARTER = 0x83.  dm2_v1_runtime_get_door_state
     * only recognizes sentinel square-type values, so verify the raw state
     * directly. */
    CHECK((dm2_v1_dungeon_get_tile_raw(dungeon, 0, 1, 0) & 0x07) ==
              DM2_DOOR_STATE_CLOSED_THREE_QUARTER,
          "class-4 door timer steps CLOSED door one tick toward OPEN");
    CHECK(dm2_v1_runtime_actuator_tile_receipt(&receipt) == 1 &&
              receipt.valid &&
              receipt.timers == 2 &&
              receipt.pitfall == 1 &&
              receipt.door == 1 &&
              receipt.pitfall_rejected == 0 &&
              receipt.door_rejected == 0,
          "actuator-tile receipt records class-2 and class-4 mutations");

    dm2_v1_boot_cleanup(&profile);
}

int main(void)
{
    printf("=== DM2 V1 Runtime Handoff Smoke Gate ===\n\n");
    test_first_tick_after_boot_profile_handoff();
    test_door_step_timer_wiring();
    test_actuator_tile_subdispatch_wiring();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: DM2 V1 boot/profile handoff reaches one deterministic runtime tick without claiming full playability");
        puts("sourceEvidence=SKULL.ASM T520/T560; ReDMCSB GAMELOOP.C lines 55-70; TOWNSGLB.H lines 1381-1388");
    }
    return failed == 0 ? 0 : 1;
}
