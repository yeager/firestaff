/*
 * test_theron_v1_startup_save_resume_pc34.c
 *
 * Theron's Quest V1 — startup save/resume smoke gate unit test.
 *
 * The smoke gate is data-free on a clean host; this test exercises:
 *   - skip-safe no-SRM path: clean root → SKIP_SAFE_NO_SAVE_ROOT
 *   - skip-safe root-present-no-slots: empty save root
 *   - skip-safe staged-no-recognized-slot: only UNRECOGNIZED .srm
 *   - .tqsv-only resume claim: valid TQR + checksum footer slot
 *   - .srm-only resume claim: synthetic gzip-deflate body + zlib inflate
 *   - dual resume claim: both .tqsv and .srm present
 *   - boot profile handoff: theron_v1_boot_startup_save_resume()
 *     propagates boot->save_root when non-empty and falls back to the
 *     gate's own default resolver when empty
 *   - status name + source evidence string contracts
 *   - malformed inputs (NULL, empty root) handled cleanly
 *
 * Source/evidence: theron_v1_startup_save_resume_source_evidence() and
 * docs/DMWEB_REFERENCE.md §6 'Theron's Quest savegame format'.
 */

#include "theron_v1_startup_save_resume.h"
#include "theron_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef FIRESTAFF_HAS_ZLIB
#define FIRESTAFF_HAS_ZLIB 0
#endif

#if defined(_WIN32) || defined(_WIN64)
#define TEST_PATH_SEP '\\'
#include <direct.h>
#include <process.h>
#define test_mkdir(p) _mkdir(p)
#define test_rmdir(p) _rmdir(p)
#define test_unlink(p) remove(p)
#else
#define TEST_PATH_SEP '/'
#include <unistd.h>
#define test_mkdir(p) mkdir((p), 0700)
#define test_rmdir(p) rmdir(p)
#define test_unlink(p) unlink(p)
#endif

#define TST_NO_ROOT         "/tmp/firestaff_tsr_unit_no_such_root"
#define TST_EMPTY_ROOT      "/tmp/firestaff_tsr_unit_empty_root"
#define TST_BAD_ROOT        "/tmp/firestaff_tsr_unit_bad_root"
#define TST_TQSV_ROOT       "/tmp/firestaff_tsr_unit_tqsv_root"
#define TST_SRM_ROOT        "/tmp/firestaff_tsr_unit_srm_root"
#define TST_DUAL_TQSV_ROOT  "/tmp/firestaff_tsr_unit_dual_tqsv_root"
#define TST_DUAL_SRM_ROOT   "/tmp/firestaff_tsr_unit_dual_srm_root"

static int g_failures = 0;
static int g_tests_run = 0;
static int g_tests_passed = 0;

static void expect_true(int cond, const char *msg) {
    ++g_tests_run;
    if (!cond) {
        printf("FAIL: %s\n", msg);
        ++g_failures;
    } else {
        ++g_tests_passed;
    }
}

static int test_setenv(const char *name, const char *value) {
#if defined(_WIN32) || defined(_WIN64)
    return _putenv_s(name, value ? value : "") == 0;
#else
    if (value) return setenv(name, value, 1) == 0;
    return unsetenv(name) == 0;
#endif
}

static int make_temp_save_root(char out[THERON_V1_SRM_PATH_MAX]) {
#if defined(_WIN32) || defined(_WIN64)
    int pid = _getpid();
    for (int i = 0; i < 32; i++) {
        int n = snprintf(out, THERON_V1_SRM_PATH_MAX,
                         "firestaff_tsr_unit_%d_%d", pid, i);
        if (n <= 0 || n >= THERON_V1_SRM_PATH_MAX) return 0;
        if (test_mkdir(out) == 0) return 1;
    }
    out[0] = '\0';
    return 0;
#else
    static const char *template = "/tmp/firestaff_tsr_unit_XXXXXX";
    if (strlen(template) + 1 > THERON_V1_SRM_PATH_MAX) return 0;
    strncpy(out, template, THERON_V1_SRM_PATH_MAX - 1);
    out[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    return mkdtemp(out) != NULL;
#endif
}

static void cleanup_srm_root(const char *root) {
    if (!root || !root[0]) return;
    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; i++) {
        char path[THERON_V1_SRM_PATH_MAX];
        if (theron_v1_srm_slot_path(root, i, path)) {
            test_unlink(path);
        }
    }
    test_rmdir(root);
}

