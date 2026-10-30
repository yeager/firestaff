/*
 * DM1 SNES ROMIMG3 graphics primitives.
 *
 * ROMIMG3 is not the IMG5 whole-image planar format. It stores 8x8
 * SNES 4bpp tiles with plane pairs interleaved for each pixel row.
 * These helpers decode indexed tile pixels and apply the per-item
 * palette selector used by the SCK ROMIMG3 map metadata.
 */
#ifndef FIRESTAFF_SNES_ROMIMG3_H
#define FIRESTAFF_SNES_ROMIMG3_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_SNES_4BPP_TILE_BYTES 32u
#define FIRESTAFF_SNES_TILE_SIDE 8u

/*
 * Decode row-major SNES 4bpp tiles to row-major pixels.
 *
 * Tiles are arranged left-to-right, then top-to-bottom. Each 32-byte
 * tile stores planes 0/1 as two bytes per row, followed by planes 2/3
 * in the same layout. Pixel bits are read most-significant first.
 * Output pixels contain palette indexes 0..15.
 *
 * tileColumns * tileRows must fit in size_t; input and output lengths
 * must be exactly the sizes implied by the tile grid.
 * Returns 0 on success and -1 for invalid arguments or size overflow.
 */
int FirestaffSnesRomimg3_DecodeTiles(const uint8_t *input,
                                     size_t inputBytes,
                                     size_t tileColumns,
                                     size_t tileRows,
                                     uint8_t *output,
                                     size_t outputBytes);

/*
 * Apply SCK PALSEL values to a decoded ROMIMG3 image in place.
 *
 * Each consecutive tilesPerItem tiles share one selector. For each
 * pixel, output = paletteBase + selector * paletteStride + pixel.
 * This converts pixel nibbles to an indexed-palette image; it does not
 * resolve RGB colors from the game's separate palette resource.
 *
 * Returns 0 on success and -1 for invalid sizes, missing selector
 * entries, or a palette-index result greater than 255.
 */
int FirestaffSnesRomimg3_ApplyPaletteSelectors(uint8_t *pixels,
                                               size_t pixelBytes,
                                               size_t tileColumns,
                                               size_t tileRows,
                                               const uint8_t *selectors,
                                               size_t selectorCount,
                                               size_t tilesPerItem,
                                               uint16_t paletteBase,
                                               uint16_t paletteStride);

#ifdef __cplusplus
}
#endif

#endif
