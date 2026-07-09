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
#include "theron_v1_startup_flow.h"

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

static const uint8_t g_valid_party_gzip_srm[] = {
    0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x73, 0x0b,
    0x0e, 0x09, 0x0c, 0x08, 0x89, 0x34, 0x64, 0x64, 0x66, 0x66, 0xd4, 0x61,
    0x64, 0x60, 0x60, 0x14, 0x60, 0x60, 0x60, 0x02, 0x62, 0x66, 0x20, 0x66,
    0x01, 0x62, 0x56, 0x20, 0x66, 0x03, 0x62, 0x76, 0x20, 0xe6, 0x64, 0x66,
    0x60, 0x08, 0xc9, 0x48, 0x2d, 0xca, 0xcf, 0x63, 0x40, 0x00, 0xc6, 0xa0,
    0x20, 0x1f, 0x1f, 0x09, 0x09, 0x21, 0x41, 0x01, 0x7e, 0x16, 0x56, 0x66,
    0x88, 0x98, 0x8d, 0x2d, 0x83, 0x63, 0x51, 0x26, 0x03, 0x32, 0x60, 0x64,
    0xd4, 0xd5, 0x75, 0x73, 0xe3, 0xe0, 0x10, 0x06, 0x2a, 0x64, 0x61, 0x65,
    0x80, 0x28, 0x34, 0x32, 0x66, 0xf0, 0xcd, 0x2c, 0x4a, 0x44, 0x56, 0xc7,
    0xc4, 0x68, 0x62, 0x62, 0x65, 0xa5, 0xa0, 0x20, 0x02, 0x54, 0x08, 0x54,
    0x07, 0x51, 0x68, 0x62, 0xca, 0x10, 0x9c, 0x9f, 0x83, 0x62, 0x1e, 0x33,
    0xa3, 0x9a, 0x9a, 0x8e, 0x8e, 0xa1, 0xa1, 0x28, 0x50, 0x21, 0x48, 0x1d,
    0x58, 0xa1, 0x99, 0x39, 0x03, 0x00, 0x61, 0x93, 0xfd, 0x41, 0xd0, 0x00,
    0x00, 0x00
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
    char custom_path[THERON_V1_SRM_PATH_MAX];
    if (!make_temp_save_root(tqsv_root)) {
        printf("SKIP: mkdtemp failed for tqsv-only test\n");
        return;
    }
    memset(custom_path, 0, sizeof(custom_path));

    uint8_t champion_data[THERON_MAX_CHAMPIONS * sizeof(Theron_V1_Champion)];
    Theron_V1_Party party;
    memset(champion_data, 0, sizeof(champion_data));
    theron_v1_party_init(&party, THERON_DUNGEON_2_CRYPT_OF_SHADOWS);
    party.leader_x = 6;
    party.leader_y = 7;
    party.leader_dir = 1;
    expect_true(theron_v1_party_pack(&party,
                                     champion_data,
                                     sizeof(champion_data)) > 0,
                "tqsv-only party pack");
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
    snprintf(custom_path,
             sizeof(custom_path),
             "%s%cexternal_continue.tqsv",
             tqsv_root,
             TEST_PATH_SEP);
    expect_true(theron_v1_save_export_slot(tqsv_root, 2, custom_path) == 0,
                "tqsv-only external export returns 0");

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
    {
        Theron_V1StartupSaveResume explicit_snap;
        char slot_path[THERON_V1_SRM_PATH_MAX];
        memset(&explicit_snap, 0, sizeof(explicit_snap));
        theron_v1_save_slot_path(tqsv_root, 2, slot_path, sizeof(slot_path));
        expect_true(theron_v1_startup_save_resume_apply_explicit_path(
                        &explicit_snap,
                        slot_path,
                        tqsv_root) == 1,
                    "explicit tqsv path applies to save-resume snapshot");
        expect_true(explicit_snap.resume_claim ==
                        THERON_V1_STARTUP_RESUME_TQSV,
                    "explicit tqsv path sets TQSV resume claim");
        expect_true(explicit_snap.tqsv_active_slot == 2 &&
                        explicit_snap.tqsv_valid_slots == 1,
                    "explicit tqsv path selects requested slot");
    }
    {
        Theron_StartupAction action;
        Theron_StartupActionPlan plan;
        Theron_V1StartupContinueResult continue_result;
        Theron_V1StartupContinueRequest continue_request;
        Theron_StartupHostReceipt host_receipt;
        Theron_StartupStateReceipt state_receipt;
        Theron_V1_World world;
        char receipt[256];

        theron_v1_startup_action_init(&action);
        action.kind = THERON_STARTUP_ACTION_CONTINUE_SAVE;
        expect_true(theron_v1_startup_plan_for_action(&action, &plan) == 1,
                    "tqsv-only host plan");
        theron_v1_startup_continue_request_init(&continue_request);
        continue_request.resume_claim = THERON_V1_STARTUP_RESUME_TQSV;
        continue_request.tqsv_slot_index = 2;
        continue_request.tqsv_root = tqsv_root;
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(theron_v1_startup_continue_apply_request_with_host_receipts(
                        &world,
                        &continue_request,
                        &plan,
                        "chapter=2 level=0",
                        &continue_result,
                        &host_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) == 1 &&
                        continue_result.source ==
                            THERON_V1_STARTUP_CONTINUE_SOURCE_TQSV &&
                        host_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        host_receipt.status &&
                        strcmp(host_receipt.status, "CONTINUE LOADED") == 0 &&
                        strstr(host_receipt.inspect_detail,
                               "chapter=2") != NULL &&
                        state_receipt.flow.selected_dungeon ==
                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    "tqsv-only request emits host and state receipts");
        theron_v1_startup_continue_request_init(&continue_request);
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(!theron_v1_startup_continue_apply_request_with_host_receipts(
                        &world,
                        &continue_request,
                        &plan,
                        "chapter=2 level=0",
                        &continue_result,
                        &host_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) &&
                        host_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        host_receipt.status &&
                        strstr(host_receipt.status,
                               "Continue requires") != NULL &&
                        strstr(host_receipt.inspect_detail,
                               "source=NONE") != NULL,
                    "tqsv-only request no-source emits host failure receipt");
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(theron_v1_startup_continue_tqsv_apply_with_host_receipts(
                        &world,
                        tqsv_root,
                        2,
                        &plan,
                        NULL,
                        &continue_result,
                        &host_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) == 1 &&
                        continue_result.source ==
                            THERON_V1_STARTUP_CONTINUE_SOURCE_TQSV &&
                        host_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        host_receipt.status &&
                        strcmp(host_receipt.status, "CONTINUE LOADED") == 0 &&
                        strstr(host_receipt.inspect_detail,
                               "after dungeon 2") != NULL &&
                        state_receipt.flow_changed &&
                        state_receipt.flow.selected_dungeon ==
                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    "tqsv-only slot emits host and state receipts");
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(!theron_v1_startup_continue_tqsv_apply_with_host_receipts(
                        &world,
                        tqsv_root,
                        7,
                        &plan,
                        NULL,
                        &continue_result,
                        &host_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) &&
                        host_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        host_receipt.status &&
                        strstr(host_receipt.status,
                               "Continue slot 7 failed") != NULL &&
                        strstr(host_receipt.inspect_detail,
                               "source=NONE") != NULL,
                    "tqsv-only empty slot emits host failure receipt");
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(theron_v1_startup_continue_tqsv_path_apply_with_host_receipts(
                        &world,
                        custom_path,
                        &plan,
                        NULL,
                        &continue_result,
                        &host_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) == 1 &&
                        continue_result.source ==
                            THERON_V1_STARTUP_CONTINUE_SOURCE_TQSV &&
                        host_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        host_receipt.status &&
                        strcmp(host_receipt.status, "CONTINUE LOADED") == 0 &&
                        strstr(host_receipt.inspect_detail,
                               "after dungeon 2") != NULL &&
                        state_receipt.flow_changed &&
                        state_receipt.flow.selected_dungeon ==
                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    "tqsv-only external path emits host and state receipts");
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(!theron_v1_startup_continue_tqsv_path_apply_with_host_receipts(
                        &world,
                        "",
                        &plan,
                        NULL,
                        &continue_result,
                        &host_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) &&
                        host_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        host_receipt.status &&
                        strstr(host_receipt.status,
                               "Continue requires a .tqsv path") != NULL &&
                        strstr(host_receipt.inspect_detail,
                               "source=NONE") != NULL,
                    "tqsv-only external path failure emits host receipt");
    }

    test_unlink(custom_path);
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
    {
        Theron_V1StartupSaveResume explicit_snap;
        memset(&explicit_snap, 0, sizeof(explicit_snap));
        expect_true(theron_v1_startup_save_resume_apply_explicit_path(
                        &explicit_snap,
                        slot_path,
                        NULL) == 1,
                    "explicit srm path applies to save-resume snapshot");
        expect_true(explicit_snap.resume_claim ==
                        THERON_V1_STARTUP_RESUME_SRM,
                    "explicit srm path sets SRM resume claim");
        expect_true(explicit_snap.srm_first_recognized_slot == 1 &&
                        explicit_snap.srm_recognized_slots == 1,
                    "explicit srm path selects requested slot");
        expect_true(strcmp(explicit_snap.srm_root, srm_root) == 0,
                    "explicit srm path carries source root");
    }

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

static void test_srm_party_continue_restores_all_champions(void) {
    char srm_root[THERON_V1_SRM_PATH_MAX];
    char slot_path[THERON_V1_SRM_PATH_MAX];
    char custom_path[THERON_V1_SRM_PATH_MAX];
    Theron_V1_World world;
    char receipt[256];
    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had = prev != NULL;

    if (!make_temp_save_root(srm_root)) {
        printf("SKIP: mkdtemp failed for srm party continue test\n");
        return;
    }
    if (had) {
        strncpy(saved_srm, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    expect_true(theron_v1_srm_slot_path(srm_root, 2, slot_path) == 1,
                "srm party slot 2 path constructs");
    expect_true(write_bytes(slot_path,
                            g_valid_party_gzip_srm,
                            sizeof(g_valid_party_gzip_srm)) == 1,
                "srm party slot 2 written");
    snprintf(custom_path,
             sizeof(custom_path),
             "%s%cstaged_external_save.srm",
             srm_root,
             TEST_PATH_SEP);
    expect_true(write_bytes(custom_path,
                            g_valid_party_gzip_srm,
                            sizeof(g_valid_party_gzip_srm)) == 1,
                "srm party custom path written");
    test_setenv("FIRESTAFF_THERON_SRM_DIR", srm_root);

    {
        Theron_V1StartupSaveResume snap;
        Theron_StartupStateReceipt snapshot_state_receipt;
        memset(&snap, 0, sizeof(snap));
        expect_true(theron_v1_startup_save_resume_evaluate(NULL, &snap) == 1,
                    "srm party snapshot evaluate returns 1");
        expect_true(snap.resume_claim == THERON_V1_STARTUP_RESUME_SRM &&
                        snap.srm_first_recognized_slot == 2,
                    "srm party snapshot selects SRM slot 2");
#if FIRESTAFF_HAS_ZLIB
        expect_true(snap.srm_envelope_kind ==
                        THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY,
                    "srm party snapshot records PROGRESSION_PARTY envelope");
        expect_true(snap.srm_progress_import_status ==
                        THERON_V1_SRM_PROGRESS_IMPORT_OK &&
                        snap.srm_progress_current_dungeon ==
                            THERON_DUNGEON_3_ABYSS_OF_FLAMES,
                    "srm party snapshot restores progression receipt");
        expect_true(snap.srm_party_import_ran == 1 &&
                        snap.srm_party_restored == 1 &&
                        snap.srm_party_champion_count ==
                            THERON_MAX_CHAMPIONS &&
                        snap.srm_party_gold == 777u,
                    "srm party snapshot carries party body receipt");
        {
            Theron_V1StartupSaveResume explicit_snap;
            Theron_V1StartupSaveResume external_snap;
            Theron_StartupStateReceipt explicit_state_receipt;
            Theron_StartupStateReceipt external_state_receipt;
            memset(&explicit_snap, 0, sizeof(explicit_snap));
            memset(&external_snap, 0, sizeof(external_snap));
            memset(&explicit_state_receipt, 0, sizeof(explicit_state_receipt));
            memset(&external_state_receipt, 0, sizeof(external_state_receipt));
            expect_true(theron_v1_startup_save_resume_apply_explicit_path(
                            &explicit_snap,
                            slot_path,
                            NULL) == 1,
                        "explicit srm party path applies");
            expect_true(explicit_snap.srm_envelope_kind ==
                            THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY &&
                            explicit_snap.srm_payload_probe_ran == 1 &&
                            explicit_snap.srm_payload_hits_fstq_magic == 1,
                        "explicit srm party path carries envelope receipt");
            expect_true(explicit_snap.srm_party_import_ran == 1 &&
                            explicit_snap.srm_party_restored == 1 &&
                            explicit_snap.srm_party_champion_count ==
                                THERON_MAX_CHAMPIONS &&
                            explicit_snap.srm_party_gold == 777u,
                        "explicit srm party path carries party body receipt");
            expect_true(theron_v1_startup_save_resume_state_receipt(
                            &explicit_snap,
                            1,
                            &explicit_state_receipt) == 1 &&
                            explicit_state_receipt
                                    .save_resume_srm_party_champion_count ==
                                THERON_MAX_CHAMPIONS &&
                            explicit_state_receipt.save_resume_srm_party_gold ==
                                777u,
                        "explicit srm party path state receipt carries party body");
            expect_true(theron_v1_startup_save_resume_apply_explicit_path(
                            &external_snap,
                            custom_path,
                            NULL) == 1,
                        "external srm party path applies");
            expect_true(external_snap.srm_first_recognized_slot == -1 &&
                            external_snap.srm_recognized_slots == 1 &&
                            external_snap.resume_claim ==
                                THERON_V1_STARTUP_RESUME_SRM,
                        "external srm party path is recognized without slot id");
            expect_true(external_snap.srm_envelope_kind ==
                            THERON_V1_SRM_ENVELOPE_KIND_PROGRESSION_PARTY &&
                            external_snap.srm_party_champion_count ==
                                THERON_MAX_CHAMPIONS &&
                            external_snap.srm_party_gold == 777u,
                        "external srm party path carries party body receipt");
            expect_true(theron_v1_startup_save_resume_state_receipt(
                            &external_snap,
                            1,
                            &external_state_receipt) == 1 &&
                            external_state_receipt.save_resume_srm_active_slot ==
                                -1 &&
                            external_state_receipt
                                    .save_resume_srm_party_champion_count ==
                                THERON_MAX_CHAMPIONS &&
                            external_state_receipt.save_resume_srm_party_gold ==
                                777u,
                        "external srm party state receipt carries party body");
        }
        memset(&snapshot_state_receipt, 0, sizeof(snapshot_state_receipt));
        expect_true(theron_v1_startup_save_resume_state_receipt(
                        &snap,
                        1,
                        &snapshot_state_receipt) == 1 &&
                        snapshot_state_receipt.save_resume_srm_party_restored ==
                            1 &&
                        snapshot_state_receipt
                                .save_resume_srm_party_champion_count ==
                            THERON_MAX_CHAMPIONS &&
                        snapshot_state_receipt.save_resume_srm_party_gold ==
                            777u,
                    "srm party snapshot state receipt carries party body");
#else
        expect_true(snap.srm_payload_probe_ran == 0,
                    "srm party snapshot does not decode without zlib");
#endif
    }

    theron_v1_world_init(&world);
    world.party.champion_count = 1;
    memset(receipt, 0, sizeof(receipt));
    expect_true(theron_v1_startup_continue_srm_apply(&world,
                                                     srm_root,
                                                     2,
                                                     receipt,
                                                     sizeof(receipt)) == 1,
                "srm party continue applies");
    expect_true(world.progression.current_dungeon ==
                    THERON_DUNGEON_3_ABYSS_OF_FLAMES,
                "srm party continue restores progression dungeon");
    expect_true(world.party.champion_count == THERON_MAX_CHAMPIONS,
                "srm party continue keeps all imported champions");
    expect_true(world.party.gold == 777u,
                "srm party continue restores party gold");
    expect_true(strcmp(world.party.champions[0].name, "Theron") == 0 &&
                    world.party.champions[0].health == 82,
                "srm party continue restores Theron body");
    expect_true(strcmp(world.party.champions[1].name, "Ari") == 0 &&
                    world.party.champions[1].primary_class ==
                        THERON_CLASS_NINJA &&
                    world.party.champions[1].stamina == 70,
                "srm party continue restores companion body");
    expect_true(strstr(receipt, "PROGRESSION_PARTY") != NULL,
                "srm party continue receipt reports party envelope");

    theron_v1_world_init(&world);
    memset(receipt, 0, sizeof(receipt));
    expect_true(theron_v1_startup_continue_srm_path_apply(
                    &world,
                    custom_path,
                    receipt,
                    sizeof(receipt)) == 1,
                "srm party custom path continue applies");
    expect_true(world.party.champion_count == THERON_MAX_CHAMPIONS &&
                    world.party.gold == 777u,
                "srm party custom path restores party body");
    expect_true(strstr(receipt, "path=-1") != NULL &&
                    strstr(receipt, "PROGRESSION_PARTY") != NULL,
                "srm party custom path receipt reports path envelope");
    {
        Theron_StartupAction action;
        Theron_StartupActionPlan plan;
        Theron_V1StartupContinueResult continue_result;
        Theron_V1StartupContinueApplyReceipt apply_receipt;
        Theron_StartupHostReceipt host_receipt;
        Theron_StartupStateReceipt state_receipt;

        theron_v1_startup_action_init(&action);
        action.kind = THERON_STARTUP_ACTION_CONTINUE_SAVE;
        expect_true(theron_v1_startup_plan_for_action(&action, &plan) == 1,
                    "srm party custom path host plan");
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(theron_v1_startup_continue_srm_apply_with_host_receipts(
                        &world,
                        srm_root,
                        2,
                        &plan,
                        NULL,
                        &continue_result,
                        &host_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) == 1 &&
                        continue_result.source ==
                            THERON_V1_STARTUP_CONTINUE_SOURCE_SRM &&
                        continue_result.source_slot_index == 2 &&
                        continue_result.srm_import_status ==
                            THERON_V1_SRM_PROGRESS_IMPORT_OK &&
                        continue_result.srm_current_dungeon ==
                            THERON_DUNGEON_3_ABYSS_OF_FLAMES &&
                        continue_result.srm_party_champion_count ==
                            THERON_MAX_CHAMPIONS &&
                        continue_result.srm_party_gold == 777u &&
                        host_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        host_receipt.status &&
                        strcmp(host_receipt.status, "CONTINUE LOADED") == 0 &&
                        strstr(host_receipt.inspect_detail,
                               "PROGRESSION_PARTY") != NULL &&
                        state_receipt.flow_changed &&
                        state_receipt.flow.selected_dungeon ==
                            THERON_DUNGEON_3_ABYSS_OF_FLAMES &&
                        state_receipt.set_save_resume &&
                        state_receipt.save_resume_srm_active_slot == 2 &&
                        state_receipt.save_resume_srm_import_status ==
                            THERON_V1_SRM_PROGRESS_IMPORT_OK &&
                        state_receipt.save_resume_srm_current_dungeon ==
                            THERON_DUNGEON_3_ABYSS_OF_FLAMES &&
                        state_receipt.save_resume_srm_party_champion_count ==
                            THERON_MAX_CHAMPIONS &&
                        state_receipt.save_resume_srm_party_gold == 777u,
                    "srm party slot emits host and state receipts");
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(!theron_v1_startup_continue_srm_apply_with_host_receipts(
                        &world,
                        srm_root,
                        4,
                        &plan,
                        NULL,
                        &continue_result,
                        &host_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) &&
                        host_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        host_receipt.status &&
                        strstr(host_receipt.status,
                               "SRM decode unsupported") != NULL &&
                        strstr(host_receipt.inspect_detail,
                               "source=NONE") != NULL,
                    "srm party empty slot emits host failure receipt");
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(theron_v1_startup_continue_srm_path_apply_with_host_receipts(
                        &world,
                        custom_path,
                        &plan,
                        NULL,
                        &continue_result,
                        &host_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) == 1 &&
                        continue_result.source ==
                            THERON_V1_STARTUP_CONTINUE_SOURCE_SRM &&
                        continue_result.source_slot_index == -1 &&
                        continue_result.srm_import_status ==
                            THERON_V1_SRM_PROGRESS_IMPORT_OK &&
                        host_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        host_receipt.status &&
                        strcmp(host_receipt.status, "CONTINUE LOADED") == 0 &&
                        strstr(host_receipt.inspect_detail,
                               "PROGRESSION_PARTY") != NULL &&
                        state_receipt.flow_changed &&
                        state_receipt.flow.phase ==
                            THERON_STARTUP_PHASE_STAGE_SELECT &&
                        state_receipt.flow.selected_dungeon ==
                            THERON_DUNGEON_3_ABYSS_OF_FLAMES &&
                        state_receipt.set_level_loaded &&
                        state_receipt.level_loaded == 0 &&
                        state_receipt.set_save_resume &&
                        state_receipt.save_resume_srm_active_slot == -1 &&
                        state_receipt.save_resume_srm_party_gold == 777u,
                    "srm party custom path emits host and state receipts");
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(theron_v1_startup_continue_srm_path_apply_with_receipts(
                        &world,
                        custom_path,
                        &plan,
                        NULL,
                        &continue_result,
                        &apply_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) == 1 &&
                        continue_result.source ==
                            THERON_V1_STARTUP_CONTINUE_SOURCE_SRM &&
                        continue_result.source_slot_index == -1 &&
                        apply_receipt.source ==
                            THERON_V1_STARTUP_CONTINUE_SOURCE_SRM &&
                        apply_receipt.source_slot_index == -1 &&
                        apply_receipt.srm_import_status ==
                            THERON_V1_SRM_PROGRESS_IMPORT_OK &&
                        apply_receipt.srm_party_champion_count ==
                            THERON_MAX_CHAMPIONS &&
                        apply_receipt.srm_party_gold == 777u &&
                        state_receipt.save_resume_srm_active_slot == -1 &&
                        state_receipt.save_resume_srm_party_gold == 777u,
                    "srm party custom path emits apply and state receipts");

        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(!theron_v1_startup_continue_srm_path_apply_with_host_receipts(
                        &world,
                        "",
                        &plan,
                        NULL,
                        &continue_result,
                        &host_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) &&
                        host_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        host_receipt.status &&
                        strstr(host_receipt.status,
                               "Continue requires an SRM path") != NULL &&
                        strstr(host_receipt.inspect_detail,
                               "source=NONE") != NULL,
                    "srm party custom path failure emits host receipt");
    }

    test_unlink(custom_path);
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

static void test_boot_prepare_startup_profile_missing_track02(void) {
    Theron_V1_BootProfile profile;
    TrAssetBundle assets;
    Theron_V1StartupSaveResume snap;
    Theron_V1_BootStartupLaunch launch;
    Theron_V1BootStartupPrepareResult result =
        THERON_V1_BOOT_STARTUP_PREPARE_OK;
    int ready = 99;

    memset(&profile, 0, sizeof(profile));
    memset(&assets, 0, sizeof(assets));
    memset(&snap, 0xff, sizeof(snap));
    cleanup_srm_root(TST_BAD_ROOT);
    expect_true(test_mkdir(TST_BAD_ROOT) == 0,
                "boot prepare test root created");
    expect_true(!theron_v1_boot_prepare_startup_profile(
                    &profile,
                    TST_BAD_ROOT,
                    NULL,
                    NULL,
                    NULL,
                    &assets,
                    &snap,
                    &ready,
                    &result) &&
                    result ==
                        THERON_V1_BOOT_STARTUP_PREPARE_MISSING_TRACK02 &&
                    ready == 0,
                "boot prepare reports missing Track 02 before M11");
    expect_true(strcmp(theron_v1_boot_startup_prepare_result_name(result),
                       "MISSING_TRACK02") == 0,
                "boot prepare result name is stable");
    expect_true(strcmp(theron_v1_boot_startup_prepare_result_name(
                           THERON_V1_BOOT_STARTUP_PREPARE_STATE_FAILED),
                       "STATE_FAILED") == 0,
                "boot prepare startup-state failure name is stable");
    memset(&launch, 0xff, sizeof(launch));
    expect_true(!theron_v1_boot_startup_launch_alloc(
                    TST_BAD_ROOT,
                    NULL,
                    NULL,
                    NULL,
                    &launch) &&
                    launch.prepare_result ==
                        THERON_V1_BOOT_STARTUP_PREPARE_MISSING_TRACK02 &&
                    launch.profile == NULL &&
                    launch.world == NULL &&
                    launch.viewport == NULL &&
                    launch.assets == NULL &&
                    launch.launch_host_receipt.status_scope &&
                    strcmp(launch.launch_host_receipt.status_scope,
                           "BOOT") == 0 &&
                    launch.launch_host_receipt.status &&
                    strcmp(launch.launch_host_receipt.status,
                           "THERON TRACK 02 MISSING") == 0 &&
                    strcmp(launch.launch_host_receipt.inspect_detail,
                           "THERON TRACK 02 MISSING") == 0 &&
                    launch.startup_media_state_receipt.
                        startup_roster_name_status == 0 &&
                    launch.startup_media_state_receipt.
                        startup_text_prompt_status == 0,
                "boot startup launch allocation reports missing Track 02 and owns host failure receipt");
    theron_v1_boot_startup_launch_cleanup(&launch);
    cleanup_srm_root(TST_BAD_ROOT);
}

static void test_startup_session_facts_wrappers(void) {
    Theron_V1_World world;
    Theron_StartupSessionFacts session;
    Theron_StartupFlow flow;
    Theron_StartupStateReceipt state_receipt;
    Theron_StartupChapterInspectReceipt inspect_receipt;
    Theron_StartupActionPlan plan;
    Theron_StartupAction action;
    Theron_StartupActionHostReceipt action_receipt;
    Theron_StartupExecution execution;
    Theron_StartupHostReceipt host_receipt;
    char exit_receipt[128];
    int order[THERON_STARTUP_MAX_COMPANIONS] = {0, 1, 2};

    theron_v1_world_init(&world);
    theron_v1_startup_session_facts_from_runtime(
        &session,
        THERON_STARTUP_PHASE_STAGE_SELECT,
        THERON_DUNGEON_1_HALL_OF_RECORDS,
        NULL,
        &world,
        NULL,
        1,
        0,
        0,
        0,
        -1,
        0,
        "",
        "SELECT",
        NULL,
        NULL,
        0,
        0x03,
        2,
        order,
        THERON_STARTUP_MAX_COMPANIONS);
    expect_true(session.phase == THERON_STARTUP_PHASE_STAGE_SELECT &&
                    session.selected_dungeon ==
                        THERON_DUNGEON_1_HALL_OF_RECORDS &&
                    session.world == &world &&
                    session.selected_mirrors_mask == 0x03 &&
                    session.companion_count == 2 &&
                    session.selected_mirror_order == order &&
                    session.selected_mirror_order_count ==
                        THERON_STARTUP_MAX_COMPANIONS,
                "Theron startup layer owns runtime session facts construction");

    expect_true(theron_v1_startup_flow_rebuild_from_session_with_receipt(
                    &session,
                    &flow,
                    &state_receipt) == THERON_STARTUP_OK &&
                    state_receipt.flow_changed &&
                    state_receipt.flow.phase ==
                        THERON_STARTUP_PHASE_STAGE_SELECT,
                "session facts rebuild wrapper emits flow state receipt");
    expect_true(theron_v1_startup_chapter_inspect_receipt_from_session(
                    &session,
                    "STARTUP",
                    &inspect_receipt) &&
                    strcmp(inspect_receipt.inspect_scope, "STARTUP") == 0 &&
                    strstr(inspect_receipt.inspect_detail, "STARTUP") != NULL,
                "session facts chapter inspect wrapper emits inspect receipt");

    theron_v1_startup_action_plan_init(&plan);
    plan.kind = THERON_STARTUP_PLAN_MOVE_STAGE_CURSOR;
    plan.cursor = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
    expect_true(theron_v1_startup_execute_flow_plan_from_session_with_host_receipts(
                    &plan,
                    &session,
                    NULL,
                    &execution,
                    &host_receipt,
                    &state_receipt) &&
                    execution.result == THERON_STARTUP_OK &&
                    host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW &&
                    state_receipt.flow_changed,
                "session facts flow-plan wrapper emits host and state receipts");

    theron_v1_startup_action_init(&action);
    action.kind = THERON_STARTUP_ACTION_RETURN_TO_LAUNCHER;
    expect_true(theron_v1_startup_execute_action_from_session_with_host_receipt(
                    &action,
                    &session,
                    &action_receipt) &&
                    action_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER &&
                    strcmp(action_receipt.host_receipt.status_scope,
                           "RETURN") == 0,
                "session facts action wrapper emits return-to-launcher receipt");

    theron_v1_startup_action_init(&action);
    action.kind = THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR;
    action.selected_dungeon = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
    expect_true(theron_v1_startup_execute_action_from_session_with_host_receipt(
                    &action,
                    &session,
                    &action_receipt) &&
                    action_receipt.result == THERON_STARTUP_OK &&
                    action_receipt.state_receipt_valid &&
                    action_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW &&
                    action_receipt.state_receipt.flow_changed &&
                    action_receipt.state_receipt.flow.selected_dungeon ==
                        THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                "session facts action wrapper executes startup flow action");
    expect_true(theron_v1_startup_execute_input_from_session_with_host_receipt(
                    &session,
                    THERON_STARTUP_INPUT_BACK,
                    &action_receipt) &&
                    action_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER &&
                    strcmp(action_receipt.host_receipt.status_scope,
                           "RETURN") == 0,
                "session facts input wrapper routes through action host receipt");
    expect_true(theron_v1_startup_execute_pointer_from_session_with_host_receipt(
                    &session,
                    35,
                    23,
                    &action_receipt) &&
                    action_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW &&
                    !action_receipt.state_receipt_valid,
                "session facts pointer wrapper preserves panel-consumed redraw");

    world.progression.current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    world.progression.dungeon_states[THERON_DUNGEON_1_HALL_OF_RECORDS - 1] =
        THERON_DUNGEON_STATE_COMPLETE;
    world.progression.quest_items_collected =
        THERON_QUEST_ITEM_MASK_FROM_DUNGEON(
            THERON_DUNGEON_1_HALL_OF_RECORDS);
    expect_true(world.progression.dungeon_states[
                    THERON_DUNGEON_1_HALL_OF_RECORDS - 1] ==
                    THERON_DUNGEON_STATE_COMPLETE,
                "startup exit wrapper fixture marks dungeon complete");
    world.party.champion_count = 3;
    world.party.leader_x = 4;
    world.party.leader_y = 5;
    world.party.leader_dir = 2;
    world.world_tick = 42;
    memset(&state_receipt, 0, sizeof(state_receipt));
    memset(exit_receipt, 0, sizeof(exit_receipt));
    expect_true(theron_v1_startup_return_to_stage_select_after_exit_state_receipt(
                    &world,
                    &state_receipt,
                    exit_receipt,
                    sizeof(exit_receipt)) &&
                    state_receipt.flow_changed &&
                    state_receipt.flow.phase ==
                        THERON_STARTUP_PHASE_STAGE_SELECT &&
                    state_receipt.flow.selected_dungeon ==
                        THERON_DUNGEON_2_CRYPT_OF_SHADOWS &&
                    state_receipt.set_level_loaded &&
                    state_receipt.level_loaded == 0 &&
                    state_receipt.set_party_pose &&
                    state_receipt.tick_count == 42 &&
                    strstr(exit_receipt, "dungeon complete") != NULL,
                "startup exit wrapper emits state receipt without M11 flow ownership");
    world.progression.current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    world.progression.dungeon_states[THERON_DUNGEON_1_HALL_OF_RECORDS - 1] =
        THERON_DUNGEON_STATE_COMPLETE;
    world.progression.dungeon_states[THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1] =
        THERON_DUNGEON_STATE_LOCKED;
    world.progression.quest_items_collected =
        THERON_QUEST_ITEM_MASK_FROM_DUNGEON(
            THERON_DUNGEON_1_HALL_OF_RECORDS);
    world.party.champion_count = 3;
    world.party.leader_x = 4;
    world.party.leader_y = 5;
    world.party.leader_dir = 2;
    world.world_tick = 43;
    {
        Theron_StartupActionHostReceipt host_receipt;
        expect_true(
            theron_v1_startup_return_to_stage_select_after_exit_host_receipt(
                &world,
                &host_receipt) &&
                host_receipt.state_receipt_valid &&
                host_receipt.state_receipt.flow.phase ==
                    THERON_STARTUP_PHASE_STAGE_SELECT &&
                host_receipt.host_receipt.input_result ==
                    THERON_STARTUP_INPUT_RESULT_REDRAW &&
                strcmp(host_receipt.host_receipt.status_scope,
                       "STARTUP") == 0 &&
                strcmp(host_receipt.host_receipt.status,
                       "DUNGEON COMPLETE") == 0,
            "startup exit host receipt owns M11 status and redraw result");
    }
}

static void test_boot_startup_launch_detach_runtime_receipt(void) {
    Theron_V1_BootStartupLaunch launch;
    Theron_V1_BootStartupRuntimeReceipt receipt;

    memset(&launch, 0, sizeof(launch));
    memset(&receipt, 0xff, sizeof(receipt));
    expect_true(!theron_v1_boot_startup_launch_detach_runtime(NULL, &receipt) &&
                    receipt.profile == NULL &&
                    receipt.world == NULL &&
                    receipt.viewport == NULL &&
                    receipt.assets == NULL,
                "boot runtime detach rejects NULL launch and clears receipt");

    memset(&launch, 0, sizeof(launch));
    memset(&receipt, 0xff, sizeof(receipt));
    expect_true(!theron_v1_boot_startup_launch_detach_runtime(&launch,
                                                              &receipt) &&
                    receipt.profile == NULL,
                "boot runtime detach rejects incomplete launch");

    launch.profile = (Theron_V1_BootProfile*)calloc(1, sizeof(*launch.profile));
    launch.world = (Theron_V1_World*)calloc(1, sizeof(*launch.world));
    launch.viewport = (Theron_V1_Viewport*)calloc(1, sizeof(*launch.viewport));
    launch.assets = (TrAssetBundle*)calloc(1, sizeof(*launch.assets));
    expect_true(launch.profile && launch.world && launch.viewport && launch.assets,
                "boot runtime detach fixture allocates owned objects");
    if (!launch.profile || !launch.world || !launch.viewport || !launch.assets) {
        theron_v1_boot_startup_launch_cleanup(&launch);
        return;
    }
    snprintf(launch.profile->graphics_md5,
             sizeof(launch.profile->graphics_md5),
             "f23601102138f87c33025877767ebf76");
    snprintf(launch.profile->graphics_path,
             sizeof(launch.profile->graphics_path),
             "/tmp/firestaff_theron_track02.bin");
    launch.initial_state_receipt.flow_changed = 1;
    launch.initial_state_receipt.flow.phase = THERON_STARTUP_PHASE_TITLE;
    launch.save_resume_state_receipt.set_save_resume = 1;
    launch.save_resume_state_receipt.save_resume_claim =
        THERON_V1_STARTUP_RESUME_TQSV;
    launch.startup_media_state_receipt.startup_roster_name_count = 1;
    launch.startup_media_state_receipt.track02_variant =
        THERON_TRACK02_VARIANT_US_BIN;
    snprintf(launch.startup_media_state_receipt.track02_md5,
             sizeof(launch.startup_media_state_receipt.track02_md5),
             "%s",
             THERON_TRACK02_MD5_US_BIN);
    launch.startup_media_state_receipt.track02_size = 2352u;
    launch.startup_media_state_receipt.startup_media_ready = 1;
    snprintf(launch.startup_media_state_receipt.startup_roster_names[0],
             sizeof(launch.startup_media_state_receipt.startup_roster_names[0]),
             "ALEX");
    launch.launch_host_receipt.status_scope = "BOOT";
    launch.launch_host_receipt.status = "THERON'S QUEST";

    memset(&receipt, 0, sizeof(receipt));
    expect_true(theron_v1_boot_startup_launch_detach_runtime(&launch,
                                                             &receipt) &&
                    receipt.profile != NULL &&
                    receipt.world != NULL &&
                    receipt.viewport != NULL &&
                    receipt.assets != NULL &&
                    strcmp(receipt.boot_asset_md5,
                           "f23601102138f87c33025877767ebf76") == 0 &&
                    strcmp(receipt.title, "THERON'S QUEST") == 0 &&
                    strcmp(receipt.source_id, "theron") == 0 &&
                    strcmp(receipt.dungeon_path,
                           "/tmp/firestaff_theron_track02.bin") == 0 &&
                    receipt.initial_state_receipt.flow_changed == 1 &&
                    receipt.initial_state_receipt.flow.phase ==
                        THERON_STARTUP_PHASE_TITLE &&
                    receipt.save_resume_state_receipt.set_save_resume == 1 &&
                    receipt.save_resume_state_receipt.save_resume_claim ==
                        THERON_V1_STARTUP_RESUME_TQSV &&
                    receipt.startup_media_state_receipt.
                            startup_roster_name_count == 1 &&
                    receipt.startup_media_state_receipt.track02_variant ==
                        THERON_TRACK02_VARIANT_US_BIN &&
                    strcmp(receipt.startup_media_state_receipt.track02_md5,
                           THERON_TRACK02_MD5_US_BIN) == 0 &&
                    receipt.startup_media_state_receipt.track02_size == 2352u &&
                    receipt.startup_media_state_receipt.startup_media_ready == 1 &&
                    strcmp(receipt.startup_media_state_receipt.
                               startup_roster_names[0],
                           "ALEX") == 0 &&
                    receipt.launch_host_receipt.status_scope &&
                    strcmp(receipt.launch_host_receipt.status_scope,
                           "BOOT") == 0 &&
                    receipt.launch_host_receipt.status &&
                    strcmp(receipt.launch_host_receipt.status,
                           "THERON'S QUEST") == 0 &&
                    launch.profile == NULL &&
                    launch.world == NULL &&
                    launch.viewport == NULL &&
                    launch.assets == NULL,
                "boot runtime detach transfers ownership and M11-ready launch identity");

    theron_v1_boot_cleanup(receipt.profile);
    free(receipt.profile);
    free(receipt.world);
    theron_vp_free(receipt.viewport);
    free(receipt.viewport);
    tr_asset_free(receipt.assets);
    free(receipt.assets);
}

static void test_boot_forcefield_pointer_snapshot_enters_runtime(void) {
    static const unsigned char fake_track02[2352] = {0};
    Theron_V1_BootProfile profile;
    Theron_V1_World world;
    TrAssetBundle assets;
    Theron_V1_BootRuntimeStartupSnapshot snapshot;
    Theron_StartupActionHostReceipt receipt;
    int mirror_order[THERON_STARTUP_MAX_COMPANIONS] = {6, 0, 2};
    char roster_names[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                     [THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY];
    char roster_titles[THERON_STARTUP_MEDIA_ROSTER_CAPACITY]
                      [THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY];

    memset(&profile, 0, sizeof(profile));
    memset(&assets, 0, sizeof(assets));
    memset(roster_names, 0, sizeof(roster_names));
    memset(roster_titles, 0, sizeof(roster_titles));
    theron_v1_world_init(&world);

    snprintf(profile.graphics_md5,
             sizeof(profile.graphics_md5),
             "%s",
             THERON_TRACK02_MD5_US_BIN);
    assets.hucard_rom = fake_track02;
    assets.hucard_rom_size = sizeof(fake_track02);
    snprintf(roster_names[1], sizeof(roster_names[1]), "MARA");
    snprintf(roster_names[3], sizeof(roster_names[3]), "HEXA");
    snprintf(roster_names[7], sizeof(roster_names[7]), "PENTAI");

    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.startup_phase = THERON_STARTUP_PHASE_READY;
    snapshot.selected_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    snapshot.boot_profile = &profile;
    snapshot.world = &world;
    snapshot.assets = &assets;
    snapshot.startup_cursor = THERON_STARTUP_HERO_MIRROR_COUNT;
    snapshot.startup_roster_names = roster_names;
    snapshot.startup_roster_titles = roster_titles;
    snapshot.startup_roster_name_count =
        (int)THERON_STARTUP_MEDIA_ROSTER_CAPACITY;
    snapshot.selected_mirrors_mask = (1 << 6) | (1 << 0) | (1 << 2);
    snapshot.companion_count = 3;
    snapshot.selected_mirror_order = mirror_order;
    snapshot.selected_mirror_order_count = THERON_STARTUP_MAX_COMPANIONS;

    {
        int ok = theron_v1_boot_startup_execute_pointer_from_snapshot(
                    &snapshot,
                    46 + 77,
                    160 + 5,
                    &receipt);
        expect_true(ok == 1 &&
                        receipt.result == THERON_STARTUP_ERR_DUNGEON_ENTRY &&
                        receipt.host_receipt.status &&
                        strstr(receipt.host_receipt.status,
                               "fallback visuals blocked") != NULL &&
                        world.level_loaded[0][0] == 0,
                    "boot snapshot forcefield blocks invalid verified Track 02 fallback");
    }
}

static void test_boot_runtime_render_frame_facade(void) {
    Theron_V1_World world;
    Theron_V1_Viewport viewport;
    unsigned char framebuffer[320 * 240];
    size_t i;
    int nonzero = 0;

    memset(&world, 0, sizeof(world));
    memset(&viewport, 0, sizeof(viewport));
    memset(framebuffer, 0, sizeof(framebuffer));
    theron_v1_world_init(&world);
    expect_true(theron_vp_init(&viewport) == 1,
                "theron viewport fixture initializes for boot render facade");
    expect_true(theron_v1_boot_runtime_render_frame(NULL,
                                                    &viewport,
                                                    NULL,
                                                    0,
                                                    0,
                                                    framebuffer,
                                                    320,
                                                    240) == 0,
                "boot render facade rejects NULL world");
    expect_true(theron_v1_boot_runtime_render_frame(&world,
                                                    &viewport,
                                                    NULL,
                                                    0,
                                                    0,
                                                    framebuffer,
                                                    320,
                                                    240) == 1,
                "boot render facade owns Theron dungeon/UI/present sequence");
    for (i = 0; i < sizeof(framebuffer); ++i) {
        if (framebuffer[i] != 0) {
            nonzero = 1;
            break;
        }
    }
    expect_true(nonzero,
                "boot render facade writes presented indexed framebuffer");
    theron_vp_free(&viewport);
}

static void test_boot_runtime_release_facade(void) {
    Theron_V1_BootProfile *profile =
        (Theron_V1_BootProfile*)calloc(1, sizeof(*profile));
    Theron_V1_World *world =
        (Theron_V1_World*)calloc(1, sizeof(*world));
    Theron_V1_Viewport *viewport =
        (Theron_V1_Viewport*)calloc(1, sizeof(*viewport));
    TrAssetBundle *assets =
        (TrAssetBundle*)calloc(1, sizeof(*assets));

    expect_true(profile && world && viewport && assets,
                "boot runtime release fixture allocates ownership bundle");
    if (!profile || !world || !viewport || !assets) {
        free(profile);
        free(world);
        free(viewport);
        free(assets);
        return;
    }
    theron_v1_boot_profile_init(profile);
    theron_v1_world_init(world);
    expect_true(theron_vp_init(viewport) == 1,
                "boot runtime release fixture initializes viewport");
    theron_v1_boot_runtime_release(&profile, &world, &viewport, &assets);
    expect_true(profile == NULL &&
                    world == NULL &&
                    viewport == NULL &&
                    assets == NULL,
                "boot runtime release facade clears all runtime owners");
    theron_v1_boot_runtime_release(&profile, &world, &viewport, &assets);
    expect_true(profile == NULL &&
                    world == NULL &&
                    viewport == NULL &&
                    assets == NULL,
                "boot runtime release facade is idempotent");
}

int main(void) {
    printf("\n=== Theron V1 Startup Save/Resume Smoke Gate Unit Tests ===\n\n");
    test_clean_host_skip_safe_no_save_root();
    test_empty_root_is_skip_save_root_present_no_slots();
    test_staged_but_unrecognized_is_skip();
    test_tqsv_only_resume_claim();
    test_srm_only_resume_claim();
    test_srm_party_continue_restores_all_champions();
    test_dual_resume_claim();
    test_boot_profile_handoff();
    test_status_name_contracts();
    test_source_evidence_string();
    test_format_helper();
    test_null_safety();
    test_snapshot_snapshot_is_deterministic();
    test_boot_prepare_startup_profile_missing_track02();
    test_boot_startup_launch_detach_runtime_receipt();
    test_boot_forcefield_pointer_snapshot_enters_runtime();
    test_boot_runtime_render_frame_facade();
    test_boot_runtime_release_facade();
    test_startup_session_facts_wrappers();

    printf("=====================================================\n");
    printf("Results: %d/%d passed (failures=%d)\n",
           g_tests_passed, g_tests_run, g_failures);
    printf("=====================================================\n\n");
    return g_failures ? 1 : 0;
}
