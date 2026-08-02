#include "dm2_v1_cdda_cd_dat.h"

#include <string.h>

int dm2_v1_cdda_cd_dat_parse(DM2_V1_CddaCdDat *out,
                             const uint8_t *data, size_t size)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    if (!data || size < DM2_CDDA_CD_DAT_SIZE) return 0;

    for (int i = 0; i < (int)DM2_CDDA_CD_DAT_ENTRY_COUNT; ++i) {
        const uint8_t *e = data + i * DM2_CDDA_CD_DAT_ENTRY_SIZE;
        out->entries[i].x     = e[0];
        out->entries[i].y     = e[1];
        out->entries[i].level = e[2];
        out->entries[i].track = e[3];
    }

    out->valid = 1;
    return 1;
}

int dm2_v1_cdda_cd_dat_track_at(const DM2_V1_CddaCdDat *cd,
                                uint8_t level, uint8_t x, uint8_t y)
{
    if (!cd || !cd->valid) return -1;

    for (int i = 0; i < (int)DM2_CDDA_CD_DAT_ENTRY_COUNT; ++i) {
        const DM2_V1_CddaEntry *e = &cd->entries[i];
        if (e->level == level && e->x == x && e->y == y)
            return (int)e->track;
    }
    return -1;
}
