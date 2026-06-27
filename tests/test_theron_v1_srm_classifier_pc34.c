/*
 * test_theron_v1_srm_classifier_pc34.c
 *
 * Theron's Quest V1 — SRM (Save RAM) classifier unit test.
 *
 * Data-free. Exercises the bounded real-artifact boundary for Theron
 * .srm save data:
 *   - default root resolution
 *   - env override
 *   - slot path construction
 *   - manifest classification with synthetic fixtures
 *   - status-name string contract
 *   - missing / corrupt / truncated inputs
 *   - source evidence
 *
 * This is the unit-test counterpart to the
 * firestaff_theron_v1_srm_classifier_probe CTest probe.  Both must
 * pass before any real .srm artifact can be promoted to a launchable
 * Theron save.
 *
 * Source/evidence: docs/DMWEB_REFERENCE.md §6 'Theron's Quest savegame
 * format' and theron_v1_srm_source_evidence().
 */

#include "theron_v1_srm_classifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef FIRESTAFF_HAS_ZLIB
#define FIRESTAFF_HAS_ZLIB 0
#endif

#if defined(_WIN32) || defined(_WIN64)
#define TST_PATH_SEP '\\'
#include <direct.h>
#include <process.h>
#define tst_mkdir(p) _mkdir(p)
#define tst_rmdir(p) _rmdir(p)
#define tst_unlink(p) remove(p)
#define TST_ENV_ROOT "firestaff_theron_srm_unit_env"
#define TST_ABSENT_ROOT "firestaff_theron_srm_unit_no_such_root"
#else
#define TST_PATH_SEP '/'
#include <unistd.h>
#define tst_mkdir(p) mkdir((p), 0700)
#define tst_rmdir(p) rmdir(p)
#define tst_unlink(p) unlink(p)
#define TST_ENV_ROOT "/tmp/firestaff_theron_srm_unit_env"
#define TST_ABSENT_ROOT "/tmp/firestaff_theron_srm_unit_no_such_root"
#endif

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

static int tst_setenv(const char *name, const char *value) {
#if defined(_WIN32) || defined(_WIN64)
    return _putenv_s(name, value ? value : "") == 0;
#else
    if (value) return setenv(name, value, 1) == 0;
    return unsetenv(name) == 0;
#endif
}

