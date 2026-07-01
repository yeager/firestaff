/*
 * firestaff_theron_v1_startup_save_resume_probe.c
 *
 * Theron's Quest V1 — startup save/resume smoke gate probe.
 *
 * Bounded handoff verification for the Theron V1 boot → save/resume
 * gate:
 *   - On a clean host (no save roots staged), the gate reports
 *     SKIP_SAFE_NO_SAVE_ROOT and NO_RESUME_CLAIM.  This is the
 *     honest CI outcome; the probe surfaces it as PASS so hosts
 *     without real data keep verifying the gate.
 *   - When the FIRESTAFF_THERON_SRM_DIR override points at an
 *     empty directory, the gate reports
 *     SKIP_SAVE_ROOT_PRESENT_NO_SLOTS — also a clean skip.
 *   - When a recognized gzip-deflate .srm is staged, the gate runs
 *     the bounded inflate + FSTQPRG1 progression decode and
 *     surfaces SRM_RESUME_CLAIM with non-zero progression fields.
 *   - The probe verifies the boot profile handoff: an empty
 *     Theron_V1_BootProfile produces a populated snapshot whose
 *     tqsv_root / srm_root are the gate's default resolvers.
 *
 * Source/evidence:
 *   - include/theron_v1_startup_save_resume.h
 *   - theron_v1_startup_save_resume_source_evidence()
 *   - THQUEST.ASM T080  — between-dungeon save/load
 *   - THQUEST.ASM T800  — champion persistence between dungeons
 *   - docs/DMWEB_REFERENCE.md §6 'Theron's Quest savegame format'
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
#define PROBE_PATH_SEP '\\'
#include <direct.h>
#include <process.h>
#define probe_mkdir(p) _mkdir(p)
#define probe_rmdir(p) _rmdir(p)
#define probe_unlink(p) remove(p)
#define PROBE_EMPTY_ROOT "firestaff_theron_startup_save_resume_probe_empty"
#define PROBE_STAGED_ROOT "firestaff_theron_startup_save_resume_probe_staged"
#else
#define PROBE_PATH_SEP '/'
#include <unistd.h>
#define probe_mkdir(p) mkdir((p), 0700)
#define probe_rmdir(p) rmdir(p)
#define probe_unlink(p) unlink(p)
#define PROBE_EMPTY_ROOT "/tmp/firestaff_theron_startup_save_resume_probe_empty"
#define PROBE_STAGED_ROOT "/tmp/firestaff_theron_startup_save_resume_probe_staged"
#endif

static int g_fail = 0;
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

static int make_temp_dir(char out[THERON_V1_SRM_PATH_MAX]) {
#if defined(_WIN32) || defined(_WIN64)
    int pid = _getpid();
    for (int i = 0; i < 32; i++) {
        int n = snprintf(out, THERON_V1_SRM_PATH_MAX,
                         "firestaff_theron_tsr_probe_%d_%d", pid, i);
        if (n <= 0 || n >= THERON_V1_SRM_PATH_MAX) return 0;
        if (probe_mkdir(out) == 0) return 1;
    }
    out[0] = '\0';
    return 0;
#else
    static const char *tpl = "/tmp/firestaff_theron_tsr_probe_XXXXXX";
    if (strlen(tpl) + 1 > THERON_V1_SRM_PATH_MAX) return 0;
    strncpy(out, tpl, THERON_V1_SRM_PATH_MAX - 1);
    out[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    return mkdtemp(out) != NULL;
#endif
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

static void cleanup_srm_root(const char *root) {
    if (!root || !root[0]) return;
    for (int i = 0; i < THERON_V1_SRM_DISK_SLOT_COUNT; i++) {
        char path[THERON_V1_SRM_PATH_MAX];
        if (theron_v1_srm_slot_path(root, i, path)) {
            probe_unlink(path);
        }
    }
    probe_rmdir(root);
}

/* Synthetic gzip-DEFLATE wrapper carrying a valid FSTQPRG1
 * progression envelope.  Same byte sequence as the SRM classifier
 * unit test fixture. */
static const uint8_t g_valid_gzip_srm[] = {
    0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff,
    0x73, 0x0b, 0x0e, 0x09, 0x0c, 0x08, 0x72, 0x37, 0x64, 0x64,
    0x66, 0x66, 0xd4, 0x61, 0x64, 0x60, 0x60, 0x14, 0x60, 0x60,
    0x60, 0x02, 0x62, 0x66, 0x20, 0x66, 0x01, 0x62, 0x56, 0x20,
    0x66, 0x03, 0x62, 0x76, 0x20, 0x06, 0x00, 0x50, 0x8a, 0x0c,
    0xc3, 0x2c, 0x00, 0x00, 0x00
};

