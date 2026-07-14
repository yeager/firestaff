/*
 * firestaff_csb_v1_pc_real_asset_title_import_ui_gate_probe.c
 *
 * PC-first CSB V1 real-asset title/import startup UI gate.
 *
 * Companion to the PC real-asset launch probe.  The launch
 * probe proves the boot profile → runtime handoff lands at
 * CSB_STATE_TITLE with a live dungeon.  This probe adds two
 * CSB-specific slices on top of that handoff:
 *
 *   1. Engine version display (CHANGE8_13 + DIALOG.C:2014-2023):
 *      After csb_v1_boot_enter_game() flips the helper to the
 *      CSB value, csb_v1_engine_version_display_get() must
 *      report "v2.1" instead of the data-free DM1 default
 *      "v2.0".  Without the flip, the CSB title surface falls
 *      back to DM1-only behavior on the engine-version line.
 *
 *   2. CMP Utility Disk champion-import handoff
 *      (CEDT002.C + CEDT021.C + CEDTINC7.C):  After a
 *      successful csb_v1_boot_set_imported_party_from_cmp()
 *      with a synthetic CMP buffer, csb_v1_boot_enter_game()
 *      must carry the CMP-derived champion into the runtime
 *      via the existing csb_v1_runtime_set_party_state() path.
 *      A follow-up bad-magic CMP must be rejected and must NOT
 *      hand a corrupt party to the runtime.
 *
 *   3. The diagnostic report must surface both the
 *      engine_version string and the cmp_import status so the
 *      launcher-side log can prove the title/import slice
 *      reached the launch intent.
 *
 * Source-lock boundary:
 *   - ReDMCSB DIALOG.C:2014-2023 + CHANGE8_13 (engine version).
 *   - ReDMCSB CEDT002.C + CEDT021.C + CEDTINC7.C
 *     (Utility Disk Champion Editor → CSB runtime).
 *   - ReDMCSB DEFS.H CMP typedef (size 496 bytes).
 *   - ReDMCSB ENTRANCE.C F0806 (CSB entrance).
 *   - ReDMCSB LOADSAVE.C F0435 (new-game map 0).
 *
 * Skip-safe on hosts without user-supplied PC CSB assets:
 * exits 0 with SKIP if GRAPHICS.DAT + DUNGEON.DAT cannot be
 * scanned under the configured data dir.
 */

#include "csb_v1_boot.h"
#include "csb_v1_cmp_import_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_engine_version_display_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "firestaff_cmp_decode.h"

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

