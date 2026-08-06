#ifndef CSB_V1_FMTOWNS_GRAPHICS_DAT_H
#define CSB_V1_FMTOWNS_GRAPHICS_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CSB FM Towns GRAPHICS.DAT classifier and item decoder.
 *
 * The FM Towns release (FTL/Fujitsu, 1992) ships a GRAPHICS.DAT that uses
 * the DMCSB1 container format (0x8001 new-format marker) in little-endian
 * byte order with 728 graphic items. Individual items use IMG2 byte-command
 * encoding (ReDMCSB IMAGE2.C) with LE16 width/height headers — distinct
 * from PC CSB (IMG3 nibble LE16) and Amiga CSB (IMG2 BE16).
 *
 * The CD also contains:
 *   - CDATA/  (English) and CJDATA/ (Japanese) game data directories
 *   - 30 CDDA audio tracks (tracks 2-31)
 *   - PORTRAIT/ champion portraits (.CMP format)
 *   - TITLE.ANM, STORY.ANM, ENDING.ANM animation sequences
 *
 * Source references:
 *   ReDMCSB IMAGE2.C F0689: IMG2 byte-command decoding
 *   CSBWin CSBCode.cpp ExpandGraphic: byte-command dispatch
 */

#define CSB_FMTOWNS_GRAPHICS_CONTAINER_WORD   0x8001u
#define CSB_FMTOWNS_GRAPHICS_ITEM_COUNT       728u
#define CSB_FMTOWNS_GRAPHICS_MIN_SIZE         390000u
#define CSB_FMTOWNS_GRAPHICS_MAX_SIZE         420000u
#define CSB_FMTOWNS_GRAPHICS_EN_EXPECTED_SIZE 402274u
#define CSB_FMTOWNS_GRAPHICS_JP_EXPECTED_SIZE 402676u

#define CSB_FMTOWNS_CDDA_TRACK_COUNT          30u
#define CSB_FMTOWNS_CDDA_FIRST_TRACK          2u
#define CSB_FMTOWNS_CDDA_LAST_TRACK           31u

typedef struct {
    int      is_fmtowns;
    uint16_t item_count;
    uint32_t file_size;
    uint32_t image_item_count;
    uint32_t empty_item_count;
    uint32_t data_item_count;
    uint32_t header_fnv1a;
    uint32_t payload_fnv1a;
} CSB_V1_FmtownsGraphicsReceipt;

typedef struct {
    int      valid;
    uint16_t width;
    uint16_t height;
    size_t   stream_byte_count;
    size_t   stream_bytes_consumed;
    size_t   container_offset;
    size_t   pixel_count;
    uint32_t stream_fnv1a;
    uint32_t pixel_fnv1a;
    int      is_image;
    int      is_data_record;
} CSB_V1_FmtownsItemDecodeReceipt;

/* Probe whether a GRAPHICS.DAT buffer is the FM Towns CSB version.
 * Returns 1 if container word is 0x8001, item count is 728,
 * and file size is in the expected range. */
int csb_v1_fmtowns_graphics_probe(const uint8_t *data, size_t size);

/* Build a receipt describing the FM Towns CSB GRAPHICS.DAT.
 * Returns 0 on success, -1 if not a valid FM Towns CSB file. */
int csb_v1_fmtowns_graphics_receipt(const uint8_t *data, size_t size,
                                    CSB_V1_FmtownsGraphicsReceipt *out);

/* Decode one FM Towns CSB graphic item using IMG2 byte commands
 * with LE16 width/height header. The caller provides the item's
 * raw bytes (from the container payload) and the expected dimensions
 * from the container header table.
 *
 * indexed_pixels must have capacity for at least width * height bytes.
 * Returns 1 on success, 0 on failure. */
int csb_v1_fmtowns_img2_decode(const uint8_t *item, size_t item_size,
                                uint16_t container_width,
                                uint16_t container_height,
                                uint8_t *indexed_pixels,
                                size_t pixel_capacity,
                                CSB_V1_FmtownsItemDecodeReceipt *receipt);

/* Select and expand one raster item directly from the original F31E/F31J
 * GRAPHICS.DAT container.  This mirrors ReDMCSB MEMORY.C F0490 selecting a
 * record followed by IMAGE2.C F0689 expansion; non-raster data records are
 * deliberately rejected instead of being interpreted as pixels. */
int csb_v1_fmtowns_graphics_decode_item(const uint8_t *data, size_t size,
                                        uint16_t item_index,
                                        uint8_t *indexed_pixels,
                                        size_t pixel_capacity,
                                        CSB_V1_FmtownsItemDecodeReceipt *receipt);

/* Copy a byte-identical uncompressed non-raster record.  M653 (graphic 695
 * on F31) is the 1bpp 1024x6 interface font: ReDMCSB TEXT.C loads it with
 * NOT_EXPANDED, so treating its 768 bytes as IMG2 pixels would be wrong.
 * This routine intentionally accepts only records whose stored and decoded
 * byte counts are equal. */
int csb_v1_fmtowns_graphics_copy_raw_item(
    const uint8_t *data, size_t size, uint16_t item_index,
    uint8_t *raw_bytes, size_t raw_capacity,
    CSB_V1_FmtownsItemDecodeReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_GRAPHICS_DAT_H */
