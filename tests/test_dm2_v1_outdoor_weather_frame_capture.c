/* Real-data outdoor frame capture for DM2 V1 weather overlays.
 *
 * Proves that the renderer consumes the live DistantEnvironment slots produced
 * by the runtime weather tick and that M11 accepts the resulting frame.  This
 * closes the DM2-011 cycle-6 gap: slot production was wired, but no test had
 * shown real GDAT weather pixels reaching the framebuffer and the M11 gate.
 *
 * Source-locks:
 *   skproject/SKULLWIN/c_weather.cpp DM2_UPDATE_WEATHER (0x54 timer + arg==0)
 *   skproject/SKULLWIN/c_bkgrnd.cpp ENVIRONMENT_DRAW_DISTANT_ELEMENT
 *   skproject/SKWIN/SkWinCore.cpp QUERY_TEMP_PICST / QUERY_GDAT_SUMMARY_IMAGE
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_update_weather_pc34_compat.h"
#include "dm2_v1_weather_gdat.h"
#include "m11_dm2_runtime_frame_receipt_gate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("PASS: %s\n", msg); } \
    else { ++failed; printf("FAIL: %s\n", msg); } \
} while (0)

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (!path || !out || !out_size) return 0;
    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

static int load_canonical_files(uint8_t **graphics, size_t *graphics_size,
                                uint8_t **dungeon, size_t *dungeon_size,
                                char *boot_root, size_t boot_root_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    char fallback[1024];
    char graphics_path[1100];
    char dungeon_path[1100];

    if (root && root[0]) {
        snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
        snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
        snprintf(boot_root, boot_root_size, "%s/..", root);
    } else if (home && home[0]) {
        snprintf(fallback, sizeof(fallback), "%s/.firestaff/data/dm2/data", home);
        snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat",
                 fallback);
        snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", fallback);
        snprintf(boot_root, boot_root_size, "%s/.firestaff/data/dm2", home);
    } else {
        return 0;
    }
    return read_file(graphics_path, graphics, graphics_size) &&
        read_file(dungeon_path, dungeon, dungeon_size);
}

/* Find a level whose MapGraphicsStyle has a complete weather GDAT receipt.
 * The runtime will use this style when dm2_v1_runtime_set_position is called. */
static int find_weather_level(const DM2_V1_AssetLoader *loader,
                              const DM2_V1_DungeonData *dungeon)
{
    for (int level = 0; level < dungeon->level_count; ++level) {
        int style = dm2_v1_dungeon_get_map_graphics_style(dungeon, level);
        DM2_V1_WeatherGdatReceipt weather;
        if (style < 0 || style > 0xff) continue;
        memset(&weather, 0, sizeof(weather));
        if (dm2_v1_weather_gdat_receipt(loader, (uint8_t)style, &weather) &&
            weather.valid && weather.material_mask != 0u) {
            return level;
        }
    }
    return -1;
}

static DM2_V1_UpdateWeatherState make_storm_state(void)
{
    DM2_V1_UpdateWeatherState s;
    memset(&s, 0, sizeof(s));
    s.zone_index = 1;         /* table1d6b76[4*1+0x70] == 1: weather allowed */
    s.retry = 0;
    s.pattern_row = 1;
    s.step = 4;
    s.intensity = 0x90;       /* cloud_state = 0x90 -> cloud cmd 0x69 */
    s.rain_counter = (int8_t)0xd0; /* rain cmd 0x6c */
    s.day_tick = 0x70000000;  /* no rollover */
    s.clouds_enabled = 1;
    s.rain_enabled = 1;
    s.lightning_enabled = 0;  /* keep the test deterministic: no bolt path */
    return s;
}

