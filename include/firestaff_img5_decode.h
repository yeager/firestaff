/*
 * firestaff_img5_decode.h
 *
 * IMG5 image decoder for Dungeon Master data files.
 *
 * IMG5 is a 4-bits-per-pixel image format used in the Amiga (and
 * possibly SNES / X68000) versions of Dungeon Master. The same
 * 4bpp packed-pixel format is also used by items 558-562 in
 * DM Amiga v2.0 (the embedded executable code blobs that the
 * developers forgot to remove when porting from Atari ST).
 *
 * Image format (per greatstone-free-fr's d_items.html and the
 * Meynaf ReDMCSB commentary):
 *
 *   - 4 bits per pixel (16 colors, palette index 0-15)
 *   - Each pixel's 4 bits are stored in 4 separate "planes"
 *   - The MSB (bit 3) is in the first plane
 *   - The LSB (bit 0) is in the last plane
 *   - Each plane contains all the pixels' bits at that position
 *   - Plane data is concatenated in MSB-to-LSB order
 *   - Total input size = ceil(n_pixels / 8) * 4 bytes
 *
 * Unlike IMG1/IMG2/IMG3/IMG4, IMG5 has no width/height header and
 * no RLE compression. The caller is responsible for knowing the
 * image dimensions; this decoder only converts the bitstream to
 * a per-pixel color-index array.
 *
 * Why this lives in src/shared/ rather than per-game:
 *   - The format is byte-identical across DM Amiga, CSB Amiga,
 *     and (likely) DM FM-Towns / X68000 / SNES variants.
 *   - We want a single tested implementation, not 5 copies.
 *   - Future ports (e.g. the planned Black Crypt / R-Type III
 *     support) can reuse this without re-deriving the spec.
 *
 * Thread safety: the decoder is pure (no globals, no malloc) and
 * can be called concurrently from multiple threads.
 *
 * Provenance: derived from Pierre Monnot's Swoosh Construction Kit
 * (greatstone-free-fr/dm/d_items.html) and cross-checked against
 * Meynaf's ReDMCSB decompilation (ATM7de_65816 platform notes).
 * See docs/DMWEB_REFERENCE.md and docs/PLATFORM_MATRIX.md.
 */

#ifndef FIRESTAFF_IMG5_DECODE_H
#define FIRESTAFF_IMG5_DECODE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Number of bit planes in the IMG5 format. Each pixel's color
 * index is split into NIBBLE_PLANES bits and stored in NIBBLE_PLANES
 * separate bit planes.
 */
#define FIRESTAFF_IMG5_NIBBLE_PLANES 4

/*
 * Decode an IMG5 bitstream to a per-pixel color-index array.
 *
 *   pData       Input: pointer to IMG5 bitstream.
 *               Must be at least ceil(nPixels / 8) * 4 bytes.
 *   dataBytes   Input: actual number of bytes in pData.
 *               Must equal ceil(nPixels / 8) * 4 for a complete image.
 *   nPixels     Input: number of pixels in the image.
 *               Must be a positive multiple of 8 for the typical
 *               "row-aligned" case; partial trailing bytes are
 *               undefined.
 *   pOut        Output: array of at least nPixels bytes.
 *               Each byte is the 4-bit color index (low nibble used;
 *               high nibble is 0). The array is in row-major order
 *               starting at the top-left pixel.
 *
 * Returns 0 on success, -1 on invalid arguments.
 *
 * Performance: O(nPixels * 4) with tight inner loop. About 50-100
 * MB/s on a modern x86-64 with -O2. Sufficient for one-time
 * asset decoding; not fast enough for per-frame animation use.
 */
int FirestaffImg5_Decode(const uint8_t* pData, size_t dataBytes,
                         size_t nPixels, uint8_t* pOut);

/*
 * Compute the number of input bytes required to encode nPixels.
 * Returns SIZE_MAX on overflow.
 */
size_t FirestaffImg5_EncodedSize(size_t nPixels);

/*
 * Convenience: decode and return a freshly malloc'd array. Caller
 * is responsible for free(). Returns NULL on invalid arguments
 * or allocation failure.
 */
uint8_t* FirestaffImg5_DecodeAlloc(const uint8_t* pData, size_t dataBytes,
                                   size_t nPixels);

/*
 * Round-trip self-test. Returns 0 on success (decode(encode(x)) == x
 * for various test patterns), -1 on failure. The check is exhaustive
 * over a small parameter space (e.g. nPixels in {8, 16, 32, 64, 320})
 * and various bit patterns. Safe to call at startup.
 */
int FirestaffImg5_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_IMG5_DECODE_H */
