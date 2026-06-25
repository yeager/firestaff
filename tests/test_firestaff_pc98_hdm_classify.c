/*
 * test_firestaff_pc98_hdm_classify.c
 *
 * Test driver for firestaff_pc98_hdm_classify. The library
 * ships an internal SelfTest that runs the synthetic-fixture
 * cases for the DM1 PC-9801 2.0a-original / 2.0a-cracked /
 * 2.0b-original boundary, the CSB PC-9801 v3.1 original /
 * cracked boundary, and a few media-shape / name-table
 * sanity checks.
 *
 * Self-tests covered:
 *   - not-a-pc98-image rejection (random 4 KB buffer)
 *   - DM1 2.0a original with copy-protection sector present
 *   - DM1 2.0a original with the "no flux" sector missing
 *     (the dmweb-distributed "(Not working).hdm" shape)
 *   - DM1 2.0a cracked (NECIO + FIRES patch bytes applied)
 *   - DM1 2.0b original (NECIO bytes still original, FIRES
 *     helper missing)
 *   - CSB 3.1 original and CSB 3.1 cracked (CSBGAME.EXE
 *     protection offset byte 0x26 vs 0x90)
 *   - FDI 4096-byte header detection (pc98-disk-tools /
 *     barbeque is_fdi.py contract)
 *   - name-table string stability
 *
 * Build:
 *   cc -std=c99 -I include tests/test_firestaff_pc98_hdm_classify.c \
 *      src/shared/firestaff_pc98_hdm_classify.c \
 *      -o test_firestaff_pc98_hdm_classify
 *
 * Scope:
 *   Data-free synthetic fixtures only. No real HDM media.
 *   No runtime claim, no emulator wiring.
 */

#include "firestaff_pc98_hdm_classify.h"

#include <stdio.h>

int main(void) {
    int rc = FirestaffPc98HdmClassify_SelfTest();
    if (rc == 0) {
        printf("test_firestaff_pc98_hdm_classify: PASS\n");
        return 0;
    }
    printf("test_firestaff_pc98_hdm_classify: FAIL\n");
    return 1;
}
