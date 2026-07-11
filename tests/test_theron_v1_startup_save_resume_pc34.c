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
#include "theron_v1_startup_receipt.h"
#include "theron_v1_startup_runtime_entry.h"
#include "theron_v2_hud_overlay_pc34.h"

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
#define TST_THERON_FULL_START_BITMAP_ROUTES \
    (THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE | \
     THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE | \
     THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM | \
     THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD)

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

static void make_complete_track02_media_receipt(
    Theron_StartupMediaStateReceipt *media) {
    const unsigned int route_bits[4] = {
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM,
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD
    };

    memset(media, 0, sizeof(*media));
    media->startup_media_ready = 1;
    media->startup_bitmap_decode_status = THERON_TRACK02_SIGNAL_OK;
    media->startup_bitmap_sample_count = 48;
    media->startup_bitmap_route_mask = TST_THERON_FULL_START_BITMAP_ROUTES;
    media->startup_bitmap_nonzero_pixel_count = 128u;
    media->startup_bitmap_checksum = 0x134679acu;
    media->startup_bitmap_title_route_ready = 1;
    media->startup_bitmap_stage_route_ready = 1;
    media->startup_bitmap_soul_room_route_ready = 1;
    media->startup_bitmap_forcefield_route_ready = 1;
    media->startup_bitmap_atlas_ready = 1;
    media->startup_bitmap_atlas_route_count = 4;
    media->startup_bitmap_atlas_route_mask = TST_THERON_FULL_START_BITMAP_ROUTES;
    media->startup_bitmap_atlas_tile_count = 48u;
    media->startup_bitmap_atlas_nonzero_pixel_count = 128u;
    media->startup_bitmap_atlas_checksum = 0x2468ace1u;
    media->runtime_media_identity.ready = 1;
    media->runtime_media_identity.track02_variant =
        THERON_TRACK02_VARIANT_US_BIN;
    media->runtime_media_identity.bank_anchor_index = 0u;
    media->runtime_media_identity.bank_descriptor_offset = 0x70be06u;
    media->runtime_media_identity.bank_first_value = 0x0020u;
    media->runtime_media_identity.bank_last_value = 0x2020u;
    media->runtime_media_identity.bank_stride = 0x0400u;
    media->runtime_media_identity.audio_frame_ready = 1;
    media->runtime_media_identity.audio_bank_id = 0x01725800u;
    media->runtime_media_identity.audio_bank_id_offset = 0x2d53dcu;
    media->runtime_media_identity.audio_bank_prefix_offset = 0x2d53d0u;
    media->runtime_media_identity.checksum = 0x71a3b1c2u;
    media->startup_bitmap_wide_route_mask = TST_THERON_FULL_START_BITMAP_ROUTES;
    media->startup_bitmap_wide_route_count = 4;
    media->startup_bitmap_wide_atlas_tile_count = 48u;
    media->startup_bitmap_raw_route_mask = TST_THERON_FULL_START_BITMAP_ROUTES;
    media->startup_bitmap_raw_route_count = 4;
    media->startup_bitmap_raw_atlas_tile_count = 48u;
    media->startup_bitmap_title_sample_count = 12;
    media->startup_bitmap_stage_sample_count = 12;
    media->startup_bitmap_soul_room_sample_count = 12;
    media->startup_bitmap_forcefield_sample_count = 12;
    media->startup_bitmap_title_nonzero_pixel_count = 32u;
    media->startup_bitmap_stage_nonzero_pixel_count = 32u;
    media->startup_bitmap_soul_room_nonzero_pixel_count = 32u;
    media->startup_bitmap_forcefield_nonzero_pixel_count = 32u;
    media->startup_bitmap_title_checksum = 1u;
    media->startup_bitmap_stage_checksum = 2u;
    media->startup_bitmap_soul_room_checksum = 3u;
    media->startup_bitmap_forcefield_checksum = 4u;
    media->startup_bitmap_title_atlas_tile_count = 12u;
    media->startup_bitmap_stage_atlas_tile_count = 12u;
    media->startup_bitmap_soul_room_atlas_tile_count = 12u;
    media->startup_bitmap_forcefield_atlas_tile_count = 12u;
    media->startup_bitmap_title_atlas_width = 96u;
    media->startup_bitmap_stage_atlas_width = 96u;
    media->startup_bitmap_soul_room_atlas_width = 96u;
    media->startup_bitmap_forcefield_atlas_width = 96u;
    media->startup_bitmap_atlas.route_count = 4u;
    media->startup_bitmap_atlas.route_mask = TST_THERON_FULL_START_BITMAP_ROUTES;
    for (size_t i = 0u; i < 4u; ++i) {
        Theron_Track02StartupBitmapAtlasRoute *route =
            &media->startup_bitmap_atlas.routes[i];
        route->route_bit = route_bits[i];
        route->tile_count = 12u;
        route->width = 96u;
        route->height = 8u;
        route->nonzero_pixel_count = 32u;
        route->checksum = (uint32_t)(0x100u + i);
        memset(route->pixels, (int)(i + 1u), 96u * 8u);
    }
}

static void wr16le_test(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static void wr16be_test(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)((v >> 8) & 0xffu);
    p[1] = (uint8_t)(v & 0xffu);
}

static void wr32be_test(uint8_t *p, uint32_t v) {
    wr16be_test(p, (uint16_t)((v >> 16) & 0xffffu));
    wr16be_test(p + 2, (uint16_t)(v & 0xffffu));
}

static void wr32le_test(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
    p[2] = (uint8_t)((v >> 16) & 0xffu);
    p[3] = (uint8_t)((v >> 24) & 0xffu);
}

typedef struct TestStartupGraphicsCounters {
    int fill_count;
    int rect_count;
    int pixel_count;
} TestStartupGraphicsCounters;

static void test_startup_fill_rect(void *userdata,
                                   int x,
                                   int y,
                                   int w,
                                   int h,
                                   int color) {
    TestStartupGraphicsCounters *counters =
        (TestStartupGraphicsCounters *)userdata;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)color;
    if (counters) {
        ++counters->fill_count;
    }
}

static void test_startup_draw_rect(void *userdata,
                                   int x,
                                   int y,
                                   int w,
                                   int h,
                                   int color) {
    TestStartupGraphicsCounters *counters =
        (TestStartupGraphicsCounters *)userdata;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)color;
    if (counters) {
        ++counters->rect_count;
    }
}