static void cleanup_tqsv_root(const char *root) {
    if (!root || !root[0]) return;
    for (int i = 0; i < THERON_SAVE_SLOT_COUNT; i++) {
        char path[THERON_V1_SRM_PATH_MAX];
        path[0] = '\0';
        theron_v1_save_slot_path(root, i, path, sizeof(path));
        if (path[0]) {
            test_unlink(path);
        }
    }
    test_rmdir(root);
}

static int write_bytes(const char *path, const uint8_t *buf, size_t size) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    if (size > 0 && buf) {
        size_t n = fwrite(buf, 1, size, fp);
        if (n != size) {
            fclose(fp);
            return 0;
        }
    }
    return fclose(fp) == 0;
}

/* Synthetic gzip-DEFLATE wrapper for the PRESENT_AND_RECOGNIZED
 * fixture.  Inflates to a valid FSTQPRG1 envelope so the bounded
 * payload probe + progression decode have something to consume. */
static const uint8_t g_valid_gzip_srm[] = {
    0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff,
    0x73, 0x0b, 0x0e, 0x09, 0x0c, 0x08, 0x72, 0x37, 0x64, 0x64,
    0x66, 0x66, 0xd4, 0x61, 0x64, 0x60, 0x60, 0x14, 0x60, 0x60,
    0x60, 0x02, 0x62, 0x66, 0x20, 0x66, 0x01, 0x62, 0x56, 0x20,
    0x66, 0x03, 0x62, 0x76, 0x20, 0x06, 0x00, 0x50, 0x8a, 0x0c,
    0xc3, 0x2c, 0x00, 0x00, 0x00
};

/* The matching inflate target.  The unit test does not validate the
 * inflated bytes directly (that contract is owned by
 * test_theron_v1_srm_classifier_pc34); here we only assert that the
 * payload probe succeeded, which is the bounded handoff guarantee
 * the startup gate cares about. */

