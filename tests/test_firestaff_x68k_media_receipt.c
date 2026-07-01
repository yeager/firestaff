/*
 * test_firestaff_x68k_media_receipt.c
 *
 * Data-free test driver for firestaff_x68k_media_receipt.
 *
 * The library owns a SelfTest() that:
 *   - verifies the SHA-256 implementation against the
 *     FIPS 180-4 "abc" test vector
 *   - verifies the MD5 implementation against the
 *     RFC 1321 "abc" test vector
 *   - verifies a 1232 KB zero buffer is hashed
 *     deterministically by both algorithms
 *   - verifies the known-hash table is well-formed
 *   - verifies the receipt finalize() routes a synthetic
 *     blank-save-disk to the documented HASH_MISMATCH
 *     error (since the synthetic zero buffer is not the
 *     real save-disk MD5)
 *   - verifies the report writer produces a multi-line
 *     text buffer that contains the documented fields
 *
 * This file is a thin main() that calls SelfTest and
 * reports PASS / FAIL.
 *
 * Source of truth:
 *   - FIPS 180-4 Secure Hash Standard (SHA-256 "abc" vector)
 *   - RFC 1321 The MD5 Message-Digest Algorithm
 *     (MD5 "abc" vector)
 *   - dmweb-free.fr/games/dungeon-master/editions/x68000
 *     (DM1 v3.0 JP HDM Original / Cracked / Save Disk
 *      boundaries)
 *   - dmweb-free.fr/games/chaos-strikes-back/editions/x68000
 *     (CSB v3.1 JP HDM Original / Cracked / Save Disk
 *      boundaries)
 *   - dmweb-free.fr/community/documentation/copy-protection
 *     "Sharp X68000" section (2DHD geometry, HPR-0007
 *     sentinel, DM / CSB share the same protection scheme)
 *   - firestaff_x68k_media_classify.h (synthetic-only
 *     classifier we run on every receipted HDM)
 *
 * Build (mirrors the existing X68000 unit-test pattern):
 *   cc -std=c99 -I include tests/test_firestaff_x68k_media_receipt.c \
 *      src/shared/firestaff_x68k_media_receipt.c \
 *      src/shared/firestaff_x68k_media_classify.c \
 *      src/shared/firestaff_ftl_container.c \
 *      src/shared/asset_find_by_hash.c \
 *      src/shared/fs_portable_compat.c \
 *      -o test_firestaff_x68k_media_receipt
 */

#include "firestaff_x68k_media_receipt.h"

#include <stdio.h>

int main(void) {
    int rc = firestaff_x68k_media_receipt_self_test();
    if (rc == 0) {
        printf("test_firestaff_x68k_media_receipt: PASS\n");
        return 0;
    }
    printf("test_firestaff_x68k_media_receipt: FAIL\n");
    return 1;
}