static void test_startup_plot_pixel(void *userdata,
                                    int x,
                                    int y,
                                    int color) {
    TestStartupGraphicsCounters *counters =
        (TestStartupGraphicsCounters *)userdata;
    (void)x;
    (void)y;
    (void)color;
    if (counters) {
        ++counters->pixel_count;
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
        Theron_StartupMediaStateReceipt media_receipt;
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
        make_complete_track02_media_receipt(&media_receipt);
        continue_request.track02_media_receipt = &media_receipt;
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
                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS &&
                        world.runtime_media.restored &&
                        world.runtime_media.route_mask ==
                            TST_THERON_FULL_START_BITMAP_ROUTES &&
                        world.runtime_media.title.ready &&
                        world.runtime_media.title.route_bit ==
                            THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE &&
                        world.runtime_media.title.pixels[0] == 1u &&
                        theron_v1_world_runtime_media_for_level(
                            &world, 1, 0) == &world.runtime_media.stage &&
                        world.runtime_media.stage.ready &&
                        world.runtime_media.stage.route_bit ==
                            THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE &&
                        theron_v1_world_runtime_media_for_level(
                            &world, 1, 1) == &world.runtime_media.forcefield &&
                        world.runtime_media.stage.pixels[0] == 2u &&
                        world.runtime_media.soul_room.ready &&
                        world.runtime_media.soul_room.route_bit ==
                            THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM &&
                        world.runtime_media.soul_room.pixels[0] == 3u &&
                        world.runtime_media.forcefield.ready &&
                        world.runtime_media.forcefield.route_bit ==
                            THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD &&
                        world.runtime_media.forcefield.pixels[0] == 4u &&
                        world.runtime_media.identity.ready &&
                        world.runtime_media.identity.bank_descriptor_offset ==
                            0x70be06u &&
                        world.runtime_media.identity.audio_frame_ready &&
                        world.runtime_media.identity.audio_bank_id ==
                            0x01725800u &&
                        continue_result.track02_level_bank.ready &&
                        continue_result.track02_level_bank.real_media_gate &&
                        continue_result.track02_level_bank.kind ==
                            THERON_RUNTIME_LEVEL_BANK_SAVE_RESUME &&
                        continue_result.track02_level_bank.dungeon_id ==
                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS &&
                        continue_result.track02_level_bank.level_index ==
                            world.current_level &&
                        continue_result.track02_level_bank.route_bit ==
                            THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE &&
                        continue_result.track02_level_bank.surface_checksum ==
                            world.runtime_media.stage.checksum &&
                        continue_result.track02_level_bank.identity_checksum ==
                            world.runtime_media.identity.checksum &&
                        strstr(receipt, "level_bank=2:") != NULL,
                    "tqsv Continue restores Track 02 later-level media and level-bank handoff with world");
        world.transition_pending = 1;
        world.transition_target_level = 2;
        expect_true(theron_v1_transition_execute(&world) == 0 &&
                        !world.runtime_media.level_bank.ready,
                    "Track 02 level-bank cache invalidates on runtime level transition");
        world.current_level = 2;
        expect_true(theron_v1_world_runtime_media_select_level_bank(
                        &world,
                        THERON_RUNTIME_LEVEL_BANK_SAVE_RESUME,
                        THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                        world.current_level) &&
                        world.runtime_media.level_bank.ready &&
                        world.runtime_media.level_bank.level_index == 2 &&
                        world.runtime_media.level_bank.route_bit ==
                            THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE,
                    "Track 02 later-level bank can be rebound after runtime transition");
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
        theron_v1_startup_continue_request_init(&continue_request);
        continue_request.resume_claim = THERON_V1_STARTUP_RESUME_TQSV;
        continue_request.tqsv_slot_index = 2;
        continue_request.tqsv_root = tqsv_root;
        memset(&media_receipt, 0, sizeof(media_receipt));
        media_receipt.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
        continue_request.track02_media_receipt = &media_receipt;
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(!theron_v1_startup_continue_apply_request(
                        &world,
                        &continue_request,
                        &continue_result,
                        receipt,
                        sizeof(receipt)) &&
                        strstr(receipt, "fallback visuals blocked") != NULL &&
                        world.level_loaded[0][0] == 0 &&
                        !world.runtime_media.restored,
                    "tqsv Continue rejects incomplete Track 02 media before restore");
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
        Theron_V1StartupContinueRequest continue_request;
        Theron_V1StartupContinueResult continue_result;
        Theron_StartupMediaStateReceipt media_receipt;

        theron_v1_startup_continue_request_init(&continue_request);
        continue_request.srm_slot_index = 2;
        continue_request.srm_root = srm_root;
        continue_request.srm_import_status =
            THERON_V1_SRM_PROGRESS_IMPORT_OK;
        memset(&media_receipt, 0, sizeof(media_receipt));
        media_receipt.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
        continue_request.track02_media_receipt = &media_receipt;
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(!theron_v1_startup_continue_apply_request(
                        &world,
                        &continue_request,
                        &continue_result,
                        receipt,
                        sizeof(receipt)) &&
                        strstr(receipt, "fallback visuals blocked") != NULL &&
                        world.progression.current_dungeon ==
                            THERON_DUNGEON_1_HALL_OF_RECORDS &&
                        !world.runtime_media.restored,
                    "srm Continue rejects incomplete Track 02 media before restore");
    }
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
                        state_receipt.set_runtime_level_route &&
                        state_receipt.runtime_level_source ==
                            THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME &&
                        state_receipt.runtime_structured_route &&
                        !state_receipt.runtime_receipt_text_route &&
                        !state_receipt.runtime_fallback_visuals_blocked &&
                        state_receipt.save_resume_srm_party_champion_count ==
                            THERON_MAX_CHAMPIONS &&
                        state_receipt.save_resume_srm_party_gold == 777u,
                    "srm party slot emits host and state receipts");
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(theron_v1_startup_continue_srm_apply_with_receipts(
                        &world,
                        srm_root,
                        2,
                        &plan,
                        NULL,
                        &continue_result,
                        &apply_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) == 1 &&
                        continue_result.source ==
                            THERON_V1_STARTUP_CONTINUE_SOURCE_SRM &&
                        continue_result.source_slot_index == 2 &&
                        apply_receipt.source ==
                            THERON_V1_STARTUP_CONTINUE_SOURCE_SRM &&
                        apply_receipt.source_slot_index == 2 &&
                        apply_receipt.srm_import_status ==
                            THERON_V1_SRM_PROGRESS_IMPORT_OK &&
                        apply_receipt.srm_current_dungeon ==
                            THERON_DUNGEON_3_ABYSS_OF_FLAMES &&
                        apply_receipt.srm_party_champion_count ==
                            THERON_MAX_CHAMPIONS &&
                        apply_receipt.srm_party_gold == 777u &&
                        state_receipt.set_save_resume &&
                        state_receipt.save_resume_srm_active_slot == 2 &&
                        state_receipt.save_resume_srm_current_dungeon ==
                            THERON_DUNGEON_3_ABYSS_OF_FLAMES &&
                        state_receipt.save_resume_srm_party_gold == 777u,
                    "srm party slot emits apply and state receipts");
        theron_v1_world_init(&world);
        memset(receipt, 0, sizeof(receipt));
        expect_true(!theron_v1_startup_continue_srm_apply_with_receipts(
                        &world,
                        srm_root,
                        4,
                        &plan,
                        NULL,
                        &continue_result,
                        &apply_receipt,
                        &state_receipt,
                        receipt,
                        sizeof(receipt)) &&
                        apply_receipt.input_result ==
                            THERON_STARTUP_INPUT_RESULT_REDRAW &&
                        apply_receipt.source ==
                            THERON_V1_STARTUP_CONTINUE_SOURCE_NONE &&
                        apply_receipt.source_slot_index == -1 &&
                        strstr(apply_receipt.status,
                               "SRM decode unsupported") != NULL &&
                        strstr(apply_receipt.inspect_detail,
                               "source=NONE") != NULL,
                    "srm party empty slot emits apply failure receipt");
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
                        state_receipt.set_runtime_level_route &&
                        state_receipt.runtime_level_source ==
                            THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME &&
                        state_receipt.runtime_structured_route &&
                        !state_receipt.runtime_receipt_text_route &&
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
    Theron_StartupActionHostReceipt view_model_host_receipt;
    Theron_StartupExecution execution;
    Theron_StartupHostReceipt host_receipt;
    Theron_V1_BootRuntimeStartupSnapshot snapshot;
    Theron_V1_BootRuntimeStartupSnapshot media_snapshot;
    Theron_V1_BootRuntimeStartupSnapshot blocked_snapshot;
    Theron_V1_BootRuntimeStartupSnapshot semantic_snapshot;
    Theron_V1_BootRuntimeStartupSnapshot semantic_level_snapshot;
    Theron_V1_BootRuntimeStartupSnapshot save_resume_snapshot;
    Theron_V1_BootStartupViewModel view_model;
    Theron_V1_BootStartupViewModel direct_view_model;
    Theron_V1_BootStartupViewModel media_view_model;
    Theron_V1_BootStartupViewModel blocked_view_model;
    Theron_V1_BootStartupViewModel semantic_view_model;
    Theron_V1_BootStartupViewModel semantic_level_view_model;
    Theron_V1_BootStartupViewModel save_resume_view_model;
    Theron_StartupMediaStateReceipt media_receipt;
    Theron_StartupLayoutElement media_layout[THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP];
    Theron_StartupRenderPlan media_plan;
    Theron_V1_BootStartupRenderRouteReceipt render_route_receipt;
    Theron_V1_BootStartupHostViewReceipt host_view_receipt;
    Theron_V1_BootStartupGraphicsRouteReceipt graphics_route_receipt;
    Theron_V1_BootStartupFullStartReceipt full_start_receipt;
    Theron_V1_BootStartupHostRenderReceipt host_render_receipt;
    Theron_V1_BootStartupMenuRuntimeHandoffReceipt handoff_receipt;
    Theron_V1_BootStartupUiCallerReceipt ui_caller_receipt;
    Theron_StartupAction media_pointer_action;
    Theron_StartupInputReceipt media_pointer_receipt;
    Theron_StartupAction media_input_action;
    Theron_StartupInputReceipt media_input_receipt;
    Theron_StartupAction stage_input_action;
    Theron_StartupInputReceipt stage_input_receipt;
    Theron_StartupActionHostReceipt legacy_host_receipt;
    Theron_StartupGraphicExecutor media_graphics_executor;
    TestStartupGraphicsCounters media_graphics_counters;
    char media_rows[THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP]
        [THERON_STARTUP_RENDER_ROW_CAPACITY];
    char exit_receipt[128];
    int order[THERON_STARTUP_MAX_COMPANIONS] = {0, 1, 2};
    int media_prompt_row_found;
    int media_roster_row_found;
    int media_layout_roster_found;
    int i;

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

    world.current_dungeon = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
    world.current_level = 0;
    world.level_loaded[THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1][0] = 1;
    world.party.champion_count = 3;
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.startup_phase = THERON_STARTUP_PHASE_STAGE_SELECT;
    snapshot.selected_dungeon = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
    snapshot.world = &world;
    snapshot.startup_cursor = 1;
    snapshot.continue_focus = 1;
    snapshot.resume_claim = THERON_V1_STARTUP_RESUME_DUAL;
    snapshot.tqsv_slot = 2;
    snapshot.srm_slot = 3;
    snapshot.srm_import_status = THERON_V1_SRM_PROGRESS_IMPORT_OK;
    snapshot.srm_root = "/tmp/firestaff-theron-srm";
    snapshot.startup_text_prompt = "SELECT";
    snapshot.selected_mirrors_mask = 0x03;
    snapshot.companion_count = 2;
    snapshot.selected_mirror_order = order;
    snapshot.selected_mirror_order_count =
        THERON_STARTUP_MAX_COMPANIONS;
    theron_v1_startup_media_state_receipt_init(&media_receipt);
    media_receipt.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(media_receipt.track02_md5,
             sizeof(media_receipt.track02_md5),
             "%s",
             THERON_TRACK02_MD5_US_BIN);
    media_receipt.track02_size = 123456u;
    media_receipt.startup_media_ready = 1;
    media_receipt.startup_roster_name_status = THERON_TRACK02_SIGNAL_OK;
    media_receipt.startup_roster_name_count = 8;
    snprintf(media_receipt.startup_roster_names[0],
             sizeof(media_receipt.startup_roster_names[0]),
             "THERON");
    snprintf(media_receipt.startup_roster_titles[0],
             sizeof(media_receipt.startup_roster_titles[0]),
             "SAVED");
    snprintf(media_receipt.startup_roster_names[4],
             sizeof(media_receipt.startup_roster_names[4]),
             "HAKAR-MEDIA");
    snprintf(media_receipt.startup_roster_titles[4],
             sizeof(media_receipt.startup_roster_titles[4]),
             "THE BRAVE");
    media_receipt.startup_text_prompt_status = THERON_TRACK02_SIGNAL_OK;
    media_receipt.startup_text_prompt_count = 1;
    snprintf(media_receipt.startup_text_prompt,
             sizeof(media_receipt.startup_text_prompt),
             "GO AWAY AND RESURRECT THERON");
    media_receipt.startup_bitmap_decode_status = THERON_TRACK02_SIGNAL_OK;
    media_receipt.startup_bitmap_sample_count = 48;
    media_receipt.startup_bitmap_route_mask =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    media_receipt.startup_bitmap_nonzero_pixel_count = 384u;
    media_receipt.startup_bitmap_checksum = 0x71f02u;
    media_receipt.startup_bitmap_title_route_ready = 1;
    media_receipt.startup_bitmap_stage_route_ready = 1;
    media_receipt.startup_bitmap_soul_room_route_ready = 1;
    media_receipt.startup_bitmap_forcefield_route_ready = 1;
    media_receipt.startup_bitmap_atlas_ready = 1;
    media_receipt.startup_bitmap_atlas_route_count = 4;
    media_receipt.startup_bitmap_atlas_route_mask =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    media_receipt.startup_bitmap_raw_route_mask =
        media_receipt.startup_bitmap_atlas_route_mask;
    media_receipt.startup_bitmap_raw_route_count = 4;
    media_receipt.startup_bitmap_raw_atlas_tile_count = 48u;
    media_receipt.startup_bitmap_iso_route_mask = 0u;
    media_receipt.startup_bitmap_iso_route_count = 0;
    media_receipt.startup_bitmap_iso_atlas_tile_count = 0u;
    media_receipt.startup_bitmap_wide_route_mask =
        media_receipt.startup_bitmap_atlas_route_mask;
    media_receipt.startup_bitmap_wide_route_count = 4;
    media_receipt.startup_bitmap_wide_atlas_tile_count = 48u;
    media_receipt.startup_bitmap_atlas_tile_count = 48u;
    media_receipt.startup_bitmap_atlas_nonzero_pixel_count = 384u;
    media_receipt.startup_bitmap_atlas_checksum = 0x9163u;
    media_receipt.startup_bitmap_atlas.variant = THERON_TRACK02_VARIANT_US_BIN;
    media_receipt.startup_bitmap_atlas.route_count = 4u;
    media_receipt.startup_bitmap_atlas.route_mask =
        media_receipt.startup_bitmap_atlas_route_mask;
    media_receipt.startup_bitmap_atlas.promoted_wide_route_mask =
        media_receipt.startup_bitmap_atlas_route_mask;
    media_receipt.startup_bitmap_atlas.promoted_wide_tile_count = 16u;
    media_receipt.startup_bitmap_atlas.total_tile_count = 48u;
    media_receipt.startup_bitmap_atlas.total_nonzero_pixel_count = 384u;
    media_receipt.startup_bitmap_atlas.checksum = 0x9163u;
    media_receipt.startup_bitmap_atlas.routes[0].route_bit =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE;
    media_receipt.startup_bitmap_atlas.routes[1].route_bit =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE;
    media_receipt.startup_bitmap_atlas.routes[2].route_bit =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM;
    media_receipt.startup_bitmap_atlas.routes[3].route_bit =
        THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    for (i = 0; i < 4; ++i) {
        size_t p;
        media_receipt.startup_bitmap_atlas.routes[i].tile_count = 12u;
        media_receipt.startup_bitmap_atlas.routes[i].width = 96u;
        media_receipt.startup_bitmap_atlas.routes[i].height = 8u;
        media_receipt.startup_bitmap_atlas.routes[i].nonzero_pixel_count = 96u;
        media_receipt.startup_bitmap_atlas.routes[i].checksum =
            (uint32_t)(0x9200u + (unsigned int)i);
        for (p = 0u;
             p < THERON_TRACK02_STARTUP_BITMAP_ATLAS_PIXELS;
             ++p) {
            media_receipt.startup_bitmap_atlas.routes[i].pixels[p] =
                (uint8_t)(((p + (size_t)i) % 15u) + 1u);
        }
    }
    media_receipt.startup_bitmap_title_sample_count = 12;
    media_receipt.startup_bitmap_stage_sample_count = 12;
    media_receipt.startup_bitmap_soul_room_sample_count = 12;
    media_receipt.startup_bitmap_forcefield_sample_count = 12;
    media_receipt.startup_bitmap_title_nonzero_pixel_count = 96u;
    media_receipt.startup_bitmap_stage_nonzero_pixel_count = 96u;
    media_receipt.startup_bitmap_soul_room_nonzero_pixel_count = 96u;
    media_receipt.startup_bitmap_forcefield_nonzero_pixel_count = 96u;
    media_receipt.startup_bitmap_title_checksum = 0x7101u;
    media_receipt.startup_bitmap_stage_checksum = 0x7102u;
    media_receipt.startup_bitmap_soul_room_checksum = 0x7104u;
    media_receipt.startup_bitmap_forcefield_checksum = 0x7108u;
    media_receipt.startup_bitmap_title_atlas_tile_count = 12u;
    media_receipt.startup_bitmap_stage_atlas_tile_count = 12u;
    media_receipt.startup_bitmap_soul_room_atlas_tile_count = 12u;
    media_receipt.startup_bitmap_forcefield_atlas_tile_count = 12u;
    media_receipt.startup_bitmap_title_sample_count = 12;
    media_receipt.startup_bitmap_stage_sample_count = 12;
    media_receipt.startup_bitmap_soul_room_sample_count = 12;
    media_receipt.startup_bitmap_forcefield_sample_count = 12;
    media_receipt.startup_bitmap_sample_count = 48;
    media_receipt.startup_bitmap_atlas_tile_count = 48u;
    media_receipt.startup_bitmap_title_atlas_width = 96u;
    media_receipt.startup_bitmap_stage_atlas_width = 96u;
    media_receipt.startup_bitmap_soul_room_atlas_width = 96u;
    media_receipt.startup_bitmap_forcefield_atlas_width = 96u;
    media_snapshot = snapshot;
    media_snapshot.startup_phase = THERON_STARTUP_PHASE_READY;
    media_snapshot.startup_text_prompt = NULL;
    media_snapshot.startup_roster_names = NULL;
    media_snapshot.startup_roster_titles = NULL;
    media_snapshot.startup_roster_name_count = 0;
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    memset(&media_graphics_executor, 0, sizeof(media_graphics_executor));
    media_graphics_executor.userdata = &media_graphics_counters;
    media_graphics_executor.fill_rect = test_startup_fill_rect;
    media_graphics_executor.draw_rect = test_startup_draw_rect;
    media_graphics_executor.plot_pixel = test_startup_plot_pixel;
    expect_true(theron_v1_boot_startup_view_model_from_snapshot(
                    &snapshot,
                    &view_model) &&
                    view_model.row_count > 0 &&
                    view_model.render_plan_valid &&
                    view_model.continue_focus == 1 &&
                    view_model.resume_claim ==
                        THERON_V1_STARTUP_RESUME_DUAL &&
                    view_model.tqsv_slot == 2 &&
                    view_model.srm_slot == 3 &&
                    view_model.srm_import_status ==
                        THERON_V1_SRM_PROGRESS_IMPORT_OK &&
                    view_model.runtime_level == 0 &&
                    view_model.runtime_champion_count == 3 &&
                    view_model.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM &&
                    view_model.runtime_track02_semantic_handoff == 0 &&
                    view_model.runtime_fallback_visuals_blocked == 0,
                "boot startup view model carries menu save and runtime route receipts");
    expect_true(theron_v1_boot_startup_view_model_from_runtime_state(
                    &direct_view_model,
                    THERON_STARTUP_PHASE_STAGE_SELECT,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    1,
                    1,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    "SELECT",
                    NULL,
                    NULL,
                    0,
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS) &&
                    direct_view_model.row_count == view_model.row_count &&
                    direct_view_model.render_plan_valid &&
                    direct_view_model.continue_focus == 1 &&
                    direct_view_model.resume_claim ==
                        THERON_V1_STARTUP_RESUME_DUAL &&
                    direct_view_model.tqsv_slot == 2 &&
                    direct_view_model.srm_slot == 3 &&
                    direct_view_model.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM &&
                    direct_view_model.runtime_champion_count == 3,
                "boot runtime-state view model wrapper carries menu save and route receipts");
    expect_true(theron_v1_boot_startup_full_start_receipt_from_view_model(
                    &direct_view_model,
                    NULL,
                    &full_start_receipt) &&
                    theron_v1_boot_startup_host_render_receipt_from_full_start_receipt(
                        &full_start_receipt,
                        &host_render_receipt) &&
                    host_render_receipt.layout_count ==
                        direct_view_model.layout_count &&
                    host_render_receipt.layout[0].kind ==
                        direct_view_model.layout[0].kind &&
                    host_render_receipt.row_count ==
                        direct_view_model.row_count &&
                    strcmp(host_render_receipt.rows[0],
                           direct_view_model.rows[0]) == 0 &&
                    host_render_receipt.render_plan_valid &&
                    host_render_receipt.render_plan.text_count ==
                        direct_view_model.render_plan.text_count &&
                    host_render_receipt.raw_session_rebuild_required == 0,
                "boot runtime host-render receipt replaces layout row and render-plan wrappers");
    expect_true(theron_v1_boot_startup_execute_input_from_runtime_state(
                    &legacy_host_receipt,
                    THERON_STARTUP_PHASE_STAGE_SELECT,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    1,
                    1,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    "SELECT",
                    NULL,
                    NULL,
                    0,
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS,
                    9) &&
                    legacy_host_receipt.result == THERON_STARTUP_ERR_NULL &&
                    legacy_host_receipt.host_receipt.status &&
                    strcmp(legacy_host_receipt.host_receipt.status,
                           "CONTINUE FAILED") == 0,
                "boot runtime input host wrapper consumes startup view model");
    expect_true(theron_v1_boot_startup_execute_pointer_from_runtime_state(
                    &legacy_host_receipt,
                    THERON_STARTUP_PHASE_STAGE_SELECT,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    1,
                    1,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    "SELECT",
                    NULL,
                    NULL,
                    0,
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS,
                    45,
                    67) &&
                    legacy_host_receipt.result == THERON_STARTUP_ERR_NULL &&
                    legacy_host_receipt.host_receipt.status &&
                    strcmp(legacy_host_receipt.host_receipt.status,
                           "CONTINUE FAILED") == 0,
                "boot runtime pointer host wrapper consumes startup view model");
    expect_true(theron_v1_boot_startup_full_start_receipt_from_view_model(
                    &view_model,
                    NULL,
                    &full_start_receipt) &&
                    theron_v1_boot_startup_host_render_receipt_from_full_start_receipt(
                        &full_start_receipt,
                        &host_render_receipt) &&
                    host_render_receipt.render_plan_valid &&
                    host_render_receipt.render_plan.text_count ==
                        view_model.render_plan.text_count,
                "boot snapshot host-render receipt replaces render-plan wrapper");
    expect_true(theron_v1_boot_startup_execute_input_from_snapshot(
                    &snapshot,
                    9,
                    &legacy_host_receipt) &&
                    legacy_host_receipt.result == THERON_STARTUP_ERR_NULL &&
                    legacy_host_receipt.host_receipt.status &&
                    strcmp(legacy_host_receipt.host_receipt.status,
                           "CONTINUE FAILED") == 0,
                "boot snapshot input host wrapper consumes startup view model");
    expect_true(theron_v1_boot_startup_view_model_from_runtime_state_with_media_receipt(
                    &media_view_model,
                    &media_receipt,
                    THERON_STARTUP_PHASE_READY,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    1,
                    1,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    NULL,
                    NULL,
                    NULL,
                    0,
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS) &&
                    media_view_model.startup_media_state_valid &&
                    media_view_model.startup_media_state_receipt
                            .startup_media_ready == 1 &&
                    media_view_model.startup_media_state_receipt
                            .track02_variant ==
                        THERON_TRACK02_VARIANT_US_BIN &&
                    media_view_model.startup_media_state_receipt
                            .startup_roster_name_count == 8 &&
                    strcmp(media_view_model.startup_media_state_receipt
                               .startup_roster_names[0],
                           "THERON") == 0 &&
                    strstr(media_view_model.startup_media_state_receipt
                               .startup_text_prompt,
                           "RESURRECT THERON") != NULL &&
                    media_view_model.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM,
                "boot runtime-state view model carries Track02 title/menu media receipt");
    media_prompt_row_found = 0;
    media_roster_row_found = 0;
    for (i = 0; i < media_view_model.row_count; ++i) {
        if (strstr(media_view_model.rows[i],
                   "RESURRECT THERON") != NULL) {
            media_prompt_row_found = 1;
        }
        if (strstr(media_view_model.rows[i],
                   "HAKAR-MEDIA") != NULL) {
            media_roster_row_found = 1;
        }
    }
    expect_true(media_prompt_row_found && media_roster_row_found,
                "boot startup view model consumes Track02 media receipt for prompt and roster rows");
    expect_true(theron_v1_boot_startup_render_rows_from_view_model(
                    &media_view_model,
                    media_rows,
                    THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP) ==
                    media_view_model.row_count,
                "boot startup row consumer uses view model without direct Track02 text");
    media_prompt_row_found = 0;
    for (i = 0; i < media_view_model.row_count; ++i) {
        if (strstr(media_rows[i], "RESURRECT THERON") != NULL) {
            media_prompt_row_found = 1;
        }
    }
    expect_true(media_prompt_row_found,
                "boot startup row consumer preserves Track02 prompt receipt text");
    expect_true(theron_v1_boot_startup_render_plan_from_view_model(
                    &media_view_model,
                    &media_plan) &&
                    media_plan.text_count ==
                        media_view_model.render_plan.text_count,
                "boot startup render-plan consumer uses view model receipt");
    expect_true(theron_v1_boot_startup_host_view_receipt_from_view_model(
                    &media_view_model,
                    &host_view_receipt) &&
                    host_view_receipt.host_consumes_view_model &&
                    host_view_receipt.view_model_valid &&
                    host_view_receipt.layout_count ==
                        media_view_model.layout_count &&
                    host_view_receipt.row_count ==
                        media_view_model.row_count &&
                    host_view_receipt.render_plan_valid &&
                    host_view_receipt.presentation_ready &&
                    host_view_receipt.render_route_valid &&
                    host_view_receipt.state_receipt_valid &&
                    host_view_receipt.track02_media_consumed &&
                    !host_view_receipt.raw_prompt_roster_required &&
                    !host_view_receipt.raw_session_rebuild_required &&
                    strcmp(host_view_receipt.view_model
                               .startup_media_state_receipt
                               .startup_roster_names[4],
                           "HAKAR-MEDIA") == 0 &&
                    strstr(host_view_receipt.view_model
                               .startup_media_state_receipt
                               .startup_text_prompt,
                           "RESURRECT THERON") != NULL,
                "boot startup host-view receipt consumes Track02 media view model without raw rebuild");
    expect_true(theron_v1_boot_startup_full_start_receipt_from_runtime_state_with_media_receipt(
                    &full_start_receipt,
                    &media_receipt,
                    NULL,
                    THERON_STARTUP_PHASE_READY,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    THERON_STARTUP_HERO_MIRROR_COUNT,
                    0,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS) &&
                    full_start_receipt.host_consumes_view_model &&
                    full_start_receipt.host_view_valid &&
                    full_start_receipt.host_view.track02_media_consumed &&
                    !full_start_receipt.raw_prompt_roster_required &&
                    !full_start_receipt.raw_session_rebuild_required &&
                    strcmp(full_start_receipt.view_model
                               .startup_media_state_receipt
                               .startup_roster_names[4],
                           "HAKAR-MEDIA") == 0 &&
                    strstr(full_start_receipt.view_model
                               .startup_media_state_receipt
                               .startup_text_prompt,
                           "RESURRECT THERON") != NULL,
                "boot runtime-state full-start receipt consumes Track02 media without raw prompt roster");
    expect_true(theron_v1_boot_startup_execute_input_from_full_start_receipt(
                    &full_start_receipt,
                    THERON_STARTUP_INPUT_BACK,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.state_receipt.flow.phase ==
                        THERON_STARTUP_PHASE_STAGE_SELECT,
                "boot runtime-state full-start receipt routes Soul Room Back without raw session rebuild");
    expect_true(theron_v1_boot_startup_execute_input_from_runtime_state_with_media_receipt(
                    &view_model_host_receipt,
                    &media_receipt,
                    THERON_STARTUP_PHASE_READY,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    THERON_STARTUP_HERO_MIRROR_COUNT,
                    0,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS,
                    10) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.state_receipt.flow.phase ==
                        THERON_STARTUP_PHASE_STAGE_SELECT,
                "boot runtime-state input wrapper consumes Track02 media receipt without raw session rebuild");
    expect_true(theron_v1_boot_startup_execute_pointer_from_runtime_state_with_media_receipt(
                    &view_model_host_receipt,
                    &media_receipt,
                    THERON_STARTUP_PHASE_READY,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    THERON_STARTUP_HERO_MIRROR_COUNT,
                    0,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS,
                    50,
                    80) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot runtime-state pointer wrapper consumes Track02 media receipt without raw layout rebuild");
    theron_v1_boot_startup_host_render_receipt_init(&host_render_receipt);
    expect_true(theron_v1_boot_startup_host_render_receipt_from_runtime_state_with_media_receipt(
                    &host_render_receipt,
                    &media_receipt,
                    THERON_STARTUP_PHASE_READY,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    THERON_STARTUP_HERO_MIRROR_COUNT,
                    0,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS) &&
                    host_render_receipt.layout_count ==
                        full_start_receipt.view_model.layout_count &&
                    host_render_receipt.row_count ==
                        full_start_receipt.view_model.row_count &&
                    host_render_receipt.render_plan_valid &&
                    host_render_receipt.render_plan.text_count ==
                        full_start_receipt.view_model.render_plan.text_count &&
                    host_render_receipt.render_plan.graphic_count ==
                        full_start_receipt.view_model.render_plan.graphic_count &&
                    strcmp(host_render_receipt.layout[2].label,
                           "HAKAR-MEDIA") == 0 &&
                    host_render_receipt.track02_real_media_ready &&
                    host_render_receipt.real_bitmap_startup_graphics_ready &&
                    !host_render_receipt.raw_prompt_roster_required &&
                    !host_render_receipt.raw_session_rebuild_required &&
                    !host_render_receipt.raw_graphics_plan_consumer_required,
                "boot runtime-state host-render receipt replaces layout row and render-plan media exports");
    media_prompt_row_found = 0;
    for (i = 0; i < host_render_receipt.row_count; ++i) {
        if (strstr(host_render_receipt.rows[i], "RESURRECT THERON") != NULL) {
            media_prompt_row_found = 1;
        }
    }
    expect_true(media_prompt_row_found,
                "boot runtime-state host-render receipt preserves Track02 prompt");
    expect_true(theron_v1_boot_startup_menu_runtime_handoff_from_runtime_state_with_media_receipt(
                    &handoff_receipt,
                    &media_receipt,
                    NULL,
                    THERON_STARTUP_PHASE_READY,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    THERON_STARTUP_HERO_MIRROR_COUNT,
                    0,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS,
                    10,
                    50,
                    80) &&
                    handoff_receipt.host_consumes_full_start_receipt &&
                    handoff_receipt.host_render_valid &&
                    handoff_receipt.track02_media_consumed &&
                    handoff_receipt.track02_media_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    handoff_receipt.track02_media_checksum ==
                        media_receipt.startup_bitmap_atlas_checksum &&
                    handoff_receipt.track02_media_title_first_raw_offset ==
                        media_receipt.startup_bitmap_title_first_raw_offset &&
                    handoff_receipt.track02_media_soul_room_last_user_data_offset ==
                        media_receipt.startup_bitmap_soul_room_last_user_data_offset &&
                    handoff_receipt.startup_menu_render_allowed &&
                    handoff_receipt.soul_room_menu_ready &&
                    handoff_receipt.real_graphics_handoff_ready &&
                    handoff_receipt.input_route_requested &&
                    handoff_receipt.input_route_valid &&
                    handoff_receipt.input_route.state_receipt_valid &&
                    handoff_receipt.pointer_route_requested &&
                    handoff_receipt.pointer_route_valid &&
                    handoff_receipt.pointer_route.state_receipt_valid &&
                    !handoff_receipt.raw_prompt_roster_required &&
                    !handoff_receipt.raw_session_rebuild_required &&
                    !handoff_receipt.raw_graphics_plan_consumer_required &&
                    !handoff_receipt.fallback_startup_graphics_executed &&
                    !handoff_receipt.host_may_draw_fallback_visuals,
                "boot menu/runtime handoff package consumes Track02 media render input and pointer receipts");
    theron_v1_boot_startup_host_view_receipt_init(&host_view_receipt);
    expect_true(theron_v1_boot_startup_host_view_from_runtime_state_with_media_receipt(
                    &host_view_receipt,
                    &media_receipt,
                    THERON_STARTUP_PHASE_READY,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    THERON_STARTUP_HERO_MIRROR_COUNT,
                    0,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS) &&
                    host_view_receipt.host_consumes_view_model &&
                    host_view_receipt.track02_media_consumed &&
                    !host_view_receipt.raw_prompt_roster_required &&
                    !host_view_receipt.raw_session_rebuild_required,
                "boot runtime-state host-view wrapper consumes full-start media receipt without raw rebuild");
    expect_true(theron_v1_boot_startup_host_view_receipt_from_snapshot_with_media_receipt(
                    &media_snapshot,
                    &media_receipt,
                    &host_view_receipt) &&
                    host_view_receipt.host_consumes_view_model &&
                    host_view_receipt.render_route_valid &&
                    host_view_receipt.state_receipt_valid &&
                    host_view_receipt.track02_media_consumed &&
                    !host_view_receipt.raw_prompt_roster_required &&
                    !host_view_receipt.raw_session_rebuild_required &&
                    host_view_receipt.render_route.track02_title_menu_ready &&
                    host_view_receipt.render_route.save_resume_start_ready &&
                    host_view_receipt.runtime_readiness_ready &&
                    host_view_receipt.runtime_level_render_allowed &&
                    host_view_receipt.title_menu_runtime_handoff_ready &&
                    host_view_receipt.save_resume_runtime_handoff_ready &&
                    host_view_receipt.no_fallback_visuals_enforced &&
                    !host_view_receipt.fallback_visuals_allowed &&
                    strcmp(host_view_receipt.status,
                           "THERON RUNTIME READY") == 0,
                "boot snapshot host-view receipt replaces raw prompt roster session rebuild");
    expect_true(theron_v1_boot_startup_render_route_receipt_from_view_model(
                    &media_view_model,
                    &render_route_receipt) &&
                    render_route_receipt.startup_menu_render_allowed &&
                    render_route_receipt.track02_title_menu_ready &&
                    render_route_receipt.render_plan_valid &&
                    render_route_receipt.render_plan.text_count ==
                        media_view_model.render_plan.text_count &&
                    render_route_receipt.state_receipt_valid &&
                    render_route_receipt.runtime_level_render_allowed &&
                    render_route_receipt.save_resume_start_ready &&
                    render_route_receipt.save_resume_runtime_handoff_ready &&
                    render_route_receipt
                        .save_resume_track02_no_fallback_ready &&
                    render_route_receipt.save_resume_claim ==
                        THERON_V1_STARTUP_RESUME_DUAL &&
                    render_route_receipt.save_resume_tqsv_slot == 2 &&
                    render_route_receipt.save_resume_srm_slot == 3 &&
                    render_route_receipt.save_resume_srm_import_status ==
                        THERON_V1_SRM_PROGRESS_IMPORT_OK &&
                    render_route_receipt.no_fallback_visuals_enforced &&
                    !render_route_receipt.fallback_visuals_allowed &&
                    strcmp(render_route_receipt.status,
                           "THERON RUNTIME READY") == 0,
                "boot startup render route receipt carries startup menu save-resume and runtime-ready facts");
    memset(&state_receipt, 0, sizeof(state_receipt));
    expect_true(theron_v1_boot_startup_state_receipt_from_view_model(
                    &media_view_model,
                    &state_receipt) &&
                    state_receipt.flow_changed &&
                    state_receipt.flow.phase ==
                        THERON_STARTUP_PHASE_READY &&
                    state_receipt.flow.selected_dungeon ==
                        THERON_DUNGEON_2_CRYPT_OF_SHADOWS &&
                    state_receipt.set_level_loaded &&
                    state_receipt.level_loaded == 0 &&
                    state_receipt.set_runtime_level_route &&
                    state_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM &&
                    state_receipt.set_save_resume &&
                    state_receipt.save_resume_claim ==
                        THERON_V1_STARTUP_RESUME_DUAL &&
                    state_receipt.save_resume_active_slot == 2 &&
                    state_receipt.save_resume_srm_active_slot == 3 &&
                    state_receipt.save_resume_srm_current_dungeon ==
                        THERON_DUNGEON_2_CRYPT_OF_SHADOWS &&
                    strcmp(state_receipt.save_resume_srm_root,
                           "/tmp/firestaff-theron-srm") == 0,
                "boot startup view model emits unified startup route/save state receipt");
    memset(&state_receipt, 0, sizeof(state_receipt));
    expect_true(theron_v1_boot_startup_state_receipt_from_snapshot_with_media_receipt(
                    &media_snapshot,
                    &media_receipt,
                    &state_receipt) &&
                    state_receipt.flow_changed &&
                    state_receipt.flow.phase ==
                        THERON_STARTUP_PHASE_READY &&
                    state_receipt.set_save_resume &&
                    state_receipt.save_resume_srm_active_slot == 3 &&
                    state_receipt.set_runtime_level_route,
                "boot startup snapshot media route emits same startup state receipt");
    blocked_snapshot = media_snapshot;
    blocked_snapshot.world = NULL;
    blocked_snapshot.runtime_level_source =
        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED;
    blocked_snapshot.runtime_track02_semantic_handoff = 0;
    blocked_snapshot.runtime_fallback_visuals_blocked = 1;
    blocked_snapshot.runtime_structured_route = 1;
    blocked_snapshot.runtime_receipt_text_route = 0;
    expect_true(theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
                    &blocked_snapshot,
                    &media_receipt,
                    &blocked_view_model) &&
                    blocked_view_model.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED &&
                    blocked_view_model.runtime_fallback_visuals_blocked == 1 &&
                    blocked_view_model.runtime_level == -1,
                "boot startup view model carries Track02 blocked route without fallback world");
    memset(&state_receipt, 0, sizeof(state_receipt));
    expect_true(theron_v1_boot_startup_state_receipt_from_view_model(
                    &blocked_view_model,
                    &state_receipt) &&
                    state_receipt.set_runtime_level_route &&
                    state_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED &&
                    state_receipt.runtime_fallback_visuals_blocked == 1 &&
                    state_receipt.runtime_structured_route == 1 &&
                    state_receipt.runtime_receipt_text_route == 0 &&
                    !state_receipt.set_level_loaded,
                "boot startup state receipt blocks Track02 fallback visuals without marking a level loaded");
    expect_true(theron_v1_boot_startup_render_route_receipt_from_snapshot_with_media_receipt(
                    &blocked_snapshot,
                    &media_receipt,
                    &render_route_receipt) &&
                    render_route_receipt.startup_menu_render_allowed &&
                    render_route_receipt.render_plan_valid &&
                    render_route_receipt.state_receipt_valid &&
                    render_route_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED &&
                    render_route_receipt.runtime_fallback_visuals_blocked == 1 &&
                    render_route_receipt.runtime_structured_route == 1 &&
                    render_route_receipt.runtime_receipt_text_route == 0 &&
                    !render_route_receipt.runtime_level_render_allowed &&
                    !render_route_receipt.runtime_readiness_ready &&
                    !render_route_receipt.title_menu_runtime_handoff_ready &&
                    render_route_receipt.save_resume_start_ready &&
                    !render_route_receipt.save_resume_runtime_handoff_ready &&
                    !render_route_receipt
                         .save_resume_track02_no_fallback_ready &&
                    render_route_receipt.save_resume_claim ==
                        THERON_V1_STARTUP_RESUME_DUAL &&
                    render_route_receipt.save_resume_tqsv_slot == 2 &&
                    render_route_receipt.save_resume_srm_slot == 3 &&
                    render_route_receipt.track02_title_menu_ready &&
                    render_route_receipt.no_fallback_visuals_enforced &&
                    !render_route_receipt.fallback_visuals_allowed &&
                    !render_route_receipt.state_receipt.set_level_loaded &&
                    strcmp(render_route_receipt.status,
                           "TRACK02 RUNTIME BLOCKED") == 0,
                "boot startup render route receipt blocks Track02 level fallback visuals");
    expect_true(theron_v1_boot_startup_host_view_receipt_from_snapshot_with_media_receipt(
                    &blocked_snapshot,
                    &media_receipt,
                    &host_view_receipt) &&
                    host_view_receipt.host_consumes_view_model &&
                    host_view_receipt.render_route_valid &&
                    host_view_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED &&
                    host_view_receipt.runtime_fallback_visuals_blocked == 1 &&
                    host_view_receipt.runtime_structured_route == 1 &&
                    host_view_receipt.runtime_receipt_text_route == 0 &&
                    !host_view_receipt.runtime_level_render_allowed &&
                    !host_view_receipt.runtime_readiness_ready &&
                    !host_view_receipt.title_menu_runtime_handoff_ready &&
                    !host_view_receipt.save_resume_runtime_handoff_ready &&
                    host_view_receipt.no_fallback_visuals_enforced &&
                    !host_view_receipt.fallback_visuals_allowed &&
                    !host_view_receipt.runtime_graphics_handoff &&
                    !host_view_receipt.track02_runtime_graphics_handoff &&
                    !host_view_receipt.save_resume_runtime_graphics_handoff &&
                    !host_view_receipt.mac_app_capture_candidate_ready &&
                    !host_view_receipt
                         .mac_app_capture_requires_external_screenshot &&
                    host_view_receipt.mac_app_capture_evidence_hash == 0u &&
                    strcmp(host_view_receipt.status,
                           "TRACK02 RUNTIME BLOCKED") == 0,
                "boot host-view receipt exposes Track02 blocked route without status fallback parsing");
    expect_true(theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
                    &blocked_snapshot,
                    &media_receipt,
                    &media_graphics_executor,
                    &full_start_receipt) &&
                    theron_v1_boot_startup_menu_runtime_handoff_from_full_start_receipt(
                        &full_start_receipt,
                        -1,
                        -1,
                        -1,
                        &handoff_receipt) &&
                    handoff_receipt.host_render_valid &&
                    handoff_receipt.track02_media_consumed &&
                    handoff_receipt.startup_menu_render_allowed &&
                    handoff_receipt.startup_graphics_blocked &&
                    handoff_receipt.no_fallback_visuals_enforced &&
                    !handoff_receipt.fallback_visuals_allowed &&
                    handoff_receipt.host_must_not_draw_fallback_visuals &&
                    !handoff_receipt.host_may_draw_fallback_visuals &&
                    !handoff_receipt.fallback_startup_graphics_executed &&
                    !handoff_receipt.runtime_handoff_ready,
                "boot menu/runtime handoff package blocks Track02 fallback visuals before host draw");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    expect_true(!theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
                    &blocked_view_model,
                    &media_graphics_executor,
                    &graphics_route_receipt) &&
                    graphics_route_receipt.host_consumes_view_model &&
                    graphics_route_receipt.render_route_valid &&
                    !graphics_route_receipt.graphics_plan_valid &&
                    !graphics_route_receipt.graphics_executed &&
                    graphics_route_receipt.graphics_blocked &&
                    graphics_route_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED &&
                    graphics_route_receipt.runtime_fallback_visuals_blocked &&
                    graphics_route_receipt.runtime_structured_route &&
                    !graphics_route_receipt.runtime_receipt_text_route &&
                    graphics_route_receipt.track02_real_media_ready &&
                    graphics_route_receipt.real_bitmap_startup_graphics_ready &&
                    graphics_route_receipt.required_bitmap_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    graphics_route_receipt.required_bitmap_route_count == 4 &&
                    graphics_route_receipt.required_bitmap_routes_ready &&
                    graphics_route_receipt.bitmap_package_route_ready &&
                    graphics_route_receipt.raw_bitmap_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    graphics_route_receipt.raw_bitmap_route_count == 4 &&
                    graphics_route_receipt.raw_bitmap_atlas_tile_count == 48u &&
                    graphics_route_receipt.iso_bitmap_route_mask == 0u &&
                    (graphics_route_receipt.bitmap_route_mask & 0x04u) &&
                    (graphics_route_receipt.bitmap_route_mask & 0x08u) &&
                    graphics_route_receipt.bitmap_route_count >= 4 &&
                    graphics_route_receipt.soul_room_bitmap_route_ready &&
                    graphics_route_receipt.forcefield_bitmap_route_ready &&
                    !graphics_route_receipt.raw_graphics_plan_consumer_required &&
                    graphics_route_receipt.no_fallback_startup_graphics_proof &&
                    !graphics_route_receipt.fallback_startup_graphics_executed &&
                    graphics_route_receipt.no_fallback_visuals_enforced &&
                    !graphics_route_receipt.fallback_visuals_allowed &&
                    !graphics_route_receipt.runtime_graphics_handoff &&
                    !graphics_route_receipt.track02_runtime_graphics_handoff &&
                    !graphics_route_receipt
                         .save_resume_runtime_graphics_handoff &&
                    media_graphics_counters.fill_count == 0 &&
                    media_graphics_counters.rect_count == 0 &&
                    strcmp(graphics_route_receipt.status,
                           "TRACK02 GRAPHICS BLOCKED") == 0,
                "boot graphics route receipt blocks Track02 fallback visuals before execution");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    expect_true(!theron_v1_boot_startup_execute_graphics_plan_from_snapshot_with_media_receipt(
                    &blocked_snapshot,
                    &media_receipt,
                    &media_graphics_executor,
                    &graphics_route_receipt) &&
                    graphics_route_receipt.graphics_blocked &&
                    graphics_route_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED &&
                    media_graphics_counters.fill_count == 0,
                "boot snapshot graphics route blocks Track02 fallback visuals without UI rebuild");
    semantic_snapshot = media_snapshot;
    semantic_snapshot.runtime_level_source =
        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC;
    semantic_snapshot.runtime_track02_semantic_handoff = 1;
    semantic_snapshot.runtime_fallback_visuals_blocked = 0;
    semantic_snapshot.runtime_structured_route = 1;
    semantic_snapshot.runtime_receipt_text_route = 0;
    semantic_snapshot.all_dungeon_real_data_capture_ready = 1;
    semantic_snapshot.all_dungeon_capture_count = THERON_DUNGEON_COUNT;
    semantic_snapshot.all_dungeon_capture_mask =
        (1u << THERON_DUNGEON_COUNT) - 1u;
    semantic_snapshot.exact_level_semantics_ready = 1;
    semantic_snapshot.exact_object_semantics_ready = 1;
    semantic_snapshot.no_fallback_semantic_role_mask =
        (1u << THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE) |
        (1u << THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE);
    semantic_snapshot.track02_state_predicates_consumed = 1;
    semantic_snapshot.track02_bitmap_routes_complete = 1;
    semantic_snapshot.track02_no_fallback_runtime_route_ready = 1;
    semantic_snapshot.object_table_no_fallback_ready = 1;
    semantic_snapshot.object_table_blocked_anchor_mask = 0x07u;
    semantic_snapshot.object_table_blocked_anchor_count = 3;
    semantic_snapshot.nonstartup_level_no_fallback_ready = 1;
    semantic_snapshot.nonstartup_level_blocked_anchor_mask = 0x07u;
    semantic_snapshot.nonstartup_level_blocked_anchor_count = 3;
    semantic_snapshot.startup_level_blocked_anchor_mask = 0x06u;
    semantic_snapshot.startup_level_blocked_anchor_count = 2;
    semantic_snapshot.object_table_route_hash = 0x12345678u;
    semantic_snapshot.level_route_hash = 0x23456789u;
    expect_true(theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
                    &semantic_snapshot,
                    &media_receipt,
                    &semantic_view_model) &&
                    semantic_view_model.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC &&
                    semantic_view_model.runtime_track02_semantic_handoff == 1 &&
                    semantic_view_model.object_table_no_fallback_ready &&
                    semantic_view_model.object_table_blocked_anchor_mask == 0x07u &&
                    semantic_view_model.nonstartup_level_no_fallback_ready &&
                    semantic_view_model.nonstartup_level_blocked_anchor_mask == 0x07u &&
                    semantic_view_model.mac_app_capture_candidate_ready &&
                    semantic_view_model
                        .mac_app_capture_requires_external_screenshot &&
                    semantic_view_model.mac_app_capture_evidence_hash != 0u &&
                    semantic_view_model.runtime_level == 0,
                "boot startup view model carries Track02 semantic first-level route");
    expect_true(theron_v1_boot_startup_render_route_receipt_from_view_model(
                    &semantic_view_model,
                    &render_route_receipt) &&
                    render_route_receipt.startup_menu_render_allowed &&
                    render_route_receipt.track02_title_menu_ready &&
                    render_route_receipt.runtime_level_render_allowed &&
                    render_route_receipt.first_level_render_ready &&
                    render_route_receipt.hud_ready &&
                    render_route_receipt.hud_seed_gate ==
                        THERON_V2_HUD_SEED_V2_READY &&
                    render_route_receipt.runtime_readiness_ready &&
                    render_route_receipt.title_menu_runtime_handoff_ready &&
                    render_route_receipt.save_resume_start_ready &&
                    render_route_receipt.save_resume_runtime_handoff_ready &&
                    render_route_receipt
                        .save_resume_track02_no_fallback_ready &&
                    render_route_receipt.save_resume_claim ==
                        THERON_V1_STARTUP_RESUME_DUAL &&
                    render_route_receipt.save_resume_tqsv_slot == 2 &&
                    render_route_receipt.save_resume_srm_slot == 3 &&
                    render_route_receipt.save_resume_srm_import_status ==
                        THERON_V1_SRM_PROGRESS_IMPORT_OK &&
                    render_route_receipt.no_fallback_visuals_enforced &&
                    !render_route_receipt.fallback_visuals_allowed &&
                    render_route_receipt.runtime_track02_semantic_handoff == 1 &&
                    render_route_receipt.runtime_structured_route == 1 &&
                    render_route_receipt.runtime_receipt_text_route == 0 &&
                    render_route_receipt.all_dungeon_real_data_capture_ready &&
                    render_route_receipt.all_dungeon_capture_count ==
                        THERON_DUNGEON_COUNT &&
                    render_route_receipt.all_dungeon_capture_mask ==
                        ((1u << THERON_DUNGEON_COUNT) - 1u) &&
                    render_route_receipt.exact_level_semantics_ready &&
                    render_route_receipt.exact_object_semantics_ready &&
                    render_route_receipt.track02_state_predicates_consumed &&
                    render_route_receipt.track02_bitmap_routes_complete &&
                    render_route_receipt
                        .track02_no_fallback_runtime_route_ready &&
                    render_route_receipt.object_table_no_fallback_ready &&
                    render_route_receipt.object_table_blocked_anchor_mask == 0x07u &&
                    render_route_receipt.nonstartup_level_no_fallback_ready &&
                    render_route_receipt.nonstartup_level_blocked_anchor_mask == 0x07u &&
                    render_route_receipt.startup_level_blocked_anchor_mask == 0x06u &&
                    render_route_receipt.object_table_route_hash ==
                        semantic_snapshot.object_table_route_hash &&
                    render_route_receipt.level_route_hash ==
                        semantic_snapshot.level_route_hash &&
                    render_route_receipt.mac_app_capture_candidate_ready &&
                    render_route_receipt
                        .mac_app_capture_requires_external_screenshot &&
                    render_route_receipt.mac_app_capture_evidence_hash ==
                        semantic_view_model.mac_app_capture_evidence_hash &&
                    strcmp(render_route_receipt.status,
                           "TRACK02 RUNTIME READY") == 0,
                "boot startup render route receipt marks Track02 semantic first level HUD-ready without fallback visuals");
    expect_true(theron_v1_boot_startup_host_view_receipt_from_snapshot_with_media_receipt(
                    &semantic_snapshot,
                    &media_receipt,
                    &host_view_receipt) &&
                    host_view_receipt.host_consumes_view_model &&
                    host_view_receipt.render_route_valid &&
                    host_view_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC &&
                    host_view_receipt.runtime_track02_semantic_handoff == 1 &&
                    host_view_receipt.runtime_structured_route == 1 &&
                    host_view_receipt.runtime_receipt_text_route == 0 &&
                    host_view_receipt.all_dungeon_real_data_capture_ready &&
                    host_view_receipt.all_dungeon_capture_count ==
                        THERON_DUNGEON_COUNT &&
                    host_view_receipt.exact_level_semantics_ready &&
                    host_view_receipt.exact_object_semantics_ready &&
                    host_view_receipt.track02_state_predicates_consumed &&
                    host_view_receipt.track02_bitmap_routes_complete &&
                    host_view_receipt
                        .track02_no_fallback_runtime_route_ready &&
                    host_view_receipt.object_table_no_fallback_ready &&
                    host_view_receipt.object_table_blocked_anchor_mask == 0x07u &&
                    host_view_receipt.nonstartup_level_no_fallback_ready &&
                    host_view_receipt.nonstartup_level_blocked_anchor_mask == 0x07u &&
                    host_view_receipt.mac_app_capture_candidate_ready &&
                    host_view_receipt
                        .mac_app_capture_requires_external_screenshot &&
                    host_view_receipt.mac_app_capture_evidence_hash ==
                        semantic_view_model.mac_app_capture_evidence_hash &&
                    host_view_receipt.runtime_level_render_allowed &&
                    host_view_receipt.runtime_readiness_ready &&
                    host_view_receipt.title_menu_runtime_handoff_ready &&
                    host_view_receipt.save_resume_runtime_handoff_ready &&
                    host_view_receipt.save_resume_track02_no_fallback_ready &&
                    host_view_receipt.no_fallback_visuals_enforced &&
                    !host_view_receipt.fallback_visuals_allowed &&
                    host_view_receipt.runtime_graphics_handoff &&
                    host_view_receipt.track02_runtime_graphics_handoff &&
                    !host_view_receipt.save_resume_runtime_graphics_handoff &&
                    host_view_receipt.hud_ready &&
                    strcmp(host_view_receipt.status,
                           "TRACK02 RUNTIME READY") == 0,
                "boot host-view receipt exposes Track02 semantic runtime handoff without fallback adapters");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    expect_true(theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
                    &semantic_view_model,
                    &media_graphics_executor,
                    &graphics_route_receipt) == 0 &&
                    !graphics_route_receipt.graphics_executed &&
                    graphics_route_receipt.graphics_blocked &&
                    graphics_route_receipt.runtime_readiness_ready &&
                    graphics_route_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC &&
                    graphics_route_receipt.runtime_track02_semantic_handoff &&
                    graphics_route_receipt.no_fallback_visuals_enforced &&
                    !graphics_route_receipt.fallback_visuals_allowed &&
                    graphics_route_receipt.runtime_graphics_handoff &&
                    graphics_route_receipt.track02_runtime_graphics_handoff &&
                    !graphics_route_receipt
                         .save_resume_runtime_graphics_handoff &&
                    media_graphics_counters.fill_count == 0 &&
                    strcmp(graphics_route_receipt.status,
                           "TRACK02 RUNTIME GRAPHICS HANDOFF") == 0,
                "boot graphics route receipt hands Track02 semantic route to runtime without fallback startup draw");
    expect_true(!theron_v1_boot_startup_execute_graphics_plan_from_snapshot_with_media_receipt(
                    &semantic_snapshot,
                    &media_receipt,
                    &media_graphics_executor,
                    &graphics_route_receipt) &&
                    graphics_route_receipt.runtime_readiness_ready &&
                    graphics_route_receipt.runtime_track02_semantic_handoff &&
                    graphics_route_receipt.runtime_graphics_handoff &&
                    graphics_route_receipt.track02_runtime_graphics_handoff &&
                    !graphics_route_receipt
                         .save_resume_runtime_graphics_handoff &&
                    graphics_route_receipt.graphics_blocked &&
                    strcmp(graphics_route_receipt.status,
                           "TRACK02 RUNTIME GRAPHICS HANDOFF") == 0,
                "boot snapshot graphics route hands semantic Track02 route to runtime");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    expect_true(theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
                    &semantic_snapshot,
                    &media_receipt,
                    &media_graphics_executor,
                    &full_start_receipt) &&
                    full_start_receipt.host_view_valid &&
                    full_start_receipt.graphics_route_valid &&
                    full_start_receipt.title_menu_ready &&
                    full_start_receipt.stage_menu_ready &&
                    full_start_receipt.soul_room_menu_ready &&
                    full_start_receipt.forcefield_menu_ready &&
                    full_start_receipt.forcefield_runtime_handoff_ready &&
                    full_start_receipt.full_start_graphics_ready &&
                    !full_start_receipt.full_start_graphics_executed &&
                    full_start_receipt.full_start_graphics_blocked &&
                    full_start_receipt.track02_real_media_ready &&
                    full_start_receipt.real_bitmap_startup_graphics_ready &&
                    (full_start_receipt.bitmap_route_mask & 0x04u) &&
                    (full_start_receipt.bitmap_route_mask & 0x08u) &&
                    full_start_receipt.bitmap_route_count >= 4 &&
                    full_start_receipt.soul_room_bitmap_route_ready &&
                    full_start_receipt.forcefield_bitmap_route_ready &&
                    !full_start_receipt.raw_graphics_plan_consumer_required &&
                    full_start_receipt.no_fallback_startup_graphics_proof &&
                    !full_start_receipt.fallback_startup_graphics_executed &&
                    full_start_receipt.no_fallback_visuals_enforced &&
                    !full_start_receipt.fallback_visuals_allowed &&
                    full_start_receipt.runtime_graphics_handoff &&
                    full_start_receipt.track02_runtime_graphics_handoff &&
                    full_start_receipt.all_dungeon_real_data_capture_ready &&
                    full_start_receipt.all_dungeon_capture_count ==
                        THERON_DUNGEON_COUNT &&
                    full_start_receipt.exact_level_semantics_ready &&
                    full_start_receipt.exact_object_semantics_ready &&
                    full_start_receipt.track02_state_predicates_consumed &&
                    full_start_receipt.track02_bitmap_routes_complete &&
                    full_start_receipt
                        .track02_no_fallback_runtime_route_ready &&
                    full_start_receipt.object_table_no_fallback_ready &&
                    full_start_receipt.object_table_blocked_anchor_mask == 0x07u &&
                    full_start_receipt.nonstartup_level_no_fallback_ready &&
                    full_start_receipt.nonstartup_level_blocked_anchor_mask == 0x07u &&
                    full_start_receipt.object_table_route_hash ==
                        semantic_snapshot.object_table_route_hash &&
                    full_start_receipt.level_route_hash ==
                        semantic_snapshot.level_route_hash &&
                    !full_start_receipt.save_resume_runtime_graphics_handoff &&
                    !full_start_receipt.raw_prompt_roster_required &&
                    !full_start_receipt.raw_session_rebuild_required &&
                    media_graphics_counters.fill_count == 0 &&
                    strcmp(full_start_receipt.status,
                           "FORCEFIELD RUNTIME HANDOFF") == 0,
                "boot full-start receipt hands Track02 forcefield route to runtime without fallback graphics");
    expect_true(theron_v1_boot_startup_menu_runtime_handoff_from_full_start_receipt(
                    &full_start_receipt,
                    -1,
                    -1,
                    -1,
                    &handoff_receipt) &&
                    handoff_receipt.host_render_valid &&
                    handoff_receipt.track02_media_consumed &&
                    handoff_receipt.track02_media_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    handoff_receipt.track02_media_checksum ==
                        media_receipt.startup_bitmap_atlas_checksum &&
                    handoff_receipt.track02_media_stage_first_raw_offset ==
                        media_receipt.startup_bitmap_stage_first_raw_offset &&
                    handoff_receipt.track02_media_forcefield_last_user_data_offset ==
                        media_receipt.startup_bitmap_forcefield_last_user_data_offset &&
                    handoff_receipt.runtime_handoff_ready &&
                    handoff_receipt.track02_runtime_handoff_ready &&
                    !handoff_receipt.save_resume_runtime_handoff_ready &&
                    handoff_receipt.real_graphics_handoff_ready &&
                    handoff_receipt.startup_graphics_blocked &&
                    handoff_receipt.no_fallback_visuals_enforced &&
                    !handoff_receipt.fallback_visuals_allowed &&
                    handoff_receipt.host_must_not_draw_fallback_visuals &&
                    !handoff_receipt.host_may_draw_fallback_visuals &&
                    !handoff_receipt.fallback_startup_graphics_executed &&
                    strcmp(handoff_receipt.status,
                           "THERON RUNTIME HANDOFF NO FALLBACK") == 0,
                "boot menu/runtime handoff package exposes Track02 runtime no-fallback handoff");
    expect_true(theron_v1_boot_startup_ui_caller_from_full_start_receipt(
                    &full_start_receipt,
                    -1,
                    -1,
                    -1,
                    &ui_caller_receipt) &&
                    ui_caller_receipt.ui_callers_ready &&
                    ui_caller_receipt.host_render_valid &&
                    ui_caller_receipt.menu_runtime_handoff_valid &&
                    ui_caller_receipt.track02_media_consumed &&
                    ui_caller_receipt.track02_media_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    ui_caller_receipt.track02_media_checksum ==
                        media_receipt.startup_bitmap_atlas_checksum &&
                    ui_caller_receipt.track02_media_title_last_raw_offset ==
                        media_receipt.startup_bitmap_title_last_raw_offset &&
                    ui_caller_receipt.track02_media_soul_room_first_user_data_offset ==
                        media_receipt.startup_bitmap_soul_room_first_user_data_offset &&
                    ui_caller_receipt.track02_media_forcefield_last_raw_offset ==
                        media_receipt.startup_bitmap_forcefield_last_raw_offset &&
                    ui_caller_receipt.title_prompt_ready &&
                    ui_caller_receipt.roster_ready &&
                    ui_caller_receipt.title_menu_ready &&
                    ui_caller_receipt.stage_menu_ready &&
                    ui_caller_receipt.soul_room_menu_ready &&
                    ui_caller_receipt.forcefield_menu_ready &&
                    ui_caller_receipt.runtime_handoff_ready &&
                    ui_caller_receipt.track02_runtime_handoff_ready &&
                    !ui_caller_receipt.save_resume_runtime_handoff_ready &&
                    ui_caller_receipt.real_graphics_handoff_ready &&
                    ui_caller_receipt.real_bitmap_decode_ready &&
                    ui_caller_receipt.bitmap_package_route_ready &&
                    ui_caller_receipt.raw_bitmap_route_mask ==
                        (int)TST_THERON_FULL_START_BITMAP_ROUTES &&
                    ui_caller_receipt.raw_bitmap_route_count == 4 &&
                    ui_caller_receipt.iso_bitmap_route_mask == 0 &&
                    (ui_caller_receipt.bitmap_route_mask & 0x04) &&
                    (ui_caller_receipt.bitmap_route_mask & 0x08) &&
                    ui_caller_receipt.bitmap_route_count >= 4 &&
                    ui_caller_receipt.soul_room_bitmap_route_ready &&
                    ui_caller_receipt.forcefield_bitmap_route_ready &&
                    ui_caller_receipt.runtime_level == 0 &&
                    ui_caller_receipt.runtime_track02_semantic_handoff &&
                    ui_caller_receipt.semantic_first_level_ready &&
                    !ui_caller_receipt.semantic_nonzero_level_ready &&
                    ui_caller_receipt.semantic_level_coverage_mask == 0x01 &&
                    ui_caller_receipt.all_dungeon_real_data_capture_ready &&
                    ui_caller_receipt.all_dungeon_capture_count ==
                        THERON_DUNGEON_COUNT &&
                    ui_caller_receipt.all_dungeon_capture_mask ==
                        ((1u << THERON_DUNGEON_COUNT) - 1u) &&
                    ui_caller_receipt.exact_level_semantics_ready &&
                    ui_caller_receipt.exact_object_semantics_ready &&
                    ui_caller_receipt.track02_state_predicates_consumed &&
                    ui_caller_receipt.track02_bitmap_routes_complete &&
                    ui_caller_receipt
                        .track02_no_fallback_runtime_route_ready &&
                    ui_caller_receipt.object_table_no_fallback_ready &&
                    ui_caller_receipt.object_table_blocked_anchor_mask == 0x07u &&
                    ui_caller_receipt.nonstartup_level_no_fallback_ready &&
                    ui_caller_receipt.nonstartup_level_blocked_anchor_mask == 0x07u &&
                    ui_caller_receipt.object_table_route_hash ==
                        semantic_snapshot.object_table_route_hash &&
                    ui_caller_receipt.level_route_hash ==
                        semantic_snapshot.level_route_hash &&
                    ui_caller_receipt.complete_runtime_support_ready &&
                    ui_caller_receipt.no_fallback_visuals_enforced &&
                    !ui_caller_receipt.fallback_visuals_allowed &&
                    !ui_caller_receipt.fallback_startup_graphics_executed &&
                    ui_caller_receipt.host_must_not_draw_fallback_visuals &&
                    !ui_caller_receipt.raw_prompt_roster_required &&
                    !ui_caller_receipt.raw_session_rebuild_required &&
                    !ui_caller_receipt.raw_graphics_plan_consumer_required &&
                    strcmp(ui_caller_receipt.status,
                           "THERON UI CALLERS TRACK02 READY") == 0,
                "boot UI caller receipt consumes Track02 title/menu/bitmap/runtime handoff without fallback visuals");
    world.current_level = 1;
    world.level_loaded[THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1][1] = 1;
    semantic_level_snapshot = media_snapshot;
    semantic_level_snapshot.runtime_level_source =
        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC;
    semantic_level_snapshot.runtime_track02_semantic_handoff = 1;
    semantic_level_snapshot.runtime_fallback_visuals_blocked = 0;
    semantic_level_snapshot.runtime_structured_route = 1;
    semantic_level_snapshot.runtime_receipt_text_route = 0;
    semantic_level_snapshot.all_dungeon_real_data_capture_ready = 1;
    semantic_level_snapshot.all_dungeon_capture_count = THERON_DUNGEON_COUNT;
    semantic_level_snapshot.all_dungeon_capture_mask =
        (1u << THERON_DUNGEON_COUNT) - 1u;
    semantic_level_snapshot.exact_level_semantics_ready = 1;
    semantic_level_snapshot.exact_object_semantics_ready = 1;
    semantic_level_snapshot.no_fallback_semantic_role_mask =
        semantic_snapshot.no_fallback_semantic_role_mask;
    semantic_level_snapshot.track02_state_predicates_consumed = 1;
    semantic_level_snapshot.track02_bitmap_routes_complete = 1;
    semantic_level_snapshot.track02_no_fallback_runtime_route_ready = 1;
    semantic_level_snapshot.object_table_no_fallback_ready = 1;
    semantic_level_snapshot.object_table_blocked_anchor_mask = 0x07u;
    semantic_level_snapshot.object_table_blocked_anchor_count = 3;
    semantic_level_snapshot.nonstartup_level_no_fallback_ready = 1;
    semantic_level_snapshot.nonstartup_level_blocked_anchor_mask = 0x07u;
    semantic_level_snapshot.nonstartup_level_blocked_anchor_count = 3;
    semantic_level_snapshot.startup_level_blocked_anchor_mask = 0x06u;
    semantic_level_snapshot.startup_level_blocked_anchor_count = 2;
    semantic_level_snapshot.object_table_route_hash = 0x12345678u;
    semantic_level_snapshot.level_route_hash = 0x23456789u;
    expect_true(theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
                    &semantic_level_snapshot,
                    &media_receipt,
                    &semantic_level_view_model) &&
                    semantic_level_view_model.runtime_level == 1 &&
                    semantic_level_view_model.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC &&
                    semantic_level_view_model.runtime_track02_semantic_handoff,
                "boot startup view model carries Track02 semantic nonzero level route");
    expect_true(theron_v1_boot_startup_render_route_receipt_from_view_model(
                    &semantic_level_view_model,
                    &render_route_receipt) &&
                    render_route_receipt.runtime_level_render_allowed &&
                    !render_route_receipt.first_level_render_ready &&
                    render_route_receipt.hud_ready &&
                    render_route_receipt.hud_seed_gate ==
                        THERON_V2_HUD_SEED_V2_READY &&
                    render_route_receipt.runtime_readiness_ready &&
                    render_route_receipt.title_menu_runtime_handoff_ready &&
                    render_route_receipt.no_fallback_visuals_enforced &&
                    !render_route_receipt.fallback_visuals_allowed &&
                    render_route_receipt.runtime_track02_semantic_handoff &&
                    render_route_receipt.runtime_structured_route &&
                    !render_route_receipt.runtime_receipt_text_route,
                "boot render route receipt covers Track02 semantic nonzero level without fallback visuals");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    expect_true(theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
                    &semantic_level_snapshot,
                    &media_receipt,
                    &media_graphics_executor,
                    &full_start_receipt) &&
                    full_start_receipt.host_view_valid &&
                    full_start_receipt.graphics_route_valid &&
                    full_start_receipt.runtime_readiness_ready &&
                    full_start_receipt.runtime_level_render_allowed &&
                    full_start_receipt.runtime_level == 1 &&
                    full_start_receipt.runtime_champion_count == 3 &&
                    full_start_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC &&
                    full_start_receipt.runtime_track02_semantic_handoff &&
                    full_start_receipt.runtime_structured_route &&
                    !full_start_receipt.runtime_receipt_text_route &&
                    full_start_receipt.hud_ready &&
                    full_start_receipt.full_start_graphics_ready &&
                    full_start_receipt.full_start_graphics_blocked &&
                    full_start_receipt.track02_real_media_ready &&
                    full_start_receipt.real_bitmap_startup_graphics_ready &&
                    full_start_receipt.no_fallback_startup_graphics_proof &&
                    !full_start_receipt.fallback_startup_graphics_executed &&
                    full_start_receipt.no_fallback_visuals_enforced &&
                    !full_start_receipt.fallback_visuals_allowed &&
                    full_start_receipt.runtime_graphics_handoff &&
                    full_start_receipt.track02_runtime_graphics_handoff &&
                    media_graphics_counters.fill_count == 0 &&
                    strcmp(full_start_receipt.status,
                           "FORCEFIELD RUNTIME HANDOFF") == 0,
                "boot full-start receipt exposes Track02 semantic nonzero level no-fallback proof");
    world.current_level = 2;
    world.level_loaded[THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1][2] = 1;
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    expect_true(theron_v1_boot_startup_full_start_receipt_from_runtime_route_with_media_receipt(
                    &full_start_receipt,
                    &media_receipt,
                    &media_graphics_executor,
                    THERON_STARTUP_PHASE_READY,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    THERON_STARTUP_HERO_MIRROR_COUNT,
                    0,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC,
                    1,
                    0,
                    1,
                    0,
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS) &&
                    full_start_receipt.host_consumes_view_model &&
                    full_start_receipt.view_model_valid &&
                    full_start_receipt.host_view_valid &&
                    full_start_receipt.graphics_route_valid &&
                    full_start_receipt.runtime_readiness_ready &&
                    full_start_receipt.runtime_level_render_allowed &&
                    full_start_receipt.runtime_level == 2 &&
                    full_start_receipt.runtime_champion_count == 3 &&
                    full_start_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC &&
                    full_start_receipt.runtime_track02_semantic_handoff &&
                    full_start_receipt.runtime_structured_route &&
                    !full_start_receipt.runtime_receipt_text_route &&
                    full_start_receipt.full_start_graphics_ready &&
                    full_start_receipt.full_start_graphics_blocked &&
                    full_start_receipt.track02_real_media_ready &&
                    full_start_receipt.real_bitmap_startup_graphics_ready &&
                    full_start_receipt.no_fallback_startup_graphics_proof &&
                    !full_start_receipt.fallback_startup_graphics_executed &&
                    full_start_receipt.no_fallback_visuals_enforced &&
                    !full_start_receipt.fallback_visuals_allowed &&
                    full_start_receipt.runtime_graphics_handoff &&
                    full_start_receipt.track02_runtime_graphics_handoff &&
                    !full_start_receipt.save_resume_runtime_graphics_handoff &&
                    !full_start_receipt.raw_prompt_roster_required &&
                    !full_start_receipt.raw_session_rebuild_required &&
                    media_graphics_counters.fill_count == 0 &&
                    strcmp(full_start_receipt.status,
                           "FORCEFIELD RUNTIME HANDOFF") == 0,
                "boot runtime-route full-start receipt proves Track02 semantic level 2 no-fallback handoff");
    expect_true(!theron_v1_boot_startup_ui_caller_from_runtime_route_with_media_receipt(
                    &ui_caller_receipt,
                    &media_receipt,
                    &media_graphics_executor,
                    THERON_STARTUP_PHASE_READY,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    NULL,
                    &world,
                    NULL,
                    THERON_STARTUP_HERO_MIRROR_COUNT,
                    0,
                    THERON_V1_STARTUP_RESUME_DUAL,
                    2,
                    3,
                    THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "/tmp/firestaff-theron-srm",
                    THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC,
                    1,
                    0,
                    1,
                    0,
                    0x03,
                    2,
                    order,
                    THERON_STARTUP_MAX_COMPANIONS,
                    -1,
                    -1,
                    -1) &&
                    !ui_caller_receipt.ui_callers_ready &&
                    ui_caller_receipt.runtime_level == 2 &&
                    ui_caller_receipt.runtime_track02_semantic_handoff &&
                    !ui_caller_receipt.semantic_first_level_ready &&
                    ui_caller_receipt.semantic_nonzero_level_ready &&
                    ui_caller_receipt.semantic_level_coverage_mask == 0x04 &&
                    ui_caller_receipt.real_bitmap_decode_ready &&
                    ui_caller_receipt.host_must_not_draw_fallback_visuals &&
                    !ui_caller_receipt.fallback_visuals_allowed &&
                    !ui_caller_receipt.fallback_startup_graphics_executed &&
                    !ui_caller_receipt.raw_prompt_roster_required &&
                    !ui_caller_receipt.raw_session_rebuild_required &&
                    !ui_caller_receipt.raw_graphics_plan_consumer_required,
                "boot UI caller wrapper carries Track02 semantic nonzero level without fallback visuals but waits for all-dungeon proof");
    world.current_level = 0;
    save_resume_snapshot = media_snapshot;
    save_resume_snapshot.runtime_level_source =
        THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME;
    save_resume_snapshot.runtime_track02_semantic_handoff = 0;
    save_resume_snapshot.runtime_fallback_visuals_blocked = 0;
    save_resume_snapshot.runtime_structured_route = 1;
    save_resume_snapshot.runtime_receipt_text_route = 0;
    expect_true(theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
                    &save_resume_snapshot,
                    &media_receipt,
                    &save_resume_view_model) &&
                    save_resume_view_model.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME &&
                    save_resume_view_model.runtime_structured_route &&
                    !save_resume_view_model.runtime_receipt_text_route,
                "boot startup view model carries save-resume runtime route");
    expect_true(theron_v1_boot_startup_render_route_receipt_from_view_model(
                    &save_resume_view_model,
                    &render_route_receipt) &&
                    render_route_receipt.runtime_level_render_allowed &&
                    render_route_receipt.runtime_readiness_ready &&
                    render_route_receipt.save_resume_runtime_handoff_ready &&
                    render_route_receipt.no_fallback_visuals_enforced &&
                    !render_route_receipt.fallback_visuals_allowed &&
                    render_route_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME &&
                    render_route_receipt.runtime_structured_route &&
                    !render_route_receipt.runtime_receipt_text_route &&
                    strcmp(render_route_receipt.status,
                           "SAVE RESUME RUNTIME READY") == 0,
                "boot render route receipt hands save-resume route to runtime without fallback visuals");
    expect_true(theron_v1_boot_startup_host_view_receipt_from_snapshot_with_media_receipt(
                    &save_resume_snapshot,
                    &media_receipt,
                    &host_view_receipt) &&
                    host_view_receipt.runtime_readiness_ready &&
                    host_view_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME &&
                    host_view_receipt.no_fallback_visuals_enforced &&
                    !host_view_receipt.fallback_visuals_allowed &&
                    host_view_receipt.runtime_graphics_handoff &&
                    !host_view_receipt.track02_runtime_graphics_handoff &&
                    host_view_receipt.save_resume_runtime_graphics_handoff &&
                    strcmp(host_view_receipt.status,
                           "SAVE RESUME RUNTIME READY") == 0,
                "boot host-view receipt exposes save-resume runtime graphics handoff without status parsing");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    expect_true(!theron_v1_boot_startup_execute_graphics_plan_from_snapshot_with_media_receipt(
                    &save_resume_snapshot,
                    &media_receipt,
                    &media_graphics_executor,
                    &graphics_route_receipt) &&
                    graphics_route_receipt.runtime_readiness_ready &&
                    graphics_route_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME &&
                    graphics_route_receipt.graphics_blocked &&
                    graphics_route_receipt.no_fallback_visuals_enforced &&
                    !graphics_route_receipt.fallback_visuals_allowed &&
                    graphics_route_receipt.runtime_graphics_handoff &&
                    !graphics_route_receipt.track02_runtime_graphics_handoff &&
                    graphics_route_receipt
                        .save_resume_runtime_graphics_handoff &&
                    media_graphics_counters.fill_count == 0 &&
                    strcmp(graphics_route_receipt.status,
                           "SAVE RESUME RUNTIME GRAPHICS HANDOFF") == 0,
                "boot graphics route receipt hands save-resume route to runtime without fallback startup draw");
    expect_true(theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
                    &save_resume_snapshot,
                    &media_receipt,
                    &media_graphics_executor,
                    &full_start_receipt) &&
                    theron_v1_boot_startup_menu_runtime_handoff_from_full_start_receipt(
                        &full_start_receipt,
                        -1,
                        -1,
                        -1,
                        &handoff_receipt) &&
                    handoff_receipt.runtime_handoff_ready &&
                    !handoff_receipt.track02_runtime_handoff_ready &&
                    handoff_receipt.save_resume_runtime_handoff_ready &&
                    handoff_receipt.startup_graphics_blocked &&
                    handoff_receipt.host_must_not_draw_fallback_visuals &&
                    !handoff_receipt.host_may_draw_fallback_visuals &&
                    !handoff_receipt.fallback_startup_graphics_executed,
                "boot menu/runtime handoff package exposes save-resume runtime no-fallback handoff");
    media_layout_roster_found = 0;
    expect_true(theron_v1_boot_startup_layout_build_from_view_model(
                    &media_view_model,
                    media_layout,
                    THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP) ==
                    media_view_model.layout_count,
                "boot startup layout consumer uses view model receipt");
    for (i = 0; i < media_view_model.layout_count; ++i) {
        if (strcmp(media_layout[i].label, "HAKAR-MEDIA") == 0) {
            media_layout_roster_found = 1;
        }
    }
    expect_true(media_layout_roster_found,
                "boot startup layout consumer preserves Track02 roster receipt labels");
    expect_true(theron_v1_boot_startup_execute_pointer_from_view_model(
                    &media_view_model,
                    50,
                    80,
                    &media_pointer_action,
                    &media_pointer_receipt) &&
                    media_pointer_action.kind ==
                        THERON_STARTUP_ACTION_TOGGLE_MIRROR &&
                    media_pointer_action.mirror_index == 0 &&
                    media_pointer_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot startup pointer consumer routes through view model layout receipt");
    expect_true(theron_v1_boot_startup_execute_input_from_view_model(
                    &direct_view_model,
                    THERON_STARTUP_INPUT_ACCEPT,
                    &stage_input_action,
                    &stage_input_receipt) &&
                    stage_input_action.kind ==
                        THERON_STARTUP_ACTION_CONTINUE_SAVE &&
                    stage_input_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot startup input consumer routes Continue from view model receipt");
    expect_true(theron_v1_boot_startup_execute_input_from_view_model(
                    &direct_view_model,
                    THERON_STARTUP_INPUT_DOWN,
                    &stage_input_action,
                    &stage_input_receipt) &&
                    stage_input_action.kind ==
                        THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR &&
                    stage_input_action.continue_focus == 0 &&
                    stage_input_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot startup input consumer moves stage focus from view model receipt");
    expect_true(theron_v1_boot_startup_execute_input_from_view_model(
                    &media_view_model,
                    THERON_STARTUP_INPUT_ACCEPT,
                    &media_input_action,
                    &media_input_receipt) &&
                    media_input_action.kind ==
                        THERON_STARTUP_ACTION_TOGGLE_MIRROR &&
                    media_input_action.mirror_index == 1 &&
                    media_input_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot startup input consumer toggles Track02 roster mirror from view model receipt");
    expect_true(theron_v1_boot_startup_execute_input_from_view_model(
                    &media_view_model,
                    THERON_STARTUP_INPUT_BACK,
                    &media_input_action,
                    &media_input_receipt) &&
                    media_input_action.kind ==
                        THERON_STARTUP_ACTION_SHOW_STAGE_SELECT &&
                    media_input_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot startup input consumer routes Back from Soul Room view model receipt");
    expect_true(theron_v1_boot_startup_execute_input_from_view_model_with_host_receipt(
                    &direct_view_model,
                    THERON_STARTUP_INPUT_ACCEPT,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result ==
                        THERON_STARTUP_ERR_NULL &&
                    view_model_host_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW &&
                    strcmp(view_model_host_receipt.host_receipt.status,
                           "CONTINUE FAILED") == 0,
                "boot startup view model host route owns Continue save failure receipt");
    expect_true(theron_v1_boot_startup_execute_input_from_view_model_with_host_receipt(
                    &media_view_model,
                    THERON_STARTUP_INPUT_ACCEPT,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.state_receipt.flow_changed &&
                    view_model_host_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW &&
                    strcmp(view_model_host_receipt.host_receipt.status,
                           "HERO RELEASED") == 0,
                "boot startup view model host route owns Track02 mirror toggle receipt");
    expect_true(theron_v1_boot_startup_execute_input_from_view_model_with_host_receipt(
                    &media_view_model,
                    THERON_STARTUP_INPUT_BACK,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.state_receipt.flow.phase ==
                        THERON_STARTUP_PHASE_STAGE_SELECT &&
                    view_model_host_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot startup view model host route owns Soul Room Back receipt");
    expect_true(theron_v1_boot_startup_execute_pointer_from_view_model_with_host_receipt(
                    &media_view_model,
                    50,
                    80,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.state_receipt.flow_changed &&
                    view_model_host_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot startup view model host route owns pointer mirror receipt");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    memset(&media_graphics_executor, 0, sizeof(media_graphics_executor));
    media_graphics_executor.userdata = &media_graphics_counters;
    media_graphics_executor.fill_rect = test_startup_fill_rect;
    media_graphics_executor.draw_rect = test_startup_draw_rect;
    media_graphics_executor.plot_pixel = test_startup_plot_pixel;
    expect_true(theron_v1_boot_startup_execute_graphics_plan_from_view_model(
                    &media_view_model,
                    &media_graphics_executor) &&
                    media_graphics_counters.fill_count > 0 &&
                    media_graphics_counters.rect_count > 0,
                "boot startup view model render route executes graphics plan receipt");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    expect_true(theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
                    &media_view_model,
                    &media_graphics_executor,
                    &graphics_route_receipt) &&
                    graphics_route_receipt.host_consumes_view_model &&
                    graphics_route_receipt.render_route_valid &&
                    graphics_route_receipt.graphics_plan_valid &&
                    graphics_route_receipt.graphics_executed &&
                    !graphics_route_receipt.graphics_blocked &&
                    graphics_route_receipt.track02_real_media_ready &&
                    graphics_route_receipt.real_bitmap_startup_graphics_ready &&
                    (graphics_route_receipt.bitmap_route_mask & 0x04u) &&
                    (graphics_route_receipt.bitmap_route_mask & 0x08u) &&
                    graphics_route_receipt.bitmap_route_count >= 4 &&
                    graphics_route_receipt.soul_room_bitmap_route_ready &&
                    graphics_route_receipt.forcefield_bitmap_route_ready &&
                    graphics_route_receipt.track02_startup_graphics_executed &&
                    graphics_route_receipt.track02_atlas_startup_graphics_ready &&
                    graphics_route_receipt.track02_atlas_startup_graphics_executed &&
                    (graphics_route_receipt.track02_atlas_graphics_route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM) &&
                    (graphics_route_receipt.track02_atlas_graphics_route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) &&
                    graphics_route_receipt.track02_atlas_graphics_route_count >= 2 &&
                    graphics_route_receipt.track02_atlas_graphics_pixel_count > 0u &&
                    graphics_route_receipt.track02_atlas_graphics_checksum != 0u &&
                    graphics_route_receipt
                        .track02_startup_graphic_receipt_valid &&
                    graphics_route_receipt.track02_startup_graphic_receipt.kind ==
                        THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME &&
                    graphics_route_receipt.track02_startup_graphic_receipt.x ==
                        28 &&
                    graphics_route_receipt.track02_startup_graphic_receipt.y ==
                        82 &&
                    !graphics_route_receipt.raw_graphics_plan_consumer_required &&
                    !graphics_route_receipt.no_fallback_startup_graphics_proof &&
                    !graphics_route_receipt.fallback_startup_graphics_executed &&
                    graphics_route_receipt.startup_menu_render_allowed &&
                    graphics_route_receipt.no_fallback_visuals_enforced &&
                    !graphics_route_receipt.fallback_visuals_allowed &&
                    media_graphics_counters.fill_count > 0 &&
                    media_graphics_counters.rect_count > 0,
                "boot graphics route receipt executes startup graphics from view model");
    {
        Theron_StartupMediaStateReceipt unpackaged_media_receipt =
            media_receipt;
        Theron_V1_BootStartupViewModel unpackaged_view_model;
        Theron_V1_BootStartupGraphicsRouteReceipt unpackaged_graphics_receipt;

        unpackaged_media_receipt.startup_bitmap_raw_route_mask = 0u;
        unpackaged_media_receipt.startup_bitmap_raw_route_count = 0;
        unpackaged_media_receipt.startup_bitmap_raw_atlas_tile_count = 0u;
        memset(&unpackaged_view_model, 0, sizeof(unpackaged_view_model));
        memset(&unpackaged_graphics_receipt,
               0,
               sizeof(unpackaged_graphics_receipt));
        memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
        expect_true(theron_v1_boot_startup_view_model_from_runtime_state_with_media_receipt(
                        &unpackaged_view_model,
                        &unpackaged_media_receipt,
                        THERON_STARTUP_PHASE_READY,
                        THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                        NULL,
                        &world,
                        NULL,
                        1,
                        1,
                        THERON_V1_STARTUP_RESUME_DUAL,
                        2,
                        3,
                        THERON_V1_SRM_PROGRESS_IMPORT_OK,
                        "/tmp/firestaff-theron-srm",
                        NULL,
                        NULL,
                        NULL,
                        0,
                        0x03,
                        2,
                        order,
                        THERON_STARTUP_MAX_COMPANIONS) &&
                        !theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
                            &unpackaged_view_model,
                            &media_graphics_executor,
                            &unpackaged_graphics_receipt) &&
                        !unpackaged_graphics_receipt.bitmap_package_route_ready &&
                        !unpackaged_graphics_receipt.required_bitmap_routes_ready &&
                        !unpackaged_graphics_receipt.real_bitmap_startup_graphics_ready &&
                        unpackaged_graphics_receipt
                            .raw_graphics_plan_consumer_required &&
                        unpackaged_graphics_receipt.graphics_blocked &&
                        unpackaged_graphics_receipt
                            .no_fallback_startup_graphics_proof &&
                        !unpackaged_graphics_receipt
                             .track02_startup_graphics_executed,
                    "boot graphics route blocks real readiness without raw Track02 package proof");
    }
    {
        Theron_StartupMediaStateReceipt partial_media_receipt =
            media_receipt;
        Theron_V1_BootStartupViewModel partial_media_view_model;
        Theron_V1_BootStartupGraphicsRouteReceipt partial_graphics_receipt;

        partial_media_receipt.startup_bitmap_route_mask &=
            ~THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
        partial_media_receipt.startup_bitmap_forcefield_route_ready = 0;
        partial_media_receipt.startup_bitmap_forcefield_sample_count = 0;
        partial_media_receipt.startup_bitmap_forcefield_nonzero_pixel_count = 0u;
        partial_media_receipt.startup_bitmap_forcefield_checksum = 0u;
        partial_media_receipt.startup_bitmap_sample_count = 3;
        partial_media_receipt.startup_bitmap_atlas_route_mask &=
            ~THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
        partial_media_receipt.startup_bitmap_atlas_route_count = 3;
        partial_media_receipt.startup_bitmap_atlas_tile_count = 3u;
        partial_media_receipt.startup_bitmap_raw_route_mask &=
            ~THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
        partial_media_receipt.startup_bitmap_raw_route_count = 3;
        partial_media_receipt.startup_bitmap_raw_atlas_tile_count = 24u;
        memset(&partial_media_view_model, 0, sizeof(partial_media_view_model));
        memset(&partial_graphics_receipt, 0, sizeof(partial_graphics_receipt));
        memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
        expect_true(theron_v1_boot_startup_view_model_from_runtime_state_with_media_receipt(
                        &partial_media_view_model,
                        &partial_media_receipt,
                        THERON_STARTUP_PHASE_READY,
                        THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                        NULL,
                        &world,
                        NULL,
                        1,
                        1,
                        THERON_V1_STARTUP_RESUME_DUAL,
                        2,
                        3,
                        THERON_V1_SRM_PROGRESS_IMPORT_OK,
                        "/tmp/firestaff-theron-srm",
                        NULL,
                        NULL,
                        NULL,
                        0,
                        0x03,
                        2,
                        order,
                        THERON_STARTUP_MAX_COMPANIONS) &&
                        !theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
                            &partial_media_view_model,
                            &media_graphics_executor,
                            &partial_graphics_receipt) &&
                        partial_graphics_receipt.required_bitmap_route_mask ==
                            TST_THERON_FULL_START_BITMAP_ROUTES &&
                        partial_graphics_receipt.required_bitmap_route_count == 4 &&
                        !partial_graphics_receipt.required_bitmap_routes_ready &&
                        !partial_graphics_receipt.real_bitmap_startup_graphics_ready &&
                        partial_graphics_receipt.raw_graphics_plan_consumer_required &&
                        !partial_graphics_receipt.track02_startup_graphics_executed &&
                        partial_graphics_receipt.graphics_blocked &&
                        partial_graphics_receipt.no_fallback_startup_graphics_proof &&
                        !partial_graphics_receipt.fallback_startup_graphics_executed,
                    "boot graphics route blocks incomplete verified Track02 bitmap routes");
    }
    {
        Theron_StartupMediaStateReceipt thin_media_receipt = media_receipt;
        Theron_V1_BootStartupViewModel thin_media_view_model;
        Theron_V1_BootStartupGraphicsRouteReceipt thin_graphics_receipt;

        thin_media_receipt.startup_bitmap_atlas_tile_count = 8u;
        thin_media_receipt.startup_bitmap_title_atlas_tile_count = 2u;
        thin_media_receipt.startup_bitmap_stage_atlas_tile_count = 2u;
        thin_media_receipt.startup_bitmap_title_atlas_width = 16u;
        thin_media_receipt.startup_bitmap_stage_atlas_width = 16u;
        thin_media_receipt.startup_bitmap_atlas.total_tile_count = 8u;
        thin_media_receipt.startup_bitmap_atlas.routes[0].tile_count = 2u;
        thin_media_receipt.startup_bitmap_atlas.routes[0].width = 16u;
        thin_media_receipt.startup_bitmap_atlas.routes[1].tile_count = 2u;
        thin_media_receipt.startup_bitmap_atlas.routes[1].width = 16u;
        memset(&thin_media_view_model, 0, sizeof(thin_media_view_model));
        memset(&thin_graphics_receipt, 0, sizeof(thin_graphics_receipt));
        memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
        expect_true(!theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
                        &thin_media_receipt) &&
                        theron_v1_boot_startup_view_model_from_runtime_state_with_media_receipt(
                            &thin_media_view_model,
                            &thin_media_receipt,
                            THERON_STARTUP_PHASE_READY,
                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                            NULL,
                            &world,
                            NULL,
                            1,
                            1,
                            THERON_V1_STARTUP_RESUME_DUAL,
                            2,
                            3,
                            THERON_V1_SRM_PROGRESS_IMPORT_OK,
                            "/tmp/firestaff-theron-srm",
                            NULL,
                            NULL,
                            NULL,
                            0,
                            0x03,
                            2,
                            order,
                            THERON_STARTUP_MAX_COMPANIONS) &&
                        !theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
                            &thin_media_view_model,
                            &media_graphics_executor,
                            &thin_graphics_receipt) &&
                        thin_graphics_receipt.required_bitmap_route_mask ==
                            TST_THERON_FULL_START_BITMAP_ROUTES &&
                        thin_graphics_receipt.required_bitmap_route_count == 4 &&
                        !thin_graphics_receipt.required_bitmap_routes_ready &&
                        !thin_graphics_receipt.real_bitmap_startup_graphics_ready &&
                        !thin_graphics_receipt.track02_atlas_startup_graphics_ready &&
                        !thin_graphics_receipt.track02_atlas_startup_graphics_executed &&
                        !thin_graphics_receipt.track02_startup_graphics_executed &&
                        thin_graphics_receipt.raw_graphics_plan_consumer_required &&
                        thin_graphics_receipt.graphics_blocked &&
                        thin_graphics_receipt.no_fallback_startup_graphics_proof &&
                        !thin_graphics_receipt.fallback_startup_graphics_executed,
                    "boot graphics route blocks thin verified Track02 title/stage atlas routes");
    }
    {
        Theron_StartupMediaStateReceipt iso_media_receipt = media_receipt;
        Theron_V1_BootStartupViewModel iso_media_view_model;
        Theron_V1_BootStartupGraphicsRouteReceipt iso_graphics_receipt;

        iso_media_receipt.track02_variant = THERON_TRACK02_VARIANT_US_ISO;
        snprintf(iso_media_receipt.track02_md5,
                 sizeof(iso_media_receipt.track02_md5),
                 "%s",
                 THERON_TRACK02_MD5_US_ISO);
        iso_media_receipt.startup_bitmap_sample_count = 4;
        iso_media_receipt.startup_bitmap_route_mask =
            THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
            THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
        iso_media_receipt.startup_bitmap_title_route_ready = 0;
        iso_media_receipt.startup_bitmap_stage_route_ready = 0;
        iso_media_receipt.startup_bitmap_title_sample_count = 0;
        iso_media_receipt.startup_bitmap_stage_sample_count = 0;
        iso_media_receipt.startup_bitmap_title_nonzero_pixel_count = 0u;
        iso_media_receipt.startup_bitmap_stage_nonzero_pixel_count = 0u;
        iso_media_receipt.startup_bitmap_title_checksum = 0u;
        iso_media_receipt.startup_bitmap_stage_checksum = 0u;
        iso_media_receipt.startup_bitmap_atlas_route_count = 2;
        iso_media_receipt.startup_bitmap_atlas_route_mask =
            iso_media_receipt.startup_bitmap_route_mask;
        iso_media_receipt.startup_bitmap_atlas_tile_count = 4u;
        iso_media_receipt.startup_bitmap_atlas.route_count = 2u;
        iso_media_receipt.startup_bitmap_atlas.route_mask =
            iso_media_receipt.startup_bitmap_atlas_route_mask;
        iso_media_receipt.startup_bitmap_atlas.total_tile_count = 4u;
        iso_media_receipt.startup_bitmap_raw_route_mask = 0u;
        iso_media_receipt.startup_bitmap_raw_route_count = 0;
        iso_media_receipt.startup_bitmap_raw_atlas_tile_count = 0u;
        iso_media_receipt.startup_bitmap_iso_route_mask =
            iso_media_receipt.startup_bitmap_route_mask;
        iso_media_receipt.startup_bitmap_iso_route_count = 2;
        iso_media_receipt.startup_bitmap_iso_atlas_tile_count = 4u;
        iso_media_receipt.startup_bitmap_atlas.routes[0] =
            media_receipt.startup_bitmap_atlas.routes[2];
        iso_media_receipt.startup_bitmap_atlas.routes[1] =
            media_receipt.startup_bitmap_atlas.routes[3];
        memset(&iso_media_receipt.startup_bitmap_atlas.routes[2],
               0,
               sizeof(iso_media_receipt.startup_bitmap_atlas.routes[2]));
        memset(&iso_media_receipt.startup_bitmap_atlas.routes[3],
               0,
               sizeof(iso_media_receipt.startup_bitmap_atlas.routes[3]));
        memset(&iso_media_view_model, 0, sizeof(iso_media_view_model));
        memset(&iso_graphics_receipt, 0, sizeof(iso_graphics_receipt));
        memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
        expect_true(!theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
                        &iso_media_receipt) &&
                        theron_v1_boot_startup_view_model_from_runtime_state_with_media_receipt(
                            &iso_media_view_model,
                            &iso_media_receipt,
                            THERON_STARTUP_PHASE_READY,
                            THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                            NULL,
                            &world,
                            NULL,
                            1,
                            1,
                            THERON_V1_STARTUP_RESUME_DUAL,
                            2,
                            3,
                            THERON_V1_SRM_PROGRESS_IMPORT_OK,
                            "/tmp/firestaff-theron-srm",
                            NULL,
                            NULL,
                            NULL,
                            0,
                            0x03,
                            2,
                            order,
                            THERON_STARTUP_MAX_COMPANIONS) &&
                        !theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
                            &iso_media_view_model,
                            &media_graphics_executor,
                            &iso_graphics_receipt) &&
                        iso_graphics_receipt.required_bitmap_route_mask ==
                            TST_THERON_FULL_START_BITMAP_ROUTES &&
                        iso_graphics_receipt.required_bitmap_route_count == 4 &&
                        !iso_graphics_receipt.required_bitmap_routes_ready &&
                        !iso_graphics_receipt.real_bitmap_startup_graphics_ready &&
                        !iso_graphics_receipt.track02_atlas_startup_graphics_ready &&
                        !iso_graphics_receipt.track02_atlas_startup_graphics_executed &&
                        !iso_graphics_receipt.track02_startup_graphics_executed &&
                        iso_graphics_receipt.raw_graphics_plan_consumer_required &&
                        iso_graphics_receipt.graphics_blocked &&
                        iso_graphics_receipt.no_fallback_startup_graphics_proof &&
                        !iso_graphics_receipt.fallback_startup_graphics_executed,
                    "boot graphics route blocks partial verified ISO Track02 routes");
    }
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    expect_true(theron_v1_boot_startup_execute_graphics_plan_from_snapshot_with_media_receipt(
                    &media_snapshot,
                    &media_receipt,
                    &media_graphics_executor,
                    &graphics_route_receipt) &&
                    graphics_route_receipt.host_consumes_view_model &&
                    graphics_route_receipt.render_route_valid &&
                    graphics_route_receipt.graphics_executed &&
                    graphics_route_receipt.startup_menu_render_allowed &&
                    media_graphics_counters.fill_count > 0,
                "boot snapshot graphics route consumes Track02 media receipt without raw UI adapter");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    expect_true(theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
                    &media_snapshot,
                    &media_receipt,
                    &media_graphics_executor,
                    &full_start_receipt) &&
                    full_start_receipt.host_consumes_view_model &&
                    full_start_receipt.view_model_valid &&
                    full_start_receipt.host_view_valid &&
                    full_start_receipt.graphics_route_valid &&
                    full_start_receipt.title_menu_ready &&
                    full_start_receipt.stage_menu_ready &&
                    full_start_receipt.soul_room_menu_ready &&
                    full_start_receipt.forcefield_menu_ready &&
                    full_start_receipt.save_resume_start_ready &&
                    full_start_receipt.save_resume_runtime_handoff_ready &&
                    full_start_receipt.forcefield_runtime_handoff_ready &&
                    full_start_receipt.full_start_graphics_ready &&
                    full_start_receipt.full_start_graphics_executed &&
                    !full_start_receipt.full_start_graphics_blocked &&
                    full_start_receipt.track02_real_media_ready &&
                    full_start_receipt.real_bitmap_startup_graphics_ready &&
                    full_start_receipt.required_bitmap_routes_ready &&
                    full_start_receipt.required_bitmap_route_count == 4 &&
                    full_start_receipt.bitmap_package_route_ready &&
                    full_start_receipt.raw_bitmap_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    full_start_receipt.raw_bitmap_route_count == 4 &&
                    full_start_receipt.raw_bitmap_atlas_tile_count == 48u &&
                    full_start_receipt.iso_bitmap_route_mask == 0u &&
                    (full_start_receipt.bitmap_route_mask & 0x04u) &&
                    (full_start_receipt.bitmap_route_mask & 0x08u) &&
                    full_start_receipt.bitmap_route_count >= 4 &&
                    full_start_receipt.soul_room_bitmap_route_ready &&
                    full_start_receipt.forcefield_bitmap_route_ready &&
                    full_start_receipt.track02_startup_graphics_executed &&
                    full_start_receipt.track02_startup_graphic_receipt_valid &&
                    full_start_receipt.track02_startup_graphic_receipt.kind ==
                        THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME &&
                    full_start_receipt.track02_startup_graphic_receipt.w ==
                        54 &&
                    full_start_receipt.track02_startup_graphic_receipt.h ==
                        28 &&
                    !full_start_receipt.raw_graphics_plan_consumer_required &&
                    !full_start_receipt.no_fallback_startup_graphics_proof &&
                    !full_start_receipt.fallback_startup_graphics_executed &&
                    full_start_receipt.no_fallback_visuals_enforced &&
                    !full_start_receipt.fallback_visuals_allowed &&
                    !full_start_receipt.raw_prompt_roster_required &&
                    !full_start_receipt.raw_session_rebuild_required &&
                    media_graphics_counters.fill_count > 0 &&
                    strcmp(full_start_receipt.status,
                           "FULL START GRAPHICS READY") == 0,
                "boot full-start receipt owns title stage soul-room save-resume graphics readiness");
    {
        Theron_StartupMediaStateReceipt iso_full_media_receipt =
            media_receipt;
        Theron_V1_BootStartupViewModel iso_full_view_model;
        Theron_V1_BootStartupGraphicsRouteReceipt iso_full_graphics_receipt;

        iso_full_media_receipt.track02_variant =
            THERON_TRACK02_VARIANT_US_ISO;
        snprintf(iso_full_media_receipt.track02_md5,
                 sizeof(iso_full_media_receipt.track02_md5),
                 "%s",
                 THERON_TRACK02_MD5_US_ISO);
        iso_full_media_receipt.startup_bitmap_atlas.variant =
            THERON_TRACK02_VARIANT_US_ISO;
        iso_full_media_receipt.startup_bitmap_raw_route_mask = 0u;
        iso_full_media_receipt.startup_bitmap_raw_route_count = 0;
        iso_full_media_receipt.startup_bitmap_raw_atlas_tile_count = 0u;
        iso_full_media_receipt.startup_bitmap_iso_route_mask =
            TST_THERON_FULL_START_BITMAP_ROUTES;
        iso_full_media_receipt.startup_bitmap_iso_route_count = 4;
        iso_full_media_receipt.startup_bitmap_iso_atlas_tile_count = 32u;
        memset(&iso_full_view_model, 0, sizeof(iso_full_view_model));
        memset(&iso_full_graphics_receipt,
               0,
               sizeof(iso_full_graphics_receipt));
        memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
        expect_true(theron_v1_boot_startup_view_model_from_runtime_state_with_media_receipt(
                        &iso_full_view_model,
                        &iso_full_media_receipt,
                        THERON_STARTUP_PHASE_READY,
                        THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                        NULL,
                        &world,
                        NULL,
                        1,
                        1,
                        THERON_V1_STARTUP_RESUME_DUAL,
                        2,
                        3,
                        THERON_V1_SRM_PROGRESS_IMPORT_OK,
                        "/tmp/firestaff-theron-srm",
                        NULL,
                        NULL,
                        NULL,
                        0,
                        0x03,
                        2,
                        order,
                        THERON_STARTUP_MAX_COMPANIONS) &&
                        theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
                            &iso_full_view_model,
                            &media_graphics_executor,
                            &iso_full_graphics_receipt) &&
                        iso_full_graphics_receipt.bitmap_package_route_ready &&
                        iso_full_graphics_receipt.required_bitmap_routes_ready &&
                        iso_full_graphics_receipt.real_bitmap_startup_graphics_ready &&
                        iso_full_graphics_receipt.iso_bitmap_route_mask ==
                            TST_THERON_FULL_START_BITMAP_ROUTES &&
                        iso_full_graphics_receipt.iso_bitmap_route_count == 4 &&
                        iso_full_graphics_receipt
                                .iso_bitmap_atlas_tile_count == 32u &&
                        iso_full_graphics_receipt.raw_bitmap_route_mask == 0u &&
                        !iso_full_graphics_receipt
                             .raw_graphics_plan_consumer_required &&
                        !iso_full_graphics_receipt
                             .fallback_startup_graphics_executed,
                    "boot graphics route accepts complete ISO Track02 package proof");
    }
    memset(media_layout, 0, sizeof(media_layout));
    expect_true(theron_v1_boot_startup_layout_build_from_full_start_receipt(
                    &full_start_receipt,
                    media_layout,
                    THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP) ==
                    full_start_receipt.view_model.layout_count &&
                    strcmp(media_layout[2].label, "HAKAR-MEDIA") == 0,
                "boot full-start receipt supplies startup layout without raw rebuild");
    memset(media_rows, 0, sizeof(media_rows));
    media_prompt_row_found = 0;
    expect_true(theron_v1_boot_startup_render_rows_from_full_start_receipt(
                    &full_start_receipt,
                    media_rows,
                    THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP) ==
                    full_start_receipt.view_model.row_count,
                "boot full-start receipt supplies startup render rows without raw rebuild");
    for (i = 0; i < full_start_receipt.view_model.row_count; ++i) {
        if (strstr(media_rows[i], "GO AWAY") != NULL) {
            media_prompt_row_found = 1;
        }
    }
    expect_true(media_prompt_row_found,
                "boot full-start receipt preserves Track02 prompt row");
    theron_v1_boot_startup_host_view_receipt_init(&host_view_receipt);
    expect_true(theron_v1_boot_startup_host_view_from_full_start_receipt(
                    &full_start_receipt,
                    &host_view_receipt) &&
                    host_view_receipt.host_consumes_view_model &&
                    host_view_receipt.render_route_valid &&
                    host_view_receipt.track02_media_consumed &&
                    !host_view_receipt.raw_prompt_roster_required &&
                    !host_view_receipt.raw_session_rebuild_required,
                "boot full-start receipt supplies host-view receipt without status parsing");
    theron_v1_boot_startup_host_render_receipt_init(&host_render_receipt);
    expect_true(theron_v1_boot_startup_host_render_receipt_from_full_start_receipt(
                    &full_start_receipt,
                    &host_render_receipt) &&
                    host_render_receipt.host_consumes_full_start_receipt &&
                    host_render_receipt.full_start_valid &&
                    host_render_receipt.layout_count ==
                        full_start_receipt.view_model.layout_count &&
                    host_render_receipt.row_count ==
                        full_start_receipt.view_model.row_count &&
                    host_render_receipt.render_plan_valid &&
                    host_render_receipt.render_plan.text_count ==
                        full_start_receipt.view_model.render_plan.text_count &&
                    host_render_receipt.render_plan.graphic_count ==
                        full_start_receipt.view_model.render_plan.graphic_count &&
                    host_render_receipt.render_route_valid &&
                    host_render_receipt.graphics_route_valid &&
                    host_render_receipt.graphics_executor_consumed &&
                    host_render_receipt.full_start_graphics_ready &&
                    host_render_receipt.full_start_graphics_executed &&
                    !host_render_receipt.full_start_graphics_blocked &&
                    host_render_receipt.track02_real_media_ready &&
                    host_render_receipt.real_bitmap_startup_graphics_ready &&
                    (host_render_receipt.bitmap_route_mask & 0x04u) &&
                    (host_render_receipt.bitmap_route_mask & 0x08u) &&
                    host_render_receipt.bitmap_route_count >= 4 &&
                    host_render_receipt.soul_room_bitmap_route_ready &&
                    host_render_receipt.forcefield_bitmap_route_ready &&
                    host_render_receipt.track02_startup_graphics_executed &&
                    host_render_receipt
                        .track02_startup_graphic_receipt_valid &&
                    host_render_receipt.track02_startup_graphic_receipt.kind ==
                        THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME &&
                    !host_render_receipt.raw_prompt_roster_required &&
                    !host_render_receipt.raw_session_rebuild_required &&
                    !host_render_receipt.raw_graphics_plan_consumer_required,
                "boot host render receipt packages Track02 bitmap routes without raw prompt roster or render-plan consumer");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    theron_v1_boot_startup_host_render_receipt_init(&host_render_receipt);
    expect_true(theron_v1_boot_startup_host_render_receipt_from_runtime_state_with_media_receipt_and_executor(
                    &host_render_receipt,
                    &media_receipt,
                    &media_graphics_executor,
                    media_snapshot.startup_phase,
                    media_snapshot.selected_dungeon,
                    media_snapshot.boot_profile,
                    media_snapshot.world,
                    media_snapshot.assets,
                    media_snapshot.startup_cursor,
                    media_snapshot.continue_focus,
                    media_snapshot.resume_claim,
                    media_snapshot.tqsv_slot,
                    media_snapshot.srm_slot,
                    media_snapshot.srm_import_status,
                    media_snapshot.srm_root,
                    media_snapshot.selected_mirrors_mask,
                    media_snapshot.companion_count,
                    media_snapshot.selected_mirror_order,
                    media_snapshot.selected_mirror_order_count) &&
                    host_render_receipt.host_consumes_full_start_receipt &&
                    host_render_receipt.graphics_executor_consumed &&
                    host_render_receipt.full_start_graphics_ready &&
                    host_render_receipt.full_start_graphics_executed &&
                    !host_render_receipt.full_start_graphics_blocked &&
                    host_render_receipt.track02_startup_graphics_executed &&
                    host_render_receipt
                        .track02_startup_graphic_receipt_valid &&
                    host_render_receipt.track02_startup_graphic_receipt.kind ==
                        THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME &&
                    media_graphics_counters.fill_count > 0 &&
                    media_graphics_counters.rect_count > 0,
                "boot runtime-state host render receipt consumes executor and returns full-start graphics proof");
    memset(&media_graphics_counters, 0, sizeof(media_graphics_counters));
    theron_v1_boot_startup_host_render_receipt_init(&host_render_receipt);
    expect_true(theron_v1_boot_startup_host_render_receipt_from_snapshot_with_media_receipt_and_executor(
                    &host_render_receipt,
                    &media_snapshot,
                    &media_receipt,
                    &media_graphics_executor) &&
                    host_render_receipt.host_consumes_full_start_receipt &&
                    host_render_receipt.graphics_executor_consumed &&
                    host_render_receipt.full_start_graphics_ready &&
                    host_render_receipt.full_start_graphics_executed &&
                    host_render_receipt.track02_real_media_ready &&
                    host_render_receipt.real_bitmap_startup_graphics_ready &&
                    host_render_receipt.track02_startup_graphics_executed &&
                    !host_render_receipt.raw_prompt_roster_required &&
                    !host_render_receipt.raw_session_rebuild_required &&
                    !host_render_receipt.raw_graphics_plan_consumer_required &&
                    media_graphics_counters.fill_count > 0 &&
                    media_graphics_counters.rect_count > 0,
                "boot snapshot host-render receipt consumes Track02 media executor without runtime field rebuild");
    theron_v1_boot_startup_host_render_receipt_init(&host_render_receipt);
    expect_true(theron_v1_boot_startup_host_render_receipt_from_runtime_state_with_media_receipt(
                    &host_render_receipt,
                    &media_receipt,
                    media_snapshot.startup_phase,
                    media_snapshot.selected_dungeon,
                    media_snapshot.boot_profile,
                    media_snapshot.world,
                    media_snapshot.assets,
                    media_snapshot.startup_cursor,
                    media_snapshot.continue_focus,
                    media_snapshot.resume_claim,
                    media_snapshot.tqsv_slot,
                    media_snapshot.srm_slot,
                    media_snapshot.srm_import_status,
                    media_snapshot.srm_root,
                    media_snapshot.selected_mirrors_mask,
                    media_snapshot.companion_count,
                    media_snapshot.selected_mirror_order,
                    media_snapshot.selected_mirror_order_count) &&
                    host_render_receipt.host_consumes_full_start_receipt &&
                    host_render_receipt.track02_real_media_ready &&
                    host_render_receipt.real_bitmap_startup_graphics_ready &&
                    !host_render_receipt.raw_prompt_roster_required &&
                    !host_render_receipt.raw_session_rebuild_required &&
                    !host_render_receipt.raw_graphics_plan_consumer_required,
                "boot runtime-state host render receipt consumes Track02 media without raw arrays");
    {
        Theron_V1_BootRuntimeStartupSnapshot title_snapshot = media_snapshot;
        title_snapshot.startup_phase = THERON_STARTUP_PHASE_TITLE;
        expect_true(theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
                        &title_snapshot,
                        &media_receipt,
                        &media_graphics_executor,
                        &full_start_receipt) &&
                        full_start_receipt.bitmap_route_mask == 0x01u &&
                        full_start_receipt.bitmap_route_count == 1 &&
                        full_start_receipt.title_bitmap_route_ready &&
                        !full_start_receipt.stage_bitmap_route_ready &&
                        !full_start_receipt.soul_room_bitmap_route_ready &&
                        !full_start_receipt.forcefield_bitmap_route_ready &&
                        theron_v1_boot_startup_execute_input_from_full_start_receipt(
                            &full_start_receipt,
                            THERON_STARTUP_INPUT_ACCEPT,
                            &view_model_host_receipt) &&
                        view_model_host_receipt.result == THERON_STARTUP_OK &&
                        view_model_host_receipt.state_receipt_valid &&
                        view_model_host_receipt.state_receipt.flow.phase ==
                            THERON_STARTUP_PHASE_STAGE_SELECT,
                    "boot full-start receipt routes title accept to stage menu");
    }
    {
        Theron_V1_BootRuntimeStartupSnapshot stage_snapshot = snapshot;
        stage_snapshot.startup_phase = THERON_STARTUP_PHASE_STAGE_SELECT;
        stage_snapshot.selected_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
        stage_snapshot.startup_cursor = 0;
        stage_snapshot.continue_focus = 0;
        stage_snapshot.resume_claim = THERON_V1_STARTUP_RESUME_NONE;
        expect_true(theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
                        &stage_snapshot,
                        &media_receipt,
                        &media_graphics_executor,
                        &full_start_receipt) &&
                        (full_start_receipt.bitmap_route_mask & 0x02u) &&
                        full_start_receipt.bitmap_route_count >= 1 &&
                        !full_start_receipt.title_bitmap_route_ready &&
                        full_start_receipt.stage_bitmap_route_ready &&
                        !full_start_receipt.soul_room_bitmap_route_ready &&
                        !full_start_receipt.forcefield_bitmap_route_ready &&
                        theron_v1_boot_startup_execute_input_from_full_start_receipt(
                            &full_start_receipt,
                            THERON_STARTUP_INPUT_ACCEPT,
                            &view_model_host_receipt) &&
                        view_model_host_receipt.result == THERON_STARTUP_OK &&
                        view_model_host_receipt.state_receipt_valid &&
                        view_model_host_receipt.state_receipt.flow.phase ==
                            THERON_STARTUP_PHASE_SOUL_ROOM,
                    "boot full-start receipt routes stage accept to Soul Room");
    }
    expect_true(theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
                    &media_snapshot,
                    &media_receipt,
                    &media_graphics_executor,
                    &full_start_receipt),
                "boot full-start receipt restores Soul Room fixture for action routing");
    expect_true(theron_v1_boot_startup_execute_input_from_full_start_receipt(
                    &full_start_receipt,
                    THERON_STARTUP_INPUT_ACCEPT,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW &&
                    strcmp(view_model_host_receipt.host_receipt.status,
                           "HERO RELEASED") == 0,
                "boot full-start receipt routes Soul Room input without rebuilding startup session");
    expect_true(theron_v1_boot_startup_execute_input_from_full_start_receipt(
                    &full_start_receipt,
                    THERON_STARTUP_INPUT_BACK,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.state_receipt.flow.phase ==
                        THERON_STARTUP_PHASE_STAGE_SELECT,
                "boot full-start receipt routes Back to stage menu without raw startup fields");
    expect_true(theron_v1_boot_startup_execute_pointer_from_full_start_receipt(
                    &full_start_receipt,
                    50,
                    80,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot full-start receipt routes Soul Room pointer without rebuilding layout");
    theron_v1_boot_startup_full_start_receipt_init(&full_start_receipt);
    expect_true(!theron_v1_boot_startup_execute_input_from_full_start_receipt(
                    &full_start_receipt,
                    THERON_STARTUP_INPUT_ACCEPT,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result == THERON_STARTUP_ERR_NULL &&
                    strcmp(view_model_host_receipt.host_receipt.status,
                           "FULL START RECEIPT MISSING") == 0,
                "boot full-start receipt input rejects missing view model");
    expect_true(theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
                    &media_snapshot,
                    &media_receipt,
                    NULL,
                    &full_start_receipt) &&
                    theron_v1_boot_startup_host_render_receipt_from_full_start_receipt(
                        &full_start_receipt,
                        &host_render_receipt),
                "boot startup snapshot media receipt builds host-render receipt through full-start receipt");
    media_prompt_row_found = 0;
    media_roster_row_found = 0;
    for (i = 0; i < host_render_receipt.row_count; ++i) {
        if (strstr(host_render_receipt.rows[i], "RESURRECT THERON") != NULL) {
            media_prompt_row_found = 1;
        }
        if (strstr(host_render_receipt.rows[i], "HAKAR-MEDIA") != NULL) {
            media_roster_row_found = 1;
        }
    }
    expect_true(media_prompt_row_found && media_roster_row_found,
                "boot startup snapshot host-render receipt preserves Track02 render text");
    expect_true(host_render_receipt.layout_count > 0 &&
                    strcmp(host_render_receipt.layout[2].label,
                           "HAKAR-MEDIA") == 0 &&
                    host_render_receipt.render_plan_valid &&
                    host_render_receipt.render_plan.text_count > 0 &&
                    host_render_receipt.render_plan.graphic_count > 0 &&
                    !host_render_receipt.raw_prompt_roster_required &&
                    !host_render_receipt.raw_session_rebuild_required &&
                    !host_render_receipt.raw_graphics_plan_consumer_required,
                "boot startup snapshot host-render receipt replaces Track02 layout and render-plan media exports");
    expect_true(theron_v1_boot_startup_execute_input_from_snapshot_with_media_receipt(
                    &media_snapshot,
                    &media_receipt,
                    9,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot startup snapshot media receipt routes input through view model host receipt");
    expect_true(theron_v1_boot_startup_execute_pointer_from_snapshot_with_media_receipt(
                    &media_snapshot,
                    &media_receipt,
                    50,
                    80,
                    &view_model_host_receipt) &&
                    view_model_host_receipt.result == THERON_STARTUP_OK &&
                    view_model_host_receipt.state_receipt_valid &&
                    view_model_host_receipt.host_receipt.input_result ==
                        THERON_STARTUP_INPUT_RESULT_REDRAW,
                "boot startup snapshot media receipt routes pointer through view model host receipt");

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

static void test_track02_startup_bitmap_decode_receipt(void) {
    static const size_t descriptor_offsets[3] = {
        0x70be06u, 0x70e2c6u, 0x710904u
    };
    static const size_t span_offsets[3] = {
        0x2d53e0u, 0x47d040u, 0x712840u
    };
    static const uint8_t post_boundary_span[44] = {
        0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
        0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
        0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
        0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
        0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
        0x93, 0x80, 0x00, 0x3f
    };
    const size_t raw_span_bitmap_bytes = 92u;
    const size_t track02_size =
        ((span_offsets[2] + raw_span_bitmap_bytes +
          THERON_TRACK02_RAW_SECTOR_BYTES - 1u) /
         THERON_TRACK02_RAW_SECTOR_BYTES) *
        THERON_TRACK02_RAW_SECTOR_BYTES;
    uint8_t *track02 = (uint8_t *)calloc(track02_size, 1u);
    Theron_Track02StartupBitmapCatalog catalog;
    Theron_Track02StartupBitmapAtlas atlas;
    Theron_StartupMediaStateReceipt receipt;
    Theron_V1_StartupReceipt startup_receipt;
    const Theron_Track02StartupBitmapAtlasRoute *title_route = NULL;
    const Theron_Track02StartupBitmapAtlasRoute *stage_route = NULL;
    const Theron_Track02StartupBitmapAtlasRoute *soul_route = NULL;
    const Theron_Track02StartupBitmapAtlasRoute *forcefield_route = NULL;

    expect_true(track02 != NULL,
                "Track02 startup bitmap sparse fixture allocates");
    if (!track02) {
        return;
    }

    for (size_t anchor = 0u; anchor < 3u; ++anchor) {
        size_t descriptor = descriptor_offsets[anchor];
        for (size_t i = 0u; i < 9u; ++i) {
            wr16le_test(track02 + descriptor + i * 2u,
                        (uint16_t)(0x0020u + i * 0x0400u));
        }
        memcpy(track02 + span_offsets[anchor],
               post_boundary_span,
               sizeof(post_boundary_span));
        for (size_t i = sizeof(post_boundary_span);
             i < raw_span_bitmap_bytes;
             ++i) {
            track02[span_offsets[anchor] + i] =
                (uint8_t)(0x33u + ((anchor * 19u + i * 11u) % 0xccu));
        }
    }

    expect_true(theron_v1_track02_catalog_startup_bitmap_samples(
                    track02,
                    track02_size,
                    THERON_TRACK02_MD5_US_BIN,
                    &catalog) == THERON_TRACK02_SIGNAL_OK &&
                    catalog.sample_count == 60u &&
                    catalog.overflow_count == 0u &&
                    (catalog.route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM) &&
                    (catalog.route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) &&
                    catalog.samples[20].raw_offset == span_offsets[0] + 16u &&
                    catalog.samples[21].raw_offset == span_offsets[0] + 20u &&
                    catalog.samples[22].raw_offset == span_offsets[0] + 24u &&
                    catalog.samples[23].raw_offset == span_offsets[0] + 28u &&
                    catalog.samples[24].raw_offset == span_offsets[0] + 32u &&
                    catalog.samples[25].raw_offset == span_offsets[0] + 36u &&
                    catalog.samples[26].raw_offset == span_offsets[0] + 40u &&
                    catalog.samples[27].raw_offset == span_offsets[0] + 44u &&
                    catalog.samples[31].raw_offset == span_offsets[0] + 60u &&
                    catalog.samples[32].raw_offset == span_offsets[1] + 32u &&
                    catalog.samples[35].raw_offset == span_offsets[1] + 44u &&
                    catalog.samples[36].raw_offset == span_offsets[2] + 32u &&
                    catalog.samples[39].raw_offset == span_offsets[2] + 44u &&
                    catalog.samples[40].raw_offset == span_offsets[1] &&
                    catalog.samples[43].raw_offset == span_offsets[2] + 12u &&
                    catalog.samples[44].raw_offset == span_offsets[1] + 48u &&
                    catalog.samples[47].raw_offset == span_offsets[1] + 60u &&
                    catalog.samples[48].raw_offset == span_offsets[2] + 48u &&
                    catalog.samples[51].raw_offset == span_offsets[2] + 60u &&
                    catalog.samples[52].raw_offset == span_offsets[1] + 16u &&
                    catalog.samples[55].raw_offset == span_offsets[1] + 28u &&
                    catalog.samples[56].raw_offset == span_offsets[2] + 16u &&
                    catalog.samples[59].raw_offset == span_offsets[2] + 28u &&
                    catalog.samples[0].nonzero_pixel_count > 0u &&
                    catalog.samples[0].checksum != 0u,
                "Track02 startup bitmap catalog decodes wider real 4bpp samples from raw-sector graphics spans");
    expect_true(theron_v1_track02_build_startup_bitmap_atlas(
                    &catalog,
                    &atlas) == THERON_TRACK02_SIGNAL_OK &&
                    atlas.route_count == 4u &&
                    atlas.total_tile_count == 60u &&
                    atlas.route_mask ==
                        (THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
                         THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
                         THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
                         THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) &&
                    atlas.routes[0].width == 112u &&
                    atlas.routes[0].height == 8u &&
                    atlas.routes[0].raw_offsets[0] == span_offsets[0] &&
                    atlas.routes[0].raw_offsets[1] == span_offsets[0] + 4u &&
                    atlas.routes[0].raw_offsets[2] == span_offsets[0] + 16u &&
                    atlas.routes[0].raw_offsets[3] == span_offsets[0] + 20u &&
                    atlas.routes[0].raw_offsets[4] == span_offsets[0] + 32u &&
                    atlas.routes[0].raw_offsets[5] == span_offsets[0] + 36u &&
                    atlas.routes[0].raw_offsets[6] == span_offsets[0] + 48u &&
                    atlas.routes[0].raw_offsets[7] == span_offsets[0] + 52u &&
                    atlas.routes[0].raw_offsets[8] == span_offsets[1] &&
                    atlas.routes[0].raw_offsets[9] == span_offsets[1] + 4u &&
                    atlas.routes[0].raw_offsets[10] == span_offsets[1] + 16u &&
                    atlas.routes[0].raw_offsets[13] == span_offsets[1] + 28u &&
                    atlas.routes[1].width == 112u &&
                    atlas.routes[1].raw_offsets[0] == span_offsets[0] + 8u &&
                    atlas.routes[1].raw_offsets[1] == span_offsets[0] + 12u &&
                    atlas.routes[1].raw_offsets[2] == span_offsets[0] + 24u &&
                    atlas.routes[1].raw_offsets[3] == span_offsets[0] + 28u &&
                    atlas.routes[1].raw_offsets[4] == span_offsets[0] + 40u &&
                    atlas.routes[1].raw_offsets[5] == span_offsets[0] + 44u &&
                    atlas.routes[1].raw_offsets[6] == span_offsets[0] + 56u &&
                    atlas.routes[1].raw_offsets[7] == span_offsets[0] + 60u &&
                    atlas.routes[1].raw_offsets[8] == span_offsets[2] + 8u &&
                    atlas.routes[1].raw_offsets[9] == span_offsets[2] + 12u &&
                    atlas.routes[1].raw_offsets[10] == span_offsets[2] + 16u &&
                    atlas.routes[1].raw_offsets[13] == span_offsets[2] + 28u &&
                    atlas.routes[2].width == 128u &&
                    atlas.routes[2].raw_offsets[0] == span_offsets[1] &&
                    atlas.routes[2].raw_offsets[7] == span_offsets[1] + 28u &&
                    atlas.routes[2].raw_offsets[8] == span_offsets[1] + 32u &&
                    atlas.routes[2].raw_offsets[11] == span_offsets[1] + 44u &&
                    atlas.routes[2].raw_offsets[12] == span_offsets[1] + 48u &&
                    atlas.routes[2].raw_offsets[15] == span_offsets[1] + 60u &&
                    atlas.routes[3].width == 128u &&
                    atlas.routes[3].raw_offsets[0] == span_offsets[2] &&
                    atlas.routes[3].raw_offsets[7] == span_offsets[2] + 28u &&
                    atlas.routes[3].raw_offsets[8] == span_offsets[2] + 32u &&
                    atlas.routes[3].raw_offsets[11] == span_offsets[2] + 44u &&
                    atlas.routes[3].raw_offsets[12] == span_offsets[2] + 48u &&
                    atlas.routes[3].raw_offsets[15] == span_offsets[2] + 60u &&
                    atlas.total_nonzero_pixel_count > 0u &&
                    atlas.checksum != 0u,
                "Track02 startup bitmap atlas joins decoded tiles into sourced per-route bitmaps");

    theron_v1_startup_media_capture_track02_state_receipt(
        track02,
        track02_size,
        THERON_TRACK02_MD5_US_BIN,
        &receipt);
    for (size_t route_index = 0u;
         route_index < receipt.startup_bitmap_atlas.route_count;
         ++route_index) {
        const Theron_Track02StartupBitmapAtlasRoute *route =
            &receipt.startup_bitmap_atlas.routes[route_index];
        switch (route->route_bit) {
        case THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE:
            title_route = route;
            break;
        case THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE:
            stage_route = route;
            break;
        case THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM:
            soul_route = route;
            break;
        case THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD:
            forcefield_route = route;
            break;
        default:
            break;
        }
    }
    expect_true(receipt.startup_media_ready &&
                    receipt.startup_bitmap_decode_status ==
                        THERON_TRACK02_SIGNAL_OK &&
                    receipt.startup_bitmap_sample_count == 60 &&
                    (receipt.startup_bitmap_route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE) &&
                    (receipt.startup_bitmap_route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE) &&
                    (receipt.startup_bitmap_route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM) &&
                    (receipt.startup_bitmap_route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) &&
                    receipt.startup_bitmap_title_route_ready &&
                    receipt.startup_bitmap_stage_route_ready &&
                    receipt.startup_bitmap_soul_room_route_ready &&
                    receipt.startup_bitmap_forcefield_route_ready &&
                    receipt.startup_bitmap_atlas_ready &&
                    receipt.startup_bitmap_atlas_route_count == 4 &&
                    receipt.startup_bitmap_atlas_route_mask ==
                        (THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
                         THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
                         THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
                         THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) &&
                    receipt.startup_bitmap_atlas_tile_count == 60u &&
                    receipt.startup_bitmap_atlas_nonzero_pixel_count > 0u &&
                    receipt.startup_bitmap_atlas_checksum != 0u &&
                    receipt.startup_bitmap_atlas.route_count == 4u &&
                    receipt.startup_bitmap_atlas.total_tile_count == 60u &&
                    receipt.startup_bitmap_atlas.routes[0].width == 112u &&
                    receipt.startup_bitmap_atlas.routes[2].width == 128u &&
                    receipt.startup_bitmap_atlas.routes[3].width == 128u &&
                    receipt.startup_bitmap_title_atlas_tile_count == 16u &&
                    receipt.startup_bitmap_stage_atlas_tile_count == 16u &&
                    receipt.startup_bitmap_soul_room_atlas_tile_count == 14u &&
                    receipt.startup_bitmap_forcefield_atlas_tile_count == 14u &&
                    receipt.startup_bitmap_title_atlas_width == 128u &&
                    receipt.startup_bitmap_stage_atlas_width == 128u &&
                    receipt.startup_bitmap_soul_room_atlas_width == 112u &&
                    receipt.startup_bitmap_forcefield_atlas_width == 112u &&
                    receipt.startup_bitmap_title_first_raw_offset ==
                        span_offsets[1] &&
                    receipt.startup_bitmap_title_last_raw_offset ==
                        span_offsets[1] + 60u &&
                    title_route != NULL &&
                    title_route->tile_count > 0u &&
                    receipt.startup_bitmap_title_last_user_data_offset ==
                        title_route->user_data_offsets[
                            title_route->tile_count - 1u] &&
                    receipt.startup_bitmap_title_first_user_data_offset <
                        receipt.startup_bitmap_title_last_user_data_offset &&
                    receipt.startup_bitmap_stage_first_raw_offset ==
                        span_offsets[2] &&
                    receipt.startup_bitmap_stage_last_raw_offset ==
                        span_offsets[2] + 60u &&
                    stage_route != NULL &&
                    stage_route->tile_count > 0u &&
                    receipt.startup_bitmap_stage_last_user_data_offset ==
                        stage_route->user_data_offsets[
                            stage_route->tile_count - 1u] &&
                    receipt.startup_bitmap_soul_room_first_raw_offset ==
                        span_offsets[0] &&
                    receipt.startup_bitmap_soul_room_last_raw_offset ==
                        span_offsets[1] + 28u &&
                    soul_route != NULL &&
                    soul_route->tile_count > 0u &&
                    receipt.startup_bitmap_soul_room_last_user_data_offset ==
                        soul_route->user_data_offsets[
                            soul_route->tile_count - 1u] &&
                    receipt.startup_bitmap_forcefield_first_raw_offset ==
                        span_offsets[0] + 8u &&
                    receipt.startup_bitmap_forcefield_last_raw_offset ==
                        span_offsets[2] + 28u &&
                    forcefield_route != NULL &&
                    forcefield_route->tile_count > 0u &&
                    receipt.startup_bitmap_forcefield_last_user_data_offset ==
                        forcefield_route->user_data_offsets[
                            forcefield_route->tile_count - 1u] &&
                    receipt.startup_bitmap_wide_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    receipt.startup_bitmap_wide_route_count == 4 &&
                    receipt.startup_bitmap_wide_atlas_tile_count == 60u &&
                    receipt.startup_bitmap_raw_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    receipt.startup_bitmap_raw_route_count == 4 &&
                    receipt.startup_bitmap_raw_atlas_tile_count == 60u &&
                    receipt.startup_bitmap_iso_route_mask == 0u &&
                    receipt.startup_bitmap_iso_route_count == 0 &&
                    receipt.startup_bitmap_iso_atlas_tile_count == 0u &&
                    receipt.startup_bitmap_title_sample_count == 16 &&
                    receipt.startup_bitmap_stage_sample_count == 16 &&
                    receipt.startup_bitmap_soul_room_sample_count == 14 &&
                    receipt.startup_bitmap_forcefield_sample_count == 14 &&
                    receipt.startup_bitmap_title_nonzero_pixel_count > 0u &&
                    receipt.startup_bitmap_stage_nonzero_pixel_count > 0u &&
                    receipt.startup_bitmap_soul_room_nonzero_pixel_count > 0u &&
                    receipt.startup_bitmap_forcefield_nonzero_pixel_count > 0u &&
                    receipt.startup_bitmap_title_checksum != 0u &&
                    receipt.startup_bitmap_stage_checksum != 0u &&
                    receipt.startup_bitmap_soul_room_checksum != 0u &&
                    receipt.startup_bitmap_forcefield_checksum != 0u &&
                    receipt.startup_bitmap_nonzero_pixel_count > 0u &&
                    receipt.startup_bitmap_checksum != 0u,
                "startup media receipt carries per-route Track02 bitmap decode proof without text/roster fallback");
    theron_v1_startup_receipt_set_placeholder(&startup_receipt);
    theron_v1_startup_receipt_apply_bitmap_art_summary(&startup_receipt,
                                                       &receipt);
    (void)theron_v1_startup_receipt_session_tick(&startup_receipt);
    expect_true(startup_receipt.startup_bitmap_real_routes_complete &&
                    !startup_receipt.startup_bitmap_fallback_routes_allowed &&
                    startup_receipt.startup_bitmap_title_first_raw_offset ==
                        receipt.startup_bitmap_title_first_raw_offset &&
                    startup_receipt.startup_bitmap_title_last_raw_offset ==
                        receipt.startup_bitmap_title_last_raw_offset &&
                    startup_receipt.startup_bitmap_title_first_user_data_offset ==
                        receipt.startup_bitmap_title_first_user_data_offset &&
                    startup_receipt.startup_bitmap_title_last_user_data_offset ==
                        receipt.startup_bitmap_title_last_user_data_offset &&
                    startup_receipt.startup_bitmap_stage_first_raw_offset ==
                        receipt.startup_bitmap_stage_first_raw_offset &&
                    startup_receipt.session_tick_token != 0u,
                "startup receipt carries Track02 bitmap raw/user-data spans for no-fallback host proof");

    free(track02);
}

static void test_track02_startup_bitmap_atlas_overflow_breadth(void) {
    Theron_Track02StartupBitmapCatalog catalog;
    Theron_Track02StartupBitmapAtlas atlas;
    Theron_Track02StartupBitmapAtlas wide_atlas;
    const size_t sample_count = 36u;

    memset(&catalog, 0, sizeof(catalog));
    catalog.variant = THERON_TRACK02_VARIANT_US_BIN;
    catalog.sample_count = sample_count;
    catalog.route_mask = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE;
    for (size_t i = 0u; i < sample_count; ++i) {
        Theron_Track02StartupBitmapSample *sample = &catalog.samples[i];
        sample->route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE;
        sample->raw_offset = 0x2000u + i * 4u;
        sample->user_data_offset = 0x1800u + i * 4u;
        sample->byte_count = THERON_TRACK02_STARTUP_BITMAP_TILE_BYTES;
        sample->width = 8u;
        sample->height = 8u;
        sample->bpp = 4u;
        sample->nonzero_pixel_count = 1u;
        sample->checksum = 0x10000u + (uint32_t)i;
        sample->pixels[i % THERON_TRACK02_STARTUP_BITMAP_PIXELS] =
            (uint8_t)((i % 15u) + 1u);
    }

    expect_true(theron_v1_track02_build_startup_bitmap_atlas(
                    &catalog,
                    &atlas) == THERON_TRACK02_SIGNAL_OK &&
                    atlas.route_count == 1u &&
                    atlas.routes[0].tile_count == 16u &&
                    atlas.routes[0].width == 128u &&
                    atlas.total_tile_count == 16u &&
                    atlas.overflow_count == sample_count - 16u,
                "Track02 legacy startup bitmap atlas reports over-cap decoded samples");
    expect_true(theron_v1_track02_build_startup_bitmap_atlas_wide(
                    &catalog,
                    &wide_atlas) == THERON_TRACK02_SIGNAL_OK &&
                    wide_atlas.route_count == 1u &&
                    wide_atlas.routes[0].tile_count == 32u &&
                    wide_atlas.routes[0].width == 256u &&
                    wide_atlas.promoted_wide_route_mask ==
                        THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE &&
                    wide_atlas.promoted_wide_tile_count == 16u &&
                    wide_atlas.total_tile_count == 32u &&
                    wide_atlas.overflow_count == sample_count - 32u &&
                    wide_atlas.routes[0].raw_offsets[31] ==
                        catalog.samples[31].raw_offset,
                "Track02 wide startup bitmap atlas promotes breadth and keeps residual overflow");
}

static uint32_t test_fnv1a_bytes(const uint8_t *bytes, size_t byte_count) {
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes || byte_count == 0u) {
        return 0u;
    }
    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash ? hash : 2166136261u;
}

static void test_track02_all_dungeon_runtime_capture_receipt(void) {
    static const uint8_t descriptor[18] = {
        0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
        0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
    };
    static const uint32_t progression_seeds[THERON_TRACK02_DUNGEON_COUNT] = {
        313u, 414u, 527u, 632u, 749u, 856u, 967u
    };
    static const size_t descriptor_offsets[3] = {
        0x70be06u, 0x70e2c6u, 0x710904u
    };
    static const size_t post_descriptor_candidate_offsets[3] = {
        (0x70be06u - 0x1584u) + 0x1820u,
        (0x70e2c6u - 0x1584u) + 0x1820u,
        (0x710904u - 0x1584u) + 0x1820u
    };
    static const size_t post_descriptor_candidate_last_offsets[3] = {
        (0x70be06u - 0x1584u) + 0x1c20u,
        (0x70e2c6u - 0x1584u) + 0x1c20u,
        (0x710904u - 0x1584u) + 0x1c20u
    };
    static const size_t span_offsets[3] = {
        0x2d53e0u, 0x47d040u, 0x712840u
    };
    static const uint8_t post_boundary_span[44] = {
        0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
        0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
        0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
        0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
        0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
        0x93, 0x80, 0x00, 0x3f
    };
    const size_t candidate_offset = 0x7015b4u;
    const size_t seed_table_offset = (0x70be06u - 0x1584u) + 0x20u;
    const size_t track02_size =
        ((0x712840u + 92u + THERON_TRACK02_RAW_SECTOR_BYTES - 1u) /
         THERON_TRACK02_RAW_SECTOR_BYTES) *
        THERON_TRACK02_RAW_SECTOR_BYTES;
    uint8_t *track02 = (uint8_t *)calloc(track02_size, 1u);
    uint8_t *track02_positive = NULL;
    uint8_t *track02_inner_positive = NULL;
    uint8_t *track02_level_positive = NULL;
    uint8_t *track02_level_inner_positive = NULL;
    Theron_StartupMediaStateReceipt media_receipt;
    Theron_Track02LevelRouteReceipt level_route_receipt;
    Theron_Track02ObjectTableRouteReceipt object_route_receipt;
    Theron_Track02SemanticBinding descriptor_semantic;
    Theron_V1StartupAllDungeonRouteReceipt receipt;
    Theron_StartupActionPlan plan;
    Theron_V1StartupRuntimeEntryResult runtime_result;
    Theron_V1StartupRuntimeEntryApplyReceipt apply_receipt;
    Theron_StartupStateReceipt state_receipt;
    Theron_StartupHostReceipt host_receipt;
    Theron_V1_World selected_world;
    char runtime_receipt[320];
    Theron_DungeonID dungeon_id;

    expect_true(track02 != NULL,
                "Theron all-dungeon Track02 sparse fixture allocates");
    if (!track02) {
        return;
    }

    for (size_t anchor = 0u; anchor < 3u; ++anchor) {
        memcpy(track02 + descriptor_offsets[anchor],
               descriptor,
               sizeof(descriptor));
        memcpy(track02 + span_offsets[anchor],
               post_boundary_span,
               sizeof(post_boundary_span));
        for (size_t i = sizeof(post_boundary_span); i < 92u; ++i) {
            track02[span_offsets[anchor] + i] =
                (uint8_t)(0x31u + ((anchor * 17u + i * 7u) % 0xcau));
        }
        for (size_t i = 0u; i < 16u; ++i) {
            track02[post_descriptor_candidate_offsets[anchor] + i] =
                (uint8_t)(0x41u + anchor + i);
            track02[post_descriptor_candidate_last_offsets[anchor] + i] =
                (uint8_t)(0x61u + anchor + i);
        }
    }
    for (size_t i = 0u; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
        wr32le_test(track02 + seed_table_offset + i * 4u,
                    progression_seeds[i]);
    }
    wr16be_test(track02 + candidate_offset + 0u, 32u);
    wr16be_test(track02 + candidate_offset + 2u, 27u);
    wr32be_test(track02 + candidate_offset + 4u, 0x0108e938u);
    wr16be_test(track02 + candidate_offset + 8u, 0x0026u);
    memset(track02 + candidate_offset + 12u,
           THERON_SQUARE_WALL,
           32u * 27u);
    track02[candidate_offset + 12u + 1u * 32u + 1u] =
        THERON_SQUARE_FLOOR;
    track02[candidate_offset + 12u + 1u * 32u + 2u] =
        THERON_SQUARE_FLOOR;
    track02[candidate_offset + 12u + 2u * 32u + 2u] =
        THERON_SQUARE_EXIT;

    theron_v1_startup_media_capture_track02_state_receipt(
        track02,
        track02_size,
        THERON_TRACK02_MD5_US_BIN,
        &media_receipt);
    expect_true(theron_v1_startup_runtime_capture_all_dungeon_routes(
                    track02,
                    track02_size,
                    THERON_TRACK02_MD5_US_BIN,
                    &media_receipt,
                    &receipt) &&
                    receipt.valid &&
                    receipt.real_data_capture_ready &&
                    receipt.capture_count == THERON_DUNGEON_COUNT &&
                    receipt.semantic_level_count == THERON_DUNGEON_COUNT &&
                    receipt.dungeon_mask ==
                        ((1u << THERON_DUNGEON_COUNT) - 1u) &&
                    receipt.exact_level_semantics_ready &&
                    receipt.exact_object_semantics_ready &&
                    receipt.object_capture_count == THERON_DUNGEON_COUNT &&
                    receipt.object_capture_mask ==
                        ((1u << THERON_DUNGEON_COUNT) - 1u) &&
                    receipt.object_count_total == 0 &&
                    (receipt.no_fallback_semantic_role_mask &
                     (1u << THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE)) &&
                    (receipt.no_fallback_semantic_role_mask &
                     (1u << THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE)) &&
                    (receipt.no_fallback_semantic_role_mask &
                     (1u << THERON_TRACK02_SEMANTIC_LEVEL_GRID_TABLE)) &&
                    receipt.object_table_no_fallback_ready &&
                    receipt.object_table_blocked_anchor_count == 3 &&
                    receipt.object_table_blocked_anchor_mask == 0x07u &&
                    receipt.nonstartup_level_no_fallback_ready &&
                    receipt.nonstartup_level_blocked_anchor_count == 3 &&
                    receipt.nonstartup_level_blocked_anchor_mask == 0x07u &&
                    receipt.startup_level_blocked_anchor_count == 2 &&
                    receipt.startup_level_blocked_anchor_mask == 0x06u &&
                    receipt.object_table_anchor_binding_status[0] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    receipt.startup_level_anchor_status[0] ==
                        THERON_TRACK02_LEVEL_HANDOFF_OK &&
                    receipt.startup_level_anchor_status[1] ==
                        THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL &&
                    receipt.startup_level_anchor_status[2] ==
                        THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL &&
                    receipt.startup_level_anchor_raw_offsets[0] ==
                        candidate_offset &&
                    receipt.startup_level_anchor_user_data_valid[0] &&
                    receipt.startup_level_anchor_width[0] == 32u &&
                    receipt.startup_level_anchor_height[0] == 27u &&
                    receipt.startup_level_anchor_level_index[0] == 0x0026u &&
                    receipt.object_table_route_hash != 0u &&
                    receipt.level_route_hash != 0u &&
                    receipt.object_route_hash != 0u &&
                    receipt.route_hash != 0u,
                "Theron Track02 all seven dungeon selections have inspectable real-data level/object capture");
    expect_true(theron_v1_track02_capture_object_table_route_receipt(
                    track02,
                    track02_size,
                    THERON_TRACK02_MD5_US_BIN,
                    &object_route_receipt) &&
                    object_route_receipt.valid &&
                    object_route_receipt.verified_track02 &&
                    object_route_receipt.descriptor_route_ready &&
                    object_route_receipt.descriptor_anchor_count == 3u &&
                    object_route_receipt.descriptor_anchor_mask == 0x07u &&
                    object_route_receipt.descriptor_entries_bound ==
                        3u * THERON_TRACK02_MAX_DESCRIPTOR_TABLE_ENTRIES &&
                    (object_route_receipt.semantic_role_mask &
                     (1u << THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE)) &&
                    (object_route_receipt.semantic_role_mask &
                     (1u << THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE)) &&
                    (object_route_receipt.semantic_role_mask &
                     (1u << THERON_TRACK02_SEMANTIC_OBJECT_TABLE)) &&
                    object_route_receipt.descriptor_table_semantic_count == 3u &&
                    object_route_receipt.descriptor_table_semantic_anchor_count == 3u &&
                    object_route_receipt.descriptor_table_semantic_anchor_mask == 0x07u &&
                    object_route_receipt.object_table_role_mapped &&
                    object_route_receipt.object_table_candidate_count == 6u &&
                    object_route_receipt.object_table_candidate_anchor_mask == 0x07u &&
                    object_route_receipt.object_table_candidate_anchor_counts[0] == 2u &&
                    object_route_receipt.object_table_candidate_anchor_counts[1] == 2u &&
                    object_route_receipt.object_table_candidate_anchor_counts[2] == 2u &&
                    object_route_receipt.object_table_candidate_entry_index[0] == 6u &&
                    object_route_receipt.object_table_candidate_raw_offsets[0] ==
                        post_descriptor_candidate_offsets[0] &&
                    object_route_receipt.object_table_candidate_user_data_valid[0] &&
                    object_route_receipt.object_table_candidate_user_data_offsets[0] ==
                        0x622ea2u &&
                    object_route_receipt.object_table_candidate_byte_counts[0] ==
                        0x0400u &&
                    object_route_receipt
                            .object_table_candidate_nonzero_byte_counts[0] ==
                        16u &&
                    object_route_receipt.object_table_candidate_hashes[0] ==
                        test_fnv1a_bytes(
                            track02 + post_descriptor_candidate_offsets[0],
                            0x0400u) &&
                    object_route_receipt.object_table_candidate_header_probe_count ==
                        3u &&
                    object_route_receipt.object_table_candidate_startup_header_shape_count ==
                        0u &&
                    object_route_receipt.object_table_candidate_header_width[0] ==
                        0x4142u &&
                    object_route_receipt.object_table_candidate_header_height[0] ==
                        0x4344u &&
                    object_route_receipt.object_table_candidate_header_seed[0] ==
                        0x45464748u &&
                    object_route_receipt.object_table_candidate_header_level_index[0] ==
                        0x494au &&
                    !object_route_receipt.object_table_candidate_startup_header_shaped[0] &&
                    object_route_receipt.object_table_candidate_last_entry_index[0] == 7u &&
                    object_route_receipt.object_table_candidate_last_raw_offsets[0] ==
                        post_descriptor_candidate_last_offsets[0] &&
                    object_route_receipt.object_table_candidate_last_byte_counts[0] ==
                        0x0400u &&
                    object_route_receipt.object_table_candidate_last_hashes[0] ==
                        test_fnv1a_bytes(
                            track02 + post_descriptor_candidate_last_offsets[0],
                            0x0400u) &&
                    object_route_receipt.object_table_candidate_descriptor_delta[0] ==
                        0x29cu &&
                    object_route_receipt.object_table_candidate_after_descriptor[0] &&
                    object_route_receipt.object_table_candidate_entry_role[0] ==
                        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA &&
                    object_route_receipt.object_table_candidate_window_kind[0] ==
                        THERON_TRACK02_DESCRIPTOR_WINDOW_DATA &&
                    object_route_receipt.object_table_blocked_anchor_count == 3u &&
                    object_route_receipt.object_table_blocked_anchor_mask == 0x07u &&
                    object_route_receipt.object_table_anchor_binding_status[0] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    object_route_receipt.object_table_anchor_binding_status[1] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    object_route_receipt.object_table_anchor_binding_status[2] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    object_route_receipt.object_table_anchor_record_count[0] ==
                        0x4241u &&
                    object_route_receipt.object_table_anchor_overflow_count[0] ==
                        (0x4241u - THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS) &&
                    object_route_receipt
                            .object_table_anchor_decoded_byte_count[0] ==
                        2u &&
                    object_route_receipt
                            .object_table_anchor_decoded_nonzero_byte_count[0] ==
                        2u &&
                    object_route_receipt
                            .object_table_anchor_decoded_checksum[0] ==
                        test_fnv1a_bytes(
                            track02 + post_descriptor_candidate_offsets[0],
                            2u) &&
                    object_route_receipt.object_table_row_probe_count == 3u &&
                    object_route_receipt.object_table_row_probe_anchor_counts[0] ==
                        1u &&
                    object_route_receipt.object_table_row_probe_anchor_counts[1] ==
                        1u &&
                    object_route_receipt.object_table_row_probe_anchor_counts[2] ==
                        1u &&
                    object_route_receipt.object_table_row_reject_count == 3u &&
                    object_route_receipt.object_table_row_bad_shape_count == 3u &&
                    object_route_receipt
                            .object_table_row_window_too_small_count == 0u &&
                    object_route_receipt.object_table_row_zero_fill_count == 0u &&
                    object_route_receipt
                            .object_table_row_first_reject_status[0] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    object_route_receipt
                            .object_table_row_first_reject_entry_index[0] ==
                        7u &&
                    object_route_receipt
                            .object_table_row_first_reject_raw_offsets[0] ==
                        post_descriptor_candidate_last_offsets[0] &&
                    object_route_receipt
                            .object_table_row_first_reject_user_data_valid[0] &&
                    object_route_receipt
                            .object_table_row_first_reject_record_counts[0] ==
                        0x6261u &&
                    object_route_receipt
                            .object_table_row_first_reject_overflow_counts[0] ==
                        (0x6261u - THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS) &&
                    object_route_receipt
                            .object_table_row_first_reject_byte_counts[0] ==
                        2u &&
                    object_route_receipt
                            .object_table_row_first_reject_checksums[0] ==
                        test_fnv1a_bytes(
                            track02 + post_descriptor_candidate_last_offsets[0],
                            2u) &&
                    object_route_receipt.object_table_shape_best_score[0] == 2 &&
                    object_route_receipt.object_table_shape_best_status[0] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    object_route_receipt.object_table_shape_best_entry_index[0] ==
                        6u &&
                    object_route_receipt.object_table_shape_best_raw_offsets[0] ==
                        post_descriptor_candidate_offsets[0] &&
                    object_route_receipt
                            .object_table_shape_best_window_offsets[0] == 0u &&
                    object_route_receipt
                            .object_table_shape_best_record_counts[0] ==
                        0x4241u &&
                    object_route_receipt
                            .object_table_shape_best_overflow_counts[0] ==
                        (0x4241u - THERON_TRACK02_OBJECT_TABLE_MAX_RECORDS) &&
                    object_route_receipt.object_table_shape_best_byte_counts[0] ==
                        2u &&
                    object_route_receipt.object_table_shape_best_checksums[0] ==
                        test_fnv1a_bytes(
                            track02 + post_descriptor_candidate_offsets[0],
                            2u) &&
                    object_route_receipt.object_table_inner_scan_probe_count ==
                        189u &&
                    object_route_receipt.object_table_inner_scan_anchor_counts[0] ==
                        63u &&
                    object_route_receipt.object_table_inner_scan_anchor_counts[1] ==
                        63u &&
                    object_route_receipt.object_table_inner_scan_anchor_counts[2] ==
                        63u &&
                    object_route_receipt
                            .object_table_inner_scan_shaped_count == 0u &&
                    object_route_receipt
                            .object_table_inner_scan_shaped_anchor_mask == 0u &&
                    object_route_receipt.object_table_row_shaped_count == 0u &&
                    object_route_receipt.object_table_row_shaped_anchor_mask == 0u &&
                    !object_route_receipt.object_table_decode_ready &&
                    object_route_receipt.blocked_for_missing_real_object_evidence &&
                    !object_route_receipt.fallback_visuals_allowed &&
                    object_route_receipt.route_hash != 0u,
                "Theron Track02 object-table route receipt blocks fallback when real object evidence is missing");
    track02_positive = (uint8_t *)malloc(track02_size);
    expect_true(track02_positive != NULL,
                "Theron Track02 row-shaped object table fixture allocates");
    if (track02_positive) {
        memcpy(track02_positive, track02, track02_size);
        track02_positive[post_descriptor_candidate_last_offsets[0] + 0u] = 1u;
        track02_positive[post_descriptor_candidate_last_offsets[0] + 1u] = 0u;
        track02_positive[post_descriptor_candidate_last_offsets[0] + 2u] = 1u;
        track02_positive[post_descriptor_candidate_last_offsets[0] + 3u] = 2u;
        track02_positive[post_descriptor_candidate_last_offsets[0] + 4u] = 3u;
        track02_positive[post_descriptor_candidate_last_offsets[0] + 5u] = 4u;
        track02_positive[post_descriptor_candidate_last_offsets[0] + 6u] = 5u;
        track02_positive[post_descriptor_candidate_last_offsets[0] + 7u] = 0x80u;
        track02_positive[post_descriptor_candidate_last_offsets[0] + 8u] = 0x34u;
        track02_positive[post_descriptor_candidate_last_offsets[0] + 9u] = 0x12u;
        memset(track02_positive + post_descriptor_candidate_last_offsets[0] + 10u,
               0,
               0x0400u - 10u);
        expect_true(theron_v1_track02_capture_object_table_route_receipt(
                    track02_positive,
                    track02_size,
                    THERON_TRACK02_MD5_US_BIN,
                    &object_route_receipt) &&
                    object_route_receipt.valid &&
                    object_route_receipt.verified_track02 &&
                    object_route_receipt.descriptor_route_ready &&
                    object_route_receipt.object_table_decode_ready &&
                    !object_route_receipt.blocked_for_missing_real_object_evidence &&
                    !object_route_receipt.fallback_visuals_allowed &&
                    object_route_receipt.object_table_row_probe_count == 3u &&
                    object_route_receipt.object_table_row_probe_anchor_counts[0] ==
                        1u &&
                    object_route_receipt.object_table_row_probe_anchor_counts[1] ==
                        1u &&
                    object_route_receipt.object_table_row_probe_anchor_counts[2] ==
                        1u &&
                    object_route_receipt.object_table_row_reject_count == 2u &&
                    object_route_receipt.object_table_row_bad_shape_count == 2u &&
                    object_route_receipt.object_table_row_first_reject_status[0] ==
                        0 &&
                    object_route_receipt
                            .object_table_row_first_reject_status[1] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    object_route_receipt.object_table_row_shaped_count == 1u &&
                    object_route_receipt.object_table_row_shaped_anchor_mask == 0x01u &&
                    object_route_receipt.object_table_row_shaped_anchor_counts[0] == 1u &&
                    object_route_receipt.object_table_row_shaped_entry_index[0] == 7u &&
                    object_route_receipt.object_table_row_shaped_raw_offsets[0] ==
                        post_descriptor_candidate_last_offsets[0] &&
                    object_route_receipt.object_table_row_shaped_user_data_valid[0] &&
                    object_route_receipt.object_table_row_shaped_record_counts[0] == 1u &&
                    object_route_receipt.object_table_row_shaped_byte_counts[0] == 10u &&
                    object_route_receipt.object_table_row_shaped_checksums[0] ==
                        test_fnv1a_bytes(
                            track02_positive +
                                post_descriptor_candidate_last_offsets[0],
                            10u) &&
                    object_route_receipt.object_table_shape_best_score[0] == 5 &&
                    object_route_receipt.object_table_shape_best_status[0] ==
                        THERON_TRACK02_SEMANTIC_BINDING_OK &&
                    object_route_receipt.object_table_shape_best_entry_index[0] ==
                        7u &&
                    object_route_receipt.object_table_shape_best_raw_offsets[0] ==
                        post_descriptor_candidate_last_offsets[0] &&
                    object_route_receipt
                            .object_table_shape_best_window_offsets[0] == 0u &&
                    object_route_receipt
                            .object_table_shape_best_record_counts[0] == 1u &&
                    object_route_receipt.object_table_shape_best_byte_counts[0] ==
                        10u &&
                    object_route_receipt.object_table_blocked_anchor_count == 2u &&
                    object_route_receipt.object_table_blocked_anchor_mask == 0x06u &&
                    object_route_receipt.route_hash != 0u,
                "Theron Track02 object-table route receipt promotes a real row-shaped post-descriptor table");
        free(track02_positive);
    }
    track02_inner_positive = (uint8_t *)malloc(track02_size);
    expect_true(track02_inner_positive != NULL,
                "Theron Track02 inner object table fixture allocates");
    if (track02_inner_positive) {
        const size_t inner_offset = 0x20u;
        const size_t inner_raw =
            post_descriptor_candidate_last_offsets[0] + inner_offset;
        memcpy(track02_inner_positive, track02, track02_size);
        memset(track02_inner_positive +
                   post_descriptor_candidate_last_offsets[0],
               0,
               0x0400u);
        track02_inner_positive[inner_raw + 0u] = 1u;
        track02_inner_positive[inner_raw + 1u] = 0u;
        track02_inner_positive[inner_raw + 2u] = 2u;
        track02_inner_positive[inner_raw + 3u] = 3u;
        track02_inner_positive[inner_raw + 4u] = 4u;
        track02_inner_positive[inner_raw + 5u] = 5u;
        track02_inner_positive[inner_raw + 6u] = 6u;
        track02_inner_positive[inner_raw + 7u] = 0u;
        track02_inner_positive[inner_raw + 8u] = 0x78u;
        track02_inner_positive[inner_raw + 9u] = 0x56u;
        expect_true(theron_v1_track02_capture_object_table_route_receipt(
                        track02_inner_positive,
                        track02_size,
                        THERON_TRACK02_MD5_US_BIN,
                        &object_route_receipt) &&
                        object_route_receipt.valid &&
                        object_route_receipt.verified_track02 &&
                        object_route_receipt.descriptor_route_ready &&
                        !object_route_receipt.object_table_decode_ready &&
                        object_route_receipt
                            .blocked_for_missing_real_object_evidence &&
                        !object_route_receipt.fallback_visuals_allowed &&
                        object_route_receipt.object_table_row_shaped_count ==
                            0u &&
                        object_route_receipt
                                .object_table_inner_scan_shaped_count == 1u &&
                        object_route_receipt
                                .object_table_inner_scan_shaped_anchor_mask ==
                            0x01u &&
                        object_route_receipt
                                .object_table_inner_scan_shaped_entry_index[0] ==
                            7u &&
                        object_route_receipt
                                .object_table_inner_scan_shaped_raw_offsets[0] ==
                            inner_raw &&
                        object_route_receipt
                                .object_table_inner_scan_shaped_window_offsets[0] ==
                            inner_offset &&
                        object_route_receipt
                                .object_table_inner_scan_shaped_user_data_valid[0] &&
                        object_route_receipt
                                .object_table_inner_scan_shaped_record_counts[0] ==
                            1u &&
                        object_route_receipt
                                .object_table_inner_scan_shaped_byte_counts[0] ==
                            10u &&
                        object_route_receipt
                                .object_table_inner_scan_shaped_checksums[0] ==
                            test_fnv1a_bytes(track02_inner_positive + inner_raw,
                                             10u) &&
                        object_route_receipt.object_table_shape_best_score[0] ==
                            5 &&
                        object_route_receipt.object_table_shape_best_status[0] ==
                            THERON_TRACK02_SEMANTIC_BINDING_OK &&
                        object_route_receipt
                                .object_table_shape_best_entry_index[0] ==
                            7u &&
                        object_route_receipt
                                .object_table_shape_best_raw_offsets[0] ==
                            inner_raw &&
                        object_route_receipt
                                .object_table_shape_best_window_offsets[0] ==
                            inner_offset &&
                        object_route_receipt
                                .object_table_shape_best_record_counts[0] ==
                            1u &&
                        object_route_receipt.object_table_blocked_anchor_count ==
                            3u &&
                        object_route_receipt.object_table_blocked_anchor_mask ==
                            0x07u &&
                        object_route_receipt.route_hash != 0u,
                    "Theron Track02 object-table route receipt records inner row-shaped diagnostics without promoting fallback");
        free(track02_inner_positive);
    }
    expect_true(theron_v1_track02_capture_level_route_receipt(
                    track02,
                    track02_size,
                    THERON_TRACK02_MD5_US_BIN,
                    &level_route_receipt) &&
                    level_route_receipt.valid &&
                    level_route_receipt.verified_track02 &&
                    level_route_receipt.descriptor_route_ready &&
                    level_route_receipt.descriptor_anchor_count == 3u &&
                    level_route_receipt.descriptor_anchor_mask == 0x07u &&
                    level_route_receipt.startup_level_route_ready &&
                    level_route_receipt.startup_level_route_count == 1u &&
                    level_route_receipt.startup_level_route_mask == 0x01u &&
                    level_route_receipt.startup_level_blocked_anchor_count == 2u &&
                    level_route_receipt.startup_level_blocked_anchor_mask == 0x06u &&
                    level_route_receipt.startup_level_anchor_status[0] ==
                        THERON_TRACK02_LEVEL_HANDOFF_OK &&
                    level_route_receipt.startup_level_anchor_status[1] ==
                        THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL &&
                    level_route_receipt.startup_level_anchor_status[2] ==
                        THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL &&
                    level_route_receipt.startup_level_anchor_raw_offsets[0] ==
                        candidate_offset &&
                    level_route_receipt.startup_level_anchor_user_data_valid[0] &&
                    level_route_receipt.startup_level_anchor_width[0] == 32u &&
                    level_route_receipt.startup_level_anchor_height[0] == 27u &&
                    level_route_receipt.startup_level_anchor_level_index[0] == 0x0026u &&
                    level_route_receipt.startup_descriptor_offset ==
                        descriptor_offsets[0] &&
                    level_route_receipt.startup_raw_offset ==
                        candidate_offset &&
                    level_route_receipt.startup_user_data_offset_valid &&
                    level_route_receipt.startup_header_width == 32u &&
                    level_route_receipt.startup_header_height == 27u &&
                    level_route_receipt.startup_header_level_index == 0x0026u &&
                    (level_route_receipt.semantic_role_mask &
                     (1u << THERON_TRACK02_SEMANTIC_DUNGEON_SEED_TABLE)) &&
                    (level_route_receipt.semantic_role_mask &
                     (1u << THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE)) &&
                    (level_route_receipt.semantic_role_mask &
                     (1u << THERON_TRACK02_SEMANTIC_LEVEL_GRID_TABLE)) &&
                    level_route_receipt.startup_level_grid_record_ready &&
                    level_route_receipt.startup_level_grid_record_count == 1u &&
                    level_route_receipt.startup_level_grid_descriptor_offset ==
                        descriptor_offsets[0] &&
                    level_route_receipt.startup_level_grid_raw_offset ==
                        candidate_offset &&
                    level_route_receipt.startup_level_grid_user_data_offset_valid &&
                    level_route_receipt.startup_level_grid_user_data_offset ==
                        level_route_receipt.startup_user_data_offset &&
                    level_route_receipt.level_grid_role_mapped &&
                    level_route_receipt.nonstartup_level_candidate_count == 6u &&
                    level_route_receipt.nonstartup_level_candidate_anchor_mask == 0x07u &&
                    level_route_receipt.nonstartup_level_candidate_anchor_counts[0] == 2u &&
                    level_route_receipt.nonstartup_level_candidate_anchor_counts[1] == 2u &&
                    level_route_receipt.nonstartup_level_candidate_anchor_counts[2] == 2u &&
                    level_route_receipt.nonstartup_level_candidate_entry_index[0] == 6u &&
                    level_route_receipt.nonstartup_level_candidate_raw_offsets[0] ==
                        post_descriptor_candidate_offsets[0] &&
                    level_route_receipt.nonstartup_level_candidate_user_data_valid[0] &&
                    level_route_receipt.nonstartup_level_candidate_user_data_offsets[0] ==
                        0x622ea2u &&
                    level_route_receipt.nonstartup_level_candidate_byte_counts[0] ==
                        0x0400u &&
                    level_route_receipt
                            .nonstartup_level_candidate_nonzero_byte_counts[0] ==
                        16u &&
                    level_route_receipt.nonstartup_level_candidate_hashes[0] ==
                        test_fnv1a_bytes(
                            track02 + post_descriptor_candidate_offsets[0],
                            0x0400u) &&
                    level_route_receipt
                            .nonstartup_level_candidate_header_probe_count ==
                        6u &&
                    level_route_receipt
                            .nonstartup_level_candidate_loader_reject_count ==
                        6u &&
                    level_route_receipt
                            .nonstartup_level_candidate_map_status[0] ==
                        THERON_MAP_ERR_INVALID_GRID &&
                    level_route_receipt
                            .nonstartup_level_candidate_header_width[0] ==
                        0x4142u &&
                    level_route_receipt
                            .nonstartup_level_candidate_header_height[0] ==
                        0x4344u &&
                    level_route_receipt
                            .nonstartup_level_candidate_header_seed[0] ==
                        0x45464748u &&
                    level_route_receipt
                            .nonstartup_level_candidate_header_level_index[0] ==
                        0x494au &&
                    level_route_receipt.nonstartup_level_candidate_last_entry_index[0] == 7u &&
                    level_route_receipt.nonstartup_level_candidate_last_raw_offsets[0] ==
                        post_descriptor_candidate_last_offsets[0] &&
                    level_route_receipt.nonstartup_level_candidate_last_byte_counts[0] ==
                        0x0400u &&
                    level_route_receipt
                            .nonstartup_level_candidate_last_hashes[0] ==
                        test_fnv1a_bytes(
                            track02 + post_descriptor_candidate_last_offsets[0],
                            0x0400u) &&
                    level_route_receipt.nonstartup_level_candidate_descriptor_delta[0] ==
                        0x29cu &&
                    level_route_receipt.nonstartup_level_candidate_after_descriptor[0] &&
                    level_route_receipt.nonstartup_level_candidate_entry_role[0] ==
                        THERON_TRACK02_DESCRIPTOR_ENTRY_ROLE_POST_DESCRIPTOR_DATA &&
                    level_route_receipt.nonstartup_level_candidate_window_kind[0] ==
                        THERON_TRACK02_DESCRIPTOR_WINDOW_DATA &&
                    level_route_receipt.nonstartup_level_blocked_anchor_count == 3u &&
                    level_route_receipt.nonstartup_level_blocked_anchor_mask == 0x07u &&
                    level_route_receipt.nonstartup_level_loaded_count == 0u &&
                    level_route_receipt.nonstartup_level_loaded_anchor_mask == 0u &&
                    level_route_receipt.nonstartup_level_inner_scan_probe_count ==
                        348u &&
                    level_route_receipt
                            .nonstartup_level_inner_scan_anchor_counts[0] ==
                        116u &&
                    level_route_receipt
                            .nonstartup_level_inner_scan_anchor_counts[1] ==
                        116u &&
                    level_route_receipt
                            .nonstartup_level_inner_scan_anchor_counts[2] ==
                        116u &&
                    level_route_receipt
                            .nonstartup_level_inner_scan_loaded_count == 0u &&
                    level_route_receipt
                            .nonstartup_level_inner_scan_loaded_anchor_mask ==
                        0u &&
                    !level_route_receipt.nonstartup_level_decode_ready &&
                    level_route_receipt.blocked_for_missing_nonstartup_level_evidence &&
                    !level_route_receipt.fallback_visuals_allowed &&
                    level_route_receipt.route_hash != 0u,
                "Theron Track02 level-route receipt promotes startup level and blocks non-startup fallback");
    track02_level_positive = (uint8_t *)malloc(track02_size);
    expect_true(track02_level_positive != NULL,
                "Theron Track02 non-startup level fixture allocates");
    if (track02_level_positive) {
        memcpy(track02_level_positive, track02, track02_size);
        wr16be_test(track02_level_positive +
                        post_descriptor_candidate_last_offsets[0] + 0u,
                    32u);
        wr16be_test(track02_level_positive +
                        post_descriptor_candidate_last_offsets[0] + 2u,
                    27u);
        wr32be_test(track02_level_positive +
                        post_descriptor_candidate_last_offsets[0] + 4u,
                    0x0108e939u);
        wr16be_test(track02_level_positive +
                        post_descriptor_candidate_last_offsets[0] + 8u,
                    0x0027u);
        memset(track02_level_positive +
                   post_descriptor_candidate_last_offsets[0] + 12u,
               THERON_SQUARE_WALL,
               32u * 27u);
        track02_level_positive[post_descriptor_candidate_last_offsets[0] +
                               12u + 1u * 32u + 1u] =
            THERON_SQUARE_FLOOR;
        track02_level_positive[post_descriptor_candidate_last_offsets[0] +
                               12u + 1u * 32u + 2u] =
            THERON_SQUARE_FLOOR;
        track02_level_positive[post_descriptor_candidate_last_offsets[0] +
                               12u + 2u * 32u + 2u] =
            THERON_SQUARE_EXIT;
        expect_true(theron_v1_track02_capture_level_route_receipt(
                        track02_level_positive,
                        track02_size,
                        THERON_TRACK02_MD5_US_BIN,
                        &level_route_receipt) &&
                        level_route_receipt.valid &&
                        level_route_receipt.verified_track02 &&
                        level_route_receipt.descriptor_route_ready &&
                        level_route_receipt.startup_level_route_ready &&
                        level_route_receipt.nonstartup_level_decode_ready &&
                        level_route_receipt.nonstartup_level_loaded_count == 1u &&
                        level_route_receipt
                                .nonstartup_level_loaded_anchor_mask == 0x01u &&
                        level_route_receipt
                                .nonstartup_level_loaded_anchor_counts[0] ==
                            1u &&
                        level_route_receipt
                                .nonstartup_level_loaded_entry_index[0] ==
                            7u &&
                        level_route_receipt
                                .nonstartup_level_loaded_raw_offsets[0] ==
                            post_descriptor_candidate_last_offsets[0] &&
                        level_route_receipt
                                .nonstartup_level_loaded_user_data_valid[0] &&
                        level_route_receipt.nonstartup_level_loaded_width[0] ==
                            32u &&
                        level_route_receipt.nonstartup_level_loaded_height[0] ==
                            27u &&
                        level_route_receipt.nonstartup_level_loaded_seed[0] ==
                            0x0108e939u &&
                        level_route_receipt
                                .nonstartup_level_loaded_level_index[0] ==
                            0x0027u &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_probe_count ==
                            290u &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_count ==
                            0u &&
                        level_route_receipt
                                .nonstartup_level_candidate_header_probe_count ==
                            6u &&
                        level_route_receipt
                                .nonstartup_level_candidate_loader_reject_count ==
                            5u &&
                        level_route_receipt
                                .nonstartup_level_blocked_anchor_count == 2u &&
                        level_route_receipt
                                .nonstartup_level_blocked_anchor_mask == 0x06u &&
                        level_route_receipt
                            .blocked_for_missing_nonstartup_level_evidence &&
                        !level_route_receipt.fallback_visuals_allowed &&
                        level_route_receipt.route_hash != 0u,
                    "Theron Track02 level-route receipt promotes a non-startup dungeon record when a candidate loads");
        free(track02_level_positive);
    }
    track02_level_inner_positive = (uint8_t *)malloc(track02_size);
    expect_true(track02_level_inner_positive != NULL,
                "Theron Track02 inner non-startup level fixture allocates");
    if (track02_level_inner_positive) {
        const size_t inner_offset = 0x20u;
        const size_t inner_raw =
            post_descriptor_candidate_last_offsets[0] + inner_offset;
        memcpy(track02_level_inner_positive, track02, track02_size);
        memset(track02_level_inner_positive +
                   post_descriptor_candidate_last_offsets[0],
               0,
               0x0400u);
        wr16be_test(track02_level_inner_positive + inner_raw + 0u, 32u);
        wr16be_test(track02_level_inner_positive + inner_raw + 2u, 27u);
        wr32be_test(track02_level_inner_positive + inner_raw + 4u,
                    0x0108e93au);
        wr16be_test(track02_level_inner_positive + inner_raw + 8u, 0x0028u);
        memset(track02_level_inner_positive + inner_raw + 12u,
               THERON_SQUARE_WALL,
               32u * 27u);
        track02_level_inner_positive[inner_raw + 12u + 1u * 32u + 1u] =
            THERON_SQUARE_FLOOR;
        track02_level_inner_positive[inner_raw + 12u + 1u * 32u + 2u] =
            THERON_SQUARE_FLOOR;
        track02_level_inner_positive[inner_raw + 12u + 2u * 32u + 2u] =
            THERON_SQUARE_EXIT;
        expect_true(theron_v1_track02_capture_level_route_receipt(
                        track02_level_inner_positive,
                        track02_size,
                        THERON_TRACK02_MD5_US_BIN,
                        &level_route_receipt) &&
                        level_route_receipt.valid &&
                        level_route_receipt.verified_track02 &&
                        level_route_receipt.descriptor_route_ready &&
                        level_route_receipt.startup_level_route_ready &&
                        level_route_receipt.nonstartup_level_decode_ready &&
                        level_route_receipt.nonstartup_level_loaded_count == 0u &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_count ==
                            1u &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_anchor_mask ==
                            0x01u &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_entry_index[0] ==
                            7u &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_raw_offsets[0] ==
                            inner_raw &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_window_offsets[0] ==
                            inner_offset &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_user_data_valid[0] &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_width[0] ==
                            32u &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_height[0] ==
                            27u &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_seed[0] ==
                            0x0108e93au &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_loaded_level_index[0] ==
                            0x0028u &&
                        level_route_receipt
                                .nonstartup_level_inner_scan_probe_count ==
                            306u &&
                        level_route_receipt
                                .nonstartup_level_blocked_anchor_count == 2u &&
                        level_route_receipt
                                .nonstartup_level_blocked_anchor_mask == 0x06u &&
                        level_route_receipt
                            .blocked_for_missing_nonstartup_level_evidence &&
                        !level_route_receipt.fallback_visuals_allowed &&
                        level_route_receipt.route_hash != 0u,
                    "Theron Track02 level-route receipt promotes an inner non-startup dungeon record when a candidate loads");
        free(track02_level_inner_positive);
    }
    expect_true(theron_v1_track02_bind_semantic_descriptor(
                    track02,
                    track02_size,
                    descriptor_offsets[0],
                    5u,
                    &descriptor_semantic) ==
                    THERON_TRACK02_SEMANTIC_BINDING_OK &&
                    descriptor_semantic.role ==
                        THERON_TRACK02_SEMANTIC_DESCRIPTOR_TABLE &&
                    descriptor_semantic.window_kind ==
                        THERON_TRACK02_DESCRIPTOR_WINDOW_DESCRIPTOR_TABLE &&
                    descriptor_semantic.absolute_offset <= descriptor_offsets[0] &&
                    descriptor_semantic.absolute_offset +
                            descriptor_semantic.byte_count >=
                        descriptor_offsets[0] + 18u,
                "Theron Track02 descriptor-table semantic entry binds without object-table fallback");

    theron_v1_startup_action_plan_init(&plan);
    plan.status_scope = "READY";
    plan.status = "THERON READY";
    theron_v1_world_init(&selected_world);
    memset(runtime_receipt, 0, sizeof(runtime_receipt));
    expect_true(theron_v1_startup_runtime_load_initial_level_with_receipts(
                    &selected_world,
                    track02,
                    track02_size,
                    THERON_TRACK02_MD5_US_BIN,
                    THERON_DUNGEON_1_HALL_OF_RECORDS,
                    &plan,
                    &runtime_result,
                    &apply_receipt,
                    &state_receipt,
                    runtime_receipt,
                    sizeof(runtime_receipt)) &&
                    runtime_result.all_dungeon_real_data_capture_ready &&
                    runtime_result.object_table_no_fallback_ready &&
                    runtime_result.object_table_blocked_anchor_mask == 0x07u &&
                    runtime_result.nonstartup_level_no_fallback_ready &&
                    runtime_result.nonstartup_level_blocked_anchor_mask == 0x07u &&
                    runtime_result.startup_level_blocked_anchor_mask == 0x06u &&
                    runtime_result.object_table_anchor_binding_status[0] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    runtime_result.startup_level_anchor_status[0] ==
                        THERON_TRACK02_LEVEL_HANDOFF_OK &&
                    runtime_result.startup_level_anchor_status[1] ==
                        THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL &&
                    runtime_result.startup_level_anchor_raw_offsets[0] ==
                        candidate_offset &&
                    runtime_result.startup_level_anchor_user_data_valid[0] &&
                    runtime_result.startup_level_anchor_width[0] == 32u &&
                    runtime_result.startup_level_anchor_level_index[0] == 0x0026u &&
                    runtime_result.object_table_route_hash != 0u &&
                    runtime_result.level_route_hash != 0u &&
                    apply_receipt.object_table_no_fallback_ready &&
                    apply_receipt.object_table_blocked_anchor_mask == 0x07u &&
                    apply_receipt.nonstartup_level_no_fallback_ready &&
                    apply_receipt.nonstartup_level_blocked_anchor_mask == 0x07u &&
                    apply_receipt.startup_level_blocked_anchor_mask == 0x06u &&
                    apply_receipt.object_table_anchor_binding_status[0] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    apply_receipt.startup_level_anchor_status[0] ==
                        THERON_TRACK02_LEVEL_HANDOFF_OK &&
                    apply_receipt.startup_level_anchor_status[1] ==
                        THERON_TRACK02_LEVEL_HANDOFF_NO_LEVEL &&
                    apply_receipt.startup_level_anchor_raw_offsets[0] ==
                        candidate_offset &&
                    apply_receipt.startup_level_anchor_user_data_valid[0] &&
                    apply_receipt.startup_level_anchor_width[0] == 32u &&
                    apply_receipt.startup_level_anchor_level_index[0] == 0x0026u &&
                    state_receipt.runtime_track02_media_route &&
                    state_receipt.runtime_track02_media_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    state_receipt.runtime_track02_media_checksum ==
                        runtime_result.track02_media.startup_bitmap_atlas_checksum &&
                    state_receipt.runtime_track02_media_title_first_raw_offset ==
                        runtime_result.track02_media.startup_bitmap_title_first_raw_offset &&
                    state_receipt.runtime_track02_media_stage_last_user_data_offset ==
                        runtime_result.track02_media.startup_bitmap_stage_last_user_data_offset &&
                    state_receipt.runtime_track02_media_soul_room_last_raw_offset ==
                        runtime_result.track02_media.startup_bitmap_soul_room_last_raw_offset &&
                    state_receipt.runtime_track02_media_forcefield_first_user_data_offset ==
                        runtime_result.track02_media
                            .startup_bitmap_forcefield_first_user_data_offset &&
                    state_receipt.runtime_object_table_blocked_anchor_mask == 0x07u &&
                    state_receipt.runtime_nonstartup_level_blocked_anchor_mask == 0x07u &&
                    state_receipt.runtime_startup_level_blocked_anchor_mask == 0x06u &&
                    state_receipt.runtime_object_table_anchor_binding_status[0] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    state_receipt.runtime_startup_level_anchor_status[0] ==
                        THERON_TRACK02_LEVEL_HANDOFF_OK &&
                    state_receipt.runtime_startup_level_anchor_raw_offsets[0] ==
                        candidate_offset &&
                    state_receipt.runtime_startup_level_anchor_user_data_valid[0] &&
                    state_receipt.runtime_startup_level_anchor_width[0] == 32u &&
                    state_receipt.runtime_startup_level_anchor_level_index[0] == 0x0026u &&
                    theron_v1_startup_state_receipt_has_complete_track02_bitmap_routes(
                        &state_receipt) &&
                    theron_v1_startup_state_receipt_has_track02_no_fallback_runtime_route(
                        &state_receipt) &&
                    apply_receipt.object_table_route_hash ==
                        runtime_result.object_table_route_hash &&
                    apply_receipt.level_route_hash ==
                        runtime_result.level_route_hash &&
                    strstr(apply_receipt.inspect_detail,
                           "no_fallback_roles=0x") != NULL,
                "Theron runtime/apply receipts propagate Track02 object and non-startup level no-fallback evidence");
    expect_true(theron_v1_startup_host_receipt_from_runtime_entry_apply(
                    &apply_receipt,
                    &host_receipt) &&
                    host_receipt.object_table_blocked_anchor_mask == 0x07u &&
                    host_receipt.nonstartup_level_blocked_anchor_mask == 0x07u &&
                    host_receipt.startup_level_blocked_anchor_mask == 0x06u &&
                    host_receipt.object_table_anchor_binding_status[0] ==
                        THERON_TRACK02_SEMANTIC_BINDING_BAD_SHAPE &&
                    host_receipt.startup_level_anchor_status[0] ==
                        THERON_TRACK02_LEVEL_HANDOFF_OK &&
                    host_receipt.startup_level_anchor_raw_offsets[0] ==
                        candidate_offset &&
                    host_receipt.startup_level_anchor_user_data_valid[0] &&
                    host_receipt.startup_level_anchor_width[0] == 32u &&
                    host_receipt.startup_level_anchor_level_index[0] == 0x0026u,
                "Theron host receipt carries Track02 per-anchor no-fallback evidence");

    for (dungeon_id = THERON_DUNGEON_1_HALL_OF_RECORDS;
         dungeon_id <= THERON_DUNGEON_COUNT;
         dungeon_id = (Theron_DungeonID)((int)dungeon_id + 1)) {
        int receipt_index = (int)dungeon_id - 1;
        expect_true(receipt.object_counts[receipt_index] == 0,
                    "Theron all-dungeon receipt records empty startup object route exactly");
        theron_v1_world_init(&selected_world);
        memset(runtime_receipt, 0, sizeof(runtime_receipt));
        expect_true(theron_v1_startup_runtime_load_initial_level(
                        &selected_world,
                        track02,
                        track02_size,
                        THERON_TRACK02_MD5_US_BIN,
                        dungeon_id,
                        runtime_receipt,
                        sizeof(runtime_receipt)) &&
                        selected_world.current_dungeon == (int)dungeon_id &&
                        selected_world.level_loaded[(int)dungeon_id - 1][0] &&
                        selected_world.object_count ==
                            selected_world.levels[(int)dungeon_id - 1][0]
                                .thing_count &&
                        strstr(runtime_receipt,
                               "Track 02 semantic initial level") != NULL,
                    "Theron selected dungeon loads through selected Track02 semantic route");
    }

    free(track02);
}

static void test_track02_startup_bitmap_decode_iso_receipt(void) {
    static const uint8_t descriptor[18] = {
        0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20, 0x10,
        0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
    };
    static const uint8_t post_boundary_span[44] = {
        0xbe, 0x80, 0xfe, 0x80, 0x34, 0x81, 0x76, 0x81,
        0xd0, 0x81, 0x2a, 0x80, 0x2b, 0x80, 0x38, 0x80,
        0x45, 0x80, 0x52, 0x80, 0x5f, 0x80, 0x6c, 0x80,
        0x79, 0x80, 0x86, 0x80, 0xa0, 0x80, 0xa5, 0x80,
        0xaa, 0x80, 0xaf, 0x80, 0xb4, 0x80, 0xb9, 0x80,
        0x93, 0x80, 0x00, 0x3f
    };
    const size_t extended_iso_bitmap_bytes = 256u;
    const size_t track02_size = 0x3000u + extended_iso_bitmap_bytes;
    uint8_t *track02 = (uint8_t *)calloc(track02_size, 1u);
    Theron_Track02StartupBitmapCatalog catalog;
    Theron_Track02StartupBitmapAtlas atlas;
    Theron_StartupMediaStateReceipt receipt;

    expect_true(track02 != NULL,
                "Track02 startup bitmap sparse ISO fixture allocates");
    if (!track02) {
        return;
    }
    memcpy(track02 + 0x1584u, descriptor, sizeof(descriptor));
    memcpy(track02 + 0x3000u, post_boundary_span, sizeof(post_boundary_span));
    for (size_t i = sizeof(post_boundary_span);
         i < extended_iso_bitmap_bytes;
         ++i) {
        track02[0x3000u + i] = (uint8_t)(0x21u + ((i * 13u) % 0xdeu));
    }

    expect_true(theron_v1_track02_catalog_startup_bitmap_samples(
                    track02,
                    track02_size,
                    THERON_TRACK02_MD5_US_ISO,
                    &catalog) == THERON_TRACK02_SIGNAL_OK &&
                    catalog.variant == THERON_TRACK02_VARIANT_US_ISO &&
                    catalog.sample_count == 48u &&
                    catalog.overflow_count == 0u &&
                    (catalog.route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE) &&
                    (catalog.route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE) &&
                    (catalog.route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM) &&
                    (catalog.route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) &&
                    catalog.samples[0].raw_offset == 0x3000u &&
                    catalog.samples[0].user_data_offset == 0x3000u &&
                    catalog.samples[1].raw_offset == 0x3004u &&
                    catalog.samples[1].user_data_offset == 0x3004u &&
                    catalog.samples[3].raw_offset == 0x300cu &&
                    catalog.samples[3].user_data_offset == 0x300cu &&
                    catalog.samples[4].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) &&
                    catalog.samples[4].user_data_offset ==
                        0x3000u + sizeof(post_boundary_span) &&
                    catalog.samples[11].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 28u &&
                    catalog.samples[19].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 44u &&
                    catalog.samples[20].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 48u &&
                    catalog.samples[20].user_data_offset ==
                        0x3000u + sizeof(post_boundary_span) + 48u &&
                    catalog.samples[23].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 60u &&
                    catalog.samples[31].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 92u &&
                    catalog.samples[32].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 96u &&
                    catalog.samples[35].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 108u &&
                    catalog.samples[36].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 112u &&
                    catalog.samples[39].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 124u &&
                    catalog.samples[40].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 128u &&
                    catalog.samples[43].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 140u &&
                    catalog.samples[44].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 144u &&
                    catalog.samples[47].raw_offset ==
                        0x3000u + sizeof(post_boundary_span) + 156u &&
                    catalog.samples[0].nonzero_pixel_count > 0u &&
                    catalog.samples[1].checksum != 0u,
                "Track02 startup bitmap catalog decodes extended ISO tail startup bitmap samples");
    expect_true(theron_v1_track02_build_startup_bitmap_atlas(
                    &catalog,
                    &atlas) == THERON_TRACK02_SIGNAL_OK &&
                    atlas.route_count == 4u &&
                    atlas.total_tile_count == 48u &&
                    atlas.route_mask ==
                        (THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE |
                         THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE |
                         THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |
                         THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) &&
                    atlas.routes[0].width == 96u &&
                    atlas.routes[0].raw_offsets[0] == 0x3000u &&
                    atlas.routes[0].raw_offsets[1] == 0x3004u &&
                    atlas.routes[0].user_data_offsets[1] == 0x3004u &&
                    atlas.routes[0].raw_offsets[2] ==
                        0x3000u + sizeof(post_boundary_span) + 48u &&
                    atlas.routes[0].raw_offsets[3] ==
                        0x3000u + sizeof(post_boundary_span) + 52u &&
                    atlas.routes[0].raw_offsets[7] ==
                        0x3000u + sizeof(post_boundary_span) + 84u &&
                    atlas.routes[0].raw_offsets[8] ==
                        0x3000u + sizeof(post_boundary_span) + 128u &&
                    atlas.routes[0].raw_offsets[11] ==
                        0x3000u + sizeof(post_boundary_span) + 140u &&
                    atlas.routes[1].width == 96u &&
                    atlas.routes[1].raw_offsets[0] == 0x3008u &&
                    atlas.routes[1].raw_offsets[1] == 0x300cu &&
                    atlas.routes[1].raw_offsets[2] ==
                        0x3000u + sizeof(post_boundary_span) + 56u &&
                    atlas.routes[1].raw_offsets[3] ==
                        0x3000u + sizeof(post_boundary_span) + 60u &&
                    atlas.routes[1].raw_offsets[7] ==
                        0x3000u + sizeof(post_boundary_span) + 92u &&
                    atlas.routes[1].raw_offsets[8] ==
                        0x3000u + sizeof(post_boundary_span) + 144u &&
                    atlas.routes[1].raw_offsets[11] ==
                        0x3000u + sizeof(post_boundary_span) + 156u &&
                    atlas.routes[2].width == 96u &&
                    atlas.routes[2].raw_offsets[0] ==
                        0x3000u + sizeof(post_boundary_span) &&
                    atlas.routes[2].raw_offsets[7] ==
                        0x3000u + sizeof(post_boundary_span) + 28u &&
                    atlas.routes[2].user_data_offsets[7] ==
                        0x3000u + sizeof(post_boundary_span) + 28u &&
                    atlas.routes[2].raw_offsets[8] ==
                        0x3000u + sizeof(post_boundary_span) + 96u &&
                    atlas.routes[2].raw_offsets[11] ==
                        0x3000u + sizeof(post_boundary_span) + 108u &&
                    atlas.routes[3].width == 96u &&
                    atlas.routes[3].raw_offsets[0] ==
                        0x3000u + sizeof(post_boundary_span) + 16u &&
                    atlas.routes[3].raw_offsets[7] ==
                        0x3000u + sizeof(post_boundary_span) + 44u &&
                    atlas.routes[3].raw_offsets[8] ==
                        0x3000u + sizeof(post_boundary_span) + 112u &&
                    atlas.routes[3].raw_offsets[11] ==
                        0x3000u + sizeof(post_boundary_span) + 124u &&
                    atlas.total_nonzero_pixel_count > 0u,
                "Track02 ISO startup bitmap atlas covers sourced title stage and visible routes");

    theron_v1_startup_media_capture_track02_state_receipt(
        track02,
        track02_size,
        THERON_TRACK02_MD5_US_ISO,
        &receipt);
    expect_true(receipt.startup_media_ready &&
                    receipt.startup_bitmap_decode_status ==
                        THERON_TRACK02_SIGNAL_OK &&
                    receipt.startup_bitmap_sample_count == 48 &&
                    receipt.startup_bitmap_atlas_ready &&
                    receipt.startup_bitmap_atlas_route_count == 4 &&
                    receipt.startup_bitmap_atlas_tile_count == 48u &&
                    receipt.startup_bitmap_atlas_nonzero_pixel_count > 0u &&
                    receipt.startup_bitmap_atlas.route_count == 4u &&
                    receipt.startup_bitmap_atlas.total_tile_count == 48u &&
                    receipt.startup_bitmap_title_atlas_tile_count == 12u &&
                    receipt.startup_bitmap_stage_atlas_tile_count == 12u &&
                    receipt.startup_bitmap_soul_room_atlas_tile_count == 12u &&
                    receipt.startup_bitmap_forcefield_atlas_tile_count == 12u &&
                    receipt.startup_bitmap_title_atlas_width == 96u &&
                    receipt.startup_bitmap_stage_atlas_width == 96u &&
                    receipt.startup_bitmap_soul_room_atlas_width == 96u &&
                    receipt.startup_bitmap_forcefield_atlas_width == 96u &&
                    receipt.startup_bitmap_wide_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    receipt.startup_bitmap_wide_route_count == 4 &&
                    receipt.startup_bitmap_wide_atlas_tile_count == 48u &&
                    receipt.startup_bitmap_iso_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    receipt.startup_bitmap_iso_route_count == 4 &&
                    receipt.startup_bitmap_iso_atlas_tile_count == 48u &&
                    receipt.startup_bitmap_raw_route_mask == 0u &&
                    receipt.startup_bitmap_raw_route_count == 0 &&
                    receipt.startup_bitmap_raw_atlas_tile_count == 0u &&
                    receipt.startup_bitmap_soul_room_sample_count == 12 &&
                    receipt.startup_bitmap_forcefield_sample_count == 12 &&
                    (receipt.startup_bitmap_route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE) &&
                    (receipt.startup_bitmap_route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE) &&
                    (receipt.startup_bitmap_route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM) &&
                    (receipt.startup_bitmap_route_mask &
                     THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) &&
                    theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
                        &receipt),
                "startup media receipt accepts extended ISO tail bitmaps as complete startup coverage");

    free(track02);
}

static void test_startup_receipt_bitmap_art_gate(void) {
    Theron_V1_StartupReceipt receipt;
    Theron_StartupMediaStateReceipt media_receipt;

    theron_v1_startup_receipt_reset(&receipt);
    theron_v1_startup_media_state_receipt_init(&media_receipt);
    media_receipt.startup_media_ready = 1;
    media_receipt.startup_bitmap_decode_status = THERON_TRACK02_SIGNAL_OK;
    media_receipt.startup_bitmap_sample_count = 32;
    media_receipt.startup_bitmap_route_mask = TST_THERON_FULL_START_BITMAP_ROUTES;
    media_receipt.startup_bitmap_nonzero_pixel_count = 384u;
    media_receipt.startup_bitmap_checksum = 0x711f02u;
    media_receipt.startup_bitmap_title_route_ready = 1;
    media_receipt.startup_bitmap_stage_route_ready = 1;
    media_receipt.startup_bitmap_soul_room_route_ready = 1;
    media_receipt.startup_bitmap_forcefield_route_ready = 1;
    media_receipt.startup_bitmap_atlas_ready = 1;
    media_receipt.startup_bitmap_atlas_route_count = 4;
    media_receipt.startup_bitmap_atlas_route_mask =
        TST_THERON_FULL_START_BITMAP_ROUTES;
    media_receipt.startup_bitmap_atlas_tile_count = 32u;
    media_receipt.startup_bitmap_atlas_nonzero_pixel_count = 384u;
    media_receipt.startup_bitmap_atlas_checksum = 0x916311u;
    media_receipt.startup_bitmap_title_sample_count = 8;
    media_receipt.startup_bitmap_stage_sample_count = 8;
    media_receipt.startup_bitmap_soul_room_sample_count = 8;
    media_receipt.startup_bitmap_forcefield_sample_count = 8;
    media_receipt.startup_bitmap_title_nonzero_pixel_count = 96u;
    media_receipt.startup_bitmap_stage_nonzero_pixel_count = 96u;
    media_receipt.startup_bitmap_soul_room_nonzero_pixel_count = 96u;
    media_receipt.startup_bitmap_forcefield_nonzero_pixel_count = 96u;
    media_receipt.startup_bitmap_title_checksum = 0x7101u;
    media_receipt.startup_bitmap_stage_checksum = 0x7102u;
    media_receipt.startup_bitmap_soul_room_checksum = 0x7104u;
    media_receipt.startup_bitmap_forcefield_checksum = 0x7108u;
    media_receipt.startup_bitmap_title_atlas_tile_count = 8u;
    media_receipt.startup_bitmap_stage_atlas_tile_count = 8u;
    media_receipt.startup_bitmap_soul_room_atlas_tile_count = 8u;
    media_receipt.startup_bitmap_forcefield_atlas_tile_count = 8u;
    media_receipt.startup_bitmap_title_atlas_width = 64u;
    media_receipt.startup_bitmap_stage_atlas_width = 64u;
    media_receipt.startup_bitmap_soul_room_atlas_width = 64u;
    media_receipt.startup_bitmap_forcefield_atlas_width = 64u;
    media_receipt.startup_bitmap_wide_route_mask = 0u;
    media_receipt.startup_bitmap_wide_route_count = 0;
    media_receipt.startup_bitmap_wide_atlas_tile_count = 0u;
    media_receipt.startup_bitmap_raw_route_mask = 0u;
    media_receipt.startup_bitmap_raw_route_count = 0;
    media_receipt.startup_bitmap_raw_atlas_tile_count = 0u;
    media_receipt.startup_bitmap_iso_route_mask = 0u;
    media_receipt.startup_bitmap_iso_route_count = 0;
    media_receipt.startup_bitmap_iso_atlas_tile_count = 0u;

    theron_v1_startup_receipt_apply_bitmap_art_summary(&receipt,
                                                       &media_receipt);
    expect_true(receipt.startup_decoded_art_count == 0u &&
                    receipt.startup_bitmap_fallback_routes_allowed == 1,
                "startup receipt no longer treats base 64px Track02 atlas as complete decoded art");
    expect_true(receipt.startup_bitmap_wide_route_mask == 0u &&
                    receipt.startup_bitmap_wide_route_count == 0u &&
                    receipt.startup_bitmap_wide_atlas_tile_count == 0u &&
                    receipt.startup_bitmap_raw_route_mask == 0u &&
                    receipt.startup_bitmap_iso_route_mask == 0u,
                "startup receipt keeps base 64px atlas separate from wide proof");

    media_receipt.startup_bitmap_title_atlas_tile_count = 12u;
    media_receipt.startup_bitmap_stage_atlas_tile_count = 12u;
    media_receipt.startup_bitmap_soul_room_atlas_tile_count = 12u;
    media_receipt.startup_bitmap_forcefield_atlas_tile_count = 12u;
    media_receipt.startup_bitmap_title_sample_count = 12;
    media_receipt.startup_bitmap_stage_sample_count = 12;
    media_receipt.startup_bitmap_soul_room_sample_count = 12;
    media_receipt.startup_bitmap_forcefield_sample_count = 12;
    media_receipt.startup_bitmap_sample_count = 48;
    media_receipt.startup_bitmap_atlas_tile_count = 48u;
    media_receipt.startup_bitmap_title_atlas_width = 96u;
    media_receipt.startup_bitmap_stage_atlas_width = 96u;
    media_receipt.startup_bitmap_soul_room_atlas_width = 96u;
    media_receipt.startup_bitmap_forcefield_atlas_width = 96u;
    media_receipt.startup_bitmap_wide_route_mask =
        TST_THERON_FULL_START_BITMAP_ROUTES;
    media_receipt.startup_bitmap_wide_route_count = 4;
    media_receipt.startup_bitmap_wide_atlas_tile_count = 48u;
    media_receipt.startup_bitmap_iso_route_mask =
        TST_THERON_FULL_START_BITMAP_ROUTES;
    media_receipt.startup_bitmap_iso_route_count = 4;
    media_receipt.startup_bitmap_iso_atlas_tile_count = 48u;
    theron_v1_startup_receipt_apply_bitmap_art_summary(&receipt,
                                                       &media_receipt);
    expect_true(receipt.startup_bitmap_wide_route_mask ==
                    TST_THERON_FULL_START_BITMAP_ROUTES &&
                    receipt.startup_bitmap_wide_route_count == 4u &&
                    receipt.startup_bitmap_wide_atlas_tile_count == 48u,
                "startup receipt preserves wide Track02 bitmap route counters");
    expect_true(receipt.startup_decoded_art_count ==
                        THERON_STARTUP_HERO_MIRROR_COUNT &&
                    receipt.startup_bitmap_fallback_routes_allowed == 0,
                "startup receipt treats complete wide Track02 bitmap proof as real art");
    expect_true(receipt.startup_bitmap_iso_route_mask ==
                    TST_THERON_FULL_START_BITMAP_ROUTES &&
                    receipt.startup_bitmap_iso_route_count == 4u &&
                    receipt.startup_bitmap_iso_atlas_tile_count == 48u,
                "startup receipt preserves wide Track02 ISO bitmap proof");
    expect_true(receipt.startup_bitmap_wide_route_mask ==
                    TST_THERON_FULL_START_BITMAP_ROUTES &&
                    receipt.startup_bitmap_wide_route_count == 4u &&
                    receipt.startup_bitmap_wide_atlas_tile_count == 48u &&
                    receipt.startup_decoded_art_count ==
                        THERON_STARTUP_HERO_MIRROR_COUNT &&
                    receipt.startup_bitmap_fallback_routes_allowed == 0 &&
                    receipt.startup_bitmap_iso_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    receipt.startup_bitmap_iso_route_count == 4u &&
                    receipt.startup_bitmap_iso_atlas_tile_count == 48u,
                "startup receipt preserves wide Track02 bitmap proof");

    media_receipt.startup_bitmap_atlas_tile_count = 24u;
    media_receipt.startup_bitmap_soul_room_atlas_tile_count = 4u;
    media_receipt.startup_bitmap_soul_room_atlas_width = 32u;
    media_receipt.startup_bitmap_wide_route_mask &=
        ~THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM;
    media_receipt.startup_bitmap_wide_route_count = 3;
    media_receipt.startup_bitmap_wide_atlas_tile_count = 36u;
    media_receipt.startup_bitmap_iso_route_mask &=
        ~THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM;
    media_receipt.startup_bitmap_iso_route_count = 3;
    media_receipt.startup_bitmap_iso_atlas_tile_count = 36u;
    theron_v1_startup_receipt_apply_bitmap_art_summary(&receipt,
                                                       &media_receipt);
    expect_true(receipt.startup_decoded_art_count == 0u,
                "startup receipt gates partial Track02 bitmap atlas as fallback art");
}

static void test_track02_bitmap_span_apply_receipts(void) {
    Theron_StartupMediaStateReceipt media_receipt;
    Theron_StartupActionPlan plan;
    Theron_V1StartupRuntimeEntryResult runtime_result;
    Theron_V1StartupRuntimeEntryApplyReceipt runtime_apply;
    Theron_StartupHostReceipt runtime_host;
    Theron_V1StartupContinueResult continue_result;
    Theron_V1StartupContinueApplyReceipt continue_apply;
    Theron_StartupHostReceipt continue_host;

    theron_v1_startup_media_state_receipt_init(&media_receipt);
    media_receipt.startup_media_ready = 1;
    media_receipt.startup_bitmap_decode_status = THERON_TRACK02_SIGNAL_OK;
    media_receipt.startup_bitmap_route_mask = TST_THERON_FULL_START_BITMAP_ROUTES;
    media_receipt.startup_bitmap_atlas_ready = 1;
    media_receipt.startup_bitmap_atlas_route_mask =
        TST_THERON_FULL_START_BITMAP_ROUTES;
    media_receipt.startup_bitmap_atlas_route_count = 4;
    media_receipt.startup_bitmap_atlas_checksum = 0x45f10042u;
    media_receipt.startup_bitmap_title_first_raw_offset = 0x1000u;
    media_receipt.startup_bitmap_title_last_raw_offset = 0x103cu;
    media_receipt.startup_bitmap_title_first_user_data_offset = 0x0800u;
    media_receipt.startup_bitmap_title_last_user_data_offset = 0x083cu;
    media_receipt.startup_bitmap_stage_first_raw_offset = 0x2000u;
    media_receipt.startup_bitmap_stage_last_raw_offset = 0x203cu;
    media_receipt.startup_bitmap_stage_first_user_data_offset = 0x1800u;
    media_receipt.startup_bitmap_stage_last_user_data_offset = 0x183cu;
    media_receipt.startup_bitmap_soul_room_first_raw_offset = 0x3000u;
    media_receipt.startup_bitmap_soul_room_last_raw_offset = 0x306cu;
    media_receipt.startup_bitmap_soul_room_first_user_data_offset = 0x2800u;
    media_receipt.startup_bitmap_soul_room_last_user_data_offset = 0x286cu;
    media_receipt.startup_bitmap_forcefield_first_raw_offset = 0x4000u;
    media_receipt.startup_bitmap_forcefield_last_raw_offset = 0x406cu;
    media_receipt.startup_bitmap_forcefield_first_user_data_offset = 0x3800u;
    media_receipt.startup_bitmap_forcefield_last_user_data_offset = 0x386cu;

    theron_v1_startup_action_plan_init(&plan);
    plan.status_scope = "READY";
    plan.status = "THERON READY";

    theron_v1_startup_runtime_entry_result_init(&runtime_result);
    runtime_result.result = THERON_STARTUP_OK;
    runtime_result.level_loaded = 1;
    runtime_result.runtime_level_source =
        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_MEDIA;
    runtime_result.track02_media_route = 1;
    runtime_result.track02_media = media_receipt;
    expect_true(theron_v1_startup_runtime_entry_apply_receipt(
                    &plan,
                    &runtime_result,
                    "Track02 media handoff",
                    &runtime_apply) == 1 &&
                    runtime_apply.track02_media_route == 1 &&
                    runtime_apply.track02_media_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    runtime_apply.track02_media_checksum == 0x45f10042u &&
                    runtime_apply.track02_media_title_first_raw_offset ==
                        0x1000u &&
                    runtime_apply.track02_media_stage_last_user_data_offset ==
                        0x183cu &&
                    runtime_apply.track02_media_soul_room_last_raw_offset ==
                        0x306cu &&
                    runtime_apply.track02_media_forcefield_first_user_data_offset ==
                        0x3800u,
                "runtime entry apply receipt carries Track02 bitmap source spans");
    expect_true(theron_v1_startup_host_receipt_from_runtime_entry_apply(
                    &runtime_apply,
                    &runtime_host) == 1 &&
                    runtime_host.track02_media_route == 1 &&
                    runtime_host.track02_media_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    runtime_host.track02_media_checksum == 0x45f10042u &&
                    runtime_host.track02_media_title_first_raw_offset ==
                        0x1000u &&
                    runtime_host.track02_media_stage_last_user_data_offset ==
                        0x183cu &&
                    runtime_host.track02_media_soul_room_last_raw_offset ==
                        0x306cu &&
                    runtime_host.track02_media_forcefield_first_user_data_offset ==
                        0x3800u,
                "runtime host receipt carries Track02 bitmap source spans");

    theron_v1_startup_continue_result_init(&continue_result);
    continue_result.source = THERON_V1_STARTUP_CONTINUE_SOURCE_SRM;
    continue_result.source_slot_index = 2;
    continue_result.track02_media_route = 1;
    continue_result.track02_media = media_receipt;
    expect_true(theron_v1_startup_continue_apply_receipt(
                    &plan,
                    &continue_result,
                    "Continue Track02 media handoff",
                    "chapter marker",
                    &continue_apply) == 1 &&
                    continue_apply.track02_media_route == 1 &&
                    continue_apply.track02_media_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    continue_apply.track02_media_checksum == 0x45f10042u &&
                    continue_apply.track02_media_title_last_user_data_offset ==
                        0x083cu &&
                    continue_apply.track02_media_stage_first_raw_offset ==
                        0x2000u &&
                    continue_apply.track02_media_soul_room_first_user_data_offset ==
                        0x2800u &&
                    continue_apply.track02_media_forcefield_last_raw_offset ==
                        0x406cu,
                "Continue apply receipt carries Track02 bitmap source spans");
    expect_true(theron_v1_startup_host_receipt_from_continue_apply(
                    &continue_apply,
                    &continue_host) == 1 &&
                    continue_host.track02_media_route == 1 &&
                    continue_host.track02_media_route_mask ==
                        TST_THERON_FULL_START_BITMAP_ROUTES &&
                    continue_host.track02_media_checksum == 0x45f10042u &&
                    continue_host.track02_media_title_last_user_data_offset ==
                        0x083cu &&
                    continue_host.track02_media_stage_first_raw_offset ==
                        0x2000u &&
                    continue_host.track02_media_soul_room_first_user_data_offset ==
                        0x2800u &&
                    continue_host.track02_media_forcefield_last_raw_offset ==
                        0x406cu,
                "Continue host receipt carries Track02 bitmap source spans");
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

static void test_runtime_entry_structured_track02_routes(void) {
    static const unsigned char fake_track02[2352] = {0};
    Theron_V1_World world;
    Theron_StartupActionPlan plan;
    Theron_V1StartupRuntimeEntryResult result;
    Theron_V1StartupRuntimeEntryApplyReceipt apply_receipt;
    Theron_StartupStateReceipt state_receipt;
    char receipt[512];

    theron_v1_startup_action_plan_init(&plan);
    plan.status_scope = "READY";
    plan.status = "THERON READY";

    theron_v1_world_init(&world);
    memset(receipt, 0, sizeof(receipt));
    expect_true(theron_v1_startup_runtime_load_initial_level_with_receipts(
                    &world,
                    NULL,
                    0u,
                    NULL,
                    THERON_DUNGEON_1_HALL_OF_RECORDS,
                    &plan,
                    &result,
                    &apply_receipt,
                    &state_receipt,
                    receipt,
                    sizeof(receipt)) &&
                    result.level_loaded &&
                    result.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM &&
                    result.structured_runtime_route &&
                    !result.runtime_receipt_text_route &&
                    apply_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM &&
                    apply_receipt.structured_runtime_route &&
                    !apply_receipt.runtime_receipt_text_route &&
                    state_receipt.set_runtime_level_route &&
                    state_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM &&
                    state_receipt.runtime_structured_route &&
                    !state_receipt.runtime_receipt_text_route,
                "runtime entry fallback route is structured without receipt text parsing");

    theron_v1_world_init(&world);
    memset(receipt, 0, sizeof(receipt));
    expect_true(!theron_v1_startup_runtime_load_initial_level_with_receipts(
                    &world,
                    fake_track02,
                    sizeof(fake_track02),
                    THERON_TRACK02_MD5_US_BIN,
                    THERON_DUNGEON_1_HALL_OF_RECORDS,
                    &plan,
                    &result,
                    &apply_receipt,
                    &state_receipt,
                    receipt,
                    sizeof(receipt)) &&
                    result.result == THERON_STARTUP_ERR_LEVEL_LOAD &&
                    result.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED &&
                    result.fallback_visuals_blocked &&
                    result.structured_runtime_route &&
                    !result.runtime_receipt_text_route &&
                    apply_receipt.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED &&
                    apply_receipt.fallback_visuals_blocked &&
                    apply_receipt.structured_runtime_route &&
                    !apply_receipt.runtime_receipt_text_route &&
                    strstr(apply_receipt.inspect_detail,
                           "structured=1 text_route=0") != NULL &&
                    world.level_loaded[0][0] == 0,
                "runtime entry verified Track02 block route is structured without fallback visuals");

    theron_v1_world_init(&world);
    memset(receipt, 0, sizeof(receipt));
    expect_true(!theron_v1_startup_runtime_load_initial_level_with_receipts(
                    &world,
                    fake_track02,
                    sizeof(fake_track02),
                    THERON_TRACK02_MD5_US_BIN,
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                    &plan,
                    &result,
                    &apply_receipt,
                    &state_receipt,
                    receipt,
                    sizeof(receipt)) &&
                    result.runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED &&
                    result.fallback_visuals_blocked &&
                    world.level_loaded[1][0] == 0,
                "runtime entry blocks verified Track02 later-level fallback route");
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
    test_runtime_entry_structured_track02_routes();
    test_boot_runtime_render_frame_facade();
    test_boot_runtime_release_facade();
    test_startup_session_facts_wrappers();
    test_track02_startup_bitmap_decode_receipt();
    test_track02_startup_bitmap_atlas_overflow_breadth();
    test_track02_all_dungeon_runtime_capture_receipt();
    test_track02_startup_bitmap_decode_iso_receipt();
    test_startup_receipt_bitmap_art_gate();
    test_track02_bitmap_span_apply_receipts();

    printf("=====================================================\n");
    printf("Results: %d/%d passed (failures=%d)\n",
           g_tests_passed, g_tests_run, g_failures);
    printf("=====================================================\n\n");
    return g_failures ? 1 : 0;
}
