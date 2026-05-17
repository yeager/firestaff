
#include "dm2_v1_dungeon_loader.h"
#include <stdlib.h>
#include <string.h>

/* DM2 dungeon.dat loader
 * Source: SKULL.ASM dungeon loading routines
 * DM2 extends DM1 format with outdoor levels and buildings. */

static uint16_t rd16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }

int dm2_v1_dungeon_load(DM2_V1_DungeonData *out, const uint8_t *dat, int size) {
    int i, levels, offset;
    if (!out || !dat || size < 4) return -1;
    memset(out, 0, sizeof(*out));
    levels = rd16(dat);
    if (levels > DM2_V1_MAX_LEVELS) levels = DM2_V1_MAX_LEVELS;
    out->level_count = levels;
    offset = 4;
    for (i = 0; i < levels && offset + 8 <= size; i++) {
        out->level_types[i] = (DM2_LevelType)dat[offset];
        out->level_widths[i] = dat[offset + 1];
        out->level_heights[i] = dat[offset + 2];
        out->level_offsets[i] = (int)(rd16(dat + offset + 4) | ((uint32_t)rd16(dat + offset + 6) << 16));
        offset += 8;
    }
    out->raw_data = (uint8_t *)malloc(size);
    if (out->raw_data) { memcpy(out->raw_data, dat, size); out->raw_size = size; }
    return 0;
}

int dm2_v1_dungeon_get_square_type(const DM2_V1_DungeonData *d, int level, int x, int y) {
    int offset, w;
    if (!d || !d->raw_data || level < 0 || level >= d->level_count) return -1;
    w = d->level_widths[level];
    if (x < 0 || x >= w || y < 0 || y >= d->level_heights[level]) return -1;
    offset = d->level_offsets[level] + (x * d->level_heights[level] + y) * 2;
    if (offset + 2 > d->raw_size) return -1;
    return rd16(d->raw_data + offset) & 0x1F;
}

int dm2_v1_dungeon_is_outdoor(const DM2_V1_DungeonData *d, int level) {
    if (!d || level < 0 || level >= d->level_count) return 0;
    return d->level_types[level] == DM2_LEVEL_OUTDOOR;
}

void dm2_v1_dungeon_free(DM2_V1_DungeonData *d) {
    if (d && d->raw_data) { free(d->raw_data); d->raw_data = NULL; }
}

const char *dm2_v1_dungeon_source_evidence(void) {
    return "SKULL.ASM: DM2 dungeon loading routines\n"
           "DM2 extensions: outdoor levels, buildings, weather zones\n"
           "Format: enhanced DM1 dungeon.dat with level type byte\n";
}

