/*
 * dm2_v1_songlist_dat.c — SONGLIST.DAT parser for DM2.
 *
 * Source: skproject c_sound.cpp — tMusicMaps[64] loaded from SONGLIST.DAT.
 * File layout: 44 bytes of map-to-track indices, then 0xFF padding to 63.
 */

#include "dm2_v1_songlist_dat.h"

#include <string.h>

int dm2_v1_songlist_dat_parse(DM2_V1_SonglistDat *out,
                              const uint8_t *data, size_t size) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    if (!data || size < DM2_SONGLIST_FILE_SIZE) return 0;

    memcpy(out->map_to_track, data, DM2_SONGLIST_MAP_COUNT);
    out->valid = 1;
    return 1;
}

int dm2_v1_songlist_dat_track_for_map(const DM2_V1_SonglistDat *sl,
                                      int map_index) {
    if (!sl || !sl->valid) return -1;
    if (map_index < 0 || map_index >= (int)DM2_SONGLIST_MAP_COUNT) return -1;
    if (sl->map_to_track[map_index] == DM2_SONGLIST_INVALID_TRACK) return -1;
    return (int)sl->map_to_track[map_index];
}
