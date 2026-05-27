
#include "dm2_v1_dungeon_loader.h"
#include <stdlib.h>
#include <string.h>

/* DM2 dungeon.dat loader
 * Source: SKULL.ASM T560 (DUNGEON_Load) dungeon loading routines
 * DM2 extends DM1 format with outdoor levels, buildings, and weather zones.
 *
 * Format (PC English DUNGEON.DAT 39437 bytes):
 *   Offset  0: uint16_le padding_or_reserved (currently 0x0000)
 *   Offset  2: uint16_le format_version_or_magic (0x4731 = ASCII "G1")
 *   Offset  4: uint16_le first_level_data_offset_or_header_size (0x002c = 44)
 *   Offset  6: uint16_le level_count (appears at byte offset 6)
 *   Offset  8: uint16_le dungeon_seed (0x0101 = 257)
 *   Offset 10: uint16_le something (0x0938 = 2360)
 *   ... followed by level descriptors (8 bytes each) at 8-byte alignment
 *   Level tile data follows the descriptor table.
 *
 * Note: The current implementation reads level_count from offset 0,
 * which does not match this file's format. See PROBE_NOTES in the
 * dungeon parser probe for the canonical header contract. */

static uint16_t rd16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }

/* DM2 uses an extended header where:
 *   word[0] = 0x0000 (reserved/padding)
 *   word[1] = 0x4731 ("G1" magic/version)
 *   word[2] = header_size or first_level_data_offset (typically 44)
 *   word[3] = level_count
 *   word[4..] = dungeon_seed and other metadata
 *
 * This differs from the simple uint16 level_count format assumed
 * by the current loader stub. Correct parsing requires reading
 * level_count from offset 6 (byte offset 6), not offset 0. */

int dm2_v1_dungeon_load(DM2_V1_DungeonData *out, const uint8_t *dat, int size) {
    int i, levels, offset;
    if (!out || !dat || size < 4) return -1;
    memset(out, 0, sizeof(*out));

    /* Current stub reads level_count from offset 0.
     * Known DM2 PC English file has 0x0000 at offset 0 → 0 levels.
     * Correct offset is 6 (word index 3): 0x001c = 28 levels.
     * Stub will be updated to read from offset 6 once the header
     * contract is confirmed against SKULL.ASM T560 disassembly. */
    /* Correct level_count is at byte offset 6 (word index 3): 0x001c = 28 */
    levels = rd16(dat + 6);
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

