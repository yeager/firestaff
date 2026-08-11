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

/* Parses a complete, checksummed Atari ST SWITCH.DAT file.  All option
 * graphics remain ranges in `bytes`; callers must keep that memory alive.
 * Invalid or incomplete source data fails closed. */
int csb_v1_atari_switch_dat_parse(const uint8_t *bytes, size_t byte_count,
                                  CSB_V1_AtariSwitchDatReceipt *out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_ATARI_SWITCH_DAT_H */
