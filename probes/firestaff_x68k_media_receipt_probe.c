/*
 * firestaff_x68k_media_receipt_probe.c
 *
 * Skip-safe real-media receipt + classification probe for the
 * DM1 / CSB X68000 HDM media import lane.
 *
 * What this probe verifies (when real X68000 HDMs are present
 * under the user's data tree):
 *
 *   - All six documented DM1 v3.0 / CSB v3.1 Japanese HDMs
 *     (Original / Cracked / Save Disk for each game) are
 *     hash-discoverable through the existing asset scanner
 *     (asset_find_by_md5_list, recursive, ZIP-aware).
 *   - Each one matches the documented MD5 + SHA-256 + size.
 *   - The X68000 media classifier returns the
 *     DMWeb-documented verdict for each kind:
 *       - Original (Not working)  -> UNPROTECTED_PUBLIC_HDM
 *       - Cracked                  -> UNPROTECTED_PUBLIC_HDM
 *       - Save Disk                -> BLANK_SAVE_DISK
 *   - The FTL payload handoff verdict holds: a full-disk HDM
 *     can host an FTL resource of the same size
 *     (ftl_handoff_fits_full_disk == 1).
 *   - The first 32 KiB of the public Original HDMs do not
 *     contain an FTL magic at offset 0 (i.e. these are not
 *     single-resource FTL payloads).
 *
 * Skip-safe by design: when no data_dir is set or no known
 * X68000 HDM is present, the probe exits 0 with a SKIP
 * message. This matches the
 * `firestaff_csb_v1_pc_real_asset_launch_probe` /
 * `csb_v1_hint_oracle_real_htc_scan` pattern.
 *
 * Source of truth:
 *   - dmweb-free.fr/games/dungeon-master/editions/x68000
 *     (DM1 v3.0 JP HDM Original / Cracked / Save Disk)
 *   - dmweb-free.fr/games/chaos-strikes-back/editions/x68000
 *     (CSB v3.1 JP HDM Original / Cracked / Save Disk)
 *   - dmweb-free.fr/community/documentation/copy-protection
 *     "Sharp X68000" section (2DHD geometry + HPR-0007
 *     sentinel + DM / CSB shared protection)
 *   - firestaff_x68k_media_classify.h (synthetic-only
 *     classifier run on every receipted HDM)
 *   - firestaff_x68k_media_receipt.h (known-hash list,
 *     receipt struct, scan + finalize contract)
 *
 * Usage:
 *   probe [data_dir]
 *   Defaults:
 *     data_dir = /Users/bosse/.firestaff/data
 *   Env overrides:
 *     FIRESTAFF_DATA_DIR (overrides default)
 *
 *   The probe exits 0 if every invariant passes, or 0 with
 *   a SKIP message when no known X68k HDM is present.
 */

#include "firestaff_x68k_media_receipt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_checks;
static int g_failures;

#define CHECK(cond, msg) do {                                              \
    ++g_checks;                                                            \
    if (cond) {                                                            \
        printf("  PASS: %s\n", msg);                                       \
    } else {                                                               \
        ++g_failures;                                                      \
        printf("  FAIL: %s\n", msg);                                       \
    }                                                                      \
} while (0)

static const char *data_dir_arg(int argc, char **argv,
                                char *buf, size_t buf_size)
{
    const char *env;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0] != '\0') {
        return argv[1];
    }
    env = getenv("FIRESTAFF_X68K_DATA");
    if (env && env[0] != '\0') {
        return env;
    }
    env = getenv("FIRESTAFF_DATA_DIR");
    if (env && env[0] != '\0') {
        return env;
    }
    home = getenv("HOME");
    if (!home || home[0] == '\0') {
        return NULL;
    }
    snprintf(buf, buf_size, "%s/.firestaff/data", home);
    return buf;
}