int main(void)
{
    uint8_t *graphics = NULL;
    uint8_t *dungeon_bytes = NULL;
    size_t graphics_size = 0u;
    size_t dungeon_size = 0u;
    char boot_root[1024];
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData *dungeon = NULL;
    DM2_V1_BootProfile boot;
    int weather_level;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_BootRuntimeRenderReceipt render_receipt;
    DM2_V1_RuntimeFrameOwnershipReceipt ownership;
    DM2_V1_ViewportM11FrameReceipt m11_receipt;

    passed = 0;
    failed = 0;

    if (!load_canonical_files(&graphics, &graphics_size,
                              &dungeon_bytes, &dungeon_size,
                              boot_root, sizeof(boot_root))) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    dungeon = (DM2_V1_DungeonData *)calloc(1u, sizeof(*dungeon));
    if (!dungeon ||
        dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        dm2_v1_dungeon_load(dungeon, dungeon_bytes, (int)dungeon_size) != 0) {
        fputs("FAIL: canonical DM2 data did not load\n", stderr);
        free(dungeon);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        free(dungeon_bytes);
        return 1;
    }
    weather_level = find_weather_level(&loader, dungeon);
    CHECK(weather_level >= 0,
          "local DM2 data has a level with verified weather GDAT");
    if (weather_level < 0) {
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        free(dungeon_bytes);
        return failed ? 1 : 0;
    }

    dm2_v1_boot_profile_init(&boot);
    if (dm2_v1_boot_scan_assets(&boot, boot_root) != 0 ||
        dm2_v1_boot_enter_game(&boot) != 0) {
        fputs("FAIL: canonical DM2 boot profile was not entered\n", stderr);
        dm2_v1_dungeon_free(dungeon);
        free(dungeon);
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
        free(dungeon_bytes);
        return 1;
    }
    boot.dungeon_data = dungeon;

    /* Bind the boot asset provider so real GDAT pixels can be fetched. */
    CHECK(dm2_v1_runtime_bind_boot_profile(&boot) == 1,
          "runtime binds boot-profile GDAT asset provider");

    /* Move to the weather-capable level and switch to outdoor mode. */
    dm2_v1_runtime_set_outdoor(1);
    dm2_v1_runtime_set_position(weather_level, 0, 0, 0);

    /* Install deterministic storm state so the frame update selects cloud
     * (0x69) and rain (0x6c) without stochastic 0x54 timer waits. */
    {
        DM2_V1_UpdateWeatherState storm = make_storm_state();
        CHECK(dm2_v1_runtime_set_weather_chain_state_for_test(&storm) == 1,
              "test helper installs deterministic storm chain state");
    }

    /* Produce and bind live DistantEnvironment slots for this frame. */
    {
        DM2_V1_DistantEnvironmentReceipt slots[3];
        unsigned int slot_count = 0u;
        memset(slots, 0, sizeof(slots));
        CHECK(dm2_v1_runtime_update_weather_frame(slots, &slot_count) == 1 &&
                  slot_count > 0u,
              "runtime weather frame update binds live DistantEnvironment slots");
    }

    /* Advance the runtime tick so the source 0x54 weather timer owner is
     * receipted; this is the same owner the renderer/M11 gate binds to the
     * DistantEnvironment slots produced above. */
    dm2_v1_runtime_tick();

    /* Render the outdoor frame through the boot/runtime path. */
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&render_receipt, 0, sizeof(render_receipt));
    {
        int render_rc = dm2_v1_boot_runtime_render_frame(
            &boot, framebuffer, DM2_VP_WIDTH,
            DM2_VP_WIDTH, DM2_VP_HEIGHT,
            NULL, NULL, &render_receipt);
        CHECK(render_rc == 1,
              "outdoor frame renders through boot/runtime path");
    }

    /* The runtime frame ownership receipt must report real GDAT consumption. */
    memset(&ownership, 0, sizeof(ownership));
    {
        int rc = dm2_v1_runtime_last_frame_ownership(&ownership);
        CHECK(rc == 1,
              "runtime produced a frame ownership receipt");
    }
    if (ownership.valid) {
        CHECK(ownership.gdat_provider_bound,
              "frame was drawn with the boot GDAT provider bound");
        CHECK(ownership.outdoor_sky_gdat_blits > 0,
              "outdoor sky plane consumed real GDAT material");
        CHECK(ownership.outdoor_ground_gdat_blits > 0,
              "outdoor ground plane consumed real GDAT material");
        CHECK(ownership.hud_gdat_blits > 0,
              "HUD consumed real GDAT material");
        CHECK(ownership.gdat_scene_weather_consumed > 0,
              "weather overlay consumed real GDAT ENVIRONMENT pixels");
        CHECK(ownership.total_runtime_fallback_draws == 0,
              "no synthetic fallback draws were produced");
        CHECK(ownership.blocked_material_draws == 0,
              "no source material passes were blocked");
        CHECK(ownership.outdoor_gdat_frame_valid,
              "outdoor GDAT frame is valid");
        CHECK(ownership.full_gdat_frame_valid,
              "full GDAT frame is valid");
    }

    /* M11 must accept the runtime receipt. */
    memset(&m11_receipt, 0, sizeof(m11_receipt));
    {
        int rc = dm2_v1_runtime_last_m11_frame_receipt(&m11_receipt);
        CHECK(rc == 1,
              "runtime produced an M11 frame receipt");
    }
    if (m11_receipt.valid) {
        CHECK(m11_receipt.m11_consume_frame,
              "M11 frame receipt requests frame consumption");
        CHECK(m11_receipt.weather_material_plan_required,
              "weather material plan was required");
        CHECK(m11_receipt.weather_material_plan_consumed,
              "weather material plan was consumed");
        CHECK(m11_receipt.weather_material_plan_command_count > 0,
              "weather material plan has commands");
        CHECK(m11_receipt.weather_graphicsset_bound,
              "weather GRAPHICSSET receipt is bound");
    }

    CHECK(M11_Dm2RuntimeFrameReceipt_ShouldPresent(
              &render_receipt, &m11_receipt) == 1,
          "M11 runtime-frame gate accepts the outdoor weather frame");

    dm2_v1_boot_cleanup(&boot);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    free(dungeon_bytes);

    printf("DM2 V1 outdoor weather frame capture: %d passed, %d failed\n",
           passed, failed);
    return failed ? 1 : 0;
}
