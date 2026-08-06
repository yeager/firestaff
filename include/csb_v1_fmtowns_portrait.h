#ifndef CSB_V1_FMTOWNS_PORTRAIT_H
#define CSB_V1_FMTOWNS_PORTRAIT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CSB FM Towns champion portrait .CMP decoder.
 *
 * Each .CMP file is 508 bytes with a 44-byte header followed by
 * 464 bytes of Atari ST four-plane data (32x29 portrait).
 *
 * Header layout:
 *   0-1:   uint16 identifier (0xA791)
 *   2-3:   uint16 field count or offset (0x000A)
 *   4-9:   3 x uint16 metadata values
 *   10-15: 6 reserved/zero bytes
 *   16-23: Champion first name (8 bytes, null-padded ASCII)
 *   24-43: Champion title (20 bytes, null-padded ASCII)
 *   44-507: Atari ST 4-plane pixels (32x29 = 928 pixels, 464 bytes)
 *
 * ReDMCSB PORTRAIT.C F7251 converts the payload to the F31 chunky bitmap.
 */

#define CSB_FMTOWNS_PORTRAIT_FILE_SIZE     508u
#define CSB_FMTOWNS_PORTRAIT_HEADER_SIZE   44u
#define CSB_FMTOWNS_PORTRAIT_DATA_SIZE     464u
#define CSB_FMTOWNS_PORTRAIT_WIDTH         32u
#define CSB_FMTOWNS_PORTRAIT_HEIGHT        29u
#define CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT   928u
#define CSB_FMTOWNS_PORTRAIT_NAME_LEN      8u
#define CSB_FMTOWNS_PORTRAIT_TITLE_LEN     20u
#define CSB_FMTOWNS_PORTRAIT_IDENTIFIER    0xA791u
#define CSB_FMTOWNS_PORTRAIT_COUNT         24u

typedef struct {
    int      valid;
    uint16_t identifier;
    char     name[CSB_FMTOWNS_PORTRAIT_NAME_LEN + 1];
    char     title[CSB_FMTOWNS_PORTRAIT_TITLE_LEN + 1];
    uint32_t pixel_fnv1a;
} CSB_V1_FmtownsPortraitReceipt;

/* Probe whether a buffer is a valid FM Towns CSB portrait .CMP file.
 * Returns 1 if size is 508 and identifier matches. */
int csb_v1_fmtowns_portrait_probe(const uint8_t *data, size_t size);

/* Decode a .CMP portrait to indexed 4bpp pixels (one byte per pixel) using
 * the F31 PORTRAIT.C F7251 planar conversion. indexed_pixels must have
 * capacity for at least 928 bytes.
 * Returns 1 on success, 0 on failure. */
int csb_v1_fmtowns_portrait_decode(const uint8_t *data, size_t size,
                                    uint8_t *indexed_pixels,
                                    size_t pixel_capacity,
                                    CSB_V1_FmtownsPortraitReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_PORTRAIT_H */
