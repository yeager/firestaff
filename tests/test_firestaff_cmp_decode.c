/*
 * test_firestaff_cmp_decode.c
 *
 * Test driver for firestaff_cmp_decode.c. Calls SelfTest
 * which runs all internal cases:
 *   - valid_cmp
 *   - too_short
 *   - bad_magic (the source compatibility header is malformed)
 *   - bad_name (lowercase character)
 *   - bad_title (control character)
 *   - max_lengths (7-char name + 19-char title)
 *
 * Build:
 *   cc -std=c99 -I include tests/test_firestaff_cmp_decode.c \
 *      src/shared/firestaff_cmp_decode.c -o test_firestaff_cmp_decode
 */

#include "firestaff_cmp_decode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int verify_real_cmp(const char *path) {
    uint8_t bytes[FIRESTAFF_CMP_FILE_SIZE];
    FirestaffCmp cmp;
    FILE *file;

    file = fopen(path, "rb");
    if (!file || fread(bytes, 1U, sizeof(bytes), file) != sizeof(bytes) ||
        fgetc(file) != EOF) {
        if (file) fclose(file);
        fprintf(stderr, "test_firestaff_cmp_decode: cannot read exact CMP %s\n", path);
        return 0;
    }
    fclose(file);
    if (FirestaffCmp_Decode(bytes, sizeof(bytes), &cmp) != 0 ||
        cmp.magic != 0x91a7u || cmp.name[0] == '\0' ||
        cmp.title[0] == '\0' ||
        memcmp(cmp.name, bytes + FIRESTAFF_CMP_NAME_OFFSET,
               strlen(cmp.name)) != 0 ||
        memcmp(cmp.title, bytes + FIRESTAFF_CMP_TITLE_OFFSET,
               strlen(cmp.title)) != 0 ||
        cmp.portrait_size != FIRESTAFF_CMP_PORTRAIT_BYTES) {
        fprintf(stderr, "test_firestaff_cmp_decode: real CMP rejected or decoded incorrectly\n");
        return 0;
    }
    return 1;
}

int main(int argc, char **argv) {
    const char *real_cmp = getenv("FIRESTAFF_CSB_CMP");
    int rc = FirestaffCmp_SelfTest();
    if (argc > 2 ||
        (argc == 2 && real_cmp && real_cmp[0] != '\0') ||
        (argc == 2 && !verify_real_cmp(argv[1])) ||
        (argc == 1 && real_cmp && real_cmp[0] != '\0' &&
         !verify_real_cmp(real_cmp))) {
        rc = -1;
    }
    if (rc == 0) {
        printf("test_firestaff_cmp_decode: PASS\n");
        return 0;
    }
    printf("test_firestaff_cmp_decode: FAIL\n");
    return 1;
}
