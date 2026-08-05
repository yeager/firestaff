/*
 * firestaff_csb_v1_pc_real_asset_title_import_ui_gate_probe.c
 *
 * CSB V1 real-asset title/import startup UI gate.
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
 *      with an original Utility Disk CMP file, Firestaff must decode
 *      the authentic portrait/name/title record but refuse to invent
 *      the omitted champion stats, possessions and slot. A follow-up
 *      bad-magic CMP must also be rejected and must NOT hand a corrupt
 *      party to the runtime.
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
 *   - ReDMCSB DEFS.H CMP typedef / CEDT001.C F7000 (size 508 bytes).
 *   - ReDMCSB ENTRANCE.C F0806 (CSB entrance).
 *   - ReDMCSB LOADSAVE.C F0435 (new-game map 0).
 *
 * Skip-safe on hosts without user-supplied CSB assets:
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

static const char *csb_data_dir(int argc, char **argv, char *buf, size_t buf_size)
{
    const char *env;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0] != '\0') return argv[1];

    env = getenv("FIRESTAFF_CSB_DATA");
    if (env && env[0] != '\0') return env;

    env = getenv("FIRESTAFF_CSB_PC_DATA");
    if (env && env[0] != '\0') return env;

    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(buf, buf_size, "%s/.firestaff/data/csb", home);
    return buf;
}


static int csb_data_present(const char *dir)
{
    CSB_V1_BootProfile profile;
    if (!dir || dir[0] == '\0') return 0;
    csb_v1_boot_profile_init(&profile);
    return csb_v1_boot_scan_assets(&profile, dir) == 0;
}

static const char *cmp_path(int argc, char **argv, const char *data_dir,
                            char *buf, size_t buf_size)
{
    const char *env;

    if (argc > 2 && argv[2] && argv[2][0] != '\0') return argv[2];
    env = getenv("FIRESTAFF_CSB_CMP");
    if (env && env[0] != '\0') return env;
    if (!data_dir || !buf || buf_size == 0u) return NULL;
    snprintf(buf, buf_size, "%s/PORTRAIT/ALEX.CMP", data_dir);
    return buf;
}

static int read_cmp_file(const char *path, uint8_t out_cmp[FIRESTAFF_CMP_FILE_SIZE])
{
    FILE *file;
    size_t read_count;
    int trailing;

    if (!path || !out_cmp) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    read_count = fread(out_cmp, 1u, FIRESTAFF_CMP_FILE_SIZE, file);
    trailing = fgetc(file);
    fclose(file);
    return read_count == FIRESTAFF_CMP_FILE_SIZE && trailing == EOF;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    char default_cmp_path[1200];
    const char *dir = csb_data_dir(argc, argv, default_dir, sizeof(default_dir));
    const char *real_cmp_path = cmp_path(argc, argv, dir, default_cmp_path,
                                         sizeof(default_cmp_path));
    CSB_V1_BootProfile profile;
    FirestaffCmp decoded_cmp;
    uint8_t cmp_buf[FIRESTAFF_CMP_FILE_SIZE];
    uint8_t bad_cmp_buf[FIRESTAFF_CMP_FILE_SIZE];
    char diag[2048];
    size_t dn;
    int initial_party_x;
    int initial_party_y;
    int initial_party_dir;

    printf("=== CSB V1 real-asset title/import UI gate probe ===\n\n");
    printf("data_dir=%s\n", dir ? dir : "(none)");

    if (!csb_data_present(dir)) {
        printf("SKIP: hash-recognised CSB GRAPHICS.DAT + DUNGEON.DAT are not available; "
               "set FIRESTAFF_CSB_DATA to enable the real-data gate.\n");
        return 0;
    }

    csb_v1_boot_profile_init(&profile);
    CHECK(csb_v1_boot_scan_assets(&profile, dir) == 0,
          "CSB assets scan by hash");
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
    initial_party_x = profile.runtime.party_x;
    initial_party_y = profile.runtime.party_y;
    initial_party_dir = profile.runtime.party_dir;

    /* CMP import handoff through the boot profile. The Atari Utility Disk
     * supplies ALEX.CMP; callers with a different original CMP can pass it
     * as argv[2] or FIRESTAFF_CSB_CMP. */
    if (!read_cmp_file(real_cmp_path, cmp_buf)) {
        printf("SKIP: no original 508-byte CMP at %s\n",
               real_cmp_path ? real_cmp_path : "(none)");
        csb_v1_boot_cleanup(&profile);
        return 0;
    }
    CHECK(1, "original Utility Disk CMP is exactly 508 bytes");
    memset(&decoded_cmp, 0, sizeof(decoded_cmp));
    CHECK(FirestaffCmp_Decode(cmp_buf, sizeof(cmp_buf), &decoded_cmp) == 0 &&
          decoded_cmp.magic == 0x91a7u &&
          decoded_cmp.portrait_size == FIRESTAFF_CMP_PORTRAIT_BYTES,
          "CMP decoder accepts the original Utility Disk record");
    CHECK(strncmp(decoded_cmp.name, "ALEX", FIRESTAFF_CMP_NAME_SIZE) == 0 &&
          strncmp(decoded_cmp.title, "ANDER", FIRESTAFF_CMP_TITLE_SIZE) == 0,
          "original ALEX.CMP name and title decode from source bytes");
    CHECK(csb_v1_boot_set_imported_party_from_cmp(&profile, cmp_buf,
                                                   sizeof(cmp_buf)) != 0,
          "portrait-only CMP cannot manufacture a live party");
    CHECK(profile.cmp_import_attempted == 1 &&
          profile.cmp_import_succeeded == 0 &&
          profile.cmp_imported_slot == -1,
          "CMP receipt records an unbound portrait-only import");
    CHECK(profile.imported_party_ready == 0 &&
          profile.imported_party.ChampionCount == 0,
          "CMP import leaves party state unbound without a real save owner");

    /* A deliberately corrupted copy of the authenticated original must be
     * rejected and must NOT leak into the runtime. */
    memcpy(bad_cmp_buf, cmp_buf, sizeof(bad_cmp_buf));
    bad_cmp_buf[0] = 0x42; bad_cmp_buf[1] = 0x42;
    {
        FirestaffCmp bad_decoded_cmp;
        int rc = csb_v1_boot_set_imported_party_from_cmp(
            &profile, bad_cmp_buf, sizeof(bad_cmp_buf));
        memset(&bad_decoded_cmp, 0, sizeof(bad_decoded_cmp));
        CHECK(FirestaffCmp_Decode(bad_cmp_buf, sizeof(bad_cmp_buf),
                                  &bad_decoded_cmp) == -2,
              "CMP decoder rejects corrupted original magic");
        CHECK(rc != 0,
              "CMP import helper rejects bad-magic buffer");
        CHECK(profile.cmp_import_attempted == 1 &&
              profile.cmp_import_succeeded == 0,
              "boot profile records cmp_import_succeeded = 0 on bad magic");
        CHECK(profile.runtime.party_x == initial_party_x &&
              profile.runtime.party_y == initial_party_y &&
              profile.runtime.party_dir == initial_party_dir,
              "rejected CMP leaves the real dungeon-header start pose unchanged");
    }

    /* Diagnostic report must surface engine version + CMP status */
    dn = csb_v1_boot_diagnostic_report(&profile, diag, sizeof(diag));
    CHECK(dn > 0U && strstr(diag, "engine_version=v2.1") != NULL,
          "diagnostic report surfaces v2.1 (CSB)");
    CHECK(strstr(diag, "flipped=YES") != NULL,
          "diagnostic report shows flipped=YES");
    CHECK(strstr(diag, "cmp_import attempted=YES succeeded=NO") != NULL,
          "diagnostic report shows last CMP attempt (bad magic) succeeded=NO");
    CHECK(strstr(diag, "imported_party_ready=NO") != NULL,
          "diagnostic report keeps portrait-only CMP party state unbound");

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
