#include "dm1_snes_h03.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL:%d: %s\n", __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int decode(const uint8_t *src, size_t src_size, uint8_t *out,
                  size_t out_capacity, size_t *out_size, size_t *consumed) {
    return dm1_snes_h03_decode_slot(src, src_size, out, out_capacity,
                                    out_size, consumed);
}

int main(void) {
    /* Literal command followed by distance-two overlapping expansion. */
    const uint8_t overlap[] = {
        0, 0, 0, 6, 2, 'a', 'b', 0x88, 2
    };
    /* Invalid distance-one reference before any output exists. */
    const uint8_t invalid_distance[] = {0, 0, 0, 3, 0x86, 1};
    const uint8_t truncated_literal[] = {0, 0, 0, 2, 2, 'x'};
    const uint8_t output_overrun[] = {0, 0, 0, 1, 2, 'x', 'y'};
    const uint8_t zero_literal[] = {0, 0, 0, 2, 1, 'x', 0, 1, 'y'};
    const uint8_t zero_repeat[] = {0, 0, 0, 2, 1, 'x', 0x80, 0, 1, 'y'};
    const uint8_t invalid_zero_repeat[] = {0, 0, 0, 2, 1, 'x', 0x80, 2, 1, 'y'};
    uint8_t output[8] = {0};
    uint8_t oversized_slot[DM1_SNES_H03_SLOT_SIZE + 1] = {0};
    size_t output_size = 99;
    size_t consumed = 99;

    CHECK(decode(overlap, sizeof(overlap), output, sizeof(output),
                 &output_size, &consumed));
    CHECK(output_size == 6 && consumed == sizeof(overlap));
    CHECK(memcmp(output, "ababab", 6) == 0);

    CHECK(!decode(invalid_distance, sizeof(invalid_distance), output,
                  sizeof(output), &output_size, &consumed));
    CHECK(output_size == 0 && consumed == 0);
    CHECK(!decode(truncated_literal, sizeof(truncated_literal), output,
                  sizeof(output), NULL, NULL));
    CHECK(!decode(output_overrun, sizeof(output_overrun), output,
                  sizeof(output), NULL, NULL));
    CHECK(decode(zero_literal, sizeof(zero_literal), output,
                 sizeof(output), &output_size, &consumed));
    CHECK(output_size == 2 && memcmp(output, "xy", 2) == 0);
    CHECK(consumed == sizeof(zero_literal));
    CHECK(decode(zero_repeat, sizeof(zero_repeat), output,
                 sizeof(output), &output_size, &consumed));
    CHECK(output_size == 2 && memcmp(output, "xy", 2) == 0);
    CHECK(consumed == sizeof(zero_repeat));
    CHECK(!decode(invalid_zero_repeat, sizeof(invalid_zero_repeat), output,
                  sizeof(output), NULL, NULL));
    CHECK(!decode(overlap, sizeof(overlap), output, 5, NULL, NULL));
    CHECK(!decode(overlap, 3, output, sizeof(output), NULL, NULL));
    memcpy(oversized_slot, overlap, sizeof(overlap));
    CHECK(!decode(oversized_slot, sizeof(oversized_slot), output,
                  sizeof(output), NULL, NULL));

    puts("PASS: DM1 SNES H03 bounded slot decoder");
    return 0;
}
