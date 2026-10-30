/* Read-only metadata reader for DM1 X68000 (X30J) GRAPHICS.DAT format 1. */
#ifndef FIRESTAFF_X68K_GRAPHICS_DAT_H
#define FIRESTAFF_X68K_GRAPHICS_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_X68K_GDAT_FORMAT1_MAX_ITEMS 4096u

typedef struct {
    const uint8_t *bytes;
    size_t size;
    uint16_t count;
    size_t payload_offset;
} FirestaffX68kGraphicsDat;

typedef struct {
    uint16_t compressed_size;
    uint16_t decompressed_size;
    uint16_t width;
    uint16_t height;
    size_t payload_offset;
} FirestaffX68kGraphicsDatItem;

/* Accepts only the X30J marked format-1 envelope (BE marker 0x8001). */
int FirestaffX68kGraphicsDat_Open(const uint8_t *bytes,
                                  size_t size,
                                  FirestaffX68kGraphicsDat *out);

/* Returns 0 for an item, -1 for invalid reader/index/metadata. */
int FirestaffX68kGraphicsDat_GetItem(const FirestaffX68kGraphicsDat *dat,
                                     uint16_t index,
                                     FirestaffX68kGraphicsDatItem *out);

#ifdef __cplusplus
}
#endif

#endif
