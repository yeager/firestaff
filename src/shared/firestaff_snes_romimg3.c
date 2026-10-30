#include "firestaff_snes_romimg3.h"

#include <limits.h>

static int checked_image_sizes(size_t tileColumns,
                               size_t tileRows,
                               size_t *tileCount,
                               size_t *pixelBytes,
                               size_t *inputBytes)
{
    size_t tiles;

    if (tileColumns == 0 || tileRows == 0) return -1;
    if (tileColumns > SIZE_MAX / tileRows) return -1;
    tiles = tileColumns * tileRows;
    if (tiles > SIZE_MAX / 64u) return -1;
    if (tiles > SIZE_MAX / FIRESTAFF_SNES_4BPP_TILE_BYTES) return -1;

    *tileCount = tiles;
    *pixelBytes = tiles * 64u;
    *inputBytes = tiles * FIRESTAFF_SNES_4BPP_TILE_BYTES;
    return 0;
}

int FirestaffSnesRomimg3_DecodeTiles(const uint8_t *input,
                                     size_t inputBytes,
                                     size_t tileColumns,
                                     size_t tileRows,
                                     uint8_t *output,
                                     size_t outputBytes)
{
    size_t tileCount;
    size_t expectedPixels;
    size_t expectedInput;
    size_t tile;

    if (input == NULL || output == NULL) return -1;
    if (checked_image_sizes(tileColumns, tileRows, &tileCount,
                            &expectedPixels, &expectedInput) != 0) {
        return -1;
    }
    if (inputBytes != expectedInput || outputBytes != expectedPixels) return -1;

    for (tile = 0; tile < tileCount; ++tile) {
        size_t tileX = tile % tileColumns;
        size_t tileY = tile / tileColumns;
        size_t tileInput = tile * FIRESTAFF_SNES_4BPP_TILE_BYTES;
        size_t y;

        for (y = 0; y < FIRESTAFF_SNES_TILE_SIDE; ++y) {
            uint8_t plane01 = input[tileInput + y * 2u];
            uint8_t plane23 = input[tileInput + 16u + y * 2u];
            uint8_t plane1 = input[tileInput + y * 2u + 1u];
            uint8_t plane3 = input[tileInput + 17u + y * 2u];
            size_t x;

            for (x = 0; x < FIRESTAFF_SNES_TILE_SIDE; ++x) {
                unsigned shift = 7u - (unsigned)x;
                uint8_t pixel = (uint8_t)(((plane01 >> shift) & 1u) |
                                          (((plane1 >> shift) & 1u) << 1) |
                                          (((plane23 >> shift) & 1u) << 2) |
                                          (((plane3 >> shift) & 1u) << 3));
                size_t outX = tileX * FIRESTAFF_SNES_TILE_SIDE + x;
                size_t outY = tileY * FIRESTAFF_SNES_TILE_SIDE + y;
                output[outY * (tileColumns * FIRESTAFF_SNES_TILE_SIDE) + outX] = pixel;
            }
        }
    }
    return 0;
}

int FirestaffSnesRomimg3_ApplyPaletteSelectors(uint8_t *pixels,
                                               size_t pixelBytes,
                                               size_t tileColumns,
                                               size_t tileRows,
                                               const uint8_t *selectors,
                                               size_t selectorCount,
                                               size_t tilesPerItem,
                                               uint16_t paletteBase,
                                               uint16_t paletteStride)
{
    size_t tileCount;
    size_t expectedPixels;
    size_t ignoredInput;
    size_t tile;

    if (pixels == NULL || selectors == NULL || tilesPerItem == 0) return -1;
    if (checked_image_sizes(tileColumns, tileRows, &tileCount,
                            &expectedPixels, &ignoredInput) != 0) {
        return -1;
    }
    if (pixelBytes != expectedPixels) return -1;
    if (selectorCount < tileCount / tilesPerItem +
                        ((tileCount % tilesPerItem) != 0u)) return -1;

    for (tile = 0; tile < tileCount; ++tile) {
        size_t item = tile / tilesPerItem;
        uint32_t paletteOffset = (uint32_t)paletteBase +
                                 (uint32_t)selectors[item] * paletteStride;
        size_t tileX = tile % tileColumns;
        size_t tileY = tile / tileColumns;
        size_t y;

        for (y = 0; y < FIRESTAFF_SNES_TILE_SIDE; ++y) {
            size_t x;
            size_t row = (tileY * FIRESTAFF_SNES_TILE_SIDE + y) *
                         (tileColumns * FIRESTAFF_SNES_TILE_SIDE) +
                         tileX * FIRESTAFF_SNES_TILE_SIDE;
            for (x = 0; x < FIRESTAFF_SNES_TILE_SIDE; ++x) {
                uint32_t indexed = paletteOffset + pixels[row + x];
                if (indexed > UINT8_MAX) return -1;
            }
        }
    }

    for (tile = 0; tile < tileCount; ++tile) {
        size_t item = tile / tilesPerItem;
        uint8_t paletteOffset = (uint8_t)((uint32_t)paletteBase +
                                          (uint32_t)selectors[item] * paletteStride);
        size_t tileX = tile % tileColumns;
        size_t tileY = tile / tileColumns;
        size_t y;

        for (y = 0; y < FIRESTAFF_SNES_TILE_SIDE; ++y) {
            size_t x;
            size_t row = (tileY * FIRESTAFF_SNES_TILE_SIDE + y) *
                         (tileColumns * FIRESTAFF_SNES_TILE_SIDE) +
                         tileX * FIRESTAFF_SNES_TILE_SIDE;
            for (x = 0; x < FIRESTAFF_SNES_TILE_SIDE; ++x) {
                pixels[row + x] = (uint8_t)(paletteOffset + pixels[row + x]);
            }
        }
    }
    return 0;
}
