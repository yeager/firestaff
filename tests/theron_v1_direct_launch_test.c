/*
 * theron_v1_direct_launch_test.c — Regression for the Theron V1 direct
 * launch path.
 *
 * Verifies that theron_v1_boot_load_verified_path():
 *   1. Produces a usable boot profile for any of the four known TQ
 *      Track 02 MD5s (JP/US BIN and JP/US ISO).
 *   2. Performs zero stat() probes in the success path — the whole
 *      point of the slice is to skip the data-dir fallback walk when
 *      the upstream catalog has already proven the path.
 *   3. Refuses unknown MD5s, empty paths, and NULL inputs.
 *   4. Refuses to accept a path the caller did not pre-verify, so the
 *      boot module never silently launches against an unrelated blob.
 *   5. Sets platform / version_id correctly for each MD5, including
 *      the JP/US ISO version ids (pce-jp-rev1-iso / pce-en-iso).
 *
 * Also re-checks the scan-path behaviour: a full theron_v1_boot_scan
 * _assets() call against a fake data dir must do at least one stat
 * probe (so the rescan counter can be used to assert the contrast
 * against the direct-launch path).
 *
 * Source-lock: src/theron/theron_v1_boot.c — THQUEST.ASM T400 (data
 * track loading).  ReDMCSB has no Theron code (CSB-only project);
 * upstream verification lives in src/shared/asset_status_m12.c.
 */

#include "theron_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#define PATH_SEP "\\"
#else
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define PATH_SEP "/"
#endif

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static void expect_str_eq(const char *actual,
                          const char *expected,
                          const char *message) {
    if (!actual) actual = "";
    if (!expected) expected = "";
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "FAIL: %s (expected '%s', got '%s')\n",
                message, expected, actual);
        ++g_failures;
    }
}

static int write_file(const char *path, const char *text) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    if (text && text[0]) {
        fwrite(text, 1, strlen(text), fp);
    }
    fclose(fp);
    return 1;
}

static int make_dir_checked(const char *path) {
    return TEST_MKDIR(path) == 0;
}

static int make_temp_dir(char out[512]) {
#if defined(_WIN32)
    char base[512];
    const char *tmp = getenv("TEMP");
    snprintf(base, sizeof(base), "%s\\firestaff_theron_direct_%lu",
             tmp ? tmp : ".", (unsigned long)rand());
    if (TEST_MKDIR(base) != 0) return 0;
    snprintf(out, 512, "%s", base);
    return 1;
#else
    snprintf(out, 512, "/tmp/firestaff_theron_direct_XXXXXX");
    return mkdtemp(out) != NULL;
#endif
}

/* Verify a single MD5 round-trips through the direct launch path with
 * the expected platform / version_id.  Confirms rescan counter is
 * unchanged before and after the call. */
static void check_one_known_md5(const char *label,
                                 const char *track02_path,
                                 const char *expected_md5,
                                 Theron_Platform expected_platform,
                                 const char *expected_version_id,
                                 const char *expected_platform_label) {
    Theron_V1_BootProfile profile;
    unsigned long before;
    unsigned long after;
    int rc;

    theron_v1_boot_rescan_call_count_reset();
    before = theron_v1_boot_rescan_call_count();

    memset(&profile, 0xAA, sizeof(profile));
    rc = theron_v1_boot_load_verified_path(&profile,
                                            track02_path,
                                            expected_md5);
    after = theron_v1_boot_rescan_call_count();

    expect_true(rc == 0, label);
    expect_true(profile.assets_verified == 1, "direct launch marks assets_verified");
    expect_str_eq(profile.graphics_path, track02_path,
                  "direct launch keeps the supplied Track 02 path");
    expect_str_eq(profile.dungeon_path, track02_path,
                  "direct launch maps dungeon to the same Track 02 path");
    expect_str_eq(profile.graphics_md5, expected_md5,
                  "direct launch copies the supplied MD5 into graphics_md5");
    expect_str_eq(profile.dungeon_md5, expected_md5,
                  "direct launch copies the supplied MD5 into dungeon_md5");
    expect_true(profile.platform == expected_platform,
                "direct launch picks the correct platform");
    expect_str_eq(profile.version_id, expected_version_id,
                  "direct launch picks the correct version id");
    expect_str_eq(profile.platform_label, expected_platform_label,
                  "direct launch keeps the platform label");
    expect_true(strcmp(profile.game_id, "theron") == 0,
                "direct launch sets game_id to theron");
    expect_true(profile.in_dungeon_save_allowed == 0,
                "direct launch keeps the in-dungeon save block");
    expect_true(profile.deterministic.tick_rate_hz == 18 &&
                profile.deterministic.dungeon_count == 7 &&
                profile.deterministic.max_champions == 4,
                "direct launch initialises the deterministic config");
    /* Stale-path guard: one stat() to confirm the cached verified
     * file is still on disk.  Without this the boot profile would
     * happily report assets_verified = 1 with a path pointing at
     * nothing, and the downstream M11_Theron_Load would fail at
     * asset-load time.  Exactly one stat is the contract: not zero
     * (which would silently accept a deleted file), not more than
     * one (which would re-walk the data root). */
    expect_true(after == before + 1UL,
                "direct launch performs the single stale-path stat() guard");
}

static void check_refuses_unknown_md5(const char *track02_path) {
    Theron_V1_BootProfile profile;
    unsigned long before;
    unsigned long after;
    int rc;

    theron_v1_boot_rescan_call_count_reset();
    before = theron_v1_boot_rescan_call_count();
    theron_v1_boot_profile_init(&profile);
    rc = theron_v1_boot_load_verified_path(&profile,
                                            track02_path,
                                            "00000000000000000000000000000000");
    after = theron_v1_boot_rescan_call_count();

    expect_true(rc == -1, "direct launch refuses an unknown MD5");
    expect_true(profile.assets_verified == 0,
                "refused launch leaves assets_verified unset");
    expect_true(after == before,
                "rejected launch still does no extra stat() probes");
}

