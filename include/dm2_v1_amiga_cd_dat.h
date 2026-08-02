#ifndef FIRESTAFF_DM2_V1_AMIGA_CD_DAT_H
#define FIRESTAFF_DM2_V1_AMIGA_CD_DAT_H

/*
 * dm2_v1_amiga_cd_dat.h — Amiga CD.DAT parser for DM2.
 *
 * CD.DAT on Amiga is a 176-byte file containing 44 entries of 4 bytes each.
 * Each entry maps a dungeon map index to a ProTracker MOD track (SK00-SK09).
 *
 * Entry format (4 bytes):
 *   byte 0-1: 0xFFFF (flags/padding)
 *   byte 2:   map index (0-43)
 *   byte 3:   MOD track index (0-9, maps to SK00.MOD through SK09.MOD)
 *
 * This is the Amiga equivalent of the PC's SONGLIST.DAT, but uses a different
 * format and maps to MOD files instead of HMP tracks.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_AMIGA_CD_DAT_SIZE       176u
#define DM2_AMIGA_CD_DAT_ENTRY_SIZE 4u
#define DM2_AMIGA_CD_DAT_MAP_COUNT  44u
#define DM2_AMIGA_MOD_TRACK_COUNT   10u
#define DM2_AMIGA_MOD_TRACK_NONE    0xFFu

typedef struct {
    int valid;
    uint8_t map_to_mod[DM2_AMIGA_CD_DAT_MAP_COUNT];
} DM2_V1_AmigaCdDat;

/* Parse the Amiga CD.DAT file into a map-to-MOD lookup table.
 * Returns 1 on success, 0 on invalid data. */
int dm2_v1_amiga_cd_dat_parse(DM2_V1_AmigaCdDat *out,
                              const uint8_t *data, size_t size);

/* Look up the MOD track index for a given map. Returns 0-9 on success,
 * -1 if the map has no music or the lookup is invalid. */
int dm2_v1_amiga_cd_dat_mod_for_map(const DM2_V1_AmigaCdDat *cd,
                                    int map_index);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_AMIGA_CD_DAT_H */
