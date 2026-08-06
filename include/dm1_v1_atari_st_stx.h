#ifndef FIRESTAFF_DM1_V1_ATARI_ST_STX_H
#define FIRESTAFF_DM1_V1_ATARI_ST_STX_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Reader for the protected Atari ST floppy image used by DM1 1.2.
 *
 * The parser exposes only the source disk's sector stream and its GEMDOS
 * directory. It does not emulate the FDC or invent missing sectors. The
 * supplied DM1 STX has 80 tracks, ten 512-byte sectors per track, four FAT
 * sectors, one root-directory sector, and two sectors per allocation unit.
 */
typedef struct {
    const uint8_t *data;
    size_t size;
    uint32_t track_offsets[256];
    uint8_t track_count;
    uint32_t sector_count;
} DM1_V1_AtariStStx;
typedef DM1_V1_AtariStStx DM1_V1_AtariStx;

int dm1_v1_atari_st_stx_open(const uint8_t *data, size_t size,
                             DM1_V1_AtariStx *out);
int dm1_v1_atari_st_stx_read_sector(const DM1_V1_AtariStx *stx,
                                    uint32_t logical_sector,
                                    uint8_t *out, size_t capacity);
int dm1_v1_atari_st_stx_extract_file(const DM1_V1_AtariStx *stx,
                                     const char *name83,
                                     uint8_t *out, size_t capacity,
                                     size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif
