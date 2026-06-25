/*
 * test_firestaff_ftl_decode.c
 *
 * Test driver for firestaff_ftl_decode.c. The library has a
 * FirestaffFtl_SelfTest() that runs all internal cases; this file
 * is a thin main() that just calls SelfTest and reports pass/fail.
 *
 * The internal self-tests cover:
 *   - Parse: minimal valid FTL container with BSS + DATA + CODE segments
 *   - Parse: bad magic, bad header invariants (Unknown1/c_6/c_7),
 *            truncated input, segment type+id collision, out-of-bounds
 *   - Decode HUNK_CODE: short-dict reference (nibble 0..7, 8-bit index)
 *   - Decode HUNK_CODE: long-dict reference (nibble 8..E, 12-bit index)
 *   - Decode HUNK_CODE: literal escape (nibble 0xF, 4 nibbles -> 2 bytes)
 *   - Decode HUNK_CODE: bad 0x5223 signature, truncated input, zero
 *            word_count, mixed round-trip
 *   - Decode HUNK_DATA: literal block, zero-run block, empty input,
 *            bad control byte, truncated literal
 *
 * Build:
 *   cc -std=c99 -I include tests/test_firestaff_ftl_decode.c \
 *      src/shared/firestaff_ftl_decode.c -o test_firestaff_ftl_decode
 */

#include "firestaff_ftl_decode.h"

#include <stdio.h>

int main(void) {
    int rc = FirestaffFtl_SelfTest();
    if (rc == 0) {
        printf("test_firestaff_ftl_decode: PASS\n");
        return 0;
    }
    printf("test_firestaff_ftl_decode: FAIL\n");
    return 1;
}
