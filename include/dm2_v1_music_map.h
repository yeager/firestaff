#ifndef FIRESTAFF_DM2_V1_MUSIC_MAP_H
#define FIRESTAFF_DM2_V1_MUSIC_MAP_H

/*
 * dm2_v1_music_map.h — Generalized DM2 music mapping (CD.DAT / md.dat).
 *
 * Three distinct music map formats exist across DM2 platforms:
 *
 * 1. PC SONGLIST.DAT: 63 bytes, one byte per map (handled by dm2_v1_songlist_dat.h)
 * 2. Amiga CD.DAT / Mac md.dat: 176 bytes, 44 x 4-byte entries [0xFF 0xFF map track]
 * 3. FM Towns/Mega CD/PC-9821 CD.DAT: 40 bytes, CDDA Red Book format
 *
 * This header handles format 2 (the 176-byte map-to-track format) with
 * configurable track count to support both MOD (10 tracks) and HMP (29 tracks).
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_MUSIC_MAP_FILE_SIZE   176u
#define DM2_MUSIC_MAP_ENTRY_SIZE  4u
#define DM2_MUSIC_MAP_FILE_ENTRIES 44u
#define DM2_MUSIC_TRACK_NONE      0xFFu

#define DM2_MUSIC_TRACK_MAX_MOD   10u
#define DM2_MUSIC_TRACK_MAX_HMP   29u

typedef enum {
    DM2_MUSIC_KIND_MOD = 0,
    DM2_MUSIC_KIND_HMP = 1
} DM2_MusicKind;

typedef struct {
    int valid;
    DM2_MusicKind kind;
    uint8_t max_tracks;
    uint8_t map_to_track[DM2_MUSIC_MAP_FILE_ENTRIES];
} DM2_V1_MusicMap;

int dm2_v1_music_map_parse(DM2_V1_MusicMap *out,
                           const uint8_t *data, size_t size,
                           DM2_MusicKind kind);

int dm2_v1_music_map_track_for_map(const DM2_V1_MusicMap *mm,
                                   int map_index);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_MUSIC_MAP_H */
