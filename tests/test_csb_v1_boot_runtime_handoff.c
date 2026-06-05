/*
 * test_csb_v1_boot_runtime_handoff.c
 *
 * CSB V1 Boot → Runtime Handoff regression
 *
 * Verifies that csb_v1_boot_enter_game() actually completes the
 * profile-to-runtime handoff. Before this fix, enter_game() set the
 * runtime path strings and CSB_STATE_TITLE, but did NOT:
 *   1. Set runtime.dungeon_asset.kind
 *   2. Copy entrance_map_index / start_map_index from the boot profile
 *   3. Load DUNGEON.DAT into the runtime (dungeon_handle == NULL)
 *
 * The handoff gap left the runtime in a TITLE state with no live
 * dungeon, forcing the game-view to do a second hash search via
 * csb_v1_runtime_boot(). This test exercises the in-place handoff with
 * a synthetic DUNGEON.DAT so we can prove the boundary works end-to-end.
 *
 * Source-locks (matches src/csb/csb_v1_boot.c citation block):
 *   ReDMCSB ENTRANCE.C F0806 lines 409-441 entrance micro-dungeon
 *   ReDMCSB LOADSAVE.C F0435 lines 1940-1944 new-game map 0
 *   CSBWin/CSBCode.cpp:6800-6950 LoadDungeon
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_game_state_pc34_compat.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#else
#define TEST_MKDIR(path) mkdir((path), 0700)
#endif

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Build a minimal valid CSB V1 DUNGEON.DAT buffer. Mirrors the
 * synthetic builder in test_csb_v1_phase7_verification.c so the
 * fixture shape matches the legacy loader (square_bytes == 2,
 * column-major 16-bit records, ReDMCSB DUNGEON.C F0151).
 *
 * Header layout (CSB V1 legacy synthetic fixture):
 *   0..1  : level_count (LE16) = 1
 *   2..3  : ignored by the legacy loader
 *   4     : level 0 width  (uint8)
 *   5     : level 0 height (uint8)
 *   6..9  : level 0 absolute byte offset to squares (LE32)
 *   10..  : squares, column-major, 2 bytes each (low byte = type)
 */
static int build_synthetic_dungeon(uint8_t *buf, int buf_size,
                                    uint8_t square_type_1_1)
{
    if (!buf || buf_size < 28) return -1;
    memset(buf, 0, (size_t)buf_size);
    buf[0] = 1; buf[1] = 0;             /* level_count = 1 */
    buf[2] = 16; buf[3] = 0;            /* ignored padding (matches existing fixture) */
    buf[4] = 3; buf[5] = 3;             /* level 0 width=3, height=3 */
    buf[6] = 10; buf[7] = 0;            /* level 0 absolute square offset = 10 */
    buf[8] = 0; buf[9] = 0;
    /* 3x3 squares, column-major, 2 bytes each.
     * Cell (x,y) lives at offset 10 + (x*3 + y) * 2.
     * Row 0: walls at (0,0),(1,0),(2,0) */
    buf[10] = 1; buf[11] = 0;
    buf[12] = 1; buf[13] = 0;
    buf[14] = 1; buf[15] = 0;
    /* Row 1: wall at (0,1), marker at (1,1), wall at (2,1) */
    buf[16] = 1; buf[17] = 0;
    buf[18] = square_type_1_1; buf[19] = 0;
    buf[20] = 1; buf[21] = 0;
    /* Row 2: walls */
    buf[22] = 1; buf[23] = 0;
    buf[24] = 1; buf[25] = 0;
    buf[26] = 1; buf[27] = 0;
    return 0;
}

static int write_synthetic_dungeon(const char *path, uint8_t square_type_1_1)
{
    uint8_t buf[32];
    FILE *f;
    size_t n;
    if (build_synthetic_dungeon(buf, (int)sizeof(buf), square_type_1_1) != 0) {
        return -1;
    }
    f = fopen(path, "wb");
    if (!f) return -1;
    n = fwrite(buf, 1, sizeof(buf), f);
    fclose(f);
    return (n == sizeof(buf)) ? 0 : -1;
}

