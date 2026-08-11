#include "nexus_v1_saturn_tile_transform.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static void test_runtime_shape_and_mask(void) {
    uint8_t input[1127];
    uint16_t output[8U * 48U];
    uint16_t table[32];
    size_t row;
    size_t pixel;

    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(table, 0, sizeof(table));

    /* Select the first table pair and make every source byte 0xAB. */
    for (row = 0; row < 8U; ++row) {
        input[row * 0x80U + 4U] = 0x00U;
        for (pixel = 0; pixel < 48U; ++pixel) {
            input[row * 0x80U + 16U + pixel * 4U + (pixel >> 1U) +
                (row >> 1U)] = 0xABU;
        }
    }

    check(nexus_v1_saturn_expand_tile_8x48(
              input, sizeof(input), output, 8U * 48U, 48U, table,
              0, 0) == 1,
          "observed 8x48 transform accepts a complete buffer");
    for (row = 0; row < 8U; ++row) {
        const uint16_t expected = (row & 1U) ? 0xA000U : 0x0000U;
        for (pixel = 0; pixel < 48U; ++pixel) {
            char message[80];
            (void)snprintf(message, sizeof(message),
                           "source mask/row shift at row %zu pixel %zu",
                           row, pixel);
            check(output[row * 48U + pixel] == expected, message);
        }
    }
}

static void test_short_input_rejected(void) {
    uint8_t input[1126];
    uint16_t output[8U * 48U];
    uint16_t table[32];

    memset(input, 0, sizeof(input));
    memset(output, 0, sizeof(output));
    memset(table, 0, sizeof(table));
    check(nexus_v1_saturn_expand_tile_8x48(
              input, sizeof(input), output, 8U * 48U, 48U, table,
              0, 0) == 0,
          "buffer shorter than the observed source window is rejected");
}

int main(void) {
    test_runtime_shape_and_mask();
    test_short_input_rejected();
    if (failures != 0) {
        fprintf(stderr, "%d Nexus Saturn tile-transform checks failed\n",
                failures);
        return 1;
    }
    puts("Nexus Saturn tile-transform checks passed");
    return 0;
}
