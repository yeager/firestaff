/*
 * test_firestaff_ftl_hunk_data_zero_run.c
 *
 * Test driver for firestaff_ftl_hunk_data_zero_run.c.
 *
 * The library owns a FirestaffFtlHunkData_SelfTest() that runs all
 * internal cases (all-zero stream, literal-only stream, mixed
 * literal + run, truncated run header, oversize / undersize declared
 * uncompressed size, odd input length, empty round trip, worst-case
 * size bound, run-storm realistic fixture). This file is a thin
 * main() that just calls SelfTest and reports PASS / FAIL.
 *
 * Source of truth for the cases:
 *   greatstone d_ftl.html "Note 7: How to decompress HUNK_DATA"
 *   (Pierre Monnot, Swoosh Construction Kit).
 *
 * Build (mirrors the firestaff_pak_decode / firestaff_ftl_container
 * unit-test pattern):
 *   cc -std=c99 -I include tests/test_firestaff_ftl_hunk_data_zero_run.c \
 *      src/shared/firestaff_ftl_hunk_data_zero_run.c \
 *      -o test_firestaff_ftl_hunk_data_zero_run
 */

#include "firestaff_ftl_hunk_data_zero_run.h"

#include <stdio.h>

int main(void) {
    int rc = FirestaffFtlHunkData_SelfTest();
    if (rc == 0) {
        printf("test_firestaff_ftl_hunk_data_zero_run: PASS\n");
        return 0;
    }
    printf("test_firestaff_ftl_hunk_data_zero_run: FAIL\n");
    return 1;
}
