/* CSB boot must not expose a viewport when no source-format dungeon is live.
 * ReDMCSB DUNGEON.C F0237 and LOADSAVE.C F0435 load the dungeon header and
 * initial party location before the first dungeon view is drawn. */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_viewport_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static int write_legacy_fixture(const char *path)
{
    /* Retired parser-fixture layout: 16-bit square records, not the
     * DUNGEON_HEADER/MAP byte-map that a CSB runtime may consume. */
    static const uint8_t fixture[] = {
        0x01, 0x00, 0x10, 0x00,
        0x02, 0x02, 0x0A, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00
    };
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    if (fwrite(fixture, 1, sizeof(fixture), file) != sizeof(fixture)) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

static void prime_verified_profile(CSB_V1_BootProfile *profile,
                                   const char *dungeon_path)
{
    csb_v1_boot_profile_init(profile);
    snprintf(profile->asset_root, sizeof(profile->asset_root), "%s", "/tmp");
    snprintf(profile->graphics_path, sizeof(profile->graphics_path), "%s",
             "/tmp/GRAPHICS.DAT");
    snprintf(profile->dungeon_path, sizeof(profile->dungeon_path), "%s",
             dungeon_path);
    snprintf(profile->dungeon_md5, sizeof(profile->dungeon_md5), "%s",
             "6695d2acebce49f95db1d8f3a5c733de");
    profile->assets_verified = 1;
    profile->graphics_verified = 1;
    profile->dungeon_verified = 1;
    profile->variant_id = CSB_V1_VARIANT_PC34_EN;
    profile->graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
}

static void test_legacy_fixture_cannot_enter_or_render(void)
{
    CSB_V1_BootProfile profile;
    CSB_V1_ViewportConfig config;
    uint8_t grid[32 * 32];
    const char *path = "/tmp/firestaff-csb-legacy-viewport-fixture.dat";

    remove(path);
    CHECK(write_legacy_fixture(path), "legacy parser fixture is written");
    prime_verified_profile(&profile, path);

    CHECK(csb_v1_boot_enter_game(&profile) == -1,
          "enter_game rejects a legacy 16-bit fixture");
    CHECK(profile.state == CSB_V1_BOOT_STATE_ASSETS_READY,
          "failed materialization returns to ASSETS_READY");
    CHECK(profile.runtime.dungeon_handle == NULL &&
          csb_v1_dungeon_get_current() == NULL,
          "failed materialization publishes no live dungeon handle");
    CHECK(profile.engine_version_displayed == 0,
          "failed materialization does not advertise a CSB runtime");

    csb_v1_viewport_init(&config);
    memset(grid, 0xa5, sizeof(grid));
    CHECK(csb_v1_viewport_bind_live_dungeon_grid(
              &config, csb_v1_dungeon_get_current(), 0, grid) == 0,
          "viewport binding rejects the absent live dungeon");
    CHECK(config.dungeon_grid == NULL && config.dungeon_width == 0 &&
          config.dungeon_height == 0,
          "rejected viewport binding clears grid ownership");

    csb_v1_boot_cleanup(&profile);
    remove(path);
}

int main(void)
{
    puts("=== CSB V1 Boot → Viewport Materialization Gate ===\n");
    test_legacy_fixture_cannot_enter_or_render();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
