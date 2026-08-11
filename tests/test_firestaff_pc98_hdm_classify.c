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
 * Running with an HDM filename or `-` reads one caller-supplied image and
 * prints its conservative receipt. This is local preservation verification,
 * not a runtime or copy-protection claim.
 */

#include "firestaff_pc98_hdm_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int classify_stream(FILE* input) {
    uint8_t* data;
    size_t size;
    FirestaffPc98HdmClassification result;
    data = (uint8_t*)malloc(FIRESTAFF_PC98_2HD_BYTES + FIRESTAFF_PC98_FDI_HEADER);
    if (!data) return 2;
    size = fread(data, 1u, FIRESTAFF_PC98_2HD_BYTES + FIRESTAFF_PC98_FDI_HEADER, input);
    if (ferror(input) || size == 0u || fgetc(input) != EOF ||
        FirestaffPc98HdmClassify(data, size, &result) != 0) {
        free(data);
        return 2;
    }
    printf("media=%s game=%s version=%s protection=%s\n",
           FirestaffPc98HdmMediaName(result.media),
           FirestaffPc98HdmGameName(result.game),
           FirestaffPc98HdmVersionName(result.version),
           FirestaffPc98HdmProtectionName(result.protection));
    free(data);
    return result.media == FIRESTAFF_PC98_MEDIA_NOT_PC98 ? 1 : 0;
}

int main(int argc, char** argv) {
    if (argc == 2) {
        FILE* input = strcmp(argv[1], "-") == 0 ? stdin : fopen(argv[1], "rb");
        int rc;
        if (!input) return 2;
        rc = classify_stream(input);
        if (input != stdin) fclose(input);
        return rc;
    }
    int rc = FirestaffPc98HdmClassify_SelfTest();
    if (rc == 0) {
        printf("test_firestaff_pc98_hdm_classify: PASS\n");
        return 0;
    }
    printf("test_firestaff_pc98_hdm_classify: FAIL\n");
    return 1;
}