static void test_clean_host_skip_safe_no_save_root(void) {
    /* Force both save roots to point at non-existent paths so the
     * gate reports the honest clean-host outcome.  With no HOME and
     * no env override, both resolvers return a non-empty path
     * (./theron-save for SRM, ./saves/theron for TQSV) but neither
     * directory exists, so the verdict is the conservative
     * SKIP_SAVE_ROOT_PRESENT_NO_SLOTS path — the gate never errors
     * on a clean host and never auto-resumes. */
    char saved_tqsv[THERON_V1_SRM_PATH_MAX] = {0};
    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev_tqsv = getenv("HOME");
    const char *prev_srm = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had_tqsv = prev_tqsv != NULL;
    int had_srm = prev_srm != NULL;
    if (had_tqsv) {
        strncpy(saved_tqsv, prev_tqsv, THERON_V1_SRM_PATH_MAX - 1);
        saved_tqsv[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    if (had_srm) {
        strncpy(saved_srm, prev_srm, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }

    test_setenv("HOME", "/tmp/firestaff_tsr_unit_no_home");
    test_setenv("FIRESTAFF_THERON_SRM_DIR", TST_NO_ROOT);

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    int rc = theron_v1_startup_save_resume_evaluate(NULL, &snap);
    expect_true(rc == 1, "evaluate on clean host returns 1");
    /* The clean-host outcome is "no recognized slot anywhere",
     * which the gate surfaces as one of the SKIP_* verdicts.  The
     * key invariant for CI: no auto-resume claim, both counts zero,
     * both roots resolved to non-empty strings (so the gate never
     * silently misroutes). */
    int skip_verdict =
        snap.verdict == THERON_V1_STARTUP_SKIP_SAFE_NO_SAVE_ROOT ||
        snap.verdict == THERON_V1_STARTUP_SKIP_SAVE_ROOT_PRESENT_NO_SLOTS ||
        snap.verdict == THERON_V1_STARTUP_SKIP_STAGED_NO_RECOGNIZED_SLOT;
    expect_true(skip_verdict,
                "clean host verdict is one of the SKIP_* values");
    expect_true(snap.resume_claim == THERON_V1_STARTUP_RESUME_NONE,
                "clean host resume claim NO_RESUME_CLAIM");
    expect_true(snap.tqsv_total_slots == THERON_SAVE_SLOT_COUNT,
                "clean host tqsv_total_slots == 8");
    expect_true(snap.srm_total_slots == THERON_V1_SRM_DISK_SLOT_COUNT,
                "clean host srm_total_slots == 5");
    expect_true(snap.tqsv_valid_slots == 0,
                "clean host tqsv_valid_slots == 0");
    expect_true(snap.srm_recognized_slots == 0,
                "clean host srm_recognized_slots == 0");
    expect_true(snap.tqsv_active_slot == -1,
                "clean host tqsv_active_slot == -1");
    expect_true(snap.srm_first_recognized_slot == -1,
                "clean host srm_first_recognized_slot == -1");
    expect_true(snap.tqsv_root[0] != '\0',
                "clean host tqsv_root resolved");
    expect_true(snap.srm_root[0] != '\0',
                "clean host srm_root resolved");

    if (had_tqsv) {
        test_setenv("HOME", saved_tqsv);
    } else {
        test_setenv("HOME", NULL);
    }
    if (had_srm) {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", saved_srm);
    } else {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void test_empty_root_is_skip_save_root_present_no_slots(void) {
    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had = prev != NULL;
    if (had) {
        strncpy(saved_srm, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    test_setenv("FIRESTAFF_THERON_SRM_DIR", TST_EMPTY_ROOT);
    test_mkdir(TST_EMPTY_ROOT);

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    int rc = theron_v1_startup_save_resume_evaluate(NULL, &snap);
    expect_true(rc == 1, "empty root evaluate returns 1");
    expect_true(snap.verdict ==
                    THERON_V1_STARTUP_SKIP_SAVE_ROOT_PRESENT_NO_SLOTS,
                "empty root verdict SKIP_SAVE_ROOT_PRESENT_NO_SLOTS");
    expect_true(snap.resume_claim == THERON_V1_STARTUP_RESUME_NONE,
                "empty root resume claim NONE");
    expect_true(snap.srm_present_slots == 0,
                "empty root srm_present_slots == 0");
    expect_true(snap.srm_recognized_slots == 0,
                "empty root srm_recognized_slots == 0");

    test_rmdir(TST_EMPTY_ROOT);
    if (had) {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", saved_srm);
    } else {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void test_staged_but_unrecognized_is_skip(void) {
    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had = prev != NULL;
    if (had) {
        strncpy(saved_srm, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    test_setenv("FIRESTAFF_THERON_SRM_DIR", TST_BAD_ROOT);
    test_mkdir(TST_BAD_ROOT);

    /* Stage a slot with a non-gzip body so the SRM classifier sees
     * an UNRECOGNIZED file but no valid .srm slot. */
    char slot_path[THERON_V1_SRM_PATH_MAX];
    int rc = theron_v1_srm_slot_path(TST_BAD_ROOT, 0, slot_path);
    expect_true(rc == 1, "slot 0 path constructs");
    static const uint8_t fake[] = "not a gzip stream";
    expect_true(write_bytes(slot_path, fake, sizeof(fake) - 1u) == 1,
                "slot 0 written");

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    rc = theron_v1_startup_save_resume_evaluate(NULL, &snap);
    expect_true(rc == 1, "unrecognized-staged evaluate returns 1");
    expect_true(snap.verdict ==
                    THERON_V1_STARTUP_SKIP_STAGED_NO_RECOGNIZED_SLOT,
                "unrecognized-staged verdict SKIP_STAGED_NO_RECOGNIZED_SLOT");
    expect_true(snap.srm_present_slots == 1,
                "unrecognized-staged srm_present_slots == 1");
    expect_true(snap.srm_recognized_slots == 0,
                "unrecognized-staged srm_recognized_slots == 0");

    cleanup_srm_root(TST_BAD_ROOT);
    if (had) {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", saved_srm);
    } else {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void test_tqsv_only_resume_claim(void) {
    /* Build a synthetic .tqsv via the existing save_to_slot API so
     * the unit test does not duplicate the obfuscated format. */
    char tqsv_root[THERON_V1_SRM_PATH_MAX];
    if (!make_temp_save_root(tqsv_root)) {
        printf("SKIP: mkdtemp failed for tqsv-only test\n");
        return;
    }

    uint8_t champion_data[THERON_SAVE_CHAMPION_BLOCK_SIZE * 4];
    memset(champion_data, 0, sizeof(champion_data));
    Theron_DungeonProgression prog;
    memset(&prog, 0, sizeof(prog));
    /* Init to a valid dungeon so the save_to_slot API does not
     * index dungeon_states[THERON_DUNGEON_INVALID - 1]. */
    prog.current_dungeon = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
    prog.dungeon_states[0] = THERON_DUNGEON_STATE_COMPLETE;
    int save_rc = theron_v1_save_to_slot(
        tqsv_root,
        2,
        champion_data,
        sizeof(champion_data),
        &prog,
        "after dungeon 2");
    expect_true(save_rc == 0, "save_to_slot returns 0");

    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had = prev != NULL;
    if (had) {
        strncpy(saved_srm, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    test_setenv("FIRESTAFF_THERON_SRM_DIR", TST_NO_ROOT);

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    int rc = theron_v1_startup_save_resume_evaluate(tqsv_root, &snap);
    expect_true(rc == 1, "tqsv-only evaluate returns 1");
    expect_true(snap.verdict == THERON_V1_STARTUP_SOURCES_LIVE,
                "tqsv-only verdict SOURCES_LIVE");
    expect_true(snap.resume_claim == THERON_V1_STARTUP_RESUME_TQSV,
                "tqsv-only resume claim TQSV");
    expect_true(snap.tqsv_valid_slots == 1,
                "tqsv-only tqsv_valid_slots == 1");
    expect_true(snap.tqsv_active_slot == 2,
                "tqsv-only tqsv_active_slot == 2");
    expect_true(snap.srm_recognized_slots == 0,
                "tqsv-only srm_recognized_slots == 0");

    cleanup_tqsv_root(tqsv_root);
    if (had) {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", saved_srm);
    } else {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void test_srm_only_resume_claim(void) {
    char srm_root[THERON_V1_SRM_PATH_MAX];
    if (!make_temp_save_root(srm_root)) {
        printf("SKIP: mkdtemp failed for srm-only test\n");
        return;
    }
    char slot_path[THERON_V1_SRM_PATH_MAX];
    int rc = theron_v1_srm_slot_path(srm_root, 1, slot_path);
    expect_true(rc == 1, "srm-only slot 1 path constructs");
    expect_true(write_bytes(slot_path, g_valid_gzip_srm,
                            sizeof(g_valid_gzip_srm)) == 1,
                "srm-only slot 1 written");

    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had = prev != NULL;
    if (had) {
        strncpy(saved_srm, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    test_setenv("FIRESTAFF_THERON_SRM_DIR", srm_root);

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    rc = theron_v1_startup_save_resume_evaluate(NULL, &snap);
    expect_true(rc == 1, "srm-only evaluate returns 1");
    expect_true(snap.verdict == THERON_V1_STARTUP_SOURCES_LIVE,
                "srm-only verdict SOURCES_LIVE");
    expect_true(snap.resume_claim == THERON_V1_STARTUP_RESUME_SRM,
                "srm-only resume claim SRM");
    expect_true(snap.srm_recognized_slots == 1,
                "srm-only srm_recognized_slots == 1");
    expect_true(snap.srm_first_recognized_slot == 1,
                "srm-only srm_first_recognized_slot == 1");
    expect_true(snap.srm_first_recognized_checksum32 != 0,
                "srm-only srm_first_recognized_checksum32 non-zero");

#if FIRESTAFF_HAS_ZLIB
    expect_true(snap.srm_payload_probe_ran == 1,
                "srm-only payload probe ran (zlib available)");
    expect_true(snap.srm_payload_probe_status ==
                    THERON_V1_SRM_PAYLOAD_PROBE_OK,
                "srm-only payload probe OK");
    expect_true(snap.srm_payload_hits_fstq_magic == 1,
                "srm-only payload hits FSTQPRG1 magic");
    expect_true(snap.srm_progress_import_ran == 1,
                "srm-only progression import ran");
    expect_true(snap.srm_progress_import_status ==
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                "srm-only progression import OK");
    expect_true(snap.srm_progress_current_dungeon == 3,
                "srm-only current dungeon 3");
    expect_true(snap.srm_progress_quest_mask == 0x03,
                "srm-only quest mask 0x03");
#else
    expect_true(snap.srm_payload_probe_ran == 0,
                "srm-only payload probe did not run without zlib");
#endif

    cleanup_srm_root(srm_root);
    if (had) {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", saved_srm);
    } else {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void test_dual_resume_claim(void) {
    char tqsv_root[THERON_V1_SRM_PATH_MAX];
    char srm_root[THERON_V1_SRM_PATH_MAX];
    if (!make_temp_save_root(tqsv_root) ||
        !make_temp_save_root(srm_root)) {
        printf("SKIP: mkdtemp failed for dual test\n");
        return;
    }

    /* Stage a synthetic .tqsv via the existing API. */
    uint8_t champion_data[THERON_SAVE_CHAMPION_BLOCK_SIZE * 4];
    memset(champion_data, 0, sizeof(champion_data));
    Theron_DungeonProgression prog;
    memset(&prog, 0, sizeof(prog));
    prog.current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    prog.dungeon_states[0] = THERON_DUNGEON_STATE_AVAILABLE;
    int save_rc = theron_v1_save_to_slot(
        tqsv_root, 0, champion_data, sizeof(champion_data),
        &prog, "after dungeon 1");
    expect_true(save_rc == 0, "dual save_to_slot returns 0");

    /* Stage a recognized .srm slot. */
    char slot_path[THERON_V1_SRM_PATH_MAX];
    expect_true(theron_v1_srm_slot_path(srm_root, 3, slot_path) == 1,
                "dual srm slot 3 path");
    expect_true(write_bytes(slot_path, g_valid_gzip_srm,
                            sizeof(g_valid_gzip_srm)) == 1,
                "dual srm slot 3 written");

    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had = prev != NULL;
    if (had) {
        strncpy(saved_srm, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    test_setenv("FIRESTAFF_THERON_SRM_DIR", srm_root);

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    int rc = theron_v1_startup_save_resume_evaluate(tqsv_root, &snap);
    expect_true(rc == 1, "dual evaluate returns 1");
    expect_true(snap.verdict == THERON_V1_STARTUP_SOURCES_LIVE,
                "dual verdict SOURCES_LIVE");
    expect_true(snap.resume_claim == THERON_V1_STARTUP_RESUME_DUAL,
                "dual resume claim DUAL");
    expect_true(snap.tqsv_valid_slots == 1,
                "dual tqsv_valid_slots == 1");
    expect_true(snap.srm_recognized_slots == 1,
                "dual srm_recognized_slots == 1");
    expect_true(snap.srm_first_recognized_slot == 3,
                "dual srm_first_recognized_slot == 3");

    cleanup_tqsv_root(tqsv_root);
    cleanup_srm_root(srm_root);
    if (had) {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", saved_srm);
    } else {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void test_boot_profile_handoff(void) {
    /* Empty boot profile → gate falls back to its own default
     * resolver.  This is the production path: the launcher calls
     * theron_v1_boot_startup_save_resume() with whatever boot
     * profile the M12 layer built. */
    Theron_V1_BootProfile profile;
    memset(&profile, 0, sizeof(profile));
    theron_v1_boot_profile_init(&profile);

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    int rc = theron_v1_boot_startup_save_resume(&profile, &snap);
    expect_true(rc == 1, "empty-profile boot handoff returns 1");
    expect_true(snap.tqsv_root[0] != '\0',
                "empty-profile boot handoff tqsv_root non-empty");
    expect_true(snap.srm_root[0] != '\0',
                "empty-profile boot handoff srm_root non-empty");

    /* Non-empty profile->save_root is propagated as the boot root. */
    Theron_V1_BootProfile profile2;
    memset(&profile2, 0, sizeof(profile2));
    theron_v1_boot_profile_init(&profile2);
    strncpy(profile2.save_root, TST_TQSV_ROOT,
            sizeof(profile2.save_root) - 1);
    Theron_V1StartupSaveResume snap2;
    memset(&snap2, 0, sizeof(snap2));
    rc = theron_v1_boot_startup_save_resume(&profile2, &snap2);
    expect_true(rc == 1, "set-root boot handoff returns 1");
    expect_true(strcmp(snap2.tqsv_root, TST_TQSV_ROOT) == 0,
                "boot save_root propagates to tqsv_root");

    /* NULL safety. */
    Theron_V1StartupSaveResume snap3;
    memset(&snap3, 0, sizeof(snap3));
    rc = theron_v1_boot_startup_save_resume(NULL, &snap3);
    expect_true(rc == 0, "NULL profile handoff returns 0");

    rc = theron_v1_boot_startup_save_resume(&profile, NULL);
    expect_true(rc == 0, "NULL snapshot handoff returns 0");
}

static void test_status_name_contracts(void) {
    expect_true(strcmp(
        theron_v1_startup_save_resume_skip_safe_name(
            THERON_V1_STARTUP_SOURCES_LIVE),
        "SOURCES_LIVE") == 0,
        "SOURCES_LIVE name");
    expect_true(strcmp(
        theron_v1_startup_save_resume_skip_safe_name(
            THERON_V1_STARTUP_SKIP_SAFE_NO_SAVE_ROOT),
        "SKIP_SAFE_NO_SAVE_ROOT") == 0,
        "SKIP_SAFE_NO_SAVE_ROOT name");
    expect_true(strcmp(
        theron_v1_startup_save_resume_skip_safe_name(
            THERON_V1_STARTUP_SKIP_SAVE_ROOT_PRESENT_NO_SLOTS),
        "SKIP_SAVE_ROOT_PRESENT_NO_SLOTS") == 0,
        "SKIP_SAVE_ROOT_PRESENT_NO_SLOTS name");
    expect_true(strcmp(
        theron_v1_startup_save_resume_skip_safe_name(
            THERON_V1_STARTUP_SKIP_STAGED_NO_RECOGNIZED_SLOT),
        "SKIP_STAGED_NO_RECOGNIZED_SLOT") == 0,
        "SKIP_STAGED_NO_RECOGNIZED_SLOT name");
    expect_true(strcmp(
        theron_v1_startup_save_resume_skip_safe_name(99), "UNKNOWN") == 0,
        "out-of-enum verdict name is UNKNOWN");

    expect_true(strcmp(
        theron_v1_startup_save_resume_claim_name(
            THERON_V1_STARTUP_RESUME_NONE),
        "NO_RESUME_CLAIM") == 0, "NO_RESUME_CLAIM name");
    expect_true(strcmp(
        theron_v1_startup_save_resume_claim_name(
            THERON_V1_STARTUP_RESUME_TQSV),
        "TQSV_RESUME_CLAIM") == 0, "TQSV_RESUME_CLAIM name");
    expect_true(strcmp(
        theron_v1_startup_save_resume_claim_name(
            THERON_V1_STARTUP_RESUME_SRM),
        "SRM_RESUME_CLAIM") == 0, "SRM_RESUME_CLAIM name");
    expect_true(strcmp(
        theron_v1_startup_save_resume_claim_name(
            THERON_V1_STARTUP_RESUME_DUAL),
        "DUAL_RESUME_CLAIM") == 0, "DUAL_RESUME_CLAIM name");
    expect_true(strcmp(
        theron_v1_startup_save_resume_claim_name(99), "UNKNOWN") == 0,
        "out-of-enum claim name is UNKNOWN");
}

static void test_source_evidence_string(void) {
    const char *ev = theron_v1_startup_save_resume_source_evidence();
    expect_true(ev != NULL && strlen(ev) > 50,
                "source evidence non-empty");
    expect_true(strstr(ev, "T080") != NULL,
                "source cites THQUEST.ASM T080");
    expect_true(strstr(ev, "T800") != NULL,
                "source cites THQUEST.ASM T800");
    expect_true(strstr(ev, "DMWEB_REFERENCE") != NULL,
                "source cites DMWEB_REFERENCE");
    expect_true(strstr(ev, "Sphenx") != NULL,
                "source cites Sphenx");
    expect_true(strstr(ev, "skip-safe") != NULL,
                "source mentions skip-safe verdict");
    expect_true(strstr(ev, "Does NOT auto-resume") != NULL,
                "source explicitly says no auto-resume");
}

static void test_format_helper(void) {
    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    char buf[1024];
    size_t n = theron_v1_startup_save_resume_format(&snap, buf, sizeof(buf));
    expect_true(n > 0, "format helper returns positive length");
    expect_true(strstr(buf, "=== Theron V1 Startup Save/Resume") != NULL,
                "format header line present");
    expect_true(strstr(buf, "verdict:") != NULL,
                "format includes verdict line");
    expect_true(strstr(buf, "resume_claim:") != NULL,
                "format includes resume_claim line");
    /* Truncation contract: never write past buf_size-1. */
    char tiny[16];
    n = theron_v1_startup_save_resume_format(&snap, tiny, sizeof(tiny));
    expect_true(n <= sizeof(tiny) - 1u,
                "format helper respects buf_size cap");
}

static void test_null_safety(void) {
    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    int rc = theron_v1_startup_save_resume_evaluate(NULL, NULL);
    expect_true(rc == 0, "NULL snapshot returns 0");

    rc = theron_v1_startup_save_resume_evaluate("", &snap);
    /* Empty boot save root is allowed (falls back to default
     * resolver); the contract must not error. */
    expect_true(rc == 1, "empty boot root falls back to default");

    expect_true(theron_v1_startup_save_resume_format(NULL, NULL, 0) == 0,
                "NULL format inputs return 0");
    char buf[8] = {'X'};
    expect_true(theron_v1_startup_save_resume_format(NULL, buf, sizeof(buf)) == 0,
                "NULL snapshot in format returns 0");
    expect_true(buf[0] == '\0',
                "NULL snapshot in format zeroes first byte");
}

static void test_snapshot_snapshot_is_deterministic(void) {
    /* Two consecutive evaluations against the same env state must
     * produce identical verdicts + counts.  This is the
     * "skip-safe CI" property: a clean host run twice must give the
     * same receipt. */
    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had = prev != NULL;
    if (had) {
        strncpy(saved_srm, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    test_setenv("FIRESTAFF_THERON_SRM_DIR", TST_NO_ROOT);

    Theron_V1StartupSaveResume snap1, snap2;
    memset(&snap1, 0, sizeof(snap1));
    memset(&snap2, 0, sizeof(snap2));
    theron_v1_startup_save_resume_evaluate(NULL, &snap1);
    theron_v1_startup_save_resume_evaluate(NULL, &snap2);
    expect_true(snap1.verdict == snap2.verdict,
                "verdict deterministic across runs");
    expect_true(snap1.resume_claim == snap2.resume_claim,
                "resume_claim deterministic across runs");
    expect_true(snap1.tqsv_total_slots == snap2.tqsv_total_slots,
                "tqsv_total_slots deterministic");
    expect_true(snap1.srm_total_slots == snap2.srm_total_slots,
                "srm_total_slots deterministic");
    expect_true(strcmp(snap1.verdict_name, snap2.verdict_name) == 0,
                "verdict_name stable across runs");
    expect_true(strcmp(snap1.resume_claim_name, snap2.resume_claim_name) == 0,
                "resume_claim_name stable across runs");

    if (had) {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", saved_srm);
    } else {
        test_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

int main(void) {
    printf("\n=== Theron V1 Startup Save/Resume Smoke Gate Unit Tests ===\n\n");
    test_clean_host_skip_safe_no_save_root();
    test_empty_root_is_skip_save_root_present_no_slots();
    test_staged_but_unrecognized_is_skip();
    test_tqsv_only_resume_claim();
    test_srm_only_resume_claim();
    test_dual_resume_claim();
    test_boot_profile_handoff();
    test_status_name_contracts();
    test_source_evidence_string();
    test_format_helper();
    test_null_safety();
    test_snapshot_snapshot_is_deterministic();

    printf("=====================================================\n");
    printf("Results: %d/%d passed (failures=%d)\n",
           g_tests_passed, g_tests_run, g_failures);
    printf("=====================================================\n\n");
    return g_failures ? 1 : 0;
}
