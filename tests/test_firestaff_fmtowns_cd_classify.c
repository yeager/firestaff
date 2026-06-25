/*
 * test_firestaff_fmtowns_cd_classify.c
 *
 * Test driver for src/shared/firestaff_fmtowns_cd_classify.c.
 *
 * The library exposes FirestaffFmtownsCd_SelfTest() which runs
 * the full internal coverage suite. This driver is a thin main()
 * that calls SelfTest and reports pass/fail to stdout. Build it
 * via CMakeLists.txt as part of the standard CTest matrix.
 *
 * The suite covers (data-free, no game data vendored):
 *   - DM1 v2.0 redump-style 20-track layout (1 data + 19 audio)
 *   - CSB v3.1 redump-style 31-track layout (1 data + 30 audio)
 *   - DM2 v1.0 redump-style 8-track layout (1 data + 7 audio)
 *   - Single-file ISO/CUE MODE1/2048 layout
 *   - All-audio disc rejection
 *   - TRACK-before-FILE rejection
 *   - Unterminated FILE quote rejection
 *   - Single-audio disc returns no game match
 *   - Synthetic ISO 9660 PVD detection in a temp file
 *   - LF-only line endings tolerated
 *   - REM and ';' comments tolerated
 *   - PREGAP / INDEX 00 markers tolerated
 *
 * Build:
 *   cc -std=c99 -I include tests/test_firestaff_fmtowns_cd_classify.c \
 *      src/shared/firestaff_fmtowns_cd_classify.c -o test_firestaff_fmtowns_cd_classify
 */

#include "firestaff_fmtowns_cd_classify.h"

#include <stdio.h>

int main(void) {
    int rc = FirestaffFmtownsCd_SelfTest();
    if (rc == 0) {
        printf("test_firestaff_fmtowns_cd_classify: PASS\n");
        return 0;
    }
    printf("test_firestaff_fmtowns_cd_classify: FAIL\n");
    return 1;
}
