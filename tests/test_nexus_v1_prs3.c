#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "nexus_v1_prs3.h"

/* Hand-crafted PRS3 blob: all-literal, 8 bytes.
 *
 * Header (16 bytes, big-endian):
 *   magic           = 'PRS3' (0x50525333)
 *   version         = 0x00000001
 *   uncompressed_sz = 0x00000008
 *   compressed_sz   = 0x00000009   (1 control byte + 8 literals)
 *
 * Payload (9 bytes):
 *   control = 0xFF  (all 8 bits set = 8 literals)
 *   literals: 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48  ("ABCDEFGH")
 */
static const uint8_t prs3_all_literal[] = {
    /* Header */
    0x50, 0x52, 0x53, 0x33,   /* magic: PRS3 */
    0x00, 0x00, 0x00, 0x01,   /* version: 1 */
    0x00, 0x00, 0x00, 0x08,   /* uncompressed size: 8 */
    0x00, 0x00, 0x00, 0x09,   /* compressed size: 9 */
    /* Payload */
    0xFF,                     /* control: all literals */
    0x41, 0x42, 0x43, 0x44,   /* A B C D */
    0x45, 0x46, 0x47, 0x48    /* E F G H */
};

/* DMWeb DecodePRS3 back-reference fixtures. Control bits are LSB-first.
 * The first fixture exercises the negative window (&HFEE -> offset 0).
 * The second exercises the forward window (raw 0 -> offset 18). */
static const uint8_t prs3_negative_window[] = {
    0x50, 0x52, 0x53, 0x33,
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x06,
    0x00, 0x00, 0x00, 0x06,
    0x07, 0x41, 0x42, 0x43, 0xEE, 0xF0
};

static const uint8_t prs3_forward_window[] = {
    0x50, 0x52, 0x53, 0x33,
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x18,
    0x00, 0x00, 0x00, 0x1A,
    0xFF,
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0xFF,
    0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
    0x1F,
    0x51, 0x52, 0x53, 0x54, 0x55,
    0x00, 0x00
};

/* A positive window reference before any output exists is malformed. The
 * active DMWeb-compatible decoder rejects it rather than zero-filling it. */
static const uint8_t prs3_forward_reference_without_output[] = {
    0x50, 0x52, 0x53, 0x33,
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00
};

static void test_all_literal(void) {
    uint8_t out[64];
    memset(out, 0xCC, sizeof(out));

    size_t n = nexus_v1_prs3_decode(prs3_all_literal, sizeof(prs3_all_literal),
                                    out, sizeof(out));
    assert(n == 8);
    assert(memcmp(out, "ABCDEFGH", 8) == 0);
    printf("  PASS: all-literal decode\n");
}

static void test_bad_magic(void) {
    uint8_t bad[25];
    memcpy(bad, prs3_all_literal, sizeof(prs3_all_literal));
    bad[0] = 0x00;  /* corrupt magic */
    uint8_t out[64];
    size_t n = nexus_v1_prs3_decode(bad, sizeof(bad), out, sizeof(out));
    assert(n == 0);
    printf("  PASS: bad magic rejected\n");
}

static void test_bad_version(void) {
    uint8_t bad[sizeof(prs3_all_literal)];
    memcpy(bad, prs3_all_literal, sizeof(bad));
    bad[7] = 0x02;
    uint8_t out[64];
    assert(nexus_v1_prs3_decode(bad, sizeof(bad), out, sizeof(out)) == 0);
    printf("  PASS: unsupported version rejected\n");
}

static void test_null_args(void) {
    uint8_t out[64];
    assert(nexus_v1_prs3_decode(NULL, 0, out, sizeof(out)) == 0);
    assert(nexus_v1_prs3_decode(prs3_all_literal, sizeof(prs3_all_literal),
                                NULL, 0) == 0);
    printf("  PASS: null args rejected\n");
}

static void test_output_too_small(void) {
    uint8_t out[4];  /* smaller than uncompressed size (8) */
    size_t n = nexus_v1_prs3_decode(prs3_all_literal, sizeof(prs3_all_literal),
                                    out, sizeof(out));
    assert(n == 0);
    printf("  PASS: output too small rejected\n");
}

static void test_negative_window(void) {
    uint8_t out[6];
    size_t n = nexus_v1_prs3_decode(prs3_negative_window,
                                    sizeof(prs3_negative_window),
                                    out, sizeof(out));
    assert(n == 6);
    assert(memcmp(out, "ABCABC", 6) == 0);
    printf("  PASS: negative-window back-reference\n");
}

static void test_forward_window(void) {
    uint8_t out[24];
    size_t n = nexus_v1_prs3_decode(prs3_forward_window,
                                    sizeof(prs3_forward_window),
                                    out, sizeof(out));
    assert(n == 24);
    assert(memcmp(out, "ABCDEFGHIJKLMNOPQRSTU", 21) == 0);
    assert(memcmp(out + 21, "STU", 3) == 0);
    printf("  PASS: forward-window back-reference\n");
}

static void test_forward_reference_without_output(void) {
    uint8_t out[3];
    assert(nexus_v1_prs3_decode(prs3_forward_reference_without_output,
                                sizeof(prs3_forward_reference_without_output),
                                out, sizeof(out)) == 0);
    printf("  PASS: positive forward reference rejected\n");
}

int main(void) {
    printf("nexus_v1_prs3 tests:\n");
    test_all_literal();
    test_bad_magic();
    test_bad_version();
    test_null_args();
    test_output_too_small();
    test_negative_window();
    test_forward_window();
    test_forward_reference_without_output();
    printf("All PRS3 tests passed.\n");
    return 0;
}
