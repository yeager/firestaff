/*
 * test_csb_v1_boot_title_import_ui_gate_pc34_compat.c
 *
 * CSB V1 title/import startup UI gate.
 *
 * Proves that the CSB boot profile reaches the launch intent
 * without falling back to DM1-only behavior on the
 * title/version surface, and that the CMP Utility Disk
 * champion-import handoff reaches the runtime party slot.
 *
 * Specifically asserts:
 *
 *   1. csb_v1_boot_profile_init() leaves the engine-version
 *      display helper at the DM1 baseline ("v2.0").
 *
 *   2. csb_v1_boot_enter_game() flips the engine-version
 *      display helper to the CSB value ("v2.1") once a
 *      verified profile (graphics + dungeon both verified)
 *      hands off to the runtime.  This closes the gap where
 *      the helper retained its data-free DM1 default ("v2.0")
 *      and the CSB launch fell back to DM1-only behavior on
 *      the version-display surface even though the runtime
 *      variant was CSB_V1_VARIANT_PC34_EN.
 *
 *   3. csb_v1_boot_cleanup() resets the engine-version display
 *      helper back to "v2.0" so a follow-up DM1 launch does
 *      not see a stale "v2.1" string from the prior CSB
 *      handoff.
 *
 *   4. csb_v1_boot_set_imported_party_from_cmp() on a valid
 *      synthetic CMP buffer increments the boot profile's
 *      imported_party.ChampionCount, sets imported_party_ready
 *      to 1, and the helper's cmp_import_attempted +
 *      cmp_import_succeeded flags report a successful import.
 *
 *   5. csb_v1_boot_set_imported_party_from_cmp() on a buffer
 *      with a bad CMP magic sets cmp_import_attempted to 1,
 *      leaves cmp_import_succeeded at 0, and leaves
 *      imported_party_ready at 0 so the runtime does not
 *      consume a corrupt import.
 *
 *   6. csb_v1_boot_diagnostic_report() surfaces both the
 *      engine-version string ("v2.0" or "v2.1") and the
 *      CMP-import status so a launcher-side log proves the
 *      CSB handoff reached the title surface.
 *
 * Source-locks:
 *   ReDMCSB DIALOG.C:2014-2023 (engine version displayed in
 *     top-right of dialog boxes).
 *   ReDMCSB CHANGE8_13 (CSB engine version 2.1 hardcoded).
 *   ReDMCSB CEDT002.C + CEDT021.C + CEDTINC7.C
 *     (Utility Disk Champion Editor → CSB runtime handoff).
 *   ReDMCSB DEFS.H CMP typedef (size 496 bytes).
 *   CSBWin/CedtData.cpp (CSB Utility Disk tool flow).
 *
 * Data-free: no real DM1/CSB assets required.
 */

#include "csb_v1_boot.h"
#include "csb_v1_cmp_import_pc34_compat.h"
#include "csb_v1_engine_version_display_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_game_state_pc34_compat.h"
#include "firestaff_cmp_decode.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else      { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

/* The decoder accepts the 464-byte portrait payload in the CMP
 * record. Keep the fixture size local to the test so it does not
 * depend on implementation-private constants. */
#ifndef CMP_PORTRAIT_BYTES_FOR_TEST
#define CMP_PORTRAIT_BYTES_FOR_TEST 464u
#endif

/* Build a synthetic CMP file with a recognisable Name/Title/portrait
 * pattern.  Mirrors the fixture used by
 * csb_v1_cmp_import_self_test but with a different sentinel
 * pattern so the two paths can be told apart. */