static void print_receipt(const FirestaffX68kMediaReceipt *r) {
    char report[1024];
    int n = firestaff_x68k_media_receipt_write_report(
        r, report, sizeof(report));
    if (n > 0) {
        printf("%s", report);
    }
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    size_t known_count = 0u;
    const FirestaffX68kMediaReceipt_KnownHash *known;
    FirestaffX68kMediaReceipt receipts[8];
    size_t present_count = 0u;
    int rc;
    size_t i;
    const char *dir;

    printf("=== DM1/CSB X68000 HDM real-media receipt probe ===\n\n");

    known = firestaff_x68k_media_receipt_known_hashes(&known_count);
    printf("known_hashes=%zu\n", known_count);
    for (i = 0u; i < known_count; ++i) {
        printf("  [%zu] kind=%s  md5=%s  size=%zu  class=%s\n",
               i,
               firestaff_x68k_media_receipt_kind_name(known[i].kind),
               known[i].md5,
               known[i].size_bytes,
               firestaff_x68k_media_receipt_class_name(known[i].expected_class));
    }
    CHECK(known_count == 6u,
          "six DM1/CSB X68000 HDM kinds are registered");
    for (i = 0u; i < known_count; ++i) {
        CHECK(strlen(known[i].md5) == 32u,
              "known hash MD5 is 32 hex chars");
        CHECK(strlen(known[i].sha256) == 64u,
              "known hash SHA-256 is 64 hex chars");
        CHECK(known[i].size_bytes == 1261568u,
              "known hash size matches DMWeb 1232 KB");
    }

    dir = data_dir_arg(argc, argv, default_dir, sizeof(default_dir));
    printf("data_dir=%s\n", dir ? dir : "(none)");

    for (i = 0u; i < known_count; ++i) {
        firestaff_x68k_media_receipt_init(&receipts[i]);
    }

    rc = firestaff_x68k_media_receipt_scan_all(
        dir, NULL, 8, receipts, known_count, &present_count);
    printf("scan_all rc=%d (%s)  present_count=%zu\n",
           rc,
           firestaff_x68k_media_receipt_result_name(rc),
           present_count);

    /* Per-kind summary before deciding on SKIP / FAIL / PASS. */
    for (i = 0u; i < known_count; ++i) {
        const FirestaffX68kMediaReceipt *r = &receipts[i];
        printf("  [scan] kind=%s result=%s present=%d size=%zu "
               "media_class=%s\n",
               firestaff_x68k_media_receipt_kind_name(r->kind),
               firestaff_x68k_media_receipt_result_name(r->result),
               r->present,
               r->actual_size_bytes,
               firestaff_x68k_media_receipt_media_class_name(
                   r->media_class));
    }

    if (rc == FIRESTAFF_X68K_RECEIPT_ERR_NOT_FOUND) {
        printf("SKIP: no known X68000 HDM found under data_dir; "
               "set FIRESTAFF_X68K_DATA to a directory containing "
               "a verified DM1 v3.0 or CSB v3.1 JP HDM to enable "
               "this gate.\n");
        return 0;
    }
    if (rc != FIRESTAFF_X68K_RECEIPT_OK &&
        rc != FIRESTAFF_X68K_RECEIPT_ERR_NOT_FOUND) {
        printf("FAIL: scan_all returned %d (%s); expected OK or "
               "NOT_FOUND.\n", rc,
               firestaff_x68k_media_receipt_result_name(rc));
        return 1;
    }

    CHECK(present_count > 0u,
          "at least one known X68000 HDM is present on this host");
    CHECK(present_count <= known_count,
          "present_count does not exceed the documented list");

    /* Per-receipt invariants. We only enforce the documented
     * invariants for kinds that are actually present; missing
     * kinds are surfaced but not enforced. */
    for (i = 0u; i < known_count; ++i) {
        const FirestaffX68kMediaReceipt *r = &receipts[i];
        if (!r->present) {
            printf("  [skip] kind=%s not present on this host\n",
                   firestaff_x68k_media_receipt_kind_name(r->kind));
            continue;
        }
        printf("--- receipt: %s ---\n",
               firestaff_x68k_media_receipt_kind_name(r->kind));
        print_receipt(r);

        CHECK(r->result == FIRESTAFF_X68K_RECEIPT_OK,
              "receipt result is OK");
        CHECK(r->expected_class_match == 1,
              "receipt expected_class_match is 1");
        CHECK(r->media_class == FIRESTAFF_X68K_MEDIA_FULL_DISK,
              "receipt media_class is full-disk");
        CHECK(r->actual_size_bytes == 1261568u,
              "receipt actual_size is 1232 KB");
        CHECK(strcmp(r->actual_md5, r->expected_md5) == 0,
              "receipt actual MD5 matches expected");
        CHECK(strcmp(r->actual_sha256, r->expected_sha256) == 0,
              "receipt actual SHA-256 matches expected");
        CHECK(r->ftl_handoff_fits_full_disk == 1,
              "FTL handoff fits full-disk (1232 KB)");
        CHECK(r->has_ftl_magic == 0,
              "no FTL magic at offset 0 on a full HDM (Original / "
              "Cracked / Save Disk)");
        /* DMWeb: the public Original and Cracked HDMs lack the
         * HPR-0007 protection-sector sentinel and are not blank
         * save disks. The MFM controller fills the protection-
         * region bytes with 0xE5 ("deleted data" mark), so the
         * PROTECTION_AREA_BLANK flag does NOT fire on real
         * preservation HDMs. We only assert the documented
         * bits the classifier is guaranteed to leave off. */
        if (r->expected_class ==
            FIRESTAFF_X68K_RECEIPT_CLASS_UNPROTECTED_PUBLIC_HDM) {
            CHECK((r->flags & FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT) == 0u,
                  "no HPR-0007 sentinel on a public Original / Cracked");
            CHECK((r->flags & FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK) == 0u,
                  "BLANK_SAVE_DISK is off on a public Original / Cracked");
        }
        if (r->expected_class ==
            FIRESTAFF_X68K_RECEIPT_CLASS_BLANK_SAVE_DISK) {
            CHECK((r->flags & FIRESTAFF_X68K_SCAN_FLAG_BLANK_SAVE_DISK) != 0u,
                  "BLANK_SAVE_DISK fires on a save disk");
            CHECK((r->flags & FIRESTAFF_X68K_SCAN_FLAG_SENTINEL_PRESENT) == 0u,
                  "no HPR-0007 sentinel on a save disk");
        }
    }

    /* Determinism: a second scan produces identical receipts. */
    printf("--- determinism re-scan ---\n");
    {
        FirestaffX68kMediaReceipt receipts2[8];
        size_t present_count2 = 0u;
        for (i = 0u; i < known_count; ++i) {
            firestaff_x68k_media_receipt_init(&receipts2[i]);
        }
        int rc2 = firestaff_x68k_media_receipt_scan_all(
            dir, NULL, 8, receipts2, known_count, &present_count2);
        CHECK(rc2 == rc,
              "second scan returns the same status code");
        CHECK(present_count2 == present_count,
              "second scan matches the same kinds");
        for (i = 0u; i < known_count; ++i) {
            if (!receipts[i].present) continue;
            CHECK(strcmp(receipts[i].actual_md5,
                          receipts2[i].actual_md5) == 0,
                  "MD5 is deterministic across scans");
            CHECK(strcmp(receipts[i].actual_sha256,
                          receipts2[i].actual_sha256) == 0,
                  "SHA-256 is deterministic across scans");
            CHECK(receipts[i].flags == receipts2[i].flags,
                  "classifier flags are deterministic across scans");
        }
    }

    if (g_failures == 0) {
        printf("\nPASS: %d/%d invariants\n", g_checks, g_checks);
        return 0;
    }
    printf("\nFAIL: %d/%d invariants (failures=%d)\n",
           g_checks - g_failures, g_checks, g_failures);
    return 1;
}