static void probe_clean_host_is_skip_safe(void) {
    /* Force both save roots to non-existent paths. */
    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    char saved_home[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev_srm = getenv("FIRESTAFF_THERON_SRM_DIR");
    const char *prev_home = getenv("HOME");
    int had_srm = prev_srm != NULL;
    int had_home = prev_home != NULL;
    if (had_srm) {
        strncpy(saved_srm, prev_srm, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    if (had_home) {
        strncpy(saved_home, prev_home, THERON_V1_SRM_PATH_MAX - 1);
        saved_home[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    probe_setenv("FIRESTAFF_THERON_SRM_DIR", "/tmp/firestaff_theron_tsr_probe_no_such");
    probe_setenv("HOME", "/tmp/firestaff_theron_tsr_probe_no_home");

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    int rc = theron_v1_startup_save_resume_evaluate(NULL, &snap);
    check_int("clean-host evaluate returns 1", rc, 1);
    int skip_verdict =
        snap.verdict == THERON_V1_STARTUP_SKIP_SAFE_NO_SAVE_ROOT ||
        snap.verdict == THERON_V1_STARTUP_SKIP_SAVE_ROOT_PRESENT_NO_SLOTS ||
        snap.verdict == THERON_V1_STARTUP_SKIP_STAGED_NO_RECOGNIZED_SLOT;
    check_int("clean-host verdict is one of SKIP_*", skip_verdict, 1);
    check_int("clean-host resume_claim NONE", snap.resume_claim,
              THERON_V1_STARTUP_RESUME_NONE);
    check_int("clean-host tqsv_total_slots", snap.tqsv_total_slots,
              THERON_SAVE_SLOT_COUNT);
    check_int("clean-host srm_total_slots", snap.srm_total_slots,
              THERON_V1_SRM_DISK_SLOT_COUNT);
    check_int("clean-host tqsv_valid_slots == 0",
              snap.tqsv_valid_slots, 0);
    check_int("clean-host srm_recognized_slots == 0",
              snap.srm_recognized_slots, 0);

    if (had_srm) {
        probe_setenv("FIRESTAFF_THERON_SRM_DIR", saved_srm);
    } else {
        probe_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
    if (had_home) {
        probe_setenv("HOME", saved_home);
    } else {
        probe_setenv("HOME", NULL);
    }
}

static void probe_empty_root_is_skip_safe(void) {
    /* Stage an empty save-disk root so the SRM resolver returns a
     * non-empty path, but no .srm file is present. */
    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had = prev != NULL;
    if (had) {
        strncpy(saved_srm, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    probe_setenv("FIRESTAFF_THERON_SRM_DIR", PROBE_EMPTY_ROOT);
    /* Best-effort: an existing directory from a prior run is still
     * an empty root for our purposes. */
    probe_mkdir(PROBE_EMPTY_ROOT);

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    int rc = theron_v1_startup_save_resume_evaluate(NULL, &snap);
    check_int("empty-root evaluate returns 1", rc, 1);
    check_int("empty-root verdict SKIP_SAVE_ROOT_PRESENT_NO_SLOTS",
              snap.verdict,
              THERON_V1_STARTUP_SKIP_SAVE_ROOT_PRESENT_NO_SLOTS);
    check_int("empty-root resume NONE",
              snap.resume_claim, THERON_V1_STARTUP_RESUME_NONE);
    check_int("empty-root srm_present_slots == 0",
              snap.srm_present_slots, 0);

    probe_rmdir(PROBE_EMPTY_ROOT);
    if (had) {
        probe_setenv("FIRESTAFF_THERON_SRM_DIR", saved_srm);
    } else {
        probe_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void probe_staged_srm_claims_resume(void) {
    /* Stage a recognized .srm and verify the gate reports
     * SRM_RESUME_CLAIM with non-zero bounded decode fields when
     * zlib is available. */
    char staged[THERON_V1_SRM_PATH_MAX];
    if (!make_temp_dir(staged)) {
        printf("SKIP probe_staged_srm: mkdtemp failed\n");
        return;
    }
    char slot_path[THERON_V1_SRM_PATH_MAX];
    int rc = theron_v1_srm_slot_path(staged, 0, slot_path);
    check_int("staged slot 0 path constructs", rc, 1);
    check_int("staged slot 0 written",
              write_bytes(slot_path, g_valid_gzip_srm,
                          sizeof(g_valid_gzip_srm)),
              1);

    char saved_srm[THERON_V1_SRM_PATH_MAX] = {0};
    const char *prev = getenv("FIRESTAFF_THERON_SRM_DIR");
    int had = prev != NULL;
    if (had) {
        strncpy(saved_srm, prev, THERON_V1_SRM_PATH_MAX - 1);
        saved_srm[THERON_V1_SRM_PATH_MAX - 1] = '\0';
    }
    probe_setenv("FIRESTAFF_THERON_SRM_DIR", staged);

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    rc = theron_v1_startup_save_resume_evaluate(NULL, &snap);
    check_int("staged evaluate returns 1", rc, 1);
    check_int("staged verdict SOURCES_LIVE",
              snap.verdict, THERON_V1_STARTUP_SOURCES_LIVE);
    check_int("staged resume_claim SRM",
              snap.resume_claim, THERON_V1_STARTUP_RESUME_SRM);
    check_int("staged srm_recognized_slots == 1",
              snap.srm_recognized_slots, 1);
    check_int("staged srm_first_recognized_slot == 0",
              snap.srm_first_recognized_slot, 0);

#if FIRESTAFF_HAS_ZLIB
    check_int("staged payload probe ran", snap.srm_payload_probe_ran, 1);
    check_int("staged payload probe OK",
              snap.srm_payload_probe_status,
              THERON_V1_SRM_PAYLOAD_PROBE_OK);
    check_int("staged payload hits FSTQPRG1",
              snap.srm_payload_hits_fstq_magic, 1);
    check_int("staged progression import ran",
              snap.srm_progress_import_ran, 1);
    check_int("staged progression import OK",
              snap.srm_progress_import_status,
              THERON_V1_SRM_PROGRESS_IMPORT_OK);
    check_int("staged progression current dungeon 3",
              snap.srm_progress_current_dungeon, 3);
    check_int("staged progression quest mask 0x03",
              snap.srm_progress_quest_mask, 0x03);
#else
    check_int("staged payload probe skipped without zlib",
              snap.srm_payload_probe_ran, 0);
#endif

    cleanup_srm_root(staged);
    if (had) {
        probe_setenv("FIRESTAFF_THERON_SRM_DIR", saved_srm);
    } else {
        probe_setenv("FIRESTAFF_THERON_SRM_DIR", NULL);
    }
}

static void probe_boot_profile_handoff(void) {
    /* Empty boot profile must fall back to the gate's default
     * resolver without erroring.  This is the production path the
     * launcher will exercise. */
    Theron_V1_BootProfile profile;
    memset(&profile, 0, sizeof(profile));
    theron_v1_boot_profile_init(&profile);

    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    int rc = theron_v1_boot_startup_save_resume(&profile, &snap);
    check_int("boot-handoff evaluate returns 1", rc, 1);
    check_int("boot-handoff tqsv_root non-empty",
              snap.tqsv_root[0] != '\0', 1);
    check_int("boot-handoff srm_root non-empty",
              snap.srm_root[0] != '\0', 1);

    /* Set boot->save_root and confirm it propagates. */
    Theron_V1_BootProfile profile2;
    memset(&profile2, 0, sizeof(profile2));
    theron_v1_boot_profile_init(&profile2);
    snprintf(profile2.save_root, sizeof(profile2.save_root), "%s",
             "/tmp/firestaff_theron_tsr_probe_set");
    Theron_V1StartupSaveResume snap2;
    memset(&snap2, 0, sizeof(snap2));
    rc = theron_v1_boot_startup_save_resume(&profile2, &snap2);
    check_int("boot-handoff set-root evaluate returns 1", rc, 1);
    check_str("boot-handoff save_root propagates",
              snap2.tqsv_root,
              "/tmp/firestaff_theron_tsr_probe_set");

    /* NULL safety. */
    Theron_V1StartupSaveResume snap3;
    memset(&snap3, 0, sizeof(snap3));
    rc = theron_v1_boot_startup_save_resume(NULL, &snap3);
    check_int("boot-handoff NULL profile returns 0", rc, 0);
    rc = theron_v1_boot_startup_save_resume(&profile, NULL);
    check_int("boot-handoff NULL snapshot returns 0", rc, 0);
}

static void probe_format_helper(void) {
    Theron_V1StartupSaveResume snap;
    memset(&snap, 0, sizeof(snap));
    char buf[1024];
    size_t n = theron_v1_startup_save_resume_format(&snap, buf, sizeof(buf));
    check_int("format returns positive length", n > 0 ? 1 : 0, 1);
    check_int("format includes header line",
              strstr(buf, "=== Theron V1 Startup Save/Resume") != NULL,
              1);
    check_int("format includes verdict line",
              strstr(buf, "verdict:") != NULL, 1);
    check_int("format includes resume_claim line",
              strstr(buf, "resume_claim:") != NULL, 1);
    /* n is the bytes actually written; the cap is buf_size - 1. */
    check_int("format respects buf_size cap",
              n < sizeof(buf), 1);
}

static void probe_skip_safe_name_contract(void) {
    check_str("SOURCES_LIVE name",
              theron_v1_startup_save_resume_skip_safe_name(
                  THERON_V1_STARTUP_SOURCES_LIVE),
              "SOURCES_LIVE");
    check_str("SKIP_SAFE_NO_SAVE_ROOT name",
              theron_v1_startup_save_resume_skip_safe_name(
                  THERON_V1_STARTUP_SKIP_SAFE_NO_SAVE_ROOT),
              "SKIP_SAFE_NO_SAVE_ROOT");
    check_str("SKIP_SAVE_ROOT_PRESENT_NO_SLOTS name",
              theron_v1_startup_save_resume_skip_safe_name(
                  THERON_V1_STARTUP_SKIP_SAVE_ROOT_PRESENT_NO_SLOTS),
              "SKIP_SAVE_ROOT_PRESENT_NO_SLOTS");
    check_str("SKIP_STAGED_NO_RECOGNIZED_SLOT name",
              theron_v1_startup_save_resume_skip_safe_name(
                  THERON_V1_STARTUP_SKIP_STAGED_NO_RECOGNIZED_SLOT),
              "SKIP_STAGED_NO_RECOGNIZED_SLOT");
    check_str("out-of-enum verdict name",
              theron_v1_startup_save_resume_skip_safe_name(99),
              "UNKNOWN");

    check_str("NO_RESUME_CLAIM name",
              theron_v1_startup_save_resume_claim_name(
                  THERON_V1_STARTUP_RESUME_NONE),
              "NO_RESUME_CLAIM");
    check_str("TQSV_RESUME_CLAIM name",
              theron_v1_startup_save_resume_claim_name(
                  THERON_V1_STARTUP_RESUME_TQSV),
              "TQSV_RESUME_CLAIM");
    check_str("SRM_RESUME_CLAIM name",
              theron_v1_startup_save_resume_claim_name(
                  THERON_V1_STARTUP_RESUME_SRM),
              "SRM_RESUME_CLAIM");
    check_str("DUAL_RESUME_CLAIM name",
              theron_v1_startup_save_resume_claim_name(
                  THERON_V1_STARTUP_RESUME_DUAL),
              "DUAL_RESUME_CLAIM");
    check_str("out-of-enum claim name",
              theron_v1_startup_save_resume_claim_name(99),
              "UNKNOWN");
}

static void probe_source_evidence_string(void) {
    const char *ev = theron_v1_startup_save_resume_source_evidence();
    if (!ev || strlen(ev) < 50) {
        printf("FAIL source_evidence too short\n");
        ++g_fail;
        return;
    }
    if (!strstr(ev, "T080") || !strstr(ev, "T800")) {
        printf("FAIL source_evidence missing THQUEST.ASM T080/T800\n");
        ++g_fail;
        return;
    }
    if (!strstr(ev, "DMWEB_REFERENCE") || !strstr(ev, "Sphenx")) {
        printf("FAIL source_evidence missing required citations\n");
        ++g_fail;
        return;
    }
    if (!strstr(ev, "Does NOT auto-resume")) {
        printf("FAIL source_evidence missing no-auto-resume guard\n");
        ++g_fail;
        return;
    }
    ++g_pass;
}

int main(void) {
    printf("=== Theron V1 Startup Save/Resume Smoke Gate Probe ===\n");
    printf("%s\n", theron_v1_startup_save_resume_source_evidence());

    probe_clean_host_is_skip_safe();
    probe_empty_root_is_skip_safe();
    probe_staged_srm_claims_resume();
    probe_boot_profile_handoff();
    probe_format_helper();
    probe_skip_safe_name_contract();
    probe_source_evidence_string();

    printf("summary: pass=%d fail=%d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
