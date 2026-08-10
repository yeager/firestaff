#ifndef FIRESTAFF_CSB_V1_CSBWIN_OBJDESC_022F_H
#define FIRESTAFF_CSB_V1_CSBWIN_OBJDESC_022F_H

#include <stddef.h>
#include <stdint.h>

/* CSBWin Data.h places GRAPHICS.DAT item 0x22f at Byte10340 and expands it
 * to exactly 0xc0e bytes (CSBCode.cpp::ReadTablesFromGraphicsFile).  The
 * object-description table begins at Byte8382, i.e. offset 10340 - 8382 =
 * 0x7a6, and has 180 six-byte OBJDESC records.  CSB.h::OBJDESC proves the
 * on-media order: big-endian objType, graphicClass, signed attackClass,
 * big-endian carry-location word4.  CSBWin calls OBJDESC::littleEndian() for
 * the two words after expansion.  This module is deliberately read-only;
 * consumers must not infer a renderer or a PC graphics fallback from it. */
#define CSB_V1_CSBWIN_OBJDESC_022F_ITEM_INDEX 0x22fu
#define CSB_V1_CSBWIN_OBJDESC_022F_DECODED_SIZE 0x0c0eu
#define CSB_V1_CSBWIN_OBJDESC_022F_TABLE_OFFSET 0x07a6u
#define CSB_V1_CSBWIN_OBJDESC_022F_COUNT 180u
#define CSB_V1_CSBWIN_OBJDESC_022F_RECORD_SIZE 6u

typedef struct {
    int16_t object_type;
    uint8_t graphic_class;
    int8_t attack_class;
    uint16_t carry_locations;
} CSB_V1_CSBWinObjectDescription022f;

typedef struct {
    int valid;
    CSB_V1_CSBWinObjectDescription022f entries[
        CSB_V1_CSBWIN_OBJDESC_022F_COUNT];
} CSB_V1_CSBWinObjectDescriptionTable022f;

/* Decode an already expanded Atari ST / CSBWin 0x22f item. */
int csb_v1_csbwin_objdesc_022f_decode(
    const uint8_t *decoded_graphic, size_t decoded_size,
    CSB_V1_CSBWinObjectDescriptionTable022f *out_table);

/* Read the original 563-item Atari ST / CSBWin GRAPHICS.DAT catalog,
 * decompress item 0x22f and decode its 180 OBJDESC records. */
int csb_v1_csbwin_objdesc_022f_read_graphics_dat(
    const char *graphics_dat_path,
    CSB_V1_CSBWinObjectDescriptionTable022f *out_table);

#endif
