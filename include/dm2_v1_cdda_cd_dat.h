#ifndef FIRESTAFF_DM2_V1_CDDA_CD_DAT_H
#define FIRESTAFF_DM2_V1_CDDA_CD_DAT_H

/*
 * dm2_v1_cdda_cd_dat.h — DM2 CDDA Red Book music trigger format.
 *
 * Used by FM Towns, Mega CD, and PC-9821.
 * 40 bytes = 10 entries × 4 bytes: [X, Y, level_index, track_index]
 *
 * Unlike the 176-byte Amiga/Mac format (which maps by map index),
 * this format triggers music at specific (X,Y) coordinates within a level.
 * Checked each time the party moves.
 *
 * Source: DMWeb "DM2 Music Triggers" — Format 1 (level coordinates).
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_CDDA_CD_DAT_SIZE        40u
#define DM2_CDDA_CD_DAT_ENTRY_SIZE  4u
#define DM2_CDDA_CD_DAT_ENTRY_COUNT 10u

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t level;
    uint8_t track;
} DM2_V1_CddaEntry;

typedef struct {
    int valid;
    DM2_V1_CddaEntry entries[DM2_CDDA_CD_DAT_ENTRY_COUNT];
} DM2_V1_CddaCdDat;

int dm2_v1_cdda_cd_dat_parse(DM2_V1_CddaCdDat *out,
                             const uint8_t *data, size_t size);

int dm2_v1_cdda_cd_dat_track_at(const DM2_V1_CddaCdDat *cd,
                                uint8_t level, uint8_t x, uint8_t y);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CDDA_CD_DAT_H */
