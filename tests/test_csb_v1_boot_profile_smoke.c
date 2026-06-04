#include "csb_v1_boot.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static void test_defaults(void)
{
    CSB_V1_BootProfile p;
    csb_v1_boot_profile_init(&p);
    CHECK(strcmp(p.game_id, "csb") == 0, "game id is csb");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY, "default state is PROFILE_READY");
    CHECK(p.tick_ms == CSB_V1_TICK_MS_NOMINAL, "tick is the V1 55 ms quantum");
    CHECK(p.entrance_map_index == 255U, "entrance map is C255_MAP_INDEX_ENTRANCE");
    CHECK(p.start_map_index == 0U, "new-game map index is 0");
    CHECK(p.default_party_x == CSB_V1_START_PARTY_X, "default party x follows CSB runtime profile");
    CHECK(p.default_party_y == CSB_V1_START_PARTY_Y, "default party y follows CSB runtime profile");
    CHECK(p.default_party_dir == CSB_V1_START_PARTY_DIR, "default party dir follows CSB runtime profile");
}

static void test_scan_missing_data(void)
{
    CSB_V1_BootProfile p;
    char diag[1024];
    size_t n;

    csb_v1_boot_profile_init(&p);
    CHECK(csb_v1_boot_scan_assets(&p, "/tmp/firestaff-csb-v1-no-assets") == -1,
          "missing data does not verify");
    CHECK(p.assets_verified == 0, "assets_verified remains false");
    CHECK(p.graphics_verified == 0, "graphics_verified remains false");
    CHECK(p.dungeon_verified == 0, "dungeon_verified remains false");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "missing data leaves profile at PROFILE_READY");
    CHECK(csb_v1_boot_probe_available("/tmp/firestaff-csb-v1-no-assets") == 0,
          "probe_available is false without both CSB assets");
    n = csb_v1_boot_diagnostic_report(&p, diag, sizeof(diag));
    CHECK(n > 0U && strstr(diag, "CSB V1 Boot Profile") != NULL,
          "diagnostic report is populated");
}

static void test_save_root_override(void)
{
    CSB_V1_BootProfile p;
    csb_v1_boot_profile_init(&p);
    csb_v1_boot_set_save_root(&p, "/tmp/firestaff-csb-saves");
    CHECK(strcmp(p.save_root, "/tmp/firestaff-csb-saves") == 0,
          "explicit save root is preserved");
}

static void test_enter_requires_assets(void)
{
    CSB_V1_BootProfile p;
    csb_v1_boot_profile_init(&p);
    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "enter_game rejects an unverified profile");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "failed enter_game leaves state unchanged");
}

static void test_enter_handoff_state(void)
{
    CSB_V1_BootProfile p;
    csb_v1_boot_profile_init(&p);

    p.assets_verified = 1;
    p.graphics_verified = 1;
    p.dungeon_verified = 1;
    p.state = CSB_V1_BOOT_STATE_ASSETS_READY;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", "/tmp/firestaff-csb-v1-data");
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s", "/tmp/firestaff-csb-v1-data/CSBGRAPH.DAT");
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", "/tmp/firestaff-csb-v1-data/DUNGEON.DAT");
    csb_v1_boot_set_save_root(&p, "/tmp/firestaff-csb-v1-saves");

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game accepts a verified profile");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "enter_game advances boot profile to RUNTIME_READY");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "runtime starts at the CSB title/entrance state");
    CHECK(p.runtime.variant_id == CSB_V1_VARIANT_PC34_EN,
          "runtime receives the verified CSB variant");
    CHECK(p.runtime.difficulty == CSB_V1_DIFFICULTY_HARD,
          "runtime handoff uses the CSB hard default");
    CHECK(p.runtime.data_dir == p.asset_root,
          "runtime data_dir points at the verified asset root");
    CHECK(p.runtime.save_dir == p.save_root,
          "runtime save_dir carries the boot save-root override");
    CHECK(p.runtime.dungeon_path == p.dungeon_path,
          "runtime dungeon_path points at the verified dungeon path");
    CHECK(p.runtime.graphics_path == p.graphics_path,
          "runtime graphics_path points at the verified graphics path");
    CHECK(p.runtime.dungeon_asset.path == p.dungeon_path,
          "runtime dungeon asset carries the verified path");
    CHECK(p.runtime.graphics_asset.path == p.graphics_path,
          "runtime graphics asset carries the verified path");
    CHECK(p.runtime.graphics_asset.kind == CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF,
          "runtime graphics asset carries the verified archive kind");
    CHECK(p.runtime.party_x == CSB_V1_START_PARTY_X &&
          p.runtime.party_y == CSB_V1_START_PARTY_Y &&
          p.runtime.party_dir == CSB_V1_START_PARTY_DIR,
          "runtime keeps the source-locked CSB start position");
    CHECK(p.runtime.chaos_magic.magic_initialized == 1 &&
          p.runtime.chaos_magic.spell_grid_version == 0U &&
          p.runtime.chaos_magic.chaos_level == 0U,
          "runtime initializes CSB chaos magic at handoff");
}

static void test_source_evidence(void)
{
    const char *e = csb_v1_boot_source_evidence();
    CHECK(e && strstr(e, "ENTRANCE.C F0806") != NULL,
          "source evidence cites ENTRANCE.C F0806");
    CHECK(e && strstr(e, "LOADSAVE.C F0435") != NULL,
          "source evidence cites LOADSAVE.C F0435");
}

int main(void)
{
    printf("=== CSB V1 Boot Profile Smoke Test ===\n\n");
    test_defaults();
    test_scan_missing_data();
    test_save_root_override();
    test_enter_requires_assets();
    test_enter_handoff_state();
    test_source_evidence();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
