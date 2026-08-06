#ifndef FIRESTAFF_DM1_V1_ATARI_ST_GRAPHICS_DAT_H
#define FIRESTAFF_DM1_V1_ATARI_ST_GRAPHICS_DAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM1 Atari ST GRAPHICS.DAT: DMCSB1-style BE size tables, 563 records.
 * The records are Atari-LZW or raw source bytes.  Pixel interpretation is
 * deliberately separate: DMWeb/ReDMCSB distinguish the ST IMG1/IMG2 path
 * from the PC34 IMG3 path. */
#define DM1_V1_ATARI_ST_GRAPHICS_COUNT 563u

typedef struct {
    uint16_t compressed_size;
    uint16_t expanded_size;
    size_t offset;
} DM1_V1_AtariStGraphic;

typedef struct {
    const uint8_t *data;
    size_t size;
    DM1_V1_AtariStGraphic records[DM1_V1_ATARI_ST_GRAPHICS_COUNT];
} DM1_V1_AtariStGraphicsDat;

int dm1_v1_atari_st_graphics_open(const uint8_t *data, size_t size,
                                  DM1_V1_AtariStGraphicsDat *out);
int dm1_v1_atari_st_graphics_read(const DM1_V1_AtariStGraphicsDat *dat,
                                  uint16_t index, uint8_t *out,
                                  size_t capacity);

#ifdef __cplusplus
}
#endif

#endif
