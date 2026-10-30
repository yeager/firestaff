#include "firestaff_snes_romimg3.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 0; \
    } \
} while (0)

static int test_planar_bit_order(void)
{
    uint8_t encoded[FIRESTAFF_SNES_4BPP_TILE_BYTES] = {0};
    uint8_t pixels[64];

    /* First four pixels of row zero select planes 0, 1, 2, then 3. */
    encoded[0] = 0x80;
    encoded[1] = 0x40;
    encoded[16] = 0x20;
    encoded[17] = 0x10;
    memset(pixels, 0xFF, sizeof(pixels));

    CHECK(FirestaffSnesRomimg3_DecodeTiles(encoded, sizeof(encoded), 1, 1,
                                            pixels, sizeof(pixels)) == 0);
    CHECK(pixels[0] == 1);
    CHECK(pixels[1] == 2);
    CHECK(pixels[2] == 4);
    CHECK(pixels[3] == 8);
    CHECK(pixels[4] == 0);
    CHECK(pixels[7] == 0);
    CHECK(pixels[8] == 0);
    return 1;
}

static int test_tile_order_and_palette_selectors(void)
{
    uint8_t encoded[2 * FIRESTAFF_SNES_4BPP_TILE_BYTES];
    uint8_t pixels[2 * 64];
    const uint8_t selectors[2] = {0, 2};
    size_t x;
    size_t y;

    memset(encoded, 0, FIRESTAFF_SNES_4BPP_TILE_BYTES);
    memset(encoded + FIRESTAFF_SNES_4BPP_TILE_BYTES, 0xFF,
           FIRESTAFF_SNES_4BPP_TILE_BYTES);
    memset(pixels, 0, sizeof(pixels));
    CHECK(FirestaffSnesRomimg3_DecodeTiles(encoded, sizeof(encoded), 2, 1,
                                            pixels, sizeof(pixels)) == 0);
    for (y = 0; y < 8; ++y) {
        for (x = 0; x < 8; ++x) {
            CHECK(pixels[y * 16 + x] == 0);
            CHECK(pixels[y * 16 + 8 + x] == 15);
        }
    }

    CHECK(FirestaffSnesRomimg3_ApplyPaletteSelectors(pixels, sizeof(pixels),
                                                      2, 1, selectors, 2, 1,
                                                      128, 16) == 0);
    for (y = 0; y < 8; ++y) {
        for (x = 0; x < 8; ++x) {
            CHECK(pixels[y * 16 + x] == 128);
            CHECK(pixels[y * 16 + 8 + x] == 175);
        }
    }
    return 1;
}

static int test_invalid_sizes_and_indices(void)
{
    uint8_t input[FIRESTAFF_SNES_4BPP_TILE_BYTES] = {0};
    uint8_t pixels[64] = {0};
    uint8_t selector[1] = {255};

    CHECK(FirestaffSnesRomimg3_DecodeTiles(input, sizeof(input) - 1, 1, 1,
                                            pixels, sizeof(pixels)) == -1);
    CHECK(FirestaffSnesRomimg3_DecodeTiles(input, sizeof(input), 1, 1,
                                            pixels, sizeof(pixels) - 1) == -1);
    CHECK(FirestaffSnesRomimg3_DecodeTiles(input, sizeof(input), 0, 1,
                                            pixels, sizeof(pixels)) == -1);
    CHECK(FirestaffSnesRomimg3_ApplyPaletteSelectors(pixels, sizeof(pixels),
                                                      1, 1, selector, 1, 1,
                                                      128, 16) == -1);
    CHECK(FirestaffSnesRomimg3_ApplyPaletteSelectors(pixels, sizeof(pixels),
                                                      1, 1, selector, 0, 1,
                                                      0, 16) == -1);
    return 1;
}

int main(void)
{
    if (!test_planar_bit_order() || !test_tile_order_and_palette_selectors() ||
        !test_invalid_sizes_and_indices()) {
        return 1;
    }
    puts("test_firestaff_snes_romimg3: 3/3 passed");
    return 0;
}