static int make_temp_root(char out[THERON_V1_SRM_PATH_MAX]) {
#if defined(_WIN32) || defined(_WIN64)
    int pid = _getpid();
    for (int i = 0; i < 32; i++) {
        int n = snprintf(out, THERON_V1_SRM_PATH_MAX,
                         "firestaff_theron_srm_unit_%d_%d", pid, i);
        if (n <= 0 || n >= THERON_V1_SRM_PATH_MAX) return 0;
        if (tst_mkdir(out) == 0) return 1;
    }
    out[0] = '\0';
    return 0;
#else
    /* POSIX: mkdtemp for portability. */
    static const char *template = "/tmp/firestaff_theron_srm_unit_XXXXXX";
    if (strlen(template) + 1 > THERON_V1_SRM_PATH_MAX) return 0;
    strncpy(out, template, THERON_V1_SRM_PATH_MAX - 1);
    out[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    return mkdtemp(out) != NULL;
#endif
}

static void cleanup_root(const char *root) {
    if (!root || !root[0]) return;
    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; i++) {
        char path[THERON_V1_SRM_PATH_MAX];
        if (theron_v1_srm_slot_path(root, i, path)) {
            tst_unlink(path);
        }
    }
    tst_rmdir(root);
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

static const uint8_t g_valid_gzip_srm[] = {
    0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff,
    0x0b, 0xf1, 0x70, 0x0d, 0xf2, 0xf7, 0xd3, 0x0d, 0x0e, 0xf2,
    0xd5, 0x0d, 0x70, 0x8c, 0xf4, 0xf1, 0x77, 0x74, 0xd1, 0x2d,
    0x33, 0xe4, 0x2a, 0xce, 0xc9, 0x2f, 0xb1, 0x35, 0xe0, 0x02,
    0x00, 0x28, 0x3c, 0x1d, 0xcd, 0x1d, 0x00, 0x00, 0x00
};

static const uint8_t g_valid_gzip_payload[] = "THERON-SRM-PAYLOAD-v1\nslot=0\n";

static int build_synthetic_gzip(uint8_t *out, size_t *out_size) {
    if (!out || !out_size) return 0;
    if (*out_size < sizeof(g_valid_gzip_srm)) return 0;
    memcpy(out, g_valid_gzip_srm, sizeof(g_valid_gzip_srm));
    *out_size = sizeof(g_valid_gzip_srm);
    return 1;
}

static void test_default_root_resolution(void) {
    char root[THERON_V1_SRM_PATH_MAX] = {0};
    int rc = theron_v1_srm_default_root(root);
    expect_true(rc == 1, "default root resolves");
    expect_true(root[0] != '\0', "default root non-empty");
}

static void test_env_override(void) {
    char saved[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had_prev = prev != NULL;
    if (had_prev) {
        strncpy(saved, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    tst_setenv("FIRESTAFF_THERON_SRM_DIR", TST_ENV_ROOT);
    char root[THERON_V1_SRM_PATH_MAX] = {0};
    int rc = theron_v1_srm_default_root(root);
    expect_true(rc == 1, "env override resolves");
    expect_true(strcmp(root, TST_ENV_ROOT) == 0,
                "env override wins over default");
    if (had_prev) {
        tst_setenv("FIRESTAFF_THERON_SRM_DIR", saved);
    } else {
        tst_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void test_slot_path_constructor(void) {
    char path[THERON_V1_SRM_PATH_MAX];
    int rc = theron_v1_srm_slot_path("/tmp/x", 0, path);
    expect_true(rc == 1, "slot 0 path constructs");
    expect_true(strstr(path, "slot0.srm") != NULL, "slot 0 path has slot0.srm");

    rc = theron_v1_srm_slot_path("/tmp/x", THERON_V1_SRM_DISK_SLOT_COUNT - 1, path);
    expect_true(rc == 1, "last slot path constructs");
    expect_true(strstr(path, "slot4.srm") != NULL, "last slot path has slot4.srm");

    /* Out-of-range and empty-root negatives. */
    path[0] = 'X';
    rc = theron_v1_srm_slot_path("/tmp/x", -1, path);
    expect_true(rc == 0, "negative slot index rejected");
    expect_true(path[0] == '\0', "negative slot path is empty");

    path[0] = 'X';
    rc = theron_v1_srm_slot_path("/tmp/x", THERON_V1_SRM_DISK_SLOT_COUNT, path);
    expect_true(rc == 0, "out-of-range slot index rejected");
    expect_true(path[0] == '\0', "out-of-range slot path is empty");

    path[0] = 'X';
    rc = theron_v1_srm_slot_path("", 0, path);
    expect_true(rc == 0, "empty root rejected");
    expect_true(path[0] == '\0', "empty root produces empty path");
}

static void test_status_name_contract(void) {
    expect_true(strcmp(theron_v1_srm_slot_status_name(THERON_V1_SRM_SLOT_ABSENT),
                       "ABSENT") == 0,
                "ABSENT status name");
    expect_true(strcmp(theron_v1_srm_slot_status_name(THERON_V1_SRM_SLOT_UNRECOGNIZED),
                       "UNRECOGNIZED") == 0,
                "UNRECOGNIZED status name");
    expect_true(strcmp(theron_v1_srm_slot_status_name(THERON_V1_SRM_SLOT_MALFORMED),
                       "MALFORMED") == 0,
                "MALFORMED status name");
    expect_true(strcmp(theron_v1_srm_slot_status_name(THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED),
                       "PRESENT_AND_RECOGNIZED") == 0,
                "PRESENT_AND_RECOGNIZED status name");
    expect_true(strcmp(theron_v1_srm_slot_status_name(99), "UNKNOWN") == 0,
                "out-of-enum status name is UNKNOWN");
}

static void test_source_evidence(void) {
    const char *ev = theron_v1_srm_source_evidence();
    expect_true(ev != NULL && strlen(ev) > 50, "source evidence non-empty");
    expect_true(strstr(ev, "DMWEB_REFERENCE") != NULL, "source cites DMWEB_REFERENCE");
    expect_true(strstr(ev, "Sphenx") != NULL, "source cites Sphenx");
    expect_true(strstr(ev, "2026-06-27") != NULL, "source has commit-date marker");
    expect_true(strstr(ev, "gzip-payload probe") != NULL, "source mentions payload probe");
}

static void test_absent_root_is_clean_manifest(void) {
    /* Force a non-existent root via env override so the test is
     * independent of the host's actual data root. */
    char saved[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had_prev = prev != NULL;
    if (had_prev) {
        strncpy(saved, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    tst_setenv("FIRESTAFF_THERON_SRM_DIR", TST_ABSENT_ROOT);

    Theron_V1SrmManifest m;
    memset(&m, 0, sizeof(m));
    int rc = theron_v1_srm_classify_root(NULL, &m);
    expect_true(rc == 1, "absent-root classify returns success");
    expect_true(m.slot_count == THERON_V1_SRM_DISK_SLOT_COUNT,
                "absent-root manifest has 5 slots");
    expect_true(m.present_count == 0, "absent-root manifest present=0");
    expect_true(m.recognized_count == 0, "absent-root manifest recognized=0");
    expect_true(m.root_resolved == 1, "absent-root manifest root_resolved");
    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; i++) {
        char label[64];
        snprintf(label, sizeof(label), "absent slot %d", i);
        expect_true(m.slots[i].status == THERON_V1_SRM_SLOT_ABSENT, label);
    }

    if (had_prev) {
        tst_setenv("FIRESTAFF_THERON_SRM_DIR", saved);
    } else {
        tst_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void test_classify_mixed_fixtures(void) {
    char root[THERON_V1_SRM_PATH_MAX];
    if (!make_temp_root(root)) {
        printf("SKIP: mkdtemp failed\n");
        return;
    }

    /* Slot 0: PRESENT_AND_RECOGNIZED */
    uint8_t gzip_buf[sizeof(g_valid_gzip_srm)];
    size_t gzip_size = sizeof(gzip_buf);
    expect_true(build_synthetic_gzip(gzip_buf, &gzip_size) == 1,
                "synthetic gzip body built");
    char slot0[THERON_V1_SRM_PATH_MAX];
    expect_true(theron_v1_srm_slot_path(root, 0, slot0) == 1,
                "slot 0 path");
    expect_true(write_bytes(slot0, gzip_buf, gzip_size) == 1, "slot 0 written");

    /* Slot 1: UNRECOGNIZED */
    static const uint8_t fake[] = "definitely not gzip";
    char slot1[THERON_V1_SRM_PATH_MAX];
    expect_true(theron_v1_srm_slot_path(root, 1, slot1) == 1,
                "slot 1 path");
    expect_true(write_bytes(slot1, fake, sizeof(fake) - 1u) == 1, "slot 1 written");

    /* Slot 2: MALFORMED (gzip magic but not DEFLATE) */
    uint8_t bad[10] = {0x1F, 0x8B, 0x07, 0, 0, 0, 0, 0, 0, 0xFF};
    char slot2[THERON_V1_SRM_PATH_MAX];
    expect_true(theron_v1_srm_slot_path(root, 2, slot2) == 1,
                "slot 2 path");
    expect_true(write_bytes(slot2, bad, sizeof(bad)) == 1, "slot 2 written");

    /* Slot 3: MALFORMED (truncated below 10-byte gzip header minimum) */
    uint8_t tiny[3] = {0x1F, 0x8B, 0x08};
    char slot3[THERON_V1_SRM_PATH_MAX];
    expect_true(theron_v1_srm_slot_path(root, 3, slot3) == 1,
                "slot 3 path");
    expect_true(write_bytes(slot3, tiny, sizeof(tiny)) == 1, "slot 3 written");

    /* Slot 4: ABSENT */

    Theron_V1SrmManifest m;
    memset(&m, 0, sizeof(m));
    int rc = theron_v1_srm_classify_root(root, &m);
    expect_true(rc == 1, "mixed-fixture classify returns success");
    expect_true(m.present_count == 4, "mixed-fixture present=4");
    expect_true(m.recognized_count == 1, "mixed-fixture recognized=1");
    expect_true(m.slots[0].status == THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED,
                "slot 0 PRESENT_AND_RECOGNIZED");
    expect_true(m.slots[0].gzip_magic_seen == 1, "slot 0 gzip magic seen");
    expect_true(m.slots[0].gzip_deflate_method_seen == 1, "slot 0 deflate seen");
    expect_true(m.slots[0].prefix_checksum32 != 0, "slot 0 checksum non-zero");
    expect_true(m.slots[1].status == THERON_V1_SRM_SLOT_UNRECOGNIZED,
                "slot 1 UNRECOGNIZED");
    expect_true(m.slots[2].status == THERON_V1_SRM_SLOT_MALFORMED,
                "slot 2 MALFORMED (wrong method)");
    expect_true(m.slots[3].status == THERON_V1_SRM_SLOT_MALFORMED,
                "slot 3 MALFORMED (truncated)");
    expect_true(m.slots[4].status == THERON_V1_SRM_SLOT_ABSENT,
                "slot 4 ABSENT");

    cleanup_root(root);
}

static void test_gzip_payload_probe(void) {
    uint8_t payload[128];
    size_t payload_size = 0;
    Theron_V1SrmPayloadProbeStatus status;

    memset(payload, 0, sizeof(payload));
    status = theron_v1_srm_probe_gzip_payload(
        g_valid_gzip_srm,
        sizeof(g_valid_gzip_srm),
        payload,
        sizeof(payload),
        &payload_size);
#if FIRESTAFF_HAS_ZLIB
    expect_true(status == THERON_V1_SRM_PAYLOAD_PROBE_OK,
                "gzip payload probe inflates when zlib is available");
    expect_true(payload_size == sizeof(g_valid_gzip_payload) - 1u,
                "gzip payload size matches fixture");
    expect_true(memcmp(payload, g_valid_gzip_payload, payload_size) == 0,
                "gzip payload bytes match fixture");
#else
    expect_true(status == THERON_V1_SRM_PAYLOAD_PROBE_ZLIB_UNAVAILABLE,
                "gzip payload probe reports zlib unavailable");
#endif

    payload_size = 0;
    status = theron_v1_srm_probe_gzip_payload(
        (const uint8_t *)"not gzip",
        8u,
        payload,
        sizeof(payload),
        &payload_size);
    expect_true(status == THERON_V1_SRM_PAYLOAD_PROBE_BAD_INPUT,
                "short non-gzip rejected as bad input");

    payload_size = 0;
    status = theron_v1_srm_probe_gzip_payload(
        (const uint8_t *)"not gzip but long enough",
        24u,
        payload,
        sizeof(payload),
        &payload_size);
    expect_true(status == THERON_V1_SRM_PAYLOAD_PROBE_NOT_GZIP,
                "long non-gzip rejected as not gzip");

    {
        uint8_t bad_method[10] = {0x1F, 0x8B, 0x07, 0, 0, 0, 0, 0, 0, 0xFF};
        payload_size = 0;
        status = theron_v1_srm_probe_gzip_payload(
            bad_method,
            sizeof(bad_method),
            payload,
            sizeof(payload),
            &payload_size);
        expect_true(status == THERON_V1_SRM_PAYLOAD_PROBE_UNSUPPORTED_METHOD,
                    "non-deflate gzip method rejected");
    }

    expect_true(strcmp(theron_v1_srm_payload_probe_status_name(
                    THERON_V1_SRM_PAYLOAD_PROBE_OK), "OK") == 0,
                "payload status OK name");
    expect_true(strcmp(theron_v1_srm_payload_probe_status_name(99), "UNKNOWN") == 0,
                "payload status unknown name");
}

int main(void) {
    printf("\n=== Theron V1 SRM Classifier Unit Tests ===\n\n");
    test_default_root_resolution();
    test_env_override();
    test_slot_path_constructor();
    test_status_name_contract();
    test_source_evidence();
    test_absent_root_is_clean_manifest();
    test_classify_mixed_fixtures();
    test_gzip_payload_probe();

    printf("=====================================================\n");
    printf("Results: %d/%d passed (failures=%d)\n",
           g_tests_passed, g_tests_run, g_failures);
    printf("=====================================================\n\n");
    return g_failures ? 1 : 0;
}
