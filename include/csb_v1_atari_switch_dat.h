#ifndef FIRESTAFF_CSB_V1_ATARI_SWITCH_DAT_H
#define FIRESTAFF_CSB_V1_ATARI_SWITCH_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB FTL.H and SWITCH.C F1503/F1504/F1510.  The Atari ST resource is
 * big-endian.  This receipt retains source-owned segment spans only; it does
 * not expand graphics or manufacture a switch screen. */
#define CSB_V1_ATARI_SWITCH_OPTION_LIMIT 50u
#define CSB_V1_ATARI_SWITCH_PALETTE_BYTES 32u

typedef struct {
    int enabled;
    uint16_t segment_type;
    uint16_t segment_id;
    int16_t x;
    int16_t y;
    int16_t transparent_color;
    char ftl_file_name[43];
    uint16_t pixel_width;
    uint16_t pixel_height;
    size_t graphic_offset;
    size_t graphic_byte_count;
} CSB_V1_AtariSwitchOption;

typedef struct {
    int valid;
    uint16_t header_segment_count;
    uint16_t option_count;
    int has_palette;
    uint8_t palette[CSB_V1_ATARI_SWITCH_PALETTE_BYTES];
    CSB_V1_AtariSwitchOption options[CSB_V1_ATARI_SWITCH_OPTION_LIMIT];
} CSB_V1_AtariSwitchDatReceipt;

/* Source-faithful expansion receipt for an Atari ST F0466 graphic stream.
 * `row_stride` is a 16-pixel-aligned row, because the original four-plane
 * destination stores complete 16-pixel units even when the visible width is
 * smaller.  Pixels after `visible_width` in a row are source-owned padding. */
typedef struct {
    int valid;
    uint16_t visible_width;
    uint16_t height;
    size_t row_stride;
    size_t pixel_count;
    size_t source_bytes_consumed;
} CSB_V1_AtariSwitchGraphicReceipt;

/* Parses a complete, checksummed Atari ST SWITCH.DAT file.  All option
 * graphics remain ranges in `bytes`; callers must keep that memory alive.
 * Invalid or incomplete source data fails closed. */
int csb_v1_atari_switch_dat_parse(const uint8_t *bytes, size_t byte_count,
                                  CSB_V1_AtariSwitchDatReceipt *out);

/* Expands one source-owned Atari ST graphic span to color indices.  The
 * command stream is the F0466 format documented by ReDMCSB EXPAND.C and
 * CSBWin's ExpandGraphic: byte 0-3 are BE16 width/height, followed by RLE,
 * literal, and previous-row commands.  The output is an indexed projection
 * of the original four-plane bitmap, including its 16-pixel row padding.
 * It never clips malformed commands; the complete padded destination must
 * be filled exactly or the call fails. */
int csb_v1_atari_switch_graphic_decode_indexed(
    const uint8_t *graphic, size_t graphic_byte_count,
    uint8_t *indexed_pixels, size_t pixel_capacity,
    CSB_V1_AtariSwitchGraphicReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_ATARI_SWITCH_DAT_H */
