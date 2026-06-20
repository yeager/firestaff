/*
 * test_firestaff_cmp_decode.c
 *
 * Test driver for firestaff_cmp_decode.c. Calls SelfTest
 * which runs all internal cases:
 *   - valid_cmp
 *   - too_short
 *   - bad_magic (cmp_i_C or cmp_i_E != 0)
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

int main(void) {
    int rc = FirestaffCmp_SelfTest();
    if (rc == 0) {
        printf("test_firestaff_cmp_decode: PASS\n");
        return 0;
    }
    printf("test_firestaff_cmp_decode: FAIL\n");
    return 1;
}
