/*
 * firestaff_csb_v1_pc_real_asset_launch_probe.c
 *
 * PC-first CSB V1 real-data launch gate.  This probe is intentionally
 * separate from the Atari ST/Amiga real-asset probe: the 24h CSB finish lane
 * should first prove the canonical PC CSB pair can scan, hand off to the CSB
 * boot profile, load DUNGEON.DAT, and tick the V1 runtime.
 *
 * Source-lock boundary:
 *   - ReDMCSB ENTRANCE.C F0806 lines 409-441 selects the CSB entrance path.
 *   - ReDMCSB LOADSAVE.C F0435 lines 1936-1944 loads the new-game dungeon.
 *   - ReDMCSB DUNGEON.C F0237 owns dungeon-load entry.
 *
 * The probe is skip-safe on hosts without user-supplied game data.
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
static int failures;

#define CHECK(cond, msg) do { \
    ++checks; \
    if (cond) { \
        printf("  PASS: %s\n", msg); \
    } else { \
        ++failures; \
        printf("  FAIL: %s\n", msg); \
    } \
} while (0)

static const char *pc_data_dir(int argc, char **argv, char *buf, size_t buf_size)
{
    const char *env;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0] != '\0') return argv[1];

    env = getenv("FIRESTAFF_CSB_PC_DATA");
    if (env && env[0] != '\0') return env;

    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(buf, buf_size, "%s/.firestaff/data/csb", home);
    return buf;
}

static int pc_data_present(const char *dir)
{
    CSB_V1_BootProfile profile;
    if (!dir || dir[0] == '\0') return 0;
    csb_v1_boot_profile_init(&profile);
    return csb_v1_boot_scan_assets(&profile, dir) == 0;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir = pc_data_dir(argc, argv, default_dir, sizeof(default_dir));
    CSB_V1_BootProfile profile;
    const CSB_V1_DungeonData *current;
    uint64_t before_play_ms;

    printf("=== CSB V1 PC real-asset launch probe ===\n\n");
    printf("data_dir=%s\n", dir ? dir : "(none)");

    if (!pc_data_present(dir)) {
        printf("SKIP: PC CSB GRAPHICS.DAT + DUNGEON.DAT not available; "
               "set FIRESTAFF_CSB_PC_DATA to enable the real-data gate.\n");
        return 0;
    }

    csb_v1_boot_profile_init(&profile);
    CHECK(csb_v1_boot_scan_assets(&profile, dir) == 0,
          "PC CSB assets scan by hash");
    CHECK(profile.assets_verified == 1, "boot profile marks assets verified");
    CHECK(profile.graphics_verified == 1, "GRAPHICS.DAT is verified");
    CHECK(profile.dungeon_verified == 1, "DUNGEON.DAT is verified");
    CHECK(strcmp(profile.graphics_md5, "61fbfd56887c94adc26888a9491c6611") == 0,
          "PC CSB graphics MD5 matches canonical PC 3.4 English");
    CHECK(strcmp(profile.dungeon_md5, "6695d2acebce49f95db1d8f3a5c733de") == 0,
          "CSB dungeon MD5 matches shared canonical dungeon");
    CHECK(profile.variant_id == CSB_V1_VARIANT_PC34_EN,
          "variant detection selects PC DOS 3.4 English");
    CHECK(profile.graphics_kind == CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS,
          "graphics archive kind is ordinary GRAPHICS.DAT");

    CHECK(csb_v1_boot_enter_game(&profile) == 0,
          "boot profile enters the CSB V1 runtime");
    CHECK(profile.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot profile advances to runtime-ready");
    CHECK(profile.runtime.state == CSB_STATE_TITLE,
          "runtime starts at the title/entrance state");
    CHECK(profile.runtime.variant_id == CSB_V1_VARIANT_PC34_EN,
          "runtime carries the PC CSB variant");
    CHECK(profile.runtime.dungeon_handle != NULL,
          "runtime owns a loaded dungeon handle");
    current = csb_v1_dungeon_get_current();
    CHECK(current == profile.runtime.dungeon_handle,
          "current dungeon singleton points at the runtime-owned dungeon");
    CHECK(current && current->level_count > 0,
          "loaded PC dungeon exposes at least one level");
    CHECK(csb_v1_dungeon_get_current_level() == 0,
          "new-game dungeon level is map 0");
    CHECK(profile.runtime.party_x == CSB_V1_START_PARTY_X &&
          profile.runtime.party_y == CSB_V1_START_PARTY_Y &&
          profile.runtime.party_dir == CSB_V1_START_PARTY_DIR,
          "runtime keeps the source-locked CSB start pose");
    CHECK(profile.runtime.chaos_magic.magic_initialized == 1,
          "CSB chaos magic is initialized at handoff");

    before_play_ms = profile.runtime.total_play_ms;
    csb_v1_runtime_tick(&profile.runtime, CSB_V1_TICK_MS_NOMINAL);
    CHECK(profile.runtime.total_play_ms > before_play_ms,
          "one CSB V1 tick advances play time");

    csb_v1_boot_cleanup(&profile);
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "boot cleanup clears the current dungeon context");

    printf("\nchecks=%d failures=%d\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