/* Build a synthetic CMP buffer that the csb_v1_cmp_import_to_party
 * decoder accepts.  Mirrors the format documented in
 * ReDMCSB DEFS.H CMP typedef. */
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
    memcpy(buf + 4, name, name_len);
    memcpy(buf + 4 + FIRESTAFF_CMP_NAME_SIZE, title, title_len);
    memset(buf + 4 + FIRESTAFF_CMP_NAME_SIZE + FIRESTAFF_CMP_TITLE_SIZE,
           portrait_fill, 464u);
    return 0;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir = pc_data_dir(argc, argv, default_dir, sizeof(default_dir));
    CSB_V1_BootProfile profile;
    uint8_t cmp_buf[FIRESTAFF_CMP_FILE_SIZE];
    uint8_t bad_cmp_buf[FIRESTAFF_CMP_FILE_SIZE];
    char diag[2048];
    size_t dn;

    printf("=== CSB V1 PC real-asset title/import UI gate probe ===\n\n");
    printf("data_dir=%s\n", dir ? dir : "(none)");

    if (!pc_data_present(dir)) {
        printf("SKIP: PC CSB GRAPHICS.DAT + DUNGEON.DAT not available; "
               "set FIRESTAFF_CSB_PC_DATA to enable the real-data gate.\n");
        return 0;
    }

    csb_v1_boot_profile_init(&profile);
    CHECK(csb_v1_boot_scan_assets(&profile, dir) == 0,
          "PC CSB assets scan by hash");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "engine version helper reports v2.0 before CSB enter_game");

    /* Engine-version flip on enter_game */
    CHECK(csb_v1_boot_enter_game(&profile) == 0,
          "boot profile enters the CSB V1 runtime");
    CHECK(profile.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot profile advances to runtime-ready");
    CHECK(profile.runtime.state == CSB_STATE_TITLE,
          "runtime starts at the title/entrance state");
    CHECK(csb_v1_engine_version_display_is_csb() == 1,
          "engine version helper is CSB after enter_game");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.1") == 0,
          "engine version helper reports v2.1 (CHANGE8_13)");
    CHECK(profile.engine_version_displayed == 1,
          "boot profile records engine_version_displayed = 1");

    /* CMP import handoff through the boot profile */
    CHECK(build_synthetic_cmp(cmp_buf, sizeof(cmp_buf),
                                "HECTOR", "WARRIOR", 0xA5) == 0,
          "synthetic CMP buffer built");
    CHECK(csb_v1_boot_set_imported_party_from_cmp(&profile, cmp_buf,
                                                   sizeof(cmp_buf)) == 0,
          "CMP import helper accepts the synthetic CMP buffer");
    CHECK(profile.cmp_import_attempted == 1 &&
          profile.cmp_import_succeeded == 1,
          "boot profile records cmp_import_attempted + cmp_import_succeeded");
    CHECK(profile.imported_party_ready == 1,
          "imported_party_ready is set so the runtime consumes the CMP party");
    CHECK(profile.imported_party.ChampionCount == 1,
          "imported_party holds one champion after CMP import");
    CHECK(strcmp(profile.imported_party.Champions[0].Name, "HECTOR") == 0,
          "imported_party champion name matches CMP Name field");
    CHECK(strcmp(profile.imported_party.Champions[0].Title, "WARRIOR") == 0,
          "imported_party champion title matches CMP Title field");
    CHECK(profile.imported_party.Champions[0].Portrait[0] == 0xA5,
          "imported_party portrait bytes copied from CMP");

    /* Bad-magic CMP must be rejected and must NOT leak into the runtime */
    CHECK(build_synthetic_cmp(bad_cmp_buf, sizeof(bad_cmp_buf),
                                "HECTOR", "WARRIOR", 0xA5) == 0,
          "synthetic bad-magic CMP buffer built");
    bad_cmp_buf[0] = 0x42; bad_cmp_buf[1] = 0x42;
    {
        int rc = csb_v1_boot_set_imported_party_from_cmp(
            &profile, bad_cmp_buf, sizeof(bad_cmp_buf));
        CHECK(rc != 0,
              "CMP import helper rejects bad-magic buffer");
        CHECK(profile.cmp_import_attempted == 1 &&
              profile.cmp_import_succeeded == 0,
              "boot profile records cmp_import_succeeded = 0 on bad magic");
        CHECK(profile.runtime.party_x == CSB_V1_START_PARTY_X &&
              profile.runtime.party_y == CSB_V1_START_PARTY_Y &&
              profile.runtime.party_dir == CSB_V1_START_PARTY_DIR,
              "runtime keeps source-locked CSB start pose after rejected CMP");
    }

    /* Diagnostic report must surface engine version + CMP status */
    dn = csb_v1_boot_diagnostic_report(&profile, diag, sizeof(diag));
    CHECK(dn > 0U && strstr(diag, "engine_version=v2.1") != NULL,
          "diagnostic report surfaces v2.1 (CSB)");
    CHECK(strstr(diag, "flipped=YES") != NULL,
          "diagnostic report shows flipped=YES");
    CHECK(strstr(diag, "cmp_import attempted=YES succeeded=NO") != NULL,
          "diagnostic report shows last CMP attempt (bad magic) succeeded=NO");
    CHECK(strstr(diag, "imported_party_ready=YES") != NULL,
          "diagnostic report shows imported_party_ready=YES "
          "(from prior good CMP)");

    /* Cleanup must reset the engine version helper back to DM1 */
    csb_v1_boot_cleanup(&profile);
    CHECK(csb_v1_engine_version_display_is_csb() == 0,
          "cleanup resets engine version helper to DM1");
    CHECK(strcmp(csb_v1_engine_version_display_get(), "v2.0") == 0,
          "engine version helper reports v2.0 after cleanup");
    CHECK(profile.engine_version_displayed == 0,
          "boot profile records engine_version_displayed = 0 after cleanup");
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "boot cleanup clears the current dungeon context");

    printf("\nchecks=%d failures=%d\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
