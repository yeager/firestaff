#ifndef FIRESTAFF_DM2_V1_SONGLIST_DAT_H
#define FIRESTAFF_DM2_V1_SONGLIST_DAT_H

/*
 * dm2_v1_songlist_dat.h — SONGLIST.DAT parser for DM2.
 *
 * SONGLIST.DAT is a 63-byte prefix of SKProject's 64-byte tblMusicsMap
 * table.  Every byte is source-owned: the DOS corpus retains valid HMP
 * selectors at offsets 44 and 45, followed by 0xFF no-music slots.
 *
 * Source: SKProject SKWINSPX/src/v5/dm2data.cpp:217-225
 * (`tblMusicsMap[64]`) and sfxsnd.cpp:493.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_SONGLIST_FILE_SIZE      63u
#define DM2_SONGLIST_MAP_COUNT      DM2_SONGLIST_FILE_SIZE
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
