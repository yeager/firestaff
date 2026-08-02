#ifndef FIRESTAFF_DM2_V1_SONGLIST_DAT_H
#define FIRESTAFF_DM2_V1_SONGLIST_DAT_H

/*
 * dm2_v1_songlist_dat.h — SONGLIST.DAT parser for DM2.
 *
 * SONGLIST.DAT is a 63-byte file mapping dungeon map indices (0-43)
 * to HMP track indices (0-28).  Bytes 44-62 are 0xFF padding.
 *
 * Source: skproject c_sound.cpp tMusicMaps[64] loaded from SONGLIST.DAT.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_SONGLIST_FILE_SIZE      63u
#define DM2_SONGLIST_MAP_COUNT      44u
#define DM2_SONGLIST_INVALID_TRACK  0xFFu

typedef struct {
    int valid;
    uint8_t map_to_track[DM2_SONGLIST_MAP_COUNT];
} DM2_V1_SonglistDat;

int dm2_v1_songlist_dat_parse(DM2_V1_SonglistDat *out,
                              const uint8_t *data, size_t size);

int dm2_v1_songlist_dat_track_for_map(const DM2_V1_SonglistDat *sl,
                                      int map_index);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SONGLIST_DAT_H */
