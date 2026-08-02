/*
 * dm2_v1_amiga_cd_dat.c — Amiga CD.DAT parser for DM2.
 *
 * Source: Amiga DM2 game data, 176-byte CD.DAT.
 * Format: 44 entries × 4 bytes = [0xFF 0xFF map_index mod_track].
 */

#include "dm2_v1_amiga_cd_dat.h"

#include <string.h>

int dm2_v1_amiga_cd_dat_parse(DM2_V1_AmigaCdDat *out,
                              const uint8_t *data, size_t size)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    memset(out->map_to_mod, DM2_AMIGA_MOD_TRACK_NONE,
           sizeof(out->map_to_mod));

    if (!data || size < DM2_AMIGA_CD_DAT_SIZE) return 0;

    for (int i = 0; i < (int)DM2_AMIGA_CD_DAT_MAP_COUNT; ++i) {
        const uint8_t *entry = data + i * DM2_AMIGA_CD_DAT_ENTRY_SIZE;
        uint8_t map_idx = entry[2];
        uint8_t mod_idx = entry[3];

        if (map_idx >= DM2_AMIGA_CD_DAT_MAP_COUNT) continue;
        if (mod_idx >= DM2_AMIGA_MOD_TRACK_COUNT) continue;

        out->map_to_mod[map_idx] = mod_idx;
    }

    out->valid = 1;
    return 1;
}

int dm2_v1_amiga_cd_dat_mod_for_map(const DM2_V1_AmigaCdDat *cd,
                                    int map_index)
{
    if (!cd || !cd->valid) return -1;
    if (map_index < 0 || map_index >= (int)DM2_AMIGA_CD_DAT_MAP_COUNT)
        return -1;

    uint8_t track = cd->map_to_mod[map_index];
    if (track == DM2_AMIGA_MOD_TRACK_NONE) return -1;
    return (int)track;
}
