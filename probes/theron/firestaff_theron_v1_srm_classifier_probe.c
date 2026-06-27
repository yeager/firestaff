/*
 * firestaff_theron_v1_srm_classifier_probe.c
 *
 * Theron's Quest V1 — SRM (Save RAM) classifier probe.
 *
 * This probe exercises the bounded real-artifact boundary for Theron
 * .srm save data:
 *   - It is data-free; the default save-disk root on the current host
 *     is empty (no Sphenx-format or other real .srm file is present),
 *     so the default-root branch is expected to report a clean
 *     `present_count=0, recognized_count=0` ABSENT manifest.
 *   - It uses a temporary save-disk root to test the recognition
 *     paths: ABSENT, UNRECOGNIZED (non-gzip body), MALFORMED
 *     (truncated gzip body), and PRESENT_AND_RECOGNIZED (gzipped
 *     deflate body that passes the magic+method check).
 *   - It exercises the public API: default-root resolution, slot
 *     path construction, manifest classification, and the
 *     status-name string contract.
 *
 * The probe is the real-artifact counterpart to the synthetic
 * theron_v1_save_load.c slot-N.tqsv coverage; the two are explicitly
 * kept separate because the underlying save models are different
 * (Save Disk cartridge vs. in-game save format).  See
 * docs/THERON_CAPTURE_READINESS.md for the readiness statement.
 *
 * Source/evidence: see theron_v1_srm_source_evidence() and
 * docs/DMWEB_REFERENCE.md §6 'Theron's Quest savegame format'.
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
#define PROBE_PATH_SEP '\\'
#include <direct.h>
#include <process.h>
#define probe_mkdir(p) _mkdir(p)
#define probe_rmdir(p) _rmdir(p)
#define probe_unlink(p) remove(p)
#define PROBE_ENV_ROOT "firestaff_theron_srm_env"
#define PROBE_EMPTY_ROOT "firestaff_theron_srm_empty_root"
#else
#define PROBE_PATH_SEP '/'
#include <unistd.h>
#define probe_mkdir(p) mkdir((p), 0700)
#define probe_rmdir(p) rmdir(p)
#define probe_unlink(p) unlink(p)
#define PROBE_ENV_ROOT "/tmp/firestaff_theron_srm_env"
#define PROBE_EMPTY_ROOT "/tmp/firestaff_theron_srm_empty_root"
#endif

static int g_fail = 0;
static int g_skip = 0;
static int g_pass = 0;

static void check_int(const char *label, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d want %d\n", label, got, want);
        ++g_fail;
    } else {
        ++g_pass;
    }
}

static void check_size(const char *label, size_t got, size_t want) {
    if (got != want) {
        printf("FAIL %s: got %zu want %zu\n", label, got, want);
        ++g_fail;
    } else {
        ++g_pass;
    }
}

static void check_str(const char *label, const char *got, const char *want) {
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s: got '%s' want '%s'\n", label,
               got ? got : "(null)", want ? want : "(null)");
        ++g_fail;
    } else {
        ++g_pass;
    }
}

static int probe_setenv(const char *name, const char *value) {
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
                         "firestaff_theron_srm_probe_%d_%d", pid, i);
        if (n <= 0 || n >= THERON_V1_SRM_PATH_MAX) return 0;
        if (probe_mkdir(out) == 0) return 1;
    }
    out[0] = '\0';
    return 0;
#else
    /* POSIX: mkdtemp for portability with the existing probes. */
    static const char *base_template = "/tmp/firestaff_theron_srm_probe_XXXXXX";
    if (strlen(base_template) + 1 > THERON_V1_SRM_PATH_MAX) return 0;
    /* mkdtemp mutates the buffer; copy first. */
    strncpy(out, base_template, THERON_V1_SRM_PATH_MAX - 1);
    out[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    return mkdtemp(out) != NULL;
#endif
}

static void remove_dir_recursive(const char *path) {
    /* Best-effort: enumerate slotN.srm files plus the dir itself.
     * The probe keeps the fixture root to exactly 5 named slots so a
     * simple fan-out works. */
    if (!path || !path[0]) return;
    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; i++) {
        char slot_path[THERON_V1_SRM_PATH_MAX];
        if (theron_v1_srm_slot_path(path, i, slot_path)) {
            probe_unlink(slot_path);
        }
    }
    probe_rmdir(path);
}