static int build_synthetic_cmp(uint8_t *buf, size_t buf_size,
                                const char *name, const char *title,
                                uint8_t portrait_fill)
{
    size_t name_len;
    size_t title_len;
    if (!buf || buf_size < FIRESTAFF_CMP_FILE_SIZE) return -1;
    if (!name || !title) return -1;
    name_len = strlen(name);
    title_len = strlen(title);
    if (name_len == 0 || name_len > FIRESTAFF_CMP_NAME_SIZE) return -1;
    if (title_len == 0 || title_len > FIRESTAFF_CMP_TITLE_SIZE) return -1;
    memset(buf, 0, buf_size);
    /* cmp_i_C + cmp_i_E are already 0 */
    memcpy(buf + 4, name, name_len);
    memcpy(buf + 4 + FIRESTAFF_CMP_NAME_SIZE, title, title_len);
    memset(buf + 4 + FIRESTAFF_CMP_NAME_SIZE + FIRESTAFF_CMP_TITLE_SIZE,
           portrait_fill, CMP_PORTRAIT_BYTES_FOR_TEST);
    return 0;
}

static void prime_verified_profile(CSB_V1_BootProfile *p)
{
    csb_v1_boot_profile_init(p);
    p->assets_verified = 1;
    p->graphics_verified = 1;
    p->dungeon_verified = 1;
    p->state = CSB_V1_BOOT_STATE_ASSETS_READY;
    p->variant_id = CSB_V1_VARIANT_PC34_EN;
    p->graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    snprintf(p->asset_root, sizeof(p->asset_root), "%s", ".");
    snprintf(p->graphics_path, sizeof(p->graphics_path), "%s", "GRAPHICS.DAT");
    snprintf(p->dungeon_path, sizeof(p->dungeon_path), "%s", "DUNGEON.DAT");
    csb_v1_boot_set_save_root(p, "/tmp/firestaff-csb-v1-title-import-gate");
}

static void test_profile_init_resets_engine_version_to_dm1(void)
{
    /* Force a stale "v2.1" so the init path has to actively
     * reset to "v2.0". */
    csb_v1_engine_version_display_set_csb(1);
    CHECK(csb_v1_engine_version_display_is_csb() == 1,
          "precondition: helper forced to CSB");
    {
        CSB_V1_BootProfile p;
        csb_v1_boot_profile_init(&p);
        CHECK(csb_v1_engine_version_display_is_csb() == 0,
              "csb_v1_boot_profile_init resets helper to DM1");
        CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
              "engine version display reports v2.0 after profile_init");
        CHECK(p.engine_version_displayed == 0,
              "boot profile marks engine version not yet displayed");
        csb_v1_boot_cleanup(&p);
    }
}

static void test_enter_game_flips_engine_version_to_csb(void)
{
    CSB_V1_BootProfile p;
    prime_verified_profile(&p);
    /* Precondition: helper still at DM1 baseline */
    CHECK(csb_v1_engine_version_display_is_csb() == 0,
          "precondition: helper is at DM1 baseline before enter_game");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "precondition: helper reports v2.0 before enter_game");
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "csb_v1_boot_enter_game accepts a verified profile");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot profile advances to RUNTIME_READY");
    CHECK(p.runtime.state == CSB_STATE_TITLE,
          "runtime starts at CSB_STATE_TITLE");
    /* The actual flip */
    CHECK(csb_v1_engine_version_display_is_csb() == 1,
          "enter_game flips helper to CSB (CHANGE8_13)");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.1") == 0,
          "engine version display reports v2.1 after CSB enter_game");
    CHECK(p.engine_version_displayed == 1,
          "boot profile records engine_version_displayed = 1");
    csb_v1_boot_cleanup(&p);
}

static void test_cleanup_resets_engine_version_to_dm1(void)
{
    CSB_V1_BootProfile p;
    prime_verified_profile(&p);
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "verified profile enters the CSB runtime");
    CHECK(csb_v1_engine_version_display_is_csb() == 1,
          "precondition: helper is at CSB after enter_game");
    csb_v1_boot_cleanup(&p);
    CHECK(csb_v1_engine_version_display_is_csb() == 0,
          "cleanup resets helper back to DM1");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "cleanup returns helper string to v2.0");
    CHECK(p.engine_version_displayed == 0,
          "boot profile records engine_version_displayed = 0 after cleanup");
    /* csb_v1_boot_reset_engine_version_to_dm1() should be idempotent
     * when called after cleanup. */
    csb_v1_boot_reset_engine_version_to_dm1();
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "csb_v1_boot_reset_engine_version_to_dm1() is idempotent");
}

