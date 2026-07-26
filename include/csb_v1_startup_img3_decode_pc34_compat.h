#ifndef CSB_V1_STARTUP_IMG3_DECODE_PC34_COMPAT_H
#define CSB_V1_STARTUP_IMG3_DECODE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/* IMG1 (dmweb) decode provenance for one CSB Amiga C001--C005 stream.
 * This records decoder facts only; it does not assign palette meaning or
 * authorize presentation. */
typedef struct CSB_V1_StartupGraphicDecodeReceipt_PC34 {
    int valid;
    uint16_t width;
    uint16_t height;
    size_t stream_byte_count;
    size_t stream_bytes_consumed;
    size_t emitted_planar_pixels;
    size_t physical_planar_pixels;
    uint32_t stream_fnv1a;
    uint32_t indexed_pixel_fnv1a;
    int ended_at_record_boundary;
    int implicit_blank_tail;
    int indexed_colors_are_4bit;
} CSB_V1_StartupGraphicDecodeReceipt_PC34;

/*
 * Converts one decompressed CSB Amiga C001--C005 IMG1 stream into indexed
 * pixels.  IMG1 is a nibble-based RLE with big-endian width/height header.
 * The legacy function name is retained at the startup compatibility boundary.
 */
int csb_v1_startup_img3_decode_to_indexed_pc34_compat(
    const uint8_t *graphic,
    size_t graphic_byte_count,
    uint16_t expected_width,
    uint16_t expected_height,
    uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count);

int csb_v1_startup_img3_decode_to_indexed_with_receipt_pc34_compat(
    const uint8_t *graphic, size_t graphic_byte_count, uint16_t expected_width,
    uint16_t expected_height, uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count,
    CSB_V1_StartupGraphicDecodeReceipt_PC34 *out_receipt);

/* CSBWin CSBCode.cpp::_DisplayChaosStrikesBack reads C001 as one 320x153
 * graphic, then consumes its PRESENTS (y=137..152), CHAOS (y=0..79), and
 * STRIKES BACK (y=80..136) regions.  Admit only a decoded original C001
 * surface whose three source regions contain material; callers must not
 * manufacture a title phase when a source region is absent. */
int csb_v1_startup_title_c001_regions_admit_pc34_compat(
    const uint8_t *indexed_pixels, uint16_t width, uint16_t height);

#endif