static int write_slot_file(const char *root, int slot_index,
                            const uint8_t *bytes, size_t size) {
    char path[THERON_V1_SRM_PATH_MAX];
    FILE *fp;
    size_t wrote;

    if (!theron_v1_srm_slot_path(root, slot_index, path)) {
        return 0;
    }
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (size > 0 && bytes) {
        wrote = fwrite(bytes, 1, size, fp);
        if (wrote != size) {
            fclose(fp);
            return 0;
        }
    }
    if (fclose(fp) != 0) return 0;
    return 1;
}

static int file_size(const char *path, uint64_t *out_size) {
    struct stat st;
    if (!path || !out_size) return 0;
    if (stat(path, &st) != 0) return 0;
    *out_size = (uint64_t)st.st_size;
    return 1;
}

/* Synthetic gzip-DEFLATE wrapper for the PRESENT_AND_RECOGNIZED fixture.
 * It carries a tiny Firestaff-only payload, not a real Theron's Quest save
 * body.  The classifier still proves gzip magic + method; the payload probe
 * additionally inflates these bytes when zlib is available. */
static const uint8_t g_valid_gzip_srm[] = {
    0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff,
    0x0b, 0xf1, 0x70, 0x0d, 0xf2, 0xf7, 0xd3, 0x0d, 0x0e, 0xf2,
    0xd5, 0x0d, 0x70, 0x8c, 0xf4, 0xf1, 0x77, 0x74, 0xd1, 0x2d,
    0x33, 0xe4, 0x2a, 0xce, 0xc9, 0x2f, 0xb1, 0x35, 0xe0, 0x02,
    0x00, 0x28, 0x3c, 0x1d, 0xcd, 0x1d, 0x00, 0x00, 0x00
};

static const uint8_t g_valid_gzip_payload[] = "THERON-SRM-PAYLOAD-v1\nslot=0\n";

static int build_synthetic_gzip_body(uint8_t *out, size_t *out_size) {
    if (!out || !out_size) return 0;
    if (*out_size < sizeof(g_valid_gzip_srm)) return 0;
    memcpy(out, g_valid_gzip_srm, sizeof(g_valid_gzip_srm));
    *out_size = sizeof(g_valid_gzip_srm);
    return 1;
}

static void probe_default_root_resolution(void) {
    char root[THERON_V1_SRM_PATH_MAX] = {0};
    int rc = theron_v1_srm_default_root(root);
    check_int("default root resolves", rc, 1);
    check_int("default root non-empty", root[0] != '\0' ? 1 : 0, 1);
    /* FIRESTAFF_THERON_SRM_DIR is set in the test harness; we don't
     * assert on its content here — the dedicated env override test
     * below covers that branch. */
    printf("default root: %s\n", root);
}

static void probe_slot_path_constructor(void) {
    char path[THERON_V1_SRM_PATH_MAX];
    int rc;

    rc = theron_v1_srm_slot_path("/tmp/firestaff_theron_srm_test", 0, path);
    check_int("slot 0 path constructs", rc, 1);
    check_str("slot 0 path uses slot0.srm suffix",
              strstr(path, "slot0.srm") ? "ok" : "missing", "ok");
    check_str("slot 0 path keeps root", strstr(path, "firestaff_theron_srm_test") ? "ok" : "missing", "ok");

    rc = theron_v1_srm_slot_path("/tmp/firestaff_theron_srm_test", 4, path);
    check_int("slot 4 path constructs", rc, 1);
    check_str("slot 4 path uses slot4.srm suffix",
              strstr(path, "slot4.srm") ? "ok" : "missing", "ok");

    /* Out-of-range slot indices must return failure with an empty
     * path so callers cannot accidentally pass the buffer to fopen. */
    path[0] = 'X';
    rc = theron_v1_srm_slot_path("/tmp/firestaff_theron_srm_test", -1, path);
    check_int("negative slot index rejected", rc, 0);
    check_size("negative slot index empties path", strlen(path), 0u);

    path[0] = 'X';
    rc = theron_v1_srm_slot_path("/tmp/firestaff_theron_srm_test",
                                  THERON_V1_SRM_DISK_SLOT_COUNT, path);
    check_int("out-of-range slot index rejected", rc, 0);
    check_size("out-of-range slot index empties path", strlen(path), 0u);

    /* Empty root must also return failure cleanly. */
    path[0] = 'X';
    rc = theron_v1_srm_slot_path("", 0, path);
    check_int("empty root rejected", rc, 0);
    check_size("empty root empties path", strlen(path), 0u);
}