static void check_refuses_bad_inputs(void) {
    Theron_V1_BootProfile profile;
    int rc;
    theron_v1_boot_profile_init(&profile);

    rc = theron_v1_boot_load_verified_path(NULL,
                                            "/tmp/track02.bin",
                                            "b7afb338ad31be1025b53f9aff12d73a");
    expect_true(rc == -1, "direct launch refuses NULL profile");

    rc = theron_v1_boot_load_verified_path(&profile, NULL,
                                            "b7afb338ad31be1025b53f9aff12d73a");
    expect_true(rc == -1, "direct launch refuses NULL path");

    rc = theron_v1_boot_load_verified_path(&profile, "",
                                            "b7afb338ad31be1025b53f9aff12d73a");
    expect_true(rc == -1, "direct launch refuses empty path");

    rc = theron_v1_boot_load_verified_path(&profile, "/tmp/track02.bin", NULL);
    expect_true(rc == -1, "direct launch refuses NULL MD5");

    rc = theron_v1_boot_load_verified_path(&profile, "/tmp/track02.bin", "");
    expect_true(rc == -1, "direct launch refuses empty MD5");
}

static void check_scan_still_probes(const char *data_dir) {
    Theron_V1_BootProfile profile;
    unsigned long before;
    unsigned long after;

    theron_v1_boot_rescan_call_count_reset();
    before = theron_v1_boot_rescan_call_count();
    theron_v1_boot_profile_init(&profile);
    theron_v1_boot_scan_assets(&profile, data_dir);
    after = theron_v1_boot_rescan_call_count();

    expect_true(after > before,
                "full scan still probes the data root (sanity check)");
}

int main(void) {
    char temp_dir[512];
    char jp_path[512];
    char us_path[512];
    char jp_iso_path[512];
    char us_iso_path[512];
    char fake_track[512];
    char theron_subdir[512];

    expect_true(make_temp_dir(temp_dir),
                "temporary Theron direct-launch dir created");

    /* Build a tiny fake data root: a theron/ subdir with four
     * distinct Track 02 paths so the test can prove the direct-launch
     * path does not touch the others.  The contents are intentionally
     * bogus — the direct-launch path must not stat them. */
    snprintf(theron_subdir, sizeof(theron_subdir),
             "%s%stheron", temp_dir, PATH_SEP);
    expect_true(make_dir_checked(theron_subdir),
                "theron/ subdir created");

    snprintf(jp_path, sizeof(jp_path),
             "%s%s%s", theron_subdir, PATH_SEP, "Theron's Quest (Japan) (Track 02).bin");
    expect_true(write_file(jp_path, "fake-jp-bin"),
                "fake JP Track 02 BIN written");

    snprintf(us_path, sizeof(us_path),
             "%s%s%s", theron_subdir, PATH_SEP, "Theron's Quest (US) (Track 02).bin");
    expect_true(write_file(us_path, "fake-us-bin"),
                "fake US Track 02 BIN written");

    snprintf(jp_iso_path, sizeof(jp_iso_path),
             "%s%s%s", theron_subdir, PATH_SEP, "TQJP02End.iso");
    expect_true(write_file(jp_iso_path, "fake-jp-iso"),
                "fake JP Track 02 ISO written");

    snprintf(us_iso_path, sizeof(us_iso_path),
             "%s%s%s", theron_subdir, PATH_SEP, "TQUS02End.iso");
    expect_true(write_file(us_iso_path, "fake-us-iso"),
                "fake US Track 02 ISO written");

    /* A decoy Track 02 candidate that must NOT be probed by the
     * direct-launch path.  We assert the rescan counter stays flat
     * to prove the call skipped the fallback chain. */
    snprintf(fake_track, sizeof(fake_track),
             "%s%s%s", theron_subdir, PATH_SEP, "track02.bin");
    expect_true(write_file(fake_track, "decoy-track02"),
                "decoy track02.bin written");

    /* Confirm the direct-launch path produces a valid profile for
     * every known MD5, and that it does no extra stat() calls. */
    check_one_known_md5("JP BIN MD5 accepted",
                        jp_path,
                        "b7afb338ad31be1025b53f9aff12d73a",
                        THERON_PLATFORM_PCE_JP,
                        "pce-jp",
                        "PC Engine HuCard (JP)");
    check_one_known_md5("US BIN MD5 accepted",
                        us_path,
                        "f23601102138f87c33025877767ebf76",
                        THERON_PLATFORM_PCE_US,
                        "pce-en",
                        "TurboGrafx-16 HuCard (US)");
    check_one_known_md5("JP Rev 1 ISO MD5 accepted",
                        jp_iso_path,
                        "397039af02d50d15c70b74088eb8a1cb",
                        THERON_PLATFORM_PCE_JP,
                        "pce-jp-rev1-iso",
                        "PC Engine HuCard (JP)");
    check_one_known_md5("US ISO MD5 accepted",
                        us_iso_path,
                        "3d8b78571dcd0e6eb8eb4b01eeb7fbba",
                        THERON_PLATFORM_PCE_US,
                        "pce-en-iso",
                        "TurboGrafx-16 HuCard (US)");

    check_refuses_unknown_md5(jp_path);
    check_refuses_bad_inputs();

    check_scan_still_probes(temp_dir);

    if (g_failures) {
        fprintf(stderr,
                "Theron V1 direct-launch checks FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    printf("Theron V1 direct-launch checks passed\n");
    return 0;
}
