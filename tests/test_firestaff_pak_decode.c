/*
 * test_firestaff_pak_decode.c
 *
 * Test driver for firestaff_pak_decode.c. The library
 * has a FirestaffPak_SelfTest() that runs all internal
 * cases; this file is a thin main() that just calls
 * SelfTest and reports pass/fail.
 *
 * The internal self-tests cover:
 *   - short dict reference (nibble 0..7, 8-bit index)
 *   - long dict reference (nibble 8..E, 12-bit index)
 *   - literal escape (nibble 0xF, 4 nibbles -> 2 bytes)
 *   - bad magic rejection
 *   - truncated input rejection
 *   - zero text_size no-allocation
 *   - self-test recursion (sanity)
 *
 * Build:
 *   cc -std=c99 -I include tests/test_firestaff_pak_decode.c \
 *      src/shared/firestaff_pak_decode.c -o test_firestaff_pak_decode
 */

#include "firestaff_pak_decode.h"

#include <stdio.h>

int main(void) {
    int rc = FirestaffPak_SelfTest();
    if (rc == 0) {
        printf("test_firestaff_pak_decode: PASS\n");
        return 0;
    }
    printf("test_firestaff_pak_decode: FAIL\n");
    return 1;
}
