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

static int theron_map_range_fits(size_t offset, size_t length, size_t size) {
    return offset <= size && length <= size - offset;
}

static const Theron_QuestBlockOffsets g_quest_offsets[THERON_TRACK02_DUNGEON_COUNT] = {
    { 0x0002A0F1, 0x0002A2D7, 0x0002A831, 0x0002AD0F, 0x0002B800, 0x0002C9D8 },
    { 0x0006A333, 0x0006A5E9, 0x0006ABA5, 0x0006B031, 0x0006B800, 0x0006CECC },
    { 0x000AA9BC, 0x000AAB7D, 0x000AB035, 0x000AB4C9, 0x000AB800, 0x000ACCC8 },
    { 0x000EA31B, 0x000EA4A8, 0x000EA95C, 0x000EADCE, 0x000EB800, 0x000ED22C },
    { 0x0012AB3C, 0x0012ACB5, 0x0012B22F, 0x0012B6DD, 0x0012B800, 0x0012D106 },
    { 0x0016A034, 0x0016A220, 0x0016A8EC, 0x0016ADEC, 0x0016B800, 0x0016C884 },
    { 0x001AA831, 0x001AA9EC, 0x001AB045, 0x001AB4BD, 0x001AB800, 0x001ACC0E },
};

/* Japanese raw-BIN map records are source-data bindings, not translated
 * guesses: dimensions, map bytes, ground-reference tables, and item-part
 * boundaries were matched in the authenticated TQJP02.bin user-data image.
 * The field order is corroborated by DMBUILDER6/src/loaddungeon.c
 * loadTheronsQuestDungeonData().  JP text bytes are localized, so their
 * offsets are carried by the same record-size arithmetic as the US layout. */
static const Theron_QuestBlockOffsets g_jp_quest_offsets[THERON_TRACK02_DUNGEON_COUNT] = {
    { 0x0002991D, 0x00029B03, 0x0002A05D, 0x0002A53B, 0x0002B000, 0x0002C128 },
    { 0x00069D50, 0x0006A006, 0x0006A5C2, 0x0006AA4E, 0x0006B000, 0x0006C6CC },
    { 0x000AA261, 0x000AA422, 0x000AA8DA, 0x000AAD6E, 0x000AB000, 0x000AC4C8 },
    { 0x000E9B47, 0x000E9CD4, 0x000EA188, 0x000EA5FA, 0x000EB000, 0x000ECA2C },
    { 0x0012A3CB, 0x0012A544, 0x0012AABE, 0x0012AF6C, 0x0012B000, 0x0012C906 },
    { 0x00169860, 0x00169A4C, 0x0016A118, 0x0016A618, 0x0016B000, 0x0016C084 },
    { 0x001AA043, 0x001AA1FE, 0x001AA857, 0x001AACCF, 0x001AB000, 0x001AC40E },
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

int theron_v1_track02_dungeon_map_quest_block_offsets_for_variant(
    Theron_Track02Variant variant,
    unsigned int dungeon_index,
    Theron_QuestBlockOffsets *out)
{
    if (!out || dungeon_index >= THERON_TRACK02_DUNGEON_COUNT) return 0;
    if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        *out = g_quest_offsets[dungeon_index];
        return 1;
    }
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        *out = g_jp_quest_offsets[dungeon_index];
        return 1;
    }
    return 0;
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
    return theron_v1_track02_dungeon_map_load_for_variant(
        ud_data, ud_size, THERON_TRACK02_VARIANT_US_BIN,
        dungeon_index, out);
}

