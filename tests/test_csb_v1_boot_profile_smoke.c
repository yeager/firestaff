#include "csb_v1_boot.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static int write_synthetic_dungeon_fixture(const char *path)
{
    static const uint8_t fixture[] = {
        0x01, 0x00, 0x10, 0x00, /* one level, 16 thing types */
        0x02, 0x02, 0x0A, 0x00, 0x00, 0x00, /* 2x2 level at offset 10 */
        0x00, 0x00, 0x01, 0x00, /* x=0 y=0..1 */
        0x02, 0x00, 0x03, 0x00  /* x=1 y=0..1 */
    };
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    if (fwrite(fixture, 1U, sizeof(fixture), f) != sizeof(fixture)) {
        fclose(f);
        return 0;
    }
    return fclose(f) == 0;
}

static void test_defaults(void)
{
    CSB_V1_BootProfile p;
    size_t skin_def_word_count = 123u;

    csb_v1_boot_profile_init(&p);
    CHECK(strcmp(p.game_id, "csb") == 0, "game id is csb");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY, "default state is PROFILE_READY");
    CHECK(p.tick_ms == CSB_V1_TICK_MS_NOMINAL, "tick is the V1 55 ms quantum");
    CHECK(p.entrance_map_index == 255U, "entrance map is C255_MAP_INDEX_ENTRANCE");
    CHECK(p.start_map_index == 0U, "new-game map index is 0");
    CHECK(p.default_party_x == CSB_V1_START_PARTY_X, "default party x follows CSB runtime profile");
    CHECK(p.default_party_y == CSB_V1_START_PARTY_Y, "default party y follows CSB runtime profile");
    CHECK(p.default_party_dir == CSB_V1_START_PARTY_DIR, "default party dir follows CSB runtime profile");
    CHECK(p.csbgraphics_skin_def_loaded == 0 &&
          p.csbgraphics_skin_def_word_count == 0u,
          "CSBgraphics skin-def ownership starts empty");
    CHECK(csb_v1_boot_csbgraphics_skin_def_words(&p,
                                                 &skin_def_word_count) == NULL &&
          skin_def_word_count == 0u,
          "CSBgraphics skin-def accessor rejects an empty boot profile");
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

static void test_rescan_missing_data_clears_stale_handoff(void)
{
    CSB_V1_BootProfile p;

    csb_v1_boot_profile_init(&p);
    p.assets_verified = 1;
    p.graphics_verified = 1;
    p.dungeon_verified = 1;
    p.state = CSB_V1_BOOT_STATE_RUNTIME_READY;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
    snprintf(p.graphics_path, sizeof(p.graphics_path),
             "%s", "/tmp/firestaff-csb-stale/CSBGRAPH.DAT");
    snprintf(p.dungeon_path, sizeof(p.dungeon_path),
             "%s", "/tmp/firestaff-csb-stale/DUNGEON.DAT");
    snprintf(p.graphics_md5, sizeof(p.graphics_md5),
             "%s", "61fbfd56887c94adc26888a9491c6611");
    snprintf(p.dungeon_md5, sizeof(p.dungeon_md5),
             "%s", "6695d2acebce49f95db1d8f3a5c733de");

    CHECK(csb_v1_boot_scan_assets(&p, "/tmp/firestaff-csb-v1-rescan-no-assets") == -1,
          "missing-data rescan fails on a reused profile");
    CHECK(p.assets_verified == 0 &&
          p.graphics_verified == 0 &&
          p.dungeon_verified == 0,
          "missing-data rescan clears all verification flags");
    CHECK(p.graphics_path[0] == '\0' &&
          p.dungeon_path[0] == '\0',
          "missing-data rescan clears stale runtime asset paths");
    CHECK(p.graphics_md5[0] == '\0' &&
          p.dungeon_md5[0] == '\0',
          "missing-data rescan clears stale hash labels");
    CHECK(p.graphics_kind == CSB_V1_ASSET_GFX_ARCHIVE_NONE,
          "missing-data rescan clears stale graphics archive kind");
    CHECK(p.variant_id == CSB_V1_VARIANT_UNKNOWN,
          "missing-data rescan clears stale CSB variant id");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "missing-data rescan returns the profile to PROFILE_READY");
    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "launch remains blocked after stale handoff fields are cleared");
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
    csb_v1_boot_cleanup(&p);
}

static void test_enter_loads_verified_dungeon_context(void)
{
    CSB_V1_BootProfile p;
    const CSB_V1_DungeonData *current;
    const char *fixture_path = "firestaff_csb_v1_handoff_fixture.dat";

    remove(fixture_path);
    CHECK(write_synthetic_dungeon_fixture(fixture_path),
          "synthetic dungeon fixture is written");

    csb_v1_boot_profile_init(&p);
    p.assets_verified = 1;
    p.graphics_verified = 1;
    p.dungeon_verified = 1;
    p.state = CSB_V1_BOOT_STATE_ASSETS_READY;
    p.variant_id = CSB_V1_VARIANT_PC34_EN;
    p.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    snprintf(p.asset_root, sizeof(p.asset_root), "%s", ".");
    snprintf(p.graphics_path, sizeof(p.graphics_path), "%s", "GRAPHICS.DAT");
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", fixture_path);

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game accepts verified file-backed handoff");
    current = csb_v1_dungeon_get_current();
    CHECK(p.runtime.dungeon_handle != NULL,
          "runtime owns a loaded dungeon handle after handoff");
    CHECK(current == p.runtime.dungeon_handle,
          "current dungeon singleton points at the runtime-owned handle");
    CHECK(csb_v1_dungeon_get_current_level() == 0,
          "handoff selects new-game map 0 as the current dungeon level");
    CHECK(current && current->level_count == 1,
          "loaded fixture exposes one dungeon level");
    CHECK(current && csb_v1_dungeon_get_square_type(current, 0, 1, 1) == 3,
          "loaded dungeon data is readable through the current context");

    csb_v1_boot_cleanup(&p);
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "boot cleanup clears the current dungeon context");
    CHECK(p.state == CSB_V1_BOOT_STATE_PROFILE_READY,
          "boot cleanup returns profile to PROFILE_READY");
    remove(fixture_path);
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
    test_rescan_missing_data_clears_stale_handoff();
    test_save_root_override();
    test_enter_requires_assets();
    test_enter_handoff_state();
    test_enter_loads_verified_dungeon_context();
    test_source_evidence();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
