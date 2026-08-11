#ifndef FIRESTAFF_CSB_V1_X68K_GRAPHICS_HANDOFF_H
#define FIRESTAFF_CSB_V1_X68K_GRAPHICS_HANDOFF_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_x68k_hdm.h"

/* Source-faithful admission of CSB X68000 GRAPHICS.DAT from a Human68k HDM.
 *
 * The X68000 release opens A:\\GRAPHICS.DAT (ReDMCSB MEMORY.C,
 * MEDIA607_X30J_X31J).  Its bytes use the shared big-endian DMCSB2 table
 * layout, but this interface deliberately keeps the media identity X68000:
 * it does not classify the file as an Amiga release merely because both
 * platforms share that container layout. */

typedef struct {
    CSB_V1_X68kHdmReceipt media;
    uint16_t item_count;
    uint16_t direct_item_count;
    uint32_t graphics_byte_count;
} CSB_V1_X68kGraphicsReceipt;

typedef struct {
    uint16_t stored_byte_count;
    uint16_t decoded_byte_count;
    uint32_t data_offset;
} CSB_V1_X68kGraphicsItem;

/* Validate and inventory the complete GRAPHICS.DAT root file.  A successful
 * result proves only its structural DMCSB2 layout and records the actual
 * X68000 HDM receipt; it is not an authenticity claim. */
int csb_v1_x68k_hdm_graphics_receipt(const uint8_t *hdm, size_t hdm_size,
                                     CSB_V1_X68kGraphicsReceipt *out);

/* Return one bounded source item.  data_offset is relative to GRAPHICS.DAT,
 * not the HDM.  The bytes remain owned by the HDM and are never exposed or
 * retained by this boundary. */
int csb_v1_x68k_hdm_graphics_item(const uint8_t *hdm, size_t hdm_size,
                                  uint16_t item_index,
                                  CSB_V1_X68kGraphicsItem *out);

#endif /* FIRESTAFF_CSB_V1_X68K_GRAPHICS_HANDOFF_H */
