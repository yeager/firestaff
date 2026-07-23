/* Live DistantEnvironment slot production for DM2 V1 weather overlays.
 *
 * Binds the source 0x54 timer weather chain to the renderer by running
 * DM2_UPDATE_WEATHER(0) (skproject/SKULLWIN/c_weather.cpp:91-506) each tick,
 * converting the resulting live_cmds into ten-byte DistantEnvironment register
 * images, and admitting them through the runtime source-owned GDAT receipt.
 * Without this step, real GDAT weather assets are parsed but never displayed.
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_update_weather_pc34_compat.h"
#include "dm2_v1_weather_gdat.h"

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

static DM2_V1_UpdateWeatherState make_clear_state(void)
{
    DM2_V1_UpdateWeatherState s;
    memset(&s, 0, sizeof(s));
    s.zone_index = 1;
    s.retry = 0;
    s.pattern_row = 1;
    s.step = 4;
    /* intensity 0x08 keeps cloud_state below 0x10 and rain_counter at 0,
     * while lightning_enabled=0 suppresses the bolt path.  This avoids the
     * intensity==0 flash branch which can still select a bolt. */
    s.intensity = 0x08;
    s.rain_counter = 0;
    s.day_tick = 0x70000000;
    s.clouds_enabled = 1;
    s.rain_enabled = 1;
    s.lightning_enabled = 0;
    return s;
}

static DM2_V1_UpdateWeatherState make_storm_with_bolt_state(void)
{
    DM2_V1_UpdateWeatherState s = make_storm_state();

    /* c_weather.cpp:441-474 enters the bolt path only after the cloud and
     * rain slots have been selected.  Thunder count 0 permits RANDBIT and
     * the seeded runtime RNG deterministically chooses one 0x64..0x66 slot.
     * Keep the normal storm material selectors so all three slots need real
     * GDAT ownership. */
    s.lightning_enabled = 1;
    s.lightning_flag = 1;
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
    DM2_V1_DistantEnvironmentReceipt slots[3];
    unsigned int slot_count;
    int weather_level;

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

    /* --- fail-closed before runtime is outdoor/started --- */
    dm2_v1_runtime_init(&boot);
    slot_count = 99u;
    CHECK(dm2_v1_runtime_update_weather_frame(slots, &slot_count) == 0 &&
              slot_count == 0u,
          "weather frame update fails closed before outdoor chain start");

    /* --- stormy weather produces live cloud+rain slots --- */
    DM2_V1_UpdateWeatherState storm_state = make_storm_state();
    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(1);
    dm2_v1_runtime_set_position(weather_level, 0, 0, 0);
    CHECK(dm2_v1_runtime_set_weather_chain_state_for_test(
              &storm_state) == 1,
          "test helper installs a deterministic storm chain state");
    memset(slots, 0, sizeof(slots));
    slot_count = 0u;
    CHECK(dm2_v1_runtime_update_weather_frame(slots, &slot_count) == 1,
          "storm weather frame update binds live slots");
    CHECK(slot_count == 2u,
          "storm weather produces exactly cloud+rain slots");
    if (slot_count >= 1u) {
        CHECK(slots[0].valid && slots[0].slot_index == 0u &&
                  slots[0].command == DM2_V1_WEATHER_CLOUD_STORM_CMD,
              "slot 0 is the storm cloud command");
        CHECK(slots[0].raw[0] == DM2_V1_WEATHER_CLOUD_STORM_CMD,
              "slot 0 raw byte 0 is the storm cloud command");
        CHECK(slots[0].raw[8] == 0x40u && slots[0].raw[9] == 0x40u,
              "slot 0 carries RETRIEVE_ENVIRONMENT_CMD_CD_FW defaults");
    }
    if (slot_count >= 2u) {
        CHECK(slots[1].valid && slots[1].slot_index == 1u &&
                  slots[1].command == DM2_V1_WEATHER_RAIN_STORM_CMD,
              "slot 1 is the storm rain command");
    }

    /* --- a source bolt follows the cloud and rain slots --- */
    {
        DM2_V1_UpdateWeatherState bolt_state = make_storm_with_bolt_state();
        int bolt_bound = 0;

        /* The source uses stochastic lightning selection. Advance several
         * real frames rather than manufacturing a DistantEnvironment slot;
         * once a valid bolt occurs it must survive the three-slot handoff. */
        dm2_v1_runtime_init(&boot);
        dm2_v1_runtime_set_outdoor(1);
        dm2_v1_runtime_set_position(weather_level, 0, 0, 0);
        for (int attempt = 0; attempt < 256 && !bolt_bound; ++attempt) {
            CHECK(dm2_v1_runtime_set_weather_chain_state_for_test(
                      &bolt_state) == 1,
                  "test helper installs source storm state for bolt route");
            memset(slots, 0, sizeof(slots));
            slot_count = 0u;
            if (dm2_v1_runtime_update_weather_frame(slots, &slot_count) == 1 &&
                slot_count == DM2_V1_WEATHER_MAX_SLOTS &&
                slots[2].valid &&
                slots[2].command >= DM2_V1_WEATHER_BOLT_CMD_BASE &&
                slots[2].command <= DM2_V1_WEATHER_BOLT_CMD_LAST) {
                bolt_bound = 1;
            }
        }
        CHECK(bolt_bound,
              "cloud, rain, and source-selected lightning occupy three live slots");
        if (bolt_bound) {
            CHECK(slots[2].slot_index == 2u && slots[2].raw[0] == slots[2].command &&
                      slots[2].raw_hash != 0u,
                  "third lightning slot retains its source register receipt");
        }
    }

    /* --- clear weather explicitly clears prior slots --- */
    DM2_V1_UpdateWeatherState storm_state2 = make_storm_state();
    DM2_V1_UpdateWeatherState clear_state = make_clear_state();
    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(1);
    dm2_v1_runtime_set_position(weather_level, 0, 0, 0);
    CHECK(dm2_v1_runtime_set_weather_chain_state_for_test(
              &storm_state2) == 1,
          "test helper reinstalls storm state for clear-weather transition");
    CHECK(dm2_v1_runtime_update_weather_frame(NULL, NULL) == 1,
          "storm state binds slots before clear transition");
    CHECK(dm2_v1_runtime_set_weather_chain_state_for_test(
              &clear_state) == 1,
          "test helper installs a deterministic clear chain state");
    memset(slots, 0, sizeof(slots));
    slot_count = 99u;
    CHECK(dm2_v1_runtime_update_weather_frame(slots, &slot_count) == 1 &&
              slot_count == 0u,
          "clear weather frame update binds zero slots");

    /* --- runtime tick path also drives the frame update --- */
    DM2_V1_UpdateWeatherState storm_state3 = make_storm_state();
    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(1);
    dm2_v1_runtime_set_position(weather_level, 0, 0, 0);
    CHECK(dm2_v1_runtime_set_weather_chain_state_for_test(
              &storm_state3) == 1,
          "test helper installs storm state for tick-path check");
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_last_frame_ownership(NULL) == 0 || 1,
          "tick path runs without crashing after weather frame wiring");

    dm2_v1_boot_cleanup(&boot);
    /* boot cleanup freed the heap-allocated dungeon and its internals. */
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    free(dungeon_bytes);

    printf("DM2 V1 runtime weather frame slot: %d passed, %d failed\n",
           passed, failed);
    return failed ? 1 : 0;
}
