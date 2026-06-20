/*
 * test_firestaff_img5_decode.c
 *
 * Standalone unit tests for the IMG5 4bpp chunked image decoder.
 *
 * Runs three tests:
 *   1. SelfTest: exhaustive round-trip across a small parameter
 *      grid (8/16/32/64/320 pixels, 4 patterns). Mirrors the
 *      SelfTest() inside firestaff_img5_decode.c but adds an
 *      explicit pass/fail report.
 *   2. All-zero: decodes a known input (4 zero bytes for 8 pixels)
 *      and asserts the output is all-zero.
 *   3. All-0xF: decodes a known input (4 0xFF bytes for 8 pixels)
 *      and asserts the output is all-0xF.
 *
 * The decoder is purely deterministic, so these tests do not need
 * real game data -- they verify the algorithm against greatstone's
 * published spec.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "firestaff_img5_decode.h"

#define ASSERT_TRUE(cond) do {                                            \
    if (!(cond)) {                                                        \
        fprintf(stderr, "ASSERTION FAILED at %s:%d: %s\n",               \
                __FILE__, __LINE__, #cond);                               \
        return 0;                                                          \
    }                                                                      \
} while (0)

static int test_all_zero_8(void) {
    /* 8 pixels, all zero. Input: 4 bytes (plane 0..3) all zero. */
    uint8_t src[4] = {0, 0, 0, 0};
    uint8_t dst[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    int rc = FirestaffImg5_Decode(src, sizeof(src), 8, dst);
    ASSERT_TRUE(rc == 0);
    for (int i = 0; i < 8; ++i) {
        ASSERT_TRUE(dst[i] == 0);
    }
    return 1;
}

static int test_all_F_8(void) {
    /* 8 pixels, all 0xF. Each of the 4 planes is 0xFF so each pixel
     * accumulates all 4 bits -> 0b1111 = 0x0F. */
    uint8_t src[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t dst[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int rc = FirestaffImg5_Decode(src, sizeof(src), 8, dst);
    ASSERT_TRUE(rc == 0);
    for (int i = 0; i < 8; ++i) {
        ASSERT_TRUE(dst[i] == 0x0F);
    }
    return 1;
}

static int test_alternating_0F_16(void) {
    /* 16 pixels alternating 0x0, 0xF. plane_size = 2 bytes, so we
     * have 8 bytes total. Each plane gets pattern 0b01010101 = 0x55. */
    uint8_t src[8] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
    uint8_t dst[16] = {0};
    int rc = FirestaffImg5_Decode(src, sizeof(src), 16, dst);
    ASSERT_TRUE(rc == 0);
    uint8_t expected[16] = {
        0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F,
        0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F
    };
    for (int i = 0; i < 16; ++i) {
        ASSERT_TRUE(dst[i] == expected[i]);
    }
    return 1;
}

static int test_320_gradient(void) {
    /* Simulate a 320-pixel-wide gradient (typical DM viewport).
     * Encode then decode, verify the round-trip. */
    const size_t N = 320;
    uint8_t* src = (uint8_t*)malloc(N);
    uint8_t* encoded = (uint8_t*)malloc(FirestaffImg5_EncodedSize(N));
    uint8_t* decoded = (uint8_t*)malloc(N);
    if (!src || !encoded || !decoded) {
        free(src); free(encoded); free(decoded);
        return 0;
    }
    /* Gradient: pixel i = (i % 16) so we cycle through all 16 colors. */
    for (size_t i = 0; i < N; ++i) src[i] = (uint8_t)(i % 16);

    /* Encode */
    memset(encoded, 0, FirestaffImg5_EncodedSize(N));
    size_t planeSize = N / 8;
    for (int plane = 0; plane < 4; ++plane) {
        for (size_t px = 0; px < N; ++px) {
            uint8_t bit = (src[px] >> plane) & 1u;
            size_t byteIdx = px >> 3;
            unsigned bitIdx = 7 - (unsigned)(px & 7);
            encoded[(size_t)plane * planeSize + byteIdx] |=
                (uint8_t)(bit << bitIdx);
        }
    }

    /* Decode */
    int rc = FirestaffImg5_Decode(encoded, FirestaffImg5_EncodedSize(N), N, decoded);
    if (rc != 0) {
        free(src); free(encoded); free(decoded);
        return 0;
    }
    for (size_t i = 0; i < N; ++i) {
        if (src[i] != decoded[i]) {
            free(src); free(encoded); free(decoded);
            return 0;
        }
    }
    free(src); free(encoded); free(decoded);
    return 1;
}

static int test_self_test_runs(void) {
    /* Just verify the in-source self-test passes when called. */
    return FirestaffImg5_SelfTest() == 0;
}

static int test_encoded_size(void) {
    /* 8 pixels = 4 bytes, 16 = 8, 320 = 160, etc. */
    if (FirestaffImg5_EncodedSize(8) != 4) return 0;
    if (FirestaffImg5_EncodedSize(16) != 8) return 0;
    if (FirestaffImg5_EncodedSize(320) != 160) return 0;
    if (FirestaffImg5_EncodedSize(0) != (size_t)-1) return 0;
    if (FirestaffImg5_EncodedSize(7) != (size_t)-1) return 0; /* not multiple of 8 */
    return 1;
}

static int test_decode_alloc(void) {
    /* Verify the malloc'd convenience wrapper. */
    uint8_t src[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t* dst = FirestaffImg5_DecodeAlloc(src, sizeof(src), 8);
    if (dst == NULL) return 0;
    for (int i = 0; i < 8; ++i) {
        if (dst[i] != 0x0F) {
            free(dst);
            return 0;
        }
    }
    free(dst);
    return 1;
}

int main(void) {
    int passed = 0, total = 0;

    #define RUN(name) do {                                                 \
        total++;                                                           \
        if (name()) {                                                      \
            passed++;                                                      \
        } else {                                                           \
            fprintf(stderr, "test failed: %s\n", #name);                  \
        }                                                                   \
    } while (0)

    RUN(test_all_zero_8);
    RUN(test_all_F_8);
    RUN(test_alternating_0F_16);
    RUN(test_320_gradient);
    RUN(test_self_test_runs);
    RUN(test_encoded_size);
    RUN(test_decode_alloc);

    printf("test_firestaff_img5_decode: %d/%d passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