static void test_enter_game_without_assets_leaves_version_at_dm1(void)
{
    CSB_V1_BootProfile p;
    csb_v1_boot_profile_init(&p);
    /* Precondition: helper at DM1 baseline */
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "precondition: helper at DM1 baseline");
    CHECK(csb_v1_boot_enter_game(&p) == -1,
          "enter_game rejects an unverified profile");
    CHECK(csb_v1_engine_version_display_is_csb() == 0,
          "failed enter_game leaves helper at DM1");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "engine version still reports v2.0 after failed enter_game");
    csb_v1_boot_cleanup(&p);
}

static void test_cmp_import_reaches_boot_party_slot(void)
{
    CSB_V1_BootProfile p;
    uint8_t cmp_buf[FIRESTAFF_CMP_FILE_SIZE];

    prime_verified_profile(&p);
    CHECK(p.cmp_import_attempted == 0,
          "precondition: cmp_import_attempted starts at 0");
    CHECK(p.cmp_import_succeeded == 0,
          "precondition: cmp_import_succeeded starts at 0");
    CHECK(p.imported_party_ready == 0,
          "precondition: imported_party_ready starts at 0");
    CHECK(build_synthetic_cmp(cmp_buf, sizeof(cmp_buf),
                                "HECTOR", "WARRIOR", 0xA5) == 0,
          "synthetic CMP buffer is built");
    CHECK(csb_v1_boot_set_imported_party_from_cmp(&p, cmp_buf,
                                                   sizeof(cmp_buf)) == 0,
          "CMP import helper returns 0 on success");
    CHECK(p.cmp_import_attempted == 1,
          "cmp_import_attempted is 1 after import");
    CHECK(p.cmp_import_succeeded == 1,
          "cmp_import_succeeded is 1 after successful import");
    CHECK(p.cmp_imported_slot == 0,
          "cmp_imported_slot reports the destination slot index");
    CHECK(p.cmp_imported_champion_count == 1,
          "cmp_imported_champion_count reflects imported_party count");
    CHECK(p.imported_party.ChampionCount == 1,
          "imported_party now holds one champion");
    CHECK(strcmp(p.imported_party.Champions[0].Name, "HECTOR") == 0,
          "imported_party champion name matches the CMP file");
    CHECK(strcmp(p.imported_party.Champions[0].Title, "WARRIOR") == 0,
          "imported_party champion title matches the CMP file");
    CHECK(p.imported_party.Champions[0].Portrait[0] == 0xA5,
          "imported_party champion portrait bytes copied");
    CHECK(p.imported_party_ready == 1,
          "imported_party_ready is set so enter_game hands it to runtime");
    /* Now run enter_game and verify the runtime receives the CMP
     * champion via the existing imported_party_ready path. */
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game accepts the profile with CMP-imported party");
    CHECK(p.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot profile reaches RUNTIME_READY with CMP party");
    csb_v1_boot_cleanup(&p);
}

static void test_cmp_import_bad_magic_rejected(void)
{
    CSB_V1_BootProfile p;
    uint8_t cmp_buf[FIRESTAFF_CMP_FILE_SIZE];

    prime_verified_profile(&p);
    CHECK(build_synthetic_cmp(cmp_buf, sizeof(cmp_buf),
                                "HECTOR", "WARRIOR", 0xA5) == 0,
          "synthetic CMP buffer is built");
    /* Corrupt the CMP magic. */
    cmp_buf[0] = 0x42; cmp_buf[1] = 0x42;
    int rc = csb_v1_boot_set_imported_party_from_cmp(&p, cmp_buf,
                                                       sizeof(cmp_buf));
    CHECK(rc != 0,
          "CMP import helper rejects bad magic");
    CHECK(p.cmp_import_attempted == 1,
          "cmp_import_attempted records the attempt even on bad magic");
    CHECK(p.cmp_import_succeeded == 0,
          "cmp_import_succeeded stays 0 on bad magic");
    CHECK(p.imported_party_ready == 0,
          "imported_party_ready stays 0 so runtime does not consume bad CMP");
    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "enter_game still succeeds without a CMP import");
    /* The runtime party must remain at the source-locked start pose
     * because imported_party_ready == 0.  This proves the runtime did
     * NOT fall back to consuming the corrupt CMP bytes. */
    CHECK(p.runtime.party_x == CSB_V1_START_PARTY_X &&
          p.runtime.party_y == CSB_V1_START_PARTY_Y &&
          p.runtime.party_dir == CSB_V1_START_PARTY_DIR,
          "runtime keeps source-locked CSB start pose after rejected CMP");
    csb_v1_boot_cleanup(&p);
}

