#include "dm1_v1_fmtowns_font_asset.h"

/* Source-locked font-asset identity for the FM Towns DM1 menu.
 * All values decoded from INIT_TEXT + GET_MY_DECODED in EDM.EXP.
 * See parity-evidence/dm1_fmtowns_font_asset.md. */

uint16_t dm1_v1_fmtowns_font_pic_library_index_pc34(void) {
    return (uint16_t)DM1_V1_FMTOWNS_FONT_PIC_LIB_INDEX;
}

uint16_t dm1_v1_fmtowns_font_decode_pic_index_pc34(uint32_t my_decoded_arg) {
    return (uint16_t)(my_decoded_arg & DM1_V1_FMTOWNS_FONT_MY_DECODED_ID_MASK);
}

int dm1_v1_fmtowns_font_is_direct_to_buffer_pc34(uint32_t my_decoded_arg) {
    return (my_decoded_arg & DM1_V1_FMTOWNS_FONT_MY_DECODED_DIRECT_MASK) != 0;
}

int dm1_v1_fmtowns_font_is_no_size_header_pc34(uint32_t my_decoded_arg) {
    return (my_decoded_arg & DM1_V1_FMTOWNS_FONT_MY_DECODED_NO_HDR_MASK) != 0;
}

size_t dm1_v1_fmtowns_font_raster_alloc_bytes_pc34(void) {
    return DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES;
}
