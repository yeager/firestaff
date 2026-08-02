/*
 * dm2_v1_music_map.c — Generalized DM2 music mapping parser.
 *
 * Handles the 176-byte map-to-track format used by:
 *   - Amiga CD.DAT (10 MOD tracks: SK00-SK09)
 *   - Mac md.dat (29 HMP tracks)
 *
 * Format: 44 entries × 4 bytes = [0xFF 0xFF map_index track_index]
 */

#include "dm2_v1_music_map.h"

#include <string.h>

int dm2_v1_music_map_parse(DM2_V1_MusicMap *out,
                           const uint8_t *data, size_t size,
                           DM2_MusicKind kind)
{
    uint8_t max_tracks;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    memset(out->map_to_track, DM2_MUSIC_TRACK_NONE,
           sizeof(out->map_to_track));

    if (!data || size < DM2_MUSIC_MAP_FILE_SIZE) return 0;

    max_tracks = (kind == DM2_MUSIC_KIND_MOD)
        ? DM2_MUSIC_TRACK_MAX_MOD
        : DM2_MUSIC_TRACK_MAX_HMP;

    for (int i = 0; i < (int)DM2_MUSIC_MAP_FILE_ENTRIES; ++i) {
        const uint8_t *entry = data + i * DM2_MUSIC_MAP_ENTRY_SIZE;
        uint8_t map_idx = entry[2];
        uint8_t track = entry[3];

        if (map_idx >= DM2_MUSIC_MAP_FILE_ENTRIES) continue;
        if (track >= max_tracks) continue;

        out->map_to_track[map_idx] = track;
    }

    out->kind = kind;
    out->max_tracks = max_tracks;
    out->valid = 1;
    return 1;
}

int dm2_v1_music_map_track_for_map(const DM2_V1_MusicMap *mm,
                                   int map_index)
{
    if (!mm || !mm->valid) return -1;
    if (map_index < 0 || map_index >= (int)DM2_MUSIC_MAP_FILE_ENTRIES)
        return -1;

    uint8_t track = mm->map_to_track[map_index];
    if (track == DM2_MUSIC_TRACK_NONE) return -1;
    return (int)track;
}
