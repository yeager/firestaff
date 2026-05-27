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
    test_source_evidence();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
