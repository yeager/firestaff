/* theron_v1_track02_dungeon_map.c — TQ dungeon map loader from Track 02 UD data.
 *
 * Format decoded from gbsphenx/dmbuilder (loaddungeon.c, loadTheronsQuestDungeonData).
 * Offsets are into UD-only extraction starting at sector 225 (pregap skip).
 * Tile format: 1 byte per tile, column-major (x outer, y inner).
 *   Bits 7-5: tile type (0=wall..6=fakewall, 7=TQ-specific)
 *   Bit 4:    has thing list
 *   Bits 3-0: attributes
 */

#include "theron_v1_track02_dungeon_map.h"
#include <string.h>

#define PREGAP_SECTORS 225
#define BYTES_PER_SECTOR 2048
#define UD_BASE (PREGAP_SECTORS * BYTES_PER_SECTOR)

static const Theron_QuestBlockOffsets g_quest_offsets[THERON_TRACK02_DUNGEON_COUNT] = {
    { 0x0002A0F1, 0x0002A2D7, 0x0002A831, 0x0002AD0F, 0x0002B800, 0x0002C9D8 },
    { 0x0006A333, 0x0006A5E9, 0x0006ABA5, 0x0006B031, 0x0006B800, 0x0006CECC },
    { 0x000AA9BC, 0x000AAB7D, 0x000AB035, 0x000AB4C9, 0x000AB800, 0x000ACCC8 },
    { 0x000EA31B, 0x000EA4A8, 0x000EA95C, 0x000EADCE, 0x000EB800, 0x000ED22C },
    { 0x0012AB3C, 0x0012ACB5, 0x0012B22F, 0x0012B6DD, 0x0012B800, 0x0012D106 },
    { 0x0016A034, 0x0016A220, 0x0016A8EC, 0x0016ADEC, 0x0016B800, 0x0016C884 },
    { 0x001AA831, 0x001AA9EC, 0x001AB045, 0x001AB4BD, 0x001AB800, 0x001ACC0E },
};

static const uint8_t g_maps_per_dungeon[THERON_TRACK02_DUNGEON_COUNT] = {
    4, 8, 5, 6, 3, 4, 4
};

static const uint16_t g_text_data_size[THERON_TRACK02_DUNGEON_COUNT] = {
    0x013C, 0x00D0, 0x00E0, 0x00E8, 0x00E0, 0x00D9, 0x00E8,
};

uint16_t theron_v1_track02_dungeon_text_data_size(unsigned int dungeon_index) {
    if (dungeon_index >= THERON_TRACK02_DUNGEON_COUNT) return 0;
    return g_text_data_size[dungeon_index];
}

int theron_v1_track02_dungeon_map_quest_block_offsets(
    unsigned int dungeon_index,
    Theron_QuestBlockOffsets *out)
{
    if (dungeon_index >= THERON_TRACK02_DUNGEON_COUNT || !out) return 0;
    *out = g_quest_offsets[dungeon_index];
    return 1;
}

int theron_v1_track02_dungeon_map_count(unsigned int dungeon_index) {
    if (dungeon_index >= THERON_TRACK02_DUNGEON_COUNT) return 0;
    return g_maps_per_dungeon[dungeon_index];
}

int theron_v1_track02_dungeon_map_load(
    const uint8_t *ud_data,
    size_t ud_size,
    unsigned int dungeon_index,
    Theron_DungeonData *out)
{
    if (!ud_data || !out || dungeon_index >= THERON_TRACK02_DUNGEON_COUNT)
        return 0;

    memset(out, 0, sizeof(*out));
    out->dungeon_index = (uint8_t)dungeon_index;

    uint8_t nmaps = g_maps_per_dungeon[dungeon_index];
    out->map_count = nmaps;

    const Theron_QuestBlockOffsets *qb = &g_quest_offsets[dungeon_index];

    size_t dims_abs = UD_BASE + qb->dims_offset;
    if (dims_abs + 9u * nmaps + 32 > ud_size) return 0;

    const uint8_t *p = ud_data + dims_abs;

    uint8_t xdims[THERON_TRACK02_MAX_MAPS];
    uint8_t ydims[THERON_TRACK02_MAX_MAPS];
    memcpy(xdims, p, nmaps); p += nmaps;
    memcpy(ydims, p, nmaps); p += nmaps;

    uint8_t xoffs[THERON_TRACK02_MAX_MAPS];
    uint8_t yoffs[THERON_TRACK02_MAX_MAPS];
    memcpy(xoffs, p, nmaps); p += nmaps;
    memcpy(yoffs, p, nmaps); p += nmaps;

    uint8_t mapids[THERON_TRACK02_MAX_MAPS];
    memcpy(mapids, p, nmaps); p += nmaps;

    uint8_t unk1[THERON_TRACK02_MAX_MAPS];
    uint8_t unk2[THERON_TRACK02_MAX_MAPS];
    memcpy(unk1, p, nmaps); p += nmaps;
    memcpy(unk2, p, nmaps); p += nmaps;

    uint8_t creatures[THERON_TRACK02_MAX_MAPS];
    memcpy(creatures, p, nmaps); p += nmaps;

    uint8_t xp_mod[THERON_TRACK02_MAX_MAPS];
    memcpy(xp_mod, p, nmaps); p += nmaps;

    memcpy(out->object_counts, p, 32); p += 32;

    for (unsigned int m = 0; m < nmaps; m++) {
        Theron_MapHeader *h = &out->maps[m].header;
        h->x_dim = xdims[m];
        h->y_dim = ydims[m];
        h->x_offset = xoffs[m];
        h->y_offset = yoffs[m];
        h->map_id = mapids[m];
        h->unk1 = unk1[m];
        h->unk2 = unk2[m];
        h->creature_count = creatures[m];
        h->xp_modifier = xp_mod[m];
    }

    size_t door_off = (size_t)(p - ud_data);
    for (unsigned int m = 0; m < nmaps; m++) {
        if (door_off + 2 > ud_size) return 0;
        out->maps[m].header.door_type1 = ud_data[door_off++];
        out->maps[m].header.door_type2 = ud_data[door_off++];
    }

    size_t map_abs = UD_BASE + qb->map_data_offset;
    if (map_abs >= ud_size) return 0;

    size_t pos = map_abs;
    for (unsigned int m = 0; m < nmaps; m++) {
        unsigned int w = (unsigned int)xdims[m] + 1;
        unsigned int h = (unsigned int)ydims[m] + 1;
        if (w > THERON_TRACK02_MAX_MAP_DIM || h > THERON_TRACK02_MAX_MAP_DIM)
            return 0;
        if (pos + (size_t)w * h > ud_size) return 0;
        for (unsigned int x = 0; x < w; x++) {
            for (unsigned int y = 0; y < h; y++) {
                out->maps[m].tiles[x][y] = ud_data[pos++];
            }
        }
    }

    return 1;
}

Theron_TileType theron_tile_type(uint8_t tile_byte) {
    return (Theron_TileType)(tile_byte >> 5);
}

int theron_tile_has_things(uint8_t tile_byte) {
    return (tile_byte >> 4) & 1;
}

uint8_t theron_tile_attributes(uint8_t tile_byte) {
    return tile_byte & 0x0F;
}