static void test_enter_game_with_verified_profile_loads_dungeon(void)
{
    CSB_V1_BootProfile p;
    char dungeon_path[ASSET_PATH_MAX];
    char graphics_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-test";
    int mkdir_ok = (TEST_MKDIR(tmp_dir) == 0) || 1; /* best-effort */

    memset(&p, 0, sizeof(p));
    (void)mkdir_ok;
    csb_v1_boot_profile_init(&p);

    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    snprintf(graphics_path, sizeof(graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, 2) == 0,
          "synthetic DUNGEON.DAT written to temp path");

    /* Simulate a hash-verified boot scan by populating the fields the
     * scanner would normally set. The handoff logic only reads these
     * fields and does not re-hash, so this is a faithful model. */
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s", graphics_path);
    snprintf(p.dungeon_md5, sizeof(p.dungeon_md5),
             "6695d2acebce49f95db1d8f3a5c733de"); /* CSB PC 3.4 EN */
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    p.entrance_map_index = 255U;
    p.start_map_index = 0U;

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game succeeds for a hash-verified boot profile");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot state advances to RUNTIME_READY after enter_game");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "runtime state machine is CSB_STATE_TITLE (ReDMCSB ENTRANCE.C F0806)");
    CHECK(p.runtime.variant_id == CSB_V1_VARIANT_PC34_EN,
          "runtime inherits the boot profile variant id");
    CHECK(p.runtime.difficulty == CSB_V1_DIFFICULTY_HARD,
          "runtime keeps the CSB V1 default difficulty");
    CHECK(p.runtime.entrance_map_index == 255U,
          "runtime entrance map matches boot profile (C255_MAP_INDEX_ENTRANCE)");
    CHECK(p.runtime.start_map_index == 0U,
          "runtime start map is 0 (ReDMCSB LOADSAVE.C F0435)");
    CHECK(p.runtime.dungeon_asset.path == p.dungeon_path,
          "runtime dungeon_asset.path points at the boot profile path");
    CHECK(p.runtime.dungeon_asset.kind == CSB_V1_ASSET_GFX_ARCHIVE_NONE,
          "runtime dungeon_asset.kind is NONE (dungeon, not graphics)");
    CHECK(p.runtime.graphics_asset.path == p.graphics_path,
          "runtime graphics_asset.path points at the boot profile path");
    CHECK(p.runtime.graphics_asset.kind == CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS,
          "runtime graphics_asset.kind matches the boot profile kind");
    CHECK(p.runtime.dungeon_handle != NULL,
          "dungeon_handle is non-NULL: DUNGEON.DAT was loaded during handoff");
    CHECK(csb_v1_dungeon_get_current() == p.runtime.dungeon_handle,
          "csb_v1_dungeon_get_current() returns the loaded handle");
    CHECK(csb_v1_dungeon_get_current_level() == 0,
          "current level defaults to 0 after handoff");
    CHECK(p.runtime.chaos_magic.magic_initialized == 1,
          "chaos magic is initialized after handoff (ReDMCSB CASTER.C F0211)");

    /* Cleanup: release the dungeon through the runtime cleanup path
     * so we don't leak heap across tests. */
    csb_v1_dungeon_set_current(NULL);
    free(p.runtime.dungeon_handle);
    p.runtime.dungeon_handle = NULL;
}

static void test_enter_game_with_missing_dungeon_path_keeps_runtime_safe(void)
{
    CSB_V1_BootProfile p;
    char dungeon_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-missing-dng";
    char bogus_dungeon[ASSET_PATH_MAX];

    (void)TEST_MKDIR(tmp_dir); /* best-effort */
    snprintf(bogus_dungeon, sizeof(bogus_dungeon),
             "%s/this-file-does-not-exist.DAT", tmp_dir);
    (void)dungeon_path;

    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);

    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", bogus_dungeon);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game still succeeds when the verified path is unreadable");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot state still advances to RUNTIME_READY");
    CHECK(p.runtime.dungeon_handle == NULL,
          "dungeon_handle stays NULL when the path cannot be opened");
    CHECK(p.runtime.dungeon_asset.path == p.dungeon_path,
          "dungeon_asset.path still points at the verified path even if load fails");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "runtime state remains CSB_STATE_TITLE");
    CHECK(p.runtime.chaos_magic.magic_initialized == 1,
          "chaos magic is still initialized (handoff is tolerant)");

    /* Free the global dungeon singleton if the loader left anything set. */
    csb_v1_dungeon_set_current(NULL);
}

static void test_enter_game_runtime_handoff_is_idempotent(void)
{
    CSB_V1_BootProfile p;
    char dungeon_path[ASSET_PATH_MAX];
    const char *tmp_dir = "/tmp/firestaff-csb-v1-handoff-idempotent";

    (void)TEST_MKDIR(tmp_dir);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", tmp_dir);
    CHECK(write_synthetic_dungeon(dungeon_path, 7) == 0,
          "synthetic DUNGEON.DAT written for idempotence test");

    memset(&p, 0, sizeof(p));
    csb_v1_boot_profile_init(&p);
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", tmp_dir);
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", dungeon_path);
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s/GRAPHICS.DAT", tmp_dir);
    p.dungeon_verified = 1;
    p.graphics_verified = 1;
    p.assets_verified = 1;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    p.entrance_map_index = 255U;
    p.start_map_index = 0U;

    CHECK(csb_v1_boot_enter_game(&p) == 0, "first enter_game succeeds");
    CSB_V1_DungeonData *first_handle = p.runtime.dungeon_handle;
    const CSB_V1_DungeonData *first_global = csb_v1_dungeon_get_current();
    CHECK(first_handle != NULL, "first handoff produced a heap dungeon handle");
    CHECK(first_global == first_handle, "singleton matches the heap handle");

    /* A second enter_game() on the same profile must re-load and
     * replace the previous handle (no double-free, no stale pointer). */
    CHECK(csb_v1_boot_enter_game(&p) == 0, "second enter_game succeeds");
    CHECK(p.runtime.dungeon_handle != NULL, "second handoff produced a fresh handle");
    CHECK(p.runtime.dungeon_handle != first_handle,
          "second handoff allocated a new handle (replaces, not aliases)");
    CHECK(csb_v1_dungeon_get_current() == p.runtime.dungeon_handle,
          "singleton now points at the second handle");

    /* Free the latest handle; the previous first_handle was already
     * released by csb_v1_dungeon_set_current() during the second boot. */
    csb_v1_dungeon_set_current(NULL);
    free(p.runtime.dungeon_handle);
    p.runtime.dungeon_handle = NULL;
}

int main(void)
{
    printf("=== CSB V1 Boot → Runtime Handoff Regression ===\n\n");
    test_enter_game_with_verified_profile_loads_dungeon();
    test_enter_game_with_missing_dungeon_path_keeps_runtime_safe();
    test_enter_game_runtime_handoff_is_idempotent();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: CSB V1 boot→runtime handoff completes dungeon load, asset metadata, and entrance/start map handoff in one step");
        puts("sourceEvidence=ReDMCSB ENTRANCE.C F0806 lines 409-441; LOADSAVE.C F0435 lines 1940-1944; CSBWin/CSBCode.cpp:6800-6950 LoadDungeon");
    }
    return failed == 0 ? 0 : 1;
}
