/*
 * test_csb_v1_cmp_import.c
 *
 * Test driver for csb_v1_cmp_import_pc34_compat.c.
 *
 * Calls SelfTest which exercises:
 *   - valid CMP import into a single CSB_V1_Champion
 *   - bad magic (cmp_i_C != 0)
 *   - bad name (lowercase character)
 *   - NULL inputs (defensive)
 *   - party import inserting into slot 0
 *   - party import filling all 4 slots
 *   - party import when full returns -4
 *
 * Build:
 *   cc -std=c99 -I include tests/test_csb_v1_cmp_import.c \
 *      src/csb/csb_v1_cmp_import_pc34_compat.c \
 *      src/csb/csb_v1_character_pc34_compat.c \
 *      src/shared/firestaff_cmp_decode.c \
 *      -o test_csb_v1_cmp_import
 *
 * Source lock:
 *   - ReDMCSB DEFS.H CMP typedef (size 496 bytes)
 *   - ReDMCSB PORTRAIT.C F0515_CHAMPION_ConvertPortraitsToAtariSTPlanar
 *   - ReDMCSB CEDT002.C / CEDT021.C (Utility Disk Champion Editor)
 *   - CSBWin/CedtData.cpp (CSB Utility Disk tool)
 */

#include "csb_v1_cmp_import_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    int rc = csb_v1_cmp_import_self_test();
    if (rc == 0) {
        printf("test_csb_v1_cmp_import: PASS\n");
        return 0;
    }
    printf("test_csb_v1_cmp_import: FAIL\n");
    return 1;
}