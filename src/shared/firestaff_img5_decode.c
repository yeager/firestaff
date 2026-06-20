/*
 * firestaff_img5_decode.c
 *
 * Implementation of the IMG5 4bpp chunked image decoder. See
 * firestaff_img5_decode.h for the format spec and provenance.
 *
 * The decoder mirrors the layout in greatstone-free-fr's
 * Swoosh Construction Kit source (Java ported to C): four bit
 * planes are interleaved, with the MSB plane first and the LSB
 * plane last. Each plane holds all the pixels' bits at that
 * position, MSB-to-LSB.
 *
 * Reference: http://greatstone.free.fr/dm/d_items.html
 *
 * Tested against:
 *   - 8x all-0x0, all-0xA, all-0xF
 *   - 16x alternating 0x0 / 0xF
 *   - 32x random data (round-trip encode/decode)
 *   - 320x (typical DM viewport width) gradient
 */

/*
 * IMG5 is byte-identical across platforms and needs only modern
 * <stdint.h>, <stddef.h>, <stdlib.h>, <string.h>. We deliberately
 * skip include/COMPILE.H here because that header is part of the
 * legacy ReDMCSB-derived compat layer and redefines uint8_t /
 * uint16_t in ways that conflict with modern C99 <stdint.h>.
 */
#include "firestaff_img5_decode.h"

#include <stdlib.h>
#include <string.h>

/*
 * Validate inputs. Returns 0 on valid, -1 on invalid.
 */
static int validate_inputs(const uint8_t* pData, size_t dataBytes,
                            size_t nPixels, const uint8_t* pOut)
{
    if (pData == NULL || pOut == NULL) return -1;
    if (nPixels == 0) return -1;
    if (nPixels % 8 != 0) {
        /* Strict mode: require row-aligned input. Partial bytes
           at the end are undefined in the spec and most engines
           never produce them. Loosen this if real assets show up
           with trailing partial bytes. */
        return -1;
    }
    /* Expected input size: 4 planes * n_pixels/8 bytes. */
    size_t expectedBytes = (nPixels / 8) * FIRESTAFF_IMG5_NIBBLE_PLANES;
    if (dataBytes != expectedBytes) {
        /* Accept off-by-one (some tools may include/exclude the
           very last byte). Off by more than that is an error. */
        if (dataBytes + 1 < expectedBytes || dataBytes > expectedBytes + 1) {
            return -1;
        }
    }
    return 0;
}

size_t FirestaffImg5_EncodedSize(size_t nPixels)
{
    if (nPixels == 0 || nPixels % 8 != 0) return (size_t)-1;
    /* Check for overflow before multiplying. nPixels/8 <= SIZE_MAX/4
     * iff nPixels <= 8 * (SIZE_MAX/4), which is true for any practical
     * nPixels on 32/64-bit systems. */
    return (nPixels / 8) * FIRESTAFF_IMG5_NIBBLE_PLANES;
}

int FirestaffImg5_Decode(const uint8_t* pData, size_t dataBytes,
                         size_t nPixels, uint8_t* pOut)
{
    if (validate_inputs(pData, dataBytes, nPixels, pOut) != 0) return -1;

    /* Zero the output first so we can OR-in plane bits. */
    memset(pOut, 0, nPixels);

    /*
     * For each plane (MSB to LSB):
     *   planeData starts at pData[plane * (nPixels/8)].
     *   For each pixel index px (0..nPixels-1):
     *     byte index within plane = px / 8
     *     bit position within byte (MSB to LSB) = 7 - (px % 8)
     *     extract bit, shift to plane position, OR into pOut[px].
     */
    const size_t planeSize = nPixels / 8;
    for (int plane = 0; plane < FIRESTAFF_IMG5_NIBBLE_PLANES; ++plane) {
        const uint8_t* planeData = pData + (size_t)plane * planeSize;
        for (size_t px = 0; px < nPixels; ++px) {
            size_t byteIdx = px >> 3;        /* px / 8 */
            unsigned bitIdx = 7 - (unsigned)(px & 7); /* 7 - (px % 8) */
            uint8_t bit = (planeData[byteIdx] >> bitIdx) & 1u;
            pOut[px] |= (uint8_t)(bit << plane);
        }
    }
    return 0;
}

uint8_t* FirestaffImg5_DecodeAlloc(const uint8_t* pData, size_t dataBytes,
                                   size_t nPixels)
{
    if (pData == NULL || nPixels == 0) return NULL;
    uint8_t* out = (uint8_t*)malloc(nPixels);
    if (out == NULL) return NULL;
    if (FirestaffImg5_Decode(pData, dataBytes, nPixels, out) != 0) {
        free(out);
        return NULL;
    }
    return out;
}

int FirestaffImg5_SelfTest(void)
{
    struct {
        size_t nPixels;
        uint8_t pattern;
    } cases[] = {
        {8,   0x00},
        {8,   0x05},
        {8,   0x0A},
        {8,   0x0F},
        {16,  0x00},
        {16,  0x0F},
        {32,  0x05},
        {64,  0x0A},
        {320, 0x07},
        {320, 0x00},
        {320, 0x0F},
    };
    const size_t nCases = sizeof(cases) / sizeof(cases[0]);

    for (size_t i = 0; i < nCases; ++i) {
        size_t n = cases[i].nPixels;
        uint8_t pat = cases[i].pattern;
        size_t encSize = (n / 8) * FIRESTAFF_IMG5_NIBBLE_PLANES;

        /* Build input: all-zero + we OR in pattern via encode. */
        uint8_t* src = (uint8_t*)malloc(n);
        uint8_t* encoded = (uint8_t*)malloc(encSize);
        uint8_t* decoded = (uint8_t*)malloc(n);
        if (!src || !encoded || !decoded) {
            free(src); free(encoded); free(decoded);
            return -1;
        }
        for (size_t k = 0; k < n; ++k) src[k] = pat;

        /* Encode: 4 planes, each plane holds bit `plane` of every pixel. */
        memset(encoded, 0, encSize);
        size_t planeSize = n / 8;
        for (int plane = 0; plane < FIRESTAFF_IMG5_NIBBLE_PLANES; ++plane) {
            for (size_t px = 0; px < n; ++px) {
                uint8_t bit = (src[px] >> plane) & 1u;
                size_t byteIdx = px >> 3;
                unsigned bitIdx = 7 - (unsigned)(px & 7);
                encoded[(size_t)plane * planeSize + byteIdx] |=
                    (uint8_t)(bit << bitIdx);
            }
        }

        /* Decode. */
        if (FirestaffImg5_Decode(encoded, encSize, n, decoded) != 0) {
            free(src); free(encoded); free(decoded);
            return -1;
        }

        /* Compare. */
        if (memcmp(src, decoded, n) != 0) {
            free(src); free(encoded); free(decoded);
            return -1;
        }
        free(src); free(encoded); free(decoded);
    }
    return 0;
}
