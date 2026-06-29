/*
 * test_firestaff_x68k_media_classify.c
 *
 * Test driver for firestaff_x68k_media_classify.c.
 *
 * The library owns a FirestaffX68kMedia_SelfTest() that runs
 * every internal case: geometry constants, empty input,
 * too-small input, single-side / full-disk / oversize size
 * classes, blank save-disk detection, HPR-0007 sentinel
 * detection, off-axis HPR-0007 label classification,
 * nonblank/no-sentinel classification, FTL-payload magic
 * detection, explicit-window FTL candidate counting,
 * FTL handoff fits / overflow, unprotected-disk flag,
 * NULL safety. This file is a thin
 * main() that calls SelfTest and reports PASS / FAIL.
 *
 * Source of truth for the cases:
 *   - dmweb-free.fr/community/documentation/copy-protection,
 *     "Sharp X68000" section: 2 sides x 77 tracks x 8 sectors
 *     x 1024 bytes = 1232 KB; Track 1 Side 1 sector 9 holds
 *     "HPR-0007" + 4 random bytes; DM and CSB share the same
 *     protection scheme.
 *   - dmweb-free.fr/games/dungeon-master/editions/x68000 +
 *     .../chaos-strikes-back/editions/x68000: Japanese v3.0 /
 *     v3.1 HDM original (cannot boot without protection
 *     sectors), cracked image, blank save disk.
 *   - greatstone d_ftl.html "20-byte common header" magic
 *     0x6160 big-endian.
 *
 * Build (mirrors the firestaff_ftl_container / hunk_data
 * unit-test pattern):
 *   cc -std=c99 -I include tests/test_firestaff_x68k_media_classify.c \
 *      src/shared/firestaff_x68k_media_classify.c \
 *      -o test_firestaff_x68k_media_classify
 */

#include "firestaff_x68k_media_classify.h"

#include <stdio.h>

int main(void) {
    int rc = FirestaffX68kMedia_SelfTest();
    if (rc == 0) {
        printf("test_firestaff_x68k_media_classify: PASS\n");
        return 0;
    }
    printf("test_firestaff_x68k_media_classify: FAIL\n");
    return 1;
}
