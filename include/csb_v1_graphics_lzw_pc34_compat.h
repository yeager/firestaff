#ifndef CSB_V1_GRAPHICS_LZW_PC34_COMPAT_H
#define CSB_V1_GRAPHICS_LZW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB LZW.C-compatible decoder for compressed PC GRAPHICS.DAT entries. */
int csb_v1_graphics_lzw_decode_pc34_compat(const uint8_t *input,
                                            size_t input_size,
                                            uint8_t *out,
                                            size_t out_capacity,
                                            size_t *out_size);

#endif