int theron_v1_track02_dungeon_map_load_for_variant(
    const uint8_t *ud_data,
    size_t ud_size,
    Theron_Track02Variant variant,
    unsigned int dungeon_index,
    Theron_DungeonData *out)
{
    if (!ud_data || !out || dungeon_index >= THERON_TRACK02_DUNGEON_COUNT)
        return 0;

    memset(out, 0, sizeof(*out));
    out->dungeon_index = (uint8_t)dungeon_index;

    uint8_t nmaps = g_maps_per_dungeon[dungeon_index];
    out->map_count = nmaps;

    Theron_QuestBlockOffsets qb;
    if (!theron_v1_track02_dungeon_map_quest_block_offsets_for_variant(
            variant, dungeon_index, &qb)) return 0;

    size_t dims_abs = UD_BASE + qb.dims_offset;
    if (!theron_map_range_fits(dims_abs, 9u * nmaps + 32u, ud_size)) return 0;

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

    /* Two 16-bit data lists per map, after door types.
     * List 1: creature graphics bank (0x0003 = standard, 0x0000 = FORMICIA).
     * List 2: cumulative column item counts. */
    size_t list_off = door_off;
    for (unsigned int m = 0; m < nmaps; m++) {
        if (list_off + 2 > ud_size) return 0;
        out->creature_gfx_bank[m] = (uint16_t)ud_data[list_off] |
            ((uint16_t)ud_data[list_off + 1] << 8);
        list_off += 2;
    }
    for (unsigned int m = 0; m < nmaps; m++) {
        if (list_off + 2 > ud_size) return 0;
        out->cumulative_column_items[m] = (uint16_t)ud_data[list_off] |
            ((uint16_t)ud_data[list_off + 1] << 8);
        list_off += 2;
    }

    /* Per-column cumulative thing counts: sum(xdim) entries as uint16 LE.
     * Used by the thing list to locate things per map column. */
    unsigned int total_columns = 0;
    for (unsigned int m = 0; m < nmaps; m++)
        total_columns += xdims[m];
    if (total_columns > THERON_TRACK02_MAX_COLUMNS) return 0;
    out->column_thing_count_total = (uint16_t)total_columns;
    if (!theron_map_range_fits(list_off,
                               (size_t)total_columns * sizeof(uint16_t) + 4u,
                               ud_size)) return 0;
    for (unsigned int i = 0; i < total_columns; i++) {
        out->column_thing_counts[i] = (uint16_t)ud_data[list_off] |
            ((uint16_t)ud_data[list_off + 1] << 8);
        list_off += 2;
    }

    /* 4 unknown bytes between column counts and thing descriptor size table. */
    list_off += 4u;

    /* 12-byte thing descriptor size table — bytes per record for each of the
     * 12 thing types (door, teleporter, text, actuator, creature, weapon,
     * clothing, scroll, potion, container, misc_item, missile). */
    if (!theron_map_range_fits(list_off, THERON_TRACK02_THING_TYPE_COUNT,
                               ud_size)) return 0;
    memcpy(out->thing_descriptor_sizes, ud_data + list_off,
           THERON_TRACK02_THING_TYPE_COUNT);
    list_off += THERON_TRACK02_THING_TYPE_COUNT;

    /* Remaining bytes before map tile data are thing list records. */
    size_t map_abs = UD_BASE + qb.map_data_offset;
    out->thing_list_offset = list_off;
    out->thing_list_size = (list_off < map_abs) ? (map_abs - list_off) : 0;

    if (map_abs >= ud_size) return 0;

    size_t pos = map_abs;
    for (unsigned int m = 0; m < nmaps; m++) {
        unsigned int w = (unsigned int)xdims[m] + 1;
        unsigned int h = (unsigned int)ydims[m] + 1;
        if (w > THERON_TRACK02_MAX_MAP_DIM || h > THERON_TRACK02_MAX_MAP_DIM)
            return 0;
        if (!theron_map_range_fits(pos, (size_t)w * h, ud_size)) return 0;
        for (unsigned int x = 0; x < w; x++) {
            for (unsigned int y = 0; y < h; y++) {
                out->maps[m].tiles[x][y] = ud_data[pos++];
            }
        }
    }

    return 1;
}
