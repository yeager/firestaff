/* CSB runtime fixture-rejection gate.
 *
 * This target formerly treated a hand-written legacy dungeon and a six-byte
 * GRAPHICS.DAT marker as a playable CSB route.  That contradicted the boot
 * materialization contract: ReDMCSB LOADSAVE.C F0435 starts from the decoded
 * original DUNGEON_HEADER, while M11 accepts only the scanned source hashes.
 * Keep the target as a regression that those fixtures cannot enter runtime.
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define ROUTE_MKDIR(path) _mkdir(path)
#else
#define ROUTE_MKDIR(path) mkdir((path), 0700)
#endif

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static int write_fixture(const char *path, const uint8_t *bytes, size_t size)
{
    FILE *file = fopen(path, "wb");
    size_t written;
    if (!file) return 0;
    written = fwrite(bytes, 1u, size, file);
    return fclose(file) == 0 && written == size;
}

int main(void)
{
    static const uint8_t legacy_dungeon[] = {
        1, 0, 16, 0, 2, 2, 10, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0
    };
    static const uint8_t marker_graphics[] = { 'C', 'S', 'B', 'R', 'T', 0 };
    const char *tmp_dir = "/tmp/firestaff-csb-v1-runtime-route-gate";
    char dungeon_path[512];
    char graphics_path[512];
    char reason[256];
    CSB_V1_BootProfile boot;
    CSB_V1_RuntimeProfile runtime;

    printf("=== CSB V1 synthetic-route rejection gate ===\n\n");
    (void)ROUTE_MKDIR(tmp_dir);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    snprintf(graphics_path, sizeof(graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    CHECK(write_fixture(dungeon_path, legacy_dungeon, sizeof(legacy_dungeon)),
          "legacy dungeon fixture written");
    CHECK(write_fixture(graphics_path, marker_graphics, sizeof(marker_graphics)),
          "marker graphics fixture written");
    {
        CSB_V1_DungeonData dungeon;
        memset(&dungeon, 0, sizeof(dungeon));
        CHECK(csb_v1_dungeon_load_from_file(&dungeon, dungeon_path) == -2 &&
                  dungeon.raw_data == NULL,
              "file loader rejects legacy dungeon fixture");
    }

    csb_v1_boot_profile_init(&boot);
    snprintf(boot.asset_root, sizeof(boot.asset_root), "%s", tmp_dir);
    snprintf(boot.dungeon_path, sizeof(boot.dungeon_path), "%s", dungeon_path);
    snprintf(boot.graphics_path, sizeof(boot.graphics_path), "%s", graphics_path);
    snprintf(boot.dungeon_md5, sizeof(boot.dungeon_md5), "%s",
             "6695d2acebce49f95db1d8f3a5c733de");
    snprintf(boot.graphics_md5, sizeof(boot.graphics_md5), "%s",
             "61fbfd56887c94adc26888a9491c6611");
    boot.dungeon_verified = 1;
    boot.graphics_verified = 1;
    boot.assets_verified = 1;
    boot.state = CSB_V1_BOOT_STATE_ASSETS_READY;
    boot.variant_id = CSB_V1_VARIANT_PC34_EN;

    CHECK(csb_v1_boot_profile_m11_entry_gate(&boot, reason, sizeof(reason)) == 0 &&
              strstr(reason, "GRAPHICS bytes changed after scan") != NULL,
          "M11 entry refuses a filename-only graphics substitute");
    CHECK(csb_v1_boot_enter_game(&boot) == -1 &&
              boot.state == CSB_V1_BOOT_STATE_ASSETS_READY &&
              boot.runtime.dungeon_handle == NULL,
          "boot rejects legacy dungeon materialization");

    csb_v1_runtime_init(&runtime, tmp_dir);
    CHECK(csb_v1_runtime_boot(&runtime, tmp_dir, "pc34_en") == -1 &&
              runtime.dungeon_handle == NULL && runtime.graphics_path == NULL,
          "direct runtime boot rejects unverified fixture pair");
    csb_v1_runtime_cleanup(&runtime);
    remove(dungeon_path);
    remove(graphics_path);

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed != 0;
}