static void probe_env_override_root(void) {
    /* Save and restore the env var so we don't leak it into other
     * tests when the probe is run multiple times. */
    char saved[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had_prev = prev != NULL;
    if (had_prev) {
        strncpy(saved, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    probe_setenv("FIRESTAFF_THERON_SRM_DIR", PROBE_ENV_ROOT);
    char root[THERON_V1_SRM_PATH_MAX] = {0};
    int rc = theron_v1_srm_default_root(root);
    check_int("env override resolves", rc, 1);
    check_str("env override wins", root, PROBE_ENV_ROOT);

    if (had_prev) {
        probe_setenv("FIRESTAFF_THERON_SRM_DIR", saved);
    } else {
        probe_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void probe_default_root_absent_manifest(void) {
    /* If FIRESTAFF_THERON_SRM_DIR is set to a path that does not
     * exist, the classifier should report a clean absent manifest
     * with zero present/recognized counts.  This is the honest
     * outcome when no real .srm data is staged for the probe. */
    char saved[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had_prev = prev != NULL;
    if (had_prev) {
        strncpy(saved, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    probe_setenv("FIRESTAFF_THERON_SRM_DIR", PROBE_EMPTY_ROOT);

    Theron_V1SrmManifest manifest;
    memset(&manifest, 0, sizeof(manifest));
    int rc = theron_v1_srm_classify_root(NULL, &manifest);
    check_int("classify on default root succeeds", rc, 1);
    check_size("classify on default root reports 5 slots",
               (size_t)manifest.slot_count,
               (size_t)THERON_V1_SRM_DISK_SLOT_COUNT);
    check_int("classify on default root reports 0 present",
              manifest.present_count, 0);
    check_int("classify on default root reports 0 recognized",
              manifest.recognized_count, 0);
    check_int("classify on default root reports root_resolved",
              manifest.root_resolved, 1);
    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; i++) {
        char label[64];
        snprintf(label, sizeof(label), "default-root slot %d ABSENT",
                 i);
        check_int(label, manifest.slots[i].status,
                  THERON_V1_SRM_SLOT_ABSENT);
        snprintf(label, sizeof(label), "default-root slot %d size=0", i);
        check_size(label, (size_t)manifest.slots[i].size_bytes, 0u);
    }
    printf("default root classified as: %s\n", manifest.root);

    if (had_prev) {
        probe_setenv("FIRESTAFF_THERON_SRM_DIR", saved);
    } else {
        probe_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void probe_classify_mixed_fixtures(void) {
    char root[THERON_V1_SRM_PATH_MAX];
    if (!make_temp_save_root(root)) {
        printf("SKIP classify mixed fixtures: mkdtemp failed\n");
        ++g_skip;
        return;
    }

    /* Slot 0: PRESENT_AND_RECOGNIZED — synthetic gzip-deflate body. */
    uint8_t gzip_body[sizeof(g_valid_gzip_srm)];
    size_t gzip_size = sizeof(gzip_body);
    if (!build_synthetic_gzip_body(gzip_body, &gzip_size)) {
        printf("FAIL build_synthetic_gzip_body returned false\n");
        ++g_fail;
        remove_dir_recursive(root);
        return;
    }
    if (!write_slot_file(root, 0, gzip_body, gzip_size)) {
        printf("SKIP classify mixed fixtures: could not write gzip body\n");
        ++g_skip;
        remove_dir_recursive(root);
        return;
    }

    /* Slot 1: UNRECOGNIZED — non-gzip body. */
    static const uint8_t fake_body[] = "this is not a gzip stream";
    if (!write_slot_file(root, 1, fake_body, sizeof(fake_body) - 1u)) {
        printf("SKIP classify mixed fixtures: could not write fake body\n");
        ++g_skip;
        remove_dir_recursive(root);
        return;
    }

    /* Slot 2: MALFORMED — gzip magic with non-DEFLATE method. */
    {
        uint8_t bad_gzip[10] = {0x1F, 0x8B, 0x07 /* not DEFLATE */, 0, 0, 0, 0, 0, 0, 0xFF};
        if (!write_slot_file(root, 2, bad_gzip, sizeof(bad_gzip))) {
            printf("SKIP classify mixed fixtures: could not write bad-gzip body\n");
            ++g_skip;
            remove_dir_recursive(root);
            return;
        }
    }

    /* Slot 3: MALFORMED — too short to even contain a gzip header. */
    {
        uint8_t tiny[3] = {0x1F, 0x8B, 0x08};
        if (!write_slot_file(root, 3, tiny, sizeof(tiny))) {
            printf("SKIP classify mixed fixtures: could not write tiny body\n");
            ++g_skip;
            remove_dir_recursive(root);
            return;
        }
    }

    /* Slot 4: left absent. */

    Theron_V1SrmManifest manifest;
    memset(&manifest, 0, sizeof(manifest));
    int rc = theron_v1_srm_classify_root(root, &manifest);
    check_int("mixed-fixture classify succeeds", rc, 1);
    check_int("mixed-fixture present_count=4", manifest.present_count, 4);
    check_int("mixed-fixture recognized_count=1", manifest.recognized_count, 1);
    check_int("slot 0 PRESENT_AND_RECOGNIZED",
              manifest.slots[0].status,
              THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED);
    check_int("slot 0 gzip_magic_seen",
              manifest.slots[0].gzip_magic_seen, 1);
    check_int("slot 0 gzip_deflate_method_seen",
              manifest.slots[0].gzip_deflate_method_seen, 1);
    {
        uint64_t size0 = 0;
        file_size(manifest.slots[0].path, &size0);
        check_size("slot 0 size matches fixture", (size_t)manifest.slots[0].size_bytes,
                   (size_t)size0);
    }
    check_int("slot 0 prefix_checksum32 non-zero",
              manifest.slots[0].prefix_checksum32 != 0, 1);

    check_int("slot 1 UNRECOGNIZED",
              manifest.slots[1].status,
              THERON_V1_SRM_SLOT_UNRECOGNIZED);
    check_int("slot 1 gzip_magic_seen=0",
              manifest.slots[1].gzip_magic_seen, 0);
    check_int("slot 1 gzip_deflate_method_seen=0",
              manifest.slots[1].gzip_deflate_method_seen, 0);

    check_int("slot 2 MALFORMED (non-DEFLATE method)",
              manifest.slots[2].status,
              THERON_V1_SRM_SLOT_MALFORMED);
    check_int("slot 2 gzip_magic_seen=1",
              manifest.slots[2].gzip_magic_seen, 1);
    check_int("slot 2 gzip_deflate_method_seen=0",
              manifest.slots[2].gzip_deflate_method_seen, 0);

    check_int("slot 3 MALFORMED (truncated header)",
              manifest.slots[3].status,
              THERON_V1_SRM_SLOT_MALFORMED);

    check_int("slot 4 ABSENT",
              manifest.slots[4].status,
              THERON_V1_SRM_SLOT_ABSENT);
    check_size("slot 4 size=0",
               (size_t)manifest.slots[4].size_bytes, 0u);

    /* Same checksum contract: identical gzip body in two runs yields
     * the same prefix checksum. */
    Theron_V1SrmManifest manifest_again;
    memset(&manifest_again, 0, sizeof(manifest_again));
    rc = theron_v1_srm_classify_root(root, &manifest_again);
    check_int("classify is deterministic", rc, 1);
    check_int("slot 0 checksum matches across runs",
              manifest.slots[0].prefix_checksum32 ==
                  manifest_again.slots[0].prefix_checksum32,
              1);
    check_int("slot 1 checksum matches across runs",
              manifest.slots[1].prefix_checksum32 ==
                  manifest_again.slots[1].prefix_checksum32,
              1);

    remove_dir_recursive(root);
}

static void probe_status_names_stable(void) {
    /* The string contract is part of the manifest/receipt surface. */
    check_str("ABSENT", theron_v1_srm_slot_status_name(THERON_V1_SRM_SLOT_ABSENT), "ABSENT");
    check_str("UNRECOGNIZED", theron_v1_srm_slot_status_name(THERON_V1_SRM_SLOT_UNRECOGNIZED), "UNRECOGNIZED");
    check_str("MALFORMED", theron_v1_srm_slot_status_name(THERON_V1_SRM_SLOT_MALFORMED), "MALFORMED");
    check_str("PRESENT_AND_RECOGNIZED",
              theron_v1_srm_slot_status_name(THERON_V1_SRM_SLOT_PRESENT_AND_RECOGNIZED),
              "PRESENT_AND_RECOGNIZED");
}

static void probe_source_evidence(void) {
    const char *ev = theron_v1_srm_source_evidence();
    if (!ev || strlen(ev) < 50) {
        printf("FAIL source evidence: too short\n");
        ++g_fail;
        return;
    }
    if (!strstr(ev, "DMWEB_REFERENCE") || !strstr(ev, "Sphenx")) {
        printf("FAIL source evidence: missing required citations\n");
        ++g_fail;
        return;
    }
    if (!strstr(ev, "2026-06-27")) {
        printf("FAIL source evidence: missing commit-date marker\n");
        ++g_fail;
        return;
    }
    if (!strstr(ev, "gzip-payload probe")) {
        printf("FAIL source evidence: missing payload probe note\n");
        ++g_fail;
        return;
    }
    ++g_pass;
}

static void probe_gzip_payload_receipt(void) {
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

    printf("gzip payload probe: status=%s size=%zu zlib=%d\n",
           theron_v1_srm_payload_probe_status_name(status),
           payload_size,
           FIRESTAFF_HAS_ZLIB);

#if FIRESTAFF_HAS_ZLIB
    check_int("gzip payload probe OK", status, THERON_V1_SRM_PAYLOAD_PROBE_OK);
    check_size("gzip payload size",
               payload_size,
               sizeof(g_valid_gzip_payload) - 1u);
    check_int("gzip payload bytes match",
              memcmp(payload, g_valid_gzip_payload, payload_size) == 0,
              1);
#else
    check_int("gzip payload probe zlib unavailable",
              status,
              THERON_V1_SRM_PAYLOAD_PROBE_ZLIB_UNAVAILABLE);
#endif

    payload_size = 0;
    status = theron_v1_srm_probe_gzip_payload(
        (const uint8_t *)"not gzip but long enough",
        24u,
        payload,
        sizeof(payload),
        &payload_size);
    check_int("gzip payload non-gzip rejected",
              status,
              THERON_V1_SRM_PAYLOAD_PROBE_NOT_GZIP);

    {
        uint8_t bad_method[10] = {0x1F, 0x8B, 0x07, 0, 0, 0, 0, 0, 0, 0xFF};
        payload_size = 0;
        status = theron_v1_srm_probe_gzip_payload(
            bad_method,
            sizeof(bad_method),
            payload,
            sizeof(payload),
            &payload_size);
        check_int("gzip payload unsupported method rejected",
                  status,
                  THERON_V1_SRM_PAYLOAD_PROBE_UNSUPPORTED_METHOD);
    }
}

int main(void) {
    printf("=== Theron V1 SRM Classifier Probe ===\n");
    printf("%s\n", theron_v1_srm_source_evidence());

    probe_default_root_resolution();
    probe_env_override_root();
    probe_slot_path_constructor();
    probe_default_root_absent_manifest();
    probe_classify_mixed_fixtures();
    probe_gzip_payload_receipt();
    probe_status_names_stable();
    probe_source_evidence();

    printf("summary: pass=%d fail=%d skip=%d\n", g_pass, g_fail, g_skip);
    return g_fail ? 1 : 0;
}