static void test_mark_imported_party_ready_uses_existing_party(void)
{
    CSB_V1_BootProfile p;
    CSB_V1_PartyState party;

    prime_verified_profile(&p);
    memset(&party, 0, sizeof(party));
    party.ChampionCount = 1;
    memcpy(party.Champions[0].Name, "ALPHA", 6);
    CHECK(csb_v1_boot_set_imported_party(&p, &party) == 0,
          "boot profile carries an externally-prepared imported party");
    CHECK(p.cmp_import_attempted == 0,
          "precondition: cmp_import_attempted still 0");
    CHECK(p.imported_party_ready == 1,
          "set_imported_party flips imported_party_ready to 1");
    CHECK(csb_v1_boot_mark_imported_party_ready(&p) == 0,
          "mark_imported_party_ready is idempotent");
    CHECK(p.cmp_import_attempted == 1,
          "mark_imported_party_ready records the attempt flag");
    csb_v1_boot_cleanup(&p);
}

static void test_diagnostic_report_surfaces_title_import_status(void)
{
    CSB_V1_BootProfile p;
    char diag[2048];
    size_t n;

    prime_verified_profile(&p);
    /* Capture DM1-baseline diagnostic */
    n = csb_v1_boot_diagnostic_report(&p, diag, sizeof(diag));
    CHECK(n > 0U && strstr(diag, "engine_version=v2.0") != NULL,
          "diagnostic report surfaces v2.0 baseline before enter_game");
    CHECK(strstr(diag, "engine_version=v2.0") != NULL &&
          strstr(diag, "flipped=NO") != NULL,
          "diagnostic report shows engine_version + flipped=NO before enter_game");
    CHECK(strstr(diag, "cmp_import attempted=NO succeeded=NO") != NULL,
          "diagnostic report shows cmp_import status before any attempt");

    CHECK(csb_v1_boot_enter_game(&p) == 0,
          "verified profile enters the CSB runtime");
    n = csb_v1_boot_diagnostic_report(&p, diag, sizeof(diag));
    CHECK(n > 0U && strstr(diag, "engine_version=v2.1") != NULL,
          "diagnostic report surfaces v2.1 after CSB enter_game");
    CHECK(strstr(diag, "flipped=YES") != NULL,
          "diagnostic report shows flipped=YES after enter_game");

    csb_v1_boot_cleanup(&p);
    n = csb_v1_boot_diagnostic_report(&p, diag, sizeof(diag));
    CHECK(n > 0U && strstr(diag, "engine_version=v2.0") != NULL,
          "diagnostic report returns to v2.0 after cleanup");
}

static void test_source_evidence(void)
{
    const char *e = csb_v1_boot_source_evidence();
    CHECK(e && strstr(e, "ENTRANCE.C F0806") != NULL,
          "boot source evidence cites ENTRANCE.C F0806");
    CHECK(e && strstr(e, "LOADSAVE.C F0435") != NULL,
          "boot source evidence cites LOADSAVE.C F0435");
}

int main(void)
{
    printf("=== CSB V1 Title/Import Startup UI Gate ===\n\n");
    test_profile_init_resets_engine_version_to_dm1();
    test_enter_game_flips_engine_version_to_csb();
    test_cleanup_resets_engine_version_to_dm1();
    test_enter_game_without_assets_leaves_version_at_dm1();
    test_cmp_import_reaches_boot_party_slot();
    test_cmp_import_bad_magic_rejected();
    test_mark_imported_party_ready_uses_existing_party();
    test_diagnostic_report_surfaces_title_import_status();
    test_source_evidence();
    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
