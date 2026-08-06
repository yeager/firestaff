/* Canonical-data outdoor frame capture for DM2 V1's weather source gate.
 *
 * A GRAPHICS.DAT/DUNGEON.DAT pair supplies authentic GDAT material, but it
 * does not contain the live c_weather v1e14xx chain that belongs to a running
 * game or imported save.  Therefore this test proves the production contract:
 * canonical installation media may be decoded for material verification, but
 * it cannot publish an M11-presentable outdoor frame while the source-owned
 * GAME_LOAD/session state is absent. It must not inject a fixture weather
 * chain and call the result game data.
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
    char graphics_path[1100];
    char dungeon_path[1100];

    if (!root || !root[0]) return -1;
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
    snprintf(boot_root, boot_root_size, "%s/..", root);
    return read_file(graphics_path, graphics, graphics_size) &&
        read_file(dungeon_path, dungeon, dungeon_size) ? 1 : 0;
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

    {
        const int load_result = load_canonical_files(
            &graphics, &graphics_size, &dungeon_bytes, &dungeon_size,
            boot_root, sizeof(boot_root));
        if (load_result < 0) {
            puts("SKIP: FIRESTAFF_DM2_DATA_DIR is not configured");
            return 0;
        }
        if (load_result == 0) {
            fputs("FAIL: configured DM2 graphics.dat/dungeon.dat is unreadable\n",
                  stderr);
            return 1;
        }
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

    /* Static installation files do not prove a live c_weather chain.  The
     * runtime must reject weather slots rather than manufacture a cloud/rain
     * choice from generic presentation state. */
    {
        DM2_V1_DistantEnvironmentReceipt slots[3];
        unsigned int slot_count = 0u;
        memset(slots, 0, sizeof(slots));
        CHECK(dm2_v1_runtime_update_weather_frame(slots, &slot_count) == 0 &&
                  slot_count == 0u,
              "weather frame stays no-draw without source-owned session state");
    }

    /* The normal tick must preserve that fail-closed decision. */
    dm2_v1_runtime_tick();

    /* Exercise the boot/runtime renderer. Its decoded source material must
     * still not become a player-facing frame without GAME_LOAD ownership. */
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&render_receipt, 0, sizeof(render_receipt));
    {
        int render_rc = dm2_v1_boot_runtime_render_frame(
            &boot, framebuffer, DM2_VP_WIDTH,
            DM2_VP_WIDTH, DM2_VP_HEIGHT,
            NULL, NULL, &render_receipt);
        CHECK(render_rc == 1,
              "outdoor material evaluation completes through boot/runtime path");
    }

    /* Static installation data cannot replace the live session/pose owner.
     * The runtime may have decoded real material above, but it must not issue
     * a presentation receipt for an invented party/map frame. */
    memset(&ownership, 0, sizeof(ownership));
    {
        int rc = dm2_v1_runtime_last_frame_ownership(&ownership);
        CHECK(rc == 0 && !ownership.valid,
              "runtime refuses an outdoor ownership receipt without original session state");
    }

    /* M11 must not accept the unowned runtime receipt. */
    memset(&m11_receipt, 0, sizeof(m11_receipt));
    {
        int rc = dm2_v1_runtime_last_m11_frame_receipt(&m11_receipt);
        CHECK(rc == 0 && !m11_receipt.valid &&
                  !m11_receipt.m11_consume_frame,
              "runtime withholds an M11 frame receipt without original session state");
    }

    CHECK(M11_Dm2RuntimeFrameReceipt_ShouldPresent(
              &render_receipt, &m11_receipt) == 0,
          "M11 runtime-frame gate rejects the unowned outdoor frame");

    dm2_v1_boot_cleanup(&boot);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    free(dungeon_bytes);

    printf("DM2 V1 outdoor weather source gate: %d passed, %d failed\n",
           passed, failed);
    return failed ? 1 : 0;
}
