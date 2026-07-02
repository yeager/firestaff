#include "asset_status_m12.h"
#include "firestaff_data_validator.h"
#include "firestaff_startup.h"
#include "theron_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#define PATH_SEP "\\"
#else
#include <unistd.h>
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
    if (!actual) {
        actual = "";
    }
    if (!expected) {
        expected = "";
    }
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "FAIL: %s (expected '%s', got '%s')\n",
                message,
                expected,
                actual);
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

static int write_synthetic_iso_pvd(const char *path) {
    static const unsigned char pvd[6] = {0x01, 'C', 'D', '0', '0', '1'};
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fseek(fp, 16L * 2048L, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    if (fwrite(pvd, 1, sizeof(pvd), fp) != sizeof(pvd)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int make_dir_checked(const char *path) {
    return mkdir(path, 0700) == 0;
}

static int make_temp_dir(char out[512]) {
#if defined(_WIN32)
    char base[512];
    const char *tmp = getenv("TEMP");
    snprintf(base, sizeof(base), "%s\\firestaff_theron_avail_%lu",
             tmp ? tmp : ".", (unsigned long)rand());
    if (mkdir(base, 0700) != 0) return 0;
    snprintf(out, 512, "%s", base);
    return 1;
#else
    snprintf(out, 512, "/tmp/firestaff_theron_avail_XXXXXX");
    return mkdtemp(out) != NULL;
#endif
}

static void check_real_theron_launch_marker_presence(const char *real_data) {
    M12_AssetStatus status;
    M12_AssetStatus direct_status;
    FS_ValidationReport report;
    FS_GameAvailability availability;
    const M12_AssetVersionStatus *matched_version = NULL;
    const M12_AssetRequiredFileStatus *required;
    size_t vi;

    if (!real_data || !real_data[0]) {
        return;
    }

    M12_AssetStatus_Scan(&status, real_data);

    if (status.theronAvailable == 0) {
        expect_true(fs_validate_data_dir(real_data, &report) >= 0 &&
                    report.theron_ready == 0,
                    "explicit missing/invalid real-data candidate is not hash-verified");
        fs_startup_check_games(real_data, &availability);
        expect_true(availability.theron_available == 0,
                    "missing/invalid real-data candidate is not launch-ready");
        expect_true(theron_v1_boot_probe_available(real_data) == 0,
                    "missing/invalid real-data candidate is blocked by quick probe");
        return;
    }

    expect_true(fs_validate_data_dir(real_data, &report) >= 0 &&
                report.theron_ready == 1,
                "validator accepts hash-verified real Theron data");

    fs_startup_check_games(real_data, &availability);
    expect_true(availability.theron_available == 1,
                "startup accepts hash-verified Theron data");
    expect_true(theron_v1_boot_probe_available(real_data) == 1,
                "Theron quick boot probe accepts hash-verified data");

    for (vi = 0; vi < M12_AssetStatus_GetVersionCount("theron"); ++vi) {
        const M12_AssetVersionStatus *v =
            M12_AssetStatus_GetVersion(&status, "theron", vi);
        if (v && v->matched) {
            matched_version = v;
            break;
        }
    }
    expect_true(matched_version != NULL,
                "real Theron Track 02 candidate resolves at least one catalog marker");

    required = M12_AssetStatus_GetRequiredFile(&status, "theron", 0);
    expect_true(required && required->required && required->matched,
                "real Theron Track 02 marker satisfies launch gate required file");
    if (matched_version && matched_version->matchedPath[0]) {
        expect_true(strcmp(matched_version->matchedPath,
                           required->matchedPath) == 0,
                    "real Theron marker match is mirrored onto required file path");

        if (strstr(matched_version->matchedPath, "::") == NULL) {
            M12_AssetStatus_Scan(&direct_status, matched_version->matchedPath);
            expect_true(direct_status.theronAvailable == 1,
                        "explicit Theron Track 02 path is accepted");
            required = M12_AssetStatus_GetRequiredFile(&direct_status,
                                                       "theron",
                                                       0);
            expect_true(required && required->matched &&
                        strcmp(required->matchedPath,
                               matched_version->matchedPath) == 0,
                        "explicit Theron Track 02 path stays the verified match");
            expect_true(strcmp(M12_AssetStatus_GetRuntimeDataDir(&direct_status,
                                                               "theron"),
                               matched_version->matchedPath) != 0,
                        "explicit Theron Track 02 path resolves to a runtime root");
            if (strcmp(matched_version->matchedMd5,
                       "397039af02d50d15c70b74088eb8a1cb") == 0) {
                expect_str_eq(matched_version->versionId,
                              "pce-jp-rev1-iso",
                              "real JP Rev 1 ISO selects ISO version id");
            } else if (strcmp(matched_version->matchedMd5,
                              "3d8b78571dcd0e6eb8eb4b01eeb7fbba") == 0) {
                expect_str_eq(matched_version->versionId,
                              "pce-en-iso",
                              "real US ISO selects ISO version id");
            }
        }
    }
}

static void check_fake_iso_fallback(const char *region_dir,
                                    const char *iso_name,
                                    Theron_Platform expected_platform,
                                    const char *expected_version_id,
                                    const char *label) {
    char temp_dir[512];
    char theron_dir[512];
    char variant_dir[512];
    char iso_path[512];
    Theron_V1_BootProfile profile;
    M12_AssetStatus status;
    const M12_AssetRequiredFileStatus *required;

    expect_true(make_temp_dir(temp_dir), "temporary Theron ISO data dir created");
    snprintf(theron_dir, sizeof(theron_dir), "%s%s%s", temp_dir, PATH_SEP, "theron");
    expect_true(make_dir_checked(theron_dir), "theron ISO subdir created");
    snprintf(variant_dir, sizeof(variant_dir), "%s%s%s", theron_dir, PATH_SEP, region_dir);
    expect_true(make_dir_checked(variant_dir), "theron ISO region subdir created");
    snprintf(iso_path, sizeof(iso_path), "%s%s%s", variant_dir, PATH_SEP, iso_name);
    expect_true(write_file(iso_path, "not a known Theron's Quest Track 02 ISO"),
                "fake Theron ISO candidate written");

    theron_v1_boot_profile_init(&profile);
    expect_true(theron_v1_boot_scan_assets(&profile, temp_dir) == 0,
                label);
    expect_true(profile.assets_verified == 0,
                "fake Theron ISO fallback remains unverified");
    expect_true(profile.platform == expected_platform,
                "fake Theron ISO fallback selects expected platform");
    expect_str_eq(profile.version_id,
                  expected_version_id,
                  "fake Theron ISO fallback selects expected version");
    expect_str_eq(profile.graphics_path,
                  iso_path,
                  "fake Theron ISO fallback keeps selected Track 02 path");
    expect_str_eq(profile.dungeon_path,
                  iso_path,
                  "fake Theron ISO fallback maps dungeon to Track 02 path");
    expect_true(theron_v1_boot_probe_available(temp_dir) == 0,
                "fake Theron ISO fallback does not pass availability probe");

    M12_AssetStatus_Scan(&status, iso_path);
    expect_true(status.originalFileCandidateFound == 1,
                "explicit Theron ISO candidate counts as original-file evidence");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
                "explicit fake Theron ISO scan remains unavailable without known MD5");
    expect_str_eq(M12_AssetStatus_GetDataDir(&status),
                  variant_dir,
                  "explicit fake Theron ISO scan resolves to containing data dir");
    expect_str_eq(M12_AssetStatus_GetRuntimeDataDir(&status, "theron"),
                  variant_dir,
                  "explicit fake Theron ISO scan keeps runtime dir isolated");
    required = M12_AssetStatus_GetRequiredFile(&status, "theron", 0);
    expect_true(required && required->required && !required->matched,
                "explicit fake Theron ISO scan leaves required Track 02 unmatched");
}

static void check_explicit_track02_does_not_use_sibling_fallback(void) {
    char temp_dir[512];
    char direct_dir[512];
    char explicit_track[512];
    char theron_dir[512];
    char us_dir[512];
    char fallback_track[512];
    M12_AssetStatus status;
    FS_GameAvailability availability;
    const M12_AssetRequiredFileStatus *required;
    size_t vi;

    expect_true(make_temp_dir(temp_dir), "temporary explicit Theron data dir created");
    snprintf(direct_dir, sizeof(direct_dir), "%s%s%s", temp_dir, PATH_SEP, "direct");
    expect_true(make_dir_checked(direct_dir), "explicit Theron direct dir created");
    snprintf(explicit_track, sizeof(explicit_track), "%s%s%s",
             direct_dir, PATH_SEP, "track02.bin");
    expect_true(write_file(explicit_track,
                           "not the requested Theron's Quest Track 02 data"),
                "explicit fake Track 02 candidate written");

    snprintf(theron_dir, sizeof(theron_dir), "%s%s%s", temp_dir, PATH_SEP, "theron");
    expect_true(make_dir_checked(theron_dir), "sibling Theron fallback dir created");
    snprintf(us_dir, sizeof(us_dir), "%s%s%s", theron_dir, PATH_SEP, "us");
    expect_true(make_dir_checked(us_dir), "sibling Theron US fallback dir created");
    snprintf(fallback_track, sizeof(fallback_track), "%s%s%s",
             us_dir, PATH_SEP, "TQUS02End.iso");
    expect_true(write_file(fallback_track,
                           "fallback-looking but unverified Theron's Quest Track 02 ISO"),
                "sibling fake Theron ISO fallback written");

    M12_AssetStatus_Scan(&status, explicit_track);
    expect_true(status.originalFileCandidateFound == 1,
                "explicit Theron direct path is original-file evidence");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
                "explicit non-matching Theron direct path remains unavailable");
    expect_str_eq(M12_AssetStatus_GetDataDir(&status),
                  direct_dir,
                  "explicit Theron direct path resolves to its containing dir");
    expect_str_eq(M12_AssetStatus_GetRuntimeDataDir(&status, "theron"),
                  direct_dir,
                  "explicit Theron direct path does not inherit sibling fallback runtime dir");
    required = M12_AssetStatus_GetRequiredFile(&status, "theron", 0);
    expect_true(required && required->required && !required->matched,
                "explicit Theron direct path does not borrow sibling fallback required file");
    for (vi = 0; vi < M12_AssetStatus_GetVersionCount("theron"); ++vi) {
        const M12_AssetVersionStatus *version =
            M12_AssetStatus_GetVersion(&status, "theron", vi);
        expect_true(version && !version->matched,
                    "explicit Theron direct path leaves fallback versions unmatched");
    }

    fs_startup_check_games(explicit_track, &availability);
    expect_true(availability.theron_available == 0,
                "startup gating rejects explicit Theron direct path despite sibling fallback");
}

static void check_theron_media_layout_classification(void) {
    char temp_dir[512];
    char theron_dir[512];
    char cue_path[512];
    char iso_path[512];
    char ogg_path[512];
    char bin_cue_path[512];
    char bin_path[512];
    M12_AssetStatus status;
    const FirestaffTheronMediaStatus *media;
    const char *iso_ogg_cue =
        "REM Firestaff synthetic Theron ISO/OGG layout\n"
        "FILE \"Track01.ogg\" OGG\n"
        "  TRACK 01 AUDIO\n"
        "    INDEX 01 00:00:00\n"
        "FILE \"TQUS02.iso\" BINARY\n"
        "  TRACK 02 MODE1/2048\n"
        "    INDEX 01 00:00:00\n"
        "FILE \"Track03.ogg\" OGG\n"
        "  TRACK 03 AUDIO\n"
        "    INDEX 01 00:00:00\n";
    const char *bin_cue =
        "FILE \"Track01.ogg\" OGG\n"
        "  TRACK 01 AUDIO\n"
        "    INDEX 01 00:00:00\n"
        "FILE \"track02.bin\" BINARY\n"
        "  TRACK 02 MODE1/2352\n"
        "    INDEX 01 00:00:00\n";

    expect_true(make_temp_dir(temp_dir), "temporary Theron media-layout dir created");
    snprintf(theron_dir, sizeof(theron_dir), "%s%s%s", temp_dir, PATH_SEP, "theron");
    expect_true(make_dir_checked(theron_dir), "theron media-layout subdir created");
    snprintf(cue_path, sizeof(cue_path), "%s%s%s", theron_dir, PATH_SEP, "theron.cue");
    snprintf(iso_path, sizeof(iso_path), "%s%s%s", theron_dir, PATH_SEP, "TQUS02.iso");
    snprintf(ogg_path, sizeof(ogg_path), "%s%s%s", theron_dir, PATH_SEP, "Track01.ogg");
    expect_true(write_file(cue_path, iso_ogg_cue), "synthetic Theron ISO/OGG CUE written");
    expect_true(write_synthetic_iso_pvd(iso_path), "synthetic Theron ISO PVD written");
    expect_true(write_file(ogg_path, "OggS synthetic audio placeholder"),
                "synthetic Theron OGG placeholder written");

    M12_AssetStatus_Scan(&status, temp_dir);
    media = M12_AssetStatus_GetTheronMediaStatus(&status);
    expect_true(media != NULL, "Theron media status is available");
    expect_true(media && media->layout == FIRESTAFF_THERON_MEDIA_LAYOUT_ISO_OGG_CUE,
                "Theron ISO/OGG CUE layout is classified from data root");
    expect_true(media && media->has_track02_data == 1 && media->launch_candidate == 1,
                "Theron ISO/OGG layout records a Track 02 data candidate");
    expect_true(media && media->has_iso9660_pvd == 1,
                "Theron ISO/OGG layout records ISO9660 PVD evidence");
    expect_true(status.originalFileCandidateFound == 1,
                "Theron media layout counts as original-file evidence");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
                "Theron media layout without known Track 02 MD5 is not launch-ready");

    snprintf(bin_cue_path, sizeof(bin_cue_path), "%s%s%s", theron_dir, PATH_SEP, "theron-bin.cue");
    snprintf(bin_path, sizeof(bin_path), "%s%s%s", theron_dir, PATH_SEP, "track02.bin");
    expect_true(write_file(bin_cue_path, bin_cue), "synthetic Theron BIN/CUE written");
    expect_true(write_file(bin_path, "synthetic raw Track 02 BIN placeholder"),
                "synthetic Theron BIN placeholder written");

    M12_AssetStatus_Scan(&status, bin_cue_path);
    media = M12_AssetStatus_GetTheronMediaStatus(&status);
    expect_true(media && media->layout == FIRESTAFF_THERON_MEDIA_LAYOUT_BIN_CUE,
                "explicit Theron BIN/CUE layout is classified");
    expect_true(media && media->has_track02_data == 1,
                "explicit Theron BIN/CUE records Track 02 data");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
                "explicit Theron BIN/CUE without known MD5 is not launch-ready");
}

