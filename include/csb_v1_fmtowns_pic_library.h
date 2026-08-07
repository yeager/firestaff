#ifndef CSB_V1_FMTOWNS_PIC_LIBRARY_H
#define CSB_V1_FMTOWNS_PIC_LIBRARY_H

#include <stddef.h>
#include <stdint.h>
#include "dm1_v1_fmtowns_pic_library.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked CSB CDATA/GRAPHICS.DAT picture library.
 *
 * Byte-verified 2026-08-07: CSB's ext_v1 GRAPHICS.DAT format is
 * IDENTICAL to DM1's legacy pic_library format PLUS a 2-byte
 * signature prefix (0x8001):
 *
 *   [u16 sig=0x8001]
 *   [u16 count=728]                (CSB has 728 assets)
 *   [u16 primary_sizes[count]]     (byte-identical to secondary)
 *   [u16 secondary_sizes[count]]
 *   [payload bytes]
 *
 * Sum of primary sizes = 0x60c9e = 396446 bytes; fits within the
 * 399358-byte payload space (2 KB tail padding).
 *
 * This module wraps a CSB GRAPHICS.DAT buffer as a DM1 pic_library
 * view by skipping the 2-byte sig, letting every DM1 pic_library
 * consumer read CSB assets unchanged.
 */

/* Open a CSB CDATA/GRAPHICS.DAT blob as a DM1 pic_library view.
 * The `data` buffer must remain valid for the lifetime of the view
 * (aliased, not copied). Returns 1 on success; 0 if the header
 * signature is not 0x8001 or the buffer is too small. */
int csb_v1_fmtowns_pic_library_open_ext_v1_pc34(
    const uint8_t                                *data,
    size_t                                         data_size,
    dm1_v1_fmtowns_pic_library_view_t             *out_view);

#ifdef __cplusplus
}
#endif

#endif /* CSB_V1_FMTOWNS_PIC_LIBRARY_H */
