#include "dm2_v1_boot.h"

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
    DM2_V1_BootProfile p;
    dm2_v1_boot_profile_init(&p);
    CHECK(strcmp(p.game_id, "dm2") == 0, "game id is dm2");
    CHECK(p.platform == DM2_PLATFORM_PC_EN, "default platform is PC English");
    CHECK(strcmp(p.platform_label, "PC English") == 0, "platform label is PC English");
    CHECK(strcmp(p.version_id, "pc-en") == 0, "version id is pc-en");
    CHECK(p.deterministic.tick_rate_hz == 18, "V1 tick rate is 18 Hz");
    CHECK(p.deterministic.tick_rate_hz_frac == 2, "fractional tick rate encodes 18.2 Hz");
    CHECK(p.deterministic.tick_ms == 55, "V1 tick quantum is 55 ms");
    CHECK(p.deterministic.dungeon_move_speed == 0x0080, "dungeon move speed Q8=0.5 sq/tick");
    CHECK(p.deterministic.max_champions == 4, "max champions is 4");
    CHECK(p.deterministic.max_party_members == 5, "max party members is 5");
    CHECK(p.deterministic.max_levels == 28, "max levels is 28 (PC English)");
    CHECK(p.deterministic.dungeon_seed == 257, "default dungeon seed is 257");
}

static void test_scan_missing_data(void)
{
    DM2_V1_BootProfile p;
    char diag[1024];
    size_t n;

    dm2_v1_boot_profile_init(&p);
    CHECK(dm2_v1_boot_scan_assets(&p, "/tmp/firestaff-dm2-v1-no-assets") == -1,
          "missing data does not verify");
    CHECK(p.assets_verified == 0, "assets_verified remains false without files");
    CHECK(p.graphics_path[0] == '\0', "graphics_path remains empty");
    CHECK(p.dungeon_path[0] == '\0', "dungeon_path remains empty");
    n = dm2_v1_diagnostic_report(&p, diag, sizeof(diag));
    CHECK(n > 0U && strstr(diag, "DM2 V1 Boot Profile") != NULL,
          "diagnostic report is populated");
}

static void test_probe_available(void)
{
    CHECK(dm2_v1_boot_probe_available("/tmp/firestaff-dm2-v1-no-assets") == 0,
          "probe_available is false without both DM2 assets");
}

static void test_save_root_default(void)
{
    DM2_V1_BootProfile p;
    dm2_v1_boot_profile_init(&p);
    dm2_v1_boot_set_save_root(&p, NULL);
    CHECK(p.save_root[0] != '\0', "save_root is set from asset_root when NULL");
}

static void test_save_root_override(void)
{
    DM2_V1_BootProfile p;
    dm2_v1_boot_profile_init(&p);
    dm2_v1_boot_set_save_root(&p, "/tmp/firestaff-dm2-saves");
    CHECK(strcmp(p.save_root, "/tmp/firestaff-dm2-saves") == 0,
          "explicit save root is preserved");
}

static void test_enter_requires_assets(void)
{
    DM2_V1_BootProfile p;
    dm2_v1_boot_profile_init(&p);
    /* enter_game fails without verified assets (no files found) */
    CHECK(dm2_v1_boot_enter_game(&p) == -1,
          "enter_game rejects unverified profile (no files found)");
}

static void test_source_evidence(void)
{
    const char *e = dm2_v1_boot_source_evidence();
    CHECK(e != NULL && strstr(e, "SKULL.ASM T560") != NULL,
          "source evidence cites SKULL.ASM T560");
    CHECK(e != NULL && strstr(e, "25247ede4dabb6a71e5dabdfbcd5907d") != NULL,
          "source evidence cites DM2 PC English GRAPHICS hash");
}

int main(void)
{
    printf("=== DM2 V1 Boot Profile Smoke Test ===\n\n");
/* ── defaults ── */
    printf("--- test_defaults ---\n");
    test_defaults();
/* ── scan with no assets --─ */
    printf("\n--- test_scan_missing_data ---\n");
    test_scan_missing_data();
/* ── probe availability --─ */
    printf("\n--- test_probe_available ---\n");
    test_probe_available();
/* ── save root --─ */
    printf("\n--- test_save_root_default ---\n");
    test_save_root_default();
    printf("\n--- test_save_root_override ---\n");
    test_save_root_override();
/* ── enter game guard --─ */
    printf("\n--- test_enter_requires_assets ---\n");
    test_enter_requires_assets();
/* ── source evidence --─ */
    printf("\n--- test_source_evidence ---\n");
    test_source_evidence();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