int main(void) {
    char temp_dir[512];
    char theron_dir[512];
    char fake_track[512];
    M12_AssetStatus status;
    FS_ValidationReport report;
    FS_GameAvailability availability;
    const char *real_data;

    expect_true(M12_AssetStatus_GameHasCompleteHashSet("theron") == 1,
                "Theron exposes a complete hash set");
    expect_true(M12_AssetStatus_GameKnownHashCount("theron") == 4U,
                "Theron exposes JP/US Track 02 BIN and ISO hashes");
    expect_true(M12_AssetStatus_GameRequiredFileCount("theron") == 1U,
                "Theron requires one Track 02 data file");
    expect_true(M12_AssetStatus_GameVerifiedFileCount("theron") == 1U,
                "Theron verifies one Track 02 data file");
    expect_true(M12_AssetStatus_FindVersionIndex("theron", "pce-jp") == 0,
                "Theron JP version id is indexed");
    expect_true(M12_AssetStatus_FindVersionIndex("theron", "pce-en") == 1,
                "Theron US version id is indexed");
    expect_true(M12_AssetStatus_FindVersionIndex("theron", "pce-jp-rev1-iso") == 2,
                "Theron JP Rev 1 ISO version id is indexed");
    expect_true(M12_AssetStatus_FindVersionIndex("theron", "pce-en-iso") == 3,
                "Theron US ISO version id is indexed");

    expect_true(make_temp_dir(temp_dir), "temporary data dir created");
    snprintf(theron_dir, sizeof(theron_dir), "%s%s%s", temp_dir, PATH_SEP, "theron");
    expect_true(mkdir(theron_dir, 0700) == 0, "theron subdir created");
    snprintf(fake_track, sizeof(fake_track), "%s%s%s", theron_dir, PATH_SEP, "track02.bin");
    expect_true(write_file(fake_track, "not a known Theron's Quest data track"),
                "fake Track 02 candidate written");

    M12_AssetStatus_Scan(&status, fake_track);
    expect_true(status.originalFileCandidateFound == 1,
                "explicit Theron Track 02 candidate counts as original-file evidence");
    expect_true(status.theronAvailable == 0,
                "explicit fake Track 02 is not marked available without a known MD5");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
                "game availability helper rejects explicit fake Theron data");

    expect_true(fs_validate_data_dir(fake_track, &report) >= 0,
                "validator runs on explicit fake Theron file");
    expect_true(report.theron_ready == 0,
                "validator rejects explicit fake Theron data");

    fs_startup_check_games(fake_track, &availability);
    expect_true(availability.theron_available == 0,
                "startup availability rejects explicit fake Theron data");
    expect_true(theron_v1_boot_probe_available(fake_track) == 0,
                "Theron quick boot probe rejects explicit unverified Track 02 data");

    check_explicit_track02_does_not_use_sibling_fallback();
    check_theron_media_layout_classification();

    check_fake_iso_fallback("jp",
                            "TQJP02End.iso",
                            THERON_PLATFORM_PCE_JP,
                            "pce-jp",
                            "Theron JP Rev 1 ISO filename fallback is detected");
    check_fake_iso_fallback("us",
                            "TQUS02End.iso",
                            THERON_PLATFORM_PCE_US,
                            "pce-en",
                            "Theron US ISO filename fallback is detected");

    real_data = getenv("FIRESTAFF_THERON_TEST_DATA");
    check_real_theron_launch_marker_presence(real_data);

    if (g_failures) {
        return 1;
    }
    printf("Theron V1 availability checks passed\n");
    return 0;
}
